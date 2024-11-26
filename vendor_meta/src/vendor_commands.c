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

CXLMI_EXPORT int
cxlmi_cmd_dimm_spd_read(struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
                        struct cxlmi_cmd_dimm_spd_read_req *in,
                        struct cxlmi_cmd_dimm_spd_read_rsp *ret) {
  struct cxlmi_cmd_dimm_spd_read_rsp *rsp_pl;
  struct cxlmi_cmd_dimm_spd_read_req *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz, req_sz;
  int rc;

  req_sz = sizeof(*req) + sizeof(*req_pl);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_DDR_DIMM_MGMT,
                  DIMM_SPD_READ);
  req_pl = (struct cxlmi_cmd_dimm_spd_read_req *)req->payload;
  req_pl->spd_id = in->spd_id;
  req_pl->offset = in->offset;
  req_pl->num_bytes = in->num_bytes;

  rsp_sz = sizeof(*rsp) + (sizeof(struct cxlmi_cmd_dimm_spd_read_rsp));
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, req, req_sz, rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_dimm_spd_read_rsp *)rsp->payload;

  memcpy(ret, rsp_pl, sizeof(struct cxlmi_cmd_dimm_spd_read_rsp));

  return rc;
}

CXLMI_EXPORT int
cxlmi_cmd_dimm_slot_info(struct cxlmi_endpoint *ep,
                         struct cxlmi_tunnel_info *ti,
                         struct cxlmi_cmd_dimm_slot_info *ret) {
  struct cxlmi_cmd_dimm_slot_info *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) != sizeof(struct cxlmi_cmd_dimm_slot_info));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_DDR_DIMM_MGMT, DIMM_SLOT_INFO);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_dimm_slot_info *)rsp->payload;
  memcpy(ret, rsp_pl, sizeof(struct cxlmi_cmd_dimm_slot_info));

  return rc;
}

CXLMI_EXPORT int cxlmi_cmd_read_ddr_temp(struct cxlmi_endpoint *ep,
                                         struct cxlmi_tunnel_info *ti,
                                         struct cxlmi_cmd_read_ddr_temp *ret) {
  struct cxlmi_cmd_read_ddr_temp *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) != sizeof(struct cxlmi_cmd_read_ddr_temp));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_DDR_DIMM_MGMT, READ_DDR_TEMP);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_read_ddr_temp *)rsp->payload;
  memcpy(ret, rsp_pl, sizeof(struct cxlmi_cmd_read_ddr_temp));

  return rc;
}

CXLMI_EXPORT int
cxlmi_cmd_health_counters_clear(struct cxlmi_endpoint *ep,
                                struct cxlmi_tunnel_info *ti,
                                struct cxlmi_cmd_health_counters_clear *in) {
  struct cxlmi_cmd_health_counters_clear *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  struct cxlmi_cci_msg rsp;
  ssize_t req_sz;
  int rc;

  req_sz = sizeof(*req_pl) + sizeof(*req);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_HEALTH_MGMT,
                  HEALTH_COUNTERS_CLEAR);
  req_pl = (struct cxlmi_cmd_health_counters_clear *)req->payload;

  req_pl->bitmask = in->bitmask;

  rc = send_cmd_cci(ep, ti, req, req_sz, &rsp, sizeof(rsp), sizeof(rsp));

  return rc;
}

CXLMI_EXPORT int
cxlmi_cmd_health_counters_get(struct cxlmi_endpoint *ep,
                              struct cxlmi_tunnel_info *ti,
                              struct cxlmi_cmd_health_counters_get *ret) {
  struct cxlmi_cmd_health_counters_get *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) !=
                     sizeof(struct cxlmi_cmd_health_counters_get));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_HEALTH_MGMT, HEALTH_COUNTERS_GET);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_health_counters_get *)rsp->payload;
  memcpy(ret, rsp_pl, sizeof(struct cxlmi_cmd_health_counters_get));

  return rc;
}

int cxlmi_cmd_pmic_vtmon_info(struct cxlmi_endpoint *ep,
                              struct cxlmi_tunnel_info *ti,
                              struct cxlmi_cmd_pmic_vtmon_info *ret) {
  struct cxlmi_cmd_pmic_vtmon_info *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) != sizeof(struct cxlmi_cmd_pmic_vtmon_info));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, PMIC_VTMON_INFO);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_pmic_vtmon_info *)rsp->payload;
  memcpy(ret, rsp_pl, sizeof(struct cxlmi_cmd_pmic_vtmon_info));

  return rc;
}

int cxlmi_cmd_read_ltssm_states(struct cxlmi_endpoint *ep,
                                struct cxlmi_tunnel_info *ti,
                                struct cxlmi_cmd_read_ltssm_states *ret) {
  struct cxlmi_cmd_read_ltssm_states *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) !=
                     sizeof(struct cxlmi_cmd_read_ltssm_states));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, READ_LTSSM_STATES);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_read_ltssm_states *)rsp->payload;
  memcpy(ret, rsp_pl, sizeof(struct cxlmi_cmd_read_ltssm_states));

  return rc;
}

