// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#ifndef __VENDOR_COMMANDS_H__
#define __VENDOR_COMMANDS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <libcxlmi.h>
#include <vendor_types.h>

int cxlmi_cmd_get_os_fw_info(struct cxlmi_endpoint *ep,
                             struct cxlmi_tunnel_info *ti,
                             struct cxlmi_cmd_get_fw_info *out);

/* Used for OS FW update/transfer and Pioneer vendor opcode for hbo-transfer-fw
 */
int cxlmi_cmd_vendor_transfer_fw(struct cxlmi_endpoint *ep,
                                 struct cxlmi_tunnel_info *ti,
                                 struct cxlmi_cmd_transfer_fw *in,
                                 uint32_t opcode);

int cxlmi_cmd_get_hbo_status(struct cxlmi_endpoint *ep,
                             struct cxlmi_tunnel_info *ti,
                             struct cxlmi_cmd_hbo_status_fields *ret);

int cxlmi_cmd_dimm_spd_read(struct cxlmi_endpoint *ep,
                            struct cxlmi_tunnel_info *ti,
                            struct cxlmi_cmd_dimm_spd_read_req *in,
                            struct cxlmi_cmd_dimm_spd_read_rsp *ret);

int cxlmi_cmd_dimm_slot_info(struct cxlmi_endpoint *ep,
                             struct cxlmi_tunnel_info *ti,
                             struct cxlmi_cmd_dimm_slot_info *ret);

int cxlmi_cmd_read_ddr_temp(struct cxlmi_endpoint *ep,
                            struct cxlmi_tunnel_info *ti,
                            struct cxlmi_cmd_read_ddr_temp *ret);

int cxlmi_cmd_health_counters_clear(struct cxlmi_endpoint *ep,
                                    struct cxlmi_tunnel_info *ti,
                                    struct cxlmi_cmd_health_counters_clear *in);

int cxlmi_cmd_health_counters_get(struct cxlmi_endpoint *ep,
                                  struct cxlmi_tunnel_info *ti,
                                  struct cxlmi_cmd_health_counters_get *ret);

int cxlmi_cmd_pmic_vtmon_info(struct cxlmi_endpoint *ep,
                              struct cxlmi_tunnel_info *ti,
                              struct cxlmi_cmd_pmic_vtmon_info *ret);

int cxlmi_cmd_read_ltssm_states(struct cxlmi_endpoint *ep,
                                struct cxlmi_tunnel_info *ti,
                                struct cxlmi_cmd_read_ltssm_states *ret);

int cxlmi_cmd_pcie_eye_run(struct cxlmi_endpoint *ep,
                           struct cxlmi_tunnel_info *ti,
                           struct cxlmi_cmd_pcie_eye_run_req *in,
                           struct cxlmi_cmd_pcie_eye_run_rsp *ret);

int cxlmi_cmd_pcie_eye_status(struct cxlmi_endpoint *ep,
                              struct cxlmi_tunnel_info *ti,
                              struct cxlmi_cmd_pcie_eye_status *ret);

int cxlmi_cmd_pcie_eye_get_sw(struct cxlmi_endpoint *ep,
                              struct cxlmi_tunnel_info *ti,
                              struct cxlmi_cmd_pcie_eye_get_sw_req *in,
                              struct cxlmi_cmd_pcie_eye_get_sw_rsp *ret);

int cxlmi_cmd_pcie_eye_get_hw(struct cxlmi_endpoint *ep,
                              struct cxlmi_tunnel_info *ti,
                              struct cxlmi_cmd_pcie_eye_get_hw *ret);

int cxlmi_cmd_pcie_eye_get_sw_ber(struct cxlmi_endpoint *ep,
                                  struct cxlmi_tunnel_info *ti,
                                  struct cxlmi_cmd_pcie_eye_get_sw_ber *ret);

int cxlmi_cmd_get_cxl_link_status(struct cxlmi_endpoint *ep,
                                  struct cxlmi_tunnel_info *ti,
                                  struct cxlmi_cmd_get_cxl_link_status *ret);

int cxlmi_cmd_get_device_info(struct cxlmi_endpoint *ep,
                              struct cxlmi_tunnel_info *ti,
                              struct cxlmi_cmd_get_device_info *ret);

int cxlmi_cmd_get_ddr_bw(struct cxlmi_endpoint *ep,
                         struct cxlmi_tunnel_info *ti,
                         struct cxlmi_cmd_get_ddr_bw_req *in,
                         struct cxlmi_cmd_get_ddr_bw_rsp *ret);

