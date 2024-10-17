// SPDX-License-Identifier: LGPL-2.1-or-later

/* std includes */
#include <errno.h>

/* libcxlmi includes */
#include <libcxlmi.h>

/* vendor includes */
#include "cxl_cmd.h"
#include <parse_option.h>
#include <util_main.h>
#include <vendor_commands.h>
#include <vendor_types.h>

/* Length of UUID in bytes */
#define LOG_UUID_LEN 0x10

static const uint8_t cel_uuid[LOG_UUID_LEN] = {
    0x0d, 0xa9, 0xc0, 0xb5, 0xbf, 0x41, 0x4b, 0x78,
    0x8f, 0x79, 0x96, 0xb1, 0x62, 0x3b, 0x3f, 0x17};

static const uint8_t ven_dbg[LOG_UUID_LEN] = {
    0x5e, 0x18, 0x19, 0xd9, 0x11, 0xa9, 0x40, 0x0c,
    0x81, 0x1f, 0xd6, 0x07, 0x19, 0x40, 0x3d, 0x86};

int cxl_cmd_get_supported_logs(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_get_supported_logs *gsl;

  gsl = calloc(1,
               sizeof(*gsl) + CXLMI_MAX_SUPPORTED_LOGS * sizeof(*gsl->entries));
  if (!gsl) {
    printf("Failed to allocate memory\r\n");
    return -ENOMEM;
  }

  rc = cxlmi_cmd_get_supported_logs(ep, NULL, gsl);
  if (!rc) {
    printf("Get Supported Logs Response : %s\n", get_devname(ep));
    printf("Entries: %d\n", gsl->num_supported_log_entries);
    for (int i = 0; i < gsl->num_supported_log_entries; i++) {
      char uuid[40];
      uuid_unparse(gsl->entries[i].uuid, uuid);
      printf("\tentries[%d] uuid: %s, size: %d\n", i, uuid,
             gsl->entries[i].log_size);
    }
  }

  free(gsl);

  return rc;
}

int cxl_cmd_get_log(struct cxlmi_endpoint *ep, const char *log_uuid,
                    uint32_t log_size) {
  uint32_t max_payload = log_size;
  struct cxlmi_cmd_get_log_req in;
  struct cxlmi_cmd_get_log_cel_rsp *ret;
  int i, rc;

  if (!log_uuid) {
    printf("%s: Please specify log uuid argument\n", get_devname(ep));
    return CXLMI_RET_INPUT; //-EINVAL;
  }
  uuid_parse(log_uuid, in.uuid);

  // Check of size is 0. If so, sould we query supported logs and assume size?
  if (log_size == 0) {
    struct cxlmi_cmd_get_supported_logs *gsl;
    int j = 0;

    gsl = calloc(1, sizeof(*gsl) +
                        CXLMI_MAX_SUPPORTED_LOGS * sizeof(*gsl->entries));
    if (!gsl) {
      printf("Failed to allocate memory\r\n");
      return -ENOMEM;
    }

    rc = cxlmi_cmd_get_supported_logs(ep, NULL, gsl);
    if (!rc) {
      for (i = 0; i < gsl->num_supported_log_entries; i++) {
        for (j = 0; j < sizeof(gsl->entries[i].uuid); j++) {
          if (gsl->entries[i].uuid[j] != in.uuid[j])
            break;
        }
        if (j == LOG_UUID_LEN) {
          max_payload = gsl->entries[i].log_size;
          printf("Log available (using non-size size), size=%d\n", max_payload);
        }
      }
    }

    free(gsl);
  }

  in.offset = 0;
  in.length = max_payload;
  ret = calloc(1, sizeof(*ret) + max_payload);
  if (!ret) {
    printf("Failed to allocate memory\r\n");
    return -ENOMEM;
  }

  rc = cxlmi_cmd_get_log_cel(ep, NULL, &in, ret);
  if (rc)
    goto done;
  printf("Log payload info: %s\n", get_devname(ep));
  printf("    out size: 0x%x\n", in.length);

  /* Check if CEL logs */
  for (i = 0; i < sizeof(in.uuid); i++) {
    if (in.uuid[i] != cel_uuid[i])
      break;
  }
  if (i == LOG_UUID_LEN) {
    uint32_t cel_size = max_payload / sizeof(*ret);
    printf("    no_cel_entries size: %d\n", cel_size);

    for (int e = 0; e < cel_size; ++e) {
      printf("    cel_entry[%d] opcode: 0x%x, effect: 0x%x\n", e,
             (ret[e].opcode), (ret[e].command_effect));
    }
    goto done;
  }
  /* Check if Vendor Logs*/
  for (i = 0; i < sizeof(in.uuid); i++) {
    if (in.uuid[i] != ven_dbg[i])
      break;
  }
  if (i == LOG_UUID_LEN) {
    printf("    number of received bytes: %d\n", max_payload);
    printf("%s\n", (char *)ret);
    goto done;
  }

  /* If control reached here, it means unsupport log is provided*/
  printf("Invalid log UUID:%s\n", log_uuid);

done:
  free(ret);

  return rc;
}

