/*
 * ca0132_base_functions:
 *
 * Functions used in multiple different ca0132 programs, basic 8051 exram
 * read/write functions, HCI read/write functions, DSP command functions, and
 * definitions of DSP command ID's/Control Flag ID's/Control Param ID's.
 */
#include "ca0132_defs.h"

const struct timespec timeout_val = {
	.tv_sec  = 0,
	.tv_nsec = 12500000,
};

/* Pack the two SCP verb structures for an SCP verb. */
static void pack_scp_verb_structs(uint32_t data, struct hda_verb_ioctl *v)
{
	uint32_t i;

	for (i = 0; i < 2; i++) {
		memset(&v[i], 0, sizeof(*v));
		v[i].verb = HDA_VERB(WIDGET_DSP_CTRL,
				DSPIO_SCP_WRITE_DATA_LOW + (i * 0x100),
				(data >> (i * 0x10)) & 0xffff);
	}
}


static int dspio_send_verb_with_status(int fd, struct hda_verb_ioctl *v)
{
	uint32_t i;

	for (i = 0; i < 6; i++) {
		ioctl(fd, HDA_IOCTL_VERB_WRITE, v);
		if ((v->res >= 0) && (v->res != STATUS_DSPIO_BUSY))
			return 0;

		nanosleep(&timeout_val, NULL);
	}

	return 1;
}

/*
 * Wait for DSP to be ready for commands
 */
static int dspio_write_wait(int fd)
{
	struct hda_verb_ioctl v;
	uint32_t i;

	v.verb = HDA_VERB(WIDGET_DSP_CTRL, DSPIO_STATUS, 0x00);
	for (i = 0; i < 6; i++) {
		ioctl(fd, HDA_IOCTL_VERB_WRITE, &v);
		if (!v.res)
			return 0;

		nanosleep(&timeout_val, NULL);
	}

	return 1;
}

/*
 * Write SCP data to DSP
 */
int dspio_write(int fd, uint32_t data)
{
	struct hda_verb_ioctl tmp[2], v;

        if (dspio_write_wait(fd))
		return 1;

	pack_scp_verb_structs(data, tmp);
        if (dspio_send_verb_with_status(fd, &tmp[0]))
		return 1;

        if (dspio_send_verb_with_status(fd, &tmp[1]))
		return 1;

        /* OK, now check if the write itself has executed*/
	v.verb = HDA_VERB(WIDGET_DSP_CTRL, DSPIO_STATUS, 0x00);
	ioctl(fd, HDA_IOCTL_VERB_WRITE, &v);
	if (v.res == STATUS_DSPIO_SCP_COMMAND_QUEUE_FULL)
		return 1;
	else
		return 0;
}

/* Functions for reading/writing ca0132's HIC bus. */
static uint32_t chipio_get_status(int fd)
{
	struct hda_verb_ioctl v;
	uint32_t i;

	v.verb = HDA_VERB(WIDGET_CHIP_CTRL, CHIPIO_STATUS, 0x00);
	for (i = 0; i < 6; i++) {
		ioctl(fd, HDA_IOCTL_VERB_WRITE, &v);
		if (!v.res)
			return 0;

		nanosleep(&timeout_val, NULL);
	}

	printf("ChipIO busy, can't process request.\n");

	return v.res;
}

static int chipio_verb_send_with_status(int fd, uint32_t verb, uint32_t data)
{
	struct hda_verb_ioctl v;
	uint32_t i;

	v.verb = HDA_VERB(WIDGET_CHIP_CTRL, verb & 0x0fff, data);
	for (i = 0; i < 4; i++) {
		ioctl(fd, HDA_IOCTL_VERB_WRITE, &v);

		if (!v.res)
			return 0;

		nanosleep(&timeout_val, NULL);
	}

	return v.res;
}

static int chipio_hic_read_data(int fd, uint32_t *data)
{
	struct hda_verb_ioctl verb;

	if (chipio_get_status(fd))
		return 1;

	chipio_verb_send_with_status(fd, CHIPIO_HIC_POST_READ, 0);

	if (chipio_get_status(fd))
		return 1;

	verb.verb = HDA_VERB(WIDGET_CHIP_CTRL, CHIPIO_HIC_READ_DATA, 0x00);
	ioctl(fd, HDA_IOCTL_VERB_WRITE, &verb);
	*data = verb.res;

        return 0;
}

static int chipio_hic_set_data(int fd, uint32_t data)
{
	uint32_t tmp[2], i;

	if (chipio_get_status(fd))
		return 1;

	for (i = 0; i < 2; i++) {
		tmp[0] = CHIPIO_DATA_LOW + (i * 0x100);
		tmp[1] = (data >> (16 * i)) & 0xffff;
		if (chipio_verb_send_with_status(fd, tmp[0], tmp[1])) {
			printf("%s: Failed to write %s.", __func__,
					i ? "DATA_HIGH" : "DATA_LOW");
			return 1;
		}
	}

	return 0;
}

static int chipio_hic_set_address(int fd, uint32_t addr)
{
	uint32_t tmp[2], i;

	if (chipio_get_status(fd))
		return 1;

	for (i = 0; i < 2; i++) {
		tmp[0] = CHIPIO_ADDRESS_LOW + (i * 0x100);
		tmp[1] = (addr >> (16 * i)) & 0xffff;
		if (chipio_verb_send_with_status(fd, tmp[0], tmp[1])) {
			printf("%s: Failed to write %s.", __func__,
					i ? "ADDRESS_HIGH" : "DATA_LOW");
			return 1;
		}
	}

	return 0;
}

