/*
 * ca0132-chipio-write-data:
 * Writes multiple data values from a file to the HCI bus.
 */
#include "ca0132_defs.h"

static void usage(char *pname)
{
        fprintf(stderr, "usage: %s <hwdep-device> <start-addr> <file-to-read>\n", pname);
}

static uint32_t get_file_size(FILE *file)
{
	uint32_t tmp;

	fseek(file, 0, SEEK_END);
	tmp = ftell(file);
	rewind(file);

	return tmp;
}

int main(int argc, char **argv)
{
	uint32_t data_size, start_addr, i;
	FILE *data_file;
	uint32_t *buf;
        int fd, ret;

        if (argc < 3) {
                usage(argv[0]);
                return 1;
        }

	ret = open_hwdep(argv[1], &fd);
	if (ret)
		return ret;

	data_file = fopen(argv[3], "r");
	if (!data_file) {
		fprintf(stderr, "Failed to open file to read from.\n");
		return 1;
	}

	/* Get size of the file, and the starting address to write it to. */
	start_addr = strtol(argv[2], NULL, 16);
	data_size = get_file_size(data_file) / 4;

	/* Allocate a buffer to read the file into, double it for readback. */
	buf = calloc(data_size * 2, sizeof(uint32_t));
	if (fread(buf, sizeof(uint32_t), data_size, data_file) != data_size) {
		printf("Failed to read file to write.\n");
		goto exit;
	}

	chipio_hic_write_data_range(fd, start_addr, data_size, buf);
	chipio_hic_read_data_range(fd, start_addr, data_size, &buf[data_size]);

	/* Confirm that the file was written. */
	for (i = 0; i < data_size; i++) {
		if (buf[i] != buf[data_size + i]) {
			printf("Addr 0x%04x: expected 0x%08x, got 0x%08x.\n",
					start_addr + i, buf[i], buf[data_size + i]);
		}
	}

exit:
	free(buf);
	fclose(data_file);
        close(fd);

	return 0;
}
