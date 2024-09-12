// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#ifndef __VENDOR_TYPES_H__
#define __VENDOR_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

enum {
  VENDOR_CMD_DDR_DIMM_MGMT = 0xC5,
  VENDOR_CMD_OEM_MGMT = 0xCD,
#define OEM_HBO_STATUS 0x00
#define TRANSFER_FW 0x01
#define ACTIVATE_FW 0x02
#define GET_OS_INFO 0x03
#define TRANSFER_OS 0x04

  VENDOR_CMD_HEALTH_MGMT = 0xCE,
#define HEALTH_COUNTERS_CLEAR 0x00
#define HEALTH_COUNTERS_GET 0x01

  VENDOR_CMD_OTHERS = 0xFB,
#define DDR_PAGE_SELECT_SET 0x2A
#define DDR_PAGE_SELECT_GET 0x2B
};

#ifdef __cplusplus
}
#endif

#endif /* __VENDOR_TYPES_H__ */
