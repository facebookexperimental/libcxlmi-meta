// SPDX-License-Identifier: LGPL-2.1-or-later

/* std includes */
#include <errno.h>
#include <stdlib.h>

/* libcxlmi includes */
#include <cxlmi/private.h>
#include <libcxlmi.h>

/* vendor includes */
#include "cxl_cmd.h"
#include "cxl_main.h"
#include <parse_option.h>
#include <util_main.h>
#include <vendor_commands.h>
#include <vendor_types.h>

#define STR_CXL_CMDS_HELP_PREFIX "cxl "
#define STR_CXL_CMDS_HELP_SUFFIX " <mem0> [<mem1>..<memN>] [<options>]"

#define STR_CXL_CMDS_HELP(STR_CMD)                                             \
  STR_CXL_CMDS_HELP_PREFIX STR_CMD STR_CXL_CMDS_HELP_SUFFIX

const char *get_devname(struct cxlmi_endpoint *ep) {
  if (ep)
    return ep->devname;
  else
    return NULL;
}

struct cxlmi_endpoint *cxlmi_ep_filter(struct cxlmi_endpoint *ep,
                                       const char *ident) {
  int ep_id;

  if (!ident || strcmp(ident, "all") == 0)
    return ep;

  if (strcmp(ident, ep->devname) == 0)
    return ep;

  if ((sscanf(ident, "%d", &ep_id) == 1 || sscanf(ident, "mem%d", &ep_id) == 1)
      /*&& cxl_memdev_get_id(ep) == ep_id*/)
    return ep;

  return NULL;
}

static int cmd_action(int argc, const char **argv, struct cxlmi_ctx *ctx,
                      int (*action)(struct cxlmi_endpoint *ep),
                      const struct option *options, const char *usage) {
  struct cxlmi_endpoint *ep = NULL;
  int i, rc = 0, count = 0, err = 0;
  const char *const u[] = {usage, NULL};
  unsigned long id;

  argc = parse_options(argc, argv, options, u, 0);
  if (argc == 0)
    usage_with_options(u, options);
  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "all") == 0) {
      argv[0] = "all";
      argc = 1;
      break;
    }
    if (sscanf(argv[i], "mem%lu", &id) != 1) {
      fprintf(stderr, "'%s' is not a valid ep name\n", argv[i]);
      err++;
    }
  }
  if (err == argc) {
    usage_with_options(u, options);
    return -CXLMI_RET_INPUT; // EINVAL;
  }

  rc = 0;
  err = 0;
  count = 0;
  if ((argc == 1) && (strcmp(argv[0], "all") == 0)) {
    // HACK: Open all the end points
    argv[0] = "mem0";
    argv[1] = "mem1";
    argc = 2;
  }

  for (i = 0; i < argc; i++) {
    if (sscanf(argv[i], "mem%lu", &id) != 1 && strcmp(argv[i], "all") != 0)
      continue;

    // printf("open '%s' endpoint\n", argv[i]);
    ep = cxlmi_open(ctx, argv[i]);
    if (!ep) {
      fprintf(stderr, "cannot open '%s' endpoint\n", argv[i]);
      continue;
    }

    cxlmi_for_each_endpoint(ctx, ep) {
      if (!cxlmi_ep_filter(ep, argv[i]))
        continue;

      rc = action(ep);
      if (rc == 0)
        count++;
      else if (rc && !err)
        err = rc;

      if (ep) {
        // printf("close '%s' endpoint\n", get_devname(ep));
        cxlmi_close(ep);
        ep = NULL;
      }
    }
  }

  /*
   * count if some actions succeeded, 0 if none were attempted,
   * negative error code otherwise.
   */
  if (count > 0)
    return count;
  return rc;
}

/* CMD_IDENTIFY */
static const struct option cmd_identify_options[] = {
    OPT_END(),
};

static int action_cmd_identify(struct cxlmi_endpoint *ep) {
  return cxl_cmd_identify(ep);
}

int cmd_identify(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_identify,
                      cmd_identify_options, STR_CXL_CMDS_HELP(STR_IDENTIFY));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* GET_SUPPORTED_LOGS */
static const struct option cmd_get_supported_logs_options[] = {
    OPT_END(),
};

static int action_cmd_get_supported_logs(struct cxlmi_endpoint *ep) {
  return cxl_cmd_get_supported_logs(ep);
}

int cmd_get_supported_logs(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_get_supported_logs,
                      cmd_get_supported_logs_options,
                      STR_CXL_CMDS_HELP(STR_GET_SUPPORTED_LOGS));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* GET_LOG */
static struct _log_uuid {
  const char *uuid;
} log_uuid;

#define LOG_UUID_OPTIONS()                                                     \
  OPT_STRING('l', "log_uuid", &log_uuid.uuid, "log-uuid", "CEL Log UUID")

static struct _log_size {
  uint32_t size;
} log_size;

#define LOG_SIZE_OPTIONS()                                                     \
  OPT_UINTEGER('s', "log_size", &log_size.size, "log-size")

static const struct option cmd_get_log_options[] = {
    LOG_UUID_OPTIONS(),
    LOG_SIZE_OPTIONS(),
    OPT_END(),
};

static int action_cmd_get_log(struct cxlmi_endpoint *ep) {
  return cxl_cmd_get_log(ep, log_uuid.uuid, log_size.size);
}

int cmd_get_log(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_get_log, cmd_get_log_options,
                      STR_CXL_CMDS_HELP(STR_GET_LOG));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* GET_ALERT_CONFIG */
static const struct option cmd_get_alert_config_options[] = {
    OPT_END(),
};

static int action_cmd_get_alert_config(struct cxlmi_endpoint *ep) {
  return cxl_cmd_get_alert_config(ep);
}

int cmd_get_alert_config(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_get_alert_config,
                      cmd_get_alert_config_options,
                      STR_CXL_CMDS_HELP(STR_GET_ALERT_CONFIG));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* SET_ALERT_CONFIG */
static struct _alert_config_params {
  uint32_t alert_prog_threshold;
  uint32_t device_temp_threshold;
  uint32_t mem_error_threshold;
} alert_config_params;

#define SET_ALERT_CONFIG_OPTIONS()                                             \
  OPT_UINTEGER('a', "alert_prog_threshold",                                    \
               &alert_config_params.alert_prog_threshold,                      \
               "Set valid, enable alert actions and life used programmable "   \
               "threshold. Fields: Valid Alert Actions (1B), Enable Alert "    \
               "Actions (1B), Life Used Programmable Warning Threshold (1B)"), \
      OPT_UINTEGER(                                                            \
          'd', "device_temp_threshold",                                        \
          &alert_config_params.device_temp_threshold,                          \
          "Set device over/under temp thresholds. Fields: Device "             \
          "Over-Temperature Programmable Warning Threshold (2B), Device "      \
          "Under-Temperature Programmable Warning Threshold (2B)"),            \
      OPT_UINTEGER(                                                            \
          'm', "mem_error_threshold",                                          \
          &alert_config_params.mem_error_threshold,                            \
          "Set memory corrected thresholds. Fields: Corrected Volatile "       \
          "Memory Error Programmable Warning Threshold (2B), Corrected "       \
          "Persistent Memory Error Programmable Warning Threshold (2B)")

static const struct option cmd_set_alert_config_options[] = {
    SET_ALERT_CONFIG_OPTIONS(),
    OPT_END(),
};

static int action_cmd_set_alert_config(struct cxlmi_endpoint *ep) {
  return cxl_cmd_set_alert_config(ep, alert_config_params.alert_prog_threshold,
                                  alert_config_params.device_temp_threshold,
                                  alert_config_params.mem_error_threshold);
}

int cmd_set_alert_config(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_set_alert_config,
                      cmd_set_alert_config_options,
                      STR_CXL_CMDS_HELP(STR_SET_ALERT_CONFIG));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* GET_HEALTH_INFO */
