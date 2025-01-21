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

/* Helper function for ddr */
void display_error_count(struct ddr_controller_errors *ddr_ctrl_err,
                         ddr_subsys ddr_id) {

  if (!ddr_ctrl_err) {
    printf("Null pointer, cannot display structure\r\n");
    return;
  }

  if (ddr_ctrl_err[ddr_id].parity.parity_crit_bit2_cnt) {
    printf("DDR-%d: FATAL: Parity error on the address/control bus "
           "(parity_crit_bit2_cnt= %u)\n",
           ddr_id, ddr_ctrl_err[ddr_id].parity.parity_crit_bit2_cnt);
  }
  if (ddr_ctrl_err[ddr_id].parity.parity_crit_bit1_cnt) {
    printf("DDR-%d: FATAL: Overlapping write data parity error "
           "(parity_crit_bit1_cnt= %u)\n",
           ddr_id, ddr_ctrl_err[ddr_id].parity.parity_crit_bit1_cnt);
  }

  if (ddr_ctrl_err[ddr_id].parity.parity_crit_bit0_cnt) {
    printf("DDR-%d: FATAL: Write data parity error "
           "(parity_crit_bit0_cnt= %u)\n",
           ddr_id, ddr_ctrl_err[ddr_id].parity.parity_crit_bit0_cnt);
  }

  if (ddr_ctrl_err[ddr_id].dfi.dfi_crit_bit5_cnt) {
    printf("DDR-%d: FATAL: DFI tINIT_COMPLETE value has timed out "
           "(dfi_crit_bit5_cnt= %u)\n",
           ddr_id, ddr_ctrl_err[ddr_id].dfi.dfi_crit_bit5_cnt);
  }

  if (ddr_ctrl_err[ddr_id].dfi.dfi_crit_bit2_cnt) {
    printf("DDR-%d: FATAL : Error received from the PHY on the DFI bus "
           "(dfi_crit_bit2_cnt= %u)\n",
           ddr_id, ddr_ctrl_err[ddr_id].dfi.dfi_crit_bit2_cnt);
  }

  if (ddr_ctrl_err[ddr_id].dfi.dfi_warn_bit1_cnt) {
    printf("DDR-%d: WARN: DFI PHY Master Interface error has occurred "
           "(dfi_warn_bit1_cnt= %u)\n",
           ddr_id, ddr_ctrl_err[ddr_id].dfi.dfi_warn_bit1_cnt);
  }

  if (ddr_ctrl_err[ddr_id].dfi.dfi_warn_bit0_cnt) {
    printf("DDR-%d: WARN: DFI update error has occurred "
           "(dfi_warn_bit0_cnt= %u)\n",
           ddr_id, ddr_ctrl_err[ddr_id].dfi.dfi_warn_bit0_cnt);
  }

  if (ddr_ctrl_err[ddr_id].crc.crc_crit_bit1_cnt) {
    printf("DDR-%d: FATAL: CA Parity or a CRC error happened during CRC Retry "
           "(crc_crit_bit1_cnt= %u)\n",
           ddr_id, ddr_ctrl_err[ddr_id].crc.crc_crit_bit1_cnt);
  }

  if (ddr_ctrl_err[ddr_id].crc.crc_crit_bit0_cnt) {
    printf("DDR-%d: FATAL: CRC error occurred on the write data bus "
           "(crc_crit_bit0_cnt= %u)\n",
           ddr_id, ddr_ctrl_err[ddr_id].crc.crc_crit_bit0_cnt);
  }

  if (ddr_ctrl_err[ddr_id].userif.userif_crit_bit2_cnt) {
    printf("DDR-%d: FATAL: Error occurred on the port command channel "
           "(userif_crit_bit2_cnt= %u)\n",
           ddr_id, ddr_ctrl_err[ddr_id].userif.userif_crit_bit2_cnt);
  }
  if (ddr_ctrl_err[ddr_id].userif.userif_crit_bit1_cnt) {
    printf("DDR-%d: FATAL: Multiple accesses outside the defined PHYSICAL "
           "memory space have occurred "
           "(userif_crit_bit1_cnt= %u)\n",
           ddr_id, ddr_ctrl_err[ddr_id].userif.userif_crit_bit1_cnt);
  }

  if (ddr_ctrl_err[ddr_id].userif.userif_crit_bit0_cnt) {
    printf("DDR-%d: FATAL: A Memory access outside the defined PHYSICAL "
           "memory space has occurred "
           "(userif_crit_bit0_cnt= %u)\n",
           ddr_id, ddr_ctrl_err[ddr_id].userif.userif_crit_bit0_cnt);
  }

  if (ddr_ctrl_err[ddr_id].ecc.ecc_warn_bit6_cnt) {
    printf("DDR-%d: WARN: One or more ECC writeback commands "
           "could not be executed "
           "(ecc_warn_bit6_cnt= %u)\n",
           ddr_id, ddr_ctrl_err[ddr_id].ecc.ecc_warn_bit6_cnt);
  }

  if (ddr_ctrl_err[ddr_id].ecc.ecc_crit_bit3_cnt) {
    printf("DDR-%d:FATAL: Multiple uncorrectable ECC events have been detected "
           "(ecc_crit_bit3_cnt= %u)\n",
           ddr_id, ddr_ctrl_err[ddr_id].ecc.ecc_crit_bit3_cnt);
  }

  if (ddr_ctrl_err[ddr_id].ecc.ecc_crit_bit2_cnt) {
    printf("DDR-%d: FATAL: A uncorrectable ECC event has been detected "
           "(ecc_crit_bit2_cnt= %u)\n",
           ddr_id, ddr_ctrl_err[ddr_id].ecc.ecc_crit_bit2_cnt);
  }
  if (ddr_ctrl_err[ddr_id].ecc.ecc_crit_bit8_cnt) {
    printf("DDR-%d: CRIT: An ECC correctable error has been detected "
           "in a scrubbing read operation "
           "(ecc_crit_bit8_cnt= %u)\n",
           ddr_id, ddr_ctrl_err[ddr_id].ecc.ecc_crit_bit8_cnt);
  }

  if (ddr_ctrl_err[ddr_id].ecc.ecc_warn_bit1_cnt) {
    printf("DDR-%d: WARN: Multiple correctable ECC events have been detected "
           "(ecc_warn_bit1_cnt= %u)\n",
           ddr_id, ddr_ctrl_err[ddr_id].ecc.ecc_warn_bit1_cnt);
  }

  if (ddr_ctrl_err[ddr_id].ecc.ecc_warn_bit0_cnt) {
    printf("DDR-%d: WARN: A correctable ECC event has been detected "
           "(ecc_warn_bit0_cnt= %u)\n",
           ddr_id, ddr_ctrl_err[ddr_id].ecc.ecc_warn_bit0_cnt);
  }
}

