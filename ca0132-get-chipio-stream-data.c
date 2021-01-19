/*
 * ca0132-get-chipio-stream-data.c:
 * Prints out all chipio streams and their associated values.
 */
#include "ca0132_defs.h"

struct stream_ports {
	uint8_t start_port[0x26];
	uint8_t end_port[0x26];
};

struct chipio_stream_data {
	uint8_t source_connid;
	uint8_t channels;
	uint8_t dest_connid;
	uint8_t type;
	uint8_t hda_node;
	uint8_t active;
	uint8_t hda_streamid;
	uint16_t hda_stream_format;
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

static void get_stream_data(int fd, struct chipio_stream_data *stream,
	       struct stream_ports *ports)
{
	uint32_t i, offset, tmp;
	uint8_t buf[0x17c];

	/* Buffer to store the streamID table. */
	memset(buf, 0, sizeof(buf));
	chipio_8051_read_exram_data_range(fd, 0x72f, 0x26 * 0x0a, buf);
	for (i = 0; i < 0x26; i++) {
		offset = i * 0x0a;

		stream[i].source_connid = buf[offset + 1];
		stream[i].channels      = buf[offset + 2];
		stream[i].dest_connid   = buf[offset + 3];
		stream[i].type          = buf[offset + 4];
		stream[i].active        = buf[offset + 5];
		stream[i].hda_node      = buf[offset + 6];
		stream[i].hda_streamid  = buf[offset + 7];

		/* HDA stream format is a 16-bit value. */
		tmp = buf[offset + 8];
		tmp |= buf[offset + 9] << 8;
		stream[i].hda_stream_format = tmp;
	}

	get_stream_ports(fd, ports);
}

static void print_stream_data(struct chipio_stream_data *stream,
	       struct stream_ports *ports)
{
	struct chipio_stream_data *tmp;
	uint32_t i;

	for (i = 0; i < 0x26; i++) {
		tmp = &stream[i];
		if ((ports->start_port[i] != 0xff) && (ports->end_port[i] != 0xff)) {
			printf("Stream 0x%02x, start_port 0x%02x, end_port 0x%02x:\n", i,
					ports->start_port[i], ports->end_port[i]);
		} else {
			printf("Stream 0x%02x:\n", i);
		}

		printf("    SourceConnID:  0x%02x\n", tmp->source_connid);
		printf("    Channels:      0x%02x\n", tmp->channels);
		printf("    DestConnID:    0x%02x\n", tmp->dest_connid);
		printf("    Type:          0x%02x\n", tmp->type);
		printf("    HDA Node:      0x%02x\n", tmp->hda_node);
		printf("    Active:        0x%02x\n", tmp->active);
		printf("    HDA StreamID:  0x%02x\n", tmp->hda_streamid);
		printf("    HDA StreamFmt: 0x%04x\n", tmp->hda_stream_format);
		putchar('\n');
	}
}

int main(int argc, char **argv)
{
	struct chipio_stream_data stream[0x26];
	struct stream_ports ports;
  	int fd, ret;

	if (argc < 1) {
		usage(argv[0]);
		return 1;
	}

	ret = open_hwdep(argv[1], &fd);
	if (ret)
		return ret;

	memset(&ports, 0, sizeof(ports));
	memset(stream, 0, sizeof(stream));

	get_stream_data(fd, stream, &ports);
	print_stream_data(stream, &ports);

	close(fd);

        return 0;
}
