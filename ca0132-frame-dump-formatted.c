/*
 * ca0132-frame-dump-formatted.c:
 * Takes a ca0132 frame dump file and prints it out in a more human readable
 * way. Makes it much easier to figure out which verbs were run.
 */
#include "ca0132_defs.h"

#define NODE_COUNT 0x18

struct hda_verb {
	uint32_t codec;
	uint32_t node;
	uint32_t verb;
	uint32_t data;
};

struct dsp_data {
	struct scp_data scp_data;

	uint32_t scp_start_addr;
	uint8_t scp_set[2], scp_data_set[2];
	uint32_t cur_data_cnt;
	uint32_t low, high;
	uint32_t data, scp;

	uint8_t scp_read_data_set_cnt, scp_post_read_cnt;
};

struct chipio_data {
	uint8_t addr_set[2], data_set[2], read_set;
	uint32_t chipio_addr, chipio_data;
	uint32_t addr_set_start;

	uint8_t param_set;
	uint32_t param_addr;
	uint32_t param;

	uint8_t u_8051_addr_set[2], u_8051_data_set, u_8051_pll_set;
	uint16_t u_8051_addr, u_8051_data;
	uint32_t u_8051_addr_set_start;
	uint32_t u_8051_data_run_len;
	uint8_t u_8051_data_run[128];

	uint32_t dsp_downloads;
	uint32_t port_free_count;
};

struct generic_node {
	uint32_t pincfg;
	uint8_t  pin_ctl;
};

struct ca0132_data {
	FILE *frames;
	uint8_t at_end;

	uint32_t main_codec_id;
	uint32_t codec_cnt;

	uint32_t cur_data, cur_addr;
	struct hda_verb cur_verb;

	struct dsp_data dsp_data;
	struct chipio_data chipio_data;
	struct generic_node nodes[NODE_COUNT];
};

static void get_next_word(struct ca0132_data *data)
{
	if (!fread(&data->cur_data, sizeof(uint32_t), 1, data->frames))
	{
		data->at_end = 1;
		return;
	}

	data->cur_addr += 0x4;
}

static void get_next_verb(struct ca0132_data *data)
{
	struct hda_verb *verb = &data->cur_verb;

	get_next_word(data);
	if (data->at_end)
		return;
	memset(verb, 0, sizeof(*verb));

	verb->codec = (data->cur_data >> 28) & 0x0f;
	verb->node  = (data->cur_data >> 20) & 0x7f;
	verb->verb  = (data->cur_data >> 8) & 0xfff;
	verb->data  = data->cur_data & 0xff;
}

static const struct hda_verb_info *find_verb_info(struct hda_verb *verb)
{
	const struct hda_verb_info *verb_info;
	uint32_t verb_val, tmp;

	tmp = verb->verb >> 8;
	if (tmp != 0x7 && tmp != 0xf)
		verb_val = verb->verb & 0xf00;
	else
		verb_val = verb->verb;

	verb_info = get_hda_verb_info(verb_val);

	return verb_info;
}

static void print_cur_verb(struct ca0132_data *data)
{
	struct hda_verb *verb = &data->cur_verb;
	const struct hda_verb_info *verb_info = NULL;

	if (verb->node != 0x15 && verb->node != 0x16)
		verb_info = find_verb_info(verb);

	if (verb_info)
		printf("0x%06x: codec 0x%02x, node 0x%02x, verb 0x%03x, data 0x%02x, name %s.\n", data->cur_addr - 0x4,
				verb->codec, verb->node, verb->verb, verb->data, verb_info->name);
	else
		printf("0x%06x: codec 0x%02x, node 0x%02x, verb 0x%03x, data 0x%02x.\n", data->cur_addr - 0x4,
				verb->codec, verb->node, verb->verb, verb->data);
}