int cxlmi_cmd_pcie_eye_run(struct cxlmi_endpoint *ep,
                           struct cxlmi_tunnel_info *ti,
                           struct cxlmi_cmd_pcie_eye_run_req *in,
                           struct cxlmi_cmd_pcie_eye_run_rsp *ret) {
  struct cxlmi_cmd_pcie_eye_run_rsp *rsp_pl;
  struct cxlmi_cmd_pcie_eye_run_req *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz, req_sz;
  int rc;

  req_sz = sizeof(*req) + sizeof(*req_pl);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS, PCIE_EYE_RUN);
  req_pl = (struct cxlmi_cmd_pcie_eye_run_req *)req->payload;
  req_pl->lane = in->lane;
  req_pl->sw_scan = in->sw_scan;
  req_pl->ber = in->ber;

  rsp_sz = sizeof(*rsp) + (sizeof(struct cxlmi_cmd_pcie_eye_run_rsp));
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, req, req_sz, rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_pcie_eye_run_rsp *)rsp->payload;

  memcpy(ret, rsp_pl, sizeof(struct cxlmi_cmd_pcie_eye_run_rsp));

  return rc;
}

int cxlmi_cmd_pcie_eye_status(struct cxlmi_endpoint *ep,
                              struct cxlmi_tunnel_info *ti,
                              struct cxlmi_cmd_pcie_eye_status *ret) {
  struct cxlmi_cmd_pcie_eye_status *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) != sizeof(struct cxlmi_cmd_pcie_eye_status));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, PCIE_EYE_STATUS);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_pcie_eye_status *)rsp->payload;
  ret->pcie_eye_status = rsp_pl->pcie_eye_status;
  ret->error = rsp_pl->error;

  return rc;
}

int cxlmi_cmd_pcie_eye_get_sw(struct cxlmi_endpoint *ep,
                              struct cxlmi_tunnel_info *ti,
                              struct cxlmi_cmd_pcie_eye_get_sw_req *in,
                              struct cxlmi_cmd_pcie_eye_get_sw_rsp *ret) {
  struct cxlmi_cmd_pcie_eye_get_sw_rsp *rsp_pl;
  struct cxlmi_cmd_pcie_eye_get_sw_req *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz, req_sz;
  int rc;

  req_sz = sizeof(*req) + sizeof(*req_pl);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS, PCIE_EYE_GET_SW);
  req_pl = (struct cxlmi_cmd_pcie_eye_get_sw_req *)req->payload;

  req_pl->offset = in->offset;

  rsp_sz = sizeof(*rsp) + (sizeof(struct cxlmi_cmd_pcie_eye_get_sw_rsp));
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, req, req_sz, rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_pcie_eye_get_sw_rsp *)rsp->payload;

  memcpy(ret, rsp_pl, sizeof(struct cxlmi_cmd_pcie_eye_get_sw_rsp));

  return rc;
}

int cxlmi_cmd_pcie_eye_get_hw(struct cxlmi_endpoint *ep,
                              struct cxlmi_tunnel_info *ti,
                              struct cxlmi_cmd_pcie_eye_get_hw *ret) {
  struct cxlmi_cmd_pcie_eye_get_hw *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) != sizeof(struct cxlmi_cmd_pcie_eye_get_hw));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, PCIE_EYE_GET_HW);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_pcie_eye_get_hw *)rsp->payload;
  memcpy(ret, rsp_pl, sizeof(struct cxlmi_cmd_pcie_eye_get_hw));

  return rc;
}

int cxlmi_cmd_pcie_eye_get_sw_ber(struct cxlmi_endpoint *ep,
                                  struct cxlmi_tunnel_info *ti,
                                  struct cxlmi_cmd_pcie_eye_get_sw_ber *ret) {
  struct cxlmi_cmd_pcie_eye_get_sw_ber *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) !=
                     sizeof(struct cxlmi_cmd_pcie_eye_get_sw_ber));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, PCIE_EYE_GET_SW_BER);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_pcie_eye_get_sw_ber *)rsp->payload;
  ret->horiz_margin = rsp_pl->horiz_margin;
  ret->vert_margin = rsp_pl->vert_margin;

  return rc;
}

int cxlmi_cmd_get_cxl_link_status(struct cxlmi_endpoint *ep,
                                  struct cxlmi_tunnel_info *ti,
                                  struct cxlmi_cmd_get_cxl_link_status *ret) {
  struct cxlmi_cmd_get_cxl_link_status *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) !=
                     sizeof(struct cxlmi_cmd_get_cxl_link_status));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, CXL_LINK_STATUS);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_get_cxl_link_status *)rsp->payload;
  ret->cxl_link_status = rsp_pl->cxl_link_status;
  ret->link_width = rsp_pl->link_width;
  ret->link_speed = rsp_pl->link_speed;
  ret->ltssm_val = rsp_pl->ltssm_val;

  return rc;
}

int cxlmi_cmd_get_device_info(struct cxlmi_endpoint *ep,
                              struct cxlmi_tunnel_info *ti,
                              struct cxlmi_cmd_get_device_info *ret) {
  struct cxlmi_cmd_get_device_info *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) != sizeof(struct cxlmi_cmd_get_device_info));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, DEVICE_INFO);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_get_device_info *)rsp->payload;
  ret->device_id = rsp_pl->device_id;
  ret->revision_id = rsp_pl->revision_id;

  return rc;
}

