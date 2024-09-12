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
#include <vendor_commands.h>

static int show_memdev_info(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_memdev_identify id;

  rc = cxlmi_cmd_memdev_identify(ep, NULL, &id);
  if (rc)
    return rc;

  printf("FW revision: %s\n", id.fw_revision);
  printf("total capacity: %ld Mb\n", 256 * id.total_capacity);
  printf("\tvolatile: %ld Mb\n", 256 * id.volatile_capacity);
  printf("\tpersistent: %ld Mb\n", 256 * id.persistent_capacity);
  printf("lsa size: %d bytes\n", id.lsa_size);
  printf("poison injection limit: %d\n", id.inject_poison_limit);
  printf("poison caps 0x%x\n", id.poison_caps);
  printf("DC event log size %d\n", id.dc_event_log_size);

  return 0;
}

static int show_some_info_from_all_devices(struct cxlmi_ctx *ctx) {
  int rc = 0;
  struct cxlmi_endpoint *ep;

  cxlmi_for_each_endpoint(ctx, ep) {
    struct cxlmi_cmd_identify id;

    rc = cxlmi_cmd_identify(ep, NULL, &id);
    if (rc)
      break;

    printf("serial number: 0x%lx\n", (uint64_t)id.serial_num);

    switch (id.component_type) {
    case 0x00:
      printf("device type: CXL Switch\n");
      printf("VID:%04x DID:%04x\n", id.vendor_id, id.device_id);
      break;
    case 0x03:
      printf("device type: CXL Type3 Device\n");
      printf("VID:%04x DID:%04x SubsysVID:%04x SubsysID:%04x\n", id.vendor_id,
             id.device_id, id.subsys_vendor_id, id.subsys_id);

      show_memdev_info(ep);
      break;
    case 0x04:
      printf("GFD not supported\n");
      /* fallthrough */
    default:
      break;
    }
  }

  return rc;
}

static int toggle_abort(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_bg_op_status sts;

  rc = cxlmi_cmd_bg_op_status(ep, NULL, &sts);
  if (rc)
    goto done;

  if (!(sts.status & (1 << 0))) {
    printf("no background operation in progress...\n");

    rc = cxlmi_cmd_memdev_sanitize(ep, NULL);
    if (rc && rc != CXLMI_RET_BACKGROUND) {
      printf("could not start sanitize: %s\n", cxlmi_cmd_retcode_tostr(rc));
      goto done;
    } else {
      printf("sanitizing op started\n");
      sleep(1);
    }
  }

  rc = cxlmi_cmd_request_bg_op_abort(ep, NULL);
  if (rc) {
    if (rc > 0)
      printf("request_bg_operation_abort error: %s\n",
             cxlmi_cmd_retcode_tostr(rc));
  } else
    printf("background operation abort requested\n");
done:
  return rc;
}

static int play_with_device_timestamp(struct cxlmi_endpoint *ep) {
  int rc;
  uint64_t orig_ts;
  struct cxlmi_cmd_get_timestamp get_ts;
  struct cxlmi_cmd_set_timestamp set_ts = {
      .timestamp = 946684800, /* Jan 1, 2000 */
  };

  rc = cxlmi_cmd_get_timestamp(ep, NULL, &get_ts);
  if (rc)
    return rc;
  printf("device timestamp: %lu\n", get_ts.timestamp);
  orig_ts = get_ts.timestamp;

  rc = cxlmi_cmd_set_timestamp(ep, NULL, &set_ts);
  if (rc)
    return rc;

  memset(&get_ts, 0, sizeof(get_ts));
  rc = cxlmi_cmd_get_timestamp(ep, NULL, &get_ts);
  if (rc)
    return rc;
  printf("new device timestamp: %lu\n", get_ts.timestamp);

  memset(&set_ts, 0, sizeof(set_ts));
  set_ts.timestamp = orig_ts;
  rc = cxlmi_cmd_set_timestamp(ep, NULL, &set_ts);
  if (rc) {
    if (rc > 0)
      printf("set_timestamp error: %s\n", cxlmi_cmd_retcode_tostr(rc));
    return rc;
  }

  memset(&get_ts, 0, sizeof(get_ts));
  rc = cxlmi_cmd_get_timestamp(ep, NULL, &get_ts);
  if (rc)
    return rc;
  printf("reset back to original device timestamp: %lu\n", get_ts.timestamp);

  return 0;
}

static const uint8_t cel_uuid[0x10] = {0x0d, 0xa9, 0xc0, 0xb5, 0xbf, 0x41,
                                       0x4b, 0x78, 0x8f, 0x79, 0x96, 0xb1,
                                       0x62, 0x3b, 0x3f, 0x17};

static const uint8_t ven_dbg[0x10] = {0x5e, 0x18, 0x19, 0xd9, 0x11, 0xa9,
                                      0x40, 0x0c, 0x81, 0x1f, 0xd6, 0x07,
                                      0x19, 0x40, 0x3d, 0x86};

static const uint8_t c_s_dump[0x10] = {0xb3, 0xfa, 0xb4, 0xcf, 0x01, 0xb6,
                                       0x43, 0x32, 0x94, 0x3e, 0x5e, 0x99,
                                       0x62, 0xf2, 0x35, 0x67};

