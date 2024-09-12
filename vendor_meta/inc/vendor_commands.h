// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#ifndef __VENDOR_COMMANDS_H__
#define __VENDOR_COMMANDS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <libcxlmi.h>

int cxlmi_cmd_get_os_fw_info(struct cxlmi_endpoint *ep,
                             struct cxlmi_tunnel_info *ti,
                             struct cxlmi_cmd_get_fw_info *out);

#ifdef __cplusplus
}
#endif

#endif /* __VENDOR_COMMANDS_H__ */
