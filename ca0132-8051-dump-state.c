/*
 * ca0132-dump-state:
 * Create a simulator state by dumping the contents of a running ca0132.
 * Then, you can enter the main loop and simulate from there.
 */
#include "ca0132_defs.h"

struct param_val_handler {
	const uint8_t *handler_entry;
	uint32_t entry_size;

	uint8_t func_calls[4];
	uint8_t func_call_cnt;

	const uint8_t *handler_exit;
	uint32_t exit_size;

	uint16_t entry_addr;
};

enum dump_func_id {
	DUMP_FUNC_PMEM,
	DUMP_FUNC_XRAM,
	DUMP_FUNC_IRAM,
	DUMP_FUNC_SIGNAL,
};

enum dump_handler_id {
	DUMP_HANDLER_PMEM_B0,
	DUMP_HANDLER_PMEM_B1,
	DUMP_HANDLER_XRAM_IRAM_SFR,
	DUMP_HANDLER_JMP_TBL,
};

struct ca0132_dump_state_data {
	uint16_t dump_func_addr[4];
	uint16_t entry_addr;
	uint16_t exit_addr;
	uint16_t jmp_tbl_addr;

	uint16_t curr_addr;

	/*
	 * JMP_TBL is always the final enumerated value. This will make it
	 * easier to add new handler_id's if someone wanted to.
	 */
	struct param_val_handler param_handler[DUMP_HANDLER_JMP_TBL];
};

/* Three separate memory areas:
 * Generic functions (0xf100).
 * ParamID value handlers/jump table. (0xf200).
 * Main function entry. (0xf300).
 *
 * The ParamID value handlers are characterized by three separate
 * portions:
 * -Entry.
 * -Function calls.
 * -Exit.
 * -After the return from the ParamID value handler, function call to signal
 *  that the operation is complete, and jump to the exit.
 */

static const uint8_t main_func_entry_addr[2] = { 0xf3, 0x00 };

#define ARR_BLOCK_SIZE            100
#define GEN_FUNC_START            0xf100
#define VAL_HANDLER_START         0xf200
#define MAIN_FUNC_ENTRY           0xf300
#define EXRAM_SIGNAL_ADDR         0xf1ff
#define PARAM_ID_36_HANDLER_ADDR  0x1759
#define DUMP_PARAM_ID             0x24

/*
 * Generic functions:
 * Generic functions start at 0xf100.
 */

/* PMEM dump function.
 * c0 a8    ; push IE
 * 75 a8 00 ; set  IE to #0x00.
 * 90 20 00 ; move dptr   #0x2000
 * 05 86    ; inc  dp_tgl
 * 90 80 00 ; move dptr   #0x8000
 * 78 60    ; move r0     #0x60
 * 79 00    ; move r1     #0x00
 * e4       ; clr A
 * 93       ; movc acc    dptr
 * a3       ; inc  dptr
 * 05 86    ; inc  dp_tgl
 * f0       ; movx dptr   acc
 * a3       ; inc  dptr
 * 05 86    ; inc  dp_tgl
 * d9 f5    ; djnz r1     to movc acc
 * d8 f1    ; djnz r0     to move r1 #0x00
 * 75 86 00 : set  dp_tgl to 0x00
 * d0 a8    ; pop IE.
 * 22       ; ret.
 */
static const uint8_t mem_dump_func0[] = {
	0xc0, 0xa8, 0x75, 0xa8, 0x00, 0x90, 0x20, 0x00,
	0x05, 0x86, 0x90, 0x80, 0x00, 0x78, 0x60, 0x79,
	0x00, 0xe4, 0x93, 0xa3, 0x05, 0x86, 0xf0, 0xa3,
	0x05, 0x86, 0xd9, 0xf5, 0xd8, 0xf1, 0x75, 0x86,
	0x00, 0xd0, 0xa8, 0x22,
};

