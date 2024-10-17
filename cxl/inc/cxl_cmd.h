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

int cmd_print_help(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_supported_logs(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_log(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_alert_config(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_health_info(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_fw_info(int argc, const char **argv, struct cxlmi_ctx *ctx);

int cxl_cmd_get_supported_logs(struct cxlmi_endpoint *ep);
int cxl_cmd_get_log(struct cxlmi_endpoint *ep, const char *log_uuid,
                    uint32_t log_size);
int cxl_cmd_get_alert_config(struct cxlmi_endpoint *ep);
int cxl_cmd_get_health_info(struct cxlmi_endpoint *ep);
int cxl_cmd_get_dev_fw_info(struct cxlmi_endpoint *ep, bool is_os);
int cxl_cmd_get_fw_info(struct cxlmi_endpoint *ep);
int cxl_cmd_get_os_fw_info(struct cxlmi_endpoint *ep);

/* helper functions */
const char *get_devname(struct cxlmi_endpoint *ep);

#ifdef __cplusplus
}
#endif
#endif /* __CXL_CMD_H__ */
