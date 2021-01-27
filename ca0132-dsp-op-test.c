/*
 * ca0132-dsp-op-test.c:
 * Assembles a register dumping program, and allows for testing individual ops
 * to see their results. Takes a hexadecimal opcode.
 *
 * Once this has been run, you'll need to reset the DSP with a restart or a
 * suspend/resume cycle to get it running again.
 */
#include "ca0132_defs.h"

#define DSP_FUNC_PMEM_DSP_ADDR 0xdf00
#define DSP_FUNC_PMEM_HIC_ADDR (DSP_FUNC_PMEM_DSP_ADDR * 0x4) + 0x80000

struct dsp_op_test_data {
	uint32_t pre_op_func, post_op_func;
	uint32_t op_cnt;

	uint32_t pgm_data[0x400];
	uint32_t pgm_len;

	uint32_t pre_op_data[0x400];
	uint32_t post_op_data[0x400];

	uint32_t post_op_pc;
	char **reg_dump_strs;
	uint32_t reg_dump_str_cnt;
};

static void usage(char *pname)
{
        fprintf(stderr, "usage: %s <hwdep-device>\n", pname);
}

/*
 * Copy these into an array of register dump strings, and then dynamically add
 * whatever other registers we're dumping after these.
 */
static const char *initial_reg_dump_strs[] = {
	"R00", "R08", "R01", "R09",
	"R02", "R10", "R03", "R11",
	"R04", "R12", "R05", "R13",
	"R04_T1", "R12_T1", "R05_T1", "R13_T1",
	"R04_T2", "R12_T2", "R05_T2", "R13_T2",
	"R06", "R14", "R07", "R15",
	"A_R0", "A_R0_MDFR", "A_R1", "A_R1_MDFR",
	"A_R2", "A_R2_MDFR", "A_R3", "A_R3_MDFR",
	"A_R4", "A_R4_MDFR", "A_R5", "A_R5_MDFR",
	"A_R6", "A_R6_MDFR", "A_R7", "A_R7_MDFR",
        "TIME0_PER_ENB", "TIME0_COUNTER", "TIME1_PER_ENB", "TIME1_COUNTER",
        "TIME2_PER_ENB", "TIME2_COUNTER", "TIME3_PER_ENB", "TIME3_COUNTER",
	"@A_R0_X_REG", "@A_R0_Y_REG", "@A_R1_X_REG", "@A_R1_Y_REG",
	"@A_R2_X_REG", "@A_R2_Y_REG", "@A_R3_X_REG", "@A_R3_Y_REG",
	"@A_R4_X_REG", "@A_R4_Y_REG", "@A_R5_X_REG", "@A_R5_Y_REG",
	"@A_R6_X_REG", "@A_R6_Y_REG", "@A_R7_X_REG", "@A_R7_Y_REG",
	"A_R0_BASE", "A_R1_BASE", "A_R2_BASE", "A_R3_BASE",
	"A_R4_BASE", "A_R5_BASE", "A_R6_BASE", "A_R7_BASE",
	"A_R0_LENG", "A_R1_LENG", "A_R2_LENG", "A_R3_LENG",
	"A_R4_LENG", "A_R5_LENG", "A_R6_LENG", "A_R7_LENG",
};

static const char *ret_asm_str = "RET;";

/*
 * Way we'll do this:
 * Use R6 as the stack register. First step to push r00-r03 onto the stack of
 * r7. Then store A_R6, A_R6_MDFR, A_R6_BASE, and A_R6_LEN in r00-r03. Push
 * this onto the stack. Set A_R6 to the dump address we're going to use, set
 * A_R6_MDFR to 1, and A_R6_BASE and A_R6_LEN to 0.
 */