int cxlmi_cmd_get_ddr_bw(struct cxlmi_endpoint *ep,
                         struct cxlmi_tunnel_info *ti,
                         struct cxlmi_cmd_get_ddr_bw_req *in,
                         struct cxlmi_cmd_get_ddr_bw_rsp *ret) {
  struct cxlmi_cmd_get_ddr_bw_rsp *rsp_pl;
  struct cxlmi_cmd_get_ddr_bw_req *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz, req_sz;
  int rc;

  req_sz = sizeof(*req) + sizeof(*req_pl);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS, GET_DDR_BW);
  req_pl = (struct cxlmi_cmd_get_ddr_bw_req *)req->payload;

  req_pl->timeout = in->timeout;
  req_pl->iterations = in->iterations;

  rsp_sz = sizeof(*rsp) + (sizeof(struct cxlmi_cmd_get_ddr_bw_rsp));
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, req, req_sz, rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_get_ddr_bw_rsp *)rsp->payload;
  for (int i = 0; i < DDR_MAX_SUBSYS; i++) {
    ret->peak_bw[i] = rsp_pl->peak_bw[i];
  }

  return rc;
}

int cxlmi_cmd_ddr_margin_run(struct cxlmi_endpoint *ep,
                             struct cxlmi_tunnel_info *ti,
                             struct cxlmi_cmd_ddr_margin_run *in) {
  struct cxlmi_cmd_ddr_margin_run *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  struct cxlmi_cci_msg rsp;
  ssize_t req_sz;
  int rc;

  req_sz = sizeof(*req_pl) + sizeof(*req);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS, DDR_MARGIN_RUN);
  req_pl = (struct cxlmi_cmd_ddr_margin_run *)req->payload;

  req_pl->slice_num = in->slice_num;
  req_pl->rd_wr_margin = in->rd_wr_margin;
  req_pl->ddr_id = in->ddr_id;

  rc = send_cmd_cci(ep, ti, req, req_sz, &rsp, sizeof(rsp), sizeof(rsp));

  return rc;
}

int cxlmi_cmd_ddr_margin_status(struct cxlmi_endpoint *ep,
                                struct cxlmi_tunnel_info *ti,
                                struct cxlmi_cmd_ddr_margin_status *ret) {
  struct cxlmi_cmd_ddr_margin_status *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) !=
                     sizeof(struct cxlmi_cmd_ddr_margin_status));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, DDR_MARGIN_STATUS);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_ddr_margin_status *)rsp->payload;
  ret->run_status = rsp_pl->run_status;

  return rc;
}

int cxlmi_cmd_ddr_margin_get(struct cxlmi_endpoint *ep,
                             struct cxlmi_tunnel_info *ti,
                             struct cxlmi_cmd_ddr_margin_get *ret) {
  struct cxlmi_cmd_ddr_margin_get *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) != sizeof(struct cxlmi_cmd_ddr_margin_get));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, DDR_MARGIN_GET);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_ddr_margin_get *)rsp->payload;
  memcpy(ret, rsp_pl, sizeof(struct cxlmi_cmd_ddr_margin_get));

  return rc;
}

int cxlmi_cmd_reboot_mode_set(struct cxlmi_endpoint *ep,
                              struct cxlmi_tunnel_info *ti,
                              struct cxlmi_cmd_reboot_mode_set *in) {
  struct cxlmi_cmd_reboot_mode_set *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  struct cxlmi_cci_msg rsp;
  ssize_t req_sz;
  int rc;

  req_sz = sizeof(*req_pl) + sizeof(*req);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS, REBOOT_MODE_SET);
  req_pl = (struct cxlmi_cmd_reboot_mode_set *)req->payload;

  req_pl->reboot_mode = in->reboot_mode;

  rc = send_cmd_cci(ep, ti, req, req_sz, &rsp, sizeof(rsp), sizeof(rsp));
  return rc;
}

int cxlmi_cmd_curr_cxl_boot_mode_get(
    struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
    struct cxlmi_cmd_curr_cxl_boot_mode_get *ret) {
  struct cxlmi_cmd_curr_cxl_boot_mode_get *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) !=
                     sizeof(struct cxlmi_cmd_curr_cxl_boot_mode_get));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, CURR_CXL_BOOT_MODE_GET);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_curr_cxl_boot_mode_get *)rsp->payload;
  ret->curr_cxl_boot = rsp_pl->curr_cxl_boot;

  return rc;
}

int cxlmi_cmd_get_ddr_ecc_err_info(struct cxlmi_endpoint *ep,
                                   struct cxlmi_tunnel_info *ti,
                                   struct cxlmi_cmd_get_ddr_ecc_err_info *ret) {
  struct cxlmi_cmd_get_ddr_ecc_err_info *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) !=
                     sizeof(struct cxlmi_cmd_get_ddr_ecc_err_info));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, GET_DDR_ECC_ERR_INFO);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_get_ddr_ecc_err_info *)rsp->payload;
  memcpy(ret, rsp_pl, sizeof(struct cxlmi_cmd_get_ddr_ecc_err_info));

  return rc;
}

int cxlmi_cmd_i2c_read(struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
                       struct cxlmi_cmd_i2c_read_req *in,
                       struct cxlmi_cmd_i2c_read_rsp *ret) {
  struct cxlmi_cmd_i2c_read_rsp *rsp_pl;
  struct cxlmi_cmd_i2c_read_req *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz, req_sz;
  int rc;

  req_sz = sizeof(*req) + sizeof(*req_pl);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS, I2C_READ);
  req_pl = (struct cxlmi_cmd_i2c_read_req *)req->payload;

  req_pl->slave_addr = in->slave_addr;
  req_pl->reg_addr = in->reg_addr;
  req_pl->num_bytes = in->num_bytes;

  rsp_sz = sizeof(*rsp) + (sizeof(struct cxlmi_cmd_i2c_read_rsp));
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, req, req_sz, rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_i2c_read_rsp *)rsp->payload;
  memcpy(ret, rsp_pl, I2C_MAX_SIZE_NUM_BYTES);
  ret->num_bytes = rsp_pl->num_bytes;

  return rc;
}

