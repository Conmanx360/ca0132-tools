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
				     "CR_DSP_ID", "CR_B1_13", "CR_B1_14", "CR_B1_15",
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
	{ .op_str = "MOVX_P",
	  .op = 0x10,
	  .layout_id = { P_OP_LAYOUT_MOVX_DUAL_READ_2 },
	  .mdfr_bit = 12,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	  .alt_op_str = "MOVX_T1_P",
	},
	{ .op_str = "MOVX_T1_P",
	  .op = 0x20,
	  .layout_id = { P_OP_LAYOUT_MOVX_DUAL_WRITE_2 },
	  .mdfr_bit = 20,
	  .alt_op_str = "MOVX_P",
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "MOV_T1_P",
	  .op = 0x30,
	  .layout_id = { P_OP_LAYOUT_MOV_2 },
	  .mdfr_bit = 16,
	  .alt_op_str = "MOV_P",
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	},
	{ .op_str = "MOVX_P",
	  .op = 0x32,
	  .layout_id = { P_OP_LAYOUT_MOVX_2 },
	},
	{ .op_str = "MOVX_P",
	  .op = 0x33,
	  .layout_id = { P_OP_LAYOUT_MOVX_2 },
	},
	{ .op_str = "MOVX_T1_P",
	  .op = 0x34,
	  .layout_id = { P_OP_LAYOUT_MOVX_2 },
	},
	{ .op_str = "MOVX_T1_P",
	  .op = 0x35,
	  .layout_id = { P_OP_LAYOUT_MOVX_2 },
	},
	{ .op_str = "MOVX_P",
	  .op = 0x3a,
	  .layout_id = { P_OP_LAYOUT_MOVX_2 },
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOVX_P",
	  .op = 0x3b,
	  .layout_id = { P_OP_LAYOUT_MOVX_2 },
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOVX_T1_P",
	  .op = 0x3c,
	  .layout_id = { P_OP_LAYOUT_MOVX_2 },
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOVX_T1_P",
	  .op = 0x3d,
	  .layout_id = { P_OP_LAYOUT_MOVX_2 },
	  .src_dst_swap = 1,
	},
	{ .op_str = "EXEC_COND_P",
	  .op = 0x3f,
	  .layout_id = { P_OP_LAYOUT_EXECUTE_COND_2 },
	},
};

