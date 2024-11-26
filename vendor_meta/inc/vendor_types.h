// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#ifndef __VENDOR_TYPES_H__
#define __VENDOR_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

/* std includes */
#include <linux/types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* libcxlmi includes */
#include <ccan/short_types/short_types.h>

/* vendor includes */
#include "cxl_link.h"
#include "ddr.h"
#include "membridge.h"

enum {
  VENDOR_CMD_DDR_DIMM_MGMT = 0xC5,
#define DIMM_SPD_READ 0x10
#define DIMM_SLOT_INFO 0x20
#define READ_DDR_TEMP 0x31

  VENDOR_CMD_OEM_MGMT = 0xCD,
#define OEM_HBO_STATUS 0x00
#define TRANSFER_FW 0x01
#define ACTIVATE_FW 0x02
#define GET_OS_INFO 0x03
#define TRANSFER_OS 0x04

  VENDOR_CMD_HEALTH_MGMT = 0xCE,
#define HEALTH_COUNTERS_CLEAR 0x00
#define HEALTH_COUNTERS_GET 0x01

  VENDOR_CMD_OTHERS = 0xFB,
#define PMIC_VTMON_INFO 0x00
#define READ_LTSSM_STATES 0x01
#define PCIE_EYE_RUN 0x02
#define PCIE_EYE_STATUS 0x03
#define PCIE_EYE_GET_SW 0x04
#define PCIE_EYE_GET_HW 0x05
#define PCIE_EYE_GET_SW_BER 0x06
#define CXL_LINK_STATUS 0x07
#define DEVICE_INFO 0x08
#define GET_DDR_BW 0x09
#define DDR_MARGIN_RUN 0x0A
#define DDR_MARGIN_STATUS 0x0B
#define DDR_MARGIN_GET 0x0C
#define REBOOT_MODE_SET 0x0D
#define CURR_CXL_BOOT_MODE_GET 0x0E
#define GET_DDR_ECC_ERR_INFO 0x0F
#define I2C_READ 0x10
#define I2C_WRITE 0x11
#define GET_DDR_LATENCY 0x12
#define GET_MEMBRIDGE_ERRORS 0x13
#define HPA_TO_DPA 0x14
#define START_DDR_ECC_SCRUB 0x15
#define DDR_ECC_SCRUB_STATUS 0x16
#define DDR_INIT_STATUS 0x17
#define GET_MEMBRIDGE_STATS 0x18
#define DDR_ERR_INJ_EN 0x19
#define TRIGGER_COREDUMP 0x1A
#define DDR_STATS_RUN 0x1B
#define DDR_STATS_STATUS 0x1C
#define DDR_STATS_GET 0x1D
#define DDR_PARAM_SET 0x1E
#define DDR_PARAM_GET 0x1F
#define DIMM_LEVEL_TRAINING_STATUS 0x20
#define CXL_VIRAL_INJ_EN 0x21
#define CXL_MEM_LL_ERR_INJ_EN 0x22
#define CXL_IO_LL_ERR_INJ_EN 0x23
#define CXL_PHY_ERR_INJ_EN 0x24
#define PCI_RAS_DES_ERR_INJ_EN 0x25
#define CORE_VOLT_SET 0x26
#define CORE_VOLT_GET 0x27
#define DDR_CONT_SCRUB_STATUS 0x28
#define DDR_CONT_SCRUB_SET 0x29
#define DDR_PAGE_SELECT_SET 0x2A
#define DDR_PAGE_SELECT_GET 0x2B
#define DDR_HPPR_SET 0x2C
#define DDR_HPPR_GET 0x2D
#define DDR_HPPR_ADDR_INFO_SET 0x2E
#define DDR_HPPR_ADDR_INFO_GET 0x2F
#define DDR_HPPR_ADDR_INFO_CLEAR 0x30
#define DDR_PPR_GET_STATUS 0x31
#define DDR_REFRESH_MODE_SET 0x32
#define DDR_REFRESH_MODE_GET 0x33
#define CXL_ERR_CNTR_GET 0x34
#define DDR_FREQUENCY_GET 0x35
#define DDR_INIT_ERR_INFO_GET 0x36
};

/* Structure for HBO status */
struct cxlmi_cmd_hbo_status_out {
  __le64 bo_status;
} __attribute__((packed));

struct cxlmi_cmd_hbo_status_fields {
  uint16_t opcode;
  uint8_t percent_complete;
  uint8_t is_running;
  uint16_t return_code;
  uint16_t extended_status;
};

