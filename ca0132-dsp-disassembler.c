/*
 * ca0132-dsp-disassembler:
 * Disassembles a DSP program file.
 */
#include "ca0132_defs.h"

typedef struct {
	FILE *pmem;

	uint32_t cur_op[4];

	uint32_t cur_addr;
	const char *cur_op_str;

	uint8_t offset_addr_op_set;
	uint16_t offset_addr;
} dsp_main;

static uint32_t get_dsp_op(dsp_main *data, const dsp_op_info **op_info)
{
	uint32_t tmp, len;

	/* Get first op word. */
	if (!fread(&data->cur_op[0], sizeof(uint32_t), 1, data->pmem))
		return 1;

	/* Extract the opcode from it and get it's length.*/
	tmp = (data->cur_op[0] & 0xffff0000) >> 16;
	len = get_dsp_op_len(tmp);

	/* If the op is greater than one word, read the rest of it's data. */
	if (len > 1) {
		if (!fread(&data->cur_op[1], sizeof(uint32_t), len - 1, data->pmem))
			return 1;
	}

	/* Check if we have an op_info structure for this op. */
	*op_info = get_dsp_op_info(tmp);

	/* If we don't, increment the current address. */
	if (!(*op_info)) {
		printf("0x%04x: Unknown op 0x%08x.\n", data->cur_addr,
				data->cur_op[0]);
		data->cur_addr += len;
	}

	return 0;
}

static uint32_t get_op_operand(dsp_main *data,
		const operand_loc_descriptor *loc)
{
	uint32_t operand, tmp;

	operand = get_bits_in_op_words(data->cur_op, loc->part1_bit_start, loc->part1_bits);
	if (loc->part2_bits) {
		tmp = get_bits_in_op_words(data->cur_op, loc->part2_bit_start,
				loc->part2_bits);
		operand = (operand << loc->part2_bits) | tmp;
	}

	return operand;
}