void display_ddr_init_status(
    struct cxlmi_cmd_ddr_init_status *ddr_init_status) {
  if (!ddr_init_status) {
    printf("Null pointer, cannot display structure\r\n");
    return;
  }

  int status = ddr_init_status->init_status.ddr_init_status;

  switch (status) {
  case DDR_INIT_INPROGRESS:
    printf("DDR INIT IS IN PROGRESS\n");
    break;
  case DDR_INIT_PASSED:
    printf("DDR INIT PASSED\n");
    break;
  case DDR_INIT_FAILED:
    printf("DDR INIT FAILED for CH:%d DIMM:%c\n",
           ddr_init_status->init_status.failed_channel_id,
           ddr_init_status->init_status.failed_dimm_silk_screen);

    printf("RECOVERY REMEDY: REPLACE CH:%d DIMM:%c and RE-TRY\n",
           ddr_init_status->init_status.failed_channel_id,
           ddr_init_status->init_status.failed_dimm_silk_screen);
    break;
  case DDR_INIT_FAILED_NO_CH0_DIMM0:
    printf("DDR INIT FAILED. CH:%d DIMM:%c is NOT PLUGGED IN\n",
           ddr_init_status->init_status.failed_channel_id,
           ddr_init_status->init_status.failed_dimm_silk_screen);

    printf("RECOVERY REMEDY: PLUG IN CH:%d DIMM:%c\n",
           ddr_init_status->init_status.failed_channel_id,
           ddr_init_status->init_status.failed_dimm_silk_screen);
    break;
  case DDR_INIT_FAILED_UNKNOWN_DIMM:
    printf("DDR INIT FAILED. UN-SUPPORTED/UNKNOWN DIMM\n");
    printf("RECOVERY REMEDY: PLUG IN SUPPORTED DIMMs\n");
    break;
  default:
    printf("DDR INIT STATUS invalid\n");
  }

  return;
}

