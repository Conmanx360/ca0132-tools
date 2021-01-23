/*
 * ca0132_dsp_functions.c:
 * Contains basic functions for DSP program assembly/disassembly, and
 * definitions for DSP opcode values.
 */
#include "ca0132_defs.h"

static const char *dsp_reg_str[] =
{
				     /* Main registers, or 'local_hw_reg'. */
				     "R00", "R01", "R02", "R03",
                                     "R04", "R05", "R06", "R07",
                                     "R08", "R09", "R10", "R11",
	                             "R12", "R13", "R14", "R15",
				     /*
				      * Address registers.
				      */
				     "A_R0", "A_R1", "A_R2", "A_R3",
                                     "A_R4", "A_R5", "A_R6", "A_R7",
				     "A_R0_MDFR", "A_R1_MDFR", "A_R2_MDFR", "A_R3_MDFR",
				     "A_R4_MDFR", "A_R5_MDFR", "A_R6_MDFR", "A_R7_MDFR",
				     /*
				      * Constant value registers. Some of
				      * these are shared with the emu10k1's
				      * constant registers.
				      */
				     "CR_0x00000000", "CR_0x00000001", "CR_0x00000002", "CR_0x00000003",
				     "CR_0x00000004", "CR_0x00000008", "CR_0x00000010", "CR_0x00000020",
				     "CR_0x00000100", "CR_0x00010000", "CR_0x00000800", "CR_0x10000000",
				     "CR_0x20000000", "CR_F_2",        "CR_0x80000000", "CR_0x7fffffff",
				     "CR_0xffffffff", "CR_0xfffffffe", "CR_0xc0000000", "CR_0x4f1bbcdc",
				     "CR_0x5a7ef9db", "CR_0x00100000", "CR_0x2d413ccd", "CR_0x80000001",
				     "CR_0x08000000", "CR_0x02000000", "CR_0x00800000", "CR_0x00200000",
				     "CR_0x00080000", "CR_0x0000001f", "CR_0x0000000f", "CR_0x00000007",
				     /*
				      * These are floating point based
				      * constants.
				      */
				     "CR_0x00000006", "CR_0x00000005", "CR_F_PI", "CR_F_PI_DIV_2",
				     "CR_F_PI_DIV_4", "CR_F_1_DIV_PI", "CR_F_1_DIV_2PI", "CR_F_0.5",
				     "CR_F_1", "CR_F_NEG_1", "CR_F_3", "CR_F_SQRT_0.5",
				     "CR_B1_12", "CR_B1_13", "CR_B1_14", "CR_B1_15",
				     "CR_B1_16", "CR_B1_17", "CR_B1_18", "CR_B1_19",
				     "CR_B1_20", "CR_B1_21", "CR_B1_22", "CR_B1_23",
				     "TIME0_PER_ENB", "TIME0_COUNTER", "TIME1_PER_ENB", "TIME1_COUNTER",
				     "TIME2_PER_ENB", "TIME2_COUNTER", "TIME3_PER_ENB", "TIME3_COUNTER",
				     /*
				      * Indirect address register r/w
				      * registers.
				      */
				     "@A_R0_X_REG", "@A_R0_Y_REG", "@A_R1_X_REG", "@A_R1_Y_REG",
				     "@A_R2_X_REG", "@A_R2_Y_REG", "@A_R3_X_REG", "@A_R3_Y_REG",
				     "@A_R4_X_REG", "@A_R4_Y_REG", "@A_R5_X_REG", "@A_R5_Y_REG",
				     "@A_R6_X_REG", "@A_R6_Y_REG", "@A_R7_X_REG", "@A_R7_Y_REG",
				     "@A_R0_X_INC_REG", "@A_R0_Y_INC_REG", "@A_R1_X_INC_REG", "@A_R1_Y_INC_REG",
				     "@A_R2_X_INC_REG", "@A_R2_Y_INC_REG", "@A_R3_X_INC_REG", "@A_R3_Y_INC_REG",
				     "@A_R4_X_INC_REG", "@A_R4_Y_INC_REG", "@A_R5_X_INC_REG", "@A_R5_Y_INC_REG",
				     "@A_R6_X_INC_REG", "@A_R6_Y_INC_REG", "@A_R7_X_INC_REG", "@A_R7_Y_INC_REG",
				     /*
				      * First bank of 32 DSP status related
				      * registers.
				      * 128.
				      */
				     "COND_REG", "STACK_FLAG_REG", "PC_STK_PTR", "SR_B0_04_UNK",
				     "CUR_LOOP_ADR_REG", "CUR_LOOP_CNT_REG", "TOP_LOOP_CNT_ST_REG", "TOP_LOOP_COUNT_ADR_REG",
				     "LOOP_STACK_PTR", "STA_S_STACK_DATA", "STA_S_STACK_PTR", "PROG_CNT_REG",
				     "SR_B0_12_UNK", "SR_B0_13_UNK", "SR_B0_14_UNK", "SR_B0_15_UNK",
				     "SR_B0_16_UNK", "SR_B0_17_UNK", "SR_B0_18_UNK", "SR_B0_19_UNK",
				     "SR_B0_20_UNK", "SR_B0_21_UNK", "SR_B0_22_UNK", "SR_B0_23_UNK",
				     "SR_B0_24_UNK", "SR_B0_25_UNK", "SR_B0_26_UNK", "SR_B0_27_UNK",
				     "SR_B0_28_UNK", "SR_B0_29_UNK", "SR_B0_30_UNK", "SR_B0_31_UNK",
				     /*
				      * Bank 1. Includes xyram_ag_index/gpio
				      * registers.
				      */
				     "SR_B1_00_UNK", "SR_B1_01_UNK", "SR_B1_02_UNK", "SR_B1_03_UNK",
				     "SR_B1_04_UNK", "SR_B1_05_UNK", "SR_B1_06_UNK", "SR_B1_07_UNK",
				     "A_R0_BASE", "A_R1_BASE", "A_R2_BASE", "A_R3_BASE",
				     "A_R4_BASE", "A_R5_BASE", "A_R6_BASE", "A_R7_BASE",
				     "A_R0_LENG", "A_R1_LENG", "A_R2_LENG", "A_R3_LENG",
				     "A_R4_LENG", "A_R5_LENG", "A_R6_LENG", "A_R7_LENG",
				     "SEMAPHORE_G_REG", "INT_CONT_MASK_REG", "INT_CONT_PEND_REG", "INT_CONT_SERV_REG",
				     "SR_B1_28_UNK", "SR_B1_29_UNK", "SR_B1_30_UNK", "GPIO_REG",
				     /*
				      * Bank 2. DSP DMA Registers. On ca0132
				      * these are mapped to HIC addresses
				      * 0x110000-0x111000.
				      */
				     "DMACFG_0_REG", "DMA_ADR_OFS_0_REG", "DMA_XFR_CNT_0_REG", "DMA_IRQ_CNT_0_REG",
				     "DMACFG_1_REG", "DMA_ADR_OFS_1_REG", "DMA_XFR_CNT_1_REG", "DMA_IRQ_CNT_1_REG",
				     "DMACFG_2_REG", "DMA_ADR_OFS_2_REG", "DMA_XFR_CNT_2_REG", "DMA_IRQ_CNT_2_REG",
				     "DMACFG_3_REG", "DMA_ADR_OFS_3_REG", "DMA_XFR_CNT_3_REG", "DMA_IRQ_CNT_3_REG",
				     "DMACFG_4_REG", "DMA_ADR_OFS_4_REG", "DMA_XFR_CNT_4_REG", "DMA_IRQ_CNT_4_REG",
				     "DMACFG_5_REG", "DMA_ADR_OFS_5_REG", "DMA_XFR_CNT_5_REG", "DMA_IRQ_CNT_5_REG",
				     "DMACFG_6_REG", "DMA_ADR_OFS_6_REG", "DMA_XFR_CNT_6_REG", "DMA_IRQ_CNT_6_REG",
				     "DMACFG_7_REG", "DMA_ADR_OFS_7_REG", "DMA_XFR_CNT_7_REG", "DMA_IRQ_CNT_7_REG",
				     /*
				      * Bank 3. Continuation of DSP DMA
				      * registers.
				      */
				     "DMACFG_8_REG", "DMA_ADR_OFS_8_REG", "DMA_XFR_CNT_8_REG", "DMA_IRQ_CNT_8_REG",
				     "DMACFG_9_REG", "DMA_ADR_OFS_9_REG", "DMA_XFR_CNT_9_REG", "DMA_IRQ_CNT_9_REG",
				     "DMACFG_A_REG", "DMA_ADR_OFS_A_REG", "DMA_XFR_CNT_A_REG", "DMA_IRQ_CNT_A_REG",
				     "DMACFG_B_REG", "DMA_ADR_OFS_B_REG", "DMA_XFR_CNT_B_REG", "DMA_IRQ_CNT_B_REG",
				     "DMACFG_CH_SEL_0_REG", "DMACFG_CH_SEL_1_REG", "DMACFG_CH_SEL_2_REG", "DMACFG_CH_SEL_3_REG",
				     "DMACFG_CH_SEL_4_REG", "DMACFG_CH_SEL_5_REG", "DMACFG_CH_SEL_6_REG", "DMACFG_CH_SEL_7_REG",
				     "DMACFG_CH_SEL_8_REG", "DMACFG_CH_SEL_9_REG", "DMACFG_CH_SEL_A_REG", "DMACFG_CH_SEL_B_REG",
				     "DMACFG_CH_START_REG", "DMACFG_CH_STAT_REG", "DMACFG_CH_PROP_REG", "DMACFG_ACTIVE_REG",
};

const char *get_dsp_operand_str(uint32_t reg_val)
{
	return dsp_reg_str[reg_val];
}

/*
 * Check the DSP register strings for a match, and if one is found,
 * return the value.
 */
uint32_t get_dsp_operand_str_val(char *operand, uint32_t *val)
{
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(dsp_reg_str); i++) {

		if (!strcmp(dsp_reg_str[i], operand)) {
			*val = i;
			return 1;
		}
	}

	return 0;
}

