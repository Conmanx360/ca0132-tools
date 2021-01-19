/*
 * ca0132-8051-command-line:
 * Allows use of the onboard 8051's serial command console by storing commands
 * in the buffer and updating the write pointer.
 *
 * Commands are:
 * ver   - Prints out device version information. Same data for all ca0132
 *	   codecs it seems.
 * echo  - On or off. Turns off command echo in console.
 * sfr   - Takes an address, returns the value of the 8051 SFR at that address.
 * dmem  - Takes an address, returns the value of the 8051 iram at that address.
 * xmem  - Takes an address, returns the value of the 8051 xram at that address.
 * cmem  - Takes an address, returns the value of the 8051 pmem at that address.
 * hci   - Takes an address, returns the value of the HCI bus at that address.
 *         The HCI bus is the chipio data interface. For some reason, in the
 *         driver, it's referred to as both HCI and HIC.
 * apb   - Takes an address, returns the value of the APB bus at that address.
 *         Not sure what this address bus is used for, if it's in use at all.
 *         Range is only 8-bits, returns a 32-bit value.
 * hda   - Takes an address, returns data at that address. This seems to be
 *         the actual HDA link interface control, uses SFR 0xdb for the
 *         address, and 0xdc for the data. Things written here are streamID's
 *         and stream format values. Range is 8-bits.
 * pmu   - Takes an address, returns the value of the 8051 PLL PMU at that address.
 * eepr  - Takes an address, returns data. Not sure if the eeprom actually
 *         exists, or if this is just left over from development.
 * i2c   - Has multiple forms:
 *         i2c target <addr> - sets the master target address.
 *         i2c mode <op_mode> <7/10> - Sets the op and address modes. op modes
 *                                     are: slave, std, fast, fast+.
 *         i2c rd <cnt> - Reads 1-8 bytes from i2c.
 *         i2c wr <<b0> ... <b7>> - Writes 1-8 bytes to i2c.
 *         None of these seem functional, reads/writes always return I/O
 *         error.
 * flag  - With no arguments, prints all flag values. With a value argument,
 *	   prints out the flag value, string, and if it's set.
 * ctrl  - Same as flag command, either prints all chipio_control_param values
 *         or a specific one specified in an argument.
 * log   - Prints a log of some sort, not sure from where.
 * pin   - Prints out HDA node pin values. Not sure what they represent.
 * codec - Takes a codec number value, seems to print out the codecs power state.
 * q     - Prints out the high water mark of the verb buffer.
 */
#include "ca0132_defs.h"

#define IN_BUF_BASE_PTR 0x1316
#define IN_BUF_WRITE_PTR 0x1318
#define OUT_BUF_BASE_PTR 0x1317
#define OUT_BUF_WRITE_PTR 0x1319

#define IN_BUF_ADDR	0x1196
#define OUT_BUF_ADDR	0x1216

#define IN_BUF_LEN      0x80
#define OUT_BUF_LEN     0x100

struct serial_buffer_offset {
	uint32_t part1_cnt;
	uint32_t part2_cnt;

	uint32_t write_ptr;
};

static void usage(char *pname)
{
	fprintf(stderr, "usage: %s <hwdep-device>\n", pname);
}

static void get_serial_buffer_offset(struct serial_buffer_offset *data,
		uint32_t start_ptr, uint32_t cnt, uint32_t buf_size)
{
	memset(data, 0, sizeof(*data));
	if (start_ptr + cnt > buf_size) {
		data->part1_cnt = buf_size - start_ptr;
		data->part2_cnt = (cnt - data->part1_cnt);
		data->write_ptr = data->part2_cnt;
	} else {
		data->part1_cnt = cnt;
		data->write_ptr = start_ptr + cnt;
	}
}

static void write_input_serial_buffer(int fd, uint8_t cnt, uint8_t *buf)
{
	struct serial_buffer_offset data;
	uint8_t base_ptr;

	base_ptr = chipio_8051_read_exram_at_addr(fd, IN_BUF_BASE_PTR);
	get_serial_buffer_offset(&data, base_ptr, cnt, IN_BUF_LEN);

	chipio_8051_write_exram_data_range(fd, IN_BUF_ADDR + base_ptr,
			data.part1_cnt, buf);
	if (data.part2_cnt) {
		chipio_8051_write_exram_data_range(fd, IN_BUF_ADDR,
				data.part2_cnt,
				&buf[data.part1_cnt]);
	}

	chipio_8051_write_exram_at_addr(fd, IN_BUF_WRITE_PTR, data.write_ptr);
}

static void write_serial_buffer_cmd(int fd, uint8_t *buf)
{
	uint8_t len;

	len = strlen((char *)buf);
	/* Remove new-line, replace with NULL. */
	buf[len - 1] = '\0';

	/*
	 * Set end value to carriage return, which signals the end of a
	 * command.
	 */
	buf[len++] = 0x0d;

	write_input_serial_buffer(fd, len, buf);
}

static void print_serial_buffer_cmd_response(int fd, uint8_t base_ptr,
		uint8_t write_ptr)
{
	struct serial_buffer_offset data;
	uint8_t buf[0x100];
	uint32_t cnt;

	if (write_ptr < base_ptr) {
		cnt = 0x100 - base_ptr;
		cnt += write_ptr;
	} else {
		cnt = write_ptr - base_ptr;
	}

	get_serial_buffer_offset(&data, base_ptr, cnt, OUT_BUF_LEN);
	chipio_8051_read_exram_data_range(fd, OUT_BUF_ADDR + base_ptr, data.part1_cnt, buf);
	if (data.part2_cnt) {
		chipio_8051_read_exram_data_range(fd, OUT_BUF_ADDR, data.part2_cnt,
				&buf[data.part1_cnt]);
	}

	buf[cnt] = '\0';

	printf("Response:\n%s\n\n", buf);
}

const struct timespec timeout_val;
static int check_out_buffer_change(int fd, uint8_t base_ptr)
{
	uint32_t i, ret;

	nanosleep(&timeout_val, NULL);
	for (i = 0; i < 4; i++) {
		ret = chipio_8051_read_exram_at_addr(fd, OUT_BUF_WRITE_PTR);
		if (ret != base_ptr)
			return 0;

		nanosleep(&timeout_val, NULL);
	}

	return 1;

}

static void send_serial_cmd(int fd, uint8_t *buf)
{
	uint8_t out_buf_base_ptr, out_buf_write_ptr;

	out_buf_base_ptr = chipio_8051_read_exram_at_addr(fd, OUT_BUF_BASE_PTR);
	write_serial_buffer_cmd(fd, buf);

	if (check_out_buffer_change(fd, out_buf_base_ptr)) {
		printf("No response.\n");
		return;
	}

	out_buf_write_ptr = chipio_8051_read_exram_at_addr(fd,
			OUT_BUF_WRITE_PTR);
	print_serial_buffer_cmd_response(fd, out_buf_base_ptr,
			out_buf_write_ptr);
}

int main(int argc, char **argv)
{
	char buf[0x100];
	int fd, ret;

	if (argc < 2) {
		usage(argv[0]);
		return 1;
	}

	ret = open_hwdep(argv[1], &fd);
	if (ret)
		return ret;

	while (1) {
		if (!fgets(buf, IN_BUF_LEN - 3, stdin))
			continue;
		send_serial_cmd(fd, (uint8_t *)buf);
	}

	close(fd);

	return 0;
}