/* XRAM dump function.
 * c0 a8    ; push IE
 * 75 a8 00 ; set  IE to #0x00.
 * 90 20 00 ; move dptr   #0x2000
 * 05 86    ; inc  dp_tgl
 * 90 00 00 ; move dptr   #0x0000
 * 78 20    ; move r0     #0x20
 * 79 00    ; move r1     #0x00
 * e4       ; clr A
 * e0       ; movx acc    dptr
 * a3       ; inc  dptr
 * 05 86    ; inc  dp_tgl
 * f0       ; movx dptr   acc
 * a3       ; inc  dptr
 * 05 86    ; inc  dp_tgl
 * d9 f5    ; djnz r1     to movc acc
 * d8 f1    ; djnz r0     to move r1 #0x00
 * 75 86 00 : set  dp_tgl to 0x00
 * d0 a8    ; pop IE.
 * 22       ; ret.
 */
static const uint8_t mem_dump_func1[] = {
	0xc0, 0xa8, 0x75, 0xa8, 0x00, 0x90, 0x20, 0x00,
	0x05, 0x86, 0x90, 0x00, 0x00, 0x78, 0x20, 0x79,
	0x00, 0xe4, 0xe0, 0xa3, 0x05, 0x86, 0xf0, 0xa3,
	0x05, 0x86, 0xd9, 0xf5, 0xd8, 0xf1, 0x75, 0x86,
	0x00, 0xd0, 0xa8, 0x22,
};

/* IRAM dump.
 * c0 a8    ; push IE
 * 75 a8 00 ; set  IE to #0x00.
 * 90 40 00 ; move dptr   #0x4000
 * 78 00    ; move r0     #0x00
 * 79 00    ; move r1     #0x00
 * e4       ; clr A
 * e7       ; mov  acc    @r1
 * f0       ; movx dptr   acc
 * 09       ; inc  r1
 * a3       ; inc  dptr
 * d8 fa    ; djnz r0
 * 75 86 00 : set  dp_tgl to 0x00
 * d0 a8    ; pop IE.
 * 22       ; ret.
 */
static const uint8_t mem_dump_func2[] = {
	0xc0, 0xa8, 0x75, 0xa8, 0x00, 0x90, 0x40, 0x00,
	0x78, 0x00, 0x79, 0x00, 0xe4, 0xe7, 0xf0, 0x09,
	0xa3, 0xd8, 0xfa, 0x75, 0x86, 0x00, 0xd0, 0xa8,
	0x22
};

/* Signal that the operation is complete..
 * 90 f1 ff ; move dptr   #0xf1ff
 * 74 ff    ; mov  acc    #0xff;
 * f0       ; movx dptr   acc // Signal that we're done.
 * 22       ; ret.
 */
static const uint8_t mem_dump_func3[] = {
	0x90, 0xf1, 0xff, 0x74, 0xff, 0xf0, 0x22,
};

/*
 * PARAM HANDLER FUNCTIONS:
 * Mem dump functions start at 0xf100.
 * Param handlers start at exram 0xf200.
 */

/*
 * Handler 0.
 * Dump pmem bank 0 entry.
 * c0 fa    ; push 0xfa
 * 75 fa 15 ; move 0xfa #0x15
 */
static const uint8_t mem_dump_handler0_entry[] = {
	0xc0, 0xfa, 0x75, 0xfa, 0x15,
};

/*
 * Dump pmem bank 0 exit.
 * d0 fa    ; pop  0xfa
 */
static const uint8_t mem_dump_handler0_exit[] = {
	0xd0, 0xfa,
};

/*
 * Handler 1.
 * Dump pmem bank 1 entry.
 * c0 fa    ; push 0xfa
 * 75 fa 2a ; move 0xfa #0x2a
 */
static const uint8_t mem_dump_handler1_entry[] = {
	0xc0, 0xfa, 0x75, 0xfa, 0x2a,
};

/*
 * Dump pmem bank 1 exit.
 * d0 fa    ; pop  0xfa
 */
static const uint8_t mem_dump_handler1_exit[] = {
	0xd0, 0xfa,
};