static const char *reg_dump_entry_asm[] = {
	/* Disable interrupts so we don't mess up our routine. */
	"INT_DISABLE;\n",
	/* A_R7 + 1 (-6) */
	"MOV_P @A_R7_X_INC, R10 :\n\
	 MOV_P @A_R7_Y_INC, R11 /\n\
	 NOP;",
	/* A_R7 + 1 (-5) */
	"MOV_P @A_R7_X_INC, R08 :\n\
	 MOV_P @A_R7_Y_INC, R09 /\n\
	 NOP;",
	/* A_R7 + 1 (-4) */
	"MOV_P @A_R7_X_INC, R00 :\n\
	 MOV_P @A_R7_Y_INC, R01 /\n\
	 NOP;",
	/* A_R7 + 1 (-3) */
	"MOV_P @A_R7_X += A_MD7, R02 :\n\
         MOV_P @A_R7_Y += A_MD7, R03 /\n\
         MOV R00, A_R6 :\n\
         MOV R01, A_R6_MDFR;",
	/* A_R7 + 1 (-2) */
	"MOV_P @A_R7_X += A_MD7, R00 :\n\
         MOV_P @A_R7_Y += A_MD7, R01 /\n\
         MOV R02, A_R6_BASE :\n\
         MOV R03, A_R6_LENG;",
	/* A_R7 + 1 (-1) */
	"MOV_P @A_R7_X_INC, R02 :\n\
	 MOV_P @A_R7_Y_INC, R03 /\n\
	 NOP;",
	"MOV A_R6_BASE, CR_0x00000000 :\n\
         MOV A_R6_LENG, CR_0x00000000;",
	"MOVX:2 R00, @A_R7_X - 0x004 :\n\
         MOVX:2 R01, @A_R7_Y - 0x004;",
	"MOVX:2 R02, @A_R7_X - 0x003 :\n\
         MOVX:2 R03, @A_R7_Y - 0x003;",
	"RET;",
};

static const char *reg_dump_pre_op_addr_reg_set[] = {
	"MOV A_R6,      #0x00004800 :\n\
         MOV A_R6_MDFR, #0x00000001;",
};

static const char *reg_dump_post_op_addr_reg_set[] = {
	"MOV A_R6,      #0x00004900 :\n\
         MOV A_R6_MDFR, #0x00000001;",
};

/*
 * Dump R00-R15, including R04/R05/R12/R13's weird extra moves.
 * Also dump timer registers, address registers, indirect address
 * registers, and address base/length registers. These involve setting/
 * restoring A_R6/A_R6_MDFR/A_R6_BASE/A_R6_LENG, so extra care has to be
 * taken.
 */
