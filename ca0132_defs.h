#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/io.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

#include <stdint.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#include "hda_hwdep.h"

/*
 * Macros and function call definitions shared between different programs
 * in ca0132-tools.
 */
#define ARRAY_SIZE(array) \
    (sizeof(array) / sizeof(*array))

#define DSP_PMEM_ADDR_TO_HIC(a) ((a * 4) + 0x80000)
#define HIC_PMEM_ADDR_TO_DSP(a) ((a - 0x80000) / 4)

struct hda_verb_info {
	const char *name;
	uint32_t verb_val;
};

struct scp_cmd_info {
	const char *name;
	uint32_t mid;
	uint32_t req;
};

struct scp_data {
	uint32_t data_size;
	uint32_t error_flag;
	uint32_t resp_flag;
	uint32_t device_flag;
	uint32_t req;
	uint32_t get_flag;
	uint32_t source_id;
	uint32_t target_id;

	uint32_t val[0x20];
};

/* ca0132_base_functions.c function declarations. */
void ca0132_command_wait();
int dspio_write(int fd, uint32_t data);

void chipio_8051_write_exram_at_addr(int fd, uint16_t addr, uint8_t data);
uint8_t chipio_8051_read_exram_at_addr(int fd, uint16_t addr);
void chipio_8051_read_exram_data_range(int fd, uint16_t start_addr, uint16_t count,
		uint8_t *buf);
void chipio_8051_read_pmem_data_range(int fd, uint16_t start_addr, uint16_t count,
		uint8_t *buf);
void chipio_8051_write_exram_data_range(int fd, uint16_t start_addr, uint16_t count,
		const uint8_t *buf);

void chipio_hic_write_at_addr(int fd, uint32_t addr, uint32_t data);
uint32_t chipio_hic_read_at_addr(int fd, uint32_t addr);
void chipio_hic_write_data_range(int fd, uint32_t start_addr, uint32_t count,
		uint32_t *buf);
void chipio_hic_read_data_range(int fd, uint32_t start_addr, uint32_t count,
		uint32_t *buf);

void chipio_set_control_flag(int fd, uint32_t flag, uint32_t set);
uint8_t chipio_get_control_flag(int fd, uint32_t flag);
void chipio_set_control_param(int fd, uint32_t param, uint8_t val);
uint8_t chipio_get_control_param(int fd, uint32_t param);

void set_dsp_pc(int fd, uint32_t dsp, uint32_t addr);
void set_dsp_dbg_single_step(int fd, uint32_t enable);
void dsp_run_steps(int fd, uint32_t step_cnt);
void dsp_run_steps_at_addr(int fd, uint32_t dsp, uint32_t addr, uint32_t step_cnt);

const struct hda_verb_info *get_hda_verb_info(uint32_t verb);
const char *chipio_get_flag_str(uint32_t flag);
const char *chipio_get_param_str(uint32_t param);

const struct scp_cmd_info *dsp_get_scp_cmd_info(uint32_t mid, uint32_t req);
void get_scp_data(struct scp_data *scp_data, uint32_t header);
void pack_scp_data(struct scp_data *data, uint32_t data_size,
		uint32_t req, uint32_t src_id, uint32_t target_id);
uint32_t create_scp_header(struct scp_data *data);

int check_hwdep(int *fd);
int open_hwdep(char *dev, int *fd);

/* HDA node ID's. */
#define       WIDGET_CHIP_CTRL               0x15
#define       WIDGET_DSP_CTRL                0x16

/*
 * Verb definitions.
 */
/*
 * ChipIO verbs.
 */
#define CHIPIO_STATUS                  0xF01
#define CHIPIO_ADDRESS_LOW             0x000
#define CHIPIO_ADDRESS_HIGH            0x100
#define CHIPIO_DATA_LOW                0x300
#define CHIPIO_DATA_HIGH               0x400
#define CHIPIO_HIC_POST_READ           0x702
#define CHIPIO_HIC_READ_DATA           0xF03
#define CHIPIO_FLAG_SET                0x70F
#define CHIPIO_FLAGS_GET               0xF0F
#define	CHIPIO_PARAM_EX_ID_SET         0x717
#define	CHIPIO_PARAM_EX_VAL_GET        0xF18
#define	CHIPIO_PARAM_EX_VAL_SET        0x718
#define CHIPIO_CT_EXTENSIONS_ENABLE    0x70A

#define CHIPIO_8051_DATA_READ          0xF07
#define CHIPIO_8051_DATA_WRITE         0x707
#define CHIPIO_8051_PMEM_READ          0xF08
#define CHIPIO_8051_ADDRESS_LOW        0x70D
#define CHIPIO_8051_ADDRESS_HIGH       0x70E
#define CHIPIO_8051_IRAM_INDIRECT_READ 0xF09
#define CHIPIO_8051_IRAM_DIRECT_READ   0xD00
/*
 * DSP Verbs.
 */
#define DSPIO_SCP_WRITE_DATA_LOW       0x000
#define DSPIO_SCP_WRITE_DATA_HIGH      0x100
                                            