static const dsp_op_info parallel_4_asm_ops[] = {
	{ .op_str = "MOV_P",
	  .op = 0x20,
	  .layout_id = { P_OP_LAYOUT_MOV_DUAL_4 },
	  .src_dst_swap = 0,
	},
	/* 0x00c9 move behavior. */
	{ .op_str = "MOVX_T1_P",
	  .op = 0x24,
	  .layout_id = { P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4 },
	  .src_dst_swap = 0,
	},
	{ .op_str = "MOVX_T1_P",
	  .op = 0x25,
	  .layout_id = { P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4 },
	  .src_dst_swap = 0,
	},
	/* 0x00c8 move behavior. */
	{ .op_str = "MOVX_P",
	  .op = 0x26,
	  .layout_id = { P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4 },
	  .src_dst_swap = 0,
	},
	{ .op_str = "MOVX_P",
	  .op = 0x27,
	  .layout_id = { P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4 },
	  .src_dst_swap = 0,
	},
	/* No increment. */
	{ .op_str = "MOVX_T1_P",
	  .op = 0x28,
	  .layout_id = { P_OP_LAYOUT_MOVX_DUAL_4 },
	  .src_dst_swap = 0,
	},
	/* Increment, but 0x00c9 type reg read. */
	{ .op_str = "MOVX_T1_P",
	  .op = 0x29,
	  .layout_id = { P_OP_LAYOUT_MOVX_DUAL_4 },
	  .src_dst_swap = 0,
	},
	/* Increment, with regular 0x00c8 reg read. */
	{ .op_str = "MOVX_P",
	  .op = 0x2b,
	  .layout_id = { P_OP_LAYOUT_MOVX_DUAL_4 },
	  .src_dst_swap = 0,
	},
	/* 0x00c9 move behavior. */
	{ .op_str = "MOVX_T1_P",
	  .op = 0x2c,
	  .layout_id = { P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4 },
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOVX_T1_P",
	  .op = 0x2d,
	  .layout_id = { P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4 },
	  .src_dst_swap = 1,
	},
	/* 0x00c8 move behavior. */
	{ .op_str = "MOVX_P",
	  .op = 0x2e,
	  .layout_id = { P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4 },
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOVX_P",
	  .op = 0x2f,
	  .layout_id = { P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4 },
	  .src_dst_swap = 1,
	},
	/* 0x00c9 type read, with no inc and increment. */
	{ .op_str = "MOVX_T1_P",
	  .op = 0x30,
	  .layout_id = { P_OP_LAYOUT_MOVX_DUAL_4 },
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOVX_T1_P",
	  .op = 0x31,
	  .layout_id = { P_OP_LAYOUT_MOVX_DUAL_4 },
	  .src_dst_swap = 1,
	},
	/* Regular reg read, with no inc and increment. */
	{ .op_str = "MOVX_P",
	  .op = 0x32,
	  .layout_id = { P_OP_LAYOUT_MOVX_DUAL_4 },
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOVX_P",
	  .op = 0x33,
	  .layout_id = { P_OP_LAYOUT_MOVX_DUAL_4 },
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOV_T1_P",
	  .op = 0x38,
	  .layout_id = { P_OP_LAYOUT_MOV_4 },
	  .src_dst_swap = 0,
	  .mdfr_bit = 17,
	  .mdfr_bit_type = OP_MDFR_BIT_TYPE_USE_ALT_STR,
	  .alt_op_str = "MOV_P",
	},
	/* Regular 0x00c8 reg read. */
	{ .op_str = "MOVX_P",
	  .op = 0x39,
	  .layout_id = { P_OP_LAYOUT_MOVX_4 },
	  .src_dst_swap = 0,
	},
	/* 0x00c9 type read. */
	{ .op_str = "MOVX_T1_P",
	  .op = 0x3a,
	  .layout_id = { P_OP_LAYOUT_MOVX_4 },
	  .src_dst_swap = 0,
	},
	/* Regular 0x00c8 reg read. */
	{ .op_str = "MOVX_P",
	  .op = 0x3d,
	  .layout_id = { P_OP_LAYOUT_MOVX_4 },
	  .src_dst_swap = 1,
	},
	/* 0x00c9 type read. */
	{ .op_str = "MOVX_T1_P",
	  .op = 0x3e,
	  .layout_id = { P_OP_LAYOUT_MOVX_4 },
	  .src_dst_swap = 1,
	},
	/* Conditional execution. */
	{ .op_str = "EXEC_COND_P",
	  .op = 0x3f,
	  .layout_id = { P_OP_LAYOUT_EXECUTE_COND_4 },
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
	 * OP_LAYOUT_MOVX_MDFR_OFFSET_1:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 21,
		       .part1_bits      = 1 },
	       .layout_val  = 0x01,
	       .operand_cnt = 2,
	       .operand_loc = {
		   { .part1_bit_start = 16,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 13,
		     .part1_bits      = 3,
		     .part2_bit_start = 22,
		     .part2_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_Y_MDFR_OFFSET,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
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
		   { .part1_bit_start = 13,
		     .part1_bits      = 3,
		     .part2_bit_start = 22,
		     .part2_bits      = 3,
		     .operand_type    = OP_OPERAND_A_REG_X_MDFR_OFFSET,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 2, },
	/*
	 * OP_LAYOUT_MOVX_LIT_OFFSET_1:
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
		   { .part1_bit_start = 14,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 9,
		     .part1_bits      = 5,
		     .operand_type    = OP_OPERAND_C_STK_5,
		     .operand_dir     = OPERAND_DIR_SRC, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } } },
	  .loc_layout_cnt = 1, },
	/*
	 * OP_LAYOUT_INTERRUPT_CLR_1:
	 */
	{ .loc_layouts = {
	     { .layout_val_loc = {
		       .part1_bit_start = 0,
		       .part1_bits      = 0 },
	       .layout_val  = 0x00,
	       .operand_cnt = 1,
	       .operand_loc = {
		   { .part1_bit_start = 17,
		     .part1_bits      = 8,
		     .operand_type    = OP_OPERAND_LITERAL_8,
		     .operand_dir     = OPERAND_DIR_NONE, } },
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
		   { .part1_bit_start = 21,
		     .part1_bits      = 1,
		     .part2_bit_start = 41,
		     .part2_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5_MOVX,
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
		   { .part1_bit_start = 21,
		     .part1_bits      = 1,
		     .part2_bit_start = 41,
		     .part2_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5_MOVX,
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
		   { .part1_bit_start = 30,
		     .part1_bits      = 11,
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
		     .part2_bit_start = 27,
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
		   { .part1_bit_start = 21,
		     .part1_bits      = 1,
		     .part2_bit_start = 41,
		     .part2_bits      = 5,
		     .operand_type    = OP_OPERAND_REG_5_MOVX,
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
		     .operand_type    = OP_OPERAND_C_STK_11,
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
		       .part1_bit_start = 77,
		       .part1_bits      = 1 },
	       .layout_val  = 0x01,
	       .operand_cnt = 3,
	       .operand_loc = {
		   { .part1_bit_start = 78,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_DST, },
		   { .part1_bit_start = 89,
		     .part1_bits      = 11,
		     .operand_type    = OP_OPERAND_REG_11,
		     .operand_dir     = OPERAND_DIR_X, },
		   { .part1_bit_start = 42,
		     .part1_bits      = 32,
		     .operand_type    = OP_OPERAND_LITERAL_32,
		     .operand_dir     = OPERAND_DIR_Y, } },
	       .bitmask = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, } },
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
	  .loc_layout_cnt = 2, },
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

uint32_t get_op_layout_id(const dsp_op_info *info, uint32_t len)
{
	if (len == 4)
		return info->layout_id[OP_LAYOUT_LEN_4];
	else if (len == 2)
		return info->layout_id[OP_LAYOUT_LEN_2];

	return info->layout_id[OP_LAYOUT_LEN_1];
}

static const dsp_op_info asm_ops[] = {
	{ .op_str = "NOP",
	  .op = 0x0000,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NOP,
	                 OP_LAYOUT_NOP,
	                 OP_LAYOUT_NOP },
	},
	{ .op_str = "JMP",
	  .op = 0x0001,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_PC_LIT_16_2,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "JMPC",
	  .op = 0x0002,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_PC_LIT_16_2,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "JMPC_T1",
	  .op = 0x0003,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_PC_LIT_16_2,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "CALL",
	  .op = 0x0004,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_PC_LIT_16_2,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "CALL_T1",
	  .op = 0x0005,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_PC_LIT_16_2,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "CALL_T2",
	  .op = 0x0006,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_PC_LIT_16_2,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "RET",
	  .op = 0x0007,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NOP,
	                 OP_LAYOUT_NOP,
	                 OP_LAYOUT_NOP },
	},
	{ .op_str = "UNK-0x008",
	  .op = 0x0008,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x009",
	  .op = 0x0009,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x00a",
	  .op = 0x000a,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x00b",
	  .op = 0x000b,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "JMP",
	  .op = 0x000c,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_PC_SET_REG_1,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x00d",
	  .op = 0x000d,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x00e",
	  .op = 0x000e,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "CALL",
	  .op = 0x000f,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_PC_SET_REG_1,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "S_JMP",
	  .op = 0x0010,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_PC_OFFSET_1,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "S_JMPC",
	  .op = 0x0011,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_PC_OFFSET_1,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "S_JMPC_T1",
	  .op = 0x0012,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_PC_OFFSET_1,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "S_CALL",
	  .op = 0x0013,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_PC_OFFSET_1,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "S_CALL_T1",
	  .op = 0x0014,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_PC_OFFSET_1,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "S_CALL_T2",
	  .op = 0x0015,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_PC_OFFSET_1,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "RETI",
	  .op = 0x0016,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NOP,
	                 OP_LAYOUT_NOP,
	                 OP_LAYOUT_NOP },
	},
	{ .op_str = "UNK-0x017",
	  .op = 0x0017,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x18",
	  .op = 0x0018,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_LOOP_PC_LIT_16_2,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "LOOP-0x19",
	  .op = 0x0019,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_LOOP_PC_LIT_16_2,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "LOOP-0x10d",
	  .op = 0x001a,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_LOOP_PC_LIT_16_2,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "LOOP-0x10d",
	  .op = 0x001b,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "LOOP-0x01c",
	  .op = 0x001c,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_LOOP_PC_OFFSET_LIT_8_CNT_1,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "LOOP-0x01d",
	  .op = 0x001d,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_LOOP_PC_OFFSET_REG_CNT_1,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x01e",
	  .op = 0x001e,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x01f",
	  .op = 0x001f,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x020",
	  .op = 0x0020,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x021",
	  .op = 0x0021,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "INT_CLR",
	  .op = 0x0022,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_INTERRUPT_CLR_1,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "HALT",
	  .op = 0x0023,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NOP,
	                 OP_LAYOUT_NOP,
	                 OP_LAYOUT_NOP },
	},
	{ .op_str = "INT_ENABLE",
	  .op = 0x0024,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NOP,
	                 OP_LAYOUT_NOP,
	                 OP_LAYOUT_NOP },
	},
	{ .op_str = "INT_DISABLE",
	  .op = 0x0025,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NOP,
	                 OP_LAYOUT_NOP,
	                 OP_LAYOUT_NOP },
	},
	{ .op_str = "UNK-0x026",
	  .op = 0x0026,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x027",
	  .op = 0x0027,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x028",
	  .op = 0x0028,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x029",
	  .op = 0x0029,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x02a",
	  .op = 0x002a,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x02b",
	  .op = 0x002b,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x02c",
	  .op = 0x002c,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x02d",
	  .op = 0x002d,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x02e",
	  .op = 0x002e,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "UNK-0x02f",
	  .op = 0x002f,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "MOVX_T1",
	  .op = 0x0030,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOVX_MDFR_OFFSET_1,
	                 OP_LAYOUT_MOVX_MDFR_OFFSET_2,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "MOVX_T1",
	  .op = 0x0031,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOVX_MDFR_OFFSET_1,
	                 OP_LAYOUT_MOVX_MDFR_OFFSET_2,
	                 OP_LAYOUT_NONE },
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOVX",
	  .op = 0x0032,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOVX_MDFR_OFFSET_1,
	                 OP_LAYOUT_MOVX_MDFR_OFFSET_2,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "MOVX",
	  .op = 0x0033,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOVX_MDFR_OFFSET_1,
	                 OP_LAYOUT_MOVX_MDFR_OFFSET_2,
	                 OP_LAYOUT_NONE },
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOVX_T1",
	  .op = 0x0034,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOVX_LIT_OFFSET_1,
	                 OP_LAYOUT_MOVX_LIT_OFFSET_2,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "MOVX_T1",
	  .op = 0x0035,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOVX_LIT_OFFSET_1,
	                 OP_LAYOUT_MOVX_LIT_OFFSET_2,
	                 OP_LAYOUT_NONE },
	  .src_dst_swap = 1,
	},
	{ .op_str = "MOVX",
	  .op = 0x0036,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOVX_LIT_OFFSET_1,
	                 OP_LAYOUT_MOVX_LIT_OFFSET_2,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "MOVX",
	  .op = 0x0037,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOVX_LIT_OFFSET_1,
	                 OP_LAYOUT_MOVX_LIT_OFFSET_2,
	                 OP_LAYOUT_NONE },
	  .src_dst_swap = 1,
	},
	{ .op_str = "LMA_S",
	  .op = 0x0040,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "LMA_AC_MV_S",
	  .op = 0x0041,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "LMA_AC_S",
	  .op = 0x0042,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "LNMA_LMA_S",
	  .op = 0x0043,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "LNMA_S",
	  .op = 0x0044,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "LMS_AC_MV_S",
	  .op = 0x0045,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "LMS_S",
	  .op = 0x0046,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "LMA_O",
	  .op = 0x0047,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "LMA_AC_MV_O",
	  .op = 0x0048,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "LMA_AC_O",
	  .op = 0x0049,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "LNMA_LMA_O",
	  .op = 0x004a,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "LNMA_O",
	  .op = 0x004b,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "U_ADD_S",
	  .op = 0x004c,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "LMS_O",
	  .op = 0x004d,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "U_ADD3_S",
	  .op = 0x004e,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "U_SUB_S",
	  .op = 0x004f,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "LMUL_S",
	  .op = 0x0050,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "U_MOV_ACC",
	  .op = 0x0051,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "U_ADD_O",
	  .op = 0x0052,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "U_ADDC_O",
	  .op = 0x0053,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "U_SUB_O",
	  .op = 0x0054,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "U_SUBB_O",
	  .op = 0x0055,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "U_ADDSUB",
	  .op = 0x0056,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "U_SUBADD",
	  .op = 0x0057,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "U_MOV_ACC",
	  .op = 0x0058,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	  .src_mdfr = { OPERAND_MDFR_INC },
	},
	{ .op_str = "U_MOV_ACC",
	  .op = 0x0059,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	  .src_mdfr = { OPERAND_MDFR_DEC },
	},
	{ .op_str = "LMUL_O",
	  .op = 0x005a,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "LNMUL_O",
	  .op = 0x005b,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "LMUL_US_O",
	  .op = 0x005c,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "LMUL_US_O_T1",
	  .op = 0x005d,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "INTERP",
	  .op = 0x005e,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "INTERP_T1",
	  .op = 0x005f,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "MA_S",
	  .op = 0x0060,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "MA_C_S",
	  .op = 0x0061,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "MA_AC_MV_S",
	  .op = 0x0062,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "MA_AC_S",
	  .op = 0x0063,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "NMA_MA_S",
	  .op = 0x0064,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "NMA_S",
	  .op = 0x0065,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "MS_S",
	  .op = 0x0066,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "MA_O",
	  .op = 0x0067,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "MA_C_O",
	  .op = 0x0068,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "MA_AC_MV_O",
	  .op = 0x0069,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "MA_AC_O",
	  .op = 0x006a,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "NMA_MA_O",
	  .op = 0x006b,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "NMA_O",
	  .op = 0x006c,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "MS_O",
	  .op = 0x006d,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "ADD_S",
	  .op = 0x006e,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "ADDC_S",
	  .op = 0x006f,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "SUB_S",
	  .op = 0x0070,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "SUBB_S",
	  .op = 0x0071,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "ADD_O",
	  .op = 0x0072,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "ADDC_O",
	  .op = 0x0073,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "SUB_O",
	  .op = 0x0074,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "SUBB_O",
	  .op = 0x0075,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "MOV",
	  .op = 0x0076,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	  .src_mdfr = { OPERAND_MDFR_INC },
	},
	{ .op_str = "MOV",
	  .op = 0x0077,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	  .src_mdfr = { OPERAND_MDFR_DEC },
	},
	{ .op_str = "MOV",
	  .op = 0x0078,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	  .src_mdfr = { OPERAND_MDFR_RL },
	},
	{ .op_str = "MOV",
	  .op = 0x0079,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	  .src_mdfr = { OPERAND_MDFR_RR },
	},
	{ .op_str = "MUL_O",
	  .op = 0x007a,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	/* Result is -(x * y); */
	{ .op_str = "NMUL_O",
	  .op = 0x007b,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "MUL_US",
	  .op = 0x007c,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "MUL_US_T1",
	  .op = 0x007d,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "MOV_ACC_S",
	  .op = 0x007e,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "U_MOV_ACC_S",
	  .op = 0x007f,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "F_MA",
	  .op = 0x0080,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "F_MA_AC_MV",
	  .op = 0x0081,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "F_MA_AC",
	  .op = 0x0082,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "F_MA_NMA",
	  .op = 0x0083,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "F_NMA_MA",
	  .op = 0x0084,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "F_NMA",
	  .op = 0x0085,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "F_MA_MS_AC_MV",
	  .op = 0x0086,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "F_MS",
	  .op = 0x0087,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "F_ADD",
	  .op = 0x0088,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "F_SUB",
	  .op = 0x0089,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "F_SUBADD",
	  .op = 0x008a,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "F_ADDSUB",
	  .op = 0x008b,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "F_MUL",
	  .op = 0x008c,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "F_NMUL",
	  .op = 0x008d,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "F_CMP",
	  .op = 0x008e,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "F_INV",
	  .op = 0x008f,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "F_ABS",
	  .op = 0x0090,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "F_LIMIT",
	  .op = 0x0091,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "F_LIMIT_T1",
	  .op = 0x0092,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "F_ADD_EXP",
	  .op = 0x0093,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "F_GET_SIG",
	  .op = 0x0094,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "F_GET_EXP",
	  .op = 0x0095,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "F_TO_I",
	  .op = 0x0096,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "I_TO_F",
	  .op = 0x0097,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "UNK-0x098",
	  .op = 0x0098,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "F_SINCOS",
	  .op = 0x0099,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "F_ARCTAN",
	  .op = 0x009a,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "F_RCP_DIV",
	  .op = 0x009b,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "F_RCP",
	  .op = 0x009c,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "F_RCP_SQRT",
	  .op = 0x009d,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "EXP",
	  .op = 0x009e,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "LOG",
	  .op = 0x009f,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "F_DFT_TWDL",
	  .op = 0x00a0,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "F_DFT_BFLY",
	  .op = 0x00a1,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0a2",
	  .op = 0x00a2,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0a3",
	  .op = 0x00a3,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0a4",
	  .op = 0x00a4,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0a5",
	  .op = 0x00a5,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0a6",
	  .op = 0x00a6,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0a7",
	  .op = 0x00a7,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0a8",
	  .op = 0x00a8,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0a9",
	  .op = 0x00a9,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0aa",
	  .op = 0x00aa,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0ab",
	  .op = 0x00ab,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0ac",
	  .op = 0x00ac,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0ad",
	  .op = 0x00ad,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0ae",
	  .op = 0x00ae,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0af",
	  .op = 0x00af,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "ADD",
	  .op = 0x00b0,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_LIT_8_Y_1,
	                 OP_LAYOUT_R_X_LIT_16_Y_2,
	                 OP_LAYOUT_R_X_LIT_32_Y_4 },
	  .src_mdfr = { OPERAND_MDFR_16_BIT_SIGNED },
	},
	/* Might not even be valid. */
	{ .op_str = "UNK-0x0b1",
	  .op = 0x00b1,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "MOV_T1",
	  .op = 0x00c0,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_MOV_LIT_16_2,
	                 OP_LAYOUT_MOV_LIT_32_4 },
	},
	{ .op_str = "MOV",
	  .op = 0x00c1,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_MOV_LIT_16_2,
	                 OP_LAYOUT_MOV_LIT_32_4 },
	},
	{ .op_str = "MOV_T2",
	  .op = 0x00c2,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_MOV_LIT_16_2,
	                 OP_LAYOUT_MOV_LIT_32_4 },
	},
	{ .op_str = "MOV_L_T1",
	  .op = 0x00c3,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_MOV_LIT_16_2,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "MOV_L",
	  .op = 0x00c4,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_MOV_LIT_16_2,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "MOV_L_T2",
	  .op = 0x00c5,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_MOV_LIT_16_2,
	                 OP_LAYOUT_MOV_LIT_32_4 },
	},
	/*
	 * 0xc6-0xc7 seem broken, they're supposed to be upper 16-bit literal
	 * set, but the literal value seems to overlap the register operand
	 * value.
	 */
	{ .op_str = "MOV_U_T1",
	  .op = 0x00c6,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_MOV_LIT_16_2,
	                 OP_LAYOUT_MOV_LIT_32_4 },
	  .src_mdfr = { OPERAND_MDFR_16_BIT_UPPER },
	},
	{ .op_str = "MOV_U",
	  .op = 0x00c7,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_NONE,
	                 OP_LAYOUT_MOV_LIT_16_2,
	                 OP_LAYOUT_MOV_LIT_32_4 },
	  .src_mdfr = { OPERAND_MDFR_16_BIT_UPPER },
	},
	{ .op_str = "MOV",
	  .op = 0x00c8,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	/*
	 * Reads from the upper 32-bits of R04/R05/R12/R13.
	 */
	{ .op_str = "MOV_T1",
	  .op = 0x00c9,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	/*
	 * Reads from the highest 8-bits of R04/R05/R12/R13.
	 */
	{ .op_str = "MOV_T2",
	  .op = 0x00ca,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "UNK-0x0cb",
	  .op = 0x00cb,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "XOR",
	  .op = 0x00cc,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "OR",
	  .op = 0x00cd,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "AND",
	  .op = 0x00ce,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "I_ABS_FFS",
	  .op = 0x00cf,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "I_INV",
	  .op = 0x00d0,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "CMPL",
	  .op = 0x00d1,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "I_ABS",
	  .op = 0x00d2,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "I_CMP",
	  .op = 0x00d3,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "I_CMP",
	  .op = 0x00d4,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_LIT_8_Y_1,
	                 OP_LAYOUT_R_X_LIT_16_Y_2,
	                 OP_LAYOUT_R_X_LIT_32_Y_4 },
	},
	{ .op_str = "LIMIT",
	  .op = 0x00d5,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "LIMIT_T1",
	  .op = 0x00d6,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "TST_NEG",
	  .op = 0x00d7,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0d8",
	  .op = 0x00d8,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0d9",
	  .op = 0x00d9,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0da",
	  .op = 0x00da,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0db",
	  .op = 0x00db,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "UNK-0x0dc",
	  .op = 0x00dc,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0dd",
	  .op = 0x00dd,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "UNK-0x0de",
	  .op = 0x00de,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "UNK-0x0df",
	  .op = 0x00df,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "FFS",
	  .op = 0x00e0,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "FFU",
	  .op = 0x00e1,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_MOV_1,
	                 OP_LAYOUT_MOV_2,
	                 OP_LAYOUT_MOV_4 },
	},
	{ .op_str = "RO_L",
	  .op = 0x00e2,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "RO_R",
	  .op = 0x00e3,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "RO_L",
	  .op = 0x00e4,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_LIT_8_Y_1,
	                 OP_LAYOUT_R_X_LIT_16_Y_2,
	                 OP_LAYOUT_R_X_LIT_32_Y_4 },
	},
	{ .op_str = "RO_R",
	  .op = 0x00e5,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_LIT_8_Y_1,
	                 OP_LAYOUT_R_X_LIT_16_Y_2,
	                 OP_LAYOUT_R_X_LIT_32_Y_4 },
	},
	{ .op_str = "SH_L",
	  .op = 0x00e6,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "SH_R",
	  .op = 0x00e7,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "SH_L",
	  .op = 0x00e8,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_LIT_8_Y_1,
	                 OP_LAYOUT_R_X_LIT_16_Y_2,
	                 OP_LAYOUT_R_X_LIT_32_Y_4 },
	},
	{ .op_str = "SH_R",
	  .op = 0x00e9,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_LIT_8_Y_1,
	                 OP_LAYOUT_R_X_LIT_16_Y_2,
	                 OP_LAYOUT_R_X_LIT_32_Y_4 },
	},
	{ .op_str = "A_SH_L",
	  .op = 0x00ea,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "A_SH_R",
	  .op = 0x00eb,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "A_SH_L",
	  .op = 0x00ec,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_LIT_8_Y_1,
	                 OP_LAYOUT_R_X_LIT_16_Y_2,
	                 OP_LAYOUT_R_X_LIT_32_Y_4 },
	},
	{ .op_str = "A_SH_R",
	  .op = 0x00ed,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_LIT_8_Y_1,
	                 OP_LAYOUT_R_X_LIT_16_Y_2,
	                 OP_LAYOUT_R_X_LIT_32_Y_4 },
	},
	/*
	 * These instructions perform a shift right but don't set their
	 * destination registers, instead only setting the COND_CODE register.
	 */
	{ .op_str = "SH_R_CHK",
	  .op = 0x00ee,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "SH_R_CHK",
	  .op = 0x00ef,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_LIT_8_Y_1,
	                 OP_LAYOUT_R_X_LIT_16_Y_2,
	                 OP_LAYOUT_R_X_LIT_32_Y_4 },
	},
	/*
	 * These two behave like SET_BIT/CLR_BIT respectively, but instead
	 * they seem to set the SEMAPHORE_G register. They always end up
	 * clearing the r register. Not sure how it's supposed to be used.
	 * Maybe the intention is to always have r be R04, so it just clears
	 * the ACC.
	 */
	{ .op_str = "SET_SEM_G_BIT",
	  .op = 0x00f0,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_LIT_8_Y_1,
	                 OP_LAYOUT_R_X_LIT_16_Y_2,
	                 OP_LAYOUT_R_X_LIT_32_Y_4 },
	},
	{ .op_str = "CLR_SEM_G_BIT",
	  .op = 0x00f1,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_LIT_8_Y_1,
	                 OP_LAYOUT_R_X_LIT_16_Y_2,
	                 OP_LAYOUT_R_X_LIT_32_Y_4 },
	},
	{ .op_str = "SET_BIT",
	  .op = 0x00f2,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "SET_BIT",
	  .op = 0x00f3,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_LIT_8_Y_1,
	                 OP_LAYOUT_R_X_LIT_16_Y_2,
	                 OP_LAYOUT_R_X_LIT_32_Y_4 },
	},
	{ .op_str = "CLR_BIT",
	  .op = 0x00f4,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "CLR_BIT",
	  .op = 0x00f5,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_LIT_8_Y_1,
	                 OP_LAYOUT_R_X_LIT_16_Y_2,
	                 OP_LAYOUT_R_X_LIT_32_Y_4 },
	},
	{ .op_str = "TGL_BIT",
	  .op = 0x00f6,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "TGL_BIT",
	  .op = 0x00f7,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_LIT_8_Y_1,
	                 OP_LAYOUT_R_X_LIT_16_Y_2,
	                 OP_LAYOUT_R_X_LIT_32_Y_4 },
	},
	{ .op_str = "SET_BITS",
	  .op = 0x00f8,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "SET_BITS_T1",
	  .op = 0x00f9,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "SET_BITS_T2",
	  .op = 0x00fa,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "GET_BITS",
	  .op = 0x00fb,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "UNK-0x0fc",
	  .op = 0x00fc,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_A_1,
	                 OP_LAYOUT_R_X_Y_A_2,
	                 OP_LAYOUT_R_X_Y_A_4 },
	},
	{ .op_str = "UNK-0x0fd",
	  .op = 0x00fd,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_R_X_Y_1,
	                 OP_LAYOUT_R_X_Y_2,
	                 OP_LAYOUT_R_X_Y_4 },
	},
	{ .op_str = "POP",
	  .op = 0x00fe,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_STACK_UNK_1,
	                 OP_LAYOUT_STACK_UNK_2,
	                 OP_LAYOUT_NONE },
	},
	{ .op_str = "PUSH",
	  .op = 0x00ff,
	  .has_op_layout = 1,
	  .layout_id = { OP_LAYOUT_STACK_UNK_1,
	                 OP_LAYOUT_STACK_UNK_2,
	                 OP_LAYOUT_NONE },
	  .src_dst_swap = 1,
	},
};