static const char *reg_dump_func_start_asm[] = {
	"MOV_P @A_R6_X_INC, R00 :\n\
	 MOV_P @A_R6_Y_INC, R08 /\n\
	 NOP;",
	"MOV_P @A_R6_X_INC, R01 :\n\
	 MOV_P @A_R6_Y_INC, R09 /\n\
	 NOP;",
	"MOV_P @A_R6_X_INC, R02 :\n\
	 MOV_P @A_R6_Y_INC, R10 /\n\
	 NOP;",
	"MOV_P @A_R6_X_INC, R03 :\n\
	 MOV_P @A_R6_Y_INC, R11 /\n\
	 NOP;",
	"MOV_P @A_R6_X_INC, R04 :\n\
	 MOV_P @A_R6_Y_INC, R12 /\n\
	 MOV_T2 R09, R12 :\n\
	 MOV_T2 R01, R04;",
	"MOV_P @A_R6_X_INC, R05 :\n\
	 MOV_P @A_R6_Y_INC, R13 /\n\
	 MOV_T2 R11, R13 :\n\
	 MOV_T2 R03, R05;",
	"MOV_T1_P @A_R6_X_INC, R04 :\n\
	 MOV_T1_P @A_R6_Y_INC, R12 /\n\
	 NOP;",
	"MOV_T1_P @A_R6_X_INC, R05 :\n\
	 MOV_T1_P @A_R6_Y_INC, R13 /\n\
	 NOP;",
	"MOV_P @A_R6_X_INC, R01 :\n\
	 MOV_P @A_R6_Y_INC, R09 /\n\
	 NOP;",
	"MOV_P @A_R6_X_INC, R03 :\n\
	 MOV_P @A_R6_Y_INC, R11 /\n\
	 NOP;",
	"MOV_P @A_R6_X_INC, R06 :\n\
	 MOV_P @A_R6_Y_INC, R14 /\n\
	 NOP;",
	"MOV_P @A_R6_X_INC, R07 :\n\
	 MOV_P @A_R6_Y_INC, R15 /\n\
	 NOP;",
	/* Begin address register and address register mdfr dump */
	"MOV R00, A_R0 :\n\
         MOV R08, A_R0_MDFR;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, A_R1 :\n\
         MOV R08, A_R1_MDFR;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, A_R2 :\n\
         MOV R08, A_R2_MDFR;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, A_R3 :\n\
         MOV R08, A_R3_MDFR;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, A_R4 :\n\
         MOV R08, A_R4_MDFR;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, A_R5 :\n\
         MOV R08, A_R5_MDFR;",
	"MOV_P @A_R6_X_INC, R00 :\n\
	 MOV_P @A_R6_Y_INC, R08 /\n\
	 NOP;",
	/* Original A_R6/A_R6_MDFR. */
	"MOVX:2 R00, @A_R7_X - 0x002 :\n\
         MOVX:2 R08, @A_R7_Y - 0x002;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, A_R7 :\n\
         MOV R08, A_R7_MDFR;",
	/* Timer register dump. */
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, TIME0_PER_ENB :\n\
         MOV R08, TIME0_COUNTER;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, TIME1_PER_ENB :\n\
         MOV R08, TIME1_COUNTER;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, TIME2_PER_ENB :\n\
         MOV R08, TIME2_COUNTER;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, TIME3_PER_ENB :\n\
         MOV R08, TIME3_COUNTER;",
	/* Indirect address register value dump. Not really necessary. */
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, @A_R0_X_REG :\n\
         MOV R08, @A_R0_Y_REG;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, @A_R1_X_REG :\n\
         MOV R08, @A_R1_Y_REG;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, @A_R2_X_REG :\n\
         MOV R08, @A_R2_Y_REG;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, @A_R3_X_REG :\n\
         MOV R08, @A_R3_Y_REG;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, @A_R4_X_REG :\n\
         MOV R08, @A_R4_Y_REG;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, @A_R5_X_REG :\n\
         MOV R08, @A_R5_Y_REG;",
	/*
	 * Pull original A_R6, store it into A_R0. Then restore A_R0
	 * afterwards.
	 */
	"MOVX:1 R02, @A_R7_X - 0x02;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R03, A_R0 :\n\
         MOV A_R0, R02;",
        "MOV R00, @A_R0_X_REG :\n\
         MOV R08, @A_R0_Y_REG;",
	"MOV_P @A_R6_X_INC, R00 :\n\
	 MOV_P @A_R6_Y_INC, R08 /\n\
	 MOV A_R0, R03;",
        "MOV R00, @A_R7_X_REG :\n\
         MOV R08, @A_R7_Y_REG;",
	/* Dump A_Rx_BASE/A_Rx_LENG registers. Get original values for A_R6. */
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, A_R0_BASE :\n\
         MOV R08, A_R1_BASE;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, A_R2_BASE :\n\
         MOV R08, A_R3_BASE;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, A_R4_BASE :\n\
         MOV R08, A_R5_BASE;",
	"MOV_P @A_R6_X_INC, R00 :\n\
	 MOV_P @A_R6_Y_INC, R08 /\n\
	 NOP;",
	/* Get original A_R6_BASE. */
	"MOVX:1 R00, @A_R7_X - 0x01;",
        "MOV R08, A_R7_BASE;",
	/* Begin A_Rx_LENG register dump. */
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, A_R0_LENG :\n\
         MOV R08, A_R1_LENG;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, A_R2_LENG :\n\
         MOV R08, A_R3_LENG;",
	"MOV_P @A_R6_X += A_MD6, R00 :\n\
         MOV_P @A_R6_Y += A_MD6, R08 /\n\
         MOV R00, A_R4_LENG :\n\
         MOV R08, A_R5_LENG;",
	"MOV_P @A_R6_X_INC, R00 :\n\
	 MOV_P @A_R6_Y_INC, R08 /\n\
	 NOP;",
	"MOVX:1 R00, @A_R7_Y - 0x01;",
        "MOV R08, A_R7_LENG;",
};

