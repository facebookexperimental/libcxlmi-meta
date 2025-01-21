// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#ifndef __MEMBRIDGE_H__
#define __MEMBRIDGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
  WSTRB_PARITY_ERROR = 0,
  WDATA_PARITY_ERROR,
  RDATA_RRESP_PARITY_ERROR,
  RDATA_RESP_RID_PARITY_ERROR,
  RDATA_PARITY_ERROR,
  R_RESP_PARITY_ERROR,
  B_RESP_PARITY_ERROR,
  AW_QOS_PARITY_ERROR,
  AW_MISC_PARITY_ERROR,
  AW_ID_PARITY_ERROR,
  AW_ADDR_RANGE_PARITY_ERROR,
  AW_ADDR_PARITY_ERROR,
  AR_QOS_PARITY_ERROR,
  AR_MISC_PARITY_ERROR,
  AR_ID_PARITY_ERROR,
  AR_ADDR_RANGE_PARITY_ERROR,
  AR_ADDR_PARITY_ERROR,
  AXI_B_ID_PARITY_ERROR,
  AXI_B_PARITY_ERROR,
  AXI_R_ID_PARITY_ERROR,
  AXI_R_DATA_PARITY_ERROR,
  AXI_R_PARITY_ERROR,
  DDR_PARITY_ERROR_COUNT = 22,
} ddr_parity_errors;

typedef enum {
  S2M_NDR_FIFO = 0,
  S2M_DRC_FIFO,
  M2S_RWD_FIFO,
  M2S_REQ_FIFO,
  DDR1_W_REQ_FIFO,
  DDR1_RDATA_RESP_FIFO,
  DDR1_R_RESP_FIFO,
  DDR1_B_RESP_FIFO,
  DDR1_AW_REQ_FIFO,
  DDR1_AR_REQ_FIFO,
  DDR0_W_REQ_FIFO,
  DDR0_RDATA_RESP_FIFO,
  DDR0_R_RESP_FIFO,
  DDR0_B_RESP_FIFO,
  DDR0_AW_REQ_FIFO,
  DDR0_AR_REQ_FIFO,
  FIFO_ERROR_COUNT = 16,
} fifo_errors;

typedef enum {
  NDR_TAG_PARITY_ERROR = 0,
  NDR_RESP_PARITY_ERROR,
  M2S_RWD_ECC_CHECK_ERR_MULTPL_FAIL,
  M2S_RWD_ECC_CHECK_ERR_DETECT_FAIL,
  M2S_REQ_ECC_CHECK_ERR_MULTPL_FAIL,
  M2S_REQ_ECC_CHECK_ERR_DETECT_FAIL,
  DRC_TAG_PARITY_ERROR,
  DRC_RESP_PARITY_ERROR,
  DRC_DATA_PARITY_ERROR,
  AW_MST_RWD_PARITY_ERROR,
  AR_MST_REQ_PARITY_ERROR,
  M2S_REQ_DUP_ADDR_PARITY_ERROR,
  M2S_RWD_DUP_ADDR_PARITY_ERROR,
  PARITY_ERROR_COUNT = 13,
} parity_errors;

typedef enum {
  MST_M2S_RWD_ERR_MULTPL = 0,
  MST_M2S_RWD_ERR_DETECT,
  MST_M2S_REQ_ERR_MULTPL,
  MST_M2S_REQ_ERR_DETECT,
  POISON_RECEIVED_IN_RWD,
  RWD_ADDRESS_INVALID,
  REQ_ADDRESS_INVALID,
  DDR1_RRESP_ERROR,
  DDR1_BRESP_ERROR,
  DDR0_RRESP_ERROR,
  DDR0_BRESP_ERROR,
  DDR1_RPARITY_ERROR,
  DDR1_BPARITY_ERROR,
  DDR0_RPARITY_ERROR,
  DDR0_BPARITY_ERROR,
  HDM_DEC1_ERR_NOT_COMMITED,
  RX_DEINIT_TIMEOUT,
  TX_DEINIT_TIMEOUT,
  VIRAL,
  DDR0_BRESP_DEC_ERROR,
  DDR1_BRESP_DEC_ERROR,
  DDR0_RRESP_DEC_ERROR,
  DDR1_RRESP_DEC_ERROR,
  MEMBRIDGE_COMMON_ERROR_COUNT = 23
} membridge_common_errors;

#ifdef __cplusplus
}
#endif

#endif /* __MEMBRIDGE_H__ */
