/*
 * ca0132-8051-write-exram:
 * Writes a single value at an address in the 8051's exram.
 */
#include "ca0132_defs.h"

static void usage(char *pname)
{
        fprintf(stderr, "usage: %s <hwdep-device>\n", pname);
}

int main(int argc, char **argv)
{
	uint32_t addr, data, readback;
        int fd, ret;

	if (argc < 1) {
		usage(argv[0]);
		return 1;
	}

	ret = open_hwdep(argv[1], &fd);
	if (ret)
		return ret;

	while (1) {
		printf("Address: ");
		if (!scanf("%x", &addr))
			break;

		printf("Data: ");
		if (!scanf("%x", &data)) {
			printf("Invalid data, try again.\n");
			continue;
		}

		chipio_8051_write_exram_at_addr(fd, addr & 0xffff, data & 0xff);
		readback = chipio_8051_read_exram_at_addr(fd, addr);
		if (readback != data) {
			printf("Data write failed, expected 0x%02x, got 0x%02x.\n",
					data, readback);
		}
	}

        close(fd);

        return 0;
}