void chipio_hic_write_at_addr(int fd, uint32_t addr, uint32_t data)
{
	if (chipio_hic_set_address(fd, addr)) {
		printf("%s: Failed to write address, try again.\n", __func__);
		return;
	}

	if (chipio_hic_set_data(fd, data))
		printf("%s: Failed to write data.\n", __func__);
}

void chipio_hic_write_data_range(int fd, uint32_t start_addr, uint32_t count,
		uint32_t *buf)
{
	uint32_t i;

	if (chipio_hic_set_address(fd, start_addr)) {
		printf("%s: Failed to write address, try again.\n", __func__);
		return;
	}

	for (i = 0; i < count; i++) {
		if (chipio_hic_set_data(fd, buf[i])) {
			printf("%s: failed to write data, aborting.\n", __func__);
			return;
		}
	}
}

uint32_t chipio_hic_read_at_addr(int fd, uint32_t addr)
{
	uint32_t data;

	if (chipio_hic_set_address(fd, addr)) {
		printf("%s: Failed to write address, try again.\n", __func__);
		return 0;
	}

	if (chipio_hic_read_data(fd, &data)) {
		printf("%s: failed to read data.\n", __func__);
		return 0;
	}

	return data;
}

void chipio_hic_read_data_range(int fd, uint32_t start_addr, uint32_t count,
		uint32_t *buf)
{
	uint32_t i;

	if (chipio_hic_set_address(fd, start_addr)) {
		printf("%s: Failed to write address, try again.\n", __func__);
		return;
	}

	for (i = 0; i < count; i++) {
		if (chipio_hic_read_data(fd, &buf[i])) {
			printf("%s: failed to read data, aborting.\n", __func__);
			return;
		}
	}
}

/* 8051 exram reading/writing functions. */
static uint8_t chipio_8051_read_exram_data(int fd)
{
	struct hda_verb_ioctl v;

	v.verb = HDA_VERB(WIDGET_CHIP_CTRL, CHIPIO_8051_DATA_READ, 0x00);
	ioctl(fd, HDA_IOCTL_VERB_WRITE, &v);

	return v.res & 0xff;
}

static uint8_t chipio_8051_read_pmem_data(int fd)
{
	struct hda_verb_ioctl v;

	v.verb = HDA_VERB(WIDGET_CHIP_CTRL, CHIPIO_8051_PMEM_READ, 0x00);
	ioctl(fd, HDA_IOCTL_VERB_WRITE, &v);

	return v.res & 0xff;
}

static void chipio_8051_set_addr(int fd, uint16_t addr)
{
	struct hda_verb_ioctl v;
	uint32_t i;

	for (i = 0; i < 2; i++) {
		memset(&v, 0, sizeof(v));
		v.verb = HDA_VERB(WIDGET_CHIP_CTRL,
				CHIPIO_8051_ADDRESS_LOW + i,
				(addr >> (i * 8)) & 0xff);
		ioctl(fd, HDA_IOCTL_VERB_WRITE, &v);
	}
}

static void chipio_8051_set_addr_lower(int fd, uint8_t addr)
{
	struct hda_verb_ioctl v;

	v.verb = HDA_VERB(WIDGET_CHIP_CTRL, CHIPIO_8051_ADDRESS_LOW, addr);
	ioctl(fd, HDA_IOCTL_VERB_WRITE, &v);
}

static void chipio_8051_set_exram_data(int fd, uint8_t data)
{
	struct hda_verb_ioctl v;

	v.verb = HDA_VERB(WIDGET_CHIP_CTRL, CHIPIO_8051_DATA_WRITE, data);
	ioctl(fd, HDA_IOCTL_VERB_WRITE, &v);
}

void chipio_8051_write_exram_at_addr(int fd, uint16_t addr, uint8_t data)
{
	chipio_8051_set_addr(fd, addr);
	chipio_8051_set_exram_data(fd, data);
}

uint8_t chipio_8051_read_exram_at_addr(int fd, uint16_t addr)
{
	chipio_8051_set_addr(fd, addr);

	return chipio_8051_read_exram_data(fd);
}

/*
 * Since reads don't automatically increment the address, we'll need to do it
 * ourselves. Sending the full 16-bits each time is two verbs, which is
 * slower. So, only send the upper 8 address bits if they change.
 */
void chipio_8051_read_exram_data_range(int fd, uint16_t start_addr, uint16_t count,
		uint8_t *buf)
{
	uint16_t i, cur_upper;

	cur_upper = 0xff;
	for (i = 0; i < count; i++) {
		if (((start_addr + i) & 0xff00) != cur_upper) {
			cur_upper = (start_addr + i) & 0xff00;
			chipio_8051_set_addr(fd, start_addr + i);
		} else {
			chipio_8051_set_addr_lower(fd, (start_addr + i) & 0xff);
		}

		buf[i] = chipio_8051_read_exram_data(fd);
	}
}

void chipio_8051_read_pmem_data_range(int fd, uint16_t start_addr, uint16_t count,
		uint8_t *buf)
{
	uint16_t i, cur_upper;

	cur_upper = 0xff;
	for (i = 0; i < count; i++) {
		if (((start_addr + i) & 0xff00) != cur_upper) {
			cur_upper = (start_addr + i) & 0xff00;
			chipio_8051_set_addr(fd, start_addr + i);
		} else {
			chipio_8051_set_addr_lower(fd, (start_addr + i) & 0xff);
		}

		buf[i] = chipio_8051_read_pmem_data(fd);
	}
}

/*
 * Automatically increments, so if we're writing a range of data, avoid
 * re-writing the address each time.
 */