/*
 * Handler 2.
 * No entry. Only exit.
 * Dump exram/iram/sfr's exit. Dump select SFR's.
 * 90 41 00 ; mov  dptr   #0x4100
 * e5 80    ; mov  acc    0x80
 * f0       ; movx dptr   acc
 * a3       ; inc  dptr
 * e5 81    ; mov  acc    0x81
 * f0       ; movx dptr   acc
 * a3       ; inc  dptr
 * e5 90    ; mov  acc    0x90
 * f0       ; movx dptr   acc
 * a3       ; inc  dptr
 * e5 a0    ; mov  acc    0xa0
 * f0       ; movx dptr   acc
 * a3       ; inc  dptr
 * e5 a8    ; mov  acc    0xa8
 * f0       ; movx dptr   acc
 * a3       ; inc  dptr
 * e5 b0    ; mov  acc    0xb0
 * f0       ; movx dptr   acc
 * a3       ; inc  dptr
 * e5 b8    ; mov  acc    0xb8
 * f0       ; movx dptr   acc
 * a3       ; inc  dptr
 * e5 fa    ; mov  acc    0xfa
 * f0       ; movx dptr   acc
 */
static const uint8_t mem_dump_handler2_exit[] = {
	0x90, 0x41, 0x00, 0xe5, 0x80, 0xf0, 0xa3, 0xe5,
	0x81, 0xf0, 0xa3, 0xe5, 0x90, 0xf0, 0xa3, 0xe5,
	0xa0, 0xf0, 0xa3, 0xe5, 0xa8, 0xf0, 0xa3, 0xe5,
	0xb0, 0xf0, 0xa3, 0xe5, 0xb8, 0xf0, 0xa3, 0xe5,
	0xfa, 0xf0,
};

/*
 * MAIN ENTRY/EXIT FUNCTIONS:
 * Starts at 0xf300.
 */

/* Main function entry.
 * c0 e0    ; push ACC
 * c0 f0    ; push B
 * c0 83    ; push DPH0
 * c0 82    ; push DPL0
 * c0 85    ; push DPH1
 * c0 84    ; push DPL1
 * c0 86    ; push DP_TGL
 * 75 86 00 ; mov  DP_TGL #0x00
 * c0 d0    ; push PSW
 * 75 d0 18 ; move PSW    #0x18
 * e5 6f    ; mov  acc    0x6f
 * 94 #0xXX ; subb acc   - We dynamically fill in the literal based in the
 *			   number of handlers.
 */
static const uint8_t main_func_entry_part1[] = {
	0xc0, 0xe0, 0xc0, 0xf0, 0xc0, 0x83, 0xc0, 0x82,
	0xc0, 0x85, 0xc0, 0x84, 0xc0, 0x86, 0x75, 0x86,
	0x00, 0xc0, 0xd0, 0x75, 0xd0, 0x18, 0xe5, 0x6f,
	0x94,
};

/*
 * 50 13    ; jnc  0x0a   - If the value is great than the number of handlers,
 *                          jump to the exit. Otherwise, calculate the jump
 *                          table offset.
 * e5 6f    ; mov  acc    0x6f
 * 75 f0 06 ; mov  0xf0   #0x06
 * a4       ; mul  A      B
 */
static const uint8_t main_func_entry_part2[] = {
	0x50, 0x0a, 0xe5, 0x6f, 0x75, 0xf0, 0x06, 0xa4,
};

/* Main function exit.
 * ASM exit: addr 0xf32f.
 * d0 d0    ; pop  PSW
 * d0 86    ; pop  DP_TGL
 * d0 84    ; pop  DPL1
 * d0 85    ; pop  DPH1
 * d0 82    ; pop  DPL0
 * d0 83    ; pop  DPH0
 * d0 f0    ; pop  B
 * d0 e0    ; pop  ACC
 * 22       ; ret
 */
static const uint8_t main_func_exit[] = {
	0xd0, 0xd0, 0xd0, 0x86, 0xd0, 0x84, 0xd0, 0x85,
	0xd0, 0x82, 0xd0, 0x83, 0xd0, 0xf0, 0xd0, 0xe0,
	0x22,
};

/* Structure from ca0132 8051 simulator. */
struct emu8051_dev {
        uint16_t pc;

        uint8_t pmem[0x8000];
        uint8_t pmem_b0[0x6000];
        uint8_t pmem_b1[0x6000];

        uint8_t xram[0x10000];
        uint8_t iram[0x100];
        uint8_t sfr[0x80];

        struct op_change *op_ch;
};

static void usage(char *pname)
{
        fprintf(stderr, "usage: %s <hwdep-device> <savestate-name>\n", pname);
}