void display_pmon_stats(ddr_stats_data_t *disp_stats, uint32_t loop_count) {
  uint32_t loop;

  if (!disp_stats) {
    printf("Null pointer, cannot display structure\r\n");
    return;
  }

  printf("PMON STATS:\n");
  printf(
      "iteration, fr_cnt, idle_cnt, rd_ot_cnt, wr_ot_cnt, wrd_ot_cnt, "
      "rd_cmd_cnt, rd_cmd_busy_cnt, wr_cmd_cnt, wr_cmd_busy_cnt, rd_data_cnt, "
      "rd_data_busy_cnt, wr_data_cnt, wr_data_busy_cnt, "
      "rd_avg_lat, wr_avg_lat, rd_trans_smpl_cnt, wr_trans_smpl_cnt\n");
  for (loop = 0; loop < loop_count; loop++) {
    printf("[%d], %lu, %u, %u, %u, %u, "
           "%u, %u, %u, %u, %u, "
           "%u, %u, %u, "
           "%lu, %lu, %u, %u\n",
           loop, disp_stats->stats.pmon.fr_cnt, disp_stats->stats.pmon.idle_cnt,
           disp_stats->stats.pmon.rd_ot_cnt, disp_stats->stats.pmon.wr_ot_cnt,
           disp_stats->stats.pmon.wrd_ot_cnt, disp_stats->stats.pmon.rd_cmd_cnt,
           disp_stats->stats.pmon.rd_cmd_busy_cnt,
           disp_stats->stats.pmon.wr_cmd_cnt,
           disp_stats->stats.pmon.wr_cmd_busy_cnt,
           disp_stats->stats.pmon.rd_data_cnt,
           disp_stats->stats.pmon.rd_data_busy_cnt,
           disp_stats->stats.pmon.wr_data_cnt,
           disp_stats->stats.pmon.wr_data_busy_cnt,
           disp_stats->stats.pmon.rd_avg_lat, disp_stats->stats.pmon.wr_avg_lat,
           disp_stats->stats.pmon.rd_trans_smpl_cnt,
           disp_stats->stats.pmon.wr_trans_smpl_cnt);
    disp_stats++;
  }
  printf("\n");
}

void display_cs_pm_stats(ddr_stats_data_t *disp_stats, uint32_t loop_count) {
  uint32_t rank, loop;

  if (!disp_stats) {
    printf("Null pointer, cannot display structure\r\n");
    return;
  }

  printf("CS PM STATS:\n");
  printf("iteration, rank, mrw_cnt, refresh_cnt, act_cnt, write_cnt, "
         "read_cnt, pre_cnt, rr_cnt, ww_cnt, rw_cnt\n ");

  for (loop = 0; loop < loop_count; loop++) {
    for (rank = 0; rank < NUM_CS; rank++) {
      printf("[%d], %d, %u, %u, %u, %u, "
             "%u, %u, %u, %u, %u\n",
             loop, rank, disp_stats->stats.cs_pm[rank].mrw_cnt,
             disp_stats->stats.cs_pm[rank].refresh_cnt,
             disp_stats->stats.cs_pm[rank].act_cnt,
             disp_stats->stats.cs_pm[rank].write_cnt,
             disp_stats->stats.cs_pm[rank].read_cnt,
             disp_stats->stats.cs_pm[rank].pre_cnt,
             disp_stats->stats.cs_pm[rank].rr_cnt,
             disp_stats->stats.cs_pm[rank].ww_cnt,
             disp_stats->stats.cs_pm[rank].rw_cnt);
    }
    disp_stats++;
  }
  printf("\n");
}

void display_cs_bank_pm_stats(ddr_stats_data_t *disp_stats,
                              uint32_t loop_count) {
  uint32_t rank, bank, loop;

  printf("CS BANK STATS:\n");
  printf("iteration, rank, bank, bank_act_cnt, bank_wr_cnt, bank_rd_cnt, "
         "bank_pre_cnt\n");
  for (loop = 0; loop < loop_count; loop++) {
    for (rank = 0; rank < NUM_CS; rank++) {
      for (bank = 0; bank < NUM_BANK; bank++) {
        printf("[%d], %d, %d, %u, %u, %u, %u\n", loop, rank, bank,
               disp_stats->stats.cs_bank_pm[rank][bank].bank_act_cnt,
               disp_stats->stats.cs_bank_pm[rank][bank].bank_wr_cnt,
               disp_stats->stats.cs_bank_pm[rank][bank].bank_rd_cnt,
               disp_stats->stats.cs_bank_pm[rank][bank].bank_pre_cnt);
      }
    }
    disp_stats++;
  }
  printf("\n");
}