static const struct option cmd_get_health_info_options[] = {
    OPT_END(),
};

static int action_cmd_get_health_info(struct cxlmi_endpoint *ep) {
  return cxl_cmd_get_health_info(ep);
}

int cmd_get_health_info(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_get_health_info,
                      cmd_get_health_info_options,
                      STR_CXL_CMDS_HELP(STR_GET_HEALTH_INFO));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* GET_FW_INFO */
static struct _fw_img_params {
  bool is_os;
} fw_img_params;

#define FW_IMG_OPTIONS()                                                       \
  OPT_BOOLEAN('z', "osimage", &fw_img_params.is_os,                            \
              "select OS(a.k.a boot1) image")

static const struct option cmd_get_fw_info_options[] = {
    FW_IMG_OPTIONS(),
    OPT_END(),
};

static int action_cmd_get_fw_info(struct cxlmi_endpoint *ep) {
  return cxl_cmd_get_dev_fw_info(ep, fw_img_params.is_os);
}

int cmd_get_fw_info(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc =
      cmd_action(argc, argv, ctx, action_cmd_get_fw_info,
                 cmd_get_fw_info_options, STR_CXL_CMDS_HELP(STR_GET_FW_INFO));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* UPDATE_FW */

struct _update_fw_params update_fw_params;

#define UPDATE_FW_OPTIONS()                                                    \
  OPT_FILENAME('f', "file", &update_fw_params.filepath, "rom-file",            \
               "filepath to read ROM for firmware update"),                    \
      OPT_UINTEGER('s', "slot", &update_fw_params.slot,                        \
                   "slot to use for firmware loading"),                        \
      OPT_BOOLEAN('b', "background", &update_fw_params.hbo,                    \
                  "runs as hidden background option"),                         \
      OPT_BOOLEAN('m', "mock", &update_fw_params.mock,                         \
                  "For testing purposes. Mock transfer with only 1 continue "  \
                  "then abort")

#define FW_IMG_OPTIONS()                                                       \
  OPT_BOOLEAN('z', "osimage", &fw_img_params.is_os,                            \
              "select OS(a.k.a boot1) image")

static const struct option cmd_update_fw_options[] = {
    UPDATE_FW_OPTIONS(),
    FW_IMG_OPTIONS(),
    OPT_END(),
};

static int action_cmd_update_fw(struct cxlmi_endpoint *ep) {
  return cxl_cmd_update_device_fw(ep, fw_img_params.is_os, &update_fw_params);
}

int cmd_update_fw(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_update_fw,
                      cmd_update_fw_options, STR_CXL_CMDS_HELP(STR_UPDATE_FW));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* GET_TIMESTAMP */
static const struct option cmd_get_timestamp_options[] = {
    OPT_END(),
};

static int action_cmd_get_timestamp(struct cxlmi_endpoint *ep) {
  return cxl_cmd_get_timestamp(ep);
}

int cmd_get_timestamp(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_get_timestamp,
                      cmd_get_timestamp_options,
                      STR_CXL_CMDS_HELP(STR_GET_TIMESTAMP));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* SET_TIMESTAMP */
static struct _ts_params {
  uint64_t timestamp;
} ts_params;

#define SET_TIMESTAMP_OPTIONS()                                                \
  OPT_U64('t', "timestamp", &ts_params.timestamp,                              \
          "Set the timestamp on the device")

static const struct option cmd_set_timestamp_options[] = {
    SET_TIMESTAMP_OPTIONS(),
    OPT_END(),
};

static int action_cmd_set_timestamp(struct cxlmi_endpoint *ep) {
  return cxl_cmd_set_timestamp(ep, ts_params.timestamp);
}

int cmd_set_timestamp(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_set_timestamp,
                      cmd_set_timestamp_options,
                      STR_CXL_CMDS_HELP(STR_SET_TIMESTAMP));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* GET_EVENT_RECORDS */
static struct _get_event_records_params {
  int event_log_type; /* 00 - information, 01 - warning, 02 - failure, 03 -
                         fatal */
} get_event_records_params;

#define GET_EVENT_RECORDS_OPTIONS()                                            \
  OPT_INTEGER('t', "log_type", &get_event_records_params.event_log_type,       \
              "Event log type (00 - information (default), 01 - warning, 02 "  \
              "- failure, 03 - fatal)")

static const struct option cmd_get_event_records_options[] = {
    GET_EVENT_RECORDS_OPTIONS(),
    OPT_END(),
};

static int action_cmd_get_event_records(struct cxlmi_endpoint *ep) {
  return cxl_cmd_get_event_records(ep, get_event_records_params.event_log_type);
}

int cmd_get_event_records(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_get_event_records,
                      cmd_get_event_records_options,
                      STR_CXL_CMDS_HELP(STR_GET_EVENT_RECORDS));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* CLEAR_EVENT_RECORDS */
static struct _clear_event_records_params {
  int event_log_type;    /* 00 - information, 01 - warning, 02 - failure, 03 -
                            fatal */
  int clear_event_flags; /* bit 0 - when set, clears all events */
  unsigned int event_record_handle; /* only one is supported */
} clear_event_records_params;

#define CLEAR_EVENT_RECORDS_OPTIONS()                                          \
  OPT_INTEGER('t', "log_type", &clear_event_records_params.event_log_type,     \
              "Event log type (00 - information (default), 01 - warning, 02 "  \
              "- failure, 03 - fatal)"),                                       \
      OPT_INTEGER('f', "event_flag",                                           \
                  &clear_event_records_params.clear_event_flags,               \
                  "Clear Event Flags: 1 - clear all events, 0 (default) - "    \
                  "clear specific event record"),                              \
      OPT_UINTEGER('i', "event_record_handle",                                 \
                   &clear_event_records_params.event_record_handle,            \
                   "Clear Specific Event specific by Event Record Handle")

static const struct option cmd_clear_event_records_options[] = {
    CLEAR_EVENT_RECORDS_OPTIONS(),
    OPT_END(),
};

static int action_cmd_clear_event_records(struct cxlmi_endpoint *ep) {
  return cxl_cmd_clear_event_records(
      ep, clear_event_records_params.event_log_type,
      clear_event_records_params.clear_event_flags,
      clear_event_records_params.event_record_handle);
}

int cmd_clear_event_records(int argc, const char **argv,
                            struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_clear_event_records,
                      cmd_clear_event_records_options,
                      STR_CXL_CMDS_HELP(STR_CLEAR_EVENT_RECORDS));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* GET_EVENT_INTERRUPT_POLICY */
static const struct option cmd_get_event_interrupt_policy_options[] = {
    OPT_END(),
};

static int action_cmd_get_event_interrupt_policy(struct cxlmi_endpoint *ep) {
  return cxl_cmd_get_event_interrupt_policy(ep);
}

int cmd_get_event_interrupt_policy(int argc, const char **argv,
                                   struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_get_event_interrupt_policy,
                      cmd_get_event_interrupt_policy_options,
                      STR_CXL_CMDS_HELP(STR_GET_EVENT_INTERRUPT_POLICY));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* SET_EVENT_INTERRUPT_POLICY */
static struct _interrupt_policy_params {
  uint32_t policy;
} interrupt_policy_params;

#define SET_INTERRUPT_POLICY_OPTIONS()                                         \
  OPT_UINTEGER('i', "int_policy", &interrupt_policy_params.policy,             \
               "Set event interrupt policy. Fields: Informational Event Log "  \
               "Interrupt Settings (1B), Warning Event Log Interrupt "         \
               "Settings (1B), Failure Event Log Interrupt Settings (1B), "    \
               "Fatal Event Log Interrupt Settings (1B)")

static const struct option cmd_set_event_interrupt_policy_options[] = {
    SET_INTERRUPT_POLICY_OPTIONS(),
    OPT_END(),
};

static int action_cmd_set_event_interrupt_policy(struct cxlmi_endpoint *ep) {
  return cxl_cmd_set_event_interrupt_policy(ep, interrupt_policy_params.policy);
}

int cmd_set_event_interrupt_policy(int argc, const char **argv,
                                   struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_set_event_interrupt_policy,
                      cmd_set_event_interrupt_policy_options,
                      STR_CXL_CMDS_HELP(STR_SET_EVENT_INTERRUPT_POLICY));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DIMM_SPD_READ */
static struct _dimm_spd_read_params {
  u32 spd_id;
  u32 offset;
  u32 num_bytes;
} dimm_spd_read_params;

#define DIMM_SPD_READ_OPTIONS()                                                \
  OPT_UINTEGER('s', "spd_id", &dimm_spd_read_params.spd_id, "SPD ID"),         \
      OPT_UINTEGER('o', "offset", &dimm_spd_read_params.offset, "Offset"),     \
      OPT_UINTEGER('n', "num_bytes", &dimm_spd_read_params.num_bytes,          \
                   "Num bytes")

static const struct option cmd_dimm_spd_read_options[] = {
    DIMM_SPD_READ_OPTIONS(),
    OPT_END(),
};

static int action_cmd_dimm_spd_read(struct cxlmi_endpoint *ep) {
  return cxl_cmd_dimm_spd_read(ep, dimm_spd_read_params.spd_id,
                               dimm_spd_read_params.offset,
                               dimm_spd_read_params.num_bytes);
}

int cmd_dimm_spd_read(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_dimm_spd_read,
                      cmd_dimm_spd_read_options,
                      STR_CXL_CMDS_HELP(STR_DIMM_SPD_READ));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DIMM_SLOT_INFO */
static const struct option cmd_dimm_slot_info_options[] = {
    OPT_END(),
};

static int action_cmd_dimm_slot_info(struct cxlmi_endpoint *ep) {
  return cxl_cmd_dimm_slot_info(ep);
}

int cmd_dimm_slot_info(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_dimm_slot_info,
                      cmd_dimm_slot_info_options,
                      STR_CXL_CMDS_HELP(STR_DIMM_SLOT_INFO));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* READ_DDR_TEMP */
static const struct option cmd_read_ddr_temp_options[] = {
    OPT_END(),
};

static int action_cmd_read_ddr_temp(struct cxlmi_endpoint *ep) {
  return cxl_cmd_read_ddr_temp(ep);
}

int cmd_read_ddr_temp(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_read_ddr_temp,
                      cmd_read_ddr_temp_options,
                      STR_CXL_CMDS_HELP(STR_READ_DDR_TEMP));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* HEALTH_COUNTERS_CLEAR */
static struct _health_counters_clear_params {
  u32 bitmask;
} health_counters_clear_params;

#define HEALTH_COUNTERS_CLEAR_OPTIONS()                                        \
  OPT_UINTEGER('b', "bitmask", &health_counters_clear_params.bitmask,          \
               "health counters bitmask")

static const struct option cmd_health_counters_clear_options[] = {
    HEALTH_COUNTERS_CLEAR_OPTIONS(),
    OPT_END(),
};

static int action_cmd_health_counters_clear(struct cxlmi_endpoint *ep) {
  return cxl_cmd_health_counters_clear(ep,
                                       health_counters_clear_params.bitmask);
}

int cmd_health_counters_clear(int argc, const char **argv,
                              struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_health_counters_clear,
                      cmd_health_counters_clear_options,
                      STR_CXL_CMDS_HELP(STR_HEALTH_COUNTERS_CLEAR));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* HEALTH_COUNTERS_GET */
static const struct option cmd_health_counters_get_options[] = {
    OPT_END(),
};

static int action_cmd_health_counters_get(struct cxlmi_endpoint *ep) {
  return cxl_cmd_health_counters_get(ep);
}

int cmd_health_counters_get(int argc, const char **argv,
                            struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_health_counters_get,
                      cmd_health_counters_get_options,
                      STR_CXL_CMDS_HELP(STR_HEALTH_COUNTERS_GET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* PMIC_VTMON_INFO */
static const struct option cmd_pmic_vtmon_info_options[] = {
    OPT_END(),
};

static int action_cmd_pmic_vtmon_info(struct cxlmi_endpoint *ep) {
  return cxl_cmd_pmic_vtmon_info(ep);
}

int cmd_pmic_vtmon_info(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_pmic_vtmon_info,
                      cmd_pmic_vtmon_info_options,
                      STR_CXL_CMDS_HELP(STR_PMIC_VTMON_INFO));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* READ_LTSSM_STATUS */
static const struct option cmd_read_ltssm_states_options[] = {
    OPT_END(),
};

static int action_cmd_read_ltssm_states(struct cxlmi_endpoint *ep) {
  return cxl_cmd_read_ltssm_states(ep);
}

int cmd_read_ltssm_states(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_read_ltssm_states,
                      cmd_read_ltssm_states_options,
                      STR_CXL_CMDS_HELP(STR_READ_LTSSM_STATUS));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* PCIE_EYE_RUN */
static struct _pcie_eye_run_params {
  u32 lane;
  u32 sw_scan;
  u32 ber;
} pcie_eye_run_params;

#define PCIE_EYE_RUN_OPTIONS()                                                 \
  OPT_UINTEGER('l', "lane", &pcie_eye_run_params.lane, "LANE ID"),             \
      OPT_UINTEGER('s', "sw_scan", &pcie_eye_run_params.sw_scan, "SW SCAN"),   \
      OPT_UINTEGER('b', "ber", &pcie_eye_run_params.ber, "BER")

static const struct option cmd_pcie_eye_run_options[] = {
    PCIE_EYE_RUN_OPTIONS(),
    OPT_END(),
};

static int action_cmd_pcie_eye_run(struct cxlmi_endpoint *ep) {
  return cxl_cmd_pcie_eye_run(ep, pcie_eye_run_params.lane,
                              pcie_eye_run_params.sw_scan,
                              pcie_eye_run_params.ber);
}

int cmd_pcie_eye_run(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc =
      cmd_action(argc, argv, ctx, action_cmd_pcie_eye_run,
                 cmd_pcie_eye_run_options, STR_CXL_CMDS_HELP(STR_PCI_EYE_RUN));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* PCIE_EYE_STATUS */
static const struct option cmd_pcie_eye_status_options[] = {
    OPT_END(),
};

static int action_cmd_pcie_eye_status(struct cxlmi_endpoint *ep) {
  return cxl_cmd_pcie_eye_status(ep);
}

int cmd_pcie_eye_status(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_pcie_eye_status,
                      cmd_pcie_eye_status_options,
                      STR_CXL_CMDS_HELP(STR_PCIE_EYE_STATUS));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* PCIE_EYE_GET */
static struct _pcie_eye_get_params {
  u32 sw_scan;
  u32 ber;
} pcie_eye_get_params;

#define PCIE_EYE_GET_OPTIONS()                                                 \
  OPT_UINTEGER('s', "sw_scan", &pcie_eye_get_params.sw_scan, "SW SCAN"),       \
      OPT_UINTEGER('b', "ber", &pcie_eye_get_params.ber, "BER")

static const struct option cmd_pcie_eye_get_options[] = {
    PCIE_EYE_GET_OPTIONS(),
    OPT_END(),
};

static int action_cmd_pcie_eye_get(struct cxlmi_endpoint *ep) {
  return cxl_cmd_pcie_eye_get(ep, pcie_eye_get_params.sw_scan,
                              pcie_eye_get_params.ber);
}

int cmd_pcie_eye_get(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc =
      cmd_action(argc, argv, ctx, action_cmd_pcie_eye_get,
                 cmd_pcie_eye_get_options, STR_CXL_CMDS_HELP(STR_PCIE_EYE_GET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* GET_CXL_LINK_STATUS */
static const struct option cmd_get_cxl_link_status_options[] = {
    OPT_END(),
};

static int action_cmd_get_cxl_link_status(struct cxlmi_endpoint *ep) {
  return cxl_cmd_get_cxl_link_status(ep);
}

int cmd_get_cxl_link_status(int argc, const char **argv,
                            struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_get_cxl_link_status,
                      cmd_get_cxl_link_status_options,
                      STR_CXL_CMDS_HELP(STR_GET_CXL_LINK_STATUS));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* GET_DEVICE_INFO */
static const struct option cmd_get_device_info_options[] = {
    OPT_END(),
};

static int action_cmd_get_device_info(struct cxlmi_endpoint *ep) {
  return cxl_cmd_get_device_info(ep);
}

int cmd_get_device_info(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_get_device_info,
                      cmd_get_device_info_options,
                      STR_CXL_CMDS_HELP(STR_GET_DEVICE_INFO));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* GET_DDR_BW */
static struct _get_ddr_bw_params {
  u32 timeout;
  u32 iterations;
} get_ddr_bw_params;

#define GET_DDR_BW_OPTIONS()                                                   \
  OPT_UINTEGER('t', "temeout", &get_ddr_bw_params.timeout, "Timeout"),         \
      OPT_UINTEGER('i', "iterations", &get_ddr_bw_params.iterations,           \
                   "No Iterations")

static const struct option cmd_get_ddr_bw_options[] = {
    GET_DDR_BW_OPTIONS(),
    OPT_END(),
};

static int action_cmd_get_ddr_bw(struct cxlmi_endpoint *ep) {
  return cxl_cmd_get_ddr_bw(ep, get_ddr_bw_params.timeout,
                            get_ddr_bw_params.iterations);
}

int cmd_get_ddr_bw(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc =
      cmd_action(argc, argv, ctx, action_cmd_get_ddr_bw, cmd_get_ddr_bw_options,
                 STR_CXL_CMDS_HELP(STR_GET_DDR_BW));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_MARGIN_RUN */
static struct _ddr_margin_run_params {
  u32 slice_num;
  u32 rd_wr_margin;
  u32 ddr_id;
} ddr_margin_run_params;

#define DDR_MARGIN_RUN_OPTIONS()                                               \
  OPT_UINTEGER('s', "slice_num", &ddr_margin_run_params.slice_num,             \
               "SLICE NUMBER"),                                                \
      OPT_UINTEGER('m', "rd_wr_margin", &ddr_margin_run_params.rd_wr_margin,   \
                   "RD/WR MARGIN"),                                            \
      OPT_UINTEGER('i', "ddr_id", &ddr_margin_run_params.ddr_id, "DDR ID")

static const struct option cmd_ddr_margin_run_options[] = {
    DDR_MARGIN_RUN_OPTIONS(),
    OPT_END(),
};

static int action_cmd_ddr_margin_run(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_margin_run(ep, ddr_margin_run_params.slice_num,
                                ddr_margin_run_params.rd_wr_margin,
                                ddr_margin_run_params.ddr_id);
}

int cmd_ddr_margin_run(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_margin_run,
                      cmd_ddr_margin_run_options,
                      STR_CXL_CMDS_HELP(STR_DDR_MARGIN_RUN));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_MARGIN_STATUS */
static const struct option cmd_ddr_margin_status_options[] = {
    OPT_END(),
};

static int action_cmd_ddr_margin_status(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_margin_status(ep);
}

int cmd_ddr_margin_status(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_margin_status,
                      cmd_ddr_margin_status_options,
                      STR_CXL_CMDS_HELP(STR_DDR_MARGIN_STATUS));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_MARGIN_GET */
static const struct option cmd_ddr_margin_get_options[] = {
    OPT_END(),
};

static int action_cmd_ddr_margin_get(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_margin_get(ep);
}

int cmd_ddr_margin_get(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_margin_get,
                      cmd_ddr_margin_get_options,
                      STR_CXL_CMDS_HELP(STR_DDR_MARGIN_GET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* REBOOT_MODE_SET */
static struct _reboot_mode_set_params {
  u32 reboot_mode;
} reboot_mode_set_params;

#define REBOOT_MODE_SET_OPTIONS()                                              \
  OPT_UINTEGER('m', "reboot_mode", &reboot_mode_set_params.reboot_mode,        \
               "0:CXL-IO-MEM or 0xCE:CXL-IO")

static const struct option cmd_reboot_mode_set_options[] = {
    REBOOT_MODE_SET_OPTIONS(),
    OPT_END(),
};

static int action_cmd_reboot_mode_set(struct cxlmi_endpoint *ep) {
  return cxl_cmd_reboot_mode_set(ep, reboot_mode_set_params.reboot_mode);
}

int cmd_reboot_mode_set(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_reboot_mode_set,
                      cmd_reboot_mode_set_options,
                      STR_CXL_CMDS_HELP(STR_REBOOT_MODE_SET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* CURR_CXL_BOOT_MODE_GET */
static const struct option cmd_curr_cxl_boot_mode_get_options[] = {
    OPT_END(),
};

static int action_cmd_curr_cxl_boot_mode_get(struct cxlmi_endpoint *ep) {
  return cxl_cmd_curr_cxl_boot_mode_get(ep);
}

int cmd_curr_cxl_boot_mode_get(int argc, const char **argv,
                               struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_curr_cxl_boot_mode_get,
                      cmd_curr_cxl_boot_mode_get_options,
                      STR_CXL_CMDS_HELP(STR_CURR_CXL_BOOT_MODE_GET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* GET_DDR_ECC_ERR_INFO  */
static const struct option cmd_get_ddr_ecc_err_info_options[] = {
    OPT_END(),
};

static int action_cmd_get_ddr_ecc_err_info(struct cxlmi_endpoint *ep) {
  return cxl_cmd_get_ddr_ecc_err_info(ep);
}

int cmd_get_ddr_ecc_err_info(int argc, const char **argv,
                             struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_get_ddr_ecc_err_info,
                      cmd_get_ddr_ecc_err_info_options,
                      STR_CXL_CMDS_HELP(STR_GET_DDR_ECC_ERR_INFO));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* I2C_READ */
static struct _i2c_read_params {
  u32 slave_addr;
  u32 reg_addr;
  u32 num_bytes;
} i2c_read_params;

#define I2C_READ_OPTIONS()                                                     \
  OPT_UINTEGER('s', "slave_addr", &i2c_read_params.slave_addr, "Slave addr"),  \
      OPT_UINTEGER('r', "reg_addr", &i2c_read_params.reg_addr, "Reg addr"),    \
      OPT_UINTEGER('n', "num_bytes", &i2c_read_params.num_bytes,               \
                   "Number of bytes")

static const struct option cmd_i2c_read_options[] = {
    I2C_READ_OPTIONS(),
    OPT_END(),
};

static int action_cmd_i2c_read(struct cxlmi_endpoint *ep) {
  return cxl_cmd_i2c_read(ep, i2c_read_params.slave_addr,
                          i2c_read_params.reg_addr, i2c_read_params.num_bytes);
}

int cmd_i2c_read(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_i2c_read,
                      cmd_i2c_read_options, STR_CXL_CMDS_HELP(STR_I2C_READ));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* I2C_WRITE */
static struct _i2c_write_params {
  u32 slave_addr;
  u32 reg_addr;
  u32 data;
} i2c_write_params;

#define I2C_WRITE_OPTIONS()                                                    \
  OPT_UINTEGER('s', "slave_addr", &i2c_write_params.slave_addr, "Slave addr"), \
      OPT_UINTEGER('r', "reg_addr", &i2c_write_params.reg_addr, "Reg addr"),   \
      OPT_UINTEGER('d', "data", &i2c_write_params.data, "Data")

static const struct option cmd_i2c_write_options[] = {
    I2C_WRITE_OPTIONS(),
    OPT_END(),
};

static int action_cmd_i2c_write(struct cxlmi_endpoint *ep) {
  return cxl_cmd_i2c_write(ep, i2c_write_params.slave_addr,
                           i2c_write_params.reg_addr, i2c_write_params.data);
}

int cmd_i2c_write(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_i2c_write,
                      cmd_i2c_write_options, STR_CXL_CMDS_HELP(STR_I2C_WRITE));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* GET_DDR_LATENCY */
static struct _get_ddr_latency_params {
  u32 measure_time;
} get_ddr_latency_params;

#define GET_DDR_LATENCY_OPTIONS()                                              \
  OPT_UINTEGER('t', "measure time", &get_ddr_latency_params.measure_time,      \
               "Measure Time in msec")

static const struct option cmd_get_ddr_latency_options[] = {
    GET_DDR_LATENCY_OPTIONS(),
    OPT_END(),
};

static int action_cmd_get_ddr_latency(struct cxlmi_endpoint *ep) {
  return cxl_cmd_get_ddr_latency(ep, get_ddr_latency_params.measure_time);
}

int cmd_get_ddr_latency(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_get_ddr_latency,
                      cmd_get_ddr_latency_options,
                      STR_CXL_CMDS_HELP(STR_GET_DDR_LATENCY));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* GET_MEMBRIDGE_ERRORS */
static const struct option cmd_get_membridge_errors_options[] = {
    OPT_END(),
};

static int action_cmd_get_membridge_errors(struct cxlmi_endpoint *ep) {
  return cxl_cmd_get_membridge_errors(ep);
}

int cmd_get_membridge_errors(int argc, const char **argv,
                             struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_get_membridge_errors,
                      cmd_get_membridge_errors_options,
                      STR_CXL_CMDS_HELP(STR_GET_MEMBRIDGE_ERRORS));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* HPA_TO_DPA */
u64 hpa_address;
#define HPA_OPTIONS() OPT_U64('h', "hpa", &hpa_address, "host physical address")

static const struct option cmd_hpa_to_dpa_options[] = {
    HPA_OPTIONS(),
    OPT_END(),
};

static int action_cmd_hpa_to_dpa(struct cxlmi_endpoint *ep) {
  return cxl_cmd_hpa_to_dpa(ep, hpa_address);
}

int cmd_hpa_to_dpa(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc =
      cmd_action(argc, argv, ctx, action_cmd_hpa_to_dpa, cmd_hpa_to_dpa_options,
                 STR_CXL_CMDS_HELP(STR_HPA_TO_DPA));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* START_DDR_ECC_SCRUB */
static const struct option cmd_start_ddr_ecc_scrub_options[] = {
    OPT_END(),
};

static int action_cmd_start_ddr_ecc_scrub(struct cxlmi_endpoint *ep) {
  return cxl_cmd_start_ddr_ecc_scrub(ep);
}

int cmd_start_ddr_ecc_scrub(int argc, const char **argv,
                            struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_start_ddr_ecc_scrub,
                      cmd_start_ddr_ecc_scrub_options,
                      STR_CXL_CMDS_HELP(STR_START_DDR_ECC_SCRUB));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_ECC_SCRUB_STATUS */
static const struct option cmd_ddr_ecc_scrub_status_options[] = {
    OPT_END(),
};

static int action_cmd_ddr_ecc_scrub_status(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_ecc_scrub_status(ep);
}

int cmd_ddr_ecc_scrub_status(int argc, const char **argv,
                             struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_ecc_scrub_status,
                      cmd_ddr_ecc_scrub_status_options,
                      STR_CXL_CMDS_HELP(STR_DDR_ECC_SCRUB_STATUS));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_INIT_STATUS */
static const struct option cmd_ddr_init_status_options[] = {
    OPT_END(),
};

static int action_cmd_ddr_init_status(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_init_status(ep);
}

int cmd_ddr_init_status(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_init_status,
                      cmd_ddr_init_status_options,
                      STR_CXL_CMDS_HELP(STR_DDR_INIT_STATUS));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* GET_MEMBRIDGE_STATS */
static const struct option cmd_get_membridge_stats_options[] = {
    OPT_END(),
};

static int action_cmd_get_membridge_stats(struct cxlmi_endpoint *ep) {
  return cxl_cmd_get_membridge_stats(ep);
}

int cmd_get_membridge_stats(int argc, const char **argv,
                            struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_get_membridge_stats,
                      cmd_get_membridge_stats_options,
                      STR_CXL_CMDS_HELP(STR_GET_MEMBRIDGE_STATS));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_ERR_INJ_EN */
static struct _ddr_err_inj_en_params {
  u32 ddr_id;
  u32 err_type;
  u64 ecc_fwc_mask;
} ddr_err_inj_en_params;

#define DDR_ERR_INJ_EN_OPTIONS()                                               \
  OPT_UINTEGER('d', "ddr_id", &ddr_err_inj_en_params.ddr_id,                   \
               "ddr id <0-DDR_CTRL0,1-DDR_CTRL1>"),                            \
      OPT_UINTEGER(                                                            \
          't', "err_type", &ddr_err_inj_en_params.err_type,                    \
          "error type\n\t\t\t0: AXI bus parity READ ADDR\n\t\t\t1: AXI bus "   \
          "parity WRITE ADDR\n\t\t\t2: AXI bus parity WRITE DATA\n\t\t\t3: "   \
          "CA bus parity\n\t\t\t4: ECC correctable\n\t\t\t5: ECC "             \
          "uncorrectable\n\t\t\t6: ECC SCRUB"),                                \
      OPT_U64(                                                                 \
          'm', "ecc_fwc_mask", &ddr_err_inj_en_params.ecc_fwc_mask,            \
          "ecc fwc mask <35bit value, upto two bit set for correctable ecc "   \
          "error\n\t\t\tAtleast 4bits for uncorrectable ecc errors>\n")

static const struct option cmd_ddr_err_inj_en_options[] = {
    DDR_ERR_INJ_EN_OPTIONS(),
    OPT_END(),
};

static int action_cmd_ddr_err_inj_en(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_err_inj_en(ep, ddr_err_inj_en_params.ddr_id,
                                ddr_err_inj_en_params.err_type,
                                ddr_err_inj_en_params.ecc_fwc_mask);
}

int cmd_ddr_err_inj_en(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_err_inj_en,
                      cmd_ddr_err_inj_en_options,
                      STR_CXL_CMDS_HELP(STR_DDR_ERR_INJ_EN));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* TRIGGER_COREDUMP*/
static const struct option cmd_trigger_coredump_options[] = {
    OPT_END(),
};

static int action_cmd_trigger_coredump(struct cxlmi_endpoint *ep) {
  return cxl_cmd_trigger_coredump(ep);
}

int cmd_trigger_coredump(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_trigger_coredump,
                      cmd_trigger_coredump_options,
                      STR_CXL_CMDS_HELP(STR_TRIGGER_COREDUMP));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_STATS_RUN */
static struct _ddr_stats_run_params {
  u32 ddr_id;
  u32 monitor_time;
  u32 loop_count;
} ddr_stats_run_params;

#define DDR_STATS_RUN_OPTIONS()                                                \
  OPT_UINTEGER('i', "ddr_id", &ddr_stats_run_params.ddr_id, "DDR ID"),         \
      OPT_UINTEGER('m', "monitor_time", &ddr_stats_run_params.monitor_time,    \
                   "MOINTOR TIME MSEC"),                                       \
      OPT_UINTEGER('n', "loop_count", &ddr_stats_run_params.loop_count,        \
                   "NUM ITERATION")

static const struct option cmd_ddr_stats_run_options[] = {
    DDR_STATS_RUN_OPTIONS(),
    OPT_END(),
};

static int action_cmd_ddr_stats_run(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_stats_run(ep, ddr_stats_run_params.ddr_id,
                               ddr_stats_run_params.monitor_time,
                               ddr_stats_run_params.loop_count);
}

int cmd_ddr_stats_run(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_stats_run,
                      cmd_ddr_stats_run_options,
                      STR_CXL_CMDS_HELP(STR_DDR_STATS_RUN));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_STATS_GET */
static const struct option cmd_ddr_stats_get_options[] = {
    OPT_END(),
};

static int action_cmd_ddr_stats_get(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_stats_get(ep);
}

int cmd_ddr_stats_get(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_stats_get,
                      cmd_ddr_stats_get_options,
                      STR_CXL_CMDS_HELP(STR_DDR_STATS_GET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_PARAM_SET */
static struct _ddr_set_params {
  u32 ddr_interleave_sz;
  u32 ddr_interleave_ctrl_choice;
} ddr_set_params;

#define DDR_PARAM_SET_OPTIONS()                                                \
  OPT_UINTEGER(                                                                \
      'm', "ddr_interleave_sz", &ddr_set_params.ddr_interleave_sz,             \
      "Intereleave SZ is: 2 pow m. Input the value of m as the Size"),         \
      OPT_UINTEGER('n', "ddr_interleave_ctrl_choice",                          \
                   &ddr_set_params.ddr_interleave_ctrl_choice,                 \
                   "CTRL Choice: 1=DDR0 2=DDR1 3= DDR0 and DDR1")

static const struct option cmd_ddr_param_set_options[] = {
    DDR_PARAM_SET_OPTIONS(),
    OPT_END(),
};

static int action_cmd_ddr_param_set(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_param_set(ep, ddr_set_params.ddr_interleave_sz,
                               ddr_set_params.ddr_interleave_ctrl_choice);
}

int cmd_ddr_param_set(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_param_set,
                      cmd_ddr_param_set_options,
                      STR_CXL_CMDS_HELP(STR_DDR_PARAM_SET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_PARAM_GET */
static const struct option cmd_ddr_param_get_options[] = {
    OPT_END(),
};

static int action_cmd_ddr_param_get(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_param_get(ep);
}

int cmd_ddr_param_get(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_param_get,
                      cmd_ddr_param_get_options,
                      STR_CXL_CMDS_HELP(STR_DDR_PARAM_GET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DIMM_LEVEL_TRAINING_STATUS*/
static const struct option cmd_dimm_level_training_status_options[] = {
    OPT_END(),
};

static int action_cmd_dimm_level_training_status(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_dimm_level_training_status(ep);
}

int cmd_ddr_dimm_level_training_status(int argc, const char **argv,
                                       struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_dimm_level_training_status,
                      cmd_dimm_level_training_status_options,
                      STR_CXL_CMDS_HELP(STR_DDR_DIMM_LEVEL_TRAINING_STATUS));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* VIRAL_INJ_EN */
static struct _oem_err_inj_viral_params {
  u32 viral_type;
  bool verbose;
} oem_err_inj_viral_params;

#define OEM_ERR_INJ_VIRAL_OPTIONS()                                            \
  OPT_UINTEGER('l', "viral_type", &oem_err_inj_viral_params.viral_type,        \
               "viral_type")

static const struct option cmd_viral_inj_en_options[] = {
    OEM_ERR_INJ_VIRAL_OPTIONS(),
    OPT_END(),
};

static int action_cmd_viral_inj_en(struct cxlmi_endpoint *ep) {
  return cxl_cmd_viral_inj_en(ep, oem_err_inj_viral_params.viral_type);
}

int cmd_viral_inj_en(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_viral_inj_en,
                      cmd_viral_inj_en_options,
                      STR_CXL_CMDS_HELP(STR_OEM_ERR_INJ_VIRAL));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* MEM_LL_INJ_EN */
static struct _err_inj_ll_poison_params {
  u32 en_dis;
  u32 ll_err_type;
} err_inj_ll_poison_params;

#define ERR_INJ_LL_POISON_OPTIONS()                                            \
  OPT_UINTEGER('e', "en_dis", &err_inj_ll_poison_params.en_dis,                \
               "enable_disable 0=dis,1=en"),                                   \
      OPT_UINTEGER('l', "ll_err_type", &err_inj_ll_poison_params.ll_err_type,  \
                   "link level err type 0=mem-poison")

static const struct option cmd_mem_ll_err_inj_en_options[] = {
    ERR_INJ_LL_POISON_OPTIONS(),
    OPT_END(),
};

static int action_cmd_mem_ll_err_inj_en(struct cxlmi_endpoint *ep) {
  return cxl_cmd_mem_ll_err_inj_en(ep, err_inj_ll_poison_params.en_dis,
                                   err_inj_ll_poison_params.ll_err_type);
}

int cmd_mem_ll_err_inj_en(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_mem_ll_err_inj_en,
                      cmd_mem_ll_err_inj_en_options,
                      STR_CXL_CMDS_HELP(STR_ERR_INJ_LL_POISON));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* PCI_ERR_INJ_EN*/
static struct _pci_err_in_params {
  u32 en_dis;
  u32 err_type;
  u32 err_subtype;
  u32 count;
  u32 opt_param1;
  u32 opt_param2;
} pci_err_inj_params;

#define PCI_ERR_INJ_OPTIONS()                                                  \
  OPT_UINTEGER('e', "en_dis", &pci_err_inj_params.en_dis,                      \
               "enable_disable:\n\t0=disable inj\n\t1=enable inj"),            \
      OPT_UINTEGER(                                                            \
          'l', "err_type", &pci_err_inj_params.err_type,                       \
          "err inj type Group:\n\t0:CRC ERR\n\t1:SEQ NUM ERR\n\t2:DLLP "       \
          "ERR\n\t3:SYMBOL ERR\n\t4:FC CREDIT ERR\n\t5:Special TLP ERR\n"),    \
      OPT_UINTEGER(                                                            \
          's', "err_subtype", &pci_err_inj_params.err_subtype,                 \
          "err inj sub-type:\n\tGroup-0:<CRC Error>\n\t\t0 = "                 \
          "TX_TLP_LCRC_ERR\n\t\t1 = TX_16B_CRC_ERR_ACK_NAK_DLLP\n\t\t2 = "     \
          "TX_16B_CRC_ERR_UPD_FC\n\t\t3 = TX_TLP_ECRC_ERR\n\t\t4 = "           \
          "TX_FCRC_ERR_TLP\n\t\t5 = TX_PARITY_TSOS_ERR\n\t\t6 = "              \
          "TX_PARITY_SKPOS_ERR\n\t\t8 = RX_LCRC_ERR\n\t\t11= "                 \
          "RX_ECRC_ERR\n\n\tGroup-1:<SEQ NUM Error>\n\t\t0 = "                 \
          "TLP_ERR_SEQNUM\n\t\t1 = "                                           \
          "ACK_NAK_DLLP_ERR_SEQNUM\n\n\tGroup-2:<DLLP Error>\n\t\t0 = "        \
          "ACK_NACK_DLLP\n\t\t1 = UPD_FC_DLLP\n\t\t2 = "                       \
          "NAK_DLLP\n\n\tGroup-3:<Symbol Error>\n\t\t0 = "                     \
          "RSVD_OR_INVRT_SYNC_HDR\n\t\t1 = COM_PAD_TS1\n\t\t2 = "              \
          "COM_PAD_TS2\n\t\t3 = COM_FTS\n\t\t4 = COM_IDL\n\t\t5 = "            \
          "END_EDB\n\t\t6 = STP_SDP\n\t\t7 = COM_SKP\n\n\tGroup-4:<FC Credit " \
          "Error>\n\t\t0 = POSTED_TLP_HDR\n\t\t1 = NON_POSTED_TLP_HDR\n\t\t2 " \
          "= CMPL_TLP_HDR\n\t\t4 = POSTED_TLP_DATA\n\t\t5 = "                  \
          "NON_POSTED_TLP_DATA\n\n\tGroup-5:<Special TLP Error>\n\t\t0 = "     \
          "DUPLICATE_DLLP\n\t\t1 = NULLIFIED_TLP\n"),                          \
      OPT_UINTEGER('c', "count", &pci_err_inj_params.count,                    \
                   "err inj count:\n\t1-255: count of err to inject\n\t0: "    \
                   "continuous inj until disable\n"),                          \
      OPT_UINTEGER(                                                            \
          'x', "opt1", &pci_err_inj_params.opt_param1,                         \
          "opt1: Optional Extra args1\n\tFor Group-1:Bad Sequence Number(2s "  \
          "compliment in hex): Min:0x1001, Max:0xfff\n\tFor Group-4:Bad "      \
          "update-FC credit val(2s compliment in hex): Min:0x1001, "           \
          "Max:0xfff\n\tFor other Groups: Pass value '0'\n"),                  \
      OPT_UINTEGER(                                                            \
          'y', "opt2", &pci_err_inj_params.opt_param2,                         \
          "opt2: Optional Extra args2\n\tGroup-4:<FC Credit Error>:Target "    \
          "VC_NUMBER: Min:0, Max:7\n\tFor other Groups:Pass value '0'\n")

static const struct option cmd_pci_err_inj_en_options[] = {
    PCI_ERR_INJ_OPTIONS(),
    OPT_END(),
};

static int action_cmd_pci_err_inj_en(struct cxlmi_endpoint *ep) {
  return cxl_cmd_pci_err_inj_en(
      ep, pci_err_inj_params.en_dis, pci_err_inj_params.err_type,
      pci_err_inj_params.err_subtype, pci_err_inj_params.count,
      pci_err_inj_params.opt_param1, pci_err_inj_params.opt_param2);
}

int cmd_pci_err_inj_en(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_pci_err_inj_en,
                      cmd_pci_err_inj_en_options,
                      STR_CXL_CMDS_HELP(STR_PCI_ERR_INJ));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* CORE_VOLT_SET */
static struct _ddr_core_volt_set_params {
  u32 val1;
  u32 val2;
  u32 val3;
} ddr_core_volt_set_params;

#define CORE_VOLT_SET_OPTIONS()                                                \
  OPT_UINTEGER('i', "core_volt_val1", &ddr_core_volt_set_params.val1,          \
               "CORE Voltage val1.val2 val3"),                                 \
      OPT_UINTEGER('m', "core_volt_val2", &ddr_core_volt_set_params.val2,      \
                   "CORE Voltage val1.val2 val3"),                             \
      OPT_UINTEGER('n', "core_volt_val3", &ddr_core_volt_set_params.val3,      \
                   "CORE Voltage val1.val2 val3")

static const struct option cmd_core_volt_set_options[] = {
    CORE_VOLT_SET_OPTIONS(),
    OPT_END(),
};

static int action_cmd_core_volt_set(struct cxlmi_endpoint *ep) {
  float volt = ddr_core_volt_set_params.val1 +
               (ddr_core_volt_set_params.val2 / 10.0) +
               (ddr_core_volt_set_params.val3 / 100.0);
  return cxl_cmd_core_volt_set(ep, volt);
}

int cmd_core_volt_set(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_core_volt_set,
                      cmd_core_volt_set_options,
                      STR_CXL_CMDS_HELP(STR_CORE_VOLT_SET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* CORE_VOLT_GET */
static const struct option cmd_core_volt_get_options[] = {
    OPT_END(),
};

static int action_cmd_core_volt_get(struct cxlmi_endpoint *ep) {
  return cxl_cmd_core_volt_get(ep);
}

int cmd_core_volt_get(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_core_volt_get,
                      cmd_core_volt_get_options,
                      STR_CXL_CMDS_HELP(STR_CORE_VOLT_GET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_CONT_SCRUB_STATUS */
static const struct option cmd_ddr_cont_scrub_status_options[] = {
    OPT_END(),
};

static int action_cmd_ddr_cont_scrub_status(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_cont_scrub_status(ep);
}

int cmd_ddr_cont_scrub_status(int argc, const char **argv,
                              struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_cont_scrub_status,
                      cmd_ddr_cont_scrub_status_options,
                      STR_CXL_CMDS_HELP(STR_DDR_CONT_SCRUB_STATUS));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_CONT_SCRUB_SET */
static struct _ddr_cont_scrub_set_params {
  u32 cont_scrub_status;
} ddr_cont_scrub_set_params;

#define DDR_CONT_SCRUB_SET_OPTIONS()                                           \
  OPT_UINTEGER('i', "cont_scrub_status",                                       \
               &ddr_cont_scrub_set_params.cont_scrub_status,                   \
               "Continuous Scrub ON:1 OFF: 0")

static const struct option cmd_ddr_cont_scrub_set_options[] = {
    DDR_CONT_SCRUB_SET_OPTIONS(),
    OPT_END(),
};

static int action_cmd_ddr_cont_scrub_set(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_cont_scrub_set(
      ep, ddr_cont_scrub_set_params.cont_scrub_status);
}

int cmd_ddr_cont_scrub_set(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_cont_scrub_set,
                      cmd_ddr_cont_scrub_set_options,
                      STR_CXL_CMDS_HELP(STR_DDR_CONT_SCRUB_SET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_PAGE_SELECT_SET */
static struct _page_policy_selection {
  int page_policy_reg_val;
} _page_policy_select;

#define DDR_PAGE_SELECT_SET_OPTIONS()                                          \
  OPT_INTEGER('p', "page_policy_reg_val",                                      \
              &_page_policy_select.page_policy_reg_val,                        \
              "Value for page policy selection")

static const struct option cmd_ddr_page_select_set_options[] = {
    DDR_PAGE_SELECT_SET_OPTIONS(),
    OPT_END(),
};

static int action_cmd_ddr_page_select_set(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_page_select_set(ep,
                                     _page_policy_select.page_policy_reg_val);
}

int cmd_ddr_page_select_set(int argc, const char **argv,
                            struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_page_select_set,
                      cmd_ddr_page_select_set_options,
                      STR_CXL_CMDS_HELP(STR_DDR_PAGE_SELECT_SET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_PAGE_SELECT_GET */
static const struct option cmd_ddr_page_select_get_options[] = {
    OPT_END(),
};

static int action_cmd_ddr_page_select_get(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_page_select_get(ep);
}

int cmd_ddr_page_select_get(int argc, const char **argv,
                            struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_page_select_get,
                      cmd_ddr_page_select_get_options,
                      STR_CXL_CMDS_HELP(STR_DDR_PAGE_SELECT_GET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_HPPR_SET */
static struct _ddr_hppr_set_params {
  u32 enable;
} ddr_hppr_set_params;

#define DDR_HPPR_SET_OPTIONS()                                                 \
  OPT_UINTEGER('e', "ddr_hppr_en", &ddr_hppr_set_params.enable,                \
               "HPPR Enable/Disable value(1/0)")

static const struct option cmd_ddr_hppr_set_options[] = {
    DDR_HPPR_SET_OPTIONS(),
    OPT_END(),
};

static int action_cmd_ddr_hppr_set(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_hppr_set(ep, ddr_hppr_set_params.enable);
}

int cmd_ddr_hppr_set(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc =
      cmd_action(argc, argv, ctx, action_cmd_ddr_hppr_set,
                 cmd_ddr_hppr_set_options, STR_CXL_CMDS_HELP(STR_DDR_HPPR_SET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_HPPR_GET */
static const struct option cmd_ddr_hppr_get_options[] = {
    OPT_END(),
};

static int action_cmd_ddr_hppr_get(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_hppr_get(ep);
}

int cmd_ddr_hppr_get(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc =
      cmd_action(argc, argv, ctx, action_cmd_ddr_hppr_get,
                 cmd_ddr_hppr_get_options, STR_CXL_CMDS_HELP(STR_DDR_HPPR_GET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_HPPR_ADDR_INFO_SET */
struct _ddr_hppr_addr_info_set_params {
  u32 ddr_id;
  u32 chip_select; /* 2bit chip select info of faulty row*/
  u32 bank;        /* 2bits bank info*/
  u32 bank_group;  /* 2bit bank group info */
  u32 row;         /* faulty row address */
} ddr_hppr_addr_info_set_params;

struct _ddr_addr_info {
  u32 ddr_id;
  u32 chip_select; /* 2bit chip select info of faulty row*/
  u32 bank;        /* 2bits bank info*/
  u32 bank_group;  /* 2bit bank group info */
  u32 row;         /* faulty row address */
  u32 channel;     /* channel 0/1 of DDR controller */
  u32 ppr_state;
} ddr_addr_info;

struct _ddr_hppr_addr_info_get_params {
  struct _ddr_addr_info addr_info[2][8];
} ddr_hppr_addr_info_get_params;

#define DDR_HPPR_ADDR_INFO_SET_OPTIONS()                                       \
  OPT_UINTEGER('d', "ddr_id", &ddr_hppr_addr_info_set_params.ddr_id,           \
               "HPPR addr info: DDR controller ID value"),                     \
      OPT_UINTEGER('c', "chip_select",                                         \
                   &ddr_hppr_addr_info_set_params.chip_select,                 \
                   "HPPR addr info: chip select value"),                       \
      OPT_UINTEGER('g', "bank_group",                                          \
                   &ddr_hppr_addr_info_set_params.bank_group,                  \
                   "HPPR addr info: bank group value"),                        \
      OPT_UINTEGER('b', "bank", &ddr_hppr_addr_info_set_params.bank,           \
                   "HPPR addr info: bank value"),                              \
      OPT_UINTEGER('r', "row", &ddr_hppr_addr_info_set_params.row,             \
                   "HPPR addr info: row value")

static const struct option cmd_ddr_hppr_addr_info_set_options[] = {
    DDR_HPPR_ADDR_INFO_SET_OPTIONS(),
    OPT_END(),
};

static int action_cmd_ddr_hppr_addr_info_set(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_hppr_addr_info_set(
      ep, ddr_hppr_addr_info_set_params.ddr_id,
      ddr_hppr_addr_info_set_params.chip_select,
      ddr_hppr_addr_info_set_params.bank_group,
      ddr_hppr_addr_info_set_params.bank, ddr_hppr_addr_info_set_params.row);
}

int cmd_ddr_hppr_addr_info_set(int argc, const char **argv,
                               struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_hppr_addr_info_set,
                      cmd_ddr_hppr_addr_info_set_options,
                      STR_CXL_CMDS_HELP(STR_DDR_HPPR_ADDR_INFO_SET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_HPPR_ADDR_INFO_GET */
static const struct option cmd_ddr_hppr_addr_info_get_options[] = {
    OPT_END(),
};

static int action_cmd_ddr_hppr_addr_info_get(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_hppr_addr_info_get(ep);
}

int cmd_ddr_hppr_addr_info_get(int argc, const char **argv,
                               struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_hppr_addr_info_get,
                      cmd_ddr_hppr_addr_info_get_options,
                      STR_CXL_CMDS_HELP(STR_DDR_HPPR_ADDR_INFO_GET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_HPPR_ADDR_INFO_CLEAR */
struct _ddr_hppr_addr_info_clear_params {
  u32 ddr_id;
  u32 channel_id;
} ddr_hppr_addr_info_clear_params;

#define DDR_HPPR_ADDR_INFO_CLEAR_OPTIONS()                                     \
  OPT_UINTEGER('d', "ddr_id", &ddr_hppr_addr_info_clear_params.ddr_id,         \
               "HPPR addr info: DDR controller ID value"),                     \
      OPT_UINTEGER('c', "channel_id",                                          \
                   &ddr_hppr_addr_info_clear_params.channel_id,                \
                   "HPPR addr info: channel ID value")

static const struct option cmd_ddr_hppr_addr_info_clear_options[] = {
    DDR_HPPR_ADDR_INFO_CLEAR_OPTIONS(),
    OPT_END(),
};

static int action_cmd_ddr_hppr_addr_info_clear(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_hppr_addr_info_clear(
      ep, ddr_hppr_addr_info_clear_params.ddr_id,
      ddr_hppr_addr_info_clear_params.channel_id);
}

int cmd_ddr_hppr_addr_info_clear(int argc, const char **argv,
                                 struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_hppr_addr_info_clear,
                      cmd_ddr_hppr_addr_info_clear_options,
                      STR_CXL_CMDS_HELP(STR_DDR_HPPR_ADDR_INFO_CLEAR));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_PPR_GET_STATUS */
static const struct option cmd_ddr_ppr_get_status_options[] = {
    OPT_END(),
};

static int action_cmd_ddr_ppr_get_status(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_ppr_get_status(ep);
}

int cmd_ddr_ppr_get_status(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_ppr_get_status,
                      cmd_ddr_ppr_get_status_options,
                      STR_CXL_CMDS_HELP(STR_DDR_PPR_GET_STATUS));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_REFRESH_MODE_SET */
static struct ddr_refresh_mode {
  u32 ddr_refresh_val;
} refresh_select;

#define DDR_REFRESH_MODE_SELECT_SET_OPTIONS()                                  \
  OPT_UINTEGER('r', "ddr_refresh_val", &refresh_select.ddr_refresh_val,        \
               "Value for refresh mode selection")

static const struct option cmd_ddr_refresh_mode_set_options[] = {
    DDR_REFRESH_MODE_SELECT_SET_OPTIONS(),
    OPT_END(),
};

static int action_cmd_ddr_refresh_mode_set(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_refresh_mode_set(ep, refresh_select.ddr_refresh_val);
}

int cmd_ddr_refresh_mode_set(int argc, const char **argv,
                             struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_refresh_mode_set,
                      cmd_ddr_refresh_mode_set_options,
                      STR_CXL_CMDS_HELP(STR_DDR_REFRESH_MODE_SET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_REFRESH_MODE_GET */
static const struct option cmd_ddr_refresh_mode_get_options[] = {
    OPT_END(),
};

static int action_cmd_ddr_refresh_mode_get(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_refresh_mode_get(ep);
}

int cmd_ddr_refresh_mode_get(int argc, const char **argv,
                             struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_refresh_mode_get,
                      cmd_ddr_refresh_mode_get_options,
                      STR_CXL_CMDS_HELP(STR_DDR_REFRESH_MODE_GET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* CXL_ERR_CNTR_GET */
static const struct option cmd_cxl_err_cntr_get_options[] = {
    OPT_END(),
};

static int action_cmd_cxl_err_cntr_get(struct cxlmi_endpoint *ep) {
  return cxl_cmd_cxl_err_cntr_get(ep);
}

int cmd_cxl_err_cntr_get(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_cxl_err_cntr_get,
                      cmd_cxl_err_cntr_get_options,
                      STR_CXL_CMDS_HELP(STR_CXL_ERR_CNTR_GET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_FREQ_GET */
static const struct option cmd_ddr_freq_get_options[] = {
    OPT_END(),
};

static int action_cmd_ddr_freq_get(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_freq_get(ep);
}

int cmd_ddr_freq_get(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  int rc =
      cmd_action(argc, argv, ctx, action_cmd_ddr_freq_get,
                 cmd_ddr_freq_get_options, STR_CXL_CMDS_HELP(STR_DDR_FREQ_GET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}

/* DDR_INIT_ERR_INFO_GET */
static const struct option cmd_ddr_init_err_info_get_options[] = {
    OPT_END(),
};

static int action_cmd_ddr_init_err_info_get(struct cxlmi_endpoint *ep) {
  return cxl_cmd_ddr_init_err_info_get(ep);
}

int cmd_ddr_init_err_info_get(int argc, const char **argv,
                              struct cxlmi_ctx *ctx) {
  int rc = cmd_action(argc, argv, ctx, action_cmd_ddr_init_err_info_get,
                      cmd_ddr_init_err_info_get_options,
                      STR_CXL_CMDS_HELP(STR_DDR_INIT_ERR_INFO_GET));

  return rc >= 0 ? 0 : EXIT_FAILURE;
}
