/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2007 Pierre Habouzit. All rights reserved. */

/* originally copied from perf and git */

#include "util.h"
#include <stdbool.h>
#include <stdint.h>

#ifndef __PARSE_OPTION_H__
#define __PARSE_OPTION_H__

#ifdef __cplusplus
extern "C" {
#endif

enum parse_opt_type {
  /* special types */
  OPTION_END,
  OPTION_ARGUMENT,
  OPTION_GROUP,
  /* options with no arguments */
  OPTION_BIT,
  OPTION_BOOLEAN,
  OPTION_INCR,
  OPTION_SET_UINT,
  OPTION_SET_PTR,
  /* options with arguments (usually) */
  OPTION_STRING,
  OPTION_INTEGER,
  OPTION_LONG,
  OPTION_CALLBACK,
  OPTION_U64,
  OPTION_UINTEGER,
  OPTION_FILENAME,
};

struct option {
  enum parse_opt_type type;
  int short_name;
  const char *long_name;
  void *value;
  const char *argh;
  const char *help;

  bool *set;
};

#define check_vtype(v, type)                                                   \
  (BUILD_BUG_ON_ZERO(!__builtin_types_compatible_p(typeof(v), type)) + v)

#define OPT_END()                                                              \
  { .type = OPTION_END }
#define OPT_ARGUMENT(l, h)                                                     \
  { .type = OPTION_ARGUMENT, .long_name = (l), .help = (h) }
#define OPT_GROUP(h)                                                           \
  { .type = OPTION_GROUP, .help = (h) }
#define OPT_BIT(s, l, v, h, b)                                                 \
  {                                                                            \
    .type = OPTION_BIT, .short_name = (s), .long_name = (l),                   \
    .value = check_vtype(v, int *), .help = (h), .defval = (b)                 \
  }
#define OPT_BOOLEAN(s, l, v, h)                                                \
  {                                                                            \
    .type = OPTION_BOOLEAN, .short_name = (s), .long_name = (l),               \
    .value = check_vtype(v, bool *), .help = (h)                               \
  }
#define OPT_INCR(s, l, v, h)                                                   \
  {                                                                            \
    .type = OPTION_INCR, .short_name = (s), .long_name = (l),                  \
    .value = check_vtype(v, int *), .help = (h)                                \
  }
#define OPT_INTEGER(s, l, v, h)                                                \
  {                                                                            \
    .type = OPTION_INTEGER, .short_name = (s), .long_name = (l),               \
    .value = check_vtype(v, int *), .help = (h)                                \
  }
#define OPT_UINTEGER(s, l, v, h)                                               \
  {                                                                            \
    .type = OPTION_UINTEGER, .short_name = (s), .long_name = (l),              \
    .value = check_vtype(v, unsigned int *), .help = (h)                       \
  }
#define OPT_LONG(s, l, v, h)                                                   \
  {                                                                            \
    .type = OPTION_LONG, .short_name = (s), .long_name = (l),                  \
    .value = check_vtype(v, long *), .help = (h)                               \
  }
#define OPT_U64(s, l, v, h)                                                    \
  {                                                                            \
    .type = OPTION_U64, .short_name = (s), .long_name = (l),                   \
    .value = check_vtype(v, uint64_t *), .help = (h)                           \
  }
#define OPT_STRING(s, l, v, a, h)                                              \
  {                                                                            \
    .type = OPTION_STRING, .short_name = (s), .long_name = (l),                \
    .value = check_vtype(v, const char **), (a), .help = (h)                   \
  }
#define OPT_FILENAME(s, l, v, a, h)                                            \
  {                                                                            \
    .type = OPTION_FILENAME, .short_name = (s), .long_name = (l),              \
    .value = check_vtype(v, const char **), (a), .help = (h)                   \
  }

/* parse_options() will filter out the processed options and leave the
 * non-option argments in argv[].
 * Returns the number of arguments left in argv[].
 */
extern int parse_options(int argc, const char **argv,
                         const struct option *options,
                         const char *const usagestr[], int flags);

extern void usage_with_options(const char *const *usagestr,
                               const struct option *options);

void uuid_unparse(uint8_t *uuid_arr, char *uuid_str);

void uuid_parse(const char *uuid_str, uint8_t *uuid_arr);

#ifdef __cplusplus
}
#endif

#endif /* __PARSE_OPTION_H__ */