/* Structure for dimm spd read request */
struct cxlmi_cmd_dimm_spd_read_req {
  __le32 spd_id;
  __le32 offset;
  __le32 num_bytes;
} __attribute__((packed));

/* Structure for dimm spd read response */
struct cxlmi_cmd_dimm_spd_read_rsp {
  uint8_t dimm_spd_data[512];
};

/* Structure for dimm slot info */
struct cxlmi_cmd_dimm_slot_info {
  u8 num_dimm_slots;
  u8 rsvd[3];
  u8 slot0_spd_i2c_addr;
  u8 slot0_channel_id;
  u8 slot0_dimm_silk_screen;
  u8 slot0_dimm_present;
  u8 rsvd1[12];
  u8 slot1_spd_i2c_addr;
  u8 slot1_channel_id;
  u8 slot1_dimm_silk_screen;
  u8 slot1_dimm_present;
  u8 rsvd2[12];
  u8 slot2_spd_i2c_addr;
  u8 slot2_channel_id;
  u8 slot2_dimm_silk_screen;
  u8 slot2_dimm_present;
  u8 rsvd3[12];
  u8 slot3_spd_i2c_addr;
  u8 slot3_channel_id;
  u8 slot3_dimm_silk_screen;
  u8 slot3_dimm_present;
  u8 rsvd4[12];
};

/* Structure for read ddr temp */
#define DDR_MAX_DIMM_CNT 4
struct cxlmi_ddr_temp {
  uint8_t ddr_temp_valid;
  uint8_t dimm_id;
  uint8_t spd_idx;
  uint8_t rsvd;
  float dimm_temp;
};

struct cxlmi_cmd_read_ddr_temp {
  struct cxlmi_ddr_temp ddr_dimm_temp_info[DDR_MAX_DIMM_CNT];
};

/* Structure for health info clear */
struct cxlmi_cmd_health_counters_clear {
  __le32 bitmask;
};

/* Structure for health info get */
struct cxlmi_cmd_health_counters_get {
  __le32 critical_over_temperature_exceeded;
  __le32 power_on_events;
  __le32 power_on_hours;
  __le32 cxl_mem_link_crc_errors;
  __le32 cxl_io_link_lcrc_errors;
  __le32 cxl_io_link_ecrc_errors;
  __le32 num_ddr_correctable_ecc_errors;
  __le32 num_ddr_uncorrectable_ecc_errors;
  __le32 link_recovery_events;
  __le32 time_in_throttled;
  __le32 over_temperature_warning_level_exceeded;
  __le32 critical_under_temperature_exceeded;
  __le32 under_temperature_warning_level_exceeded;
  __le32 rx_retry_request;
  __le32 rcmd_qs0_hi_threshold_detect;
  __le32 rcmd_qs1_hi_threshold_detect;
  __le32 num_pscan_correctable_ecc_errors;
  __le32 num_pscan_uncorrectable_ecc_errors;
  __le32 num_ddr_dimm0_correctable_ecc_errors;
  __le32 num_ddr_dimm0_uncorrectable_ecc_errors;
  __le32 num_ddr_dimm1_correctable_ecc_errors;
  __le32 num_ddr_dimm1_uncorrectable_ecc_errors;
  __le32 num_ddr_dimm2_correctable_ecc_errors;
  __le32 num_ddr_dimm2_uncorrectable_ecc_errors;
  __le32 num_ddr_dimm3_correctable_ecc_errors;
  __le32 num_ddr_dimm3_uncorrectable_ecc_errors;
};

/* Structure for pmic vtmon info */
#define MAX_PMIC 8
#define PMIC_NAME_MAX_SIZE 20

struct pmic_data {
  char pmic_name[PMIC_NAME_MAX_SIZE];
  float vin;
  float vout;
  float iout;
  float powr;
  float temp;
};

struct cxlmi_cmd_pmic_vtmon_info {
  struct pmic_data pmic_data[MAX_PMIC];
};

/* Structure for read ltssm_states*/
#define LTSSM_DUMP_SIZE 0x200
#define LTSSM_EXPECTED_STATE 0x11
#define LTSSM_STATE_DUMP_COUNT_MAX (LTSSM_DUMP_SIZE / 4)
struct cxlmi_cmd_read_ltssm_states {
  uint32_t ltssm_states[LTSSM_STATE_DUMP_COUNT_MAX];
};