void chipio_8051_write_exram_data_range(int fd, uint16_t start_addr, uint16_t count,
		const uint8_t *buf)
{
	uint16_t i;

	chipio_8051_set_addr(fd, start_addr);
	for (i = 0; i < count; i++)
		chipio_8051_set_exram_data(fd, buf[i]);
}

/*
 * Set/Get ChipIO flags.
 */
void chipio_set_control_flag(int fd, uint32_t flag, uint32_t set)
{
	struct hda_verb_ioctl v;
	uint32_t tmp;

	tmp = flag & 0x7f;
	if (set)
		tmp |= 0x80;

	v.verb = HDA_VERB(WIDGET_CHIP_CTRL, CHIPIO_FLAG_SET, tmp & 0xff);
	ioctl(fd, HDA_IOCTL_VERB_WRITE, &v);
}

uint8_t chipio_get_control_flag(int fd, uint32_t flag)
{
	struct hda_verb_ioctl v;

	v.verb = HDA_VERB(WIDGET_CHIP_CTRL, CHIPIO_FLAGS_GET, 0);
	ioctl(fd, HDA_IOCTL_VERB_WRITE, &v);

	return (v.res >> flag) & 0x01;
}

/*
 * Set/Get ChipIO paramID values.
 */
static void chipio_set_param_id(int fd, uint32_t param)
{
	struct hda_verb_ioctl v;

	v.verb = HDA_VERB(WIDGET_CHIP_CTRL, CHIPIO_PARAM_EX_ID_SET, param);
	ioctl(fd, HDA_IOCTL_VERB_WRITE, &v);
}

static void chipio_set_param_val(int fd, uint32_t val)
{
	struct hda_verb_ioctl v;

	v.verb = HDA_VERB(WIDGET_CHIP_CTRL, CHIPIO_PARAM_EX_VAL_SET, val);
	ioctl(fd, HDA_IOCTL_VERB_WRITE, &v);
}

static uint8_t chipio_get_param_val(int fd)
{
	struct hda_verb_ioctl v;

	v.verb = HDA_VERB(WIDGET_CHIP_CTRL, CHIPIO_PARAM_EX_VAL_GET, 0x00);
	ioctl(fd, HDA_IOCTL_VERB_WRITE, &v);

	return v.res & 0xff;
}

/* Set a ParamID value. */
void chipio_set_control_param(int fd, uint32_t param, uint8_t val)
{
	chipio_set_param_id(fd, param);
	chipio_set_param_val(fd, val);
}

/* Get a ParamID value. */
uint8_t chipio_get_control_param(int fd, uint32_t param)
{
	chipio_set_param_id(fd, param);

	return chipio_get_param_val(fd);
}

/*
 * DSP debug functions.
 */
void set_dsp_pc(int fd, uint32_t dsp, uint32_t addr)
{
	chipio_hic_write_at_addr(fd, 0x100e2c + (0x2000 * dsp), addr);
}

void set_dsp_dbg_single_step(int fd, uint32_t enable)
{
	uint32_t dbg_reg, halt_state, tmp;

	/* Read the debug register, discard upper 16-bits. */
	dbg_reg = chipio_hic_read_at_addr(fd, 0x100e30);
	dbg_reg &= 0x0000ffff;

	/* Halt state bits, four bits, seems to represent each DSP. */
	halt_state = dbg_reg >> 10;

	if (enable) {
		/*
		 * If we're already in a halt state, and the single step bits
		 * are set, do nothing.
		 */
		if ((halt_state == 0xf) && (dbg_reg & 0x000000f0))
			return;

		/* Set all halt bits. */
		tmp = dbg_reg | 0x00003cff;
	} else {
		if (!halt_state)
			return;

		/* Make sure the single step bits are unset. */
		tmp = dbg_reg & ~((halt_state << 4) & 0x000000f0);
		chipio_hic_write_at_addr(fd, 0x100e30, tmp);

		/* Set the execute bits. */
		tmp |= (halt_state & 0x0000000f);
	}

	chipio_hic_write_at_addr(fd, 0x100e30, tmp);
}

void dsp_run_steps(int fd, uint32_t step_cnt)
{
	uint32_t i, tmp;

	for (i = 0; i < step_cnt; i++) {
		tmp = chipio_hic_read_at_addr(fd, 0x100e30);
		tmp |= 0x0000000f;
		chipio_hic_write_at_addr(fd, 0x100e30, tmp);
	}
}

void dsp_run_steps_at_addr(int fd, uint32_t dsp, uint32_t addr, uint32_t step_cnt)
{
	set_dsp_pc(fd, dsp, addr);
	dsp_run_steps(fd, step_cnt);
}