uint32_t get_dsp_op_len(uint32_t op)
{
	uint32_t len;

	/* If MSB (for DSP program word) is set, it'll be at least length two. */
	if (op & 0x01000000) {
		/* If bit below MSB is set, we're at length 4. */
		if (op & 0x00800000)
			len = 4;
		else
			len = 2;
	} else {
		len = 1;
	}

	return len;
}

static uint32_t create_dsp_opcode(uint32_t op, uint32_t len)
{
	uint32_t opcode;

	if (len > 1) {
		opcode = 0x01000000 | (op << 15);
		if (len == 4)
			opcode |= 0x00800000;
	} else {
		opcode = op << 16;
	}

	return opcode;
}

static uint32_t get_op_len_from_len_id(uint32_t len_id)
{
	if (len_id == OP_LAYOUT_LEN_4)
		return 4;
	else if (len_id == OP_LAYOUT_LEN_2)
		return 2;

	return 1;
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
		else if ((operand->val & 0xff) > 5)
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
		i_tmp = (operand->val & 0x7ff) - (tmp_op->val & 0x7ff);
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

	case OP_OPERAND_REG_5_MOVX:
		if ((operand->val & 0xff) > 0x0f)
			operand->val = (operand->val & 0x1f) | 0x20;

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

	len = data->op_len;
	src_dst_swap = op->src_dst_swap;
	loc_layout = op->loc_layout;

	memset(buf, 0, sizeof(buf));
	buf[0] = op->opcode;
	if (op->use_op_mdfr_bit)
		set_bits_in_op_words(buf, op->mdfr_bit, 1, 1);


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
		buf[0] |= p_op->opcode;
		loc_layout = data->p_op.loc_layout;
		operand_cnt = loc_layout->operand_cnt;
		loc_descriptors = loc_layout->operand_loc;
		src_dst_swap = p_op->src_dst_swap;

		if (p_op->use_op_mdfr_bit)
			set_bits_in_op_words(buf, p_op->mdfr_bit, 1, 1);

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

	case OP_OPERAND_REG_5_MOVX:
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
		if (operand->val > tmp_op->val)
			i_tmp = operand->val - tmp_op->val;
		else
			i_tmp = tmp_op->val - operand->val;

		if (i_tmp < 0x200)
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

	case OP_OPERAND_C_STK_5:
		if (operand->type != OPERAND_TYPE_STACK)
			break;

		if (operand->val < 0x10)
			ret = 1;

		break;

	case OP_OPERAND_C_STK_11:
		if (operand->type != OPERAND_TYPE_STACK)
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
		dsp_asm_op_data *data, uint32_t layout_id, uint32_t is_p_op)
{
	uint32_t src_mdfr, src_dst_swap;
	const op_operand_layout *layout;

	/* No layout for this length, so obviously incompatible. */
	if (layout_id == OP_LAYOUT_NONE)
		return 0;

	/*
	 * If we have no operands and the layout id is NOP, we're compatible.
	 * Single length was ruled out earlier.
	 */
	if (!data->operand_cnt && (layout_id == OP_LAYOUT_NOP) && !is_p_op) {
		layout = get_op_layout(layout_id);
		data->loc_layout = &layout->loc_layouts[0];
		return 1;
	}

	if (is_p_op)
		layout = get_p_op_layout(layout_id);
	else
		layout = get_op_layout(layout_id);

	/*
	 * Now we check if any of this lengths operand layouts are compatible.
	 */
	src_dst_swap = info->src_dst_swap;
	src_mdfr = info->src_mdfr[0];
	if (check_op_layout_operand_compatibility(layout, data, src_dst_swap, src_mdfr))
		return 1;

	return 0;
}