/* Get the string for the operand passed in operand_data and print it. */
static void print_operand_str(dsp_main *data, operand_data *operand, uint8_t final)
{
	const char *reg_str;
	uint32_t tmp0, tmp1;
	int32_t i_tmp;
	char buf[96];

	switch (operand->operand_type) {
	case OP_OPERAND_REG_2:
		tmp0 = (operand->operand_val & 0x03);
		reg_str = get_dsp_operand_str(tmp0);

		break;

	case OP_OPERAND_REG_2_4:
		tmp0 = 4 + (operand->operand_val & 0x03);
		reg_str = get_dsp_operand_str(tmp0);

		break;

	case OP_OPERAND_REG_2_8:
		tmp0 = 8 + (operand->operand_val & 0x03);
		reg_str = get_dsp_operand_str(tmp0);

		break;

	case OP_OPERAND_REG_2_12:
		tmp0 = 12 + (operand->operand_val & 0x03);
		reg_str = get_dsp_operand_str(tmp0);

		break;

	case OP_OPERAND_REG_2_T1:
		if (operand->operand_val & 0x02)
			tmp0 = 8;
		else
			tmp0 = 12;

		tmp0 += (operand->operand_val & 0x01);
		reg_str = get_dsp_operand_str(tmp0);
		break;

	case OP_OPERAND_REG_2_T2:
		if (operand->operand_val & 0x02)
			tmp0 = 0;
		else
			tmp0 = 4;

		tmp0 += (operand->operand_val & 0x01);
		reg_str = get_dsp_operand_str(tmp0);
		break;

	case OP_OPERAND_REG_3_X_T1:
		if (operand->operand_val & 0x04)
			tmp0 = 0;
		else
			tmp0 = 4;

		tmp0 += (operand->operand_val & 0x03);
		reg_str = get_dsp_operand_str(tmp0);

		break;

	case OP_OPERAND_REG_3_Y_T1:
		if (operand->operand_val > 0x05)
			tmp0 = 12 + (operand->operand_val & 0x01);
		else
			tmp0 = operand->operand_val;

		reg_str = get_dsp_operand_str(tmp0);

		break;

	case OP_OPERAND_REG_3_X_T2:
		if (operand->operand_val & 0x04)
			tmp0 = 8;
		else
			tmp0 = 12;

		tmp0 += (operand->operand_val & 0x03);
		reg_str = get_dsp_operand_str(tmp0);

		break;

	case OP_OPERAND_REG_3_Y_T2:
		switch (operand->operand_val) {
		case 0 ... 3:
			tmp0 = 8 + (operand->operand_val & 0x03);
			break;

		case 4 ...  5:
			tmp0 = operand->operand_val;
			break;

		case 6 ... 7:
			tmp0 = 12 + (operand->operand_val & 0x01);
			break;

		default:
			tmp0 = 0;
			break;
		}

		reg_str = get_dsp_operand_str(tmp0);

		break;

	case OP_OPERAND_REG_3_FMA_X_T1:
		if (operand->operand_val > 5)
			tmp0 = 8 + (operand->operand_val & 0x01);
		else
			tmp0 = operand->operand_val;

		reg_str = get_dsp_operand_str(tmp0);

		break;

	case OP_OPERAND_REG_3_FMA_X_T2:
		if (operand->operand_val > 5)
			tmp0 = (operand->operand_val & 0x01);
		else
			tmp0 = 8 + operand->operand_val;

		reg_str = get_dsp_operand_str(tmp0);

		break;

	case OP_OPERAND_REG_3_FMA_A_T1:
		if (operand->operand_val > 4)
			tmp0 = 12 + (operand->operand_val & 0x03);
		else
			tmp0 = 4 + (operand->operand_val & 0x03);

		reg_str = get_dsp_operand_str(tmp0);

		break;

	case OP_OPERAND_REG_3_FMA_Y_T1:
	case OP_OPERAND_REG_3:
		reg_str = get_dsp_operand_str((operand->operand_val & 0x07));
		break;

	case OP_OPERAND_REG_4:
		reg_str = get_dsp_operand_str(operand->operand_val & 0x0f);
		break;

	case OP_OPERAND_REG_5:
		reg_str = get_dsp_operand_str(operand->operand_val & 0x1f);
		break;

	case OP_OPERAND_REG_7:
		reg_str = get_dsp_operand_str(operand->operand_val & 0x7f);
		break;

	case OP_OPERAND_REG_3_FMA_Y_T2:
	case OP_OPERAND_REG_3_8:
		reg_str = get_dsp_operand_str((operand->operand_val & 0x07) + 8);
		break;

	case OP_OPERAND_REG_9:
	case OP_OPERAND_REG_10:
		reg_str = get_dsp_operand_str(operand->operand_val & 0xff);
		break;

	case OP_OPERAND_REG_11:
		if (operand->operand_val & 0x400) {
			if (operand->operand_val & 0x200)
				sprintf(buf, "YGPRAM_%03d", operand->operand_val & 0xff);
			else
				sprintf(buf, "XGPRAM_%03d", operand->operand_val & 0xff);
			reg_str = buf;
		} else {
			reg_str = get_dsp_operand_str(operand->operand_val & 0xff);
		}
		break;
	case OP_OPERAND_REG_11_4_OFFSET:
		tmp0 = operand->operand_val & 0xf;
		tmp1 = (operand->operand_val >> 11) & 0xf;
		tmp1 += tmp0;
		tmp0 = (operand->operand_val & 0x7f0) | (tmp1 & 0xf);

		if ((tmp0 & 0x600) != (operand->operand_val & 0x600)) {
			tmp0 &= 0x1ff;
			tmp0 |= (operand->operand_val & 0x600);
		}

		if (tmp0 & 0x400) {
			if (tmp0 & 0x800)
				sprintf(buf, "YGPRAM_%03d", tmp0 & 0x3ff);
			else
				sprintf(buf, "XGPRAM_%03d", tmp0 & 0x7f);

			reg_str = buf;
		} else {
			sprintf(buf, "%s", get_dsp_operand_str(tmp0 & 0x7f));
			reg_str = buf;
		}
		break;

	case OP_OPERAND_REG_11_10_OFFSET:
		tmp0 = operand->operand_val & 0x7ff;
		tmp1 = (operand->operand_val >> 11) & 0x3ff;
		if (tmp1 & 0x200)
			tmp1 |= 0xfc00;

		tmp0 += (uint16_t)tmp1;


		if ((tmp0 & 0x600) != (operand->operand_val & 0x600)) {
			tmp0 &= 0x1ff;
			tmp0 |= (operand->operand_val & 0x600);
		}


		if (tmp0 & 0x400) {
			if (tmp0 & 0x800)
				sprintf(buf, "YGPRAM_%03d", tmp0 & 0x3ff);
			else
				sprintf(buf, "XGPRAM_%03d", tmp0 & 0x7f);

			reg_str = buf;
		} else {
			reg_str = get_dsp_operand_str(tmp0 & 0xff);
		}
		break;

	case OP_OPERAND_A_REG:
		if (operand->operand_val & 0x08)
			sprintf(buf, "@A_R%d_Y", operand->operand_val & 0x07);
		else
			sprintf(buf, "@A_R%d_X", operand->operand_val & 0x07);

		reg_str = buf;
		break;

	case OP_OPERAND_A_REG_PLUS_MDFR:
		tmp0 = (operand->operand_val >> 3) & 0x07;
		tmp1 = operand->operand_val & 0x07;

		if (operand->operand_val & 0x40)
			sprintf(buf, "@A_R%d_Y += A_MD%d", tmp0, tmp1);
		else
			sprintf(buf, "@A_R%d_X += A_MD%d", tmp0, tmp1);

		reg_str = buf;
		break;

	case OP_OPERAND_A_REG_X_PLUS_MDFR:
	case OP_OPERAND_A_REG_Y_PLUS_MDFR:
		tmp0 = (operand->operand_val >> 3) & 0x07;
		tmp1 = operand->operand_val & 0x07;

		if (operand->operand_type == OP_OPERAND_A_REG_X_PLUS_MDFR)
			sprintf(buf, "@A_R%d_X += A_MD%d", tmp0, tmp1);
		else
			sprintf(buf, "@A_R%d_Y += A_MD%d", tmp0, tmp1);

		reg_str = buf;
		break;

	case OP_OPERAND_A_REG_CALL_MDFR:
		reg_str = get_dsp_operand_str(operand->operand_val + 0x18);
		break;

	case OP_OPERAND_A_REG_CALL:
		reg_str = get_dsp_operand_str(operand->operand_val + 0x10);
		break;

	case OP_OPERAND_A_REG_INT_7_OFFSET:
		tmp0 = operand->operand_val & 0xf;
		tmp1 = (operand->operand_val >> 4) & 0x7f;
		if (tmp1 & 0x40)
			tmp1 |= 0x80;

		if (!(tmp0 & 0x8)) {
			if ((int8_t)tmp1 < 0)
				sprintf(buf, "@A_R%d_X - 0x%02x", tmp0 & 0x7, -(int8_t)tmp1);
			else
				sprintf(buf, "@A_R%d_X + 0x%02x", tmp0 & 0x7, (int8_t)tmp1);
		} else {
			if ((int8_t)tmp1 < 0)
				sprintf(buf, "@A_R%d_Y - 0x%02x", tmp0 & 0x7, -(int8_t)tmp1);
			else
				sprintf(buf, "@A_R%d_Y + 0x%02x", tmp0 & 0x7, (int8_t)tmp1);
		}

		reg_str = buf;
		break;

	case OP_OPERAND_A_REG_INT_17_OFFSET:
		tmp0 = operand->operand_val & 0x7;
		if (operand->operand_val & 0x100000)
			tmp0 |= 0x8;

		tmp1 = (operand->operand_val >> 3) & 0x1ffff;
		if (tmp1 & 0x10000)
			tmp1 |= 0xfffe0000;

		if (!(tmp0 & 0x8)) {
			if ((int32_t)tmp1 < 0)
				sprintf(buf, "@A_R%d_X - 0x%04x", tmp0 & 0x7, -(int32_t)tmp1);
			else
				sprintf(buf, "@A_R%d_X + 0x%04x", tmp0 & 0x7, (int32_t)tmp1);
		} else {
			if ((int32_t)tmp1 < 0)
				sprintf(buf, "@A_R%d_Y - 0x%04x", tmp0 & 0x7, -(int32_t)tmp1);
			else
				sprintf(buf, "@A_R%d_Y + 0x%04x", tmp0 & 0x7, (int32_t)tmp1);
		}

		reg_str = buf;
		break;

	case OP_OPERAND_A_REG_X_INT_11_OFFSET:
	case OP_OPERAND_A_REG_Y_INT_11_OFFSET:
		tmp0 = operand->operand_val & 0x7;
		tmp1 = (operand->operand_val >> 3) & 0x7ff;
		if (tmp1 & 0x400)
			tmp1 |= 0xf800;

		if (operand->operand_type == OP_OPERAND_A_REG_X_INT_11_OFFSET) {
			if ((int16_t)tmp1 < 0)
				sprintf(buf, "@A_R%d_X - 0x%03x", tmp0, -(int16_t)tmp1);
			else
				sprintf(buf, "@A_R%d_X + 0x%03x", tmp0, (int16_t)tmp1);

		} else {
			if ((int16_t)tmp1 < 0)
				sprintf(buf, "@A_R%d_Y - 0x%03x", tmp0, -(int16_t)tmp1);
			else
				sprintf(buf, "@A_R%d_Y + 0x%03x", tmp0, (int16_t)tmp1);
		}

		reg_str = buf;
		break;

	case OP_OPERAND_A_REG_X_INC:
		sprintf(buf, "@A_R%d_X_INC", operand->operand_val & 0x07);
		reg_str = buf;
		break;

	case OP_OPERAND_A_REG_Y_INC:
		sprintf(buf, "@A_R%d_Y_INC", operand->operand_val & 0x07);
		reg_str = buf;
		break;

	case OP_OPERAND_A_REG_X:
		if (operand->operand_val & 0x08)
			sprintf(buf, "@A_R%d_X_INC", operand->operand_val & 0x07);
		else
			sprintf(buf, "@A_R%d_X", operand->operand_val & 0x07);

		reg_str = buf;
		break;

	case OP_OPERAND_A_REG_Y:
		if (operand->operand_val & 0x08)
			sprintf(buf, "@A_R%d_Y_INC", operand->operand_val & 0x07);
		else
			sprintf(buf, "@A_R%d_Y", operand->operand_val & 0x07);
		reg_str = buf;
		break;

	case OP_OPERAND_REG_3_ACC:
		if (operand->operand_val & 0x04)
			tmp0 = 4;
		else
			tmp0 = 12;

		tmp0 += (operand->operand_val & 0x03);

		reg_str = get_dsp_operand_str(tmp0 & 0x0f);

		break;

	case OP_OPERAND_REG_3_FMA:
		tmp0 = 0;
		if (operand->operand_val & 0x04)
			tmp0 += 4;
		if (operand->operand_val & 0x02)
			tmp0 += 8;
		if (operand->operand_val & 0x01)
			tmp0 += 1;

		reg_str = get_dsp_operand_str(tmp0 & 0x0f);
		break;

	case OP_OPERAND_LITERAL_7_INT:
		if (operand->operand_val & 0x40)
			operand->operand_val |= 0x80;

		sprintf(buf, "#%d", (int8_t)operand->operand_val);
		reg_str = buf;
		break;

	case OP_OPERAND_LITERAL_8_INT:
		sprintf(buf, "#%d", (int8_t)operand->operand_val);
		reg_str = buf;
		break;

	case OP_OPERAND_LITERAL_8_INT_PC_OFFSET:
		i_tmp = (int8_t)operand->operand_val;
		tmp0 = data->cur_addr + i_tmp;

		if (i_tmp >= 0)
			sprintf(buf, "#0x%02x", i_tmp);
		else
			sprintf(buf, "#-0x%02x", -i_tmp);

		data->offset_addr_op_set = 1;
		data->offset_addr = tmp0;

		reg_str = buf;
		break;

	case OP_OPERAND_LITERAL_16_INT:
		sprintf(buf, "#%d", (int16_t)operand->operand_val);
		reg_str = buf;
		break;

	case OP_OPERAND_LITERAL_17_INT:
		if (operand->operand_val & 0x10000)
			sprintf(buf, "#%d", (int32_t)operand->operand_val | 0xfffe);
		else
			sprintf(buf, "#%d", (int32_t)operand->operand_val);

		reg_str = buf;
		break;

	case OP_OPERAND_LITERAL_8:
		sprintf(buf, "#0x%02x", operand->operand_val);
		reg_str = buf;
		break;

	case OP_OPERAND_LITERAL_11:
		sprintf(buf, "#0x%03x", operand->operand_val & 0x7ff);
		reg_str = buf;
		break;

	case OP_OPERAND_LITERAL_16:
		switch (operand->operand_mod_type) {
		case OPERAND_MDFR_16_BIT_UPPER:
			sprintf(buf, "#0x%08x", operand->operand_val << 16);
			break;

		case OPERAND_MDFR_16_BIT_SIGNED:
			sprintf(buf, "#%d", (int16_t)operand->operand_val);
			break;

		default:
			sprintf(buf, "#0x%04x", operand->operand_val);
			break;
		}

		reg_str = buf;
		break;

	case OP_OPERAND_LITERAL_16_ADDR:
		if (operand->operand_val < 0x10000)
			sprintf(buf, "@#0x%04x_X", operand->operand_val & 0xffff);
		else
			sprintf(buf, "@#0x%04x_Y", operand->operand_val & 0xffff);
		reg_str = buf;

		break;

	case OP_OPERAND_LITERAL_32:
		switch (operand->operand_mod_type) {
		case OPERAND_MDFR_32_BIT_SIGNED:
			i_tmp = (int32_t)operand->operand_val;
			if (i_tmp < 0)
				sprintf(buf, "#-0x%08x", -i_tmp);
			else
				sprintf(buf, "#0x%08x", i_tmp);

			break;

		default:
			sprintf(buf, "#0x%08x", operand->operand_val);
			break;
		}

		reg_str = buf;
		break;
	case OP_OPERAND_NOP:
		/*
		sprintf(buf, "NO_ARGS");
		reg_str = buf;
		*/
		reg_str = NULL;
		break;

	default:
		reg_str = "UNK_REG";
		break;
	}

	if (reg_str)
		printf("%s", reg_str);

	switch (operand->operand_mod_type) {
	case OPERAND_MDFR_INC:
		printf("++");
		break;
	case OPERAND_MDFR_DEC:
		printf("--");
		break;
	case OPERAND_MDFR_RR:
		printf(" >> 1");
		break;
	case OPERAND_MDFR_RL:
		printf(" << 1");
		break;
	default:
		break;
	}

	/*
	 * If this isn't the final operand, and it also doesn't end a set of
	 * parallel operands, then put a comma.
	 */
	if (!final && !operand->parallel_end)
		printf(", ");
	else if (operand->parallel_end)
		printf(" :\n        %s ", operand->op_str);
}

