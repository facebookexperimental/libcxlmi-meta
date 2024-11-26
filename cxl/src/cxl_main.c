// SPDX-License-Identifier: LGPL-2.1-or-later

/* std includes */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* libcxlmi includes */
#include <libcxlmi.h>

/* vendor includes */
#include "cxl_cmd.h"
#include "cxl_main.h"
#include <util_main.h>
#include <vendor_commands.h>

/* List of support commands and respective handlers */
struct cmd_struct commands[] = {
    {STR_HELP, cmd_print_help},
    {STR_IDENTIFY, cmd_identify},
    {STR_GET_SUPPORTED_LOGS, cmd_get_supported_logs},
    {STR_GET_LOG, cmd_get_log},
    {STR_GET_ALERT_CONFIG, cmd_get_alert_config},
    {STR_SET_ALERT_CONFIG, cmd_set_alert_config},
    {STR_GET_HEALTH_INFO, cmd_get_health_info},
    {STR_GET_FW_INFO, cmd_get_fw_info},
    {STR_UPDATE_FW, cmd_update_fw},
    {STR_GET_TIMESTAMP, cmd_get_timestamp},
    {STR_SET_TIMESTAMP, cmd_set_timestamp},
    {STR_GET_EVENT_RECORDS, cmd_get_event_records},
    {STR_CLEAR_EVENT_RECORDS, cmd_clear_event_records},
    {STR_GET_EVENT_INTERRUPT_POLICY, cmd_get_event_interrupt_policy},
    {STR_SET_EVENT_INTERRUPT_POLICY, cmd_set_event_interrupt_policy},
    /* vendor commands */
    {STR_DIMM_SPD_READ, cmd_dimm_spd_read},
    {STR_DIMM_SLOT_INFO, cmd_dimm_slot_info},
    {STR_READ_DDR_TEMP, cmd_read_ddr_temp},
    {STR_HEALTH_COUNTERS_CLEAR, cmd_health_counters_clear},
    {STR_HEALTH_COUNTERS_GET, cmd_health_counters_get},
    {STR_PMIC_VTMON_INFO, cmd_pmic_vtmon_info},
    {STR_READ_LTSSM_STATUS, cmd_read_ltssm_states},
    {STR_PCI_EYE_RUN, cmd_pcie_eye_run},
    {STR_PCIE_EYE_STATUS, cmd_pcie_eye_status},
    {STR_PCIE_EYE_GET, cmd_pcie_eye_get},
    {STR_GET_CXL_LINK_STATUS, cmd_get_cxl_link_status},
    {STR_GET_DEVICE_INFO, cmd_get_device_info},
    {STR_GET_DDR_BW, cmd_get_ddr_bw},
    {STR_DDR_MARGIN_RUN, cmd_ddr_margin_run},
    {STR_DDR_MARGIN_STATUS, cmd_ddr_margin_status},
    {STR_DDR_MARGIN_GET, cmd_ddr_margin_get},
    {STR_REBOOT_MODE_SET, cmd_reboot_mode_set},
    {STR_CURR_CXL_BOOT_MODE_GET, cmd_curr_cxl_boot_mode_get},
    {STR_GET_DDR_ECC_ERR_INFO, cmd_get_ddr_ecc_err_info},
    {STR_I2C_READ, cmd_i2c_read},
    {STR_I2C_WRITE, cmd_i2c_write},
    {STR_GET_DDR_LATENCY, cmd_get_ddr_latency},
    {STR_GET_MEMBRIDGE_ERRORS, cmd_get_membridge_errors},
    {STR_HPA_TO_DPA, cmd_hpa_to_dpa},
    {STR_START_DDR_ECC_SCRUB, cmd_start_ddr_ecc_scrub},
    {STR_DDR_ECC_SCRUB_STATUS, cmd_ddr_ecc_scrub_status},
    {STR_DDR_INIT_STATUS, cmd_ddr_init_status},
    {STR_GET_MEMBRIDGE_STATS, cmd_get_membridge_stats},
    {STR_DDR_ERR_INJ_EN, cmd_ddr_err_inj_en},
    {STR_TRIGGER_COREDUMP, cmd_trigger_coredump},
    {STR_DDR_STATS_RUN, cmd_ddr_stats_run},
    {STR_DDR_STATS_GET, cmd_ddr_stats_get},
    {STR_DDR_PARAM_SET, cmd_ddr_param_set},
    {STR_DDR_PARAM_GET, cmd_ddr_param_get},
    {STR_DDR_DIMM_LEVEL_TRAINING_STATUS, cmd_ddr_dimm_level_training_status},
    {STR_OEM_ERR_INJ_VIRAL, cmd_viral_inj_en},
    {STR_ERR_INJ_LL_POISON, cmd_mem_ll_err_inj_en},
    {STR_PCI_ERR_INJ, cmd_pci_err_inj_en},
    {STR_CORE_VOLT_SET, cmd_core_volt_set},
    {STR_CORE_VOLT_GET, cmd_core_volt_get},
    {STR_DDR_CONT_SCRUB_STATUS, cmd_ddr_cont_scrub_status},
    {STR_DDR_CONT_SCRUB_SET, cmd_ddr_cont_scrub_set},
    {STR_DDR_PAGE_SELECT_SET, cmd_ddr_page_select_set},
    {STR_DDR_PAGE_SELECT_GET, cmd_ddr_page_select_get},
    {STR_DDR_HPPR_SET, cmd_ddr_hppr_set},
    {STR_DDR_HPPR_GET, cmd_ddr_hppr_get},
    {STR_DDR_HPPR_ADDR_INFO_SET, cmd_ddr_hppr_addr_info_set},
    {STR_DDR_HPPR_ADDR_INFO_GET, cmd_ddr_hppr_addr_info_get},
    {STR_DDR_HPPR_ADDR_INFO_CLEAR, cmd_ddr_hppr_addr_info_clear},
    {STR_DDR_PPR_GET_STATUS, cmd_ddr_ppr_get_status},
    {STR_DDR_REFRESH_MODE_SET, cmd_ddr_refresh_mode_set},
    {STR_DDR_REFRESH_MODE_GET, cmd_ddr_refresh_mode_get},
    {STR_CXL_ERR_CNTR_GET, cmd_cxl_err_cntr_get},
    {STR_DDR_FREQ_GET, cmd_ddr_freq_get},
    {STR_DDR_INIT_ERR_INFO_GET, cmd_ddr_init_err_info_get},
};