static void print_scp_write_data(struct dsp_data *data)
{
	struct scp_data *scp_data = &data->scp_data;
	const struct scp_cmd_info *scp_info;
	uint32_t i;
	float *tmp;

	scp_info = dsp_get_scp_cmd_info(scp_data->target_id, scp_data->req);
	if (scp_data->target_id == 0x96 && scp_data->req == 0x3a) {
		if (scp_data->val[0] == 0x3f800000) {
			printf("--------------------------------------------------------------------------------\n");
			printf("Begin output switch.\n");
		}
	}

	printf("0x%06x: size %d, err_flag %d, resp_flag %d, dev_flag %d, req 0x%02x,\n",
			data->scp_start_addr, scp_data->data_size, scp_data->error_flag,
			scp_data->resp_flag, scp_data->device_flag, scp_data->req);
	printf("                  get_flag %d, src_id 0x%02x,       target_id 0x%02x.\n",
			scp_data->get_flag, scp_data->source_id, scp_data->target_id);


	if (scp_info)
		printf("                  ReqID: %s.\n", scp_info->name);

	for (i = 0; i < scp_data->data_size; ++i) {
		tmp = (float *)&scp_data->val[i];
		printf("Val[%d]: %f, %#08x.\n", i, *tmp, scp_data->val[i]);
	}

	if (scp_data->target_id == 0x96 && scp_data->req == 0x3a) {
		if (!scp_data->val[0]) {
			printf("Exit output switch.\n");
			printf("--------------------------------------------------------------------------------\n");
		}
	}
	putchar('\n');
}

static uint16_t extract_16_bit_data(struct hda_verb *verb)
{
	return ((verb->verb & 0xff) << 8) | verb->data;
}

static void dsp_scp_clear(struct dsp_data *data)
{
	memset(data, 0, sizeof(*data));
}

static uint8_t dsp_scp_set(struct dsp_data *data)
{
	return data->scp_set[0] && data->scp_set[1];
}

static void dsp_scp_read_check(struct dsp_data *data)
{
	if (data->scp_read_data_set_cnt == 4 && data->scp_post_read_cnt == 2)
	{
		print_scp_write_data(data);
		dsp_scp_clear(data);
	}
}

static void dsp_scp_write_check(struct dsp_data *data)
{
	print_scp_write_data(data);
	dsp_scp_clear(data);
}

static void dsp_scp_check(struct dsp_data *data)
{
	if (dsp_scp_set(data)) {
		if (data->cur_data_cnt == data->scp_data.data_size) {
			if (data->scp_data.get_flag)
				dsp_scp_read_check(data);
			else
				dsp_scp_write_check(data);
		}
	}
}