/* Get the operand values for an opcode. */
static void get_op_operands(dsp_main *data, const dsp_op_info *op_info,
		const operand_loc_descriptor *operand_loc, uint32_t operand_cnt,
		operand_data *operands, uint8_t parallel_op)
{
	const operand_loc_descriptor *tmp;
	uint32_t i, src_mdfr, src_dst_swap;
	operand_data op_data_tmp;
	const char *op_str;

	src_dst_swap = op_info->src_dst_swap;
	src_mdfr = op_info->src_mdfr[0];
	op_str = op_info->op_str;

	if (op_info->mdfr_bit) {
		if (get_bits_in_op_words(data->cur_op, op_info->mdfr_bit, 1)) {
			switch (op_info->mdfr_bit_type) {
			case OP_MDFR_BIT_TYPE_SRC_DST_SWAP:
				if (op_info->alt_op_str)
					op_str = op_info->alt_op_str;
				src_dst_swap = 1;
				break;

			case OP_MDFR_BIT_TYPE_USE_ALT_MDFR:
				if (op_info->alt_op_str)
					op_str = op_info->alt_op_str;
				src_mdfr = op_info->src_mdfr[1];
				break;

			case OP_MDFR_BIT_TYPE_USE_ALT_STR:
				op_str = op_info->alt_op_str;
				break;

			case OP_MDFR_BIT_TYPE_USE_ALT_LAYOUT:
				if (op_info->alt_op_str)
					op_str = op_info->alt_op_str;
				break;

			default:
				break;
			}
		}
	}

	data->cur_op_str = op_str;
	printf("%s ", op_str);

	for (i = 0; i < operand_cnt; i++) {
		memset(&op_data_tmp, 0, sizeof(op_data_tmp));
		tmp = &operand_loc[i];

		op_data_tmp.op_str       = op_str;
		op_data_tmp.operand_val  = get_op_operand(data, tmp);
		op_data_tmp.operand_type = tmp->operand_type;
		op_data_tmp.operand_dir  = tmp->operand_dir;
		op_data_tmp.parallel_end = tmp->parallel_end;
		if (op_data_tmp.operand_dir == OPERAND_DIR_SRC || op_data_tmp.operand_dir == OPERAND_DIR_X
				|| op_data_tmp.operand_dir == OPERAND_DIR_Y)
			op_data_tmp.operand_mod_type = src_mdfr;

		if (src_dst_swap) {
			if (op_data_tmp.operand_dir == OPERAND_DIR_DST) {
				op_data_tmp.operand_dir = OPERAND_DIR_SRC;
				op_data_tmp.operand_mod_type = src_mdfr;
				operands[i + 1] = op_data_tmp;
			} else if (op_data_tmp.operand_dir == OPERAND_DIR_SRC) {
				op_data_tmp.parallel_end = 0;
				op_data_tmp.operand_dir = OPERAND_DIR_DST;
				op_data_tmp.operand_mod_type = 0;

				operands[i].parallel_end = tmp->parallel_end;
				operands[i - 1] = op_data_tmp;
			}
		} else {
			operands[i] = op_data_tmp;
		}
	}
}