void display_mc_pm_stats(ddr_stats_data_t *disp_stats, uint32_t loop_count) {
  uint32_t loop;

  if (!disp_stats) {
    printf("Null pointer, cannot display structure\r\n");
    return;
  }

  printf("PM STATS:\n");
  printf("iteration, cmd_queue_full_events, info_fifo_full_events, "
         "wrdata_hold_fifo_full_events, port_cmd_fifo0_full_events, "
         "port_wrresp_fifo0_full_events, port_wr_fifo0_full_events, "
         "port_rd_fifo0_full_events, port_cmd_fifo1_full_events, "
         "port_wrresp_fifo1_full_events, port_wr_fifo1_full_events, "
         "port_rd_fifo1_full_events, ecc_dataout_corrected, "
         "ecc_dataout_uncorrected, pd_ex, pd_en, srex, sren, "
         "write, read, rmw, bank_act, precharge, precharge_all, "
         "mrw, auto_ref, rw_auto_pre, zq_cal_short, zq_cal_long, "
         "same_addr_ww_collision, same_addr_wr_collision, "
         "same_addr_rw_collision, same_addr_rr_collision\n");

  for (loop = 0; loop < loop_count; loop++) {
    printf("[%d], %u, %u, "
           "%u, %u, "
           "%u, %u, "
           "%u, %u, "
           "%u, %u, "
           "%u, %u, "
           "%u, %u, %u, %u, %u,"
           "%u, %u, %u, %u, %u, %u,"
           "%u, %u, %u, %u, %u,"
           "%u, %u, "
           "%u, %u\n",
           loop, disp_stats->stats.mc_pm.cmd_queue_full_events,
           disp_stats->stats.mc_pm.info_fifo_full_events,
           disp_stats->stats.mc_pm.wrdata_hold_fifo_full_events,
           disp_stats->stats.mc_pm.port_cmd_fifo0_full_events,
           disp_stats->stats.mc_pm.port_wrresp_fifo0_full_events,
           disp_stats->stats.mc_pm.port_wr_fifo0_full_events,
           disp_stats->stats.mc_pm.port_rd_fifo0_full_events,
           disp_stats->stats.mc_pm.port_cmd_fifo1_full_events,
           disp_stats->stats.mc_pm.port_wrresp_fifo1_full_events,
           disp_stats->stats.mc_pm.port_wr_fifo1_full_events,
           disp_stats->stats.mc_pm.port_rd_fifo1_full_events,
           disp_stats->stats.mc_pm.ecc_dataout_corrected,
           disp_stats->stats.mc_pm.ecc_dataout_uncorrected,
           disp_stats->stats.mc_pm.pd_ex, disp_stats->stats.mc_pm.pd_en,
           disp_stats->stats.mc_pm.srex, disp_stats->stats.mc_pm.sren,
           disp_stats->stats.mc_pm.write, disp_stats->stats.mc_pm.read,
           disp_stats->stats.mc_pm.rmw, disp_stats->stats.mc_pm.bank_act,
           disp_stats->stats.mc_pm.precharge,
           disp_stats->stats.mc_pm.precharge_all, disp_stats->stats.mc_pm.mrw,
           disp_stats->stats.mc_pm.auto_ref,
           disp_stats->stats.mc_pm.rw_auto_pre,
           disp_stats->stats.mc_pm.zq_cal_short,
           disp_stats->stats.mc_pm.zq_cal_long,
           disp_stats->stats.mc_pm.same_addr_ww_collision,
           disp_stats->stats.mc_pm.same_addr_wr_collision,
           disp_stats->stats.mc_pm.same_addr_rw_collision,
           disp_stats->stats.mc_pm.same_addr_rr_collision);
    disp_stats++;
  }
  printf("\n");
}

/* Helper functions for DDR DIMM level training status */
void print_ddr_training_status(uint32_t instance,
                               struct ddr_dimm_training_status *dimm_tr_status);
void print_read_gate_training_status(
    uint32_t instance, struct ddr_read_gate_training_status *rd_gate_tr_status);
void print_write_levelling_status(
    uint32_t instance, struct ddr_wr_levelling_status *wr_levl_status);
void print_ddr_phy_pll_status(uint32_t instance,
                              struct ddr_phy_pll_status *phy_pll_status);
void print_ddr_training_status(uint32_t instance,
                               struct ddr_dimm_training_status *dimm_tr_status);
void print_margin_vref_low_high(
    uint32_t instance, struct ddr_dimm_training_status *dimm_tr_status);
void print_margin_rdlvl_delay_window(
    int instance, struct ddr_dimm_training_status *dimm_tr_status);
void print_margin_wrdqlvl_delay_window(
    int instance, struct ddr_dimm_training_status *dimm_tr_status);
void print_err_status(int instance,
                      struct ddr_dimm_training_status *dimm_tr_status);