int cxlmi_cmd_ddr_margin_run(struct cxlmi_endpoint *ep,
                             struct cxlmi_tunnel_info *ti,
                             struct cxlmi_cmd_ddr_margin_run *in);

int cxlmi_cmd_ddr_margin_status(struct cxlmi_endpoint *ep,
                                struct cxlmi_tunnel_info *ti,
                                struct cxlmi_cmd_ddr_margin_status *ret);

int cxlmi_cmd_ddr_margin_get(struct cxlmi_endpoint *ep,
                             struct cxlmi_tunnel_info *ti,
                             struct cxlmi_cmd_ddr_margin_get *ret);

int cxlmi_cmd_reboot_mode_set(struct cxlmi_endpoint *ep,
                              struct cxlmi_tunnel_info *ti,
                              struct cxlmi_cmd_reboot_mode_set *in);

int cxlmi_cmd_curr_cxl_boot_mode_get(
    struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
    struct cxlmi_cmd_curr_cxl_boot_mode_get *ret);

int cxlmi_cmd_get_ddr_ecc_err_info(struct cxlmi_endpoint *ep,
                                   struct cxlmi_tunnel_info *ti,
                                   struct cxlmi_cmd_get_ddr_ecc_err_info *ret);

int cxlmi_cmd_i2c_read(struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
                       struct cxlmi_cmd_i2c_read_req *in,
                       struct cxlmi_cmd_i2c_read_rsp *ret);

int cxlmi_cmd_i2c_write(struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
                        struct cxlmi_cmd_i2c_write *in);

int cxlmi_cmd_get_ddr_latency(struct cxlmi_endpoint *ep,
                              struct cxlmi_tunnel_info *ti,
                              struct cxlmi_cmd_get_ddr_latency_req *in,
                              struct cxlmi_cmd_get_ddr_latency_rsp *ret);

int cxlmi_cmd_get_membridge_errors(struct cxlmi_endpoint *ep,
                                   struct cxlmi_tunnel_info *ti,
                                   struct cxlmi_cmd_get_membridge_errors *ret);

int cxlmi_cmd_hpa_to_dpa(struct cxlmi_endpoint *ep,
                         struct cxlmi_tunnel_info *ti,
                         struct cxlmi_cmd_hpa_to_dpa_req *in,
                         struct cxlmi_cmd_hpa_to_dpa_rsp *ret);

int cxlmi_cmd_start_ddr_ecc_scrub(struct cxlmi_endpoint *ep,
                                  struct cxlmi_tunnel_info *ti);

int cxlmi_cmd_ddr_ecc_scrub_status(struct cxlmi_endpoint *ep,
                                   struct cxlmi_tunnel_info *ti,
                                   struct cxlmi_cmd_ddr_ecc_scrub_status *ret);

int cxlmi_cmd_ddr_init_status(struct cxlmi_endpoint *ep,
                              struct cxlmi_tunnel_info *ti,
                              struct cxlmi_cmd_ddr_init_status *ret);

int cxlmi_cmd_get_membridge_stats(struct cxlmi_endpoint *ep,
                                  struct cxlmi_tunnel_info *ti,
                                  struct cxlmi_cmd_get_membridge_stats *ret);

int cxlmi_cmd_ddr_err_inj_en(struct cxlmi_endpoint *ep,
                             struct cxlmi_tunnel_info *ti,
                             struct cxlmi_cmd_ddr_err_inj_en *in);

int cxlmi_cmd_trigger_coredump(struct cxlmi_endpoint *ep,
                               struct cxlmi_tunnel_info *ti);

int cxlmi_cmd_ddr_stats_run(struct cxlmi_endpoint *ep,
                            struct cxlmi_tunnel_info *ti,
                            struct cxlmi_cmd_ddr_stats_run *in);

int cxlmi_cmd_ddr_stats_status(struct cxlmi_endpoint *ep,
                               struct cxlmi_tunnel_info *ti,
                               struct cxlmi_cmd_ddr_stats_status *ret);

int cxlmi_cmd_ddr_stats_get(struct cxlmi_endpoint *ep,
                            struct cxlmi_tunnel_info *ti,
                            struct cxlmi_cmd_ddr_stats_get_req *in,
                            cxlmi_cmd_ddr_stats_get_rsp_t *ret);

int cxlmi_cmd_ddr_param_set(struct cxlmi_endpoint *ep,
                            struct cxlmi_tunnel_info *ti,
                            struct cxlmi_cmd_ddr_param_set *in);

