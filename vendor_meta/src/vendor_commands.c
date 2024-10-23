// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

/* std includes */
#include <stdlib.h>

/* libcxlmi includes */
#include <ccan/endian/endian.h>
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

CXLMI_EXPORT int cxlmi_cmd_vendor_transfer_fw(struct cxlmi_endpoint *ep,
                                              struct cxlmi_tunnel_info *ti,
                                              struct cxlmi_cmd_transfer_fw *in,
                                              uint32_t opcode) {
  struct cxlmi_cmd_transfer_fw *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;

  struct cxlmi_cci_msg rsp;
  ssize_t req_sz, data_sz = struct_size(in, data, 0);
  int rc = -1;

  req_sz = sizeof(*req_pl) + data_sz + sizeof(*req);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OEM_MGMT, opcode);
  req_pl = (struct cxlmi_cmd_transfer_fw *)req->payload;

  req_pl->action = in->action;
  req_pl->slot = in->slot;
  req_pl->offset = cpu_to_le32(in->offset);
  memcpy(req_pl->data, in->data, data_sz);

  rc = send_cmd_cci(ep, ti, req, req_sz, &rsp, sizeof(rsp), sizeof(rsp));

  return rc;
}

CXLMI_EXPORT int
cxlmi_cmd_get_hbo_status(struct cxlmi_endpoint *ep,
                         struct cxlmi_tunnel_info *ti,
                         struct cxlmi_cmd_hbo_status_fields *ret) {
  struct cxlmi_cmd_hbo_status_out *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;
  uint8_t opcode_shift = 0;
  uint8_t percent_shift = 16;
  uint8_t running_shift = 23;
  uint8_t retcode_shift = 32;
  uint8_t extended_shift = 48;
  uint64_t opcode_mask = (1 << percent_shift) - (1 << opcode_shift);    // 0-15
  uint64_t percent_mask = (1 << running_shift) - (1 << percent_shift);  // 16-22
  uint64_t running_mask = (1 << running_shift);                         // 23
  uint64_t retcode_mask = (1 << extended_shift) - (1 << retcode_shift); // 32-47
  uint64_t extended_mask =
      0xffffffffffffffff - (1 << extended_shift) + 1; // 48-63

  CXLMI_BUILD_BUG_ON(sizeof(*ret) != sizeof(struct cxlmi_cmd_hbo_status_out));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OEM_MGMT, OEM_HBO_STATUS);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_hbo_status_out *)rsp->payload;
  ret->opcode = (rsp_pl->bo_status & opcode_mask) >> opcode_shift;
  ret->percent_complete = (rsp_pl->bo_status & percent_mask) >> percent_shift;
  ret->is_running = (rsp_pl->bo_status & running_mask) >> running_shift;
  ret->return_code = (rsp_pl->bo_status & retcode_mask) >> retcode_shift;
  ret->extended_status = (rsp_pl->bo_status & extended_mask) >> extended_shift;
  if (ret->is_running) {
    rc = 1;
  }

  return rc;
}