int cxlmi_cmd_i2c_write(struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
                        struct cxlmi_cmd_i2c_write *in) {
  struct cxlmi_cmd_i2c_write *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  struct cxlmi_cci_msg rsp;
  ssize_t req_sz;
  int rc;

  req_sz = sizeof(*req_pl) + sizeof(*req);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS, I2C_WRITE);
  req_pl = (struct cxlmi_cmd_i2c_write *)req->payload;

  req_pl->slave_addr = in->slave_addr;
  req_pl->reg_addr = in->reg_addr;
  req_pl->data = in->data;

  rc = send_cmd_cci(ep, ti, req, req_sz, &rsp, sizeof(rsp), sizeof(rsp));

  return rc;
}

int cxlmi_cmd_get_ddr_latency(struct cxlmi_endpoint *ep,
                              struct cxlmi_tunnel_info *ti,
                              struct cxlmi_cmd_get_ddr_latency_req *in,
                              struct cxlmi_cmd_get_ddr_latency_rsp *ret) {
  struct cxlmi_cmd_get_ddr_latency_rsp *rsp_pl;
  struct cxlmi_cmd_get_ddr_latency_req *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz, req_sz;
  int rc;

  req_sz = sizeof(*req) + sizeof(*req_pl);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS, GET_DDR_LATENCY);
  req_pl = (struct cxlmi_cmd_get_ddr_latency_req *)req->payload;

  req_pl->measure_time = in->measure_time;

  rsp_sz = sizeof(*rsp) + (sizeof(struct cxlmi_cmd_get_ddr_latency_rsp));
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, req, req_sz, rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_get_ddr_latency_rsp *)rsp->payload;
  memcpy(ret, rsp_pl, sizeof(struct cxlmi_cmd_get_ddr_latency_rsp));

  return rc;
}

int cxlmi_cmd_get_membridge_errors(struct cxlmi_endpoint *ep,
                                   struct cxlmi_tunnel_info *ti,
                                   struct cxlmi_cmd_get_membridge_errors *ret) {
  struct cxlmi_cmd_get_membridge_errors *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) !=
                     sizeof(struct cxlmi_cmd_get_membridge_errors));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, GET_MEMBRIDGE_ERRORS);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_get_membridge_errors *)rsp->payload;
  memcpy(ret, rsp_pl, sizeof(struct cxlmi_cmd_get_membridge_errors));

  return rc;
}

int cxlmi_cmd_hpa_to_dpa(struct cxlmi_endpoint *ep,
                         struct cxlmi_tunnel_info *ti,
                         struct cxlmi_cmd_hpa_to_dpa_req *in,
                         struct cxlmi_cmd_hpa_to_dpa_rsp *ret) {
  struct cxlmi_cmd_hpa_to_dpa_rsp *rsp_pl;
  struct cxlmi_cmd_hpa_to_dpa_req *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz, req_sz;
  int rc;

  req_sz = sizeof(*req) + sizeof(*req_pl);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS, HPA_TO_DPA);
  req_pl = (struct cxlmi_cmd_hpa_to_dpa_req *)req->payload;

  req_pl->hpa_address = in->hpa_address;

  rsp_sz = sizeof(*rsp) + (sizeof(struct cxlmi_cmd_hpa_to_dpa_rsp));
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, req, req_sz, rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_hpa_to_dpa_rsp *)rsp->payload;
  ret->dpa_address = rsp_pl->dpa_address;

  return rc;
}

int cxlmi_cmd_start_ddr_ecc_scrub(struct cxlmi_endpoint *ep,
                                  struct cxlmi_tunnel_info *ti) {
  struct cxlmi_cci_msg req, rsp;

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, START_DDR_ECC_SCRUB);

  return send_cmd_cci(ep, ti, &req, sizeof(req), &rsp, sizeof(rsp),
                      sizeof(rsp));
}

int cxlmi_cmd_ddr_ecc_scrub_status(struct cxlmi_endpoint *ep,
                                   struct cxlmi_tunnel_info *ti,
                                   struct cxlmi_cmd_ddr_ecc_scrub_status *ret) {
  struct cxlmi_cmd_ddr_ecc_scrub_status *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) !=
                     sizeof(struct cxlmi_cmd_ddr_ecc_scrub_status));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, DDR_ECC_SCRUB_STATUS);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_ddr_ecc_scrub_status *)rsp->payload;
  for (int i = 0; i < DDR_MAX_SUBSYS; i++) {
    ret->ecc_scrub_status[i] = rsp_pl->ecc_scrub_status[i];
  }

  return rc;
}
int cxlmi_cmd_ddr_init_status(struct cxlmi_endpoint *ep,
                              struct cxlmi_tunnel_info *ti,
                              struct cxlmi_cmd_ddr_init_status *ret) {
  struct cxlmi_cmd_ddr_init_status *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) != sizeof(struct cxlmi_cmd_ddr_init_status));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, DDR_INIT_STATUS);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_ddr_init_status *)rsp->payload;

  ret->init_status.ddr_init_status = rsp_pl->init_status.ddr_init_status;
  ret->init_status.failed_channel_id = rsp_pl->init_status.failed_channel_id;
  ret->init_status.failed_dimm_silk_screen =
      rsp_pl->init_status.failed_dimm_silk_screen;

  return rc;
}

