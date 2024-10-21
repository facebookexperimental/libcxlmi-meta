// SPDX-License-Identifier: LGPL-2.1-or-later

/* std includes */
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
  for (i = 0; i < argc; i++) {
    if (sscanf(argv[i], "mem%lu", &id) != 1 && strcmp(argv[i], "all") != 0)
      continue;

    cxlmi_for_each_endpoint(ctx, ep) {
      if (!cxlmi_ep_filter(ep, argv[i]))
        continue;
      rc = action(ep);
      if (rc == 0)
        count++;
      else if (rc && !err)
        err = rc;
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