#define DSPIO_STATUS                   0xF01
#define DSPIO_SCP_POST_READ_DATA       0x702
#define DSPIO_SCP_READ_DATA            0xF02
#define DSPIO_DSP_INIT                 0x703
#define DSPIO_SCP_POST_COUNT_QUERY     0x704
#define DSPIO_SCP_READ_COUNT           0xF04

#define STATUS_DSPIO_OK                        0x00
#define STATUS_DSPIO_BUSY                      0x01
#define STATUS_DSPIO_SCP_COMMAND_QUEUE_FULL    0x02
#define STATUS_DSPIO_SCP_RESPONSE_QUEUE_EMPTY  0x03


/* From patch_ca0132.c */
enum hda_vendor_status_dspio {
        /* Success */
        VENDOR_STATUS_DSPIO_OK                       = 0x00,
        /* Busy, unable to accept new command, the host must retry */
        VENDOR_STATUS_DSPIO_BUSY                     = 0x01,
        /* SCP command queue is full */
        VENDOR_STATUS_DSPIO_SCP_COMMAND_QUEUE_FULL   = 0x02,
        /* SCP response queue is empty */
        VENDOR_STATUS_DSPIO_SCP_RESPONSE_QUEUE_EMPTY = 0x03
};

enum hda_vendor_status_chipio {
        /* Success */
        VENDOR_STATUS_CHIPIO_OK   = 0x00,
        /* Busy, unable to accept new command, the host must retry */
        VENDOR_STATUS_CHIPIO_BUSY = 0x01
};

enum hda_cmd_vendor_io {
        /* for DspIO node */
        VENDOR_DSPIO_SCP_WRITE_DATA_LOW      = 0x000,
        VENDOR_DSPIO_SCP_WRITE_DATA_HIGH     = 0x100,

        VENDOR_DSPIO_STATUS                  = 0xF01,
        VENDOR_DSPIO_SCP_POST_READ_DATA      = 0x702,
        VENDOR_DSPIO_SCP_READ_DATA           = 0xF02,
        VENDOR_DSPIO_DSP_INIT                = 0x703,
        VENDOR_DSPIO_SCP_POST_COUNT_QUERY    = 0x704,
        VENDOR_DSPIO_SCP_READ_COUNT          = 0xF04,

        /* for ChipIO node */
        VENDOR_CHIPIO_ADDRESS_LOW            = 0x000,
        VENDOR_CHIPIO_ADDRESS_HIGH           = 0x100,
        VENDOR_CHIPIO_STREAM_FORMAT          = 0x200,
        VENDOR_CHIPIO_DATA_LOW               = 0x300,
        VENDOR_CHIPIO_DATA_HIGH              = 0x400,

        VENDOR_CHIPIO_8051_WRITE_DIRECT      = 0x500,
        VENDOR_CHIPIO_8051_READ_DIRECT       = 0xD00,

        VENDOR_CHIPIO_GET_PARAMETER          = 0xF00,
        VENDOR_CHIPIO_STATUS                 = 0xF01,
        VENDOR_CHIPIO_HIC_POST_READ          = 0x702,
        VENDOR_CHIPIO_HIC_READ_DATA          = 0xF03,

        VENDOR_CHIPIO_8051_DATA_WRITE        = 0x707,
        VENDOR_CHIPIO_8051_DATA_READ         = 0xF07,
        VENDOR_CHIPIO_8051_PMEM_READ         = 0xF08,
        VENDOR_CHIPIO_8051_IRAM_WRITE        = 0x709,
        VENDOR_CHIPIO_8051_IRAM_READ         = 0xF09,

        VENDOR_CHIPIO_CT_EXTENSIONS_ENABLE   = 0x70A,
        VENDOR_CHIPIO_CT_EXTENSIONS_GET      = 0xF0A,

        VENDOR_CHIPIO_PLL_PMU_WRITE          = 0x70C,
        VENDOR_CHIPIO_PLL_PMU_READ           = 0xF0C,
        VENDOR_CHIPIO_8051_ADDRESS_LOW       = 0x70D,
        VENDOR_CHIPIO_8051_ADDRESS_HIGH      = 0x70E,
        VENDOR_CHIPIO_FLAG_SET               = 0x70F,
        VENDOR_CHIPIO_FLAGS_GET              = 0xF0F,
        VENDOR_CHIPIO_PARAM_SET              = 0x710,
        VENDOR_CHIPIO_PARAM_GET              = 0xF10,

        VENDOR_CHIPIO_PORT_ALLOC_CONFIG_SET  = 0x711,
        VENDOR_CHIPIO_PORT_ALLOC_SET         = 0x712,
        VENDOR_CHIPIO_PORT_ALLOC_GET         = 0xF12,
        VENDOR_CHIPIO_PORT_FREE_SET          = 0x713,

