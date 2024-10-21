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

/* shell command handlers  */
int cmd_print_help(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_identify(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_supported_logs(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_log(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_alert_config(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_set_alert_config(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_health_info(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_fw_info(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_timestamp(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_set_timestamp(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_event_records(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_clear_event_records(int argc, const char **argv, struct cxlmi_ctx *ctx);
int cmd_get_event_interrupt_policy(int argc, const char **argv,
                                   struct cxlmi_ctx *ctx);
int cmd_set_event_interrupt_policy(int argc, const char **argv,
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
int cxl_cmd_get_timestamp(struct cxlmi_endpoint *ep);
int cxl_cmd_set_timestamp(struct cxlmi_endpoint *ep, uint64_t timestamp);
int cxl_cmd_get_event_records(struct cxlmi_endpoint *ep, uint8_t type);
int cxl_cmd_clear_event_records(struct cxlmi_endpoint *ep, uint8_t type,
                                uint8_t flags, uint16_t handles);
int cxl_cmd_get_event_interrupt_policy(struct cxlmi_endpoint *ep);
int cxl_cmd_set_event_interrupt_policy(struct cxlmi_endpoint *ep,
                                       uint32_t interrupt_policy);

/* helper functions */
const char *get_devname(struct cxlmi_endpoint *ep);

#ifdef __cplusplus
}
#endif
#endif /* __CXL_CMD_H__ */