int cxlmi_cmd_get_membridge_stats(struct cxlmi_endpoint *ep,
                                  struct cxlmi_tunnel_info *ti,
                                  struct cxlmi_cmd_get_membridge_stats *ret) {
  struct cxlmi_cmd_get_membridge_stats *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) !=
                     sizeof(struct cxlmi_cmd_get_membridge_stats));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, GET_MEMBRIDGE_STATS);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_get_membridge_stats *)rsp->payload;
  memcpy(ret, rsp_pl, sizeof(struct cxlmi_cmd_get_membridge_stats));

  return rc;
}

int cxlmi_cmd_ddr_err_inj_en(struct cxlmi_endpoint *ep,
                             struct cxlmi_tunnel_info *ti,
                             struct cxlmi_cmd_ddr_err_inj_en *in) {
  struct cxlmi_cmd_ddr_err_inj_en *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  struct cxlmi_cci_msg rsp;
  ssize_t req_sz;
  int rc;

  req_sz = sizeof(*req_pl) + sizeof(*req);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS, DDR_ERR_INJ_EN);
  req_pl = (struct cxlmi_cmd_ddr_err_inj_en *)req->payload;

  req_pl->ddr_id = in->ddr_id;
  req_pl->err_type = in->err_type;
  req_pl->ecc_fwc_mask = in->ecc_fwc_mask;

  rc = send_cmd_cci(ep, ti, req, req_sz, &rsp, sizeof(rsp), sizeof(rsp));

  return rc;
}

int cxlmi_cmd_trigger_coredump(struct cxlmi_endpoint *ep,
                               struct cxlmi_tunnel_info *ti) {
  struct cxlmi_cci_msg req, rsp;

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, TRIGGER_COREDUMP);

  return send_cmd_cci(ep, ti, &req, sizeof(req), &rsp, sizeof(rsp),
                      sizeof(rsp));
}

int cxlmi_cmd_ddr_stats_run(struct cxlmi_endpoint *ep,
                            struct cxlmi_tunnel_info *ti,
                            struct cxlmi_cmd_ddr_stats_run *in) {
  struct cxlmi_cmd_ddr_stats_run *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  struct cxlmi_cci_msg rsp;
  ssize_t req_sz;
  int rc;

  req_sz = sizeof(*req_pl) + sizeof(*req);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS, DDR_STATS_RUN);
  req_pl = (struct cxlmi_cmd_ddr_stats_run *)req->payload;

  req_pl->ddr_id = in->ddr_id;
  req_pl->monitor_time = in->monitor_time;
  req_pl->loop_count = in->loop_count;

  rc = send_cmd_cci(ep, ti, req, req_sz, &rsp, sizeof(rsp), sizeof(rsp));

  return rc;
}

int cxlmi_cmd_ddr_stats_status(struct cxlmi_endpoint *ep,
                               struct cxlmi_tunnel_info *ti,
                               struct cxlmi_cmd_ddr_stats_status *ret) {
  struct cxlmi_cmd_ddr_stats_status *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) != sizeof(struct cxlmi_cmd_ddr_stats_status));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, DDR_STATS_STATUS);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_ddr_stats_status *)rsp->payload;
  ret->run_status = rsp_pl->run_status;
  ret->loop_count = rsp_pl->loop_count;

  return rc;
}

int cxlmi_cmd_ddr_stats_get(struct cxlmi_endpoint *ep,
                            struct cxlmi_tunnel_info *ti,
                            struct cxlmi_cmd_ddr_stats_get_req *in,
                            cxlmi_cmd_ddr_stats_get_rsp_t *ret) {
  cxlmi_cmd_ddr_stats_get_rsp_t rsp_pl;
  struct cxlmi_cmd_ddr_stats_get_req *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz, req_sz;
  int rc;

  req_sz = sizeof(*req) + sizeof(*req_pl);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS, DDR_STATS_GET);
  req_pl = (struct cxlmi_cmd_ddr_stats_get_req *)req->payload;

  req_pl->offset = in->offset;
  req_pl->transfer_sz = in->transfer_sz;

  /* Allocate memory for requested transfer size */
  rsp_sz = sizeof(*rsp) + in->transfer_sz;
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, req, req_sz, rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (cxlmi_cmd_ddr_stats_get_rsp_t)(rsp->payload);
  memcpy(*ret, rsp_pl, in->transfer_sz);

  return rc;
}

int cxlmi_cmd_ddr_param_set(struct cxlmi_endpoint *ep,
                            struct cxlmi_tunnel_info *ti,
                            struct cxlmi_cmd_ddr_param_set *in) {
  struct cxlmi_cmd_ddr_param_set *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  struct cxlmi_cci_msg rsp;
  ssize_t req_sz;
  int rc;

  req_sz = sizeof(*req_pl) + sizeof(*req);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS, DDR_PARAM_SET);
  req_pl = (struct cxlmi_cmd_ddr_param_set *)req->payload;

  req_pl->ddr_inter.ddr_interleave_sz = in->ddr_inter.ddr_interleave_sz;
  req_pl->ddr_inter.ddr_interleave_ctrl_choice =
      in->ddr_inter.ddr_interleave_ctrl_choice;

  rc = send_cmd_cci(ep, ti, req, req_sz, &rsp, sizeof(rsp), sizeof(rsp));

  return rc;
}

