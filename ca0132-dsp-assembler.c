/*
 * ca0132-dsp-assembler:
 * Assembles a DSP assembly string. With no arguments, takes a string input.
 * With arguments, takes an assembly file to read.
 */
#include "ca0132_defs.h"

static uint32_t get_next_asm_op_str(FILE *asm_file, FILE *tmp, char *buf)
{
	uint32_t ret, i;

	i = ret = 0;
	while (fread(buf + i, sizeof(*buf), 1, asm_file)) {
		if (i && (buf[i - 1] == '.') && (buf[i] == '\n')) {
			i = 0;
			memset(buf, 0, 0x200);
			fwrite(&i, sizeof(i), 1, tmp);
			continue;
		}

		if (buf[i] == ';') {
			ret = 1;
			buf[i + 1] = '\n';
			buf[i + 2] = '\0';
			break;
		}

		i++;
	}

	return ret;
}

int main(int argc, char **argv)
{
	dsp_asm_data data;
	char buf[0x200];
	uint32_t i = 0;
	FILE *asm_file, *data_file;

	if (argc < 1) {
		printf("Usage: %s <file>\n", argv[0]);
		return -1;
	}


	memset(&data, 0, sizeof(data));
	if (argc > 1) {
		asm_file = fopen(argv[1], "r");
		data_file = fopen("test-file.bin", "w+");
		if (!asm_file) {
			printf("Failed to open asm file!\n");
			return 1;
		}

		while (get_next_asm_op_str(asm_file, data_file, buf)) {
			get_asm_data_from_str(&data, buf);
			fwrite(data.opcode, sizeof(data.opcode[0]),
					get_dsp_op_len(data.opcode[0] >> 16), data_file);

			memset(&data, 0, sizeof(data));
			memset(buf, 0, sizeof(buf));
		}

		fclose(data_file);
		return 0;
	}

/*
	print_op_layouts();
*/
	while (scanf("%c", &buf[i])) {
		if (buf[i] == ';') {
			buf[i + 1] = '\0';
			break;
		}

		i++;
	}

	get_asm_data_from_str(&data, buf);

	printf("Op %s: ", data.op.op_str);
	for (i = 0; i < 4; i++)
		printf("0x%08x ", data.opcode[i]);
	putchar('\n');

	return 0;
}
