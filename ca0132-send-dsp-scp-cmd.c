/*
 * ca0132-send-dsp-scp-cmd:
 * Sends a single DSP scp command.
 */
#include "ca0132_defs.h"

static void usage(char *pname)
{
        fprintf(stderr, "usage: %s <hwdep-device>\n", pname);
}

int main(int argc, char **argv)
{
	uint32_t src_id, req, size, mid, i, tmp, hdr;
	struct scp_data scp_cmd;
	uint32_t data[0x20];
        int fd, ret;

	if (argc < 2) {
		usage(argv[0]);
		return 1;
	}

	ret = open_hwdep(argv[1], &fd);
	if (ret)
		return ret;

	while (1) {
		printf("Enter src_id:\n");
		if (!scanf("%x", &src_id))
			break;

		printf("Enter mid:\n");
		if (!scanf("%x", &mid))
			break;

		printf("Enter req:\n");
		if (!scanf("%x", &req))
			break;

		printf("Enter data size:\n");
		if (!scanf("%x", &size))
			break;

		if (size > 0x1f) {
			printf("Data size too large, try again.\n");
			continue;
		}

		tmp = 0;
		for (i = 0; i < size; i++) {
			printf("Enter data[%d]:\n", i);
			if (!scanf("%x", &data[i])) {
				tmp = 1;
				break;
			}
		}

		if (tmp)
			break;

		pack_scp_data(&scp_cmd, size, req, src_id, mid);
		hdr = create_scp_header(&scp_cmd);
		if (dspio_write(fd, hdr)) {
			printf("DSP busy, try again.\n");
			continue;
		}

		for (i = 0; i < size; i++) {
			if (dspio_write(fd, data[i])) {
				printf("DSP busy, try again.\n");
				break;
			}
		}
	}

	printf("Invalid data entry, exiting.\n");

        close(fd);

        return 0;
}