        VENDOR_CHIPIO_PARAM_EX_ID_GET        = 0xF17,
        VENDOR_CHIPIO_PARAM_EX_ID_SET        = 0x717,
        VENDOR_CHIPIO_PARAM_EX_VALUE_GET     = 0xF18,
        VENDOR_CHIPIO_PARAM_EX_VALUE_SET     = 0x718,

        VENDOR_CHIPIO_DMIC_CTL_SET           = 0x788,
        VENDOR_CHIPIO_DMIC_CTL_GET           = 0xF88,
        VENDOR_CHIPIO_DMIC_PIN_SET           = 0x789,
        VENDOR_CHIPIO_DMIC_PIN_GET           = 0xF89,
        VENDOR_CHIPIO_DMIC_MCLK_SET          = 0x78A,
        VENDOR_CHIPIO_DMIC_MCLK_GET          = 0xF8A,

        VENDOR_CHIPIO_EAPD_SEL_SET           = 0x78D
};

enum speaker_range_reqs {
        SPEAKER_BASS_REDIRECT            = 0x15,
        SPEAKER_BASS_REDIRECT_XOVER_FREQ = 0x16,
        /* Between 0x16-0x1a are the X-Bass reqs. */
        SPEAKER_FULL_RANGE_FRONT_L_R     = 0x1a,
        SPEAKER_FULL_RANGE_CENTER_LFE    = 0x1b,
        SPEAKER_FULL_RANGE_REAR_L_R      = 0x1c,
        SPEAKER_FULL_RANGE_SURROUND_L_R  = 0x1d,
        SPEAKER_BASS_REDIRECT_SUB_GAIN   = 0x1e,
};

/*
 * Definitions for the DSP req's to handle speaker tuning. These all belong to
 * module ID 0x96, the output effects module.
 */
enum speaker_tuning_reqs {
        /*
         * Currently, this value is always set to 0.0f. However, on Windows,
         * when selecting certain headphone profiles on the new Sound Blaster
         * connect software, the QUERY_SPEAKER_EQ_ADDRESS req on mid 0x80 is
         * sent. This gets the speaker EQ address area, which is then used to
         * send over (presumably) an equalizer profile for the specific
         * headphone setup. It is sent using the same method the DSP
         * firmware is uploaded with, which I believe is why the 'ctspeq.bin'
         * file exists in linux firmware tree but goes unused. It would also
         * explain why the QUERY_SPEAKER_EQ_ADDRESS req is defined but unused.
         * Once this profile is sent over, SPEAKER_TUNING_USE_SPEAKER_EQ is
         * set to 1.0f.
         */
        SPEAKER_TUNING_USE_SPEAKER_EQ           = 0x1f,
        SPEAKER_TUNING_ENABLE_CENTER_EQ         = 0x20,
        SPEAKER_TUNING_FRONT_LEFT_VOL_LEVEL     = 0x21,
        SPEAKER_TUNING_FRONT_RIGHT_VOL_LEVEL    = 0x22,
        SPEAKER_TUNING_CENTER_VOL_LEVEL         = 0x23,
        SPEAKER_TUNING_LFE_VOL_LEVEL            = 0x24,
        SPEAKER_TUNING_REAR_LEFT_VOL_LEVEL      = 0x25,
        SPEAKER_TUNING_REAR_RIGHT_VOL_LEVEL     = 0x26,
        SPEAKER_TUNING_SURROUND_LEFT_VOL_LEVEL  = 0x27,
        SPEAKER_TUNING_SURROUND_RIGHT_VOL_LEVEL = 0x28,
        /*
         * Inversion is used when setting headphone virtualization to line
         * out. Not sure why this is, but it's the only place it's ever used.
         */
        SPEAKER_TUNING_FRONT_LEFT_INVERT        = 0x29,
        SPEAKER_TUNING_FRONT_RIGHT_INVERT       = 0x2a,
        SPEAKER_TUNING_CENTER_INVERT            = 0x2b,
        SPEAKER_TUNING_LFE_INVERT               = 0x2c,
        SPEAKER_TUNING_REAR_LEFT_INVERT         = 0x2d,
        SPEAKER_TUNING_REAR_RIGHT_INVERT        = 0x2e,
        SPEAKER_TUNING_SURROUND_LEFT_INVERT     = 0x2f,
        SPEAKER_TUNING_SURROUND_RIGHT_INVERT    = 0x30,
        /* Delay is used when setting surround speaker distance in Windows. */
        SPEAKER_TUNING_FRONT_LEFT_DELAY         = 0x31,
        SPEAKER_TUNING_FRONT_RIGHT_DELAY        = 0x32,
        SPEAKER_TUNING_CENTER_DELAY             = 0x33,
        SPEAKER_TUNING_LFE_DELAY                = 0x34,
        SPEAKER_TUNING_REAR_LEFT_DELAY          = 0x35,
        SPEAKER_TUNING_REAR_RIGHT_DELAY         = 0x36,
        SPEAKER_TUNING_SURROUND_LEFT_DELAY      = 0x37,
        SPEAKER_TUNING_SURROUND_RIGHT_DELAY     = 0x38,
        /* Of these two, only mute seems to ever be used. */
        SPEAKER_TUNING_MAIN_VOLUME              = 0x39,
        SPEAKER_TUNING_MUTE                     = 0x3a,
};