/*
 * Find a parallel op_info structure to match our assembly info.
 */
static const dsp_op_info *find_compatible_p_op_info(dsp_asm_data *data,
		const dsp_op_info *start, uint8_t op_len)
{
	const dsp_op_info *op_info;
	uint8_t alt_str_match;
	uint32_t layout_id;

	op_info = start;
	op_info = find_dsp_p_op(data->p_op.op_str, op_len, op_info, &alt_str_match);
	while (op_info) {
		if (alt_str_match)
			data->p_op.use_op_mdfr_bit = 1;
		else
			data->p_op.use_op_mdfr_bit = 0;

		layout_id = op_info->layout_id[0];
		/* If we've found a p_op that matches, return. */
		if (check_op_layout_compatibility(op_info, &data->p_op, layout_id, 1))
			return op_info;

		op_info = find_dsp_p_op(data->p_op.op_str, op_len, op_info, &alt_str_match);
	}

	return NULL;
}

static void set_asm_op_data_from_op_info(dsp_asm_op_data *data, const dsp_op_info *op_info,
		uint32_t len)
{
	/* If len is 0, it's a p_op. This should be removed in later patches. */
	if (len)
		data->opcode = create_dsp_opcode(op_info->op, len);
	else
		data->opcode = op_info->op;

	data->src_dst_swap = op_info->src_dst_swap;
	data->mdfr_bit = op_info->mdfr_bit;
	data->mdfr_bit_type = op_info->mdfr_bit_type;
	data->matched = 1;
}

