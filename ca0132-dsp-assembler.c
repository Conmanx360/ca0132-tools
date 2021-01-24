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
			buf[i + 1] = '\0';
			break;
		}

		i++;
	}

	return ret;
}

static int read_assembly_from_file(char *asm_file_name, char *data_file_name)
{
	FILE *asm_file, *data_file;
	dsp_asm_data data;
	char buf[0x200];
	int ret = 0;

	asm_file = fopen(asm_file_name, "r");
	if (!asm_file) {
		printf("Failed to open asm file!\n");
		return 1;
	}

	data_file = fopen(data_file_name, "w+");
	if (!data_file) {
		fclose(asm_file);
		printf("Failed to open data file!\n");
		return 1;
	}

	memset(&data, 0, sizeof(data));
	memset(buf, 0, sizeof(buf));
	while (get_next_asm_op_str(asm_file, data_file, buf)) {
		if (!get_asm_data_from_str(&data, buf)) {
			printf("Invalid asm op: %s.\n", buf);
			ret = 1;
			goto exit;
		}

		fwrite(data.opcode, sizeof(data.opcode[0]),
				get_dsp_op_len(data.opcode[0]), data_file);

		memset(&data, 0, sizeof(data));
		memset(buf, 0, sizeof(buf));
	}

exit:
	fclose(data_file);
	fclose(asm_file);

	return ret;
}

static int read_assembly_from_terminal()
{
	dsp_asm_data data;
	char buf[0x200];
	uint32_t i = 0;

	memset(&data, 0, sizeof(data));
	memset(buf, 0, sizeof(buf));

	while (scanf("%c", &buf[i])) {
		if (buf[i] == ';') {
			buf[i + 1] = '\0';
			break;
		}

		i++;
	}

	if (!get_asm_data_from_str(&data, buf)) {
		printf("Failed to get asm data.\n");
		return 1;
	}

	printf("Op %s: ", data.op.op_str);
	for (i = 0; i < 4; i++)
		printf("0x%08x ", data.opcode[i]);

	putchar('\n');

	return 0;
}

int main(int argc, char **argv)
{
	int ret;

	ret = 0;
	if ((argc > 1) && (argc < 3)) {
		printf("Usage: %s <asm_file> <output_file>\n", argv[0]);
		printf("Or, no arguments, and enter the assembly string into the terminal.\n");
		goto exit;
	}

	if (argc > 2)
		ret = read_assembly_from_file(argv[1], argv[2]);
	else
		ret = read_assembly_from_terminal();

exit:

	return ret;
}
