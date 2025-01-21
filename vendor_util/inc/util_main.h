/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2015-2020 Intel Corporation. All rights reserved. */
/* Copyright (C) 2006 Linus Torvalds. All rights reserved. */

/* originally copied from perf and git */

#ifndef __UTIL_MAIN_H__
#define __UTIL_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * ARRAY_SIZE - get the number of elements in a visible array
 * @arr: the array whose size you want.
 *
 * This does not work on pointers, or arrays declared as [], or
 * function parameters.  With correct compiler support, such usage
 * will cause a build error (see build_assert).
 */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct cxlmi_ctx;

struct cmd_struct {
  const char *cmd;
  int (*c_fn)(int argc, const char **argv, struct cxlmi_ctx *ctx);
};

int main_handle_options(const char ***argv, int *argc, const char *usage_msg,
                        struct cmd_struct *cmds, int num_cmds);
void main_handle_internal_command(int argc, const char **argv, void *ctx,
                                  struct cmd_struct *cmds, int num_cmds);

#ifdef __cplusplus
}
#endif

#endif /* __UTIL_MAIN_H__ */