/* Structure for pcie_eye run */
struct cxlmi_cmd_pcie_eye_run_req {
  uint8_t lane;
  uint8_t sw_scan;
  uint8_t ber;
} __attribute__((packed));

struct cxlmi_cmd_pcie_eye_run_rsp {
  int pcie_eye_run_status;
};

/* Structure for pcie_eye status */
struct cxlmi_cmd_pcie_eye_status {
  int pcie_eye_status;
  int error;
};

/* Structure for pcie_eye get sw*/
struct cxlmi_cmd_pcie_eye_get_sw_req {
  uint32_t offset;
};

#define NUM_EYESCOPE_HORIZ_VALS 40
#define TOTAL_EYESCOPE_HORIZ_VALS ((NUM_EYESCOPE_HORIZ_VALS * 2) + 1)

struct cxlmi_cmd_pcie_eye_get_sw_rsp {
  char pcie_eye_data[TOTAL_EYESCOPE_HORIZ_VALS + 1];
};

/* Structure for pcie_eye get hw*/
struct eyescope_results {
  double merged_horizontal_eye_left;
  double merged_horizontal_eye_right;
  double merged_vertical_eye_top;
  double merged_vertical_eye_bottom;
};

struct rx_settings {
  int iskew_signed;
  int qskew_signed;
  int dlev00_signed;
  int dlev01_signed;
  int dlev10_signed;
  int dlev11_signed;
  int irphase_signed;
  int h1po;
  int h1no;
  int h1pe;
  int h1ne;
  int h2;
  int h3;
  int h4;
  int h5;
  int h6;
  int h7;
  int h8;
  int h9;
  int aeq;
  int vga;
  int appmd;
  int rxrt;
  int shd;
  int step;
  int wm;
};

struct cxlmi_cmd_pcie_eye_get_hw {
  struct eyescope_results eyescope_results;
  struct rx_settings rx_settings;
  int eyescope_request_status;
};

/* Structure for pcie_eye get sw_ber */
struct cxlmi_cmd_pcie_eye_get_sw_ber {
  float horiz_margin;
  float vert_margin;
};

/* Structure for get cxl link status */
struct cxlmi_cmd_get_cxl_link_status {
  float cxl_link_status;
  uint32_t link_width;
  uint32_t link_speed;
  uint32_t ltssm_val;
};

/* Structure for get device info */
struct cxlmi_cmd_get_device_info {
  uint16_t device_id;
  uint8_t revision_id;
} __attribute__((packed));

/* Structure for get ddr bw */
typedef enum {
  DDR_CTRL0 = 0,
  DDR_CTRL1 = 1,
  DDR_MAX_SUBSYS,
} ddr_subsys;

struct cxlmi_cmd_get_ddr_bw_req {
  u32 timeout;
  u32 iterations;
};

struct cxlmi_cmd_get_ddr_bw_rsp {
  float peak_bw[DDR_MAX_SUBSYS];
};

/* Structure for DDR margin run */
struct cxlmi_cmd_ddr_margin_run {
  u8 slice_num;
  u8 rd_wr_margin;
  u8 ddr_id;
} __attribute__((packed));

/* Structure for DDR margin status */
struct cxlmi_cmd_ddr_margin_status {
  int run_status;
};

/* Structure for DDR margin get */
/* MAX_NUM_ROWS per BIT_COUNT should be in Sync with the FW Mbox DDR MARGIN code
 */
#define MAX_NUM_ROWS 1024
#define MAX_MARGIN_BIT_COUNT 8

struct ddr_margin_info {
  uint32_t slicenumber;
  uint32_t bitnumber;
  int32_t vreflevel;
  int margin_low;
  int margin_high;
  double min_delay_ps;
  double max_delay_ps;
} __attribute__((packed));

struct cxlmi_cmd_ddr_margin_get {
  uint32_t row_count;
  struct ddr_margin_info
      ddr_margin_slice_data[MAX_NUM_ROWS * MAX_MARGIN_BIT_COUNT];
};

#define CXL_IO_MEM_MODE 0x0
#define CXL_IO_MODE 0xCE

/* Structure for reboot mode set */
struct cxlmi_cmd_reboot_mode_set {
  uint8_t reboot_mode;
};

/* Structure for curr cxl boot mode get */
struct cxlmi_cmd_curr_cxl_boot_mode_get {
  uint8_t curr_cxl_boot;
};