/* Default HDA-verb string getting functions. */
static const struct hda_verb_info verb_info_table[] = {
	{ .name     = "AC_VERB_GET_STREAM_FORMAT",
	  .verb_val = 0xa00
	},
	{ .name = "AC_VERB_GET_AMP_GAIN_MUTE",
	  .verb_val = 0xb00,
	},

	{ .name = "AC_VERB_GET_PROC_COEF",
	  .verb_val = 0x0c00
	},
	{ .name = "AC_VERB_GET_COEF_INDEX",
	  .verb_val = 0x0d00
	},
	{ .name = "AC_VERB_PARAMETERS",
	  .verb_val = 0x0f00
	},
	{ .name = "AC_VERB_GET_CONNECT_SEL",
	  .verb_val = 0x0f01
	},
	{ .name = "AC_VERB_GET_CONNECT_LIST",
	  .verb_val = 0x0f02
	},
	{ .name = "AC_VERB_GET_PROC_STATE",
	  .verb_val = 0x0f03
	},
	{ .name = "AC_VERB_GET_SDI_SELECT",
	  .verb_val = 0x0f04
	},
	{ .name = "AC_VERB_GET_POWER_STATE",
	  .verb_val = 0x0f05
	},
	{ .name = "AC_VERB_GET_CONV",
	  .verb_val = 0x0f06
	},
	{ .name = "AC_VERB_GET_PIN_WIDGET_CONTROL",
	  .verb_val = 0x0f07
	},
	{ .name = "AC_VERB_GET_UNSOLICITED_RESPONSE",
	  .verb_val = 0x0f08
	},
	{ .name = "AC_VERB_GET_PIN_SENSE",
	  .verb_val = 0x0f09
	},
	{ .name = "AC_VERB_GET_BEEP_CONTROL",
	  .verb_val = 0x0f0a
	},
	{ .name = "AC_VERB_GET_EAPD_BTLENABLE",
	  .verb_val = 0x0f0c
	},
	{ .name = "AC_VERB_GET_DIGI_CONVERT_1",
	  .verb_val = 0x0f0d
	},
	{ .name = "AC_VERB_GET_DIGI_CONVERT_2",
	  .verb_val = 0x0f0e /* unused */
	},
	{ .name = "AC_VERB_GET_VOLUME_KNOB_CONTROL",
	  .verb_val = 0x0f0f
	},
	/* f10-f1a: GPIO */
	{ .name = "AC_VERB_GET_GPIO_DATA",
	  .verb_val = 0x0f15
	},
	{ .name = "AC_VERB_GET_GPIO_MASK",
	  .verb_val = 0x0f16
	},
	{ .name = "AC_VERB_GET_GPIO_DIRECTION",
	  .verb_val = 0x0f17
	},
	{ .name = "AC_VERB_GET_GPIO_WAKE_MASK",
	  .verb_val = 0x0f18
	},
	{ .name = "AC_VERB_GET_GPIO_UNSOLICITED_RSP_MASK",
	  .verb_val = 0x0f19
	},
	{ .name = "AC_VERB_GET_GPIO_STICKY_MASK",
	  .verb_val = 0x0f1a
	},
	{ .name = "AC_VERB_GET_CONFIG_DEFAULT",
	  .verb_val = 0x0f1c
	},
	/* f20: AFG/MFG */
	{ .name = "AC_VERB_GET_SUBSYSTEM_ID",
	  .verb_val = 0x0f20
	},
	{ .name = "AC_VERB_GET_STRIPE_CONTROL",
	  .verb_val = 0x0f24
	},
	{ .name = "AC_VERB_GET_CVT_CHAN_COUNT",
	  .verb_val = 0x0f2d
	},
	{ .name = "AC_VERB_GET_HDMI_DIP_SIZE",
	  .verb_val = 0x0f2e
	},
	{ .name = "AC_VERB_GET_HDMI_ELDD",
	  .verb_val = 0x0f2f
	},
	{ .name = "AC_VERB_GET_HDMI_DIP_INDEX",
	  .verb_val = 0x0f30
	},
	{ .name = "AC_VERB_GET_HDMI_DIP_DATA",
	  .verb_val = 0x0f31
	},
	{ .name = "AC_VERB_GET_HDMI_DIP_XMIT",
	  .verb_val = 0x0f32
	},
	{ .name = "AC_VERB_GET_HDMI_CP_CTRL",
	  .verb_val = 0x0f33
	},
	{ .name = "AC_VERB_GET_HDMI_CHAN_SLOT",
	  .verb_val = 0x0f34
	},
	{ .name = "AC_VERB_GET_DEVICE_SEL",
	  .verb_val = 0xf35
	},
	{ .name = "AC_VERB_GET_DEVICE_LIST",
	  .verb_val = 0xf36
	},
/*
 * SET verbs
 */
	{ .name = "AC_VERB_SET_STREAM_FORMAT",
	  .verb_val = 0x200
	},
	{ .name = "AC_VERB_SET_AMP_GAIN_MUTE",
	  .verb_val = 0x300
	},
	{ .name = "AC_VERB_SET_PROC_COEF",
	  .verb_val = 0x400
	},
	{ .name = "AC_VERB_SET_COEF_INDEX",
	  .verb_val = 0x500
	},
	{ .name = "AC_VERB_SET_CONNECT_SEL",
	  .verb_val = 0x701
	},
	{ .name = "AC_VERB_SET_PROC_STATE",
	  .verb_val = 0x703
	},
	{ .name = "AC_VERB_SET_SDI_SELECT",
	  .verb_val = 0x704
	},
	{ .name = "AC_VERB_SET_POWER_STATE",
	  .verb_val = 0x705
	},
	{ .name = "AC_VERB_SET_CHANNEL_STREAMID",
	  .verb_val = 0x706
	},
	{ .name = "AC_VERB_SET_PIN_WIDGET_CONTROL",
	  .verb_val = 0x707
	},
	{ .name = "AC_VERB_SET_UNSOLICITED_ENABLE",
	  .verb_val = 0x708
	},
	{ .name = "AC_VERB_SET_PIN_SENSE",
	  .verb_val = 0x709
	},
	{ .name = "AC_VERB_SET_BEEP_CONTROL",
	  .verb_val = 0x70a
	},
	{ .name = "AC_VERB_SET_EAPD_BTLENABLE",
	  .verb_val = 0x70c
	},
	{ .name = "AC_VERB_SET_DIGI_CONVERT_1",
	  .verb_val = 0x70d
	},
	{ .name = "AC_VERB_SET_DIGI_CONVERT_2",
	  .verb_val = 0x70e
	},
	{ .name = "AC_VERB_SET_DIGI_CONVERT_3",
	  .verb_val = 0x73e
	},
	{ .name = "AC_VERB_SET_VOLUME_KNOB_CONTROL",
	  .verb_val = 0x70f
	},
	{ .name = "AC_VERB_SET_GPIO_DATA",
	  .verb_val = 0x715
	},
	{ .name = "AC_VERB_SET_GPIO_MASK",
	  .verb_val = 0x716
	},
	{ .name = "AC_VERB_SET_GPIO_DIRECTION",
	  .verb_val = 0x717
	},
	{ .name = "AC_VERB_SET_GPIO_WAKE_MASK",
	  .verb_val = 0x718
	},
	{ .name = "AC_VERB_SET_GPIO_UNSOLICITED_RSP_MASK",
	  .verb_val = 0x719
	},
	{ .name = "AC_VERB_SET_GPIO_STICKY_MASK",
	  .verb_val = 0x71a
	},
	{ .name = "AC_VERB_SET_CONFIG_DEFAULT_BYTES_0",
	  .verb_val = 0x71c
	},
	{ .name = "AC_VERB_SET_CONFIG_DEFAULT_BYTES_1",
	  .verb_val = 0x71d
	},
	{ .name = "AC_VERB_SET_CONFIG_DEFAULT_BYTES_2",
	  .verb_val = 0x71e
	},
	{ .name = "AC_VERB_SET_CONFIG_DEFAULT_BYTES_3",
	  .verb_val = 0x71f
	},
	{ .name = "AC_VERB_SET_EAPD",
	  .verb_val = 0x788
	},
	{ .name = "AC_VERB_SET_CODEC_RESET",
	  .verb_val = 0x7ff
	},
	{ .name = "AC_VERB_SET_STRIPE_CONTROL",
	  .verb_val = 0x724
	},
	{ .name = "AC_VERB_SET_CVT_CHAN_COUNT",
	  .verb_val = 0x72d
	},
	{ .name = "AC_VERB_SET_HDMI_DIP_INDEX",
	  .verb_val = 0x730
	},
	{ .name = "AC_VERB_SET_HDMI_DIP_DATA",
	  .verb_val = 0x731
	},
	{ .name = "AC_VERB_SET_HDMI_DIP_XMIT",
	  .verb_val = 0x732
	},
	{ .name = "AC_VERB_SET_HDMI_CP_CTRL",
	  .verb_val = 0x733
	},
	{ .name = "AC_VERB_SET_HDMI_CHAN_SLOT",
	  .verb_val = 0x734
	},
	{ .name = "AC_VERB_SET_DEVICE_SEL",
	  .verb_val = 0x735
	}
};

