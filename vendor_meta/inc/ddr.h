// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#ifndef __DDR_H__
#define __DDR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#define NUM_BANK 16
#define NUM_CS 4

struct dfi_cs_pm {
  uint32_t mrw_cnt;
  uint32_t refresh_cnt;
  uint32_t act_cnt;
  uint32_t write_cnt;
  uint32_t read_cnt;
  uint32_t pre_cnt;
  uint32_t rr_cnt;
  uint32_t ww_cnt;
  uint32_t rw_cnt;
} __attribute__((packed));

struct dfi_cs_bank_pm {
  uint32_t bank_act_cnt;
  uint32_t bank_wr_cnt;
  uint32_t bank_rd_cnt;
  uint32_t bank_pre_cnt;
} __attribute__((packed));

struct dfi_mc_pm {
  uint32_t cmd_queue_full_events;
  uint32_t info_fifo_full_events;
  uint32_t wrdata_hold_fifo_full_events;
  uint32_t port_cmd_fifo0_full_events;
  uint32_t port_wrresp_fifo0_full_events;
  uint32_t port_wr_fifo0_full_events;
  uint32_t port_rd_fifo0_full_events;
  uint32_t port_cmd_fifo1_full_events;
  uint32_t port_wrresp_fifo1_full_events;
  uint32_t port_wr_fifo1_full_events;
  uint32_t port_rd_fifo1_full_events;
  uint32_t ecc_dataout_corrected;
  uint32_t ecc_dataout_uncorrected;
  uint32_t pd_ex;
  uint32_t pd_en;
  uint32_t srex;
  uint32_t sren;
  uint32_t write;
  uint32_t read;
  uint32_t rmw;
  uint32_t bank_act;
  uint32_t precharge;
  uint32_t precharge_all;
  uint32_t mrw;
  uint32_t auto_ref;
  uint32_t rw_auto_pre;
  uint32_t zq_cal_short;
  uint32_t zq_cal_long;
  uint32_t same_addr_ww_collision;
  uint32_t same_addr_wr_collision;
  uint32_t same_addr_rw_collision;
  uint32_t same_addr_rr_collision;
} __attribute__((packed));

struct ddr_pmon_data {
  uint64_t fr_cnt;
  uint32_t idle_cnt;
  uint32_t rd_ot_cnt;
  uint32_t wr_ot_cnt;
  uint32_t wrd_ot_cnt;
  uint32_t rd_cmd_cnt;
  uint32_t rd_cmd_busy_cnt;
  uint32_t wr_cmd_cnt;
  uint32_t wr_cmd_busy_cnt;
  uint32_t rd_data_cnt;
  uint32_t rd_data_busy_cnt;
  uint32_t wr_data_cnt;
  uint32_t wr_data_busy_cnt;
  uint64_t rd_avg_lat;
  uint64_t wr_avg_lat;
  uint32_t rd_trans_smpl_cnt;
  uint32_t wr_trans_smpl_cnt;
} __attribute__((packed));

struct ddr_data {
  struct ddr_pmon_data pmon;
  struct dfi_cs_pm cs_pm[NUM_CS];
  struct dfi_cs_bank_pm cs_bank_pm[NUM_CS][NUM_BANK];
  struct dfi_mc_pm mc_pm;
} __attribute__((packed));

struct ddr_stats_data {
  struct ddr_data stats;
} __attribute__((packed));

typedef struct ddr_stats_data ddr_stats_data_t;

/* Structure for DDR DIMM level training status */

#define DDR_MAX_CS 4
#define DDR_CS_DEVICE_MAX 18
#define DDR_REG_MAX_NIBBLE 9
#define DDR_MAX_SLICE 9
#define DDR_MAX_SLICE_BIT 8

struct ddr_phy_pll_status {
  uint32_t bs0_status;
  uint32_t bs1_status;
};

struct ddr_wr_levelling_status {
  uint32_t lower_nibble_err[DDR_REG_MAX_NIBBLE];
  uint32_t upper_nibble_err[DDR_REG_MAX_NIBBLE];
};

struct ddr_read_gate_training_status {
  uint32_t lower_nibble_min_err[DDR_REG_MAX_NIBBLE];
  uint32_t lower_nibble_max_err[DDR_REG_MAX_NIBBLE];
  uint32_t upper_nibble_min_err[DDR_REG_MAX_NIBBLE];
  uint32_t upper_nibble_max_err[DDR_REG_MAX_NIBBLE];
};