/* Simulator save state creation functions. */
static const char *file_sections[] = {"8051", "PMEM", "PMB0", "PMB1", "XRAM", "IRAM", "SFR ", "BKOP"};
enum file_section_enum {
        FILE_HEADER,
        PMEM_SEC,
        PMEM_B0_SEC,
        PMEM_B1_SEC,
        XRAM_SEC,
        IRAM_SEC,
        SFR_SEC,
        BKOP_SEC,
        NUM_OF_SECS,
};

struct op_change {
        uint16_t pc;

        uint8_t changes;
        uint16_t change_type[6];
        uint16_t change_val[6];
};

struct arr_block {
        struct op_change op[ARR_BLOCK_SIZE];
};

void write_header(FILE *save_file, uint8_t section)
{
        const char *tmp = file_sections[section];

        fwrite(tmp, strlen(tmp), 1, save_file);
}

void write_simulator_save_state(struct emu8051_dev *emu_dev, char *file_name)
{
	FILE *save_file = NULL;
	uint32_t i, tmp;

	save_file = fopen(file_name, "w+");
	if (!save_file)
		return;

        write_header(save_file, FILE_HEADER);
        fwrite(&emu_dev->pc, sizeof(uint16_t), 1, save_file);

        write_header(save_file, PMEM_SEC);
        fwrite(emu_dev->pmem, sizeof(emu_dev->pmem), 1, save_file);

        write_header(save_file, PMEM_B0_SEC);
        fwrite(emu_dev->pmem_b0, sizeof(emu_dev->pmem_b0), 1, save_file);

        write_header(save_file, PMEM_B1_SEC);
        fwrite(emu_dev->pmem_b1, sizeof(emu_dev->pmem_b1), 1, save_file);

        write_header(save_file, XRAM_SEC);
        fwrite(emu_dev->xram, sizeof(emu_dev->xram), 1, save_file);

        write_header(save_file, IRAM_SEC);
        fwrite(emu_dev->iram, sizeof(emu_dev->iram), 1, save_file);

        write_header(save_file, SFR_SEC);
        fwrite(emu_dev->sfr, sizeof(emu_dev->sfr), 1, save_file);

	/* Just write dumby data here so it doesn't break anything. */
	write_header(save_file, BKOP_SEC);

	/* Set current count to 0. */
	tmp = 0;
        fwrite(&tmp, sizeof(uint32_t), 1, save_file);

	/* Starting backops = 1000, always an extra 1000 for the deque. */
	tmp = 11;
        fwrite(&tmp, sizeof(uint32_t), 1, save_file);

	/* Start offset = 0. */
	tmp = 0;
        fwrite(&tmp, sizeof(uint32_t), 1, save_file);
        for (i = 0; i < 11; i++)
                fwrite(&tmp, sizeof(struct arr_block), 1, save_file);

	fclose(save_file);
}

static void write_8051_func_call(int fd, uint16_t start_addr, uint16_t func_addr)
{
	uint8_t data[3];

	data[0] = 0x12;
	data[1] = (func_addr >> 8) & 0xff;
	data[2] = func_addr & 0xff;
	chipio_8051_write_exram_data_range(fd, start_addr, 3, data);
}

static void write_8051_jmp(int fd, uint16_t start_addr, uint16_t jmp_addr)
{
	uint8_t data[3];

	data[0] = 0x02;
	data[1] = (jmp_addr >> 8) & 0xff;
	data[2] = jmp_addr & 0xff;
	chipio_8051_write_exram_data_range(fd, start_addr, 3, data);
}

static void write_8051_dptr_set(int fd, uint16_t start_addr, uint16_t addr)
{
	uint8_t data[3];

	data[0] = 0x90;
	data[1] = (addr >> 8) & 0xff;
	data[2] = addr & 0xff;
	chipio_8051_write_exram_data_range(fd, start_addr, 3, data);
}

