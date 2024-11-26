// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* Helper functions for ltssm states */
char ltssm_state_name[][20] = {"DETECT_QUIET",
                               "DETECT_ACT",
                               "POLL_ACTIVE",
                               "POLL_COMPLIANCE",
                               "POLL_CONFIG",
                               "PRE_DETECT_QUIET",
                               "DETECT_WAIT",
                               "CFG_LINKWD_START",
                               "CFG_LINKWD_ACEPT",
                               "CFG_LANENUM_WAIT",
                               "CFG_LANENUM_ACEPT",
                               "CFG_COMPLETE",
                               "CFG_IDLE",
                               "RCVRY_LOCK",
                               "RCVRY_SPEED",
                               "RCVRY_RCVRCFG",
                               "RCVRY_IDLE",
                               "L0",
                               "L0S",
                               "L123_SEND_EIDLE",
                               "L1_IDLE",
                               "L2_IDLE",
                               "L2_WAKE",
                               "DISABLED_ENTRY",
                               "DISABLED_IDLE",
                               "DISABLED",
                               "LPBK_ENTRY",
                               "LPBK_ACTIVE",
                               "LPBK_EXIT",
                               "LPBK_EXIT_TIMEOUT",
                               "HOT_RESET_ENTRY",
                               "HOT_RESET",
                               "RCVRY_EQ0",
                               "RCVRY_EQ1",
                               "RCVRY_EQ2",
                               "RCVRY_EQ3"};