/* DDR phy pll status */
void print_ddr_phy_pll_status(uint32_t instance,
                              struct ddr_phy_pll_status *phy_pll_status) {
  uint32_t read_data;

  if (!phy_pll_status) {
    printf("Null pointer, cannot display structure\r\n");
    return;
  }

  read_data = phy_pll_status->bs0_status;
  printf("DDR%d PHY PLL Status: \n", instance);
  printf("\tOBS0: \n");
  printf("\t\tPLL Lock Status   = %d \n", (read_data & 1));
  printf("\t\tReady			 = %d \n", ((read_data & 0x2) >> 1));
  printf("\t\tLock assert count = 0x%x \n", ((read_data & 0x7F8) >> 3));

  read_data = phy_pll_status->bs1_status;
  printf("\tOBS1: \n");
  printf("\t\tPLL Lock Status   = %d \n", (read_data & 1));
  printf("\t\tReady			 = %d \n", ((read_data & 0x2) >> 1));
  printf("\t\tLock assert count = 0x%x \n\n", ((read_data & 0x7F8) >> 3));
}

void print_write_levelling_status(
    uint32_t instance, struct ddr_wr_levelling_status *wr_levl_status) {
  uint32_t read_data = 0;
  int i = 0;

  if (!wr_levl_status) {
    printf("Null pointer, cannot display structure\r\n");
    return;
  }

  printf("\t\tBYTE# \t\t\t\t 0 \t 1 \t 2 \t 3 \t 4 \t 5 \t 6 \t 7 \t 8\n");
  read_data = wr_levl_status->lower_nibble_err[i++];
  printf("\t\tLOWER NIBBLE ERROR FLAG \t %d", (read_data & 0x1000) >> 12);
  read_data = wr_levl_status->lower_nibble_err[i++];
  printf("\t %d", (read_data & 0x1000) >> 12);
  read_data = wr_levl_status->lower_nibble_err[i++];
  printf("\t %d", (read_data & 0x1000) >> 12);
  read_data = wr_levl_status->lower_nibble_err[i++];
  printf("\t %d", (read_data & 0x1000) >> 12);
  read_data = wr_levl_status->lower_nibble_err[i++];
  printf("\t %d", (read_data & 0x1000) >> 12);
  read_data = wr_levl_status->lower_nibble_err[i++];
  printf("\t %d", (read_data & 0x1000) >> 12);
  read_data = wr_levl_status->lower_nibble_err[i++];
  printf("\t %d", (read_data & 0x1000) >> 12);
  read_data = wr_levl_status->lower_nibble_err[i++];
  printf("\t %d", (read_data & 0x1000) >> 12);
  read_data = wr_levl_status->lower_nibble_err[i++];
  printf("\t %d\n", (read_data & 0x1000) >> 12);

  i = 0;
  read_data = wr_levl_status->upper_nibble_err[i++];
  printf("\t\tUPPER NIBBLE ERROR FLAG \t %d", (read_data & 0x4000) >> 14);
  read_data = wr_levl_status->upper_nibble_err[i++];
  printf("\t %d", (read_data & 0x4000) >> 14);
  read_data = wr_levl_status->upper_nibble_err[i++];
  printf("\t %d", (read_data & 0x4000) >> 14);
  read_data = wr_levl_status->upper_nibble_err[i++];
  printf("\t %d", (read_data & 0x4000) >> 14);
  read_data = wr_levl_status->upper_nibble_err[i++];
  printf("\t %d", (read_data & 0x4000) >> 14);
  read_data = wr_levl_status->upper_nibble_err[i++];
  printf("\t %d", (read_data & 0x4000) >> 14);
  read_data = wr_levl_status->upper_nibble_err[i++];
  printf("\t %d", (read_data & 0x4000) >> 14);
  read_data = wr_levl_status->upper_nibble_err[i++];
  printf("\t %d", (read_data & 0x4000) >> 14);
  read_data = wr_levl_status->upper_nibble_err[i++];
  printf("\t %d\n", (read_data & 0x4000) >> 14);
}