/*
 * Set a dsp_asm_op_data structure from the data stored in a dsp_asm_p_op_data
 * structure.
 */
static void set_asm_op_data_from_p_op_data(dsp_asm_op_data *data,
		dsp_asm_p_op_data *p_op_data)
{
	/*
	 * Store opcode with bitshift already done, so when we go to set it
	 * all we have to do is use |=.
	 */
	data->opcode = p_op_data->opcode << 9;
	data->loc_layout = p_op_data->loc_layout;
	data->src_dst_swap = p_op_data->src_dst_swap;
	data->mdfr_bit = p_op_data->mdfr_bit;
	if (data->mdfr_bit)
		data->use_op_mdfr_bit = 1;

	data->matched = 1;
}

/*
 * Set a dsp_asm_p_op_data structure with data from the matching op_info
 * structure we found. Once a suitable primary opcode is found, we can pull
 * this info depending on the opcode length.
 */
static void set_asm_p_op_data_from_op_info(dsp_asm_p_op_data *data, const dsp_op_info *info,
		dsp_asm_op_data *p_op)
{
	data->opcode = info->op;
	data->loc_layout = p_op->loc_layout;
	data->src_dst_swap = info->src_dst_swap;
	if (p_op->use_op_mdfr_bit)
		data->mdfr_bit = info->mdfr_bit;
	else
		data->mdfr_bit = 0;

	data->matched = 1;
}