static int parse_supported_logs(struct cxlmi_cmd_get_supported_logs *pl,
                                size_t *cel_size) {
  int i, j;

  *cel_size = 0;
  printf("Get Supported Logs Response %d\n", pl->num_supported_log_entries);

  for (i = 0; i < pl->num_supported_log_entries; i++) {
    for (j = 0; j < sizeof(pl->entries[i].uuid); j++) {
      if (pl->entries[i].uuid[j] != cel_uuid[j])
        break;
    }
    if (j == 0x10) {
      *cel_size = pl->entries[i].log_size;
      printf("\tCommand Effects Log (CEL) available\n");
    }
    for (j = 0; j < sizeof(pl->entries[i].uuid); j++) {
      if (pl->entries[i].uuid[j] != ven_dbg[j])
        break;
    }
    if (j == 0x10)
      printf("\tVendor Debug Log available\n");
    for (j = 0; j < sizeof(pl->entries[i].uuid); j++) {
      if (pl->entries[i].uuid[j] != c_s_dump[j])
        break;
    }
    if (j == 0x10)
      printf("\tComponent State Dump Log available\n");
  }
  if (cel_size == 0) {
    return -1;
  }
  return 0;
}

static int show_cel(struct cxlmi_endpoint *ep, int cel_size) {
  struct cxlmi_cmd_get_log_req in = {
      .offset = 0,
      .length = cel_size,
  };
  struct cxlmi_cmd_get_log_cel_rsp *ret;
  int i, rc;

  ret = calloc(1, sizeof(*ret) + cel_size);
  if (!ret)
    return -1;

  memcpy(in.uuid, cel_uuid, sizeof(in.uuid));
  rc = cxlmi_cmd_get_log_cel(ep, NULL, &in, ret);
  if (rc)
    goto done;

  for (i = 0; i < cel_size / sizeof(*ret); i++) {
    printf("\t[%04x] %s%s%s%s%s%s%s%s\n", ret[i].opcode,
           ret[i].command_effect & 0x1 ? "ColdReset " : "",
           ret[i].command_effect & 0x2 ? "ImConf " : "",
           ret[i].command_effect & 0x4 ? "ImData " : "",
           ret[i].command_effect & 0x8 ? "ImPol " : "",
           ret[i].command_effect & 0x10 ? "ImLog " : "",
           ret[i].command_effect & 0x20 ? "ImSec" : "",
           ret[i].command_effect & 0x40 ? "BgOp" : "",
           ret[i].command_effect & 0x80 ? "SecSup" : "");
  }
done:
  free(ret);
  return rc;
}

static int get_device_logs(struct cxlmi_endpoint *ep) {
  int rc;
  size_t cel_size;
  struct cxlmi_cmd_get_supported_logs *gsl;

  gsl = calloc(1,
               sizeof(*gsl) + CXLMI_MAX_SUPPORTED_LOGS * sizeof(*gsl->entries));
  if (!gsl)
    return -1;

  rc = cxlmi_cmd_get_supported_logs(ep, NULL, gsl);
  if (rc)
    return rc;

  rc = parse_supported_logs(gsl, &cel_size);
  if (rc)
    return rc;
  else {
    /* we know there is a CEL */
    rc = show_cel(ep, cel_size);
  }

  free(gsl);
  return rc;
}

static int get_alert_config(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_memdev_get_alert_config *alert_config;

  alert_config = calloc(1, sizeof(*alert_config));
  rc = cxlmi_cmd_memdev_get_alert_config(ep, NULL, alert_config);
  if (rc)
    return rc;
  else {
    printf("valid_alerts:0x%x\n", alert_config->valid_alerts);
    printf("programmable_alerts:0x%x\n", alert_config->programmable_alerts);
    printf("life_used_critical_alert_threshold:%d\n",
           alert_config->life_used_critical_alert_threshold);
    printf("life_used_programmable_warning_threshold:%d\n",
           alert_config->life_used_programmable_warning_threshold);
    printf("device_over_temperature_critical_alert_threshold:%d\n",
           alert_config->device_over_temperature_critical_alert_threshold);
    printf("device_under_temperature_critical_alert_threshold:%d\n",
           alert_config->device_under_temperature_critical_alert_threshold);
    printf(
        "device_over_temperature_programmable_warning_threshold:%d\n",
        alert_config->device_over_temperature_programmable_warning_threshold);
    printf(
        "device_under_temperature_programmable_warning_threshold:%d\n",
        alert_config->device_under_temperature_programmable_warning_threshold);
    printf("corrected_volatile_mem_error_programmable_warning_threshold:%d\n",
           alert_config
               ->corrected_volatile_mem_error_programmable_warning_threshold);
    printf("corrected_persistent_mem_error_programmable_warning_threshold:%d\n",
           alert_config
               ->corrected_persistent_mem_error_programmable_warning_threshold);
  }
  free(alert_config);
  return rc;
}

