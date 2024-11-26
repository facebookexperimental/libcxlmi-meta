// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

/* std includes */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* libcxlmi includes */
#include <ccan/short_types/short_types.h>

/* vendor includes */
#include <vendor_types.h>

char *ddr_parity_error_strings[DDR_PARITY_ERROR_COUNT] = {
    "WSTRB_PARITY_ERROR",         "WDATA_PARITY_ERROR",
    "RDATA_RRESP_PARITY_ERROR",   "RDATA_RESP_RID_PARITY_ERROR",
    "RDATA_PARITY_ERROR",         "R_RESP_PARITY_ERROR",
    "B_RESP_PARITY_ERROR",        "AW_QOS_PARITY_ERROR",
    "AW_MISC_PARITY_ERROR",       "AW_ID_PARITY_ERROR",
    "AW_ADDR_RANGE_PARITY_ERROR", "AW_ADDR_PARITY_ERROR",
    "AR_QOS_PARITY_ERROR",        "AR_MISC_PARITY_ERROR",
    "AR_ID_PARITY_ERROR",         "AR_ADDR_RANGE_PARITY_ERROR",
    "AR_ADDR_PARITY_ERROR",       "AXI_B_ID_PARITY_ERROR",
    "AXI_B_PARITY_ERROR",         "AXI_R_ID_PARITY_ERROR",
    "AXI_R_DATA_PARITY_ERROR",    "AXI_R_PARITY_ERROR",
};

char *fifo_error_strings[FIFO_ERROR_COUNT] = {
    "S2M_NDR_FIFO",     "S2M_DRC_FIFO",     "M2S_RWD_FIFO",
    "M2S_REQ_FIFO",     "DDR1_W_REQ_FIFO",  "DDR1_RDATA_RESP_FIFO",
    "DDR1_R_RESP_FIFO", "DDR1_B_RESP_FIFO", "DDR1_AW_REQ_FIFO",
    "DDR1_AR_REQ_FIFO", "DDR0_W_REQ_FIFO",  "DDR0_RDATA_RESP_FIFO",
    "DDR0_R_RESP_FIFO", "DDR0_B_RESP_FIFO", "DDR0_AW_REQ_FIFO",
    "DDR0_AR_REQ_FIFO"};

char *parity_error_strings[PARITY_ERROR_COUNT] = {
    "NDR_TAG_PARITY_ERROR",
    "NDR_RESP_PARITY_ERROR",
    "M2S_RWD_ECC_CHECK_ERR_MULTPL_FAIL",
    "M2S_RWD_ECC_CHECK_ERR_DETECT_FAIL",
    "M2S_REQ_ECC_CHECK_ERR_MULTPL_FAIL",
    "M2S_REQ_ECC_CHECK_ERR_DETECT_FAIL",
    "DRC_TAG_PARITY_ERROR",
    "DRC_RESP_PARITY_ERROR",
    "DRC_DATA_PARITY_ERROR",
    "AW_MST_RWD_PARITY_ERROR",
    "AR_MST_REQ_PARITY_ERROR",
    "M2S_REQ_DUP_ADDR_PARITY_ERROR",
    "M2S_RWD_DUP_ADDR_PARITY_ERROR",
};

char *membridge_common_error_strings[MEMBRIDGE_COMMON_ERROR_COUNT] = {
    "MST_M2S_RWD_ERR_MULTPL",
    "MST_M2S_RWD_ERR_DETECT",
    "MST_M2S_REQ_ERR_MULTPL",
    "MST_M2S_REQ_ERR_DETECT",
    "POISON_RECEIVED_IN_RWD",
    "RWD_ADDRESS_INVALID",
    "REQ_ADDRESS_INVALID",
    "DDR1_RRESP_ERROR",
    "DDR1_BRESP_ERROR",
    "DDR0_RRESP_ERROR",
    "DDR0_BRESP_ERROR",
    "DDR1_RPARITY_ERROR",
    "DDR1_BPARITY_ERROR",
    "DDR0_RPARITY_ERROR",
    "DDR0_BPARITY_ERROR",
    "HDM_DEC1_ERR_NOT_COMMITED",
    "RX_DEINIT_TIMEOUT",
    "TX_DEINIT_TIMEOUT",
    "VIRAL",
    "DDR0_BRESP_DEC_ERROR",
    "DDR1_BRESP_DEC_ERROR",
    "DDR0_RRESP_DEC_ERROR",
    "DDR1_RRESP_DEC_ERROR",
};