static void dsp_scp_handler(struct ca0132_data *data)
{
	struct hda_verb *verb = &data->cur_verb;
	struct dsp_data *dsp_data = &data->dsp_data;
	uint32_t tmp, tmp1;

	tmp =  verb->verb & 0xf00;
	switch (tmp) {
	case VENDOR_DSPIO_SCP_WRITE_DATA_LOW:
		if (!dsp_scp_set(dsp_data)) {
			if (dsp_data->scp_set[0])
			{
				printf("scp isn't in correct format.\n");
				print_cur_verb(data);
				dsp_scp_clear(dsp_data);
				break;
			}
			dsp_scp_clear(dsp_data);
			dsp_data->scp_start_addr = data->cur_addr - 0x4;
			tmp1 = verb->data;
			tmp1 |= (verb->verb & 0xff) << 8;
			dsp_data->scp = tmp1;
			dsp_data->scp_set[0] = 1;
		} else if (dsp_data->scp_data.data_size != dsp_data->cur_data_cnt) {
			dsp_data->scp_data_set[0] = dsp_data->scp_data_set[1] = 0;
			tmp1 = verb->data;
			tmp1 |= (verb->verb & 0xff) << 8;
			dsp_data->data = tmp1;
			dsp_data->scp_data_set[0] = 1;
		} else {
			printf("Aborting current scp, incorrect format, dsp_data->cur_data_cnt %d.\n",
					dsp_data->cur_data_cnt);
			print_scp_write_data(dsp_data);
			dsp_scp_clear(dsp_data);
			dsp_data->scp_start_addr = data->cur_addr - 0x4;
			tmp1 = verb->data;
			tmp1 |= (verb->verb & 0xff) << 8;
			dsp_data->scp = tmp1;
			dsp_data->scp_set[0] = 1;
		}
		break;

	case VENDOR_DSPIO_SCP_WRITE_DATA_HIGH:
		if (!dsp_scp_set(dsp_data)) {
			if (!dsp_data->scp_set[0])
			{
				printf("scp isn't in correct format.\n");
				print_cur_verb(data);
				dsp_scp_clear(dsp_data);
				break;
			}
			tmp1 = verb->data;
			tmp1 |= (verb->verb & 0xff) << 8;
			dsp_data->scp |= (tmp1 << 16);
			dsp_data->scp_set[1] = 1;
			get_scp_data(&dsp_data->scp_data, dsp_data->scp);
			if (dsp_data->scp_data.data_size >= 4) {
				printf("scp isn't in correct format.\n");
				print_cur_verb(data);
				dsp_scp_clear(dsp_data);
				break;
			}
		} else if (dsp_data->scp_data.data_size != dsp_data->cur_data_cnt) {
			if (!dsp_data->scp_data_set[0])
			{
				printf("scp isn't in correct format.\n");
				dsp_scp_clear(dsp_data);
				print_cur_verb(data);
			}
			tmp1 = verb->data;
			tmp1 |= (verb->verb & 0xff) << 8;
			dsp_data->data |= (tmp1 << 16);
			dsp_data->scp_data_set[1] = 1;
			if (dsp_data->cur_data_cnt >= 15)
				break;

			dsp_data->scp_data.val[dsp_data->cur_data_cnt++] = dsp_data->data;
		}

		break;
	default:
		break;
	}

	dsp_scp_check(dsp_data);
}

static void dsp_verb_handler(struct ca0132_data *data)
{
	struct hda_verb *verb = &data->cur_verb;
	uint32_t tmp;

	tmp = (verb->verb >> 8) & 0xf;

	switch (tmp) {
	case 0x0:
	case 0x1:
		dsp_scp_handler(data);
		break;
	case 0x7:
		switch (verb->verb) {
		case VENDOR_DSPIO_SCP_POST_READ_DATA:
			data->dsp_data.scp_read_data_set_cnt++;
			dsp_scp_handler(data);
		case VENDOR_DSPIO_SCP_POST_COUNT_QUERY:
			data->dsp_data.scp_post_read_cnt++;
			dsp_scp_handler(data);
			break;
		default:
			break;
		}

		break;
	case 0xf:
		switch (verb->verb) {
		case VENDOR_DSPIO_STATUS:
			if (dsp_scp_set(&data->dsp_data))
				break;
			break;
		case VENDOR_DSPIO_SCP_READ_DATA:
			data->dsp_data.scp_read_data_set_cnt++;
			dsp_scp_handler(data);
			break;
		case VENDOR_DSPIO_SCP_READ_COUNT:
			data->dsp_data.scp_post_read_cnt++;
			dsp_scp_handler(data);
			break;
		default:
			print_cur_verb(data);
			break;
		}
		break;

	default:
		print_cur_verb(data);
		break;
	}

}

/* ChipIO flag related functions. */
static void chipio_flag_set(struct ca0132_data *data)
{
	struct hda_verb *verb = &data->cur_verb;
	uint32_t flag, val;

	flag = verb->data & 0x7f;
	val = (verb->data >> 7) & 0x1;

	printf("0x%06x: Flag %s (%d), set %d.\n", data->cur_addr - 0x4,
			chipio_get_flag_str(flag), flag, val);
}

/* ChipIO param related functions. */
static void chipio_param_extract(struct ca0132_data *data)
{
	struct hda_verb *verb = &data->cur_verb;
	uint32_t param, val;

	param = verb->data & 0x1f;
	val = (verb->data >> 5);

	printf("0x%06x: Param %s (%d), set %d.\n", data->cur_addr - 0x4,
			chipio_get_param_str(param), param, val);
}