/*
 * Keep these probably.
 */
static void print_op(dsp_main *data, const dsp_op_info *op_info,
		const op_operand_loc_layout *loc_layout)
{
	const operand_loc_descriptor *loc_descriptors;
	uint32_t i, operand_cnt, final;
	operand_data op_data[8];

	/* Get location descriptors and operand count. */
	operand_cnt = loc_layout->operand_cnt;
	loc_descriptors = loc_layout->operand_loc;

	/*
	 * Get the operand values from the locations described in the
	 * descriptors.
	 */
	get_op_operands(data, op_info, loc_descriptors, operand_cnt, op_data, 0);

	/* Next up: print the operands. */
	for (i = final = 0; i < operand_cnt; i++) {
		final = (i + 1) == operand_cnt ? 1 : 0;

		print_operand_str(data, &op_data[i], final);
	}
}

static void get_parallel_op_data(dsp_main *data, const dsp_op_info *op_info)
{
	uint32_t val, i, op_len, tmp, layout_id;
	const op_operand_loc_layout *loc_layout;
	const op_operand_layout *layout;
	const dsp_op_info *p_op;

	val = get_bits_in_op_words(data->cur_op, 10, 6);
	loc_layout = NULL;
	op_len = get_dsp_op_len(op_info->op);
	if (op_len == 2) {

		if (val >= 0x3e) {
			if (get_bits_in_op_words(data->cur_op, 16, 1))
				return;
		}

		if (val < 0x30)
			val &= 0x30;
	} else {
		if (val == 0x3f) {
			if (get_bits_in_op_words(data->cur_op, 17, 1))
				return;
		}
	}

	p_op = get_dsp_p_op_info(val, op_len);
	if (!p_op)
		return;

	layout_id = p_op->layout_id;
	if (p_op->mdfr_bit_type == OP_MDFR_BIT_TYPE_USE_ALT_LAYOUT) {
		if (get_bits_in_op_words(data->cur_op, p_op->mdfr_bit, 1))
			layout_id = p_op->alt_layout_id;
	}

	layout = get_p_op_layout(layout_id);
	for (i = 0; i < layout->loc_layout_cnt; i++) {
		loc_layout = &layout->loc_layouts[i];

		/*
		 * If the value of bits is 0, we've reached the end, and
		 * there's no need to check. Otherwise, check if the described
		 * bits are set.
		 */
		if (loc_layout->layout_val_loc.part1_bits != 0)
			tmp = get_op_operand(data, &loc_layout->layout_val_loc);
		else
			break;

		if (loc_layout->layout_val == tmp)
			break;
	}

	if (!loc_layout)
		return;

	print_op(data, p_op, loc_layout);
	printf(" /");
	printf("\n        ");
}