struct ddr_margin_vref_data {
  float lower_nibble_vref_low_volt[DDR_REG_MAX_NIBBLE];
  float lower_nibble_vref_high_volt[DDR_REG_MAX_NIBBLE];
  float upper_nibble_vref_low_volt[DDR_REG_MAX_NIBBLE];
  float upper_nibble_vref_high_volt[DDR_REG_MAX_NIBBLE];
};

struct ddr_margin_write_dq_vref_data {
  float vref_low_volt[DDR_CS_DEVICE_MAX];
  float vref_high_volt[DDR_CS_DEVICE_MAX];
};

struct ddr_margin_write_dq_vref_data_cs {
  float vref_low_volt_cs[DDR_MAX_CS][DDR_CS_DEVICE_MAX];
  float vref_high_volt_cs[DDR_MAX_CS][DDR_CS_DEVICE_MAX];
};

struct ddr_margin_rdlvl_delay_dqs_rise_data {
  uint32_t te_delay_data[DDR_MAX_SLICE][DDR_MAX_SLICE_BIT];
  uint32_t le_delay_data[DDR_MAX_SLICE][DDR_MAX_SLICE_BIT];
  float te_delay_time[DDR_MAX_SLICE][DDR_MAX_SLICE_BIT];
  float le_delay_time[DDR_MAX_SLICE][DDR_MAX_SLICE_BIT];
};

struct ddr_margin_rdlvl_delay_dqs_fall_data {
  uint32_t te_delay_data[DDR_MAX_SLICE][DDR_MAX_SLICE_BIT];
  uint32_t le_delay_data[DDR_MAX_SLICE][DDR_MAX_SLICE_BIT];
  float te_delay_time[DDR_MAX_SLICE][DDR_MAX_SLICE_BIT];
  float le_delay_time[DDR_MAX_SLICE][DDR_MAX_SLICE_BIT];
};

struct ddr_margin_wrdqlvl_delay_data {
  uint32_t te_delay_data[DDR_MAX_SLICE][DDR_MAX_SLICE_BIT];
  uint32_t le_delay_data[DDR_MAX_SLICE][DDR_MAX_SLICE_BIT];
  float te_delay_time[DDR_MAX_SLICE][DDR_MAX_SLICE_BIT];
  float le_delay_time[DDR_MAX_SLICE][DDR_MAX_SLICE_BIT];
};

struct ddr_dimm_training_status {
  struct ddr_phy_pll_status phy_pll_status;
  struct ddr_wr_levelling_status wr_levl_status;
  struct ddr_read_gate_training_status rd_gate_tr_status;
  struct ddr_margin_vref_data vref_data;
  struct ddr_margin_write_dq_vref_data wdq_vref_data;
  struct ddr_margin_write_dq_vref_data_cs wdq_vref_data_cs;
  struct ddr_margin_rdlvl_delay_dqs_rise_data rddqslvl_rise_data;
  struct ddr_margin_rdlvl_delay_dqs_fall_data rddqslvl_fall_data;
  struct ddr_margin_wrdqlvl_delay_data wrdqlvl_delay_data;
  uint32_t err_status;
};

/* Structure for DDR Page Policy Select */
struct page_policy_selection {
  uint8_t page_policy_reg_val;
} __attribute__((packed));

/* Structure for DDR HPPR address */
struct _ddr_addr_info_in {
  uint8_t ddr_id;
  uint8_t chip_select; /* 2bit chip select info of faulty row*/
  uint8_t bank;        /* 2bits bank info*/
  uint8_t bank_group;  /* 2bit bank group info */
  uint32_t row;        /* faulty row address */
} __attribute__((packed));

struct _ddr_addr_info_out {
  uint8_t ddr_id;
  uint8_t chip_select; /* 2bit chip select info of faulty row*/
  uint8_t bank;        /* 2bits bank info*/
  uint8_t bank_group;  /* 2bit bank group info */
  uint32_t row;        /* faulty row address */
  uint8_t channel;     /* channel 0/1 of DDR controller */
  uint8_t ppr_state;
} __attribute__((packed));

#ifdef __cplusplus
}
#endif

#endif /* __DDR_H__ */
