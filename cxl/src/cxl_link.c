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

/* Helper functions for CXL errors */
// corr errors list
static char *corr_errors_list[MAX_CORR_ERR_COUNT] = {
    "CORR_CACHE_DATA_ECC",  "CORR_MEM_DATA_ECC", "CORR_CRC_THRESHOLD",
    "CORR_RETRY_THRESHOLD", "CACHE_POISON_RVD",  "MEM_POISON_RVD",
    "PHY_LAYER_ERR"};

// uncorr errors list
static char *uncorr_errors_list[MAX_UNCORR_ERR_COUNT] = {
    "CACHE_DATA_PAR",   "CACHE_ADD_PAR",   "CACHE_BE_PAR",     "CACHE_DATA_ECC",
    "MEM_DATA_PAR",     "MEM_ADD_PAR",     "MEM_BE_PAR",       "MEM_DATA_ECC",
    "REINIT_THRESHOLD", "ENCOD_VIOLATION", "POISON_RVD",       "RCVR_OVERFLOW",
    "RSVDP_12",         "RSVDP_13",        "CXL_INTERNAL_ERR", "CXL_IDE_TX_ER",
    "CXL_IDE_RX_ER"};

// uncorr errors list
static char *cxl_cfg_errors_list[MAX_CXL_CFG_ERR_COUNT] = {
    "UNCOR_INTERNAL_ERR_STS",
    "SURPRISE_DOWN_ER_STS",
    "REPLAY_TIMER_TIMEOUT_ERR_STS",
    "REPLAY_NUMBER_ROLLOVER_ERR_STS",
    "RCVR_OVERFLOW_ERR_STS",
    "RCVR_ERR_STS",
    "MLF_TLP_ERR_STS",
    "FC_PROTOCOL_ERR_STS",
    "ECRC_ERR_STS",
    "DL_PROTOCOL_ERR_STS",
    "CORRECTED_INTERNAL_ERR_STS",
    "BAD_TLP_ERR_STS",
    "BAD_DLLP_ERR_STS"};

void display_cxl_error_info(struct cxlmi_cmd_cxl_err_cntr_get *cxl_err_cnt) {
  if (!cxl_err_cnt) {
    printf("Null pointer, cannot display structure\r\n");
    return;
  }
  int i = 0;

  printf("Correctable Error : \n");
  for (i = 0; i < MAX_CORR_ERR_COUNT; i++) {
    printf("cxl : %25s  : %4d \n", corr_errors_list[i],
           cxl_err_cnt->corr_err[i]);
  }

  printf("UnCorrectable Error : \n");
  for (i = 0; i < MAX_UNCORR_ERR_COUNT; i++) {
    printf("cxl : %25s  : %4d \n", uncorr_errors_list[i],
           cxl_err_cnt->uncorr_err[i]);
  }

  printf("Cxl Cfg status Error : \n");
  for (i = 0; i < MAX_CXL_CFG_ERR_COUNT; i++) {
    printf("cxl : %25s  : %4d \n", cxl_cfg_errors_list[i],
           cxl_err_cnt->cxl_conf_err[i]);
  }
  printf("Total count of Correctable errors %ld\n",
         cxl_err_cnt->total_corr_err_cnt);
  printf("Total count of Uncorrectable errors %ld\n",
         cxl_err_cnt->total_uncorr_err_cnt);
  printf("Total count of cxl_cfg errors %ld\n",
         cxl_err_cnt->total_cxl_cfg_err_cnt);
  printf("Total number of errors %ld\n", cxl_err_cnt->total_err_cnt);
}