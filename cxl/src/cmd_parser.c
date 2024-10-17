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