int cxl_cmd_get_alert_config(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_memdev_get_alert_config alert_config;

  rc = cxlmi_cmd_memdev_get_alert_config(ep, NULL, &alert_config);
  if (!rc) {
    printf("Alert Config Summary : %s\n", get_devname(ep));

    printf("    valid_alerts: 0x%x\n", alert_config.valid_alerts);
    printf("    programmable_alerts: 0x%x\n", alert_config.programmable_alerts);
    printf("    life_used_critical_alert_threshold: 0x%x\n",
           alert_config.life_used_critical_alert_threshold);
    printf("    life_used_prog_warn_threshold: 0x%x\n",
           alert_config.life_used_programmable_warning_threshold);
    printf("    dev_over_temp_crit_alert_threshold: 0x%x\n",
           alert_config.device_over_temperature_critical_alert_threshold);
    printf("    dev_under_temp_crit_alert_threshold: 0x%x\n",
           alert_config.device_under_temperature_critical_alert_threshold);
    printf("    dev_over_temp_prog_warn_threshold: 0x%x\n",
           alert_config.device_over_temperature_programmable_warning_threshold);
    printf(
        "    dev_under_temp_prog_warn_threshold: 0x%x\n",
        alert_config.device_under_temperature_programmable_warning_threshold);
    printf("    corr_vol_mem_err_prog_warn_thresold: 0x%x\n",
           alert_config
               .corrected_volatile_mem_error_programmable_warning_threshold);
    printf("    corr_pers_mem_err_prog_warn_threshold: 0x%x\n",
           alert_config
               .corrected_persistent_mem_error_programmable_warning_threshold);
  }

  return rc;
}

int cxl_cmd_get_health_info(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_memdev_get_health_info health_info;

  rc = cxlmi_cmd_memdev_get_health_info(ep, NULL, &health_info);
  if (!rc) {
    printf("Device Health Info : %s\n", get_devname(ep));
    printf("    health_state: 0x%x\n", health_info.health_status);
    printf("    media_status: 0x%x\n", health_info.media_status);
    printf("    additional_status: 0x%x\n", health_info.additional_status);
    printf("    life_used: 0x%x\n", health_info.life_used);
    printf("    device_temp: 0x%x\n", health_info.device_temperature);
    printf("    dirty_shutdown_count: 0x%x\n",
           health_info.dirty_shutdown_count);
    printf("    corr_vol_mem_err_count: 0x%x\n",
           health_info.corrected_volatile_error_count);
    printf("    corr_pers_mem_err_count: 0x%x\n",
           health_info.corrected_persistent_error_count);
  }

  return rc;
}

int cxl_cmd_get_dev_fw_info(struct cxlmi_endpoint *ep, bool is_os) {
  if (is_os)
    return cxl_cmd_get_os_fw_info(ep);
  else
    return cxl_cmd_get_fw_info(ep);
}

#define SLOT_MASK 0x07
int cxl_cmd_get_fw_info(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_get_fw_info fw_info;
  uint8_t slotmask = SLOT_MASK;

  rc = cxlmi_cmd_get_fw_info(ep, NULL, &fw_info);
  if (!rc) {
    printf("================================= %s : get fw info "
           "==================================\r\n",
           get_devname(ep));
    printf("FW Slots Supported: %d\n", fw_info.slots_supported);

    printf("Active FW Slot: %x\n", fw_info.slot_info & slotmask);
    printf("Staged FW Slot: %x\n", (fw_info.slot_info >> 3) & slotmask);
    printf("FW Activation Capabilities: %x\n", fw_info.caps);
    printf("Slot 1 FW Revision: %s\n", fw_info.fw_rev1);
    printf("Slot 2 FW Revision: %s\n", fw_info.fw_rev2);
    printf("Slot 3 FW Revision: %s\n", fw_info.fw_rev3);
    printf("Slot 4 FW Revision: %s\n", fw_info.fw_rev4);
  }

  return rc;
}

int cxl_cmd_get_os_fw_info(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_get_fw_info fw_info;
  uint8_t slotmask = SLOT_MASK;

  rc = cxlmi_cmd_get_os_fw_info(ep, NULL, &fw_info);
  if (!rc) {
    printf("================================= %s : get fw info "
           "==================================\r\n",
           get_devname(ep));
    printf("OS Slots Supported: %d\n", fw_info.slots_supported);

    printf("Active OS Slot: %x\n", fw_info.slot_info & slotmask);
    printf("Staged OS Slot: %x\n", (fw_info.slot_info >> 3) & slotmask);
    printf("OS Activation Capabilities: %x\n", fw_info.caps);
    printf("Slot 1 OS Revision: %s\n", fw_info.fw_rev1);
    printf("Slot 2 OS Revision: %s\n", fw_info.fw_rev2);
    printf("Slot 3 OS Revision: %s\n", fw_info.fw_rev3);
    printf("Slot 4 OS Revision: %s\n", fw_info.fw_rev4);
  }

  return rc;
}