const struct hda_verb_info *get_hda_verb_info(uint32_t verb)
{
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(verb_info_table); i++) {
		if (verb_info_table[i].verb_val == verb)
			return &verb_info_table[i];
	}

	return NULL;
}

/* ChipIO flagID/paramID string get functions. */
static const char *control_flag_id[]  = {
        /* Connection manager stream setup is bypassed/enabled */
        "CONTROL_FLAG_C_MGR",
        /* DSP DMA is bypassed/enabled */
        "CONTROL_FLAG_DMA",
        /* 8051 'idle' mode is disabled/enabled */
        "CONTROL_FLAG_IDLE_ENABLE",
        /* Tracker for the SPDIF-in path is bypassed/enabled */
        "CONTROL_FLAG_TRACKER",
        /* DigitalOut to Spdif2Out connection is disabled/enabled */
        "CONTROL_FLAG_SPDIF2OUT",
        /* Digital Microphone is disabled/enabled */
        "CONTROL_FLAG_DMIC",
        /* ADC_B rate is 48 kHz/96 kHz */
        "CONTROL_FLAG_ADC_B_96KHZ",
        /* ADC_C rate is 48 kHz/96 kHz */
        "CONTROL_FLAG_ADC_C_96KHZ",
        /* DAC rate is 48 kHz/96 kHz (affects all DACs) */
        "CONTROL_FLAG_DAC_96KHZ",
        /* DSP rate is 48 kHz/96 kHz */
        "CONTROL_FLAG_DSP_96KHZ",
        /* SRC clock is 98 MHz/196 MHz (196 MHz forces rate to 96 KHz) */
        "CONTROL_FLAG_SRC_CLOCK_196MHZ",
        /* SRC rate is 48 kHz/96 kHz (48 kHz disabled when clock is 196 MHz) */
        "CONTROL_FLAG_SRC_RATE_96KHZ",
        /* Decode Loop (DSP->SRC->DSP) is disabled/enabled */
        "CONTROL_FLAG_DECODE_LOOP",
        /* De-emphasis filter on DAC-1 disabled/enabled */
        "CONTROL_FLAG_DAC1_DEEMPHASIS",
        /* De-emphasis filter on DAC-2 disabled/enabled */
        "CONTROL_FLAG_DAC2_DEEMPHASIS",
        /* De-emphasis filter on DAC-3 disabled/enabled */
        "CONTROL_FLAG_DAC3_DEEMPHASIS",
        /* High-pass filter on ADC_B disabled/enabled */
        "CONTROL_FLAG_ADC_B_HIGH_PASS",
        /* High-pass filter on ADC_C disabled/enabled */
        "CONTROL_FLAG_ADC_C_HIGH_PASS",
        /* Common mode on Port_A disabled/enabled */
        "CONTROL_FLAG_PORT_A_COMMON_MODE",
        /* Common mode on Port_D disabled/enabled */
        "CONTROL_FLAG_PORT_D_COMMON_MODE",
        /* Impedance for ramp generator on Port_A 16 Ohm/10K Ohm */
        "CONTROL_FLAG_PORT_A_10KOHM_LOAD",
        /* Impedance for ramp generator on Port_D, 16 Ohm/10K Ohm */
        "CONTROL_FLAG_PORT_D_10KOHM_LOAD",
        /* ASI rate is 48kHz/96kHz */
        "CONTROL_FLAG_ASI_96KHZ",
        /* DAC power settings able to control attached ports no/yes */
        "CONTROL_FLAG_DACS_CONTROL_PORTS",
        /* Clock Stop OK reporting is disabled/enabled */
        "CONTROL_FLAG_CONTROL_STOP_OK_ENABLE",
};