/* Borrowed from kernel's include/sound/hda_verbs.h file. */
/*
 * GET verbs
 */
#define AC_VERB_GET_STREAM_FORMAT		0x0a00
#define AC_VERB_GET_AMP_GAIN_MUTE		0x0b00
#define AC_VERB_GET_PROC_COEF			0x0c00
#define AC_VERB_GET_COEF_INDEX			0x0d00
#define AC_VERB_PARAMETERS			0x0f00
#define AC_VERB_GET_CONNECT_SEL			0x0f01
#define AC_VERB_GET_CONNECT_LIST		0x0f02
#define AC_VERB_GET_PROC_STATE			0x0f03
#define AC_VERB_GET_SDI_SELECT			0x0f04
#define AC_VERB_GET_POWER_STATE			0x0f05
#define AC_VERB_GET_CONV			0x0f06
#define AC_VERB_GET_PIN_WIDGET_CONTROL		0x0f07
#define AC_VERB_GET_UNSOLICITED_RESPONSE	0x0f08
#define AC_VERB_GET_PIN_SENSE			0x0f09
#define AC_VERB_GET_BEEP_CONTROL		0x0f0a
#define AC_VERB_GET_EAPD_BTLENABLE		0x0f0c
#define AC_VERB_GET_DIGI_CONVERT_1		0x0f0d
#define AC_VERB_GET_DIGI_CONVERT_2		0x0f0e /* unused */
#define AC_VERB_GET_VOLUME_KNOB_CONTROL		0x0f0f
/* f10-f1a: GPIO */
#define AC_VERB_GET_GPIO_DATA			0x0f15
#define AC_VERB_GET_GPIO_MASK			0x0f16
#define AC_VERB_GET_GPIO_DIRECTION		0x0f17
#define AC_VERB_GET_GPIO_WAKE_MASK		0x0f18
#define AC_VERB_GET_GPIO_UNSOLICITED_RSP_MASK	0x0f19
#define AC_VERB_GET_GPIO_STICKY_MASK		0x0f1a
#define AC_VERB_GET_CONFIG_DEFAULT		0x0f1c
/* f20: AFG/MFG */
#define AC_VERB_GET_SUBSYSTEM_ID		0x0f20
#define AC_VERB_GET_STRIPE_CONTROL		0x0f24
#define AC_VERB_GET_CVT_CHAN_COUNT		0x0f2d
#define AC_VERB_GET_HDMI_DIP_SIZE		0x0f2e
#define AC_VERB_GET_HDMI_ELDD			0x0f2f
#define AC_VERB_GET_HDMI_DIP_INDEX		0x0f30
#define AC_VERB_GET_HDMI_DIP_DATA		0x0f31
#define AC_VERB_GET_HDMI_DIP_XMIT		0x0f32
#define AC_VERB_GET_HDMI_CP_CTRL		0x0f33
#define AC_VERB_GET_HDMI_CHAN_SLOT		0x0f34
#define AC_VERB_GET_DEVICE_SEL			0xf35
#define AC_VERB_GET_DEVICE_LIST			0xf36

/*
 * SET verbs
 */
#define AC_VERB_SET_STREAM_FORMAT		0x200
#define AC_VERB_SET_AMP_GAIN_MUTE		0x300
#define AC_VERB_SET_PROC_COEF			0x400
#define AC_VERB_SET_COEF_INDEX			0x500
#define AC_VERB_SET_CONNECT_SEL			0x701
#define AC_VERB_SET_PROC_STATE			0x703
#define AC_VERB_SET_SDI_SELECT			0x704
#define AC_VERB_SET_POWER_STATE			0x705
#define AC_VERB_SET_CHANNEL_STREAMID		0x706
#define AC_VERB_SET_PIN_WIDGET_CONTROL		0x707
#define AC_VERB_SET_UNSOLICITED_ENABLE		0x708
#define AC_VERB_SET_PIN_SENSE			0x709
#define AC_VERB_SET_BEEP_CONTROL		0x70a
#define AC_VERB_SET_EAPD_BTLENABLE		0x70c
#define AC_VERB_SET_DIGI_CONVERT_1		0x70d
#define AC_VERB_SET_DIGI_CONVERT_2		0x70e
#define AC_VERB_SET_DIGI_CONVERT_3		0x73e
#define AC_VERB_SET_VOLUME_KNOB_CONTROL		0x70f
#define AC_VERB_SET_GPIO_DATA			0x715
#define AC_VERB_SET_GPIO_MASK			0x716
#define AC_VERB_SET_GPIO_DIRECTION		0x717
#define AC_VERB_SET_GPIO_WAKE_MASK		0x718
#define AC_VERB_SET_GPIO_UNSOLICITED_RSP_MASK	0x719
#define AC_VERB_SET_GPIO_STICKY_MASK		0x71a
#define AC_VERB_SET_CONFIG_DEFAULT_BYTES_0	0x71c
#define AC_VERB_SET_CONFIG_DEFAULT_BYTES_1	0x71d
#define AC_VERB_SET_CONFIG_DEFAULT_BYTES_2	0x71e
#define AC_VERB_SET_CONFIG_DEFAULT_BYTES_3	0x71f
#define AC_VERB_SET_EAPD			0x788
#define AC_VERB_SET_CODEC_RESET			0x7ff
#define AC_VERB_SET_STRIPE_CONTROL		0x724
#define AC_VERB_SET_CVT_CHAN_COUNT		0x72d
#define AC_VERB_SET_HDMI_DIP_INDEX		0x730
#define AC_VERB_SET_HDMI_DIP_DATA		0x731
#define AC_VERB_SET_HDMI_DIP_XMIT		0x732
#define AC_VERB_SET_HDMI_CP_CTRL		0x733
#define AC_VERB_SET_HDMI_CHAN_SLOT		0x734
#define AC_VERB_SET_DEVICE_SEL			0x735