static void write_8051_mem_dump_functions(int fd, struct ca0132_dump_state_data *data)
{
	uint16_t offset;

	/* Dump PMEM function. */
	data->dump_func_addr[DUMP_FUNC_PMEM] = offset = GEN_FUNC_START;
	chipio_8051_write_exram_data_range(fd, offset,
			ARRAY_SIZE(mem_dump_func0), mem_dump_func0);
	offset += ARRAY_SIZE(mem_dump_func0);

	/* Dump XRAM function. */
	data->dump_func_addr[DUMP_FUNC_XRAM] = offset;
	chipio_8051_write_exram_data_range(fd, offset,
			ARRAY_SIZE(mem_dump_func1), mem_dump_func1);
	offset += ARRAY_SIZE(mem_dump_func1);

	/* Dump IRAM function. */
	data->dump_func_addr[DUMP_FUNC_IRAM] = offset;
	chipio_8051_write_exram_data_range(fd, offset,
			ARRAY_SIZE(mem_dump_func2), mem_dump_func2);
	offset += ARRAY_SIZE(mem_dump_func2);

	/* Function to signal that the dump handler has been completed. */
	data->dump_func_addr[DUMP_FUNC_SIGNAL] = offset;
	chipio_8051_write_exram_data_range(fd, offset,
			ARRAY_SIZE(mem_dump_func3), mem_dump_func3);
	offset += ARRAY_SIZE(mem_dump_func3);
}

static void write_8051_mem_dump_handlers(int fd, struct ca0132_dump_state_data *data)
{
	struct param_val_handler *tmp;
	uint16_t offset;
	uint32_t i, j;

	/*
	 * Write each handler from the structures we setup in
	 * setup_8051_dump_handler_structs.
	 */
	offset = VAL_HANDLER_START;
	for (i = 0; i < DUMP_HANDLER_JMP_TBL; i++) {
		tmp = &data->param_handler[i];

		tmp->entry_addr = offset;
		/* Remove later. */
		if (tmp->handler_entry) {
			chipio_8051_write_exram_data_range(fd, offset,
					tmp->entry_size, tmp->handler_entry);
			offset += tmp->entry_size;
		}

		for (j = 0; j < tmp->func_call_cnt; j++) {
			write_8051_func_call(fd, offset,
					data->dump_func_addr[tmp->func_calls[j]]);
			offset += 3;
		}

		if (tmp->handler_exit) {
			chipio_8051_write_exram_data_range(fd, offset,
					tmp->exit_size, tmp->handler_exit);
			offset += tmp->exit_size;
		}

		/* Signal that we're done with the handler. */
		write_8051_func_call(fd, offset, data->dump_func_addr[DUMP_FUNC_SIGNAL]);
		offset += 3;

		/* RET. */
		chipio_8051_write_exram_at_addr(fd, offset, 0x22);
		offset++;
	}

	data->jmp_tbl_addr = offset;
}

static void write_8051_handler_jmp_table(int fd, struct ca0132_dump_state_data *data)
{
	uint16_t offset = data->jmp_tbl_addr;
	struct param_val_handler *tmp;
	uint32_t i;

	for (i = 0; i < DUMP_HANDLER_JMP_TBL; i++) {
		tmp = &data->param_handler[i];

		write_8051_func_call(fd, offset,
				tmp->entry_addr);
		offset += 3;

		write_8051_jmp(fd, offset, data->exit_addr);
		offset += 3;
	}
}

static void write_8051_mem_dump_entry_exit(int fd, struct ca0132_dump_state_data *data)
{
	uint16_t offset;

	/*
	 * Main entry. Push registers onto the stack, and jump to the handler
	 * for the selected parameter.
	 */
	data->entry_addr = offset = MAIN_FUNC_ENTRY;
	chipio_8051_write_exram_data_range(fd, offset,
			ARRAY_SIZE(main_func_entry_part1),
			main_func_entry_part1);
	offset += ARRAY_SIZE(main_func_entry_part1);

	/*
	 * Change the value we subb the param by to match how many handlers we
	 * have.
	 */
	chipio_8051_write_exram_at_addr(fd, offset, DUMP_HANDLER_JMP_TBL);
	offset++;

	/* Write part 2, which is the multiplier/jmp if out of range. */
	chipio_8051_write_exram_data_range(fd, offset,
			ARRAY_SIZE(main_func_entry_part2),
			main_func_entry_part2);
	offset += ARRAY_SIZE(main_func_entry_part2);

	write_8051_dptr_set(fd, offset, data->jmp_tbl_addr);
	offset += 3;

	/* JMP A + DPTR. */
	chipio_8051_write_exram_at_addr(fd, offset, 0x73);
	offset++;

	/* Main function exit. Pop registers off the stack and return. */
	data->exit_addr = offset;
	chipio_8051_write_exram_data_range(fd, offset,
			ARRAY_SIZE(main_func_exit),
			main_func_exit);
	offset += ARRAY_SIZE(main_func_exit);
}