static void chipio_param_write(struct ca0132_data *data)
{
	struct chipio_data *chipio_data = &data->chipio_data;
	struct hda_verb *verb = &data->cur_verb;

	printf("0x%06x: Param %s (%d), set 0x%02x.\n", chipio_data->param_addr,
			chipio_get_param_str(chipio_data->param), chipio_data->param, verb->data);
}

static void chipio_param_get(struct ca0132_data *data)
{
	struct hda_verb *verb = &data->cur_verb;

	printf("0x%06x: Get param %s (%d).\n", data->cur_addr - 0x4,
			chipio_get_param_str(verb->data), verb->data);
}

/* ChipIO HIC related functions. */
static void chipio_hic_clear(struct chipio_data *data)
{
	memset(data->addr_set, 0, sizeof(data->addr_set) * 2);
	memset(data->data_set, 0, sizeof(data->data_set) * 2);
	data->chipio_addr = data->chipio_data = 0;
	data->addr_set_start = 0;
	data->read_set = 0;
}

static uint8_t chipio_hic_complete(struct chipio_data *data)
{
	return ((data->addr_set[0] && data->addr_set[1])
			&& (data->data_set[0] && data->data_set[1]));
}

static uint8_t chipio_hic_complete_read(struct chipio_data *data)
{
	return ((data->addr_set[0] && data->addr_set[1])
			&& data->read_set);
}

static void chipio_hic_check(struct chipio_data *data)
{
	if (chipio_hic_complete(data)) {
		printf("0x%06x: HIC Addr 0x%06x, Data 0x%08x.\n", data->addr_set_start,
			data->chipio_addr, data->chipio_data);

		chipio_hic_clear(data);
	}
}

static void chipio_hic_check_read(struct chipio_data *data)
{
	if (chipio_hic_complete_read(data)) {
		printf("0x%06x: Readback HIC Addr 0x%06x.\n", data->addr_set_start,
			data->chipio_addr);

		chipio_hic_clear(data);
	}
}


static void chipio_hic_handler(struct ca0132_data *data)
{
	struct chipio_data *chipio_data = &data->chipio_data;
	struct hda_verb *verb = &data->cur_verb;
	uint32_t tmp;

	tmp = (verb->verb >> 8) & 0xf;
	switch (tmp) {
	case 0x0:
		chipio_hic_clear(chipio_data);
		chipio_data->addr_set_start = data->cur_addr - 0x4;
		chipio_data->chipio_addr = extract_16_bit_data(verb);
		chipio_data->addr_set[0] = 1;
		break;
	case 0x1:
		chipio_data->chipio_addr |= (extract_16_bit_data(verb) << 16);
		chipio_data->addr_set[1] = 1;

		break;
	case 0x3:
		chipio_data->chipio_data = extract_16_bit_data(verb);
		chipio_data->data_set[0] = 1;
		break;
	case 0x4:
		chipio_data->chipio_data |= (extract_16_bit_data(verb) << 16);
		chipio_data->data_set[1] = 1;
		break;
	}

	chipio_hic_check(chipio_data);
}

/* ChipIO 8051 related functions. */
static void chipio_8051_direct_write(struct ca0132_data *data)
{
	struct hda_verb *verb = &data->cur_verb;

	printf("0x%06x: 8051 write direct addr 0x%02x, value 0x%02x.\n", data->cur_addr - 0x4,
		       	verb->data, verb->verb & 0xff);
}

static void chipio_8051_data_clear(struct chipio_data *data)
{
	data->u_8051_addr_set[0] = data->u_8051_addr_set[1] = 0;
	data->u_8051_data_set = data->u_8051_pll_set = 0;
	data->u_8051_addr = data->u_8051_data = 0;
	data->u_8051_addr_set_start = 0;
	data->u_8051_data_run_len = 0;
	memset(data->u_8051_data_run, 0, 128);
}