/*
 * Check the op_str for a length suffix, which is ':4', ':2', or ':1'. This
 * helps the assembler produce 1:1 binaries.
 * A length doesn't have to be set, and if it isn't, the assembler will always
 * try to use the smallest op.
 */
static uint32_t check_op_str_for_len(dsp_asm_op_data *op)
{
	uint32_t str_len, op_len;

	str_len = strlen(op->op_str);
	op_len = 0;
	if (op->op_str[str_len - 2] == ':') {
		op_len = strtol(&op->op_str[str_len - 1], NULL, 10);
		op->op_str[str_len - 2] = '\0';
	}

	return op_len;
}

/*
 * Checks which op lengths are possible given the current assembly string.
 * This is done by checking if we need a parallel op, and if we do, which
 * lengths can use this parallel op. Returns a bitmask.
 */
static uint32_t get_compatible_op_len(dsp_asm_data *data)
{
	uint32_t op_len_bitmask, op_len;
	const dsp_op_info *info;

	op_len_bitmask = op_len = 0;
	/*
	 * If we have a parallel op, only lengths two and four are potentially
	 * viable.
	 */
	if (data->has_p_op) {
		/* Check for a compatible length 2 parallel op. */
		info = find_compatible_p_op_info(data, NULL, 2);
		if (info) {
			set_asm_p_op_data_from_op_info(&data->valid_p_ops[0],
					info, &data->p_op);
			op_len_bitmask |= 0x02;
		}

		/* Check for a compatible length 4 parallel op. */
		info = find_compatible_p_op_info(data, NULL, 4);
		if (info) {
			set_asm_p_op_data_from_op_info(&data->valid_p_ops[1],
					info, &data->p_op);
			op_len_bitmask |= 0x04;
		}
	} else {
		/* No parallel op, so all lengths are compatible. */
		op_len_bitmask |= 0x07;
	}

	op_len = check_op_str_for_len(&data->op);
	if (op_len)
		op_len_bitmask = op_len;

	data->p_op.matched = 0;

	return op_len_bitmask;
}