void print_read_gate_training_status(
    uint32_t instance,
    struct ddr_read_gate_training_status *rd_gate_tr_status) {
  uint32_t read_data = 0;
  int i = 0;

  if (!rd_gate_tr_status) {
    printf("Null pointer, cannot display structure\r\n");
    return;
  }

  printf("\t\tBYTE# \t\t\t\t 0 \t 1 \t 2 \t 3 \t 4 \t 5 \t 6 \t 7 \t 8\n");
  read_data = rd_gate_tr_status->lower_nibble_min_err[i++];
  printf("\t\tLOWER NIBBLE MIN ERROR \t\t %d", (read_data & 0x80) >> 7);
  read_data = rd_gate_tr_status->lower_nibble_min_err[i++];
  printf("\t %d", (read_data & 0x80) >> 7);
  read_data = rd_gate_tr_status->lower_nibble_min_err[i++];
  printf("\t %d", (read_data & 0x80) >> 7);
  read_data = rd_gate_tr_status->lower_nibble_min_err[i++];
  printf("\t %d", (read_data & 0x80) >> 7);
  read_data = rd_gate_tr_status->lower_nibble_min_err[i++];
  printf("\t %d", (read_data & 0x80) >> 7);
  read_data = rd_gate_tr_status->lower_nibble_min_err[i++];
  printf("\t %d", (read_data & 0x80) >> 7);
  read_data = rd_gate_tr_status->lower_nibble_min_err[i++];
  printf("\t %d", (read_data & 0x80) >> 7);
  read_data = rd_gate_tr_status->lower_nibble_min_err[i++];
  printf("\t %d", (read_data & 0x80) >> 7);
  read_data = rd_gate_tr_status->lower_nibble_min_err[i++];
  printf("\t %d\n", (read_data & 0x80) >> 7);

  i = 0;
  read_data = rd_gate_tr_status->lower_nibble_max_err[i++];
  printf("\t\tLOWER NIBBLE MAX ERROR \t\t %d", (read_data & 0x100) >> 8);
  read_data = rd_gate_tr_status->lower_nibble_max_err[i++];
  printf("\t %d", (read_data & 0x100) >> 8);
  read_data = rd_gate_tr_status->lower_nibble_max_err[i++];
  printf("\t %d", (read_data & 0x100) >> 8);
  read_data = rd_gate_tr_status->lower_nibble_max_err[i++];
  printf("\t %d", (read_data & 0x100) >> 8);
  read_data = rd_gate_tr_status->lower_nibble_max_err[i++];
  printf("\t %d", (read_data & 0x100) >> 8);
  read_data = rd_gate_tr_status->lower_nibble_max_err[i++];
  printf("\t %d", (read_data & 0x100) >> 8);
  read_data = rd_gate_tr_status->lower_nibble_max_err[i++];
  printf("\t %d", (read_data & 0x100) >> 8);
  read_data = rd_gate_tr_status->lower_nibble_max_err[i++];
  printf("\t %d", (read_data & 0x100) >> 8);
  read_data = rd_gate_tr_status->lower_nibble_max_err[i++];
  printf("\t %d\n", (read_data & 0x100) >> 8);

  i = 0;
  read_data = rd_gate_tr_status->upper_nibble_min_err[i++];
  printf("\t\tUPPER NIBBLE MIN ERROR \t\t %d", (read_data & 0x200) >> 9);
  read_data = rd_gate_tr_status->upper_nibble_min_err[i++];
  printf("\t %d", (read_data & 0x200) >> 9);
  read_data = rd_gate_tr_status->upper_nibble_min_err[i++];
  printf("\t %d", (read_data & 0x200) >> 9);
  read_data = rd_gate_tr_status->upper_nibble_min_err[i++];
  printf("\t %d", (read_data & 0x200) >> 9);
  read_data = rd_gate_tr_status->upper_nibble_min_err[i++];
  printf("\t %d", (read_data & 0x200) >> 9);
  read_data = rd_gate_tr_status->upper_nibble_min_err[i++];
  printf("\t %d", (read_data & 0x200) >> 9);
  read_data = rd_gate_tr_status->upper_nibble_min_err[i++];
  printf("\t %d", (read_data & 0x200) >> 9);
  read_data = rd_gate_tr_status->upper_nibble_min_err[i++];
  printf("\t %d", (read_data & 0x200) >> 9);
  read_data = rd_gate_tr_status->upper_nibble_min_err[i++];
  printf("\t %d\n", (read_data & 0x200) >> 9);

  i = 0;
  read_data = rd_gate_tr_status->upper_nibble_max_err[i++];
  printf("\t\tUPPER NIBBLE MAX ERROR \t\t %d", (read_data & 0x400) >> 10);
  read_data = rd_gate_tr_status->upper_nibble_max_err[i++];
  printf("\t %d", (read_data & 0x400) >> 10);
  read_data = rd_gate_tr_status->upper_nibble_max_err[i++];
  printf("\t %d", (read_data & 0x400) >> 10);
  read_data = rd_gate_tr_status->upper_nibble_max_err[i++];
  printf("\t %d", (read_data & 0x400) >> 10);
  read_data = rd_gate_tr_status->upper_nibble_max_err[i++];
  printf("\t %d", (read_data & 0x400) >> 10);
  read_data = rd_gate_tr_status->upper_nibble_max_err[i++];
  printf("\t %d", (read_data & 0x400) >> 10);
  read_data = rd_gate_tr_status->upper_nibble_max_err[i++];
  printf("\t %d", (read_data & 0x400) >> 10);
  read_data = rd_gate_tr_status->upper_nibble_max_err[i++];
  printf("\t %d", (read_data & 0x400) >> 10);
  read_data = rd_gate_tr_status->upper_nibble_max_err[i++];
  printf("\t %d\n", (read_data & 0x400) >> 10);
}