static uint8_t chipio_8051_complete_addr(struct chipio_data *data)
{
	return data->u_8051_addr_set[0] && data->u_8051_addr_set[1];
}

static void chipio_8051_print_data_run(struct chipio_data *data)
{
	uint32_t i;

	printf("0x%06x: 8051_addr_start 0x%04x:\n", data->u_8051_addr_set_start, data->u_8051_addr);
	for (i = 0; i < data->u_8051_data_run_len; ++i)
		printf("Data: 0x%02x.\n", data->u_8051_data_run[i]);
}

static void chipio_8051_print_read(struct chipio_data *data)
{
	printf("0x%06x: 8051_addr_read 0x%04x.\n", data->u_8051_addr_set_start, data->u_8051_addr);
}

static void chipio_8051_handler(struct ca0132_data *data)
{
	struct chipio_data *chipio_data = &data->chipio_data;
	struct hda_verb *verb = &data->cur_verb;

	if (verb->verb != VENDOR_CHIPIO_8051_DATA_WRITE && chipio_data->u_8051_data_run_len)
	{
		chipio_8051_print_data_run(chipio_data);
		chipio_8051_data_clear(chipio_data);
	}

	switch (verb->verb) {
	case VENDOR_CHIPIO_8051_ADDRESS_LOW:
		chipio_data->u_8051_addr_set_start = data->cur_addr - 0x4;
		chipio_data->u_8051_addr = verb->data;
		chipio_data->u_8051_addr_set[0] = 1;

		break;
	case VENDOR_CHIPIO_8051_ADDRESS_HIGH:
		if (chipio_data->u_8051_addr_set[0]) {
			chipio_data->u_8051_addr |= (verb->data << 8);
			chipio_data->u_8051_addr_set[1] = 1;
		} else {
			chipio_8051_data_clear(chipio_data);
		}
		break;
	case VENDOR_CHIPIO_8051_DATA_WRITE:
		if (chipio_8051_complete_addr(chipio_data))
		{
			chipio_data->u_8051_addr_set[0] = chipio_data->u_8051_addr_set[1] = 0;
			chipio_data->u_8051_data_run[chipio_data->u_8051_data_run_len++] = verb->data;
		} else if (chipio_data->u_8051_data_run_len) {
			chipio_data->u_8051_data_run[chipio_data->u_8051_data_run_len++] = verb->data;
		} else {
			printf("random 8051 data write!\n");
			print_cur_verb(data);
		}

		break;
	case VENDOR_CHIPIO_PLL_PMU_WRITE:
		if (chipio_data->u_8051_addr_set[0])
		{
			printf("0x%06x: PLL PMU write addr 0x%02x, data 0x%02x.\n",
					chipio_data->u_8051_addr_set_start,
					chipio_data->u_8051_addr, verb->data);
		} else {
			print_cur_verb(data);
		}

		chipio_8051_data_clear(chipio_data);

		break;
	case VENDOR_CHIPIO_8051_DATA_READ:
		if (chipio_8051_complete_addr(chipio_data)) {
			chipio_8051_print_read(chipio_data);
			chipio_8051_data_clear(chipio_data);
		} else {
			print_cur_verb(data);
		}
		break;
	}

	if (verb->verb != VENDOR_CHIPIO_8051_DATA_WRITE && chipio_data->u_8051_data_run_len)
	{
		chipio_8051_print_data_run(chipio_data);
		chipio_8051_data_clear(chipio_data);
	}
}