static const char *reg_dump_exit_asm[] = {
	/* Push final values onto the A_R6 stack. */
	"MOV_P @A_R6_X_INC, R00 :\n\
	 MOV_P @A_R6_Y_INC, R08 /\n\
	 NOP;",
	/* Restore A_R6_BASE/LENG register values. */
	"MOVX:2 R00, @A_R7_X - 0x001 :\n\
         MOVX:2 R01, @A_R7_Y - 0x001;",
        "MOV A_R6_BASE, R00 :\n\
         MOV A_R6_LENG, R01;",
	/* Restore A_R6/A_R6_MDFR and registers 0-3/8-11. */
	"MOVX:2 A_R6,      @A_R7_X - 0x002 :\n\
         MOVX:2 A_R6_MDFR, @A_R7_Y - 0x002;",
	"MOVX:2 R02, @A_R7_X - 0x003 :\n\
         MOVX:2 R03, @A_R7_Y - 0x003;",
	"MOVX:2 R00, @A_R7_X - 0x004 :\n\
         MOVX:2 R01, @A_R7_Y - 0x004;",
	"MOVX:2 R08, @A_R7_X - 0x005 :\n\
         MOVX:2 R09, @A_R7_Y - 0x005;",
	"MOVX:2 R10, @A_R7_X - 0x006 :\n\
         MOVX:2 R11, @A_R7_Y - 0x006;",
	/* Restore stack register value back to initial value. */
	"ADD A_R7, A_R7, #-6;",
	"RET;",
};

/*
 * Functions for adding new strings to be used in register comparison.
 */
static void set_reg_dump_str(struct dsp_op_test_data *data, uint32_t idx, char *str)
{
	data->reg_dump_strs[idx] = strdup(str);
}

static void add_reg_dump_strs(struct dsp_op_test_data *data, uint32_t cnt)
{
	if (!data->reg_dump_strs) {
		data->reg_dump_strs = calloc(cnt, sizeof(*data->reg_dump_strs));
		data->reg_dump_str_cnt = cnt;
	} else {
		data->reg_dump_strs = realloc(data->reg_dump_strs,
			(data->reg_dump_str_cnt + cnt) * sizeof(*data->reg_dump_strs));
		memset(&data->reg_dump_strs[data->reg_dump_str_cnt], 0, sizeof(char *));
		data->reg_dump_str_cnt += cnt;
	}
}

/* Functions for creating generic register read assembly code. */
static void create_reg_dump_asm_op(char *buf, uint32_t reg)
{
	sprintf(buf,
"		 MOV_P @A_R6_X += A_MD6, R00 :\n\
		 MOV_P @A_R6_Y += A_MD6, R08 /\n\
		 MOV R00, %s :\n\
		 MOV R08, %s; \n", get_dsp_operand_str(reg), get_dsp_operand_str(reg + 1));
}

static void create_gpram_dump_asm_op(char *buf, uint32_t val)
{
	sprintf(buf,
"		 MOV_P @A_R6_X += A_MD6, R00 :\n\
		 MOV_P @A_R6_Y += A_MD6, R08 /\n\
		 MOV R00, XGPRAM_%03d :\n\
		 MOV R08, YGPRAM_%03d; \n", val, val);
}


/* Assemble an array of strings. */
static void assemble_asm_strs(dsp_asm_data *data, const char **str, uint32_t str_cnt,
		uint32_t *cur_offset, uint32_t *opcodes)
{
	dsp_asm_str_tokens tokens;
	char buf[0x100];
	uint32_t i, len;

	for (i = 0; i < str_cnt; i++) {
		memset(data, 0, sizeof(*data));
		memset(&tokens, 0, sizeof(tokens));
		memset(buf, 0, sizeof(buf));

		strcpy(buf, str[i]);

		get_asm_data_from_str(data, buf);

		len = get_dsp_op_len(data->opcode[0]);
		memcpy(opcodes + (*cur_offset), data->opcode, sizeof(uint32_t) * len);
		*cur_offset += len;
	}
}

