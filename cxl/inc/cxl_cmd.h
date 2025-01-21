// SPDX-License-Identifier: LGPL-2.1-or-later
#ifndef __CXL_CMD_H__
#define __CXL_CMD_H__

#ifdef __cplusplus
extern "C" {
#endif

/* std includes */
#include <stdbool.h>
#include <stdint.h>

/* libcxlmi includes */
#include <libcxlmi.h>

/* vendor includes */
#include <util_main.h>

/* Helper structures */
struct _update_fw_params {
  const char *filepath;
  uint32_t slot;
  bool hbo;
  bool mock;
};

/* shell command handlers  */
int cmd_print_help(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_identify(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_supported_logs(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_log(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_alert_config(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_set_alert_config(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_health_info(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_fw_info(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_update_fw(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_timestamp(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_set_timestamp(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_event_records(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_clear_event_records(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_event_interrupt_policy(int argc, const char **argv,
                                   struct cxlmi_ctx *ctx);
int cmd_set_event_interrupt_policy(int argc, const char **argv,
                                   struct cxlmi_ctx *ctx);

int cmd_dimm_spd_read(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_dimm_slot_info(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_read_ddr_temp(int argc, const char **argv, struct cxlmi_ctx *ctx);

int cmd_health_counters_clear(int argc, const char **argv,
                              struct cxlmi_ctx *ctx);
int cmd_health_counters_get(int argc, const char **argv, struct cxlmi_ctx *ctx);

int cmd_pmic_vtmon_info(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_read_ltssm_states(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_pcie_eye_run(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_pcie_eye_status(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_pcie_eye_get(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_cxl_link_status(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_device_info(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_ddr_bw(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_margin_run(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_margin_status(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_margin_get(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_reboot_mode_set(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_curr_cxl_boot_mode_get(int argc, const char **argv,
                               struct cxlmi_ctx *ctx);
int cmd_get_ddr_ecc_err_info(int argc, const char **argv,
                             struct cxlmi_ctx *ctx);
int cmd_i2c_read(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_i2c_write(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_ddr_latency(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_membridge_errors(int argc, const char **argv,
                             struct cxlmi_ctx *ctx);
int cmd_hpa_to_dpa(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_start_ddr_ecc_scrub(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_ecc_scrub_status(int argc, const char **argv,
                             struct cxlmi_ctx *ctx);
int cmd_ddr_init_status(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_membridge_stats(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_err_inj_en(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_trigger_coredump(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_stats_run(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_stats_get(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_param_set(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_param_get(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_dimm_level_training_status(int argc, const char **argv,
                                       struct cxlmi_ctx *ctx);
int cmd_viral_inj_en(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_mem_ll_err_inj_en(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_pci_err_inj_en(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_core_volt_set(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_core_volt_get(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_cont_scrub_status(int argc, const char **argv,
                              struct cxlmi_ctx *ctx);
int cmd_ddr_cont_scrub_set(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_page_select_set(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_page_select_get(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_hppr_set(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_hppr_get(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_hppr_addr_info_set(int argc, const char **argv,
                               struct cxlmi_ctx *ctx);
int cmd_ddr_hppr_addr_info_get(int argc, const char **argv,
                               struct cxlmi_ctx *ctx);
int cmd_ddr_hppr_addr_info_clear(int argc, const char **argv,
                                 struct cxlmi_ctx *ctx);
int cmd_ddr_ppr_get_status(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_refresh_mode_set(int argc, const char **argv,
                             struct cxlmi_ctx *ctx);
int cmd_ddr_refresh_mode_get(int argc, const char **argv,
                             struct cxlmi_ctx *ctx);
int cmd_cxl_err_cntr_get(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_freq_get(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_ddr_init_err_info_get(int argc, const char **argv,
                              struct cxlmi_ctx *ctx);

/* CXL command handlers */
int cxl_cmd_identify(struct cxlmi_endpoint *ep);
int cxl_cmd_get_supported_logs(struct cxlmi_endpoint *ep);
int cxl_cmd_get_log(struct cxlmi_endpoint *ep, const char *log_uuid,
                    uint32_t log_size);
int cxl_cmd_get_alert_config(struct cxlmi_endpoint *ep);
int cxl_cmd_set_alert_config(struct cxlmi_endpoint *ep,
                             uint32_t alert_prog_threshold,
                             uint32_t device_temp_threshold,
                             uint32_t mem_error_threshold);
int cxl_cmd_get_health_info(struct cxlmi_endpoint *ep);
int cxl_cmd_get_dev_fw_info(struct cxlmi_endpoint *ep, bool is_os);
int cxl_cmd_get_fw_info(struct cxlmi_endpoint *ep);
int cxl_cmd_get_os_fw_info(struct cxlmi_endpoint *ep);
int cxl_cmd_update_device_fw(struct cxlmi_endpoint *ep, bool is_os,
                             struct _update_fw_params *fw_params);
int cxl_cmd_get_timestamp(struct cxlmi_endpoint *ep);
int cxl_cmd_set_timestamp(struct cxlmi_endpoint *ep, uint64_t timestamp);
int cxl_cmd_get_event_records(struct cxlmi_endpoint *ep, uint8_t type);
int cxl_cmd_clear_event_records(struct cxlmi_endpoint *ep, uint8_t type,
                                uint8_t flags, uint16_t handles);
int cxl_cmd_get_event_interrupt_policy(struct cxlmi_endpoint *ep);
int cxl_cmd_set_event_interrupt_policy(struct cxlmi_endpoint *ep,
                                       uint32_t interrupt_policy);

int cxl_cmd_dimm_spd_read(struct cxlmi_endpoint *ep, uint32_t spd_id,
                          uint32_t offset, uint32_t num_bytes);
int cxl_cmd_dimm_slot_info(struct cxlmi_endpoint *ep);
int cxl_cmd_read_ddr_temp(struct cxlmi_endpoint *ep);

int cxl_cmd_health_counters_clear(struct cxlmi_endpoint *ep, uint32_t bitmask);
int cxl_cmd_health_counters_get(struct cxlmi_endpoint *ep);

int cxl_cmd_pmic_vtmon_info(struct cxlmi_endpoint *ep);
int cxl_cmd_read_ltssm_states(struct cxlmi_endpoint *ep);
int cxl_cmd_pcie_eye_run(struct cxlmi_endpoint *ep, uint8_t lane,
                         uint8_t sw_scan, uint8_t ber);
int cxl_cmd_pcie_eye_status(struct cxlmi_endpoint *ep);
int cxl_cmd_pcie_eye_get(struct cxlmi_endpoint *ep, uint32_t sw_scan,
                         uint32_t ber);
int cxl_cmd_get_cxl_link_status(struct cxlmi_endpoint *ep);
int cxl_cmd_get_device_info(struct cxlmi_endpoint *ep);
int cxl_cmd_get_ddr_bw(struct cxlmi_endpoint *ep, uint32_t timeout,
                       uint32_t iterations);
int cxl_cmd_ddr_margin_run(struct cxlmi_endpoint *ep, uint8_t slice_num,
                           uint8_t rd_wr_margin, uint8_t ddr_id);
int cxl_cmd_ddr_margin_status(struct cxlmi_endpoint *ep);
int cxl_cmd_ddr_margin_get(struct cxlmi_endpoint *ep);
int cxl_cmd_reboot_mode_set(struct cxlmi_endpoint *ep, uint8_t reboot_mode);
int cxl_cmd_curr_cxl_boot_mode_get(struct cxlmi_endpoint *ep);
int cxl_cmd_get_ddr_ecc_err_info(struct cxlmi_endpoint *ep);
int cxl_cmd_i2c_read(struct cxlmi_endpoint *ep, uint16_t slave_addr,
                     uint8_t reg_addr, uint8_t num_bytes);
int cxl_cmd_i2c_write(struct cxlmi_endpoint *ep, uint16_t slave_addr,
                      uint8_t reg_addr, uint8_t data);
int cxl_cmd_get_ddr_latency(struct cxlmi_endpoint *ep, uint32_t measure_time);
int cxl_cmd_get_membridge_errors(struct cxlmi_endpoint *ep);
int cxl_cmd_hpa_to_dpa(struct cxlmi_endpoint *ep, uint64_t hpa_address);
int cxl_cmd_start_ddr_ecc_scrub(struct cxlmi_endpoint *ep);
int cxl_cmd_ddr_ecc_scrub_status(struct cxlmi_endpoint *ep);
int cxl_cmd_ddr_init_status(struct cxlmi_endpoint *ep);
int cxl_cmd_get_membridge_stats(struct cxlmi_endpoint *ep);
int cxl_cmd_ddr_err_inj_en(struct cxlmi_endpoint *ep, uint32_t ddr_id,
                           uint32_t err_type, uint64_t ecc_fwc_mask);
int cxl_cmd_trigger_coredump(struct cxlmi_endpoint *ep);
int cxl_cmd_ddr_stats_run(struct cxlmi_endpoint *ep, uint8_t ddr_id,
                          uint32_t monitor_time, uint32_t loop_count);
int cxl_cmd_ddr_stats_get(struct cxlmi_endpoint *ep);
int cxl_cmd_ddr_param_set(struct cxlmi_endpoint *ep, uint32_t ddr_interleave_sz,
                          uint32_t ddr_interleave_ctrl_choice);
int cxl_cmd_ddr_param_get(struct cxlmi_endpoint *ep);
int cxl_cmd_ddr_dimm_level_training_status(struct cxlmi_endpoint *ep);
int cxl_cmd_viral_inj_en(struct cxlmi_endpoint *ep, uint32_t viral_type);
int cxl_cmd_mem_ll_err_inj_en(struct cxlmi_endpoint *ep, uint32_t en_dis,
                              uint32_t ll_err_type);
int cxl_cmd_pci_err_inj_en(struct cxlmi_endpoint *ep, uint32_t en_dis,
                           uint32_t type, uint32_t err, uint32_t count,
                           uint32_t opt1, uint32_t opt2);
int cxl_cmd_core_volt_set(struct cxlmi_endpoint *ep, float core_volt);
int cxl_cmd_core_volt_get(struct cxlmi_endpoint *ep);
int cxl_cmd_ddr_cont_scrub_status(struct cxlmi_endpoint *ep);
int cxl_cmd_ddr_cont_scrub_set(struct cxlmi_endpoint *ep,
                               uint32_t cont_scrub_status);
int cxl_cmd_ddr_page_select_set(struct cxlmi_endpoint *ep,
                                uint8_t page_select_option);
int cxl_cmd_ddr_page_select_get(struct cxlmi_endpoint *ep);
int cxl_cmd_ddr_hppr_set(struct cxlmi_endpoint *ep,
                         uint64_t hppr_enable_option);
int cxl_cmd_ddr_hppr_get(struct cxlmi_endpoint *ep);
int cxl_cmd_ddr_hppr_addr_info_set(struct cxlmi_endpoint *ep, uint8_t ddr_id,
                                   uint8_t chip_select, uint8_t bank_group,
                                   uint8_t bank, uint32_t row);
int cxl_cmd_ddr_hppr_addr_info_get(struct cxlmi_endpoint *ep);
int cxl_cmd_ddr_hppr_addr_info_clear(struct cxlmi_endpoint *ep, uint8_t ddr_id,
                                     uint8_t channel_id);
int cxl_cmd_ddr_ppr_get_status(struct cxlmi_endpoint *ep);
int cxl_cmd_ddr_refresh_mode_set(struct cxlmi_endpoint *ep,
                                 uint8_t refresh_mode);
int cxl_cmd_ddr_refresh_mode_get(struct cxlmi_endpoint *ep);
int cxl_cmd_cxl_err_cntr_get(struct cxlmi_endpoint *ep);
int cxl_cmd_ddr_freq_get(struct cxlmi_endpoint *ep);
int cxl_cmd_ddr_init_err_info_get(struct cxlmi_endpoint *ep);

/* helper functions */
const char *get_devname(struct cxlmi_endpoint *ep);

#ifdef __cplusplus
}
#endif
#endif /* __CXL_CMD_H__ */