static void get_op_data(dsp_main *data, const dsp_op_info *op_info)
{
	const op_operand_loc_layout *loc_layout;
	const op_operand_layout *layout;
	uint32_t i, tmp, layout_id;

	if (!op_info->has_op_layout) {
		printf("0x%04x: %s ", data->cur_addr, op_info->op_str);
		return;
	}

	printf("0x%04x: ", data->cur_addr);

	layout_id = op_info->layout_id;
	loc_layout = NULL;
	/* Check if the op has a possible alternative layout. */
	if (op_info->mdfr_bit_type == OP_MDFR_BIT_TYPE_USE_ALT_LAYOUT) {
		if (get_bits_in_op_words(data->cur_op, op_info->mdfr_bit, 1)) {
			layout_id = op_info->alt_layout_id;
		}
	}

	if (layout_id == OP_LAYOUT_NOP) {
		if ((get_dsp_op_len(op_info->op) > 1))
			get_parallel_op_data(data, op_info);

		printf("%s;", op_info->op_str);
		return;
	}

	layout = get_op_layout(layout_id);
	for (i = 0; i < layout->loc_layout_cnt; i++) {
		loc_layout = &layout->loc_layouts[i];

		/*
		 * If the value of bits is 0, we've reached the end, and
		 * there's no need to check. Otherwise, check if the described
		 * bits are set.
		 */
		if (loc_layout->layout_val_loc.part1_bits != 0)
			tmp = get_op_operand(data, &loc_layout->layout_val_loc);
		else
			break;

		if (loc_layout->layout_val == tmp)
			break;
	}

	if ((get_dsp_op_len(op_info->op) > 1) && loc_layout->supports_opt_args)
		get_parallel_op_data(data, op_info);

	print_op(data, op_info, loc_layout);
	printf(";");
}