const char *chipio_get_flag_str(uint32_t flag)
{
	if (flag > ARRAY_SIZE(control_flag_id))
		return NULL;
	else
		return control_flag_id[flag];
}

static const char *control_param_id[]  = {
	"dm",
	"vip (CONTROL_PARAM_VIP_SOURCE)",
	"spdif1 (CONTROL_PARAM_SPDIF1_SOURCE)",
	"mch",
	"gaout",
	"gaint",
	"gdout",
	"gdint",
	"ga16 (CONTROL_PARAM_PORTA_1600OHM_GAIN)",
	"ga10k",
	"gd16 (CONTROL_PARAM_PORTD_1600OHM_GAIN)",
	"gd10k",
	"dualhp",
	"dac2port",
	"mpce0",
	"mpce1",
	"mpce2",
	"mpce3",
	"mpce4",
	"mpce5",
	"mpce6",
	"mpce7",
	"pice",
	"asi",
	"sid (CONTROL_PARAM_STREAM_ID)",
	"scp (CONTROL_PARAM_STREAM_SOURCE_CONN_POINT)",
	"dcp (CONTROL_PARAM_STREAM_DEST_CONN_ID)",
	"sch (CONTROL_PARAM_STREAM_CHANNELS)",
	"sctl (CONTROL_PARAM_STREAM_CONTROL)",
	"cpid (CONTROL_PARAM_CONN_POINT_ID)",
	"cpsr (CONTROL_PARAM_CONN_POINT_SAM_RTE)",
	"nid (CONTROL_PARAM_HDA_NODE_ID)",
	"n2sid (CONTROL_PARAM_HDA_NODE_STREAM_ID)",
	"pi1d",
	"pi2d",
	"eapd",
};

const char *chipio_get_param_str(uint32_t param)
{
	if (param > ARRAY_SIZE(control_param_id))
		return NULL;
	else
		return control_param_id[param];
}