void print_ddr_training_status(
    uint32_t instance, struct ddr_dimm_training_status *dimm_tr_status) {

  if (!dimm_tr_status) {
    printf("Null pointer, cannot display structure\r\n");
    return;
  }

  printf("DDR%d TRAINING STATUS: \n", instance);
  printf("\tWRITE LEVELLING STATUS: \n");
  print_write_levelling_status(instance, &dimm_tr_status->wr_levl_status);
  printf("\n\tREAD GATE TRAINING STATUS: \n");
  print_read_gate_training_status(instance, &dimm_tr_status->rd_gate_tr_status);
}

void print_margin_vref_low_high(
    uint32_t instance, struct ddr_dimm_training_status *dimm_tr_status) {
  int i = 0, j = 0;
  float vref_low_volt, vref_high_volt;

  if (!dimm_tr_status) {
    printf("Null pointer, cannot display structure\r\n");
    return;
  }

  printf("DDR%d MARGIN VALUES:\n", instance);
  printf("\tREAD LEVEL VREF: \n");
  printf("\t\t					 VREF_LOW   VREF_LOW_VOLT(mV)  "
         " VREF_HIGH   VREF_HIGH_VOLT(mV)   VREF_MARGIN(mV)\n");
  for (i = 0; i < 9; i++) {
    printf("\t\tSlice%d Lower Nibble:   ", i);
    vref_low_volt = dimm_tr_status->vref_data.lower_nibble_vref_low_volt[i];
    printf("%04.2f			  ", vref_low_volt);
    vref_high_volt = dimm_tr_status->vref_data.lower_nibble_vref_high_volt[i];
    printf("%04.2f			", vref_high_volt);
    printf("%0.2f\n", (vref_high_volt - vref_low_volt));

    printf("\t\tSlice%d Upper Nibble:   ", i);
    vref_low_volt = dimm_tr_status->vref_data.upper_nibble_vref_low_volt[i];
    printf("%04.2f			  ", vref_low_volt);
    vref_high_volt = dimm_tr_status->vref_data.upper_nibble_vref_high_volt[i];
    printf("%04.2f			", vref_high_volt);
    printf("%0.2f\n\n", (vref_high_volt - vref_low_volt));
  }
  printf("\tWRITE DQ LEVEL VREF: \n");
  printf("\t\t					 VREF_LOW   VREF_LOW_VOLT(mV)  "
         " VREF_HIGH   VREF_HIGH_VOLT(mV)   VREF_MARGIN(mV)\n");
  for (i = 0; i < 18; i++) {
    printf("\t\tCS0 Device%d :\t", i);
    vref_low_volt = dimm_tr_status->wdq_vref_data.vref_low_volt[i];
    printf("%04.1f			", vref_low_volt);
    vref_high_volt = dimm_tr_status->wdq_vref_data.vref_high_volt[i];
    printf("%04.1f			  ", vref_high_volt);
    printf("%0.1f		  \n", (vref_high_volt - vref_low_volt));
  }
  for (j = 1; j < 4; j++) {
    for (i = 0; i < 18; i++) {
      printf("\t\tCS%d Device%d :\t", j, i);
      vref_low_volt = dimm_tr_status->wdq_vref_data_cs.vref_low_volt_cs[j][i];
      printf("%04.1f			", vref_low_volt);
      vref_high_volt = dimm_tr_status->wdq_vref_data_cs.vref_high_volt_cs[j][i];
      printf("%04.1f			  ", vref_high_volt);
      printf("%0.1f		   \n", (vref_high_volt - vref_low_volt));
    }
    printf("\n");
  }
}