void display_membridge_errors(
    struct cxlmi_cmd_get_membridge_errors *membridge_err) {
  int idx = 0;

  if (!membridge_err) {
    printf("Null pointer, cannot display structure\r\n");
    return;
  }

  printf("fifo_overflow errors : %d\n", membridge_err->fifo_overflow);
  for (idx = 0; idx < FIFO_ERROR_COUNT; idx++) {
    if (membridge_err->fifo_overflows[idx] != 0)
      printf("%s : 0x%x\n", fifo_error_strings[idx],
             membridge_err->fifo_overflows[idx]);
  }

  printf("fifo_underflow errors : %d\n", membridge_err->fifo_underflow);
  for (idx = 0; idx < FIFO_ERROR_COUNT; idx++) {
    if (membridge_err->fifo_underflows[idx] != 0)
      printf("%s : 0x%x\n", fifo_error_strings[idx],
             membridge_err->fifo_underflows[idx]);
  }

  printf("ddr0 parity errors : %d\n", membridge_err->ddr0_parity_error);
  for (idx = 0; idx < DDR_PARITY_ERROR_COUNT; idx++) {
    if (membridge_err->ddr0_parity_errors[idx] != 0)
      printf("%s : 0x%x\n", ddr_parity_error_strings[idx],
             membridge_err->ddr0_parity_errors[idx]);
  }

  printf("ddr1 parity errors : %d\n", membridge_err->ddr1_parity_error);
  for (idx = 0; idx < DDR_PARITY_ERROR_COUNT; idx++) {
    if (membridge_err->ddr1_parity_errors[idx] != 0)
      printf("%s : 0x%x\n", ddr_parity_error_strings[idx],
             membridge_err->ddr1_parity_errors[idx]);
  }

  printf("membridge common errors :\n");
  for (idx = 0; idx < MEMBRIDGE_COMMON_ERROR_COUNT; idx++) {
    if (membridge_err->common_errors[idx] != 0)
      printf("%s : 0x%x\n", membridge_common_error_strings[idx],
             membridge_err->common_errors[idx]);
  }

  printf("parity errors : %d\n", membridge_err->parity_error);
  for (idx = 0; idx < PARITY_ERROR_COUNT; idx++) {
    if (membridge_err->parity_errors[idx] != 0)
      printf("%s : 0x%x\n", parity_error_strings[idx],
             membridge_err->parity_errors[idx]);
  }
}

void display_membridge_stats(struct cxlmi_cmd_get_membridge_stats *stats) {
  if (!stats) {
    printf("Null pointer, cannot display structure\r\n");
    return;
  }
  printf("m2s_req_count:              %lu\n", stats->m2s_req_count);
  printf("m2s_rwd_count:              %lu\n", stats->m2s_rwd_count);
  printf("s2m_drs_count:              %lu\n", stats->s2m_drs_count);
  printf("s2m_ndr_count:              %lu\n", stats->s2m_ndr_count);
  printf("rwd_first_poison_hpa:       0x%lx\n",
         stats->rwd_first_poison_hpa_log);
  printf("rwd_latest_poison_hpa:      0x%lx\n",
         stats->rwd_latest_poison_hpa_log);
  printf("req_first_hpa_log:          0x%lx\n", stats->req_first_hpa_log);
  printf("rwd_first_hpa_log:          0x%lx\n", (u64)stats->rwd_first_hpa_log);
  printf("m2s_req_corr_err_count:     %u\n", stats->mst_m2s_req_corr_err_count);
  printf("m2s_rwd_corr_err_count:     %u\n", stats->mst_m2s_rwd_corr_err_count);
  printf("fifo_full_status:           0x%x\n", stats->fifo_full_status);
  printf("fifo_empty_status:          0x%x\n", stats->fifo_empty_status);
  printf("m2s_rwd_credit_count:       %u\n", stats->m2s_rwd_credit_count);
  printf("m2s_req_credit_count:       %u\n", stats->m2s_req_credit_count);
  printf("s2m_ndr_credit_count:       %u\n", stats->s2m_ndr_credit_count);
  printf("s2m_drc_credit_count:       %u\n", stats->s2m_drc_credit_count);
  printf("rx_status_rx_deinit:        0x%x\n", stats->rx_fsm_status_rx_deinit);
  printf("rx_status_m2s_req:          0x%x\n", stats->rx_fsm_status_m2s_req);
  printf("rx_status_m2s_rwd:          0x%x\n", stats->rx_fsm_status_m2s_rwd);
  printf("rx_status_ddr0_ar_req:      0x%x\n",
         stats->rx_fsm_status_ddr0_ar_req);
  printf("rx_status_ddr0_aw_req:      0x%x\n",
         stats->rx_fsm_status_ddr0_aw_req);
  printf("rx_status_ddr0_w_req:       0x%x\n", stats->rx_fsm_status_ddr0_w_req);
  printf("rx_status_ddr1_ar_req:      0x%x\n",
         stats->rx_fsm_status_ddr1_ar_req);
  printf("rx_status_ddr1_aw_req:      0x%x\n",
         stats->rx_fsm_status_ddr1_aw_req);
  printf("rx_status_ddr1_w_req:       0x%x\n", stats->rx_fsm_status_ddr1_w_req);
  printf("tx_status_tx_deinit:        0x%x\n", stats->tx_fsm_status_tx_deinit);
  printf("tx_status_s2m_ndr:          0x%x\n", stats->tx_fsm_status_s2m_ndr);
  printf("tx_status_s2m_drc:          0x%x\n", stats->tx_fsm_status_s2m_drc);
  printf("qos_tel_dev_load_read:      %u\n", stats->stat_qos_tel_dev_load_read);
  printf("qos_tel_dev_load_type2_read:%u\n",
         stats->stat_qos_tel_dev_load_type2_read);
  printf("qos_tel_dev_load_write:     %u\n",
         stats->stat_qos_tel_dev_load_write);
}