/* DSP SCP Command info structures. */
static const struct scp_cmd_info scp_cmds[] = {
	{ .name = "Surround Enable",
	  .mid = 0x96,
	  .req = 0,
	},
	{ .name = "Surround Level",
	  .mid = 0x96,
	  .req = 1,
	},
	{ .name = "Crystalizer Enable",
	  .mid = 0x96,
	  .req = 7,
	},
	{ .name = "Crystalizer Level",
	  .mid = 0x96,
	  .req = 8,
	},
	{ .name = "Dialog Plus Enable",
	  .mid = 0x96,
	  .req = 2,
	},
	{ .name = "Dialog Plus Level",
	  .mid = 0x96,
	  .req = 3,
	},
	{ .name = "Smart Volume Enable",
	  .mid = 0x96,
	  .req = 4,
	},
	{ .name = "Smart Volume Level",
	  .mid = 0x96,
	  .req = 5,
	},
	{ .name = "Smart Volume Preset",
	  .mid = 0x96,
	  .req = 6,
	},
	{ .name = "X-Bass Crossover",
	  .mid = 0x96,
	  .req = 23,
	},
	{ .name = "X-Bass Enable",
	  .mid = 0x96,
	  .req = 24,
	},
	{ .name = "X-Bass Level",
	  .mid = 0x96,
	  .req = 25,
	},
	{ .name = "Equalizer Enable",
	  .mid = 0x96,
	  .req = 9,
	},
	{ .name = "Equalizer Gain",
	  .mid = 0x96,
	  .req = 10,
	},
	{ .name = "Equalizer Band0",
	  .mid = 0x96,
	  .req = 11,
	},
	{ .name = "Equalizer Band1",
	  .mid = 0x96,
	  .req = 12,
	},
	{ .name = "Equalizer Band2",
	  .mid = 0x96,
	  .req = 13,
	},
	{ .name = "Equalizer Band3",
	  .mid = 0x96,
	  .req = 14,
	},
	{ .name = "Equalizer Band4",
	  .mid = 0x96,
	  .req = 15,
	},
	{ .name = "Equalizer Band5",
	  .mid = 0x96,
	  .req = 16,
	},
	{ .name = "Equalizer Band6",
	  .mid = 0x96,
	  .req = 17,
	},
	{ .name = "Equalizer Band7",
	  .mid = 0x96,
	  .req = 18,
	},
	{ .name = "Equalizer Band8",
	  .mid = 0x96,
	  .req = 19,
	},
	{ .name = "Equalizer Band9",
	  .mid = 0x96,
	  .req = 20,
	},
	{ .name = "Echo Cancellation",
	  .mid = 0x95,
	  .req = 0,
	},
	{ .name = "Echo Cancellation",
	  .mid = 0x95,
	  .req = 1,
	},
	{ .name = "Echo Cancellation",
	  .mid = 0x95,
	  .req = 2,
	},
	{ .name = "Echo Cancellation",
	  .mid = 0x95,
	  .req = 3,
	},
	{ .name = "Voice Focus",
	  .mid = 0x95,
	  .req = 6,
	},
	{ .name = "Voice Focus",
	  .mid = 0x95,
	  .req = 7,
	},
	{ .name = "Voice Focus",
	  .mid = 0x95,
	  .req = 8,
	},
	{ .name = "Voice Focus",
	  .mid = 0x95,
	  .req = 9,
	},
	{ .name = "Mic SVM",
	  .mid = 0x95,
	  .req = 44,
	},
	{ .name = "Mic SVM",
	  .mid = 0x95,
	  .req = 45,
	},
	{ .name = "Noise Reduction",
	  .mid = 0x95,
	  .req = 4,
	},
	{ .name = "Noise Reduction",
	  .mid = 0x95,
	  .req = 5,
	},
	{ .name = "VoiceFX",
	  .mid = 0x95,
	  .req = 10,
	},
	{ .name = "VoiceFX",
	  .mid = 0x95,
	  .req = 11,
	},
	{ .name = "VoiceFX",
	  .mid = 0x95,
	  .req = 12,
	},
	{ .name = "VoiceFX",
	  .mid = 0x95,
	  .req = 13,
	},
	{ .name = "VoiceFX",
	  .mid = 0x95,
	  .req = 14,
	},
	{ .name = "VoiceFX",
	  .mid = 0x95,
	  .req = 15,
	},
	{ .name = "VoiceFX",
	  .mid = 0x95,
	  .req = 16,
	},
	{ .name = "VoiceFX",
	  .mid = 0x95,
	  .req = 17,
	},
	{ .name = "VoiceFX",
	  .mid = 0x95,
	  .req = 18,
	},
        { .name = "SPEAKER_BASS_REDIRECT",
	  .mid = 0x96,
	  .req = 21,
	},
	{ .name = "SPEAKER_BASS_REDIRECT_XOVER_FREQ",
	  .mid = 0x96,
	  .req = 22,
	},
	{ .name = "SPEAKER_FULL_RANGE_FRONT_L_R",
	  .mid = 0x96,
	  .req = 26,
	},
	{ .name = "SPEAKER_FULL_RANGE_CENTER_LFE",
	  .mid = 0x96,
	  .req = 27,
	},
	{ .name = "SPEAKER_FULL_RANGE_REAR_L_R",
	  .mid = 0x96,
	  .req = 28,
	},
	{ .name = "SPEAKER_FULL_RANGE_SURROUND_L_R",
	  .mid = 0x96,
	  .req = 29,
	},
	{ .name = "SPEAKER_BASS_REDIRECT_SUB_GAIN",
	  .mid = 0x96,
	  .req = 30,
	},
	{ .name = "SPEAKER_TUNING_USE_SPEAKER_EQ",
	  .mid = 0x96,
	  .req = 31,
	},
	{ .name = "SPEAKER_TUNING_ENABLE_CENTER_EQ",
	  .mid = 0x96,
	  .req = 32,
	},
	{ .name = "SPEAKER_TUNING_FRONT_LEFT_VOL_LEVEL",
	  .mid = 0x96,
	  .req = 33,
	},
	{ .name = "SPEAKER_TUNING_FRONT_RIGHT_VOL_LEVEL",
	  .mid = 0x96,
	  .req = 34,
	},
	{ .name = "SPEAKER_TUNING_CENTER_VOL_LEVEL",
	  .mid = 0x96,
	  .req = 35,
	},
	{ .name = "SPEAKER_TUNING_LFE_VOL_LEVEL",
	  .mid = 0x96,
	  .req = 36,
	},
	{ .name = "SPEAKER_TUNING_REAR_LEFT_VOL_LEVEL",
	  .mid = 0x96,
	  .req = 37,
	},
	{ .name = "SPEAKER_TUNING_REAR_RIGHT_VOL_LEVEL",
	  .mid = 0x96,
	  .req = 38,
	},
	{ .name = "SPEAKER_TUNING_SURROUND_LEFT_VOL_LEVEL",
	  .mid = 0x96,
	  .req = 39,
	},
	{ .name = "SPEAKER_TUNING_SURROUND_RIGHT_VOL_LEVEL",
	  .mid = 0x96,
	  .req = 40,
	},
	{ .name = "SPEAKER_TUNING_FRONT_LEFT_INVERT",
	  .mid = 0x96,
	  .req = 41,
	},
	{ .name = "SPEAKER_TUNING_FRONT_RIGHT_INVERT",
	  .mid = 0x96,
	  .req = 42,
	},
	{ .name = "SPEAKER_TUNING_CENTER_INVERT",
	  .mid = 0x96,
	  .req = 43,
	},
	{ .name = "SPEAKER_TUNING_LFE_INVERT",
	  .mid = 0x96,
	  .req = 44,
	},
	{ .name = "SPEAKER_TUNING_REAR_LEFT_INVERT",
	  .mid = 0x96,
	  .req = 45,
	},
	{ .name = "SPEAKER_TUNING_REAR_RIGHT_INVERT",
	  .mid = 0x96,
	  .req = 46,
	},
	{ .name = "SPEAKER_TUNING_SURROUND_LEFT_INVERT",
	  .mid = 0x96,
	  .req = 47,
	},
	{ .name = "SPEAKER_TUNING_SURROUND_RIGHT_INVERT",
	  .mid = 0x96,
	  .req = 48,
	},
	{ .name = "SPEAKER_TUNING_FRONT_LEFT_DELAY",
	  .mid = 0x96,
	  .req = 49,
	},
	{ .name = "SPEAKER_TUNING_FRONT_RIGHT_DELAY",
	  .mid = 0x96,
	  .req = 50,
	},
	{ .name = "SPEAKER_TUNING_CENTER_DELAY",
	  .mid = 0x96,
	  .req = 51,
	},
	{ .name = "SPEAKER_TUNING_LFE_DELAY",
	  .mid = 0x96,
	  .req = 52,
	},
	{ .name = "SPEAKER_TUNING_REAR_LEFT_DELAY",
	  .mid = 0x96,
	  .req = 53,
	},
	{ .name = "SPEAKER_TUNING_REAR_RIGHT_DELAY",
	  .mid = 0x96,
	  .req = 54,
	},
	{ .name = "SPEAKER_TUNING_SURROUND_LEFT_DELAY",
	  .mid = 0x96,
	  .req = 55,
	},
	{ .name = "SPEAKER_TUNING_SURROUND_RIGHT_DELAY",
	  .mid = 0x96,
	  .req = 56,
	},
	{ .name = "SPEAKER_TUNING_MAIN_VOLUME",
	  .mid = 0x96,
	  .req = 57,
	},
	{ .name = "SPEAKER_TUNING_MUTE",
	  .mid = 0x96,
	  .req = 58,
	},
	{ .name = "Speaker EQ Select",
	  .mid = 0x8f,
	  .req = 0,
	},
	{ .name = "EQ Bypass Attenuation",
	  .mid = 0x8f,
	  .req = 1,
	},
	{ .name = "Mic1 Channel Config",
	  .mid = 0x80,
	  .req = 0,
	},
	{ .name = "Mic2 Channel Config",
	  .mid = 0x80,
	  .req = 1,
	},
	{ .name = "Speaker Channel Config",
	  .mid = 0x80,
	  .req = 4,
	},
	{ .name = "CrystalVoice Source",
	  .mid = 0x80,
	  .req = 5,
	},
	{ .name = "WUH Source",
	  .mid = 0x31,
	  .req = 0,
	},
	{ .name = "Output Source",
	  .mid = 0x32,
	  .req = 0,
	},
	{ .name = "Output Vol Update",
	  .mid = 0x32,
	  .req = 2,
	},
	{ .name = "Output Left Vol",
	  .mid = 0x32,
	  .req = 3,
	},
	{ .name = "Output Right Vol",
	  .mid = 0x32,
	  .req = 4,
	},
	{ .name = "Mic1 Vol Update",
	  .mid = 0x37,
	  .req = 1,
	},
	{ .name = "Mic1 Left Vol",
	  .mid = 0x37,
	  .req = 2,
	},
	{ .name = "Mic1 Right Vol",
	  .mid = 0x37,
	  .req = 3,
	},
	{ .name = "Mic1 Loopback Mute",
	  .mid = 0x37,
	  .req = 8,
	},
	{ .name = "Mic2 Loopback Mute",
	  .mid = 0x37,
	  .req = 16,
	},
	{ .name = "Mic Noise Reduction",
	  .mid = 0x47,
	  .req = 0,
	},
};

