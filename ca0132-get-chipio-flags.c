/*
 * ca0132-get-chipio-flags.c:
 * Prints out all ChipIO flags, with their associated strings and where or not
 * they're enabled/disabled.
 */
#include "ca0132_defs.h"

static void usage(char *pname)
{
        fprintf(stderr, "usage: %s <hwdep-device>\n", pname);
}

int main(int argc, char **argv)
{
        int fd, ret;
	uint32_t i;

	if (argc < 2) {
		usage(argv[0]);
		return 1;
	}

	ret = open_hwdep(argv[1], &fd);
	if (ret)
		return ret;

	for (i = 0; i < 0x19; i++) {
		printf("Flag %s, set %d.\n", chipio_get_flag_str(i),
				chipio_get_control_flag(fd, i));
	}

        close(fd);

        return 0;
}