const char cxl_usage_string[] = "cxl COMMAND [ARGS]";

int cmd_print_help(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  fprintf(stderr, "%s\n", cxl_usage_string);
  fprintf(stderr, "Usage: %s <cmd> <device>\n", "cxl");
  fprintf(stderr, "<device> device_name (ex: mem0)\n");
  fprintf(stderr, "%s --list-cmds to see all available commands\n", "cxl");

  return 0;
}

int main(int argc, const char **argv) {
  struct cxlmi_ctx *ctx = NULL;
  int rc = EXIT_FAILURE;

  if (argc < 2) {
    cmd_print_help(argc, argv, NULL);
    goto exit;
  }

  ctx = cxlmi_new_ctx(stdout, DEFAULT_LOGLEVEL);
  if (!ctx) {
    fprintf(stderr, "cannot create new context object\n");
    goto exit;
  }

  /* Look for flags.. */
  argv++;
  argc--;
  main_handle_options(&argv, &argc, cxl_usage_string, commands,
                      ARRAY_SIZE(commands));

  if (argc < 1) {
    cmd_print_help(argc, argv, NULL);
    goto exit_free_ctx;
  }

  main_handle_internal_command(argc, argv, ctx, commands, ARRAY_SIZE(commands));

exit_free_ctx:
  if (ctx)
    cxlmi_free_ctx(ctx);
exit:
  return rc;
}