/* Get operand data for the current opcode. */
static int get_next_op(dsp_main *data)
{
	const dsp_op_info *op_info;

	if (get_dsp_op(data, &op_info))
		return 0;

	if (!op_info) {
		return 1;
	}

	get_op_data(data, op_info);

	/* Put an extra newline after certain ops to make things more clear. */
	switch (op_info->op) {
	case 0x0007: /* RET */
	case 0x0016: /* RETI */
		putchar('\n');
		break;

	case 0x0100: /* JMP */
		if (get_bits_in_op_words(data->cur_op, op_info->mdfr_bit, 1))
			putchar('\n');
		break;
	default:
		break;
	}

	if (data->offset_addr_op_set) {
		printf(" /* 0x%04x */", data->offset_addr);
		data->offset_addr_op_set = data->offset_addr = 0;
	}

	printf("\n");

	data->cur_addr += get_dsp_op_len(op_info->op);

	return 1;
}

int main(int argc, char **argv)
{
	dsp_main dsp_data;

	if (argc < 2) {
		printf("Usage: %s <file>\n", argv[0]);
		return -1;
	}

	memset(&dsp_data, 0, sizeof(dsp_data));
	dsp_data.pmem = fopen(argv[1], "r");
	if (!dsp_data.pmem) {
		printf("Failed to open file %s!\n", argv[1]);
		return -1;
	}

	while (get_next_op(&dsp_data))
	{
	}

	fclose(dsp_data.pmem);

	return 0;
}