static void setup_8051_dump_handler_structs(struct ca0132_dump_state_data *data)
{
	struct param_val_handler *tmp;

	/* Program memory bank 0 dump handler. Value 0. */
	tmp = &data->param_handler[DUMP_HANDLER_PMEM_B0];
	tmp->handler_entry = mem_dump_handler0_entry;
	tmp->entry_size = ARRAY_SIZE(mem_dump_handler0_entry);

	tmp->func_calls[0] = DUMP_FUNC_PMEM;
	tmp->func_call_cnt = 1;

	tmp->handler_exit = mem_dump_handler0_exit;
	tmp->exit_size = ARRAY_SIZE(mem_dump_handler0_exit);

	/* Program memory bank 1 dump handler. Value 1. */
	tmp = &data->param_handler[DUMP_HANDLER_PMEM_B1];
	tmp->handler_entry = mem_dump_handler1_entry;
	tmp->entry_size = ARRAY_SIZE(mem_dump_handler1_entry);

	tmp->func_calls[0] = DUMP_FUNC_PMEM;
	tmp->func_call_cnt = 1;

	tmp->handler_exit = mem_dump_handler1_exit;
	tmp->exit_size = ARRAY_SIZE(mem_dump_handler1_exit);

	/* XRAM/IRAM/SFR dump handler. Value 2. */
	tmp = &data->param_handler[DUMP_HANDLER_XRAM_IRAM_SFR];
	tmp->handler_entry = NULL;
	tmp->entry_size = 0;

	tmp->func_calls[0] = DUMP_FUNC_XRAM;
	tmp->func_calls[1] = DUMP_FUNC_IRAM;
	tmp->func_call_cnt = 2;

	tmp->handler_exit = mem_dump_handler2_exit;
	tmp->exit_size = ARRAY_SIZE(mem_dump_handler2_exit);
}

static void write_8051_exploits(int fd)
{
	struct ca0132_dump_state_data data;

	memset(&data, 0, sizeof(data));

	/* Setup the dump handler structures. */
	setup_8051_dump_handler_structs(&data);

	/* Write the memory dump functions. */
	write_8051_mem_dump_functions(fd, &data);

	/* Write the handlers for each valid param value. */
	write_8051_mem_dump_handlers(fd, &data);

	/* Write the entry and exit portions of the main function. */
	write_8051_mem_dump_entry_exit(fd, &data);

	/* Write the jmp table for the possible param values. */
	write_8051_handler_jmp_table(fd, &data);

	/* Over write the original paramID 36 handler, which does nothing but
	 * jump to a ret instruction. */
	chipio_8051_write_exram_data_range(fd, PARAM_ID_36_HANDLER_ADDR,
			ARRAY_SIZE(main_func_entry_addr),
			main_func_entry_addr);
}

const struct timespec timeout_val;
static int check_dump_status(int fd)
{
	uint32_t i, ret;

	nanosleep(&timeout_val, NULL);
	for (i = 0; i < 4; i++) {
		ret = chipio_8051_read_exram_at_addr(fd, EXRAM_SIGNAL_ADDR);
		if (ret == 0xff) {
			chipio_8051_write_exram_at_addr(fd, EXRAM_SIGNAL_ADDR, 0x00);
			return 0;
		}

		putchar('.');
		fflush(stdout);
		nanosleep(&timeout_val, NULL);
	}

	return -1;
}

