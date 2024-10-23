// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#ifndef __VENDOR_COMMANDS_H__
#define __VENDOR_COMMANDS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <libcxlmi.h>
#include <vendor_types.h>

int cxlmi_cmd_get_os_fw_info(struct cxlmi_endpoint *ep,
                             struct cxlmi_tunnel_info *ti,
                             struct cxlmi_cmd_get_fw_info *out);

/* Used for OS FW update/transfer and Pioneer vendor opcode for hbo-transfer-fw
 */
int cxlmi_cmd_vendor_transfer_fw(struct cxlmi_endpoint *ep,
                                 struct cxlmi_tunnel_info *ti,
                                 struct cxlmi_cmd_transfer_fw *in,
                                 uint32_t opcode);

int cxlmi_cmd_get_hbo_status(struct cxlmi_endpoint *ep,
                             struct cxlmi_tunnel_info *ti,
                             struct cxlmi_cmd_hbo_status_fields *ret);

#ifdef __cplusplus
}
#endif

#endif /* __VENDOR_COMMANDS_H__ */