static void chipio_verb_handler(struct ca0132_data *data)
{
	struct chipio_data *chipio_data = &data->chipio_data;
	struct hda_verb *verb = &data->cur_verb;
	uint32_t tmp;

	tmp = (verb->verb >> 8) & 0xf;

	switch (tmp) {
	case 0x2:
		print_cur_verb(data);
		break;

	case 0x0:
	case 0x1:
	case 0x3:
	case 0x4:
		chipio_hic_handler(data);
		break;

	case 0x5:
		chipio_8051_direct_write(data);
		break;
	case 0xd:
		print_cur_verb(data);
		break;

	case 0x7:
		switch (verb->verb) {
		case VENDOR_CHIPIO_FLAG_SET:
			chipio_flag_set(data);
			break;
		case VENDOR_CHIPIO_PARAM_SET:
			chipio_param_extract(data);
			break;
		case VENDOR_CHIPIO_PARAM_GET:
			chipio_param_get(data);
			break;
		case VENDOR_CHIPIO_PARAM_EX_ID_SET:
			chipio_data->param_set = 1;
			chipio_data->param = verb->data;
			chipio_data->param_addr = data->cur_addr - 0x4;
			break;
		case VENDOR_CHIPIO_PARAM_EX_VALUE_SET:
			if (chipio_data->param_set) {
				chipio_param_write(data);
			}
			chipio_data->param_set = 0;
			break;
		case VENDOR_CHIPIO_HIC_POST_READ:
			if (chipio_data->addr_set[0] && chipio_data->addr_set[1])
				chipio_data->read_set = 1;
			break;
		case VENDOR_CHIPIO_PORT_FREE_SET:
			chipio_data->port_free_count++;
			if (chipio_data->port_free_count == chipio_data->dsp_downloads)
				printf("\n----------END_DSP_DOWNLOAD------------\n\n");
			break;
		case VENDOR_CHIPIO_8051_ADDRESS_LOW:
		case VENDOR_CHIPIO_8051_ADDRESS_HIGH:
		case VENDOR_CHIPIO_8051_DATA_WRITE:
		case VENDOR_CHIPIO_PLL_PMU_WRITE:
			chipio_8051_handler(data);
			break;
		default:
			print_cur_verb(data);
			break;
		}
		break;
	case 0xf:
		switch (verb->verb) {
		case VENDOR_CHIPIO_8051_DATA_READ:
			chipio_8051_handler(data);
			break;
		case VENDOR_CHIPIO_HIC_READ_DATA:
			chipio_hic_check_read(chipio_data);
			break;
		case VENDOR_CHIPIO_STATUS:
			if ((chipio_data->addr_set[0] && chipio_data->addr_set[1]) || chipio_data->param_set)
				break;
			else
				print_cur_verb(data);
			break;
		default:
			print_cur_verb(data);
			break;
		}
		break;

	default:
		print_cur_verb(data);
		break;
	}

}

static void node_pincfg_set(struct ca0132_data *data)
{
	struct hda_verb *verb = &data->cur_verb;
	struct generic_node *node = &data->nodes[verb->node];
	uint32_t tmp, shift;

	shift = verb->verb - AC_VERB_SET_CONFIG_DEFAULT_BYTES_0;
	tmp = 0xff << (shift * 0x8);
	node->pincfg &= ~tmp;

	node->pincfg |= verb->data << (shift * 0x8);

	if (verb->verb == AC_VERB_SET_CONFIG_DEFAULT_BYTES_3)
		printf("Node 0x%02x pincfg 0x%08x.\n", verb->node, node->pincfg);
}

static void get_pinctl_vals(uint32_t pinctl, uint32_t *hp_enable,
		uint32_t *out_enable, uint32_t *in_enable, uint32_t *vref)
{
	*hp_enable  = !!((pinctl >> 7) & 0x1);
	*out_enable = !!((pinctl >> 6) & 0x1);
	*in_enable  = !!((pinctl >> 5) & 0x1);
	*vref = pinctl & 0x7;
}