/*
 * HIC Addresses.
 */
#define       XRAM_START_ADDRESS                    0x0
#define       AXRAM_START_ADDRESS                   0x3C000
#define       YRAM_START_ADDRESS                    0x40000
#define       AYRAM_START_ADDRESS                   0x78000
#define       UC_CHIP_START_ADDRESS                 0x80000
#define       DSP_CHIP_START_ADDRESS                0x100000
#define       DSP_DMA_CONFIG_START_ADDRESS          0x110F00

#define       XRAM_END_ADDRESS                      0x38000
#define       AXRAM_END_ADDRESS                     0x40000
#define       YRAM_END_ADDRESS                      0x60000
#define       AYRAM_END_ADDRESS                     0x7C000
#define       UC_CHIP_END_ADDRESS                   0xC0000
#define       DSP_CHIP_END_ADDRESS                  0x110000
#define       DSP_DMA_CONFIG_END_ADDRESS            0x110FC0

#define       DSP0IO_START 			0x100000
#define       DSP0IO_END	 		0x101FFC
#define       AUDIORINGIPDSP0_START 		0x100000
#define       AUDIORINGIPDSP0_END 		0x1003FC
#define       AUDIORINGOPDSP0_START 		0x100400
#define       AUDIORINGOPDSP0_END 		0x1007FC
#define       AUDPARARINGIODSP0_START		0x100800
#define       AUDPARARINGIODSP0_END	 	0x100BFC
#define       DSP0LOCALHWREG_START 		0x100C00
#define       DSP0LOCALHWREG_END	 	0x100C3C
#define       DSP0XYRAMAGINDEX_START 		0x100C40
#define       DSP0XYRAMAGINDEX_END	 	0x100C5C
#define       DSP0XYRAMAGMDFR_START 		0x100C60
#define       DSP0XYRAMAGMDFR_END	 	0x100C7C
#define       DSP0INTCONTLVEC_START 		0x100C80
#define       DSP0INTCONTLVEC_END	 	0x100CD8
#define       INTCONTLGLOBALREG_START		0x100D1C
#define       INTCONTLGLOBALREG_END	 	0x100D3C
#define       HOSTINTFPORTADDRCONTDSP0		0x100D40
#define       HOSTINTFPORTDATADSP0		0x100D44
#define       TIME0PERENBDSP0			0x100D60
#define       TIME0COUNTERDSP0			0x100D64
#define       TIME1PERENBDSP0			0x100D68
#define       TIME1COUNTERDSP0			0x100D6C
#define       TIME2PERENBDSP0			0x100D70
#define       TIME2COUNTERDSP0			0x100D74
#define       TIME3PERENBDSP0			0x100D78
#define       TIME3COUNTERDSP0			0x100D7C
#define       XRAMINDOPERREFNOUP_STARTDSP0 	0x100D80
#define       XRAMINDOPERREFNOUP_ENDDSP0	0x100D9C
#define       XRAMINDOPERREFUP_STARTDSP0	0x100DA0
#define       XRAMINDOPERREFUP_ENDDSP0		0x100DBC
#define       YRAMINDOPERREFNOUP_STARTDSP0 	0x100DC0
#define       YRAMINDOPERREFNOUP_ENDDSP0 	0x100DDC
#define       YRAMINDOPERREFUP_STARTDSP0	0x100DE0
#define       YRAMINDOPERREFUP_ENDDSP0		0x100DFC
#define       DSP0CONDCODE 			0x100E00
#define       DSP0STACKFLAG 			0x100E04
#define       DSP0PROGCOUNTSTACKPTREG		0x100E08
#define       DSP0PROGCOUNTSTACKDATAREG 	0x100E0C
#define       DSP0CURLOOPADDRREG 		0x100E10
#define       DSP0CURLOOPCOUNT			0x100E14
#define       DSP0TOPLOOPCOUNTSTACK 		0x100E18
#define       DSP0TOPLOOPADDRSTACK 		0x100E1C
#define       DSP0LOOPSTACKPTR			0x100E20
#define       DSP0STASSTACKDATAREG 		0x100E24
#define       DSP0STASSTACKPTR			0x100E28
#define       DSP0PROGCOUNT 			0x100E2C
#define       GLOBDSPDEBGREG			0x100E30
#define       GLOBDSPBREPTRREG			0x100E30
#define       DSP0XYRAMBASE_START 		0x100EA0
#define       DSP0XYRAMBASE_END 		0x100EBC
#define       DSP0XYRAMLENG_START 		0x100EC0
#define       DSP0XYRAMLENG_END 		0x100EDC
#define       SEMAPHOREREGDSP0			0x100EE0
#define       DSP0INTCONTMASKREG		0x100EE4
#define       DSP0INTCONTPENDREG		0x100EE8
#define       DSP0INTCONTSERVINT		0x100EEC
#define       DSPINTCONTEXTINTMODREG		0x100EEC
#define       GPIODSP0				0x100EFC
#define       DMADSPBASEADDRREG_STARTDSP0	0x100F00
#define       DMADSPBASEADDRREG_ENDDSP0		0x100F1C
#define       DMAHOSTBASEADDRREG_STARTDSP0	0x100F20
#define       DMAHOSTBASEADDRREG_ENDDSP0	0x100F3C
#define       DMADSPCURADDRREG_STARTDSP0	0x100F40
#define       DMADSPCURADDRREG_ENDDSP0		0x100F5C
#define       DMAHOSTCURADDRREG_STARTDSP0	0x100F60
#define       DMAHOSTCURADDRREG_ENDDSP0		0x100F7C
#define       DMATANXCOUNTREG_STARTDSP0		0x100F80
#define       DMATANXCOUNTREG_ENDDSP0		0x100F9C
#define       DMATIMEBUGREG_STARTDSP0		0x100FA0
#define       DMATIMEBUGREG_ENDDSP0		0x100FAC
#define       DMACNTLMODFREG_STARTDSP0		0x100FA0
#define       DMACNTLMODFREG_ENDDSP0		0x100FAC