int cxlmi_cmd_ddr_param_get(struct cxlmi_endpoint *ep,
                            struct cxlmi_tunnel_info *ti,
                            struct cxlmi_cmd_ddr_param_get *ret) {
  struct cxlmi_cmd_ddr_param_get *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) != sizeof(struct cxlmi_cmd_ddr_param_get));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, DDR_PARAM_GET);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_ddr_param_get *)rsp->payload;
  ret->ddr_inter.ddr_interleave_sz = rsp_pl->ddr_inter.ddr_interleave_sz;
  ret->ddr_inter.ddr_interleave_ctrl_choice =
      rsp_pl->ddr_inter.ddr_interleave_ctrl_choice;

  return rc;
}

int cxlmi_cmd_dimm_level_training_status(
    struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
    struct cxlmi_cmd_dimm_level_training_status *ret) {
  struct cxlmi_cmd_dimm_level_training_status *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) !=
                     sizeof(struct cxlmi_cmd_dimm_level_training_status));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, DIMM_LEVEL_TRAINING_STATUS);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_dimm_level_training_status *)rsp->payload;
  memcpy(ret, rsp_pl, sizeof(struct cxlmi_cmd_dimm_level_training_status));

  return rc;
}

int cxlmi_cmd_viral_inj_en(struct cxlmi_endpoint *ep,
                           struct cxlmi_tunnel_info *ti,
                           struct cxlmi_cmd_viral_inj_en *in) {
  struct cxlmi_cmd_viral_inj_en *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  struct cxlmi_cci_msg rsp;
  ssize_t req_sz;
  int rc;

  req_sz = sizeof(*req_pl) + sizeof(*req);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS,
                  CXL_VIRAL_INJ_EN);
  req_pl = (struct cxlmi_cmd_viral_inj_en *)req->payload;

  req_pl->viral_type = in->viral_type;
  rc = send_cmd_cci(ep, ti, req, req_sz, &rsp, sizeof(rsp), sizeof(rsp));

  return rc;
}

int cxlmi_cmd_mem_ll_err_inj_en(struct cxlmi_endpoint *ep,
                                struct cxlmi_tunnel_info *ti,
                                struct cxlmi_cmd_mem_ll_err_inj_en *in) {
  struct cxlmi_cmd_mem_ll_err_inj_en *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  struct cxlmi_cci_msg rsp;
  ssize_t req_sz;
  int rc;

  req_sz = sizeof(*req_pl) + sizeof(*req);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS,
                  CXL_MEM_LL_ERR_INJ_EN);
  req_pl = (struct cxlmi_cmd_mem_ll_err_inj_en *)req->payload;

  req_pl->en_dis = in->en_dis;
  req_pl->ll_err_type = in->ll_err_type;

  rc = send_cmd_cci(ep, ti, req, req_sz, &rsp, sizeof(rsp), sizeof(rsp));

  return rc;
}

int cxlmi_cmd_pci_err_inj_en(struct cxlmi_endpoint *ep,
                             struct cxlmi_tunnel_info *ti,
                             struct cxlmi_cmd_pci_err_inj_en *in) {
  struct cxlmi_cmd_pci_err_inj_en *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  struct cxlmi_cci_msg rsp;
  ssize_t req_sz;
  int rc;

  req_sz = sizeof(*req_pl) + sizeof(*req);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS,
                  PCI_RAS_DES_ERR_INJ_EN);
  req_pl = (struct cxlmi_cmd_pci_err_inj_en *)req->payload;

  req_pl->en_dis = in->en_dis;
  req_pl->err_type = in->err_type;
  req_pl->err_subtype = in->err_subtype;
  req_pl->count = in->count;
  req_pl->opt_param1 = in->opt_param1;
  req_pl->opt_param2 = in->opt_param2;

  rc = send_cmd_cci(ep, ti, req, req_sz, &rsp, sizeof(rsp), sizeof(rsp));

  return rc;
}

int cxlmi_cmd_core_volt_set(struct cxlmi_endpoint *ep,
                            struct cxlmi_tunnel_info *ti,
                            struct cxlmi_cmd_core_volt_set *in) {
  struct cxlmi_cmd_core_volt_set *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  struct cxlmi_cci_msg rsp;
  ssize_t req_sz;
  int rc;

  req_sz = sizeof(*req_pl) + sizeof(*req);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS, CORE_VOLT_SET);
  req_pl = (struct cxlmi_cmd_core_volt_set *)req->payload;

  req_pl->core_volt = in->core_volt;

  rc = send_cmd_cci(ep, ti, req, req_sz, &rsp, sizeof(rsp), sizeof(rsp));

  return rc;
}

int cxlmi_cmd_core_volt_get(struct cxlmi_endpoint *ep,
                            struct cxlmi_tunnel_info *ti,
                            struct cxlmi_cmd_core_volt_get *ret) {
  struct cxlmi_cmd_core_volt_get *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) != sizeof(struct cxlmi_cmd_core_volt_get));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, CORE_VOLT_GET);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_core_volt_get *)rsp->payload;
  ret->core_volt = rsp_pl->core_volt;

  return rc;
}

int cxlmi_cmd_ddr_cont_scrub_status(
    struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
    struct cxlmi_cmd_ddr_cont_scrub_status *ret) {
  struct cxlmi_cmd_ddr_cont_scrub_status *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) !=
                     sizeof(struct cxlmi_cmd_ddr_cont_scrub_status));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, DDR_CONT_SCRUB_STATUS);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_ddr_cont_scrub_status *)rsp->payload;
  ret->cont_scrub_status = rsp_pl->cont_scrub_status;

  return rc;
}