static void create_reg_dump_range_ops(dsp_asm_data *data,
		struct dsp_op_test_data *op_test_data,
		uint32_t reg_start, uint32_t reg_cnt,
		uint32_t *cur_offset, uint32_t *opcodes)
{
	uint32_t i, len, str_offset;
	dsp_asm_str_tokens tokens;
	char buf[0x100];

	/* Store strings for register comparison printing. */
	str_offset = op_test_data->reg_dump_str_cnt;
	add_reg_dump_strs(op_test_data, reg_cnt);
	for (i = 0; i < reg_cnt; i++) {
		set_reg_dump_str(op_test_data, str_offset + i,
				(char *)get_dsp_operand_str(reg_start + i));
	}

	for (i = 0; i < reg_cnt; i += 2) {
		memset(data, 0, sizeof(*data));
		memset(&tokens, 0, sizeof(tokens));
		memset(buf, 0, sizeof(buf));

		create_reg_dump_asm_op(buf, reg_start + i);
		get_asm_data_from_str(data, buf);

		len = get_dsp_op_len(data->opcode[0]);
		memcpy(opcodes + (*cur_offset), data->opcode, sizeof(uint32_t) * len);
		*cur_offset += len;
	}
}

static void create_gpram_dump_ops(dsp_asm_data *data,
		struct dsp_op_test_data *op_test_data,
		uint32_t *cur_offset, uint32_t *opcodes)
{
	uint32_t i, len, str_offset;
	dsp_asm_str_tokens tokens;
	char buf[0x100];

	/* Store strings for register comparison printing. */
	str_offset = op_test_data->reg_dump_str_cnt;
	add_reg_dump_strs(op_test_data, 32);
	for (i = 0; i < 16; i++) {
		sprintf(buf, "XGPRAM_%03d", i);
		set_reg_dump_str(op_test_data, str_offset + (i * 2), buf);
		sprintf(buf, "YGPRAM_%03d", i);
		set_reg_dump_str(op_test_data, str_offset + ((i * 2) + 1), buf);
	}

	for (i = 0; i < 16; i++) {
		memset(data, 0, sizeof(*data));
		memset(&tokens, 0, sizeof(tokens));
		memset(buf, 0, sizeof(buf));

		create_gpram_dump_asm_op(buf, i);
		get_asm_data_from_str(data, buf);

		len = get_dsp_op_len(data->opcode[0]);
		memcpy(opcodes + (*cur_offset), data->opcode, sizeof(uint32_t) * len);
		*cur_offset += len;
	}
}

static void create_func_call_op(dsp_asm_data *data, uint32_t func_addr,
		uint32_t *cur_offset, uint32_t *opcodes)
{
	dsp_asm_str_tokens tokens;
	char buf[0x100];
	uint32_t len;

	memset(data, 0, sizeof(*data));
	memset(&tokens, 0, sizeof(tokens));
	memset(buf, 0, sizeof(buf));

	sprintf(buf, "CALL #0x0f, #0x%04x;\n", func_addr);

	get_asm_data_from_str(data, buf);

	len = get_dsp_op_len(data->opcode[0]);
	memcpy(opcodes + (*cur_offset), data->opcode, sizeof(uint32_t) * len);
	*cur_offset += len;
}


