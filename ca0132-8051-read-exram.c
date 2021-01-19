/*
 * ca0132-8051-read-exram:
 * Reads a single value at an 8051 exram address.
 */
#include "ca0132_defs.h"

static void usage(char *pname)
{
        fprintf(stderr, "usage: %s <hwdep-device>\n", pname);
}

int main(int argc, char **argv)
{
	uint32_t addr, data;
        int fd, ret;

	if (argc < 2) {
		usage(argv[0]);
		return 1;
	}

	ret = open_hwdep(argv[1], &fd);
	if (ret)
		return ret;

	while (1) {
		printf("Enter address: ");
		if (!scanf("%x", &addr))
			break;

		data = chipio_8051_read_exram_at_addr(fd, addr & 0xffff);
		printf("Addr 0x%04x, val 0x%02x.\n", addr, data);
	}

        close(fd);

        return 0;
}
