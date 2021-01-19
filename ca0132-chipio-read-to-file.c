/*
 * ca0132-chipio-read-to-file:
 * Reads a range of HCI data into a file.
 */
#include "ca0132_defs.h"

static void usage(char *pname)
{
        fprintf(stderr, "usage: %s <hwdep-device> <start-addr> <end-addr> <file-name>\n", pname);
}

static void read_hic_range_with_progress(int fd, uint32_t start_addr,
		uint32_t cnt, uint32_t *buf)
{
	uint32_t data_left, chunk_size, cur_offset, cur_addr;

	/* Read the range in 8 separate pieces to help track progress. */
	if (cnt < 0x8)
		chunk_size = 1;
	else
		chunk_size = cnt / 8;

	cur_addr = start_addr;
	cur_offset = 0;
	data_left = cnt;

	printf("Reading [");
	fflush(stdout);

	while (data_left) {
		if (data_left < chunk_size) {
			chipio_hic_read_data_range(fd, cur_addr, data_left,
					&buf[cur_offset]);
		} else {
			chipio_hic_read_data_range(fd, cur_addr, chunk_size,
					&buf[cur_offset]);
		}

		putchar('.');
		fflush(stdout);

		cur_offset += chunk_size;
		cur_addr += (chunk_size * 4);

		if (data_left < chunk_size)
			data_left = 0;
		else
			data_left -= chunk_size;
	}

	printf("]\n");
}

int main(int argc, char **argv)
{
	uint32_t data_size, start_addr, end_addr;
	uint32_t *buf;
        int fd, ret;
	FILE *file;

        if (argc < 4) {
                usage(argv[0]);
                return 1;
        }

	ret = open_hwdep(argv[1], &fd);
	if (ret)
		return ret;

	/* Get the number of words to read. */
	start_addr = strtol(argv[2], NULL, 16);
	end_addr   = strtol(argv[3], NULL, 16);

	if (end_addr < start_addr) {
		printf("end_addr less than start_addr.\n");
		close(fd);
		return 1;
	}

	data_size  = (end_addr - start_addr) / 4;

	/* Open file we're going to write to. */
	file = fopen(argv[4], "w+");
	if (!file) {
		fprintf(stderr, "Failed to open file to write to.\n");
		return 1;
	}

	/* Create a buffer to read into. */
	buf = calloc(data_size, sizeof(*buf));

	/* Read into the buffer. */
	read_hic_range_with_progress(fd, start_addr, data_size, buf);

	/* Write the buffer to the file. */
	fwrite(buf, sizeof(*buf), data_size, file);

	fclose(file);
	free(buf);
	close(fd);

	return 0;
}