static const op_operand_layout p_operand_layouts[] = {
	/*
	 * P_OP_LAYOUT_MOV_2:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 23,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 17,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * P_OP_LAYOUT_MOVX_2:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 16,
		       .part1_bits      = 1 },
	       .layout_val  = 0x00,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 17,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 15,
		     .part1_bits      = 1,
		     .part2_bit_start = 21,
		     .part2_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 16,
		       .part1_bits      = 1 },
	       .layout_val  = 0x1,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 17,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 15,
		     .part1_bits      = 1,
		     .part2_bit_start = 21,
		     .part2_bits      = 6,
		     .operand_type    = OP_OPERAND_A_REG_PLUS_MDFR,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 2, },
	/*
	 * P_OP_LAYOUT_MOVX_DUAL_READ_2:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 16,
		       .part1_bits      = 1 },
	       .layout_val  = 0x00,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 20,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 13,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_X,
		     .operand_dir     = OPERAND_DIR_SRC,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 24,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 17,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_Y,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 16,
		       .part1_bits      = 1 },
	       .layout_val  = 0x1,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 20,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 13,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_X_INC,
		     .operand_dir     = OPERAND_DIR_SRC,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 24,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 17,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_Y_INC,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 2, },
	/*
	 * P_OP_LAYOUT_MOVX_DUAL_WRITE_2:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 24,
		       .part1_bits      = 1 },
	       .layout_val  = 0x00,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 21,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_X,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 12,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_SRC,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 25,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_Y,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 16,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 24,
		       .part1_bits      = 1 },
	       .layout_val  = 0x1,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 21,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_X_INC,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 12,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_SRC,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 25,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_Y_INC,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 16,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 2, },
	/*
	 * P_OP_LAYOUT_EXECUTE_COND_2:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 16,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 1,
	       .operand_loc = {
		   { .part1_bit_start = 20,
		     .part1_bits      = 8,
		     .operand_type    = OP_OPERAND_LITERAL_8,
		     .operand_dir     = OPERAND_DIR_NONE, } },
	       .supports_opt_args = 0,
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * P_OP_LAYOUT_MOV_4:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 24,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 18,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * P_OP_LAYOUT_MOV_DUAL_4:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 26,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 16,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_SRC,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 32,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 20,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * P_OP_LAYOUT_MOVX_4:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 17,
		       .part1_bits      = 1 },
	       .layout_val  = 0x00,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 18,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 16,
		     .part1_bits      = 1,
		     .part2_bit_start = 22,
		     .part2_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 17,
		       .part1_bits      = 1 },
	       .layout_val  = 0x1,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 18,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 16,
		     .part1_bits      = 1,
		     .part2_bit_start = 22,
		     .part2_bits      = 6,
		     .operand_type    = OP_OPERAND_A_REG_PLUS_MDFR,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 2, },
	/*
	 * P_OP_LAYOUT_MOVX_DUAL_4:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 15,
		       .part1_bits      = 1 },
	       .layout_val  = 0x00,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 16,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 24,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_X,
		     .operand_dir     = OPERAND_DIR_SRC,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 20,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 30,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_Y,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 15,
		       .part1_bits      = 1 },
	       .layout_val  = 0x1,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 16,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 24,
		     .part1_bits      = 6,
		     .operand_type    = OP_OPERAND_A_REG_X_PLUS_MDFR,
		     .operand_dir     = OPERAND_DIR_SRC,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 20,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 30,
		     .part1_bits      = 6,
		     .operand_type    = OP_OPERAND_A_REG_Y_PLUS_MDFR,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 2, },
	/*
	 * P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 15,
		       .part1_bits      = 1 },
	       .layout_val  = 0x00,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 16,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 24,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_X,
		     .operand_dir     = OPERAND_DIR_SRC,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 30,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_Y,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 20,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 15,
		       .part1_bits      = 1 },
	       .layout_val  = 0x1,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 16,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 24,
		     .part1_bits      = 6,
		     .operand_type    = OP_OPERAND_A_REG_X_PLUS_MDFR,
		     .operand_dir     = OPERAND_DIR_SRC,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 30,
		     .part1_bits      = 6,
		     .operand_type    = OP_OPERAND_A_REG_Y_PLUS_MDFR,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 20,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 2, },
	/*
	 * P_OP_LAYOUT_EXECUTE_COND_4:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 16,
		       .part1_bits      = 2, },
	       .layout_val  = 0x02,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 28,
		     .part1_bits      = 8,
		     .operand_type    = OP_OPERAND_LITERAL_8,
		     .operand_dir     = OPERAND_DIR_NONE,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 20,
		     .part1_bits      = 8,
		     .operand_type    = OP_OPERAND_LITERAL_8,
		     .operand_dir     = OPERAND_DIR_NONE, } },
	       .supports_opt_args = 1,
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
};

const op_operand_layout *get_p_op_layout(uint32_t layout_id)
{
	if (layout_id < ARRAY_SIZE(p_operand_layouts))
		return &p_operand_layouts[layout_id];
	else
		return NULL;
}

static const dsp_op_info parallel_2_asm_ops[] = {
	{ .op_str = "MOV_P",
	  .op = 0x10,
	  .layout_id = P_OP_LAYOUT_MOVX_DUAL_READ_2,
	  .mdfr_bit = 12,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	  .alt_op_str = "MOV_T1_P",
	},
	{ .op_str = "MOV_T1_P",
	  .op = 0x20,
	  .layout_id = P_OP_LAYOUT_MOVX_DUAL_WRITE_2,
	  .mdfr_bit = 20,
	  .alt_op_str = "MOV_P",
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "MOV_T1_P",
	  .op = 0x30,
	  .layout_id = P_OP_LAYOUT_MOV_2,
	  .mdfr_bit = 16,
	  .alt_op_str = "MOV_P",
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "MOV_P",
	  .op = 0x32,
	  .layout_id = P_OP_LAYOUT_MOVX_2,
	},
	{ .op_str = "MOV_P",
	  .op = 0x33,
	  .layout_id = P_OP_LAYOUT_MOVX_2,
	},
	{ .op_str = "MOV_T1_P",
	  .op = 0x34,
	  .layout_id = P_OP_LAYOUT_MOVX_2,
	},
	{ .op_str = "MOV_T1_P",
	  .op = 0x35,
	  .layout_id = P_OP_LAYOUT_MOVX_2,
	},
	{ .op_str = "MOV_P",
	  .op = 0x3a,
	  .layout_id = P_OP_LAYOUT_MOVX_2,
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOV_P",
	  .op = 0x3b,
	  .layout_id = P_OP_LAYOUT_MOVX_2,
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOV_T1_P",
	  .op = 0x3c,
	  .layout_id = P_OP_LAYOUT_MOVX_2,
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOV_T1_P",
	  .op = 0x3d,
	  .layout_id = P_OP_LAYOUT_MOVX_2,
	  .src_dst_swap = 1,
	},
	{ .op_str = "EXEC_COND_P",
	  .op = 0x3f,
	  .layout_id = P_OP_LAYOUT_EXECUTE_COND_2,
	},
};

static const dsp_op_info parallel_4_asm_ops[] = {
	{ .op_str = "MOV_P-20",
	  .op = 0x20,
	  .layout_id = P_OP_LAYOUT_MOV_DUAL_4,
	  .src_dst_swap = 0,
	},
	/* 0x00c9 move behavior. */
	{ .op_str = "MOV_T1_P",
	  .op = 0x24,
	  .layout_id = P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4,
	  .src_dst_swap = 0,
	},
	{ .op_str = "MOV_T1_P",
	  .op = 0x25,
	  .layout_id = P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4,
	  .src_dst_swap = 0,
	},
	/* 0x00c8 move behavior. */
	{ .op_str = "MOV_P",
	  .op = 0x26,
	  .layout_id = P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4,
	  .src_dst_swap = 0,
	},
	{ .op_str = "MOV_P",
	  .op = 0x27,
	  .layout_id = P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4,
	  .src_dst_swap = 0,
	},
	/* No increment. */
	{ .op_str = "MOV_T1_P",
	  .op = 0x28,
	  .layout_id = P_OP_LAYOUT_MOVX_DUAL_4,
	  .src_dst_swap = 0,
	},
	/* Increment, but 0x00c9 type reg read. */
	{ .op_str = "MOV_T1_P",
	  .op = 0x29,
	  .layout_id = P_OP_LAYOUT_MOVX_DUAL_4,
	  .src_dst_swap = 0,
	},
	/* Increment, with regular 0x00c8 reg read. */
	{ .op_str = "MOV_P",
	  .op = 0x2b,
	  .layout_id = P_OP_LAYOUT_MOVX_DUAL_4,
	  .src_dst_swap = 0,
	},
	/* 0x00c9 move behavior. */
	{ .op_str = "MOV_T1_P",
	  .op = 0x2c,
	  .layout_id = P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4,
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOV_T1_P",
	  .op = 0x2d,
	  .layout_id = P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4,
	  .src_dst_swap = 1,
	},
	/* 0x00c8 move behavior. */
	{ .op_str = "MOV_P",
	  .op = 0x2e,
	  .layout_id = P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4,
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOV_P",
	  .op = 0x2f,
	  .layout_id = P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4,
	  .src_dst_swap = 1,
	},
	/* 0x00c9 type read, with no inc and increment. */
	{ .op_str = "MOV_T1_P",
	  .op = 0x30,
	  .layout_id = P_OP_LAYOUT_MOVX_DUAL_4,
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOV_T1_P",
	  .op = 0x31,
	  .layout_id = P_OP_LAYOUT_MOVX_DUAL_4,
	  .src_dst_swap = 1,
	},
	/* Regular reg read, with no inc and increment. */
	{ .op_str = "MOV_P",
	  .op = 0x32,
	  .layout_id = P_OP_LAYOUT_MOVX_DUAL_4,
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOV_P",
	  .op = 0x33,
	  .layout_id = P_OP_LAYOUT_MOVX_DUAL_4,
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOV_P-38",
	  .op = 0x38,
	  .layout_id = P_OP_LAYOUT_MOV_4,
	  .src_dst_swap = 0,
	},
	/* Regular 0x00c8 reg read. */
	{ .op_str = "MOV_P",
	  .op = 0x39,
	  .layout_id = P_OP_LAYOUT_MOVX_4,
	  .src_dst_swap = 0,
	},
	/* 0x00c9 type read. */
	{ .op_str = "MOV_T1_P",
	  .op = 0x3a,
	  .layout_id = P_OP_LAYOUT_MOVX_4,
	  .src_dst_swap = 0,
	},
	/* Regular 0x00c8 reg read. */
	{ .op_str = "MOV_P",
	  .op = 0x3d,
	  .layout_id = P_OP_LAYOUT_MOVX_4,
	  .src_dst_swap = 1,
	},
	/* 0x00c9 type read. */
	{ .op_str = "MOV_T1_P",
	  .op = 0x3e,
	  .layout_id = P_OP_LAYOUT_MOVX_4,
	  .src_dst_swap = 1,
	},
	/* Conditional execution. */
	{ .op_str = "EXEC_COND_P",
	  .op = 0x3f,
	  .layout_id = P_OP_LAYOUT_EXECUTE_COND_4,
	},
};

const dsp_op_info *get_dsp_p_op_info(uint32_t opcode, uint32_t op_len)
{
	const dsp_op_info *p_ops;
	uint32_t i, p_ops_cnt;

	if (op_len == 2) {
		p_ops = parallel_2_asm_ops;
		p_ops_cnt = ARRAY_SIZE(parallel_2_asm_ops);
	} else {
		p_ops = parallel_4_asm_ops;
		p_ops_cnt = ARRAY_SIZE(parallel_4_asm_ops);
	}

	for (i = 0; i < p_ops_cnt; i++) {
		if (p_ops[i].op == opcode)
			return &p_ops[i];
	}

	return NULL;
}