int cxlmi_cmd_ddr_cont_scrub_set(struct cxlmi_endpoint *ep,
                                 struct cxlmi_tunnel_info *ti,
                                 struct cxlmi_cmd_ddr_cont_scrub_set *in) {
  struct cxlmi_cmd_ddr_cont_scrub_set *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  struct cxlmi_cci_msg rsp;
  ssize_t req_sz;
  int rc;

  req_sz = sizeof(*req_pl) + sizeof(*req);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS,
                  DDR_CONT_SCRUB_SET);
  req_pl = (struct cxlmi_cmd_ddr_cont_scrub_set *)req->payload;

  req_pl->cont_scrub_status = in->cont_scrub_status;

  rc = send_cmd_cci(ep, ti, req, req_sz, &rsp, sizeof(rsp), sizeof(rsp));

  return rc;
}

int cxlmi_cmd_ddr_page_select_set(struct cxlmi_endpoint *ep,
                                  struct cxlmi_tunnel_info *ti,
                                  struct cxlmi_cmd_ddr_page_select_set *in) {
  struct cxlmi_cmd_ddr_page_select_set *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  struct cxlmi_cci_msg rsp;
  ssize_t req_sz;
  int rc;

  req_sz = sizeof(*req_pl) + sizeof(*req);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS,
                  DDR_PAGE_SELECT_SET);
  req_pl = (struct cxlmi_cmd_ddr_page_select_set *)req->payload;

  req_pl->pp_select.page_policy_reg_val = in->pp_select.page_policy_reg_val;

  rc = send_cmd_cci(ep, ti, req, req_sz, &rsp, sizeof(rsp), sizeof(rsp));

  return rc;
}

int cxlmi_cmd_ddr_page_select_get(struct cxlmi_endpoint *ep,
                                  struct cxlmi_tunnel_info *ti,
                                  struct cxlmi_cmd_ddr_page_select_get *ret) {
  struct cxlmi_cmd_ddr_page_select_get *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) !=
                     sizeof(struct cxlmi_cmd_ddr_page_select_get));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, DDR_PAGE_SELECT_GET);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_ddr_page_select_get *)rsp->payload;
  ret->pp_select.page_policy_reg_val = rsp_pl->pp_select.page_policy_reg_val;

  return rc;
}

int cxlmi_cmd_ddr_hppr_set(struct cxlmi_endpoint *ep,
                           struct cxlmi_tunnel_info *ti,
                           struct cxlmi_cmd_ddr_hppr_set *in) {
  struct cxlmi_cmd_ddr_hppr_set *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  struct cxlmi_cci_msg rsp;
  ssize_t req_sz;
  int rc;

  req_sz = sizeof(*req_pl) + sizeof(*req);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS, DDR_HPPR_SET);
  req_pl = (struct cxlmi_cmd_ddr_hppr_set *)req->payload;

  req_pl->enable = in->enable;

  rc = send_cmd_cci(ep, ti, req, req_sz, &rsp, sizeof(rsp), sizeof(rsp));

  return rc;
}

int cxlmi_cmd_ddr_hppr_get(struct cxlmi_endpoint *ep,
                           struct cxlmi_tunnel_info *ti,
                           struct cxlmi_cmd_ddr_hppr_get *ret) {
  struct cxlmi_cmd_ddr_hppr_get *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) != sizeof(struct cxlmi_cmd_ddr_hppr_get));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, DDR_HPPR_GET);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_ddr_hppr_get *)rsp->payload;
  ret->hppr_enable[0] = rsp_pl->hppr_enable[0];
  ret->hppr_enable[1] = rsp_pl->hppr_enable[1];

  return rc;
}

int cxlmi_cmd_ddr_hppr_addr_info_set(
    struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
    struct cxlmi_cmd_ddr_hppr_addr_info_set *in) {
  struct cxlmi_cmd_ddr_hppr_addr_info_set *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  struct cxlmi_cci_msg rsp;
  ssize_t req_sz;
  int rc;

  req_sz = sizeof(*req_pl) + sizeof(*req);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS,
                  DDR_HPPR_ADDR_INFO_SET);
  req_pl = (struct cxlmi_cmd_ddr_hppr_addr_info_set *)req->payload;

  req_pl->hppr_addr_info.ddr_id = in->hppr_addr_info.ddr_id;
  req_pl->hppr_addr_info.chip_select = in->hppr_addr_info.chip_select;
  req_pl->hppr_addr_info.bank = in->hppr_addr_info.bank;
  req_pl->hppr_addr_info.bank_group = in->hppr_addr_info.bank_group;
  req_pl->hppr_addr_info.row = in->hppr_addr_info.row;

  rc = send_cmd_cci(ep, ti, req, req_sz, &rsp, sizeof(rsp), sizeof(rsp));

  return rc;
}

int cxlmi_cmd_ddr_hppr_addr_info_get(
    struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
    struct cxlmi_cmd_ddr_hppr_addr_info_get *ret) {
  struct cxlmi_cmd_ddr_hppr_addr_info_get *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) !=
                     sizeof(struct cxlmi_cmd_ddr_hppr_addr_info_get));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, DDR_HPPR_ADDR_INFO_GET);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_ddr_hppr_addr_info_get *)rsp->payload;
  memcpy(ret, rsp_pl, sizeof(struct cxlmi_cmd_ddr_hppr_addr_info_get));

  return rc;
}