void print_margin_rdlvl_delay_window(
    int instance, struct ddr_dimm_training_status *dimm_tr_status) {
  uint32_t te_delay_data = 0, le_delay_data = 0;
  int i = 0, j = 0;
  float te_delay_time, le_delay_time;

  if (!dimm_tr_status) {
    printf("Null pointer, cannot display structure\r\n");
    return;
  }

  printf("DDR%d Margin Delays: \n", instance);
  printf("\tREAD DQSLEVEL RISE DELAY WINDOW: \n");
  printf("\t\t		   TE_DATA   TE_DELAY(ns)   LE_DATA   LE_DELAY(ns)   "
         "RD_RISE_DELAY(ns)\n");
  for (j = 0; j < 9; j++) {
    for (i = 0; i < 8; i++) {
      printf("\t\tSLICE%d BIT%d   ", j, i);
      te_delay_data = dimm_tr_status->rddqslvl_rise_data.te_delay_data[j][i];
      printf("%d	  ", te_delay_data);
      te_delay_time = dimm_tr_status->rddqslvl_rise_data.te_delay_time[j][i];
      printf("%0.03f		  ", te_delay_time);
      le_delay_data = dimm_tr_status->rddqslvl_rise_data.le_delay_data[j][i];
      printf("%02d		", le_delay_data);
      le_delay_time = dimm_tr_status->rddqslvl_rise_data.le_delay_time[j][i];
      printf("%0.03f			", le_delay_time);
      printf("%0.03f\n", (te_delay_time - le_delay_time));
    }
    printf("\n");
  }
  printf("\tREAD DQSLEVEL FALL DELAY WINDOW: \n");
  printf("\t\t		   TE_DATA   TE_DELAY(ns)   LE_DATA   LE_DELAY(ns)   "
         "RD_FALL_DELAY(ns)\n");
  for (j = 0; j < 9; j++) {
    for (i = 0; i < 8; i++) {
      printf("\t\tSLICE%d BIT%d   ", j, i);
      te_delay_data = dimm_tr_status->rddqslvl_fall_data.te_delay_data[j][i];
      printf("%d	  ", te_delay_data);
      te_delay_time = dimm_tr_status->rddqslvl_fall_data.te_delay_time[j][i];
      printf("%0.03f		  ", te_delay_time);
      le_delay_data = dimm_tr_status->rddqslvl_fall_data.le_delay_data[j][i];
      printf("%02d		", le_delay_data);
      le_delay_time = dimm_tr_status->rddqslvl_fall_data.le_delay_time[j][i];
      printf("%0.03f			", le_delay_time);
      printf("%0.03f\n", (te_delay_time - le_delay_time));
    }
    printf("\n");
  }
}

void print_margin_wrdqlvl_delay_window(
    int instance, struct ddr_dimm_training_status *dimm_tr_status) {
  uint32_t te_delay_data = 0, le_delay_data = 0;
  int i = 0, j = 0;
  float te_delay_time, le_delay_time;

  if (!dimm_tr_status) {
    printf("Null pointer, cannot display structure\r\n");
    return;
  }

  printf("\tWRITE DQLEVEL DELAY WINDOW: \n");
  printf("\t\t		   TE_DATA   TE_DELAY(ns)   LE_DATA   LE_DELAY(ns)   "
         "WRDQLVL_DELAY(ns)\n");
  for (j = 0; j < 9; j++) {
    for (i = 0; i < 8; i++) {
      printf("\t\tSLICE%d BIT%d   ", j, i);
      te_delay_data = dimm_tr_status->wrdqlvl_delay_data.te_delay_data[j][i];
      printf("%d	  ", te_delay_data);
      te_delay_time = dimm_tr_status->wrdqlvl_delay_data.te_delay_time[j][i];
      printf("%0.03f		  ", te_delay_time);
      le_delay_data = dimm_tr_status->wrdqlvl_delay_data.le_delay_data[j][i];
      printf("%02d		", le_delay_data);
      le_delay_time = dimm_tr_status->wrdqlvl_delay_data.le_delay_time[j][i];
      printf("%0.03f			", le_delay_time);
      printf("%0.03f\n", (te_delay_time - le_delay_time));
    }
    printf("\n");
  }
}

void print_err_status(int instance,
                      struct ddr_dimm_training_status *dimm_tr_status) {
  uint32_t read_data = dimm_tr_status->err_status;

  if (!dimm_tr_status) {
    printf("Null pointer, cannot display structure\r\n");
    return;
  }

  printf("DIMM %d Tranining status\n", instance);
  printf("\tWRLVL_ERR	 = %d\n", (read_data >> 4) & 0x1);
  printf("\tGTLVL_ERR	 = %d\n", (read_data >> 3) & 0x1);
  printf("\tRDLVL_ERR	 = %d\n", (read_data >> 2) & 0x1);
  printf("\tWDQLVL_ERR	= %d\n", (read_data >> 5) & 0x1);
  printf("\tCA PARTIY ERR = %d\n", (read_data >> 1) & 0x1);
}
