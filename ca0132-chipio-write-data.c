/*
 * ca0132-chipio-write-data:
 * Writes a single data value at an HCI address.
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

		printf("Enter data: ");
		if (!scanf("%x", &data))
			break;

		chipio_hic_write_at_addr(fd, addr, data);
		readback = chipio_hic_read_at_addr(fd, addr);

		if (readback != data)
			printf("Addr 0x%06x: expected 0x%08x, got 0x%08x.\n", addr, data, readback);
		else
			printf("Addr 0x%06x: 0x%08x.\n", addr, readback);
	}

        close(fd);

        return 0;
}