static const op_operand_layout operand_layouts[] = {
	/*
	 * OP_LAYOUT_MOV_1:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 16,
		       .part1_bits      = 1 },
	       .layout_val  = 0x00,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 9,
		     .part1_bits      = 7,
		     .operand_type    = OP_OPERAND_REG_7,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 18,
		     .part1_bits      = 7,
		     .operand_type    = OP_OPERAND_REG_7,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 16,
		       .part1_bits      = 1 },
	       .layout_val  = 0x1,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 17,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 21,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_REG_3,
		     .operand_dir     = OPERAND_DIR_SRC,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 9,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 13,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_REG_3_8,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000001, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 2, },
	/*
	 * OP_LAYOUT_MOVX_1:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 16,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 9,
		     .part1_bits      = 7,
		     .part2_bit_start = 21,
		     .part2_bits      = 4,
		     .operand_type    = OP_OPERAND_A_REG_INT_7_OFFSET,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * OP_LAYOUT_R_X_Y_1:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 16,
		       .part1_bits      = 1 },
	       .layout_val  = 0x00,
	       .operand_cnt = 3,
	       .operand_loc = {
		   { .part1_bit_start = 9,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 20,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 15,
		     .part1_bits      = 1,
		     .part2_bit_start = 17,
		     .part2_bits      = 3,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_Y, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 16,
		       .part1_bits      = 1 },
	       .layout_val  = 0x01,
	       .operand_cnt = 6,
	       .operand_loc = {
		   { .part1_bit_start = 17,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_REG_3_ACC,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 22,
		     .part1_bits      = 2,
		     .operand_type    = OP_OPERAND_REG_2_T2,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 20,
		     .part1_bits      = 2,
		     .operand_type    = OP_OPERAND_REG_2,
		     .operand_dir     = OPERAND_DIR_Y,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 9,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_REG_3_ACC,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 14,
		     .part1_bits      = 2,
		     .operand_type    = OP_OPERAND_REG_2_T1,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 12,
		     .part1_bits      = 2,
		     .operand_type    = OP_OPERAND_REG_2_8,
		     .operand_dir     = OPERAND_DIR_Y, } },
	       .bitmask = { 0x00000001, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 2, },
	/*
	 * OP_LAYOUT_R_X_Y_A_1:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 9,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_REG_3_FMA,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 17,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 12,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_Y, },
		   { .part1_bit_start = 21,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_A, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * OP_LAYOUT_R_X_LIT_8_Y_1:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 3,
	       .operand_loc = {
		   { .part1_bit_start = 21,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 17,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 9,
		     .part1_bits      = 8,
		     .operand_type    = OP_OPERAND_LITERAL_8_INT,
		     .operand_dir     = OPERAND_DIR_Y, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * OP_LAYOUT_PC_OFFSET_1:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 9,
		     .part1_bits      = 8,
		     .operand_type    = OP_OPERAND_LITERAL_8,
		     .operand_dir     = OPERAND_DIR_NONE, },
		   { .part1_bit_start = 17,
		     .part1_bits      = 8,
		     .operand_type    = OP_OPERAND_LITERAL_8_INT_PC_OFFSET,
		     .operand_dir     = OPERAND_DIR_NONE, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * OP_LAYOUT_LOOP_PC_OFFSET_REG_CNT_1:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 12,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5,
		     .operand_dir     = OPERAND_DIR_NONE, },
		   { .part1_bit_start = 17,
		     .part1_bits      = 8,
		     .operand_type    = OP_OPERAND_LITERAL_8_INT_PC_OFFSET,
		     .operand_dir     = OPERAND_DIR_NONE, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * OP_LAYOUT_LOOP_PC_OFFSET_LIT_8_CNT_1:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 9,
		     .part1_bits      = 8,
		     .operand_type    = OP_OPERAND_LITERAL_8_INT,
		     .operand_dir     = OPERAND_DIR_NONE, },
		   { .part1_bit_start = 17,
		     .part1_bits      = 8,
		     .operand_type    = OP_OPERAND_LITERAL_8_INT_PC_OFFSET,
		     .operand_dir     = OPERAND_DIR_NONE, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * OP_LAYOUT_PC_SET_REG_1:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 18,
		       .part1_bits      = 1 },
	       .layout_val  = 0x01,
	       .operand_cnt = 3,
	       .operand_loc = {
		   { .part1_bit_start = 9,
		     .part1_bits      = 8,
		     .operand_type    = OP_OPERAND_LITERAL_8,
		     .operand_dir     = OPERAND_DIR_NONE, },
		   { .part1_bit_start = 19,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_CALL_MDFR,
		     .operand_dir     = OPERAND_DIR_NONE, },
		   { .part1_bit_start = 22,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_CALL,
		     .operand_dir     = OPERAND_DIR_NONE, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 9,
		     .part1_bits      = 8,
		     .operand_type    = OP_OPERAND_LITERAL_8,
		     .operand_dir     = OPERAND_DIR_NONE, },
		   { .part1_bit_start = 22,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_CALL,
		     .operand_dir     = OPERAND_DIR_NONE, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 2, },
	/*
	 * OP_LAYOUT_STACK_UNK_1:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 20,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 9,
		     .part1_bits      = 8,
		     .operand_type    = OP_OPERAND_LITERAL_8,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * OP_LAYOUT_MOV_2:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 10,
		       .part1_bits      = 7 },
	       .layout_val  = 0x7c,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 29,
		     .part1_bits      = 21,
		     .operand_type    = OP_OPERAND_LITERAL_16_ADDR,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 17,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 10,
		       .part1_bits      = 7 },
	       .layout_val  = 0x7d,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 17,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 29,
		     .part1_bits      = 21,
		     .operand_type    = OP_OPERAND_LITERAL_16_ADDR,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 38,
		       .part1_bits      = 1 },
	       .layout_val  = 0x00,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 28,
		     .part1_bits      = 10,
		     .operand_type    = OP_OPERAND_REG_10,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 39,
		     .part1_bits      = 10,
		     .operand_type    = OP_OPERAND_REG_10,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .supports_opt_args = 1,
	       .bitmask = { 0x00000000, 0x00000001, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 38,
		       .part1_bits      = 1 },
	       .layout_val  = 0x1,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 39,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 45,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_SRC,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 28,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 34,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .supports_opt_args = 1,
	       .bitmask = { 0x00000000, 0x00000001, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 4, },
	/*
	 * OP_LAYOUT_MOVX_MDFR_OFFSET_2:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 46,
		       .part1_bits      = 1 },
	       .layout_val  = 0x00,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 41,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 38,
		     .part1_bits      = 3,
		     .part2_bit_start = 47,
		     .part2_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_X_MDFR_OFFSET,
		     .operand_dir     = OPERAND_DIR_SRC,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 21,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 18,
		     .part1_bits      = 3,
		     .part2_bit_start = 27,
		     .part2_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_Y_MDFR_OFFSET,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 23,
		       .part1_bits      = 1 },
	       .layout_val  = 0x01,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 41,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 38,
		     .part1_bits      = 3,
		     .part2_bit_start = 47,
		     .part2_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_Y_MDFR_OFFSET,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000008, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 41,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 38,
		     .part1_bits      = 3,
		     .part2_bit_start = 47,
		     .part2_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_X_MDFR_OFFSET,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000008, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 3, },
	/*
	 * OP_LAYOUT_MOVX_LIT_OFFSET_2:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 46,
		       .part1_bits      = 1 },
	       .layout_val  = 0x00,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 41,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 27,
		     .part1_bits      = 14,
		     .part2_bit_start = 47,
		     .part2_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_X_INT_11_OFFSET,
		     .operand_dir     = OPERAND_DIR_SRC,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 21,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 10,
		     .part1_bits      = 11,
		     .part2_bit_start = 47,
		     .part2_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_Y_INT_11_OFFSET,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 41,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 23,
		     .part1_bits      = 18,
		     .part2_bit_start = 47,
		     .part2_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_INT_17_OFFSET,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000008, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 2, },
	/*
	 * OP_LAYOUT_R_X_Y_2:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 10,
		       .part1_bits      = 7 },
	       .layout_val  = 0x7d,
	       .operand_cnt = 3,
	       .operand_loc = {
		   { .part1_bit_start = 17,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 29,
		     .part1_bits      = 21,
		     .operand_type    = OP_OPERAND_LITERAL_16_ADDR,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 25,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_Y, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 38,
		       .part1_bits      = 1 },
	       .layout_val  = 0x00,
	       .operand_cnt = 3,
	       .operand_loc = {
		   { .part1_bit_start = 28,
		     .part1_bits      = 7,
		     .operand_type    = OP_OPERAND_REG_7,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 43,
		     .part1_bits      = 7,
		     .operand_type    = OP_OPERAND_REG_7,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 35,
		     .part1_bits      = 3,
		     .part2_bit_start = 39,
		     .part2_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_7,
		     .operand_dir     = OPERAND_DIR_Y, } },
	       .supports_opt_args = 1,
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 38,
		       .part1_bits      = 1 },
	       .layout_val  = 0x01,
	       .operand_cnt = 6,
	       .operand_loc = {
		   { .part1_bit_start = 39,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 46,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_REG_3_X_T1,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 43,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_REG_3_Y_T1,
		     .operand_dir     = OPERAND_DIR_Y,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 28,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 35,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_REG_3_X_T2,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 32,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_REG_3_Y_T2,
		     .operand_dir     = OPERAND_DIR_Y, } },
	       .supports_opt_args = 1,
	       .bitmask = { 0x00000000, 0x00000001, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 3, },
	/*
	 * OP_LAYOUT_R_X_Y_A_2:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 30,
		       .part1_bits      = 2 },
	       .layout_val  = 0x00,
	       .operand_cnt = 8,
	       .operand_loc = {
		   { .part1_bit_start = 39,
		     .part1_bits      = 2,
		     .operand_type    = OP_OPERAND_REG_2_4,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 44,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_REG_3_FMA_X_T1,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 41,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_REG_3_FMA_Y_T1,
		     .operand_dir     = OPERAND_DIR_Y, },
		   { .part1_bit_start = 47,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_REG_3_FMA_A_T1,
		     .operand_dir     = OPERAND_DIR_A,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 28,
		     .part1_bits      = 2,
		     .operand_type    = OP_OPERAND_REG_2_12,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 33,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_REG_3_FMA_X_T2,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 30,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_REG_3_FMA_Y_T2,
		     .operand_dir     = OPERAND_DIR_Y, },
		   { .part1_bit_start = 36,
		     .part1_bits      = 3,
		     .operand_type    = OP_OPERAND_REG_3_FMA_A_T1,
		     .operand_dir     = OPERAND_DIR_A, } },
	       .supports_opt_args = 1,
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 30,
		       .part1_bits      = 2 },
	       .layout_val  = 0x3,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 33,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 42,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 38,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_Y, },
		   { .part1_bit_start = 46,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_A, } },
	       .supports_opt_args = 1,
	       .bitmask = { 0x00000000, 0x000e0000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 2, },
	/*
	 * OP_LAYOUT_MOV_LIT_16_2:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 39,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 12,
		     .part1_bits      = 16,
		     .operand_type    = OP_OPERAND_LITERAL_16,
		     .operand_dir     = OPERAND_DIR_SRC,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 28,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 12,
		     .part1_bits      = 16,
		     .operand_type    = OP_OPERAND_LITERAL_16,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * OP_LAYOUT_R_X_LIT_16_Y_2:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 3,
	       .operand_loc = {
		   { .part1_bit_start = 39,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 28,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 12,
		     .part1_bits      = 16,
		     .operand_type    = OP_OPERAND_LITERAL_16,
		     .operand_dir     = OPERAND_DIR_Y, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * OP_LAYOUT_PC_LIT_16_2:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 20,
		     .part1_bits      = 8,
		     .operand_type    = OP_OPERAND_LITERAL_8,
		     .operand_dir     = OPERAND_DIR_NONE, },
		   { .part1_bit_start = 34,
		     .part1_bits      = 16,
		     .operand_type    = OP_OPERAND_LITERAL_16,
		     .operand_dir     = OPERAND_DIR_NONE, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * OP_LAYOUT_LOOP_PC_LIT_16_2:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 9,
		       .part1_bits      = 1 },
	       .layout_val  = 0x00,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 24,
		     .part1_bits      = 4,
		     .operand_type    = OP_OPERAND_REG_4,
		     .operand_dir     = OPERAND_DIR_NONE, },
		   { .part1_bit_start = 34,
		     .part1_bits      = 16,
		     .operand_type    = OP_OPERAND_LITERAL_16,
		     .operand_dir     = OPERAND_DIR_NONE, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 9,
		       .part1_bits      = 1 },
	       .layout_val  = 0x1,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 12,
		     .part1_bits      = 16,
		     .operand_type    = OP_OPERAND_LITERAL_16,
		     .operand_dir     = OPERAND_DIR_NONE, },
		   { .part1_bit_start = 34,
		     .part1_bits      = 16,
		     .operand_type    = OP_OPERAND_LITERAL_16,
		     .operand_dir     = OPERAND_DIR_NONE, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 2, },
	/*
	 * OP_LAYOUT_STACK_UNK_2:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 39,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 28,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_LITERAL_11,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .supports_opt_args = 1,
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * OP_LAYOUT_MOV_4:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 77,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 89,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_SRC,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 53,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 65,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .supports_opt_args = 1,
	       .bitmask = { 0x00000000, 0x00002000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * OP_LAYOUT_R_X_Y_4:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 10,
		       .part1_bits      = 3 },
	       .layout_val  = 0x00,
	       .operand_cnt = 6,
	       .operand_loc = {
		   { .part1_bit_start = 67,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 89,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 78,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_Y,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 34,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 56,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 45,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_Y, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 36,
		       .part1_bits      = 1 },
	       .layout_val  = 0x00,
	       .operand_cnt = 3,
	       .operand_loc = {
		   { .part1_bit_start = 67,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 89,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 78,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_Y, } },
	       .supports_opt_args = 1,
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 36,
		       .part1_bits      = 1 },
	       .layout_val  = 0x1,
	       .operand_cnt = 6,
	       .operand_loc = {
		   { .part1_bit_start = 67,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 89,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 78,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_Y,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 37,
		     .part1_bits      = 10,
		     .part2_bit_start = 67,
		     .part2_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11_10_OFFSET,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 57,
		     .part1_bits      = 10,
		     .part2_bit_start = 89,
		     .part2_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11_10_OFFSET,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 47,
		     .part1_bits      = 10,
		     .part2_bit_start = 78,
		     .part2_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11_10_OFFSET,
		     .operand_dir     = OPERAND_DIR_Y, } },
	       .supports_opt_args = 1,
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 3, },
	/*
	 * OP_LAYOUT_R_X_Y_A_4:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 10,
		       .part1_bits      = 2 },
	       .layout_val  = 0x00,
	       .operand_cnt = 8,
	       .operand_loc = {
		   { .part1_bit_start = 56,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 78,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 67,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_Y, },
		   { .part1_bit_start = 89,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_A,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 12,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 34,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 23,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_Y, },
		   { .part1_bit_start = 45,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_A, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 39,
		       .part1_bits      = 1 },
	       .layout_val  = 0x00,
	       .operand_cnt = 8,
	       .operand_loc = {
		   { .part1_bit_start = 56,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 78,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 67,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_Y, },
		   { .part1_bit_start = 89,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_A,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 40,
		     .part1_bits      = 4,
		     .part2_bit_start = 56,
		     .part2_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11_4_OFFSET,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 44,
		     .part1_bits      = 4,
		     .part2_bit_start = 78,
		     .part2_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11_4_OFFSET,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 48,
		     .part1_bits      = 4,
		     .part2_bit_start = 67,
		     .part2_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11_4_OFFSET,
		     .operand_dir     = OPERAND_DIR_Y, },
		   { .part1_bit_start = 52,
		     .part1_bits      = 4,
		     .part2_bit_start = 89,
		     .part2_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11_4_OFFSET,
		     .operand_dir     = OPERAND_DIR_A, } },
	       .supports_opt_args = 1,
	       .bitmask = { 0x00000000, 0x00003800, 0x00000000, 0x00000000, } },
	     { .layout_val_loc = {
		       .part1_bit_start = 39,
		       .part1_bits      = 1 },
	       .layout_val  = 0x1,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 56,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 78,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 67,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_Y, },
		   { .part1_bit_start = 89,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_A, } },
	       .supports_opt_args = 1,
	       .bitmask = { 0x00000000, 0x00003800, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 3, },
	/*
	 * OP_LAYOUT_MOV_LIT_32_4:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 4,
	       .operand_loc = {
		   { .part1_bit_start = 89,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 42,
		     .part1_bits      = 32,
		     .operand_type    = OP_OPERAND_LITERAL_32,
		     .operand_dir     = OPERAND_DIR_SRC,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 78,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 10,
		     .part1_bits      = 32,
		     .operand_type    = OP_OPERAND_LITERAL_32,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * OP_LAYOUT_R_X_LIT_32_Y_4:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 6,
	       .operand_loc = {
		   { .part1_bit_start = 90,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 95,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 42,
		     .part1_bits      = 32,
		     .operand_type    = OP_OPERAND_LITERAL_32,
		     .operand_dir     = OPERAND_DIR_Y,
		     .parallel_end    = 1, },
		   { .part1_bit_start = 80,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 85,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 10,
		     .part1_bits      = 32,
		     .operand_type    = OP_OPERAND_LITERAL_32,
		     .operand_dir     = OPERAND_DIR_Y, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * OP_LAYOUT_NOP:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 1,
	       .operand_loc = {
		   { .part1_bit_start = 9,
		     .part1_bits      = 1,
		     .operand_type    = OP_OPERAND_NOP,
		     .operand_dir     = OPERAND_DIR_NONE, } },
	       .supports_opt_args = 1,
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
};

const op_operand_layout *get_op_layout(uint32_t layout_id)
{
	if (layout_id < ARRAY_SIZE(operand_layouts))
		return &operand_layouts[layout_id];
	else
		return NULL;
}

static const dsp_op_info asm_ops[] = {
	{ .op_str = "NOP",
	  .op = 0x0000,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_NOP,
	},
	{ .op_str = "RET",
	  .op = 0x0007,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_NOP,
	},
	{ .op_str = "JMP",
	  .op = 0x000c,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_PC_SET_REG_1,
	},
	{ .op_str = "CALL",
	  .op = 0x000f,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_PC_SET_REG_1,
	},
	{ .op_str = "S_JMP",
	  .op = 0x0010,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_PC_OFFSET_1,
	},
	{ .op_str = "S_JMPC",
	  .op = 0x0011,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_PC_OFFSET_1,
	},
	{ .op_str = "S_JMPC_T1",
	  .op = 0x0012,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_PC_OFFSET_1,
	},
	{ .op_str = "S_CALL",
	  .op = 0x0013,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_PC_OFFSET_1,
	},
	{ .op_str = "RETI",
	  .op = 0x0016,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_NOP,
	},
	{ .op_str = "LOOP-0x01c",
 	  .op = 0x001c,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_LOOP_PC_OFFSET_LIT_8_CNT_1,
 	},
	{ .op_str = "LOOP-0x01d",
 	  .op = 0x001d,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_LOOP_PC_OFFSET_REG_CNT_1,
 	},
	{ .op_str = "HALT",
	  .op = 0x0023,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_NOP,
	},
	{ .op_str = "INT_ENABLE",
	  .op = 0x0024,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_NOP,
	},
	{ .op_str = "INT_DISABLE",
	  .op = 0x0025,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_NOP,
	},
	/* 0x00c9 behavior. */
	{ .op_str = "MOVX_T1_1",
	  .op = 0x0034,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOVX_1,
	},
	{ .op_str = "MOVX_T1_1",
	  .op = 0x0035,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOVX_1,
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOVX_1",
	  .op = 0x0036,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOVX_1,
	},
	{ .op_str = "MOVX_1",
	  .op = 0x0037,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOVX_1,
	  .src_dst_swap = 1,
	},
	{ .op_str = "UNK-0x042",
	  .op = 0x0042,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x043",
	  .op = 0x0043,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x044",
	  .op = 0x0044,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x045",
	  .op = 0x0045,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x046",
	  .op = 0x0046,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x049",
	  .op = 0x0049,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x04a",
	  .op = 0x004a,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x04b",
	  .op = 0x004b,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x04c",
	  .op = 0x004c,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x04d",
	  .op = 0x004d,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x04e",
	  .op = 0x004e,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x04f",
	  .op = 0x004f,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x050",
	  .op = 0x0050,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x051",
	  .op = 0x0051,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "UNK-0x052",
	  .op = 0x0052,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x053",
	  .op = 0x0053,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x054",
	  .op = 0x0054,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x055",
	  .op = 0x0055,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x056",
	  .op = 0x0056,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x057",
	  .op = 0x0057,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x058",
	  .op = 0x0058,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "UNK-0x059",
	  .op = 0x0059,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "UNK-0x05a",
	  .op = 0x005a,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x05b",
	  .op = 0x005b,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x05c",
	  .op = 0x005c,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x05d",
	  .op = 0x005d,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x05e",
	  .op = 0x005e,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x05f",
	  .op = 0x005f,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x060",
	  .op = 0x0060,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x061",
	  .op = 0x0061,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x062",
	  .op = 0x0062,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x063",
	  .op = 0x0063,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x064",
	  .op = 0x0064,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x065",
	  .op = 0x0065,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x067",
	  .op = 0x0067,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "I_FMA",
	  .op = 0x0068,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x069",
	  .op = 0x0069,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "I_FMS",
	  .op = 0x006a,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x06b",
	  .op = 0x006b,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "I_NFMA",
	  .op = 0x006c,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "I_ADD",
	  .op = 0x006e,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "I_SUB",
	  .op = 0x0070,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x071",
	  .op = 0x0071,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x074",
	  .op = 0x0074,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x075",
	  .op = 0x0075,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "MOV",
	  .op = 0x0076,
	  .has_op_layout = 1,
	  .src_mdfr = { OPERAND_MDFR_INC },
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "MOV",
	  .op = 0x0077,
	  .has_op_layout = 1,
	  .src_mdfr = { OPERAND_MDFR_DEC },
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "UNK-0x078",
	  .op = 0x0078,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "MOV",
	  .op = 0x0079,
	  .has_op_layout = 1,
	  .src_mdfr = { OPERAND_MDFR_RR },
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "I_MUL",
	  .op = 0x007a,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x07b",
	  .op = 0x007b,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x07c",
	  .op = 0x007c,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "I_MUL-0x07d",
	  .op = 0x007d,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x07e",
	  .op = 0x007e,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "UNK-0x07f",
	  .op = 0x007f,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "F_FMA",
	  .op = 0x0080,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x082",
	  .op = 0x0082,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x083",
	  .op = 0x0083,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "F_NFMA",
	  .op = 0x0085,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x086",
	  .op = 0x0086,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "F_FMS",
	  .op = 0x0087,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "F_ADD",
	  .op = 0x0088,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "F_SUB",
	  .op = 0x0089,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x08a",
	  .op = 0x008a,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x08b",
	  .op = 0x008b,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "F_MUL",
	  .op = 0x008c,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x08d",
	  .op = 0x008d,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "F_MUL-8e",
	  .op = 0x008e,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "F_INV_SIGN",
	  .op = 0x008f,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "MOV-0x090",
	  .op = 0x0090,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "UNK-0x0091",
	  .op = 0x0091,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x092",
	  .op = 0x0092,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "F_ADD_EXP",
	  .op = 0x0093,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x094",
	  .op = 0x0094,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "F_GET_EXP",
	  .op = 0x0095,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "UNK-0x096",
	  .op = 0x0096,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "INT_TO_FLOAT",
	  .op = 0x0097,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x098",
	  .op = 0x0098,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x099",
	  .op = 0x0099,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "UNK-0x09a",
	  .op = 0x009a,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "UNK-0x09b",
	  .op = 0x009b,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "F_RCP",
	  .op = 0x009c,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "UNK-0x09d",
	  .op = 0x009d,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "UNK-0x09e",
	  .op = 0x009e,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0a0",
	  .op = 0x00a0,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x0a1",
	  .op = 0x00a1,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0a2",
	  .op = 0x00a2,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0a3",
	  .op = 0x00a3,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0a4",
	  .op = 0x00a4,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0a5",
	  .op = 0x00a5,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0a6",
	  .op = 0x00a6,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0a7",
	  .op = 0x00a7,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0a8",
	  .op = 0x00a8,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0a9",
	  .op = 0x00a9,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0aa",
	  .op = 0x00aa,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0ab",
	  .op = 0x00ab,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0ac",
	  .op = 0x00ac,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0ad",
	  .op = 0x00ad,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0ae",
	  .op = 0x00ae,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0af",
	  .op = 0x00af,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "ADD",
	  .op = 0x00b0,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	{ .op_str = "UNK-0x0c0",
	  .op = 0x00c0,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	{ .op_str = "UNK-0x0c1",
	  .op = 0x00c1,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	{ .op_str = "UNK-0x0c2",
	  .op = 0x00c2,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	/*
	 * 0xc6-0xc7 seem broken, they're supposed to be upper 16-bit literal
	 * set, but the literal value seems to overlap the register operand
	 * value.
	 */
	{ .op_str = "UNK-0x0c6",
	  .op = 0x00c6,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "UNK-0x0c7",
	  .op = 0x00c7,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "MOV",
	  .op = 0x00c8,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	/*
	 * Reads from the upper 32-bits of R04/R05/R12/R13.
	 */
	{ .op_str = "MOV_T1",
	  .op = 0x00c9,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	/*
	 * Reads from the highest 8-bits of R04/R05/R12/R13.
	 */
	{ .op_str = "MOV_T2",
	  .op = 0x00ca,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "UNK-0x0cb",
	  .op = 0x00cb,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "XOR",
	  .op = 0x00cc,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "OR",
	  .op = 0x00cd,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "AND",
	  .op = 0x00ce,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x0cf",
	  .op = 0x00cf,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "TWOS_CMPL",
	  .op = 0x00d0,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "CMPL",
	  .op = 0x00d1,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "TWO_CMPL_INV",
	  .op = 0x00d2,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "I_CMP_R",
	  .op = 0x00d3,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	{ .op_str = "I_CMP",
	  .op = 0x00d4,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	{ .op_str = "UNK-0x0d5",
	  .op = 0x00d5,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0d6",
	  .op = 0x00d6,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0d7",
	  .op = 0x00d7,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0d8",
	  .op = 0x00d8,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0d9",
	  .op = 0x00d9,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0da",
	  .op = 0x00da,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0db",
	  .op = 0x00db,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x0dc",
	  .op = 0x00dc,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0dd",
	  .op = 0x00dd,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x0de",
	  .op = 0x00de,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "UNK-0x0df",
	  .op = 0x00df,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0e0",
	  .op = 0x00e0,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "UNK-0x0e1",
	  .op = 0x00e1,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_1,
	},
	{ .op_str = "UNK-0x0e2",
	  .op = 0x00e2,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x0e3",
	  .op = 0x00e3,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x0e4",
	  .op = 0x00e4,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	{ .op_str = "UNK-0x0e5",
	  .op = 0x00e5,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	{ .op_str = "RL",
	  .op = 0x00e6,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "RR",
	  .op = 0x00e7,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "RL",
	  .op = 0x00e8,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	{ .op_str = "RR",
	  .op = 0x00e9,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	{ .op_str = "ARITH_RL",
	  .op = 0x00ea,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "ARITH_RR",
	  .op = 0x00eb,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "ARITH_RL",
	  .op = 0x00ec,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	{ .op_str = "ARITH_RR",
	  .op = 0x00ed,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	/* These are both some sort of 'rotate into carry' or something. */
	{ .op_str = "UNK-0x0ee",
	  .op = 0x00ee,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	{ .op_str = "UNK-0x0ef",
	  .op = 0x00ef,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	{ .op_str = "UNK-0x0f0",
	  .op = 0x00f0,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	{ .op_str = "UNK-0x0f1",
	  .op = 0x00f1,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	{ .op_str = "UNK-0x0f2",
	  .op = 0x00f2,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x0f3",
	  .op = 0x00f3,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	{ .op_str = "UNK-0x0f4",
	  .op = 0x00f4,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x0f5",
	  .op = 0x00f5,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	{ .op_str = "UNK-0x0f6",
	  .op = 0x00f6,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x0f7",
	  .op = 0x00f7,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_8_Y_1,
	},
	{ .op_str = "UNK-0x0f8",
	  .op = 0x00f8,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0f9",
	  .op = 0x00f9,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0fa",
	  .op = 0x00fa,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0fb",
	  .op = 0x00fb,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "UNK-0x0fc",
	  .op = 0x00fc,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_1,
	},
	{ .op_str = "UNK-0x0fd",
	  .op = 0x00fd,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_1,
	},
	{ .op_str = "POP-0x0fe",
	  .op = 0x00fe,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_STACK_UNK_1,
	},
	{ .op_str = "PUSH-0x0ff",
	  .op = 0x00ff,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_STACK_UNK_1,
	  .src_dst_swap = 1,
	},
	/* Similarly, instead of JMP, perhaps "LJMP" or "SJMP". */
	{ .op_str = "NOP",
	  .op = 0x0100,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_NOP,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_LAYOUT,
	  .alt_layout_id = OP_LAYOUT_PC_LIT_16_2,
	  .alt_op_str = "JMP",
	},
	{ .op_str = "JMPC",
	  .alt_op_str = "JMPC_T1",
	  .op = 0x0101,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_PC_LIT_16_2,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	/* Could probably call this "LCALL". */
	{ .op_str = "CALL",
	  .op = 0x0102,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_PC_LIT_16_2,
	},
	/* needs research. */
	{ .op_str = "LOOP-0x10c",
	  .op = 0x010c,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_LOOP_PC_LIT_16_2,
	},
	{ .op_str = "LOOP-0x10d",
	  .op = 0x010d,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_LOOP_PC_LIT_16_2,
	},
	{ .op_str = "MOVX_T2_2",
	  .op = 0x0118,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOVX_MDFR_OFFSET_2,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_SRC_DST_SWAP,
	},
	{ .op_str = "MOVX_T1_2",
	  .op = 0x011a,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOVX_LIT_OFFSET_2,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_SRC_DST_SWAP,
	},
	{ .op_str = "MOVX_2",
	  .op = 0x011b,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOVX_LIT_OFFSET_2,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_SRC_DST_SWAP,
	},
	/* Unk, some form of r = x y. */
	{ .op_str = "UNK-0x12a",
	  .op = 0x012a,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_2,
	},
	{ .op_str = "I_FMA-0x134",
	  .op = 0x0134,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_2,
	},
	{ .op_str = "I_ADD_2",
	  .op = 0x0137,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_2,
	},
	{ .op_str = "I_SUB",
	  .op = 0x0138,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_2,
	},
	{ .op_str = "MOV",
	  .op = 0x013b,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_2,
	  .src_mdfr = { OPERAND_MDFR_INC, OPERAND_MDFR_DEC },
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_MDFR,
	},
	{ .op_str = "MOV",
	  .op = 0x013c,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_2,
	  .src_mdfr = { OPERAND_MDFR_RL, OPERAND_MDFR_RR },
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_MDFR,
	},
	{ .op_str = "I_MUL-0x13d",
	  .op = 0x013d,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_2,
	},
	{ .op_str = "I_MUL-0x13e",
	  .op = 0x013e,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_2,
	},
	{ .op_str = "F_FMA",
	  .op = 0x0140,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_2,
	},
	/*
	 * Not sure what to name this one, but:
	 * Data path 0: r += (int)((float_x * float_y) + (float_x * float_y))
	 * Data path 1: r += float_x * float_y;
	 */
	{ .op_str = "UNK-0x141",
	  .op = 0x0141,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_2,
	},
	{ .op_str = "F_NFMA",
	  .op = 0x0142,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_2,
	},
	{ .op_str = "F_FMS",
	  .op = 0x0143,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_2,
	},
	{ .op_str = "F_ADD",
	  .op = 0x0144,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_2,
	},
	{ .op_str = "F_MUL",
	  .op = 0x0146,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_2,
	},
	{ .op_str = "F_CMP",
	  .op = 0x0147,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_2,
	},
	{ .op_str = "UNK-0x148",
          .op = 0x0148,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_2,
	},
	{ .op_str = "INT_TO_FLOAT-0x149",
          .op = 0x0149,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_2,
	},
	{ .op_str = "UNK-0x14a",
          .op = 0x014a,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_2,
	},
	/* Second argument is 2^y * x - then convert. */
	{ .op_str = "FLOAT_TO_INT",
          .op = 0x014b,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_2,
	  .alt_op_str = "INT_TO_FLOAT",
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "UNK-0x14d",
          .op = 0x014d,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_2,
	},
	{ .op_str = "F_RCP",
          .op = 0x014e,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_2,
	},
	{ .op_str = "I_ADD",
          .op = 0x0158,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_16_Y_2,
	  .src_mdfr = { OPERAND_MDFR_16_BIT_SIGNED },
	},
	{ .op_str = "MOV_T2",
	  .alt_op_str = "MOV_T1",
          .op = 0x0161,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_LIT_16_2,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "MOV",
	  .alt_op_str = "MOV_T2",
          .op = 0x0162,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_LIT_16_2,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "MOV_T1",
	  .alt_op_str = "MOV",
          .op = 0x0163,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_LIT_16_2,
	  .src_mdfr = { OPERAND_MDFR_16_BIT_UPPER },
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "MOV",
	  .alt_op_str = "MOV_T1",
          .op = 0x0164,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_2,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "MOV_T2",
          .op = 0x0165,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_2,
	},
	{ .op_str = "AND",
          .op = 0x0166,
	  .has_op_layout = 1,
	  .alt_op_str = "OR",
	  .layout_id = OP_LAYOUT_R_X_Y_2,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "AND_T1",
          .op = 0x0167,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_2,
	},
	{ .op_str = "UNK-0x168",
          .op = 0x0168,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_2,
	},
	{ .op_str = "I_CMP",
          .op = 0x0169,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_2,
	},
	{ .op_str = "I_CMP",
          .op = 0x016a,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_LIT_16_2,
	},
	{ .op_str = "HIGHEST_BIT",
          .op = 0x0170,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_2,
	},
	{ .op_str = "RL",
          .op = 0x0173,
	  .alt_op_str = "RR",
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_2,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "RL",
	  .alt_op_str = "RR",
          .op = 0x0174,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_16_Y_2,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "ARITH_RL",
	  .alt_op_str = "ARITH_RR",
          .op = 0x0175,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_2,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "ARITH_RL",
	  .alt_op_str = "ARITH_RR",
          .op = 0x0176,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_16_Y_2,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	/* Equivalent of 0x0ee/0x0ef. */
	{ .op_str = "UNK-0x177_RL",
	  .alt_op_str = "UNK-0x177_RR",
          .op = 0x0177,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_16_Y_2,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	/* Not fully understood yet, as far as the stack value integer goes. */
	{ .op_str = "POP-0x17f",
          .op = 0x017f,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_STACK_UNK_2,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_SRC_DST_SWAP,
	  .alt_op_str = "PUSH-0x17f",
	},
	{ .op_str = "NOP-0x180",
          .op = 0x0180,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_NOP,
	},
	{ .op_str = "I_ADD-0x1a6",
          .op = 0x01a6,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	},
	{ .op_str = "I_SUB-0x1a7",
          .op = 0x01a7,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	},
	{ .op_str = "I_SUB-0x1aa",
          .op = 0x01aa,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	},
	{ .op_str = "I_SUB-0x1ab",
          .op = 0x01ab,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	},
	{ .op_str = "I_FMA",
          .op = 0x01b4,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_4,
	},
	{ .op_str = "I_NFMA",
          .op = 0x01b6,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_4,
	  .alt_op_str = "I_FMS",
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "I_ADD",
          .op = 0x01b7,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	},
	{ .op_str = "I_SUB-0x1b8",
          .op = 0x01b8,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	},
	{ .op_str = "MOV",
          .op = 0x01bb,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_4,
	  .src_mdfr = { OPERAND_MDFR_INC, OPERAND_MDFR_DEC },
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_MDFR,
	},
	{ .op_str = "I_MUL",
          .op = 0x01bd,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	},
	{ .op_str = "F_FMA-0x1c0",
          .op = 0x01c0,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_4,
	},
	{ .op_str = "F_MUL-0x1c1",
          .op = 0x01c1,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	},
	{ .op_str = "F_FMA-0x1c2",
          .op = 0x01c2,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_4,
	  .alt_op_str = "F_NFMA",
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "F_FMS-0x1c3",
          .op = 0x01c3,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_4,
	},
	{ .op_str = "F_ADD",
          .op = 0x01c4,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	  .alt_op_str = "F_SUB",
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "F_ADDSUB",
          .op = 0x01c5,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	},
	{ .op_str = "F_MUL-0x1c6",
          .op = 0x01c6,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	},
	{ .op_str = "F_NEG-0x1c7",
          .op = 0x01c7,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_4,
	},
	/* Some sort of conditional move I think. */
	{ .op_str = "UNK-0x1c8",
          .op = 0x01c8,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_4,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_LAYOUT,
	  .alt_layout_id = OP_LAYOUT_R_X_Y_A_4,
	},
	{ .op_str = "UNK-0x1c9",
          .op = 0x01c9,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_4,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_LAYOUT,
	  .alt_layout_id = OP_LAYOUT_R_X_Y_4,
	},
	{ .op_str = "FLOAT_TO_INT-0x1cb",
          .op = 0x01cb,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	  .alt_op_str = "INT_TO_FLOAT-0x1cb",
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "UNK-0x1cd",
          .op = 0x01cd,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	},
	{ .op_str = "UNK-0x1d0",
          .op = 0x01d0,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_LAYOUT,
	  .alt_layout_id = OP_LAYOUT_R_X_Y_A_4,
	},
	{ .op_str = "I_ADD-0x1d8",
          .op = 0x01d8,
	  .src_mdfr = { OPERAND_MDFR_32_BIT_SIGNED, 0 },
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_32_Y_4,
	},
	{ .op_str = "MOV_T1",
	  .alt_op_str = "MOV",
          .op = 0x01e0,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_LIT_32_4,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "MOV",
	  .alt_op_str = "MOV_T1",
          .op = 0x01e4,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_4,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "OR_T1-0x1e6",
	  .alt_op_str = "OR-0x1e6",
          .op = 0x01e6,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "AND-0x1e7",
          .op = 0x01e7,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	},
	{ .op_str = "TWOS_CMPL-0x1e8",
          .op = 0x01e8,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_4,
	},
	{ .op_str = "UNK-0x1e9",
          .op = 0x01e9,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_4,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_LAYOUT,
	  .alt_layout_id = OP_LAYOUT_R_X_Y_4,
	},
	{ .op_str = "UNK-0x1ea",
          .op = 0x01ea,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_MOV_LIT_32_4,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_LAYOUT,
	  .alt_layout_id = OP_LAYOUT_R_X_Y_A_4,
	},
	{ .op_str = "RL",
	  .alt_op_str = "RR",
          .op = 0x01f3,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "RL",
	  .alt_op_str = "RR",
          .op = 0x01f4,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_32_Y_4,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "ARITH_RL",
	  .alt_op_str = "ARITH_RR",
          .op = 0x01f5,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "ARITH_RL",
	  .alt_op_str = "ARITH_RR",
          .op = 0x01f6,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_LIT_32_Y_4,
	  .mdfr_bit = 9,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "UNK-0x1fc",
          .op = 0x01fc,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_A_4,
	},
	{ .op_str = "UNK-0x1fd",
          .op = 0x01fd,
	  .has_op_layout = 1,
	  .layout_id = OP_LAYOUT_R_X_Y_4,
	},
};

/*
 * Only three possible op lengths:
 * Length 1 if the opcode is less than 0x0100,
 * Length 2 if it's less than 0x0180,
 * Length 4 for 0x0180 and up.
 */
uint32_t get_dsp_op_len(uint32_t op)
{
	uint32_t len;

	if (op < 0x0100)
		len = 1;
	else if (op < 0x0180)
		len = 2;
	else
		len = 4;

	return len;
}

const dsp_op_info *get_dsp_op_info(uint32_t opcode)
{
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(asm_ops); i++) {
		if (asm_ops[i].op == opcode)
			return &asm_ops[i];
	}

	return NULL;
}

/*
 * Search an array of op_info structures to find one that matches the given
 * string. If we match with an op_str, make sure that's known.
 */
const dsp_op_info *find_dsp_op_info_by_str(const dsp_op_info *ops, const dsp_op_info *start,
		uint32_t op_cnt, char *op_str, uint8_t *alt_str_match)
{
	const dsp_op_info *op_info;
	uint32_t i, start_op_offset;

	if (start)
		start_op_offset = (start - ops) + 1;
	else
		start_op_offset = 0;

	*alt_str_match = 0;
	for (i = start_op_offset; i < op_cnt; i++) {
		op_info = &ops[i];

		if (strcmp(op_info->op_str, op_str)) {
			if (!op_info->alt_op_str)
				continue;

			/*
			 * If we have an alternative op_str, check if it
			 * matches.
			 */
			if (strcmp(op_info->alt_op_str, op_str))
				continue;

			*alt_str_match = 1;
		}

		return op_info;
	}

	return NULL;
}

/*
 * Search the main op_info structures for a certain op string.
 */
const dsp_op_info *find_dsp_op(char *op_str, const dsp_op_info *start,
		uint8_t *alt_str_match)
{
	return find_dsp_op_info_by_str(asm_ops, start, ARRAY_SIZE(asm_ops),
			op_str, alt_str_match);
}

/*
 * Search the p_op_info structures of a given op length for a certain op
 * string..
 */
const dsp_op_info *find_dsp_p_op(char *p_op_str, uint8_t op_len,
		const dsp_op_info *start, uint8_t *alt_str_match)
{
	const dsp_op_info *p_ops;
	uint32_t p_ops_cnt;

	if (op_len == 2) {
		p_ops = parallel_2_asm_ops;
		p_ops_cnt = ARRAY_SIZE(parallel_2_asm_ops);
	} else {
		p_ops = parallel_4_asm_ops;
		p_ops_cnt = ARRAY_SIZE(parallel_4_asm_ops);
	}

	return find_dsp_op_info_by_str(p_ops, start, p_ops_cnt, p_op_str, alt_str_match);
}

/* Functions for getting/setting the bits described in a layout. */
static void set_bits_in_word(uint32_t *word,
                uint32_t bit_start, uint32_t len, uint32_t val)
{
        uint32_t tmp;

        tmp = val & ((1 << len) - 1);
        *word |= (tmp << (25 - (bit_start + len)));
}

static uint32_t extract_bits_from_word(uint32_t word,
                uint32_t bit_start, uint32_t len)
{
        uint32_t tmp, mask;

        mask = (1 << len) - 1;
        tmp = word >> (25 - (bit_start + len));
        tmp &= mask;

        return tmp;
}

static void handle_op_word_bits(uint32_t *op_words, uint32_t start,
		uint32_t len, uint32_t *val, uint32_t set)
{
        uint32_t start_word, cur_word;
        uint32_t tmp, start_bit, bits_left;
        uint32_t bits_in_cur_word;

        /* First, figure out what word our operand starts and ends in. */
        cur_word = start_word = start ? start / 25 : 0;
        start_bit = start - (cur_word * 25);

	/* Get the amount of bits left in the starting word. */
        bits_in_cur_word = 25 - start_bit;
        bits_left = len;

	/*
	 * If the amount of bits we have to get is less than the amount left
	 * in the current word, only pull the amount we need.
	 */
	if (bits_in_cur_word > bits_left)
		bits_in_cur_word = bits_left;

        while (bits_left) {
		if (set) {
			tmp = (*val) >> (bits_left - bits_in_cur_word);
			set_bits_in_word(&op_words[cur_word++], start_bit,
					bits_in_cur_word, tmp);
		} else {
			tmp = extract_bits_from_word(op_words[cur_word++], start_bit,
					bits_in_cur_word);
		}

                bits_left -= bits_in_cur_word;
                if (bits_left > 25)
                        bits_in_cur_word = 25;
                else
                        bits_in_cur_word = bits_left;

		if (!set)
			*val |= tmp << bits_left;

                start_bit = 0;
        }
}

void set_bits_in_op_words(uint32_t *op_words, uint32_t start,
		uint32_t len, uint32_t val)
{
	handle_op_word_bits(op_words, start, len, &val, 1);
}

uint32_t get_bits_in_op_words(uint32_t *op_words, uint32_t start,
		uint32_t len)
{
	uint32_t val = 0;

	handle_op_word_bits(op_words, start, len, &val, 0);

	return val;
}

static void asm_op_operand_val_fixup(const operand_loc_descriptor *loc,
		dsp_asm_op_operand *operand)
{
	dsp_asm_op_operand *tmp_op;
	int32_t i_tmp;
	uint32_t tmp;

	switch (loc->operand_type) {
	case OP_OPERAND_REG_2_4:
		operand->val -= 4;
		break;

	case OP_OPERAND_REG_2_8:
		operand->val -= 8;
		break;

	case OP_OPERAND_REG_2_12:
		operand->val -= 12;
		break;

	case OP_OPERAND_REG_2_T1:
		/* Base is 12, so if we're less than that, set bit 2. */
		if ((operand->val & 0xff) < 12)
			operand->val = 0x02 + (operand->val & 0x01);
		else
			operand->val &= 0x01;

		break;

	case OP_OPERAND_REG_2_T2:
		/* Base is 4, so if we're below that, set bit 1. */
		if ((operand->val & 0xff) < 4)
			operand->val = 0x02 + (operand->val & 0x01);
		else
			operand->val &= 0x01;

		break;

	case OP_OPERAND_REG_3_X_T1:
		/* Base is 4, so if we're below this, need to set MSB. */
		if ((operand->val & 0xff) < 4)
			operand->val = 0x04 + (operand->val & 0x03);
		else
			operand->val &= 0x03;

		break;

	case OP_OPERAND_REG_3_Y_T1:
		if ((operand->val & 0xff) < 12)
			tmp = operand->val & 0xff;
		else
			tmp = 6 + (operand->val & 0x01);

		operand->val = tmp;
		break;

	case OP_OPERAND_REG_3_X_T2:
		/* Base is 12, so if we're below this, need to set MSB. */
		if ((operand->val & 0xff) < 12)
			tmp = 0x04 + (operand->val & 0x03);
		else
			tmp = (operand->val & 0x03);

		operand->val = tmp;
		break;

	case OP_OPERAND_REG_3_Y_T2:
		/*
		 * Registers 12/13 are 0x06/0x07, 4/5 are just 0x04/0x05, and
		 * 8-11 are 0x00-0x03.
		 */
		if ((operand->val & 0xff) > 11)
			operand->val = 6 + (operand->val & 0x01);
		else if (operand->val > 5)
			operand->val &= 0x03;

		break;

	case OP_OPERAND_REG_3_FMA_X_T1:
		if ((operand->val & 0xff) > 7)
			operand->val = 6 + (operand->val & 0x01);

		break;

	case OP_OPERAND_REG_3_FMA_X_T2:
		if ((operand->val & 0xff) < 2)
			operand->val = 6 + (operand->val & 0x01);
		else
			operand->val -= 8;

		break;

	case OP_OPERAND_REG_3_FMA_A_T1:
		if ((operand->val & 0xff) > 11)
			operand->val = 4 + (operand->val & 0x03);
		else
			operand->val &= 0x03;

		break;

	case OP_OPERAND_REG_3_8:
		operand->val -= 8;
		break;

	case OP_OPERAND_REG_3_FMA:
		switch (operand->val & 0x0e) {
		case 12:
			tmp = 0x02;
			break;

		case 8:
			tmp = 0x06;
			break;

		case 0:
			tmp = 0x04;
			break;

		default:
			tmp = 0x00;
			break;
		}

		if (operand->val & 0x01)
			tmp |= 0x01;

		operand->val = tmp;

		break;

	case OP_OPERAND_REG_3_ACC:
		if ((operand->val & 0xff) < 12)
			operand->val = 4 + (operand->val & 0x03);
		else
			operand->val &= 0x03;

		break;

	case OP_OPERAND_REG_11_4_OFFSET:
		tmp_op = operand - 4;

		tmp = operand->val & 0x0f;
		if (tmp < (tmp_op->val & 0x0f))
			tmp += 0x10;

		tmp = tmp - (tmp_op->val & 0x0f);

		operand->val = tmp << 11;
		break;

	case OP_OPERAND_REG_11_10_OFFSET:
		tmp_op = operand - 3;
		i_tmp = (operand->val & 0xff) - (tmp_op->val & 0xff);
		tmp = i_tmp & 0x3ff;

		operand->val = tmp << 11;
		break;


	case OP_OPERAND_LITERAL_16_ADDR:
		if (operand->type == OPERAND_TYPE_IND_LITERAL_Y)
			operand->val |= (1 << 20);

		break;

	case OP_OPERAND_A_REG_INT_7_OFFSET:
		tmp = (operand->val << 1) & ~0x0000000f;
		operand->val &= 0x7;
		operand->val |= tmp;

		if (operand->type == OPERAND_TYPE_IND_A_REG_Y_LIT_OFFSET)
			operand->val |= (1 << 3);

		break;

	case OP_OPERAND_A_REG_INT_17_OFFSET:
		operand->val &= 0xfffff;
		if (operand->type == OPERAND_TYPE_IND_A_REG_Y_LIT_OFFSET)
			operand->val |= 0x100000;

		break;

	case OP_OPERAND_A_REG:
		if (operand->type == OPERAND_TYPE_IND_A_REG_Y)
			operand->val |= (1 << 3);

		break;

	case OP_OPERAND_A_REG_CALL:
	case OP_OPERAND_A_REG_CALL_MDFR:
		operand->val &= 0x07;
		break;

	case OP_OPERAND_LITERAL_16:
		if (operand->mdfr == OPERAND_MDFR_16_BIT_UPPER)
			operand->val = operand->val >> 16;

		break;

	default:
		break;
	}
}

static void set_op_operand(uint32_t *buf,
		const operand_loc_descriptor *loc,
		dsp_asm_op_operand *operand)
{
	uint32_t set_val[2], len, val;

	len = loc->part1_bits + loc->part2_bits;
	asm_op_operand_val_fixup(loc, operand);
	val = operand->val;

	if (loc->part2_bits) {
		set_val[0] = val >> (len - loc->part1_bits);
		set_val[0] &= (1 << loc->part1_bits) - 1;
		set_val[1] = val & ((1 << loc->part2_bits) - 1);
	} else  {
		if (loc->part1_bits < 31) {
			/* Apply bitmask if bits don't occupy full word. */
			set_val[0] = val & ((1 << loc->part1_bits) - 1);
			set_val[1] = 0;
		} else {
			set_val[0] = val;
			set_val[1] = 0;
		}
	}


	set_bits_in_op_words(buf, loc->part1_bit_start, loc->part1_bits, set_val[0]);
	if (loc->part2_bits)
		set_bits_in_op_words(buf, loc->part2_bit_start, loc->part2_bits, set_val[1]);
}

/*
 * Once we've found compatible op_info structures and layouts, now create the
 * op word.
 */
static void create_op_words(dsp_asm_data *data, uint32_t *op_words)
{
	uint32_t buf[4], len, operand_cnt, i, src_dst_swap;
	const operand_loc_descriptor *loc_descriptors;
	const op_operand_loc_layout *loc_layout;
	const operand_loc_descriptor *tmp_desc;
	dsp_asm_op_data *op, *p_op;

	op = &data->op;
	if (data->has_p_op)
		p_op = &data->p_op;
	else
		p_op = NULL;

	len = get_dsp_op_len(op->opcode);
	src_dst_swap = op->src_dst_swap;
	loc_layout = op->loc_layout;

	memset(buf, 0, sizeof(buf));
	buf[0] = (op->opcode << 16);
	if (op->use_op_mdfr_bit) {
		set_bits_in_op_words(buf, op->mdfr_bit, 1, 1);
		if (op->mdfr_bit_type == OP_MDFR_BIT_TYPE_SRC_DST_SWAP)
			src_dst_swap = 1;
	}


	if ((loc_layout) && !p_op && loc_layout->supports_opt_args) {
		if (len == 2) {
			buf[0] |= 0x7fff;
			buf[1] |= 0x01c00000;
		} else if (len == 4) {
			buf[0] |= 0x7fff;
			buf[1] |= 0x01ffc000;
		}
	}

	if ((loc_layout) && loc_layout->layout_val_loc.part1_bits) {
		dsp_asm_op_operand tmp_operand;

		memset(&tmp_operand, 0, sizeof(tmp_operand));
		tmp_operand.val = loc_layout->layout_val;
		set_op_operand(buf, &loc_layout->layout_val_loc, &tmp_operand);
	}

	if (data->op.operand_cnt) {
		operand_cnt = loc_layout->operand_cnt;
		loc_descriptors = loc_layout->operand_loc;

		for (i = 0; i < 4; i++)
			buf[i] |= loc_layout->bitmask[i];

		for (i = 0; i < operand_cnt; i++) {
			if (src_dst_swap) {
				if (i & 0x1)
					tmp_desc = &loc_descriptors[i - 1];
				else
					tmp_desc = &loc_descriptors[i + 1];
			} else {
				tmp_desc = &loc_descriptors[i];
			}

			set_op_operand(buf, tmp_desc, &data->op.operands[i]);
		}
	}

	/*
	 * Could probably unify the regular op and p_op set_op_operand stuffs
	 * into a single function.
	 */
	src_dst_swap = 0;
	if (p_op) {
		buf[0] |= (p_op->opcode << 9);
		loc_layout = data->p_op.loc_layout;
		operand_cnt = loc_layout->operand_cnt;
		loc_descriptors = loc_layout->operand_loc;
		src_dst_swap = p_op->src_dst_swap;

		if (data->p_op.use_op_mdfr_bit) {
			set_bits_in_op_words(buf, p_op->mdfr_bit, 1, 1);
			if (p_op->mdfr_bit_type == OP_MDFR_BIT_TYPE_SRC_DST_SWAP)
				src_dst_swap = 1;
		}

		set_bits_in_op_words(buf, loc_layout->layout_val_loc.part1_bit_start,
				loc_layout->layout_val_loc.part1_bits, loc_layout->layout_val);

		for (i = 0; i < operand_cnt; i++) {
			if (src_dst_swap) {
				if (i & 0x1)
					tmp_desc = &loc_descriptors[i - 1];
				else
					tmp_desc = &loc_descriptors[i + 1];
			} else {
				tmp_desc = &loc_descriptors[i];
			}

			set_op_operand(buf, tmp_desc, &data->p_op.operands[i]);
		}
	}

	for (i = 0; i < len; i++) {
		data->opcode[i] = buf[i];
		op_words[i] = buf[i];
	}
}

static int32_t restore_integer_offset(uint32_t val)
{
	uint32_t tmp;

	if (val & 0x10000000)
		tmp = 0xe0000000 | val;
	else
		tmp = val;

	return (int32_t)tmp;
}

/*
 * Check if the current layouts operand types would work with our assembly
 * data.
 */
static uint32_t check_operand_compatibility(dsp_asm_op_data *op_data,
		layout_operand_info *info, dsp_asm_op_operand *operand,
		uint8_t src_mdfr)
{
	dsp_asm_op_operand *tmp_op;
	uint32_t tmp0, reg_val, ret;
	int32_t i_tmp;

	ret = reg_val = 0;
	/* Easy early exit. */
	if ((operand->type == OPERAND_TYPE_REG)
			&& (info->operand_type > OP_OPERAND_REG_TYPE_END))
		return ret;

	/*
	 * Register values are always represented as their 11-bit version, but
	 * in these comparsions, since we already know what the upper 3 bits
	 * are, use just the register value. Keep the XGPRAM/YGPRAM bits
	 * though, if they're set.
	 */
	if (operand->type == OPERAND_TYPE_REG)
		reg_val = operand->val & 0xcff;

	switch (info->operand_type) {
	case OP_OPERAND_REG_2:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		if (reg_val < 4)
			ret = 1;

		break;

	case OP_OPERAND_REG_2_4:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		if ((reg_val > 3) && (reg_val < 8))
			ret = 1;

		break;

	case OP_OPERAND_REG_2_8:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		if ((reg_val > 7) && (reg_val < 12))
			ret = 1;

		break;

	case OP_OPERAND_REG_2_12:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		if ((reg_val > 11) && (reg_val < 16))
			ret = 1;

		break;

	case OP_OPERAND_REG_2_T1:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		switch (reg_val) {
		case 12:
		case 13:
		case 8:
		case 9:
			ret = 1;
			break;
		default:
			break;
		}

		break;

	case OP_OPERAND_REG_2_T2:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		switch (reg_val) {
		case 4:
		case 5:
		case 0:
		case 1:
			ret = 1;
			break;
		default:
			break;
		}

		break;

	case OP_OPERAND_REG_3_X_T1:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		if (reg_val < 8)
			ret = 1;

		break;

	case OP_OPERAND_REG_3_Y_T1:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		switch (reg_val) {
		case 0  ... 5:
		case 12 ... 13:
			ret = 1;
			break;

		default:
			break;
		}

		break;

	case OP_OPERAND_REG_3_X_T2:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		if ((reg_val & 0x08) && (reg_val < 16))
			ret = 1;

		break;

	case OP_OPERAND_REG_3_Y_T2:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		switch (reg_val) {
		case  4 ...  5:
		case  8 ... 11:
		case 12 ... 13:
			ret = 1;
			break;

		default:
			break;
		}

		break;

	case OP_OPERAND_REG_3_ACC:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		switch (reg_val) {
		case  4 ...  7:
		case 12 ... 15:
			ret = 1;
			break;

		default:
			break;
		}

		break;

	case OP_OPERAND_REG_3_FMA_Y_T1:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		if (reg_val < 6)
			ret = 1;

		break;

	case OP_OPERAND_REG_3_FMA_X_T1:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		switch (reg_val) {
		case 0 ... 5:
		case 8 ... 9:
			ret = 1;
			break;

		default:
			break;
		}

		break;

	case OP_OPERAND_REG_3_FMA_Y_T2:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		if ((reg_val & 0x08) && (reg_val < 13))
			ret = 1;

		break;

	case OP_OPERAND_REG_3_FMA_X_T2:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		switch (reg_val) {
		case 0 ...  1:
		case 8 ... 13:
			ret = 1;
			break;

		default:
			break;
		}

		break;

	case OP_OPERAND_REG_3_FMA_A_T1:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		switch (reg_val) {
		case 4  ...  7:
		case 12 ... 15:
			ret = 1;
			break;

		default:
			break;
		}

		break;

	case OP_OPERAND_REG_3_FMA:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		switch (reg_val) {
		case  0 ...  1:
		case  4 ...  5:
		case  8 ...  9:
		case 12 ... 13:
			ret = 1;
			break;

		default:
			break;
		}

		break;

	case OP_OPERAND_REG_3:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		if (reg_val < 0x08)
			ret = 1;

		break;

	case OP_OPERAND_REG_4:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		if (reg_val < 0x10)
			ret = 1;

		break;

	case OP_OPERAND_REG_5:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		if (reg_val < 0x20)
			ret = 1;

		break;

	case OP_OPERAND_REG_7:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		if (reg_val < 0x80)
			ret = 1;

		break;

	case OP_OPERAND_REG_3_8:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		if ((reg_val < 0x10) && (reg_val > 0x07))
			ret = 1;

		break;

	case OP_OPERAND_REG_10:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		if (reg_val < 0x100)
			ret = 1;

		break;

	case OP_OPERAND_REG_11:
		if (operand->type != OPERAND_TYPE_REG)
			break;
		/* Any register value can be represented by REG_11. */
		ret = 1;

		break;

	case OP_OPERAND_REG_11_4_OFFSET:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		tmp0 = operand->operand_num;
		tmp_op = &op_data->operands[tmp0];
		if ((operand->val & 0xff0) == (tmp_op->val & 0xff0))
			ret = 1;

		break;

	case OP_OPERAND_REG_11_10_OFFSET:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		tmp0 = operand->operand_num;
		tmp_op = &op_data->operands[tmp0];

		/* If the upper 3 bits match, this will work. */
		if ((operand->val & 0x700) == (tmp_op->val & 0x700))
			ret = 1;

		break;

	case OP_OPERAND_A_REG:
		if ((operand->type != OPERAND_TYPE_IND_A_REG_X)
				&& (operand->type != OPERAND_TYPE_IND_A_REG_Y))
			break;

		ret = 1;
		break;

	case OP_OPERAND_A_REG_PLUS_MDFR:
		if ((operand->type != OPERAND_TYPE_IND_A_REG_X_PLUS_MDFR)
				&& (operand->type != OPERAND_TYPE_IND_A_REG_Y_PLUS_MDFR))
			break;

		ret = 1;
		break;

	case OP_OPERAND_A_REG_X_PLUS_MDFR:
		if (operand->type != OPERAND_TYPE_IND_A_REG_X_PLUS_MDFR)
			break;

		ret = 1;

		break;

	case OP_OPERAND_A_REG_Y_PLUS_MDFR:
		if (operand->type != OPERAND_TYPE_IND_A_REG_Y_PLUS_MDFR)
			break;

		ret = 1;

		break;

	case OP_OPERAND_A_REG_CALL_MDFR:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		tmp0 = operand->val & 0x1f;
		if ((tmp0 > 0x17) && (tmp0 < 0x20))
			ret = 1;

		break;


	case OP_OPERAND_A_REG_CALL:
		if (operand->type != OPERAND_TYPE_REG)
			break;

		tmp0 = operand->val & 0x1f;
		if ((tmp0 > 0x0f) && (tmp0 < 0x18))
			ret = 1;

		break;

	case OP_OPERAND_A_REG_X_MDFR_OFFSET:
		if ((operand->type != OPERAND_TYPE_IND_A_REG_X_MDFR_OFFSET))
			break;

		ret = 1;

		break;

	case OP_OPERAND_A_REG_Y_MDFR_OFFSET:
		if (operand->type != OPERAND_TYPE_IND_A_REG_Y_MDFR_OFFSET)
			break;

		ret = 1;

		break;

	case OP_OPERAND_A_REG_INT_7_OFFSET:
		if ((operand->type != OPERAND_TYPE_IND_A_REG_X_LIT_OFFSET) &&
				operand->type != OPERAND_TYPE_IND_A_REG_Y_LIT_OFFSET)
			break;

		i_tmp = restore_integer_offset(operand->val >> 3);
		if ((i_tmp < -0x40) || (i_tmp > 0x3f))
			break;

		ret = 1;

		break;

	case OP_OPERAND_A_REG_INT_17_OFFSET:
		if ((operand->type != OPERAND_TYPE_IND_A_REG_X_LIT_OFFSET) &&
				operand->type != OPERAND_TYPE_IND_A_REG_Y_LIT_OFFSET)
			break;

		i_tmp = restore_integer_offset(operand->val >> 3);
		if ((i_tmp < -0x8000) || (i_tmp > 0x7fff))
			break;

		ret = 1;

		break;

	case OP_OPERAND_A_REG_X_INT_11_OFFSET:
		if (operand->type != OPERAND_TYPE_IND_A_REG_X_LIT_OFFSET)
			break;

		i_tmp = restore_integer_offset(operand->val >> 3);
		if ((i_tmp < -0x400) || (i_tmp > 0x3ff))
			break;

		ret = 1;

		break;

	case OP_OPERAND_A_REG_Y_INT_11_OFFSET:
		if (operand->type != OPERAND_TYPE_IND_A_REG_Y_LIT_OFFSET)
			break;

		i_tmp = restore_integer_offset(operand->val >> 3);
		if ((i_tmp < -0x400) || (i_tmp > 0x3ff))
			break;

		ret = 1;

		break;

	case OP_OPERAND_A_REG_X:
		if ((operand->type != OPERAND_TYPE_IND_A_REG_X))
			break;

		ret = 1;

		break;

	case OP_OPERAND_A_REG_X_INC:
		if ((operand->type != OPERAND_TYPE_IND_A_REG_X_INC))
			break;
		ret = 1;

		break;

	case OP_OPERAND_A_REG_Y_INC:
		if ((operand->type != OPERAND_TYPE_IND_A_REG_Y_INC))
			break;
		ret = 1;

		break;

	case OP_OPERAND_A_REG_Y:
		if ((operand->type != OPERAND_TYPE_IND_A_REG_Y))
			break;

		ret = 1;

		break;


	case OP_OPERAND_LITERAL_7_INT:
		if (operand->type != OPERAND_TYPE_LITERAL)
			break;

		i_tmp = operand->val;
		if ((i_tmp < -0x40) || (i_tmp > 0x3f))
			break;

		ret = 1;

		break;

	case OP_OPERAND_LITERAL_8_INT_PC_OFFSET:
	case OP_OPERAND_LITERAL_8_INT:
		if (operand->type != OPERAND_TYPE_LITERAL)
			break;

		i_tmp = operand->val;
		if ((i_tmp < - 0x80) || (i_tmp > 0x7f))
			break;

		ret = 1;

		break;

	case OP_OPERAND_LITERAL_8:
		if (operand->type != OPERAND_TYPE_LITERAL)
			break;
		if (operand->val < 0x100)
			ret = 1;

		break;

	case OP_OPERAND_LITERAL_11:
		if (operand->type != OPERAND_TYPE_LITERAL)
			break;

		if (operand->val < 0x800)
			ret = 1;

		break;


	case OP_OPERAND_LITERAL_16:
		if (operand->type != OPERAND_TYPE_LITERAL)
			break;

		operand->mdfr = 0;
		/*
		 * If both values aren't the same, 16-bit literals won't work.
		 * Only 32-bit literal instructions allow two different
		 * literal values.
		 */
		tmp0 = op_data->parallel_split_operand + operand->operand_num;
		tmp_op = &op_data->operands[tmp0];
		if (tmp_op->val != operand->val)
			break;

		switch (src_mdfr) {
		case OPERAND_MDFR_16_BIT_UPPER:
			if ((operand->val & 0xffff0000) && !(operand->val & 0xffff))
				ret = 1;
			break;

		case OPERAND_MDFR_16_BIT_SIGNED:
			i_tmp = operand->val;
			if ((i_tmp < -0x8000) || (i_tmp > 0x7fff))
				break;

			ret = 1;
			break;

		default:
			if (operand->val < 0x10000)
				ret = 1;

			break;
		}

		if (ret)
			operand->mdfr = src_mdfr;

		break;

	case OP_OPERAND_LITERAL_16_ADDR:
		if ((operand->type != OPERAND_TYPE_IND_LITERAL_X)
				&& (operand->type != OPERAND_TYPE_IND_LITERAL_Y))
			break;

		ret = 1;
		break;

	case OP_OPERAND_LITERAL_32:
		if (operand->type != OPERAND_TYPE_LITERAL)
			break;

		ret = 1;

		break;

	case OP_OPERAND_NOP:
		if (operand->type != OPERAND_TYPE_NOP)
			break;

		ret = 1;

		break;

	default:
		break;
	}

	return ret;
}

/*
 * Set the temporary layout_operand_info structures with data from the current
 * operand_loc_descriptor structures. This is done to make things easier in
 * the case of the src/dst being swapped.
 */
static void set_layout_operand_info_data(const operand_loc_descriptor *loc_desc,
		layout_operand_info *info, uint8_t src_dst_swap, uint32_t cnt)
{
	uint32_t i;

	for (i = 0; i < cnt; i++) {
		if (src_dst_swap) {
			if (i & 1) {
				info[i].operand_type = loc_desc[i - 1].operand_type;
				info[i].parallel_end = loc_desc[i].parallel_end;
			} else {
				info[i].operand_type = loc_desc[i + 1].operand_type;
				info[i].parallel_end = 0;
			}
		} else {
			info[i].operand_type = loc_desc[i].operand_type;
			info[i].parallel_end = loc_desc[i].parallel_end;
		}
	}
}

static uint8_t check_op_layout_operand_compatibility(const op_operand_layout *layout,
		dsp_asm_op_data *data, uint8_t src_dst_swap, uint8_t src_mdfr)
{
	const op_operand_loc_layout *loc_layout;
	const operand_loc_descriptor *loc_desc;
	layout_operand_info tmp[8];
	uint32_t i, y, unmatched;

	for (i = 0; i < layout->loc_layout_cnt; i++) {
		loc_layout = &layout->loc_layouts[i];
		loc_desc = loc_layout->operand_loc;
		memset(tmp, 0, sizeof(tmp));
		unmatched = 0;

		if (data->needs_alt_args && !loc_layout->supports_opt_args)
			continue;

		if (loc_layout->operand_cnt != data->operand_cnt)
			continue;

		set_layout_operand_info_data(loc_desc, tmp, src_dst_swap, data->operand_cnt);
		for (y = 0; y < data->operand_cnt; y++) {
			if (!check_operand_compatibility(data, &tmp[y],
						&data->operands[y], src_mdfr)) {
				unmatched = 1;
				break;
			}
		}

		if (!unmatched) {
			data->loc_layout = &layout->loc_layouts[i];
			return 1;
		}
	}

	return 0;
}

/*
 * Modifier bit type helpers.
 */
uint8_t mdfr_type_is_alt_src_mdfr(uint32_t mdfr_type)
{
	return mdfr_type == OP_MDFR_BIT_TYPE_USE_ALT_MDFR ? 1 : 0;
}

uint8_t mdfr_type_is_src_dst_swap(uint32_t mdfr_type)
{
	return mdfr_type == OP_MDFR_BIT_TYPE_SRC_DST_SWAP ? 1 : 0;
}

uint8_t mdfr_type_is_alt_layout(uint32_t mdfr_type)
{
	return mdfr_type == OP_MDFR_BIT_TYPE_USE_ALT_LAYOUT ? 1 : 0;
}

/* Check if two src mdfrs are compatible. */
static uint32_t check_src_mdfr_compatibility(uint32_t src_mdfr1, uint32_t src_mdfr2)
{
	if (src_mdfr1 == src_mdfr2)
		return 1;

	/*
	 * If src_mdfr2 is just a hint, and src_mdfr1 has no modifier,
	 * then they can't be ruled out here as incompatible.
	 */
	if ((src_mdfr2 > OPERAND_MDFR_HINT_START) && !(src_mdfr1))
		return 1;

	return 0;
}

/*
 * If we've found an asm op with a matching string, narrow down compatibility.
 * First check for source modifier compatibility, then check for operand
 * layout compatibility. If it's compatible, we've found a usable op.
 */
static uint32_t check_op_layout_compatibility(const dsp_op_info *info,
		dsp_asm_op_data *data, uint32_t is_p_op)
{
	uint32_t mdfr_bit_set, mdfr_bit_type, op_src_mdfr, op_src_dst_swap;
	const op_operand_layout *layout;
	uint32_t layout_id;


	mdfr_bit_set = data->use_op_mdfr_bit;
	layout_id = info->layout_id;
	op_src_mdfr = info->src_mdfr[0];
	op_src_dst_swap = info->src_dst_swap;
	mdfr_bit_type = info->mdfr_bit_type;

	/*
	 * If the modifier bit is set before calling, i.e because we're using
	 * the alternative op string, then change the default values
	 * according to the mdfr_bit_type.
	 */
	if (mdfr_bit_set) {
		switch (mdfr_bit_type) {
		case OP_MDFR_BIT_TYPE_USE_ALT_LAYOUT:
			layout_id = info->alt_layout_id;
			break;

		case OP_MDFR_BIT_TYPE_SRC_DST_SWAP:
			op_src_dst_swap = 1;
			break;

		case OP_MDFR_BIT_TYPE_USE_ALT_MDFR:
			op_src_mdfr = info->src_mdfr[1];
			break;

		default:
			break;
		}
	}

	/*
	 * If we have no operands and the layout id is NOP, we're compatible.
	 * Unless it's a single length NOP and we need opt args.
	 */
	if (!data->operand_cnt && (layout_id == OP_LAYOUT_NOP) && !is_p_op) {
		if (data->needs_alt_args && (get_dsp_op_len(info->op) < 2))
			return 0;

		layout = get_op_layout(layout_id);
		data->loc_layout = &layout->loc_layouts[0];
		return 1;
	}

	/*
	 * Check if the src mdfrs are compatible. If they're not, and the op
	 * has an alternative src mdfr, test that too.
	 */
	if (!check_src_mdfr_compatibility(data->src_mdfr, op_src_mdfr)) {
		if (!mdfr_bit_set && mdfr_type_is_alt_src_mdfr(mdfr_bit_type)) {
			if (!check_src_mdfr_compatibility(data->src_mdfr, info->src_mdfr[1]))
				return 0;

			/* The alternative works, set it. */
			op_src_mdfr = info->src_mdfr[1];
			data->use_op_mdfr_bit = 1;
			mdfr_bit_set = 1;
		} else {
			return 0;
		}
	}

	if (is_p_op)
		layout = get_p_op_layout(layout_id);
	else
		layout = get_op_layout(layout_id);

	/*
	 * Now we check if any of the default layouts have compatible operand
	 * types with our asm op.
	 */
	if (check_op_layout_operand_compatibility(layout, data, op_src_dst_swap, op_src_mdfr))
		return 1;

	/* if we can swap the src/dst, try it and see if it works. */
	if (!mdfr_bit_set && mdfr_type_is_src_dst_swap(mdfr_bit_type)) {
		if (check_op_layout_operand_compatibility(layout, data, 1, op_src_mdfr)) {
			data->use_op_mdfr_bit = 1;
			mdfr_bit_set = 1;

			return 1;
		}
	}

	/* if we can use an alt layout, try this and see if it works. */
	if (!mdfr_bit_set && mdfr_type_is_alt_layout(mdfr_bit_type)) {
		if (is_p_op)
			layout = get_p_op_layout(info->alt_layout_id);
		else
			layout = get_op_layout(info->alt_layout_id);

		if (check_op_layout_operand_compatibility(layout, data, op_src_dst_swap, op_src_mdfr)) {
			data->use_op_mdfr_bit = 1;
			return 1;
		}
	}

	return 0;
}

/*
 * Find an op_info structure to match our assembly info.
 */
static const dsp_op_info *find_compatible_op_info(dsp_asm_data *data,
		const dsp_op_info *start)
{
	const dsp_op_info *op_info;
	uint8_t alt_str_match;

	op_info = start;
	op_info = find_dsp_op(data->op.op_str, op_info, &alt_str_match);
	while (op_info) {
		if (alt_str_match)
			data->op.use_op_mdfr_bit = 1;
		else
			data->op.use_op_mdfr_bit = 0;

		/* If we've found a matching opcode, return. */
		if (check_op_layout_compatibility(op_info, &data->op, 0)) {
			return op_info;
		}

		op_info = find_dsp_op(data->op.op_str, op_info, &alt_str_match);
	}

	return NULL;
}

/*
 * Find a parallel op_info structure to match our assembly info.
 */
static const dsp_op_info *find_compatible_p_op_info(dsp_asm_data *data,
		const dsp_op_info *start, uint8_t op_len)
{
	const dsp_op_info *op_info;
	uint8_t alt_str_match;

	op_info = start;
	op_info = find_dsp_p_op(data->p_op.op_str, op_len, op_info, &alt_str_match);
	while (op_info) {
		if (alt_str_match)
			data->p_op.use_op_mdfr_bit = 1;
		else
			data->p_op.use_op_mdfr_bit = 0;

		/* If we've found a p_op that matches, return. */
		if (check_op_layout_compatibility(op_info, &data->p_op, 1))
			return op_info;

		op_info = find_dsp_p_op(data->p_op.op_str, op_len, op_info, &alt_str_match);
	}

	return NULL;
}

static void set_asm_op_data_from_op_info(dsp_asm_op_data *data, const dsp_op_info *op_info)
{
	data->opcode = op_info->op;
	data->src_dst_swap = op_info->src_dst_swap;
	data->mdfr_bit = op_info->mdfr_bit;
	data->mdfr_bit_type = op_info->mdfr_bit_type;
	data->matched = 1;
}

/*
 * Search all asm ops until we find one that matches our given assembly info.
 */
static void find_compatible_asm_opcode(dsp_asm_data *data)
{
	const dsp_op_info *op_info, *p_op_info;
	uint32_t len;

	op_info = p_op_info = NULL;
	op_info = find_compatible_op_info(data, op_info);
	while (op_info) {
		if (data->has_p_op) {
			len = get_dsp_op_len(op_info->op);
			p_op_info = NULL;
			p_op_info = find_compatible_p_op_info(data, p_op_info, len);

			if (p_op_info)
				break;
		} else {
			break;
		}

		op_info = find_compatible_op_info(data, op_info);
	}

	if (op_info)
		set_asm_op_data_from_op_info(&data->op, op_info);
	if (p_op_info)
		set_asm_op_data_from_op_info(&data->p_op, p_op_info);
}

/*
 * Assembler string parsing definitions.
 */

/* String helper functions. */
static char get_final_str_char(char *str)
{
	return str[strlen(str) - 1];
}

static void remove_final_str_char(char *str)
{
	int32_t len;

	len = strlen(str);
	if (len)
		str[len - 1] = '\0';
}

static uint8_t is_char_upper_alpha(char val)
{
	if (val >= 'A' && val <= 'Z')
		return 1;
	else
		return 0;

}

static void append_str_to_operand_str(char *buf, char *append)
{
	int32_t len;

	len = strlen(buf);

	if (len) {
		buf[len] = ' ';
		strcpy(&buf[len + 1], append);
	} else {
		strcpy(buf, append);
	}

	if (get_final_str_char(append) == ',')
		buf[strlen(buf) - 1] = '\0';
}

/*
 * Check an operands token for a src_mdfr.
 */
static uint8_t check_token_str_for_src_mdfr(char *str)
{
	char tmp0, tmp1;

	tmp0 = str[0];
	tmp1 = str[1];
	if ((tmp0 == '>') && (tmp1 == '>'))
		return OPERAND_MDFR_RR;
	if ((tmp0 == '<') && (tmp1 == '<'))
		return OPERAND_MDFR_RL;

	tmp0 = str[strlen(str) - 1];
	tmp1 = str[strlen(str) - 2];
	if ((tmp0 == '+') && (tmp1 == '+'))
		return OPERAND_MDFR_INC;
	if ((tmp0 == '-') && (tmp1 == '-'))
		return OPERAND_MDFR_DEC;

	return OPERAND_MDFR_NONE;
}

/*
 * Remove trailing src_mdfr characters from the operand string.
 */
static void remove_mdfr_from_str(uint32_t mdfr, char *str)
{
	int32_t len;
	uint32_t i;

	len = strlen(str);

	switch (mdfr) {
	case OPERAND_MDFR_RR:
	case OPERAND_MDFR_RL:
		for (i = 0; i < len; i++) {
			if ((str[i] == '>') || (str[i] == '<'))
				break;
		}

		if (i < len)
			str[i - 1] = '\0';

		break;

	case OPERAND_MDFR_INC:
	case OPERAND_MDFR_DEC:
		str[len - 2] = '\0';
		break;

	default:
		break;
	}
}

/*
 * There are quite a few different indirect address register types, figure out
 * which one it is.
 */
static uint32_t get_indirect_address_reg_type(dsp_asm_op_operand *operand)
{
	uint32_t tmp0, tmp1;
	char *operand_str;
	int32_t i_tmp;

	operand_str = operand->operand_str;

	/* Indirect address is a literal encoded into the instruction. */
	if (operand_str[1] == '#') {
		operand->val = strtol(&operand_str[2], NULL, 0);
		tmp0 = get_final_str_char(operand_str);
		if ((tmp0 == 'Y') || (tmp0 == 'y'))
			operand->type = OPERAND_TYPE_IND_LITERAL_Y;
		else
			operand->type = OPERAND_TYPE_IND_LITERAL_X;

		return 1;

	}

	tmp0 = strtol(&operand_str[4], NULL, 10);
	if ((operand_str[6] == 'Y') || (operand_str[6] == 'y'))
		tmp1 = 1;
	else
		tmp1 = 0;

	if (strlen(operand_str) == 7) {
		/* Type: @A_Rx_X or @A_Rx_Y. */
		operand->val = tmp0;
		if (tmp1) {
			operand->type = OPERAND_TYPE_IND_A_REG_Y;
			operand->val |= 0x08;
		} else
			operand->type = OPERAND_TYPE_IND_A_REG_X;

		return 1;
	}

	if (strlen(operand_str) == 11) {
		/* Type: @A_Rx_X_INC or @A_Rx_Y_INC. */
		operand->val = tmp0;
		operand->val |= 0x08;
		if (tmp1)
			operand->type = OPERAND_TYPE_IND_A_REG_Y_INC;
		else
			operand->type = OPERAND_TYPE_IND_A_REG_X_INC;

		return 1;
	}

	if (operand_str[9] == '=') {
		/* Type is @A_Rx_X += A_MDx or @A_Rx_Y += A_MDx. */
		operand->val = tmp0 << 3;
		operand->val |= strtol(&operand_str[15], NULL, 10);

		if (tmp1) {
			operand->type = OPERAND_TYPE_IND_A_REG_Y_PLUS_MDFR;
			operand->val |= 0x40;
		} else {
			operand->type = OPERAND_TYPE_IND_A_REG_X_PLUS_MDFR;
		}

		return 1;
	}


	if ((operand_str[8] == '+') && (!strncmp(&operand_str[10], "A_MD", 4))) {
		/* Type is @A_Rx_X + A_MDx or @A_Rx_Y + A_MDx. */
		operand->val = (strtol(&operand_str[14], NULL, 10) & 0x07) << 3;
		operand->val |= tmp0 & 0x07;

		if (tmp1)
			operand->type = OPERAND_TYPE_IND_A_REG_Y_MDFR_OFFSET;
		else
			operand->type = OPERAND_TYPE_IND_A_REG_X_MDFR_OFFSET;

		return 1;
	}

	if ((operand_str[8] == '+') || (operand_str[8] == '-')) {
		/* Type is @A_Rx_X +/- 0xXXX or @A_Rx_Y +/- 0xXXX. */
		i_tmp = strtol(&operand_str[10], NULL, 0);
		if (operand_str[8] == '-')
			i_tmp = -i_tmp;

		operand->val = (i_tmp << 3);
		operand->val |= tmp0;

		if (tmp1)
			operand->type = OPERAND_TYPE_IND_A_REG_Y_LIT_OFFSET;
		else
			operand->type = OPERAND_TYPE_IND_A_REG_X_LIT_OFFSET;

		return 1;
	}

	return 0;
}

static uint32_t get_operand_val_from_token(dsp_asm_op_operand *operand)
{
	char *operand_str;
	int32_t i_tmp;

	operand_str = operand->operand_str;
	if (get_dsp_operand_str_val(operand_str, &operand->val)) {
		operand->val |= 0x300;
		operand->type = OPERAND_TYPE_REG;
		return 1;
	}

	if (!strcmp(operand_str, "NO_ARGS")) {
		operand->val = 0;
		operand->type = OPERAND_TYPE_NOP;
		return 1;
	}

	if (!strncmp(operand_str, "XGPRAM", 6)) {
		operand->val = strtol(&operand_str[7], NULL, 10);
		operand->val |= 0x400;
		operand->type = OPERAND_TYPE_REG;
		return 1;
	}

	if (!strncmp(operand_str, "YGPRAM", 6)) {
		operand->val = strtol(&operand_str[7], NULL, 10);
		operand->val |= 0x600;
		operand->type = OPERAND_TYPE_REG;
		return 1;
	}

	if (operand_str[0] == '@')
		return get_indirect_address_reg_type(operand);

	if (operand_str[0] == '#') {
		i_tmp = strtol(&operand_str[1], NULL, 0);
		operand->val = i_tmp;
		operand->type = OPERAND_TYPE_LITERAL;
		return 1;
	}

	if ((operand_str[0] == '0') && (operand_str[1] == 'x'))
		return 0;

	return 0;
}

static const char *parallel_operand_layout_id_str[] = {
	"P_OP_LAYOUT_MOV_2",
	"P_OP_LAYOUT_MOVX_2",
	"P_OP_LAYOUT_MOVX_DUAL_READ_2",
	"P_OP_LAYOUT_MOVX_DUAL_WRITE_2",
	"P_OP_LAYOUT_EXECUTE_COND_2",
	"P_OP_LAYOUT_MOV_4",
	"P_OP_LAYOUT_MOV_DUAL_4",
	"P_OP_LAYOUT_MOVX_4",
	"P_OP_LAYOUT_MOVX_DUAL_4",
	"P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4",
	"P_OP_LAYOUT_EXECUTE_COND_4",
};

static uint32_t get_p_op_layout_id_from_str(char *layout_str, uint32_t *layout_id)
{
	const char *tmp;
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(parallel_operand_layout_id_str); i++) {
		tmp = parallel_operand_layout_id_str[i];
		if (!strcmp(layout_str, &tmp[12])) {
			*layout_id = i;

			return 1;
		}
	}

	return 0;
}

static const char *operand_layout_id_str[] = {
	"OP_LAYOUT_MOV_1",
	"OP_LAYOUT_MOVX_1",
	"OP_LAYOUT_R_X_Y_1",
	"OP_LAYOUT_R_X_Y_A_1",
	"OP_LAYOUT_R_X_LIT_8_Y_1",
	"OP_LAYOUT_PC_OFFSET_1",
	"OP_LAYOUT_LOOP_PC_OFFSET_REG_CNT_1",
	"OP_LAYOUT_LOOP_PC_OFFSET_LIT_8_CNT_1",
	"OP_LAYOUT_PC_SET_REG_1",
	"OP_LAYOUT_STACK_UNK_1",
	"OP_LAYOUT_MOV_2",
	"OP_LAYOUT_MOVX_MDFR_OFFSET_2",
	"OP_LAYOUT_MOVX_LIT_OFFSET_2",
	"OP_LAYOUT_R_X_Y_2",
	"OP_LAYOUT_R_X_Y_A_2",
	"OP_LAYOUT_MOV_LIT_16_2",
	"OP_LAYOUT_R_X_LIT_16_Y_2",
	"OP_LAYOUT_PC_LIT_16_2",
	"OP_LAYOUT_LOOP_PC_LIT_16_2",
	"OP_LAYOUT_STACK_UNK_2",
	"OP_LAYOUT_MOV_4",
	"OP_LAYOUT_R_X_Y_4",
	"OP_LAYOUT_R_X_Y_A_4",
	"OP_LAYOUT_MOV_LIT_32_4",
	"OP_LAYOUT_R_X_LIT_32_Y_4",
	"OP_LAYOUT_NOP",
};

static uint32_t get_op_layout_id_from_str(char *layout_str, uint32_t *layout_id)
{
	const char *tmp;
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(operand_layout_id_str); i++) {
		tmp = operand_layout_id_str[i];
		if (!strcmp(layout_str, &tmp[10])) {
			*layout_id = i;

			return 1;
		}
	}

	return 0;
}

/*
 * Format is:
 * OP_0x164.1:MOV_2 R00, R04;
 * Where:
 * OP_ prefix indicates we're going to specify a specific opcode.
 * ".1" suffix signifies to set the mdfr bit.
 * ":" signifies that we're going to select a specific operand layout.
 *
 * Operand layouts should omit the OP_LAYOUT prefix present in the enum name.
 * For parallel ops, the only difference is that the prefix is P_OP.
 *
 * If an operand layout is not specified, the op_info structure associated
 * with the opcode will be used to find the layout. If there isn't an op_info
 * structure, this will fail.
 *
 * Main usage for this is testing unknown opcodes.
 */
static uint32_t get_asm_opcode_data_from_op_str(char *asm_op_str, uint32_t *opcode,
		uint32_t *layout_id, uint32_t *layout_set, uint8_t *mdfr_bit,
		uint8_t is_p_op)
{
	char *end_ptr;
	uint32_t ret;

	*layout_set = *mdfr_bit = 0;

	/* Get the specified opcode. */
	*opcode = strtol(&asm_op_str[3], &end_ptr, 16);

	/* Check if the mdfr_bit is specified as set/unset. */
	if (*end_ptr == '.')
		*mdfr_bit = strtol(end_ptr + 1, &end_ptr, 0);

	/* Check if a layout is specified. */
	if (*end_ptr == ':') {
		if (is_p_op)
			ret = get_p_op_layout_id_from_str(end_ptr + 1, layout_id);
		else
			ret = get_op_layout_id_from_str(end_ptr + 1, layout_id);

		if (!ret) {
			printf("Invalid layout id %s.\n", end_ptr + 1);
			return 0;
		}

		*layout_set = 1;
	}

	return 1;
}

static void extract_opcode_from_spec_op_str(dsp_asm_data *data)
{
	uint32_t layout_id, layout_set, i;
	const op_operand_layout *layout;
	dsp_asm_op_data *op, *p_op;
	const dsp_op_info *info;

	op = &data->op;
	p_op = &data->p_op;
	if (!get_asm_opcode_data_from_op_str(op->op_str, &op->opcode,
			&layout_id, &layout_set, &op->use_op_mdfr_bit, 0))
		return;

	/*
	 * If a layout wasn't specified, we'll have to find one from an
	 * existing op_info structure, if it exists.
	 */
	if (!layout_set) {
		info = get_dsp_op_info(op->opcode);
		if (!info) {
			printf("No layout specified, and no op info structure. Aborting.\n");
			return;
		}

		if (check_op_layout_compatibility(info, op, 0))
			set_asm_op_data_from_op_info(op, info);
	} else {
		if (op->use_op_mdfr_bit && layout_set)
			op->mdfr_bit = 9;

		layout = get_op_layout(layout_id);
		/* Try with and without src_dst swap. */
		for (i = 0; i < 2; i++) {
			if (check_op_layout_operand_compatibility(layout, op, i, 0)) {
				op->matched = 1;
				op->src_dst_swap = i;
				break;
			}
		}
	}

	/* Next, if we have a p_op, find a compatible one. */
	if (data->has_p_op) {
		if (strncmp(p_op->op_str, "P_OP_0x", 7)) {
			info = NULL;
			info = find_compatible_p_op_info(data, info,
					get_dsp_op_len(op->opcode));
			if (info)
				set_asm_op_data_from_op_info(p_op, info);

			return;
		}

		/*
		 * Add 2 to the pointer to get past the extra "P_" prefix on
		 * parallel ops.
		 */
		if (!get_asm_opcode_data_from_op_str(p_op->op_str + 2, &p_op->opcode,
			&layout_id, &layout_set, &p_op->use_op_mdfr_bit, 1))
			return;

		/*
		 * Should be careful with this. The alt mdfr bit on parallel
		 * opcodes is different per opcode.
		 */
		if (p_op->use_op_mdfr_bit && layout_set)
			p_op->mdfr_bit = 7;

		layout = get_p_op_layout(layout_id);
		/* Try with and without src_dst swap. */
		for (i = 0; i < 2; i++) {
			if (check_op_layout_operand_compatibility(layout, p_op, i, 0)) {
				p_op->matched = 1;
				p_op->src_dst_swap = i;
				break;
			}
		}
	}
}

/*
 * Add an operand from the data currently gathered from the tokens.
 */
static uint8_t finalize_operand(dsp_asm_op_data *op, dsp_asm_op_operand *operand,
		char *buf, uint32_t buf_size, uint32_t cur_operand_cnt)
{
	if (operand->mdfr) {
		remove_mdfr_from_str(operand->mdfr, buf);
		op->src_mdfr = operand->mdfr;
	}

	operand->operand_str = strdup(buf);
	operand->operand_num = cur_operand_cnt;

	if (!get_operand_val_from_token(operand)) {
		printf("Unidentified operand, '%s'. Aborting.\n", operand->operand_str);
		return 0;
	}

	op->operand_cnt++;

	memset(buf, 0, buf_size);

	return 1;
}

/*
 * Sort and identify the opcode info from the given assembly tokens, get the
 * op string, operand values/modifiers, and potential parallel ops as well.
 */
static uint8_t get_opcode_data_from_tokens(dsp_asm_op_data *op, uint32_t t_start,
		uint32_t t_end, dsp_asm_str_tokens *tokens)
{
	uint32_t operand_start, cur_operand_cnt, i;
	dsp_asm_op_operand *cur_operand;
	char buf[0x100], *cur_token;

	operand_start = 0;
	/* Ignore tokens that don't start with an OP string. */
	for (i = t_start; i < t_end; i++) {
		if (is_char_upper_alpha(tokens->token[i][0])) {
			operand_start = i + 1;
			break;
		}
	}

	op->op_str = strdup(tokens->token[operand_start - 1]);
	memset(buf, 0, sizeof(buf));
	cur_operand_cnt = 0;
	cur_operand = NULL;

	/* Operands are separated by commas, not spaces. So continue adding
	 * tokens to the operand string until we encounter a comma. Also,
	 * reset operand_num if we encounter a ':' parallel separator.
	 */
	for (i = operand_start; i < t_end; i++) {
		/*
		 * If we aren't in the middle of getting operands, and we
		 * encounter a token matching the op string, ignore it.
		 */
		if ((!cur_operand_cnt) && (!strcmp(tokens->token[i], op->op_str)))
				continue;

		cur_token = tokens->token[i];
		cur_operand = &op->operands[op->operand_cnt];

		/*
		 * If we encounter a parallel separator, reset the operand
		 * count.
		 */
		if (cur_token[0] == ':') {
			finalize_operand(op, cur_operand, buf, sizeof(buf),
					cur_operand_cnt);
			op->parallel_split_operand = op->operand_cnt;
			cur_operand_cnt = 0;

			continue;
		}

		if (!cur_operand->mdfr)
			cur_operand->mdfr = check_token_str_for_src_mdfr(cur_token);

		append_str_to_operand_str(buf, tokens->token[i]);

		if (get_final_str_char(tokens->token[i]) == ',') {
			if (!finalize_operand(op, cur_operand, buf, sizeof(buf),
					cur_operand_cnt))
				return 0;

			cur_operand_cnt++;
		}
	}

	/* If there's characters in the buffer, add the final operand. */
	if (strlen(buf)) {
		if (!finalize_operand(op, cur_operand, buf, sizeof(buf),
				cur_operand_cnt))
			return 0;
	}

	return 1;
}

/* Remove comments from the passed in assembly string. */
static void remove_asm_str_comments(char *asm_str, char *buf)
{
	uint32_t i, y, in_comment;

	in_comment = 0;
	for (i = y = 0; i < strlen(asm_str); i++) {
		if (!in_comment && (asm_str[i] == '/') && (asm_str[i + 1] == '*')) {
			in_comment = 1;
			i++;
			continue;
		}

		if (!in_comment)
			buf[y++] = asm_str[i];
		else if ((asm_str[i] == '*') && (asm_str[i + 1] == '/')) {
			in_comment = 0;
			i++;
		}
	}

	if (y == strlen(asm_str))
		buf[y - 1] = '\0';
	else
		buf[y] = '\0';

}

/*
 * Split the assembly string into tokens for parsing by
 * get_asm_data_from_tokens.
 */
static void tokenize_asm_str(char *asm_str, dsp_asm_str_tokens *tokens)
{
	const char *delim = " \n\t";
	char *token_str;
	char *buf;

	/* Allocate a buffer for a copy of the assembly string. */
	buf = calloc(strlen(asm_str), sizeof(*buf));
	remove_asm_str_comments(asm_str, buf);

	token_str = strtok(buf, delim);
	while (token_str) {
		tokens->token[tokens->token_cnt++] = strdup(token_str);

		if (get_final_str_char(token_str) == ';') {
			/*
			 * If the final token is just a semi-colon and nothing
			 * else, remove the token entirely. Otherwise, just
			 * remove the semicolon from the end of the token and
			 * keep the rest.
			 */
			if (strlen(token_str) < 2) {
				free(tokens->token[tokens->token_cnt - 1]);
				tokens->token_cnt--;
			} else {
				remove_final_str_char(tokens->token[tokens->token_cnt - 1]);
			}

			break;
		}

		token_str = strtok(NULL, delim);
	}

	free(buf);
}

uint8_t get_asm_data_from_str(dsp_asm_data *data, char *asm_str)
{
	uint32_t i, t_start, buf[4];
	dsp_asm_str_tokens tokens;
	uint8_t ret = 1;

	memset(&tokens, 0, sizeof(tokens));
	tokenize_asm_str(asm_str, &tokens);

	for (i = 0; i < tokens.token_cnt; i++) {
		if (tokens.token[i][0] == '/') {
			data->has_p_op = 1;
			data->p_op_end_token = i;
			data->op.needs_alt_args = 1;
			break;
		}
	}

	t_start = 0;
	if (data->has_p_op) {
		get_opcode_data_from_tokens(&data->p_op, 0, data->p_op_end_token, &tokens);
		t_start = data->p_op_end_token + 1;
	}

	if (!get_opcode_data_from_tokens(&data->op, t_start,
				tokens.token_cnt, &tokens)) {
		printf("Failed to get opcode data from tokens!\n");
		ret = 0;
		goto exit;
	}

	/*
	 * If the op string starts with "OP_0x", then the op has been
	 * explicitly defined in the assembly string. Otherwise, search the
	 * op_info structure and attempt to find a match to the op string.
	 */
	if (!strncmp(data->op.op_str, "OP_0x", 4))
		extract_opcode_from_spec_op_str(data);
	else
		find_compatible_asm_opcode(data);

	/* Found a valid op_info struct, create op. */
	if (data->op.matched) {
		if (data->has_p_op && !data->p_op.matched) {
			printf("Failed to find a matching p_op, aborting.\n");
			ret = 0;
			goto exit;
		}

		create_op_words(data, buf);
	} else {
		printf("Failed to find a match for op %s!\n", data->op.op_str);
		ret = 0;
	}

exit:
	/* Free tokens. */
	for (i = 0; i < tokens.token_cnt; i++)
		free(tokens.token[i]);

	return ret;
}