#define       DMAGLOBSTATSREGDSP0		0x100FEC
#define       DSP0XGPRAM_START			0x101000
#define       DSP0XGPRAM_END 			0x1017FC
#define       DSP0YGPRAM_START 		        0x101800
#define       DSP0YGPRAM_END 			0x101FFC

/*
 * DSP definitions.
 */

/*
 * Types of opcode operands.
 */
enum op_operand_type {
	OP_OPERAND_REG_2,
	OP_OPERAND_REG_2_4,
	OP_OPERAND_REG_2_8,
	OP_OPERAND_REG_2_12,
	OP_OPERAND_REG_2_T1,
	OP_OPERAND_REG_2_T2,
	OP_OPERAND_REG_3_X_T1,
	OP_OPERAND_REG_3_Y_T1,
	OP_OPERAND_REG_3_X_T2,
	OP_OPERAND_REG_3_Y_T2,
	OP_OPERAND_REG_3,
	OP_OPERAND_REG_3_8,
	OP_OPERAND_REG_3_FMA,
	OP_OPERAND_REG_3_ACC,
	OP_OPERAND_REG_3_FMA_Y_T1,
	OP_OPERAND_REG_3_FMA_X_T1,
	OP_OPERAND_REG_3_FMA_Y_T2,
	OP_OPERAND_REG_3_FMA_X_T2,
	OP_OPERAND_REG_3_FMA_A_T1,
	OP_OPERAND_REG_4,
	OP_OPERAND_REG_5,
	OP_OPERAND_REG_7,
	OP_OPERAND_REG_9,
	OP_OPERAND_REG_10,
	OP_OPERAND_REG_11,
	OP_OPERAND_REG_11_10_OFFSET,
	OP_OPERAND_REG_11_4_OFFSET,
	OP_OPERAND_A_REG_CALL,
	OP_OPERAND_A_REG_CALL_MDFR,
	OP_OPERAND_REG_TYPE_END,
	OP_OPERAND_A_REG,
	OP_OPERAND_A_REG_X,
	OP_OPERAND_A_REG_Y,
	OP_OPERAND_A_REG_X_INC,
	OP_OPERAND_A_REG_Y_INC,
	OP_OPERAND_A_REG_PLUS_MDFR,
	OP_OPERAND_A_REG_X_PLUS_MDFR,
	OP_OPERAND_A_REG_Y_PLUS_MDFR,
	OP_OPERAND_A_REG_X_MDFR_OFFSET,
	OP_OPERAND_A_REG_Y_MDFR_OFFSET,
	OP_OPERAND_A_REG_INT_7_OFFSET,
	OP_OPERAND_A_REG_X_INT_11_OFFSET,
	OP_OPERAND_A_REG_Y_INT_11_OFFSET,
	OP_OPERAND_A_REG_INT_17_OFFSET,
	OP_OPERAND_LITERAL_7_INT,
	OP_OPERAND_LITERAL_8_INT,
	OP_OPERAND_LITERAL_8_INT_PC_OFFSET,
	OP_OPERAND_LITERAL_16_INT,
	OP_OPERAND_LITERAL_17_INT,
	OP_OPERAND_LITERAL_8,
	OP_OPERAND_LITERAL_11,
	OP_OPERAND_LITERAL_16,
	OP_OPERAND_LITERAL_16_UPPER,
	OP_OPERAND_LITERAL_16_ADDR,
	OP_OPERAND_LITERAL_32,
	OP_OPERAND_NOP,
};