int cxlmi_cmd_ddr_hppr_addr_info_clear(
    struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
    struct cxlmi_cmd_ddr_hppr_addr_info_clear *in) {
  struct cxlmi_cmd_ddr_hppr_addr_info_clear *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  struct cxlmi_cci_msg rsp;
  ssize_t req_sz;
  int rc;

  req_sz = sizeof(*req_pl) + sizeof(*req);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS,
                  DDR_HPPR_ADDR_INFO_CLEAR);
  req_pl = (struct cxlmi_cmd_ddr_hppr_addr_info_clear *)req->payload;

  req_pl->ddr_id = in->ddr_id;
  req_pl->channel_id = in->channel_id;

  rc = send_cmd_cci(ep, ti, req, req_sz, &rsp, sizeof(rsp), sizeof(rsp));

  return rc;
}

int cxlmi_cmd_ddr_ppr_get_status(struct cxlmi_endpoint *ep,
                                 struct cxlmi_tunnel_info *ti,
                                 struct cxlmi_cmd_ddr_ppr_get_status *ret) {
  struct cxlmi_cmd_ddr_ppr_get_status *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) !=
                     sizeof(struct cxlmi_cmd_ddr_ppr_get_status));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, DDR_PPR_GET_STATUS);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_ddr_ppr_get_status *)rsp->payload;
  ret->status = rsp_pl->status;

  return rc;
}

int cxlmi_cmd_ddr_refresh_mode_set(struct cxlmi_endpoint *ep,
                                   struct cxlmi_tunnel_info *ti,
                                   struct cxlmi_cmd_ddr_refresh_mode_set *in) {
  struct cxlmi_cmd_ddr_refresh_mode_set *req_pl;
  _cleanup_free_ struct cxlmi_cci_msg *req = NULL;
  struct cxlmi_cci_msg rsp;
  ssize_t req_sz;
  int rc;

  req_sz = sizeof(*req_pl) + sizeof(*req);
  req = calloc(1, req_sz);
  if (!req)
    return -1;

  arm_cci_request(ep, req, sizeof(*req_pl), VENDOR_CMD_OTHERS,
                  DDR_REFRESH_MODE_SET);
  req_pl = (struct cxlmi_cmd_ddr_refresh_mode_set *)req->payload;

  req_pl->ddr_refresh_val = in->ddr_refresh_val;

  rc = send_cmd_cci(ep, ti, req, req_sz, &rsp, sizeof(rsp), sizeof(rsp));

  return rc;
}

int cxlmi_cmd_ddr_refresh_mode_get(struct cxlmi_endpoint *ep,
                                   struct cxlmi_tunnel_info *ti,
                                   struct cxlmi_cmd_ddr_refresh_mode_get *ret) {
  struct cxlmi_cmd_ddr_refresh_mode_get *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) !=
                     sizeof(struct cxlmi_cmd_ddr_refresh_mode_get));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, DDR_REFRESH_MODE_GET);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_ddr_refresh_mode_get *)rsp->payload;
  ret->ddr_refresh_val = rsp_pl->ddr_refresh_val;

  return rc;
}

int cxlmi_cmd_cxl_err_cntr_get(struct cxlmi_endpoint *ep,
                               struct cxlmi_tunnel_info *ti,
                               struct cxlmi_cmd_cxl_err_cntr_get *ret) {
  struct cxlmi_cmd_cxl_err_cntr_get *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) != sizeof(struct cxlmi_cmd_cxl_err_cntr_get));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, CXL_ERR_CNTR_GET);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_cxl_err_cntr_get *)rsp->payload;
  memcpy(ret, rsp_pl, sizeof(struct cxlmi_cmd_cxl_err_cntr_get));

  return rc;
}

int cxlmi_cmd_ddr_freq_get(struct cxlmi_endpoint *ep,
                           struct cxlmi_tunnel_info *ti,
                           struct cxlmi_cmd_ddr_freq_get *ret) {
  struct cxlmi_cmd_ddr_freq_get *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) != sizeof(struct cxlmi_cmd_ddr_freq_get));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, DDR_FREQUENCY_GET);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_ddr_freq_get *)rsp->payload;
  ret->ddr_freq = rsp_pl->ddr_freq;

  return rc;
}

int cxlmi_cmd_ddr_init_err_info_get(
    struct cxlmi_endpoint *ep, struct cxlmi_tunnel_info *ti,
    struct cxlmi_cmd_ddr_init_err_info_get *ret) {
  struct cxlmi_cmd_ddr_init_err_info_get *rsp_pl;
  struct cxlmi_cci_msg req;
  _cleanup_free_ struct cxlmi_cci_msg *rsp = NULL;
  ssize_t rsp_sz;
  int rc;

  CXLMI_BUILD_BUG_ON(sizeof(*ret) !=
                     sizeof(struct cxlmi_cmd_ddr_init_err_info_get));

  arm_cci_request(ep, &req, 0, VENDOR_CMD_OTHERS, DDR_INIT_ERR_INFO_GET);

  rsp_sz = sizeof(*rsp) + sizeof(*rsp_pl);
  rsp = calloc(1, rsp_sz);
  if (!rsp)
    return -1;

  rc = send_cmd_cci(ep, ti, &req, sizeof(req), rsp, rsp_sz, rsp_sz);
  if (rc)
    return rc;

  rsp_pl = (struct cxlmi_cmd_ddr_init_err_info_get *)rsp->payload;
  memcpy(ret, rsp_pl, sizeof(struct cxlmi_cmd_ddr_init_err_info_get));

  return rc;
}