static uint32_t create_reg_dump_function(struct dsp_op_test_data *op_test_data,
		uint32_t *opcodes)
{
	uint32_t entry_func_addr, reg_dump_func_addr, exit_func_addr;
	uint32_t pre_op_func_addr, post_op_func_addr;
	uint32_t len, op_cnt, i;
	dsp_asm_data data;

	len = op_cnt = 0;

	/* Entry function. */
	entry_func_addr = DSP_FUNC_PMEM_DSP_ADDR;
	assemble_asm_strs(&data, reg_dump_entry_asm, ARRAY_SIZE(reg_dump_entry_asm),
			&len, opcodes);
	op_cnt += ARRAY_SIZE(reg_dump_entry_asm);

	/* Exit function. */
	exit_func_addr = entry_func_addr + len;
	assemble_asm_strs(&data, reg_dump_exit_asm, ARRAY_SIZE(reg_dump_exit_asm),
			&len, opcodes);
	op_cnt += ARRAY_SIZE(reg_dump_exit_asm);

	/*
	 * Dump initial DSP status registers up to the A_Rx_BASE/LENG
	 * registers.
	 */
	reg_dump_func_addr = entry_func_addr + len;
	assemble_asm_strs(&data, reg_dump_func_start_asm, ARRAY_SIZE(reg_dump_func_start_asm),
			&len, opcodes);
	op_cnt += ARRAY_SIZE(reg_dump_func_start_asm);

	/* Set the initial strings for the register dump comparison function. */
	add_reg_dump_strs(op_test_data, ARRAY_SIZE(initial_reg_dump_strs));
	for (i = 0; i < op_test_data->reg_dump_str_cnt; i++)
		set_reg_dump_str(op_test_data, i, (char *)initial_reg_dump_strs[i]);

	/* Dump DSP cond code reg and stack register etc. */
	create_reg_dump_range_ops(&data, op_test_data, 128, 40, &len, opcodes);
	op_cnt += 20;

	/* Dump from 'SEMAPHORE_G_REG' to the final register. */
	create_reg_dump_range_ops(&data, op_test_data, 184, 70, &len, opcodes);
	op_cnt += 35;

	/* Dump XGPRAM and YGPRAM. */
	create_gpram_dump_ops(&data, op_test_data, &len, opcodes);
	op_cnt += 16;

	/* Return from dump function. */
	assemble_asm_strs(&data, &ret_asm_str, 1, &len, opcodes);
	op_cnt++;

	/*
	 * Need three calls: entry function, reg dump function, exit function,
	 * then return.
	 */
	pre_op_func_addr = entry_func_addr + len;
	create_func_call_op(&data, entry_func_addr, &len, opcodes);
	assemble_asm_strs(&data, reg_dump_pre_op_addr_reg_set,
			ARRAY_SIZE(reg_dump_pre_op_addr_reg_set), &len, opcodes);
	op_cnt += ARRAY_SIZE(reg_dump_pre_op_addr_reg_set);
	create_func_call_op(&data, reg_dump_func_addr, &len, opcodes);
	create_func_call_op(&data, exit_func_addr, &len, opcodes);
	assemble_asm_strs(&data, &ret_asm_str, 1, &len, opcodes);

	/*
	 * Final op_cnt addition, don't do it on the next function because we
	 * don't run both at once, it's only one or the other.
	 */
	op_cnt += 4;

	/* Post opcode run function. */
	post_op_func_addr = entry_func_addr + len;
	create_func_call_op(&data, entry_func_addr, &len, opcodes);
	assemble_asm_strs(&data, reg_dump_post_op_addr_reg_set,
			ARRAY_SIZE(reg_dump_post_op_addr_reg_set), &len, opcodes);
	create_func_call_op(&data, reg_dump_func_addr, &len, opcodes);
	create_func_call_op(&data, exit_func_addr, &len, opcodes);
	assemble_asm_strs(&data, &ret_asm_str, 1, &len, opcodes);

	op_test_data->pre_op_func = entry_func_addr + len;
	create_func_call_op(&data, pre_op_func_addr, &len, opcodes);

	op_test_data->post_op_func = entry_func_addr + len;
	create_func_call_op(&data, post_op_func_addr, &len, opcodes);

	/* Add one extra to cover the initial function call. */
	op_test_data->op_cnt = op_cnt + 1;

	return len;
}

static uint32_t get_test_op_from_str(char *buf, uint32_t *test_op)
{
	uint32_t in_cnt;

	in_cnt = sscanf((const char *)buf, "%x %x %x %x", &test_op[0],
				&test_op[1], &test_op[2], &test_op[3]);

	/* If no valid hex found, return 0. Effectively exits. */
	if (!in_cnt)
		return 0;

	return get_dsp_op_len(test_op[0]);
}