enum op_operand_dir {
	OPERAND_DIR_NONE,
	OPERAND_DIR_SRC,
	OPERAND_DIR_DST,
	OPERAND_DIR_X,
	OPERAND_DIR_Y,
	OPERAND_DIR_A,
};

enum op_operand_modifier {
	OPERAND_MDFR_NONE,
	OPERAND_MDFR_INC,
	OPERAND_MDFR_DEC,
	OPERAND_MDFR_OFFSET,
	OPERAND_MDFR_UPPER_BITS,
	OPERAND_MDFR_RR,
	OPERAND_MDFR_RL,
	OPERAND_MDFR_PARALLEL,
	OPERAND_MDFR_XRAM,
	OPERAND_MDFR_YRAM,
	/*
	 * Modifiers above MDFR_HINT_START aren't always disqualifying if they
	 * don't immediately match, so they need to be distinguished as
	 * separate.
	 */
	OPERAND_MDFR_HINT_START,
	OPERAND_MDFR_16_BIT_UPPER,
	OPERAND_MDFR_16_BIT_SIGNED,
	OPERAND_MDFR_32_BIT_SIGNED,
};

enum op_operand_modifier_bit {
	OP_MDFR_BIT_NONE,
	OP_MDFR_BIT_TYPE_SRC_DST_SWAP,
	OP_MDFR_BIT_TYPE_USE_ALT_MDFR,
	OP_MDFR_BIT_TYPE_USE_ALT_STR,
	OP_MDFR_BIT_TYPE_USE_ALT_LAYOUT,
};

enum operand_token_type {
	OPERAND_TYPE_REG,
	OPERAND_TYPE_A_REG,
	OPERAND_TYPE_IND_LITERAL_X,
	OPERAND_TYPE_IND_LITERAL_Y,
	OPERAND_TYPE_IND_A_REG_X,
	OPERAND_TYPE_IND_A_REG_Y,
	OPERAND_TYPE_IND_A_REG_X_INC,
	OPERAND_TYPE_IND_A_REG_Y_INC,
	OPERAND_TYPE_IND_A_REG_X_PLUS_MDFR,
	OPERAND_TYPE_IND_A_REG_Y_PLUS_MDFR,
	OPERAND_TYPE_IND_A_REG_X_MDFR_OFFSET,
	OPERAND_TYPE_IND_A_REG_Y_MDFR_OFFSET,
	OPERAND_TYPE_IND_A_REG_X_LIT_OFFSET,
	OPERAND_TYPE_IND_A_REG_Y_LIT_OFFSET,
	OPERAND_TYPE_LITERAL,
	OPERAND_TYPE_NOP,
};

/*
 * Bit layout ID enums.
 * Postfixes are the length of the op they are used by.
 *
 * Don't forget to update their respective strings in ca0132_dsp_functions.c
 * when adding new layouts.
 */
enum parallel_operand_layout_id {
	P_OP_LAYOUT_MOV_2,
	P_OP_LAYOUT_MOVX_2,
	P_OP_LAYOUT_MOVX_DUAL_READ_2,
	P_OP_LAYOUT_MOVX_DUAL_WRITE_2,
	P_OP_LAYOUT_EXECUTE_COND_2,
	P_OP_LAYOUT_MOV_4,
	P_OP_LAYOUT_MOV_DUAL_4,
	P_OP_LAYOUT_MOVX_4,
	P_OP_LAYOUT_MOVX_DUAL_4,
	P_OP_LAYOUT_MOVX_READ_X_WRITE_Y_4,
	P_OP_LAYOUT_EXECUTE_COND_4,
};

