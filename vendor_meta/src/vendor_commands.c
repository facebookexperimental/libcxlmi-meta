// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

/* std includes */
#include <stdlib.h>

/* libcxlmi includes */
#include <cxlmi/log.h>
#include <cxlmi/private.h>
#include <libcxlmi.h>

/* vendor includes */
#include <vendor_commands.h>
#include <vendor_types.h>

CXLMI_EXPORT int cxlmi_cmd_get_os_fw_info(struct cxlmi_endpoint *ep,
                                          struct cxlmi_tunnel_info *ti,
                                          struct cxlmi_cmd_get_fw_info *ret) {
  struct cxlmi_cmd_get_fw_info *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) != 0x50);

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OEM_MGMT, GET_OS_INFO);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_get_fw_info *)rsp->payload;
  ret->slots_supported = rsp_pl->slots_supported;
  ret->slot_info = rsp_pl->slot_info;
  ret->caps = rsp_pl->caps;
  pstrcpy(ret->fw_rev1, sizeof(rsp_pl->fw_rev1), rsp_pl->fw_rev1);
  pstrcpy(ret->fw_rev2, sizeof(rsp_pl->fw_rev2), rsp_pl->fw_rev2);
  pstrcpy(ret->fw_rev3, sizeof(rsp_pl->fw_rev3), rsp_pl->fw_rev3);
  pstrcpy(ret->fw_rev4, sizeof(rsp_pl->fw_rev4), rsp_pl->fw_rev4);

  return rc;
}