int cxlmi_cmd_ddr_param_get(struct cxlmi_endpoint *ep,
                            struct cxlmi_tunnel_info *ti,
                            struct cxlmi_cmd_ddr_param_get *ret);

int cxlmi_cmd_dimm_level_training_status(
    struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
    struct cxlmi_cmd_dimm_level_training_status *ret);

int cxlmi_cmd_viral_inj_en(struct cxlmi_endpoint *ep,
                           struct cxlmi_tunnel_info *ti,
                           struct cxlmi_cmd_viral_inj_en *in);

int cxlmi_cmd_mem_ll_err_inj_en(struct cxlmi_endpoint *ep,
                                struct cxlmi_tunnel_info *ti,
                                struct cxlmi_cmd_mem_ll_err_inj_en *in);

int cxlmi_cmd_pci_err_inj_en(struct cxlmi_endpoint *ep,
                             struct cxlmi_tunnel_info *ti,
                             struct cxlmi_cmd_pci_err_inj_en *in);

int cxlmi_cmd_core_volt_set(struct cxlmi_endpoint *ep,
                            struct cxlmi_tunnel_info *ti,
                            struct cxlmi_cmd_core_volt_set *in);

int cxlmi_cmd_core_volt_get(struct cxlmi_endpoint *ep,
                            struct cxlmi_tunnel_info *ti,
                            struct cxlmi_cmd_core_volt_get *ret);

int cxlmi_cmd_ddr_cont_scrub_status(
    struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
    struct cxlmi_cmd_ddr_cont_scrub_status *ret);

int cxlmi_cmd_ddr_cont_scrub_set(struct cxlmi_endpoint *ep,
                                 struct cxlmi_tunnel_info *ti,
                                 struct cxlmi_cmd_ddr_cont_scrub_set *in);

int cxlmi_cmd_ddr_page_select_set(struct cxlmi_endpoint *ep,
                                  struct cxlmi_tunnel_info *ti,
                                  struct cxlmi_cmd_ddr_page_select_set *in);

int cxlmi_cmd_ddr_page_select_get(struct cxlmi_endpoint *ep,
                                  struct cxlmi_tunnel_info *ti,
                                  struct cxlmi_cmd_ddr_page_select_get *ret);

int cxlmi_cmd_ddr_hppr_set(struct cxlmi_endpoint *ep,
                           struct cxlmi_tunnel_info *ti,
                           struct cxlmi_cmd_ddr_hppr_set *in);

int cxlmi_cmd_ddr_hppr_get(struct cxlmi_endpoint *ep,
                           struct cxlmi_tunnel_info *ti,
                           struct cxlmi_cmd_ddr_hppr_get *ret);

int cxlmi_cmd_ddr_hppr_addr_info_set(
    struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
    struct cxlmi_cmd_ddr_hppr_addr_info_set *in);

int cxlmi_cmd_ddr_hppr_addr_info_get(
    struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
    struct cxlmi_cmd_ddr_hppr_addr_info_get *ret);

int cxlmi_cmd_ddr_hppr_addr_info_clear(
    struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
    struct cxlmi_cmd_ddr_hppr_addr_info_clear *in);

int cxlmi_cmd_ddr_ppr_get_status(struct cxlmi_endpoint *ep,
                                 struct cxlmi_tunnel_info *ti,
                                 struct cxlmi_cmd_ddr_ppr_get_status *ret);

int cxlmi_cmd_ddr_refresh_mode_set(struct cxlmi_endpoint *ep,
                                   struct cxlmi_tunnel_info *ti,
                                   struct cxlmi_cmd_ddr_refresh_mode_set *in);

int cxlmi_cmd_ddr_refresh_mode_get(struct cxlmi_endpoint *ep,
                                   struct cxlmi_tunnel_info *ti,
                                   struct cxlmi_cmd_ddr_refresh_mode_get *ret);

int cxlmi_cmd_cxl_err_cntr_get(struct cxlmi_endpoint *ep,
                               struct cxlmi_tunnel_info *ti,
                               struct cxlmi_cmd_cxl_err_cntr_get *ret);

int cxlmi_cmd_ddr_freq_get(struct cxlmi_endpoint *ep,
                           struct cxlmi_tunnel_info *ti,
                           struct cxlmi_cmd_ddr_freq_get *ret);

int cxlmi_cmd_ddr_init_err_info_get(
    struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
    struct cxlmi_cmd_ddr_init_err_info_get *ret);

#ifdef __cplusplus
}
#endif

#endif /* __VENDOR_COMMANDS_H__ */