static int get_health_info(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_memdev_get_health_info *health_info;

  health_info = calloc(1, sizeof(*health_info));
  rc = cxlmi_cmd_memdev_get_health_info(ep, NULL, health_info);
  if (rc)
    return rc;
  else {
    printf("health_status:%d\n", health_info->health_status);
    printf("media_status:%d\n", health_info->media_status);
    printf("additional_status:%d\n", health_info->additional_status);
    printf("life_used:%d\n", health_info->life_used);
    printf("device_temperature:%d\n", health_info->device_temperature);
    printf("dirty_shutdown_count:%d\n", health_info->dirty_shutdown_count);
    printf("corrected_volatile_error_count:%d\n",
           health_info->corrected_volatile_error_count);
    printf("corrected_persistent_error_count:%d\n",
           health_info->corrected_persistent_error_count);
  }
  free(health_info);
  return rc;
}

static int get_fw_info(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_get_fw_info *fw_info;

  fw_info = calloc(1, sizeof(*fw_info));
  rc = cxlmi_cmd_get_fw_info(ep, NULL, fw_info);
  if (rc)
    return rc;
  else {
    printf("slots_supported:%d\n", fw_info->slots_supported);
    // printf("slot_info:0x%x\n", fw_info->slot_info);
    printf("active_slot:0x%x\n", fw_info->slot_info & 0x7);
    printf("stage_slot:0x%x\n", (fw_info->slot_info >> 3) & 0x7);
    printf("caps:0x%x\n", fw_info->caps);
    printf("fw_rev1:%s\n", fw_info->fw_rev1);
    printf("fw_rev2:%s\n", fw_info->fw_rev2);
    printf("fw_rev3:%s\n", fw_info->fw_rev3);
    printf("fw_rev4:%s\n", fw_info->fw_rev4);
  }
  free(fw_info);
  return rc;
}

static int get_os_fw_info(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_get_fw_info *fw_info;

  fw_info = calloc(1, sizeof(*fw_info));
  rc = cxlmi_cmd_get_os_fw_info(ep, NULL, fw_info);
  if (rc)
    return rc;
  else {
    printf("slots_supported:%d\n", fw_info->slots_supported);
    // printf("slot_info:0x%x\n", fw_info->slot_info);
    printf("os_active_slot:0x%x\n", fw_info->slot_info & 0x7);
    printf("os_stage_slot:0x%x\n", (fw_info->slot_info >> 3) & 0x7);
    printf("os_caps:0x%x\n", fw_info->caps);
    printf("os_fw_rev1:%s\n", fw_info->fw_rev1);
    printf("os_fw_rev2:%s\n", fw_info->fw_rev2);
    printf("os_fw_rev3:%s\n", fw_info->fw_rev3);
    printf("os_fw_rev4:%s\n", fw_info->fw_rev4);
  }
  free(fw_info);
  return rc;
}

const char cmd_help[] = {"help\n"
                         "show_info\n"
                         "get_dev_logs\n"
                         "toggle_abort\n"
                         "play_timestamp\n"
                         "get_alert_config\n"
                         "get_health_info\n"
                         "get_fw_info\n"
                         "get_os_fw_info\n"};

int main(int argc, char **argv) {
  struct cxlmi_ctx *ctx;
  struct cxlmi_endpoint *ep;
  int rc = EXIT_FAILURE;

  if (argc < 3) {
    fprintf(stderr, "Must provide a device name (ie: mem0)\n");
    fprintf(stderr, "Usage: cxl <device> <cmd>\n");
    fprintf(stderr, "%s\n", cmd_help);
    goto exit;
  }

  ctx = cxlmi_new_ctx(stdout, DEFAULT_LOGLEVEL);
  if (!ctx) {
    fprintf(stderr, "cannot create new context object\n");
    goto exit;
  }

  ep = cxlmi_open(ctx, argv[1]);
  if (!ep) {
    fprintf(stderr, "cannot open '%s' endpoint\n", argv[1]);
    goto exit_free_ctx;
  }

  printf("ep '%s'\n", argv[1]);

  /* yes, only 1 endpoint, but might add more */
  if (strcmp(argv[2], "show_info") == 0) {
    rc = show_some_info_from_all_devices(ctx);
  } else if (strcmp(argv[2], "play_timestamp") == 0) {
    rc = play_with_device_timestamp(ep);
  } else if (strcmp(argv[2], "get_dev_logs") == 0) {
    rc = get_device_logs(ep);
  } else if (strcmp(argv[2], "toggle_abort") == 0) {
    rc = toggle_abort(ep);
  } else if (strcmp(argv[2], "get_alert_config") == 0) {
    rc = get_alert_config(ep);
  } else if (strcmp(argv[2], "get_health_info") == 0) {
    rc = get_health_info(ep);
  } else if (strcmp(argv[2], "get_fw_info") == 0) {
    rc = get_fw_info(ep);
  } else if (strcmp(argv[2], "get_os_fw_info") == 0) {
    rc = get_os_fw_info(ep);
  } else {
    fprintf(stderr, "cmd not found:%s\n", argv[2]);
    fprintf(stderr, "%s\n", cmd_help);
  }
  cxlmi_close(ep);
exit_free_ctx:
  cxlmi_free_ctx(ctx);
exit:
  return rc;
}