static void node_pinctl_set(struct ca0132_data *data)
{
	struct hda_verb *verb = &data->cur_verb;
	struct generic_node *node = &data->nodes[verb->node];
	uint32_t hp, out, in, vref;

	node->pin_ctl = verb->data;
	get_pinctl_vals(verb->data, &hp, &out, &in, &vref);
	printf("0x%06x: codec 0x%02x, node 0x%02x, pinctl 0x%02x.\n", data->cur_addr - 0x4, verb->codec, verb->node,
			node->pin_ctl);
	printf("pinctl: HP-Enable %d, Out-Enable %d, In-Enable %d, vref 0x%02x.\n", hp, out, in, vref);
}

#define AMP_DIR_OUT_SET_MASK 0x8000
#define AMP_CH_LEFT_SET_MASK 0x2000
static void node_amp_set(struct ca0132_data *data)
{
	struct hda_verb *verb = &data->cur_verb;
	const char *channel, *dir;
	uint32_t payload;

	payload = verb->data;
	payload |= (verb->verb & 0xff) << 8;

	if (payload & AMP_DIR_OUT_SET_MASK)
		dir = "OUT";
	else
		dir = "IN";

	if (payload & AMP_CH_LEFT_SET_MASK)
		channel = "LEFT";
	else
		channel = "RIGHT";

	printf("0x%06x: codec 0x%02x, node 0x%02x, dir %s, ch %s, mute %d, gain 0x%02x.\n", data->cur_addr - 0x4, verb->codec, verb->node,
			dir, channel, !!(payload & 0x80), payload & 0x7f);
}

static void default_node_verb_handler(struct ca0132_data *data)
{
	struct hda_verb *verb = &data->cur_verb;
	uint32_t tmp;

	tmp = (verb->verb >> 8) & 0xf;

	switch (tmp) {
	case 0x3:
		node_amp_set(data);
		break;
	case 0x7:
		switch (verb->verb) {
		case AC_VERB_SET_CONFIG_DEFAULT_BYTES_0:
		case AC_VERB_SET_CONFIG_DEFAULT_BYTES_1:
		case AC_VERB_SET_CONFIG_DEFAULT_BYTES_2:
		case AC_VERB_SET_CONFIG_DEFAULT_BYTES_3:
			print_cur_verb(data);
			node_pincfg_set(data);
			break;
		case AC_VERB_SET_PIN_WIDGET_CONTROL:
			node_pinctl_set(data);
			break;
		default:
			print_cur_verb(data);
			break;
		}

		break;
	default:
		print_cur_verb(data);
		break;
	}
}

/*static void check_verb(uint32_t corb_data)*/
static void check_verb(struct ca0132_data *data)
{
	struct hda_verb *verb = &data->cur_verb;

	switch (verb->node) {
	case 0x16:
		dsp_verb_handler(data);
		break;

	case 0x15:
		chipio_verb_handler(data);
		break;

	default:
		default_node_verb_handler(data);
		break;
	}
}

static void count_dsp_downloads(struct ca0132_data *data)
{
	struct hda_verb *verb = &data->cur_verb;

	while (!data->at_end) {
		get_next_verb(data);

		if (verb->node == 0x15 && verb->verb == VENDOR_CHIPIO_PORT_FREE_SET)
			data->chipio_data.dsp_downloads++;

		if (verb->node == 0x15 && verb->verb == CHIPIO_CT_EXTENSIONS_ENABLE)
			data->main_codec_id = verb->codec;
	}

	rewind(data->frames);
	data->cur_addr = 0;
	data->at_end = 0;
}

int main(int argc, char **argv)
{
	struct ca0132_data main_data;

	memset(&main_data, 0, sizeof(main_data));

	if (argc < 1) {
		printf("Usage: %s <allCORBframes-location>\n", argv[0]);
		return 1;
	}

	main_data.frames = fopen(argv[1], "r");
	if (!main_data.frames) {
		printf("Failed to open file %s!\n", argv[1]);
		return 1;
	}



	count_dsp_downloads(&main_data);

	while (!main_data.at_end) {
		get_next_verb(&main_data);
		if (main_data.cur_verb.codec == main_data.main_codec_id)
			check_verb(&main_data);
	}

	fclose(main_data.frames);

	return 0;
}
