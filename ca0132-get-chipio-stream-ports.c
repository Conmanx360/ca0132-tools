/*
 * ca0132-get-chipio-stream-ports.c:
 * Prints out all currently active ChipIO streams that are using the audio
 * router at HCI address 0x190000, and prints the associated values.
 */
#include "ca0132_defs.h"

struct stream_ports {
	uint8_t start_port[0x26];
	uint8_t end_port[0x26];
};

static void usage(char *pname)
{
        fprintf(stderr, "usage: %s <hwdep-device>\n", pname);
}

static void get_stream_ports(int fd, struct stream_ports *data)
{
	chipio_8051_read_exram_data_range(fd, 0x1578, 0x26, data->start_port);
	chipio_8051_read_exram_data_range(fd, 0x159d, 0x26, data->end_port);
}

static void print_stream_ports(int fd, struct stream_ports *data, uint32_t stream)
{
	uint32_t cur_addr = 0x190000;
	uint32_t i, cnt;

	cur_addr += (data->start_port[stream] * 0x04);
	cnt = data->end_port[stream] - data->start_port[stream];
	printf("Stream 0x%02x, start 0x%02x, cnt %d.\n", stream,
			data->start_port[stream], cnt + 1);

	for (i = 0; i < (cnt + 1); i++) {
		printf("    port 0x%06x: 0x%08x\n", cur_addr,
				chipio_hic_read_at_addr(fd, cur_addr));
		cur_addr += 0x04;
	}
}

static void check_stream_ports(int fd, struct stream_ports *data)
{
	uint32_t i;

	for (i = 0; i < 0x26; i++) {
		if ((data->start_port[i] == 0xff) || (data->end_port[i] == 0xff))
				continue;

		print_stream_ports(fd, data, i);
	}
}

int main(int argc, char **argv)
{
	struct stream_ports data;
        int fd, ret;

	if (argc < 1) {
		usage(argv[0]);
		return 1;
	}

	ret = open_hwdep(argv[1], &fd);
	if (ret)
		return ret;

	memset(&data, 0, sizeof(data));

	get_stream_ports(fd, &data);
	check_stream_ports(fd, &data);

	close(fd);

        return 0;
}