static void read_x_y_ram_dump(int fd, uint32_t addr, uint32_t cnt, uint32_t *data)
{
	uint32_t tmp_x[0x400];
	uint32_t tmp_y[0x400];
	uint32_t reg_cnt, i;

	/* Divide by two, as they're split between X/YRAM. */
	reg_cnt = cnt / 2;

	chipio_hic_read_data_range(fd, addr, reg_cnt, tmp_x);
	chipio_hic_read_data_range(fd, 0x40000 + addr, reg_cnt, tmp_y);

	for (i = 0; i < reg_cnt; i++) {
		data[(i * 2)]     = tmp_x[i];
		data[(i * 2) + 1] = tmp_y[i];
	}
}

static void get_test_op_registers(int fd, struct dsp_op_test_data *data)
{
	uint32_t i;

	memset(data->pre_op_data, 0, sizeof(data->pre_op_data));
	memset(data->post_op_data, 0, sizeof(data->post_op_data));

	/* Get pre-op register dump data. */
	read_x_y_ram_dump(fd, 0x12000, data->reg_dump_str_cnt, data->pre_op_data);

	/* Get post-op registers. */
	read_x_y_ram_dump(fd, 0x12400, data->reg_dump_str_cnt, data->post_op_data);

	printf("\nStart PC 0x%04x, PC after 0x%04x.\n", DSP_FUNC_PMEM_DSP_ADDR + data->pgm_len,
			data->post_op_pc);
	/* Compare registers: */
	for (i = 0; i < data->reg_dump_str_cnt; i++) {
		if (data->pre_op_data[i] != data->post_op_data[i]) {
			printf("reg[%s] diff: prev 0x%08x, curr 0x%08x.\n",
					data->reg_dump_strs[i], data->pre_op_data[i],
					data->post_op_data[i]);
		}
	}
}

int main(int argc, char **argv)
{
	uint32_t i, buf_len, op_len;
	struct dsp_op_test_data data;
	uint32_t test_op[4];
	char buf[0x100];
	int fd, ret;

	if (argc < 2) {
		usage(argv[0]);
		return 1;
	}

	ret = open_hwdep(argv[1], &fd);
	if (ret)
		return ret;

	memset(&data, 0, sizeof(data));

	/* Assemble register dump program functions. */
	data.pgm_len = create_reg_dump_function(&data, data.pgm_data);

	/* Write our assembled program out to the DSP. */
	chipio_hic_write_data_range(fd, DSP_FUNC_PMEM_HIC_ADDR, data.pgm_len, data.pgm_data);

	/* Set the DSP into single step mode. */
	set_dsp_dbg_single_step(fd, 1);

	/* Run pre-op/post-op dump functions to populate memory. */
	dsp_run_steps_at_addr(fd, 0, data.pre_op_func, data.op_cnt);
	dsp_run_steps_at_addr(fd, 0, data.post_op_func, data.op_cnt);

	while (1) {
		memset(buf, 0, sizeof(buf));
		memset(test_op, 0, sizeof(test_op));
		buf_len = 0;

		printf("Enter op to test:\n");
		while (scanf("%c", &buf[buf_len])) {
			if (buf[buf_len] == '\n') {
				buf[buf_len + 1] = '\0';
				break;
			}

			buf_len++;
		}

		op_len = get_test_op_from_str(buf, test_op);
		if (!op_len)
			break;

		/* Write op to test. */
		chipio_hic_write_data_range(fd, DSP_FUNC_PMEM_HIC_ADDR + (data.pgm_len * 4),
				op_len, test_op);

		/* Run pre-op register dump function. */
		dsp_run_steps_at_addr(fd, 0, data.pre_op_func, data.op_cnt);

		/* Run op to test. */
		dsp_run_steps_at_addr(fd, 0, DSP_FUNC_PMEM_DSP_ADDR + data.pgm_len, 1);
		data.post_op_pc = chipio_hic_read_at_addr(fd, 0x100e2c);

		/* Run post-op register dump function. */
		dsp_run_steps_at_addr(fd, 0, data.post_op_func, data.op_cnt);

		/* Pull the register data and compare. */
		get_test_op_registers(fd, &data);
	}

	for (i = 0; i < data.reg_dump_str_cnt; i++)
		free(data.reg_dump_strs[i]);

	free(data.reg_dump_strs);
	close(fd);

	return 0;
}