enum operand_layout_id {
	OP_LAYOUT_MOV_1,
	OP_LAYOUT_MOVX_MDFR_OFFSET_1,
	OP_LAYOUT_MOVX_LIT_OFFSET_1,
	OP_LAYOUT_R_X_Y_1,
	OP_LAYOUT_R_X_Y_A_1,
	OP_LAYOUT_R_X_LIT_8_Y_1,
	OP_LAYOUT_PC_OFFSET_1,
	OP_LAYOUT_LOOP_PC_OFFSET_REG_CNT_1,
	OP_LAYOUT_LOOP_PC_OFFSET_LIT_8_CNT_1,
	OP_LAYOUT_PC_SET_REG_1,
	OP_LAYOUT_STACK_UNK_1,
	OP_LAYOUT_INTERRUPT_CLR_1,
	OP_LAYOUT_MOV_2,
	OP_LAYOUT_MOVX_MDFR_OFFSET_2,
	OP_LAYOUT_MOVX_LIT_OFFSET_2,
	OP_LAYOUT_R_X_Y_2,
	OP_LAYOUT_R_X_Y_A_2,
	OP_LAYOUT_MOV_LIT_16_2,
	OP_LAYOUT_R_X_LIT_16_Y_2,
	OP_LAYOUT_PC_LIT_16_2,
	OP_LAYOUT_LOOP_PC_LIT_16_2,
	OP_LAYOUT_STACK_UNK_2,
	OP_LAYOUT_MOV_4,
	OP_LAYOUT_R_X_Y_4,
	OP_LAYOUT_R_X_Y_A_4,
	OP_LAYOUT_MOV_LIT_32_4,
	OP_LAYOUT_R_X_LIT_32_Y_4,
	OP_LAYOUT_NOP,
};

/*
 * Structure defining the location of a single operand of an opcode.
 */
typedef struct {
	uint8_t part1_bit_start;
	uint8_t part1_bits;

	uint8_t part2_bit_start;
	uint8_t part2_bits;
	
	uint8_t operand_type;
	uint8_t operand_dir;
	uint8_t parallel_end;
} operand_loc_descriptor;

/*
 * Structure used for describing how the operands of an opcode are layed out.
 */
typedef struct {
	operand_loc_descriptor layout_val_loc;
	uint32_t layout_val;
	
	operand_loc_descriptor operand_loc[8];
	uint8_t operand_cnt;
	uint32_t bitmask[4];

	uint8_t supports_opt_args;
} op_operand_loc_layout;

/*
 * Structure containing all possible operand layouts for an opcode.
 */
typedef struct {
	op_operand_loc_layout loc_layouts[6];
	uint32_t loc_layout_cnt;
} op_operand_layout;

/*
 * Structure used for storing operand data to be printed.
 */
typedef struct {
	const char *op_str;
	uint32_t operand_val;

	uint8_t operand_type;
	uint8_t operand_dir;
	uint8_t operand_mod_type;
	uint8_t parallel_end;
} operand_data;

/*
 * Main opcode information structure.
 */
typedef struct {
	char *op_str;
	char *alt_op_str;
	uint32_t op;
	
	uint8_t has_op_layout;
	uint8_t layout_id;
	uint8_t alt_layout_id;

	uint8_t mdfr_bit;
	uint8_t mdfr_bit_type;
	uint8_t src_mdfr[2];
	uint8_t src_dst_swap;
} dsp_op_info;

/*
 * Used for temporary storage when checking for layout compatibility in the
 * assembler.
 */
typedef struct {
	uint32_t operand_type;
	uint32_t parallel_end;
	uint32_t mdfr;
} layout_operand_info;

/*
 * Structure used for storing operand values to be set.
 */
typedef struct {
	char *operand_str;
	uint32_t operand_num;
	uint32_t mdfr;

	uint32_t val;
	uint32_t type;
} dsp_asm_op_operand;

typedef struct {
	uint32_t opcode;
	char *op_str;

	uint32_t parallel_split_operand;
	dsp_asm_op_operand operands[0x08];
	uint32_t operand_cnt;

	const op_operand_loc_layout *loc_layout;
	uint8_t matched;

	uint8_t use_op_mdfr_bit;
	uint32_t mdfr_bit_type;
	uint32_t mdfr_bit;

	uint32_t src_mdfr;
	uint8_t src_dst_swap;

	uint8_t needs_alt_args;
} dsp_asm_op_data;

typedef struct {
	uint32_t has_p_op;
	uint32_t p_op_end_token;

	dsp_asm_op_data op, p_op;
	uint32_t opcode[4];
} dsp_asm_data;

typedef struct {
	char *token[0x40];
	uint32_t token_cnt;
} dsp_asm_str_tokens;

/* ca0132_dsp_functions.c defs. */
const char *get_dsp_operand_str(uint32_t reg_val);
uint32_t get_dsp_operand_str_val(char *operand, uint32_t *val);
const dsp_op_info *get_dsp_op_info(uint32_t opcode);
const dsp_op_info *get_dsp_p_op_info(uint32_t opcode, uint32_t op_len);
const op_operand_layout *get_op_layout(uint32_t layout_id);
const op_operand_layout *get_p_op_layout(uint32_t layout_id);
const dsp_op_info *find_dsp_asm_op(dsp_asm_data *data, const dsp_op_info *start);
const dsp_op_info *find_dsp_asm_p_op(dsp_asm_data *data, const dsp_op_info *start,
		uint32_t op_len);
uint32_t get_dsp_op_len(uint32_t op);
uint8_t get_asm_data_from_str(dsp_asm_data *data, char *asm_str);
uint32_t get_bits_in_op_words(uint32_t *op_words, uint32_t start,
		uint32_t len);