/* Structure for get ddr ecc err info */
struct ddr_parity_err {
  uint32_t parity_crit_bit2_cnt; /* Parity error on the address/control bus*/
  uint32_t parity_crit_bit1_cnt; /* Overlapping write data parity error */
  uint32_t parity_crit_bit0_cnt; /* Write data parity error */
};

struct ddr_dfi_err {
  uint32_t dfi_crit_bit5_cnt; /* DFI tINIT_COMPLETE value has timed out */
  uint32_t dfi_crit_bit2_cnt; /* Error received from the PHY on the DFI bus */

  uint32_t dfi_warn_bit1_cnt; /* DFI PHY Master Interface error has occurred */
  uint32_t dfi_warn_bit0_cnt; /* DFI update error has occurred */
};

struct ddr_crc_err {
  uint32_t crc_crit_bit1_cnt; /* CA Parity or a CRC error happened during CRC
                                Retry. */
  uint32_t crc_crit_bit0_cnt; /* CRC error occurred on the write data bus */
};

struct ddr_userif_err {
  uint32_t
      userif_crit_bit2_cnt; /* Error occurred on the port command channel. */
  uint32_t userif_crit_bit1_cnt; /* Multiple accesses outside the defined
                                    PHYSICAL memory space have occurred. */
  uint32_t userif_crit_bit0_cnt; /* A Memory access outside the defined PHYSICAL
                                    memory space has occurred */
};

struct ddr_ecc_err {
  uint32_t ecc_warn_bit6_cnt; /* One or more ECC writeback commands
                                could not be executed */
  uint32_t ecc_crit_bit3_cnt; /* Multiple uncorrectable ECC events
                                have been detected */
  uint32_t ecc_crit_bit2_cnt; /* A uncorrectable ECC event has been detected */
  uint32_t ecc_crit_bit8_cnt; /* An ECC correctable error has been detected in a
                                 scrubbing read operation */
  uint32_t ecc_warn_bit1_cnt; /* Multiple correctable ECC events
                                 have been detected */
  uint32_t ecc_warn_bit0_cnt; /* A correctable ECC event has been detected */
};

struct ddr_controller_errors {
  struct ddr_parity_err parity;
  struct ddr_dfi_err dfi;
  struct ddr_crc_err crc;
  struct ddr_userif_err userif;
  struct ddr_ecc_err ecc;
};

struct cxlmi_cmd_get_ddr_ecc_err_info {
  struct ddr_controller_errors ddr_ctrl_err[DDR_MAX_SUBSYS];
};

/* Structure for i2c read */
#define I2C_MAX_SIZE_NUM_BYTES 128

struct cxlmi_cmd_i2c_read_req {
  u16 slave_addr;
  u8 reg_addr;
  u8 num_bytes;
};

struct cxlmi_cmd_i2c_read_rsp {
  char buf[I2C_MAX_SIZE_NUM_BYTES];
  u8 num_bytes;
};

/* Structure for i2c write */
struct cxlmi_cmd_i2c_write {
  u16 slave_addr;
  u8 reg_addr;
  u8 data;
};

/* Structure for get ddr latency */
struct ddr_lat_op {
  uint64_t readlat;
  uint64_t writelat;
  uint32_t rdsamplecnt;
  uint32_t wrsamplecnt;
  float avg_rdlatency;
  float avg_wrlatency;
};

struct cxlmi_cmd_get_ddr_latency_req {
  u32 measure_time;
};

struct cxlmi_cmd_get_ddr_latency_rsp {
  struct ddr_lat_op ddr_lat_op[DDR_MAX_SUBSYS];
};

/* Structure for get membridge errors */
struct cxlmi_cmd_get_membridge_errors {
  uint32_t fifo_overflow;
  uint32_t fifo_overflows[FIFO_ERROR_COUNT];
  uint32_t fifo_underflow;
  uint32_t fifo_underflows[FIFO_ERROR_COUNT];
  uint32_t ddr0_parity_error;
  uint32_t ddr0_parity_errors[DDR_PARITY_ERROR_COUNT];
  uint32_t ddr1_parity_error;
  uint32_t ddr1_parity_errors[DDR_PARITY_ERROR_COUNT];
  uint32_t parity_error;
  uint32_t parity_errors[PARITY_ERROR_COUNT];
  uint32_t common_errors[MEMBRIDGE_COMMON_ERROR_COUNT];
};

/* Structure for HPA to DPA */
struct cxlmi_cmd_hpa_to_dpa_req {
  uint64_t hpa_address;
};

