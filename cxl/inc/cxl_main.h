// SPDX-License-Identifier: LGPL-2.1-or-later
#ifndef __CXL_MAIN_H__
#define __CXL_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

#define STR_HELP "help"

/* standard commands */
#define STR_IDENTIFY "id-cmd"
#define STR_GET_SUPPORTED_LOGS "get-supported-logs"
#define STR_GET_LOG "get-log"
#define STR_GET_ALERT_CONFIG "get-alert-config"
#define STR_SET_ALERT_CONFIG "set-alert-config"
#define STR_GET_HEALTH_INFO "get-health-info"
#define STR_GET_FW_INFO "get-fw-info"
#define STR_UPDATE_FW "update-fw"
#define STR_GET_TIMESTAMP "get-timestamp"
#define STR_SET_TIMESTAMP "set-timestamp"
#define STR_GET_EVENT_RECORDS "get-event-records"
#define STR_CLEAR_EVENT_RECORDS "clear-event-records"
#define STR_GET_EVENT_INTERRUPT_POLICY "get-event-interrupt-policy"
#define STR_SET_EVENT_INTERRUPT_POLICY "set-event-interrupt-policy"

/* vendor commands */
#define STR_DIMM_SPD_READ "dimm-spd-read"
#define STR_DIMM_SLOT_INFO "dimm-slot-info"
#define STR_READ_DDR_TEMP "read-ddr-temp"

#define STR_HEALTH_COUNTERS_CLEAR "health-counters-clear"
#define STR_HEALTH_COUNTERS_GET "health-counters-get"

#define STR_PMIC_VTMON_INFO "pmic-vtmon-info"
#define STR_READ_LTSSM_STATUS "read-ltssm-states"
#define STR_PCI_EYE_RUN "pcie-eye-run"
#define STR_PCIE_EYE_STATUS "pcie-eye-status"
#define STR_PCIE_EYE_GET "pcie-eye-get"
#define STR_GET_CXL_LINK_STATUS "get-cxl-link-status"
#define STR_GET_DEVICE_INFO "get-device-info"
#define STR_GET_DDR_BW "get-ddr-bw"
#define STR_DDR_MARGIN_RUN "ddr-margin-run"
#define STR_DDR_MARGIN_STATUS "ddr-margin-status"
#define STR_DDR_MARGIN_GET "ddr-margin-get"
#define STR_REBOOT_MODE_SET "reboot-mode-set"
#define STR_CURR_CXL_BOOT_MODE_GET "curr-cxl-boot-mode-get"
#define STR_GET_DDR_ECC_ERR_INFO "get-ddr-ecc-err-info"
#define STR_I2C_READ "i2c-read"
#define STR_I2C_WRITE "i2c-write"
#define STR_GET_DDR_LATENCY "get-ddr-latency"
#define STR_GET_MEMBRIDGE_ERRORS "get-cxl-membridge-errors"
#define STR_HPA_TO_DPA "cxl-hpa-to-dpa"
#define STR_START_DDR_ECC_SCRUB "start-ddr-ecc-scrub"
#define STR_DDR_ECC_SCRUB_STATUS "ddr-ecc-scrub-status"
#define STR_DDR_INIT_STATUS "ddr-init-status"
#define STR_GET_MEMBRIDGE_STATS "get-cxl-membridge-stats"
#define STR_DDR_ERR_INJ_EN "ddr-err-inj-en"
#define STR_TRIGGER_COREDUMP "trigger-coredump"
#define STR_DDR_STATS_RUN "ddr-stats-run"
#define STR_DDR_STATS_GET "ddr-stats-get"
#define STR_DDR_PARAM_SET "ddr-param-set"
#define STR_DDR_PARAM_GET "ddr-param-get"
#define STR_DDR_DIMM_LEVEL_TRAINING_STATUS "ddr-dimm-level-training-status"
#define STR_OEM_ERR_INJ_VIRAL "oem-err-inj-viral"
#define STR_ERR_INJ_LL_POISON "err-inj-ll-poison"
#define STR_PCI_ERR_INJ "pci-err-inj"
#define STR_CORE_VOLT_SET "core-volt-set"
#define STR_CORE_VOLT_GET "core-volt-get"
#define STR_DDR_CONT_SCRUB_STATUS "ddr-cont-scrub-status"
#define STR_DDR_CONT_SCRUB_SET "ddr-cont-scrub-set"
#define STR_DDR_PAGE_SELECT_SET "ddr-page-select-set"
#define STR_DDR_PAGE_SELECT_GET "ddr-page-select-get"
#define STR_DDR_HPPR_SET "ddr-hppr-set"
#define STR_DDR_HPPR_GET "ddr-hppr-get"
#define STR_DDR_HPPR_ADDR_INFO_SET "ddr-hppr-addr-info-set"
#define STR_DDR_HPPR_ADDR_INFO_GET "ddr-hppr-addr-info-get"
#define STR_DDR_HPPR_ADDR_INFO_CLEAR "ddr-hppr-addr-info-clear"
#define STR_DDR_PPR_GET_STATUS "ddr-ppr-status-get"
#define STR_DDR_REFRESH_MODE_SET "ddr-refresh-mode-set"
#define STR_DDR_REFRESH_MODE_GET "ddr-refresh-mode-get"
#define STR_CXL_ERR_CNTR_GET "cxl-err-cnt-get"
#define STR_DDR_FREQ_GET "ddr-freq-get"
#define STR_DDR_INIT_ERR_INFO_GET "ddr-err-bist-info-get"

#ifdef __cplusplus
}
#endif
#endif /* __CXL_MAIN_H__ */