static void dump_8051_pmem(struct emu8051_dev *dev, int fd)
{
	uint32_t i;

	printf("Reading pmem_lo [");
	fflush(stdout);
	for (i = 0; i < 0x8; i++) {
		chipio_8051_read_pmem_data_range(fd, i * 0x1000,
			0x1000, &dev->pmem[i * 0x1000]);

		putchar('.');
		fflush(stdout);
	}

	printf("]");
	fflush(stdout);

	/* Program memory B0. */
	chipio_set_control_param(fd, DUMP_PARAM_ID, 0);
	if (check_dump_status(fd) < 0) {
		printf("Failed to get proper dump status!\n");
		return;
	}

	printf("\nReading pmem_b0 [");
	fflush(stdout);

	for (i = 0; i < 0x6; i++) {
		chipio_8051_read_exram_data_range(fd, 0x2000 + (i * 0x1000),
			0x1000, &dev->pmem_b0[i * 0x1000]);

		putchar('.');
		fflush(stdout);
	}

	printf("]");
	fflush(stdout);

	/* Program memory B1. */
	chipio_set_control_param(fd, DUMP_PARAM_ID, 1);
	if (check_dump_status(fd) < 0) {
		printf("Failed to get proper dump status!\n");
		return;
	}

	printf("\nReading pmem_b1 [");
	fflush(stdout);
	for (i = 0; i < 0x6; i++) {
		chipio_8051_read_exram_data_range(fd, 0x2000 + (i * 0x1000),
			0x1000, &dev->pmem_b1[i * 0x1000]);

		putchar('.');
		fflush(stdout);
	}

	printf("]\n");
	fflush(stdout);
}

/* SFR's to pull from exram 0x4100. */
static const uint8_t sfr_addrs[8] = { 0x80, 0x81, 0x90, 0xa0,
				      0xa8, 0xb0, 0xb8, 0xfa };
/* SFR's to pull off the stack. */
static const uint8_t sfr_stack[8] = { 0xd0, 0x86, 0x84, 0x85,
				      0x82, 0x83, 0xf0, 0xe0 };

static void read_8051_ram_and_registers(struct emu8051_dev *dev, int fd)
{
	uint32_t i, stack_ptr;
	uint8_t tmp[8];

	/* Dump exram/iram/sfrs. */

	printf("Reading xram_lo [");
	fflush(stdout);

	chipio_set_control_param(fd, DUMP_PARAM_ID, 2);
	if (check_dump_status(fd) < 0) {
		printf("Failed to get proper dump status!\n");
		return;
	}

	for (i = 0; i < 0x4; i++) {
		chipio_8051_read_exram_data_range(fd, 0x2000 + (i * 0x800),
			0x800, &dev->xram[i * 0x800]);

		putchar('.');
		fflush(stdout);
	}

	printf("]\nReading xram_hi [");
	fflush(stdout);

	for (i = 0; i < 0x4; i++) {
		chipio_8051_read_exram_data_range(fd, 0xe000 + (i * 0x800),
			0x800, &dev->xram[0xe000 + (i * 0x800)]);

		putchar('.');
		fflush(stdout);
	}

	printf("]\n");
	fflush(stdout);

	chipio_8051_read_exram_data_range(fd, 0x4000, 0x100, dev->iram);

	printf("Read iram.\n");
	fflush(stdout);

	chipio_8051_read_exram_data_range(fd, 0x4100, 0x08, tmp);
	for (i = 0; i < 0x08; i++)
		dev->sfr[sfr_addrs[i] - 0x80] = tmp[i];

	/* Get stack pointer. */
	stack_ptr = dev->sfr[0x01] - 2;
	for (i = 0; i < 0x08; i++) {
		dev->sfr[sfr_stack[i] - 0x80] = dev->iram[stack_ptr - i];
	}

	printf("Read sfrs.\n");
	fflush(stdout);

	/* Decrement stack pointer by 12. */
	dev->sfr[0x01] -= 0x0c;

	/* Set PC to 0x7a99. */
	dev->pc = 0x7a99;
}

int main(int argc, char **argv)
{
	struct emu8051_dev dev;
        int fd;

	if (argc < 3) {
		usage(argv[0]);
		return 1;
	}

	open_hwdep(argv[1], &fd);

	memset(&dev, 0, sizeof(dev));

	/* Install exploit to dump the internal memory. */
	write_8051_exploits(fd);

	/* Run the exploits. */
	dump_8051_pmem(&dev, fd);
	read_8051_ram_and_registers(&dev, fd);

	/* Create save state file. */
	write_simulator_save_state(&dev, argv[2]);

	close(fd);

        return 0;
}