struct cxlmi_cmd_hpa_to_dpa_rsp {
  uint64_t dpa_address;
};

/* Structure for DDR ECC scrub status */
struct cxlmi_cmd_ddr_ecc_scrub_status {
  int ecc_scrub_status[DDR_MAX_SUBSYS];
};

/* STructure for DDR init status */
typedef enum {
  DDR_INIT_INPROGRESS = 0,
  DDR_INIT_PASSED = 1,
  DDR_INIT_FAILED = -1,
  DDR_INIT_FAILED_NO_CH0_DIMM0 = -2,
  DDR_INIT_FAILED_UNKNOWN_DIMM = -3,
} ddr_status;

typedef enum {
  CH_0 = 0,
  CH_1 = 1,
  CH_NA = -1,
} f_channel_id;

struct ddr_init_boot_status {
  int8_t ddr_init_status;
  int8_t failed_channel_id;
  char failed_dimm_silk_screen;
};

struct cxlmi_cmd_ddr_init_status {
  struct ddr_init_boot_status init_status;
};

/* Structure for membridge stats */
struct cxlmi_cmd_get_membridge_stats {
  // mem transaction counters
  uint64_t m2s_req_count;
  uint64_t m2s_rwd_count;
  uint64_t s2m_drs_count;
  uint64_t s2m_ndr_count;
  // HPA logs for poison & out-of-range
  uint64_t rwd_first_poison_hpa_log;
  uint64_t rwd_latest_poison_hpa_log;
  uint64_t req_first_hpa_log;
  uint64_t rwd_first_hpa_log;
  // correctible errors counters
  uint32_t mst_m2s_req_corr_err_count;
  uint32_t mst_m2s_rwd_corr_err_count;
  // membridge fifo full/empty status
  uint32_t fifo_full_status;
  uint32_t fifo_empty_status;
  // credit counters
  uint8_t m2s_rwd_credit_count;
  uint8_t m2s_req_credit_count;
  uint8_t s2m_ndr_credit_count;
  uint8_t s2m_drc_credit_count;
  // rx state machine status 0
  uint8_t rx_fsm_status_rx_deinit;
  uint8_t rx_fsm_status_m2s_req;
  uint8_t rx_fsm_status_m2s_rwd;
  uint8_t rx_fsm_status_ddr0_ar_req;
  uint8_t rx_fsm_status_ddr0_aw_req;
  uint8_t rx_fsm_status_ddr0_w_req;
  // rx state machine status 1
  uint8_t rx_fsm_status_ddr1_ar_req;
  uint8_t rx_fsm_status_ddr1_aw_req;
  uint8_t rx_fsm_status_ddr1_w_req;
  // tx state machine status 0
  uint8_t tx_fsm_status_tx_deinit;
  uint8_t tx_fsm_status_s2m_ndr;
  uint8_t tx_fsm_status_s2m_drc;
  // stat QoS TEL
  uint8_t stat_qos_tel_dev_load_read;
  uint8_t stat_qos_tel_dev_load_type2_read;
  uint8_t stat_qos_tel_dev_load_write;
  uint8_t resvd;
};

/* Structure for DDR error injection enable */
struct cxlmi_cmd_ddr_err_inj_en {
  uint32_t ddr_id;
  uint32_t err_type;
  uint64_t ecc_fwc_mask;
};

/* Structure for DDR stats run */
struct cxlmi_cmd_ddr_stats_run {
  uint8_t ddr_id;
  uint32_t monitor_time;
  uint32_t loop_count;
} __attribute__((packed));

/* Structure for DDR stats status */
struct cxlmi_cmd_ddr_stats_status {
  int run_status;
  uint32_t loop_count;
};

/* Structure for DDR stats get */
struct cxlmi_cmd_ddr_stats_get_req {
  uint32_t offset;
  uint32_t transfer_sz;
};

typedef unsigned char *cxlmi_cmd_ddr_stats_get_rsp_t;

/* Structure for DDR param set */
struct ddr_interleave_options {
  uint8_t ddr_interleave_sz;
  uint8_t ddr_interleave_ctrl_choice;
} __attribute__((packed));

struct cxlmi_cmd_ddr_param_set {
  struct ddr_interleave_options ddr_inter;
};

/* Structure for DDR param get */
struct cxlmi_cmd_ddr_param_get {
  struct ddr_interleave_options ddr_inter;
};