/*
 * Search all asm ops until we find one that matches our given assembly info.
 */
static void find_compatible_asm_opcode(dsp_asm_data *data)
{
	uint32_t len_compat_mask, layout_id, matched, i;
	const dsp_op_info *op_info;
	uint8_t alt_str_match;
	dsp_asm_op_data *op;

	/* Get compatible op_lengths. */
	len_compat_mask = get_compatible_op_len(data);

	op = &data->op;
	op_info = NULL;
	while ((op_info = find_dsp_op(data->op.op_str, op_info, &alt_str_match))) {
		if (alt_str_match)
			data->op.use_op_mdfr_bit = 1;
		else
			data->op.use_op_mdfr_bit = 0;

		/*
		 * Before even checking the layout, check if the src_mdfr is
		 * compatible.
		 */
		if (!check_src_mdfr_compatibility(op->src_mdfr, op_info->src_mdfr[0]))
			continue;

		/* Check each compatible lengths op layout. */
		for (i = matched = 0; i < OP_LAYOUT_LEN_CNT; i++) {
			/* If the current length is incompatible, continue. */
			if (!((1 << i) & len_compat_mask))
				continue;

			layout_id = op_info->layout_id[i];
			if (!check_op_layout_compatibility(op_info, op, layout_id, 0))
				continue;

			/* We have found a compatible opcode. */
			data->op_len = get_op_len_from_len_id(i);

			/* Set p_op data if we have it. */
			if (data->has_p_op && (data->op_len == 2))
				set_asm_op_data_from_p_op_data(&data->p_op,
						&data->valid_p_ops[0]);

			if (data->has_p_op && (data->op_len == 4))
				set_asm_op_data_from_p_op_data(&data->p_op,
						&data->valid_p_ops[1]);

			matched = 1;
			break;
		}

		if (matched)
			break;

	}

	if (op_info)
		set_asm_op_data_from_op_info(&data->op, op_info, data->op_len);
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

	if (!strncmp(operand_str, "C_STK", 5)) {
		operand->type = OPERAND_TYPE_STACK;

		if (!strncmp(&operand_str[5], "_TOP", 4)) {
			operand->val = 0x20;
			/* With modifier bit. */
			if (strlen(operand_str) > 9)
				operand->val |= 0x40;

			return 1;
		}

		if (!strncmp(&operand_str[5], "_BASE +", 7)) {
			operand->val = strtol(&operand_str[12], NULL, 0);
			return 1;
		}

		if (!strncmp(&operand_str[5], "_BASE_MD +", 10)) {
			operand->val = strtol(&operand_str[15], NULL, 0);
			operand->val |= 0x080;
			return 1;
		}
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
	"OP_LAYOUT_MOVX_MDFR_OFFSET_1",
	"OP_LAYOUT_MOVX_LIT_OFFSET_1",
	"OP_LAYOUT_R_X_Y_1",
	"OP_LAYOUT_R_X_Y_A_1",
	"OP_LAYOUT_R_X_LIT_8_Y_1",
	"OP_LAYOUT_PC_OFFSET_1",
	"OP_LAYOUT_LOOP_PC_OFFSET_REG_CNT_1",
	"OP_LAYOUT_LOOP_PC_OFFSET_LIT_8_CNT_1",
	"OP_LAYOUT_PC_SET_REG_1",
	"OP_LAYOUT_STACK_UNK_1",
	"OP_LAYOUT_INTERRUPT_CLR_1",
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
	"OP_LAYOUT_NONE",
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

		if (check_op_layout_compatibility(info, op, info->layout_id[0], 0))
			set_asm_op_data_from_op_info(op, info, get_dsp_op_len(op->opcode << 16));
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
					get_dsp_op_len(op->opcode << 16));
			if (info)
				set_asm_op_data_from_op_info(p_op, info, 0);

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