const struct scp_cmd_info *dsp_get_scp_cmd_info(uint32_t mid, uint32_t req)
{
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(scp_cmds); i++) {
		if (scp_cmds[i].mid == mid && scp_cmds[i].req == req)
			return &scp_cmds[i];
	}

	return NULL;
}

/*
 * Extract scp data from an SCP command header and store into an scp_data
 * structure.
 */
void get_scp_data(struct scp_data *data, uint32_t header)
{
	data->data_size   = (header >> 27) & 0x1f;
	data->error_flag  = (header >> 26) & 0x01;
	data->resp_flag   = (header >> 25) & 0x01;
	data->device_flag = (header >> 24) & 0x01;
	data->req         = (header >> 17) & 0x7f;
	data->get_flag    = (header >> 16) & 0x01;
	data->source_id   = (header >> 8) & 0xff;
	data->target_id   = header & 0xff;
}

void pack_scp_data(struct scp_data *data, uint32_t data_size,
		uint32_t req, uint32_t src_id, uint32_t target_id)
{
	memset(data, 0, sizeof(*data));

	data->data_size = data_size & 0x1f;
	data->req       = req & 0x7f;
	data->source_id = src_id & 0xff;
	data->target_id = target_id & 0xff;
}

uint32_t create_scp_header(struct scp_data *data)
{
	uint32_t header;

	header  = (data->data_size & 0x1f) << 27;
	header |= (data->error_flag & 0x01) << 26;
	header |= (data->resp_flag & 0x01) << 25;
	header |= (data->device_flag & 0x01) << 24;
	header |= (data->req & 0x7f) << 17;
	header |= (data->get_flag & 0x01) << 16;
	header |= (data->source_id & 0xff) << 8;
	header |= data->target_id & 0xff;

	return header;
}

/* hwdep device check function. */
int check_hwdep(int *fd)
{
	int version;

        version = 0;
        if (ioctl(*fd, HDA_IOCTL_PVERSION, &version) < 0) {
                perror("ioctl(PVERSION)");
                fprintf(stderr, "Looks like an invalid hwdep device...\n");
                return 1;
        }

        if (version < HDA_HWDEP_VERSION) {
                fprintf(stderr, "Invalid version number 0x%x\n", version);
                fprintf(stderr, "Looks like an invalid hwdep device...\n");
                return 1;
        }

	return 0;
}

int open_hwdep(char *dev, int *fd)
{
        *fd = open(dev, O_RDWR);
        if (fd < 0) {
                perror("open");
                return 1;
        }

	if (check_hwdep(fd))
		return 1;

	return 0;
}