/* Structure for DDR DIMM level training status */
struct cxlmi_cmd_dimm_level_training_status {
  struct ddr_dimm_training_status dimm_training_status[DDR_MAX_SUBSYS];
};

/* Structure for Viral Injection Enable */
struct cxlmi_cmd_viral_inj_en {
  u32 viral_type;
};

/* Structure for Mem LL Error Injection Enable */
struct cxlmi_cmd_mem_ll_err_inj_en {
  u32 en_dis;
  u32 ll_err_type;
};

/* Structure for PCI Error Injection Enable */
struct cxlmi_cmd_pci_err_inj_en {
  u32 en_dis;
  u32 err_type;
  u32 err_subtype;
  u32 count;
  u32 opt_param1;
  u32 opt_param2;
};

/* Strcutre for core volt set */
struct cxlmi_cmd_core_volt_set {
  float core_volt;
};

/* Strcture for core volt get */
struct cxlmi_cmd_core_volt_get {
  float core_volt;
};

/* Structure for DDR continuous scrub status */
struct cxlmi_cmd_ddr_cont_scrub_status {
  uint32_t cont_scrub_status;
};

/* Structure for DDR continuous scrub set */
struct cxlmi_cmd_ddr_cont_scrub_set {
  uint32_t cont_scrub_status;
};

/* Structure for DDR Page Policy Select set */
struct cxlmi_cmd_ddr_page_select_set {
  struct page_policy_selection pp_select;
} __attribute__((packed));

/* Structure for DDR Page Policy Select get */
struct cxlmi_cmd_ddr_page_select_get {
  struct page_policy_selection pp_select;
} __attribute__((packed));

/* Structure for DDR HPPR set */
struct cxlmi_cmd_ddr_hppr_set {
  uint16_t enable;
} __attribute__((packed));

/* Structure for DDR HPPR get */
struct cxlmi_cmd_ddr_hppr_get {
  uint8_t hppr_enable[2];
} __attribute__((packed));

/* Structure for DDR HPPR  address info set */
struct cxlmi_cmd_ddr_hppr_addr_info_set {
  struct _ddr_addr_info_in hppr_addr_info;
};

/* Structure for DDR HPPR  address info get */
struct cxlmi_cmd_ddr_hppr_addr_info_get {
  struct _ddr_addr_info_out hppr_addr_info[2][8];
} __attribute__((packed));

/* Structure for DDR HPPR  address info clear */
struct cxlmi_cmd_ddr_hppr_addr_info_clear {
  uint8_t ddr_id;
  uint8_t channel_id;
} __attribute__((packed));

/* Structure for DDR PPR get status */
struct cxlmi_cmd_ddr_ppr_get_status {
  uint8_t status;
} __attribute__((packed));

/* Structure for DDR Refresh mode set */
struct cxlmi_cmd_ddr_refresh_mode_set {
  uint8_t ddr_refresh_val;
} __attribute__((packed));

/* Structure for DDR Refresh mode get */
struct cxlmi_cmd_ddr_refresh_mode_get {
  uint8_t ddr_refresh_val;
} __attribute__((packed));

/* Structure for CXL error counter get*/
struct cxlmi_cmd_cxl_err_cntr_get {
  uint64_t total_err_cnt;
  uint64_t total_corr_err_cnt;
  uint64_t total_uncorr_err_cnt;
  uint64_t total_cxl_cfg_err_cnt;
  uint32_t corr_err[MAX_CORR_ERR_COUNT];
  uint32_t uncorr_err[MAX_UNCORR_ERR_COUNT];
  uint32_t cxl_conf_err[MAX_CXL_CFG_ERR_COUNT];
} __attribute__((packed));

/* Structure for DDR frequency get */
struct cxlmi_cmd_ddr_freq_get {
  float ddr_freq;
};

/* Structure for DDR init error info get */
struct ddr_init_bist_err_info {
  uint32_t ddr_bist_err_cnt;
  uint32_t ddr_bist_err_info_col;
  uint32_t ddr_bist_err_info_row;
  uint16_t ddr_bist_err_info_bank;
  uint16_t ddr_bist_err_info_cs;
} __attribute__((packed));

struct cxlmi_cmd_ddr_init_err_info_get {
  struct ddr_init_bist_err_info ddr_bist_err_info[DDR_MAX_SUBSYS];
};

#ifdef __cplusplus
}
#endif

#endif /* __VENDOR_TYPES_H__ */
