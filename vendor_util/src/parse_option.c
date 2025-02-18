// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2007 Pierre Habouzit. All rights reserved.

/* originally copied from perf and git */

#include "parse_option.h"
#include "strbuf.h"
#include <stdio.h>
#include <sys/types.h>

enum {
  PARSE_OPT_HELP = -1,
  PARSE_OPT_DONE,
  PARSE_OPT_LIST_OPTS,
  PARSE_OPT_LIST_SUBCMDS,
  PARSE_OPT_UNKNOWN,
};

enum parse_opt_flags {
  PARSE_OPT_KEEP_DASHDASH = 1,
  PARSE_OPT_STOP_AT_NON_OPTION = 2,
  PARSE_OPT_KEEP_ARGV0 = 4,
  PARSE_OPT_KEEP_UNKNOWN = 8,
  PARSE_OPT_NO_INTERNAL_HELP = 16,
};

enum parse_opt_option_flags {
  PARSE_OPT_OPTARG = 1,
  PARSE_OPT_NOARG = 2,
  PARSE_OPT_NONEG = 4,
  PARSE_OPT_HIDDEN = 8,
  PARSE_OPT_LASTARG_DEFAULT = 16,
};

/*
 * It's okay for the caller to consume argv/argc in the usual way.
 * Other fields of that structure are private to parse-options and should not
 * be modified in any way.
 */
struct parse_opt_ctx_t {
  const char **argv;
  const char **out;
  int argc, cpidx;
  const char *opt;
  int flags;
  const char *prefix;
};

#define OPT_SHORT 1
#define OPT_UNSET 2

void uuid_unparse(uint8_t *uuid_arr, char *uuid_str) {
  int j = 0;
  if (!uuid_arr || !uuid_str) {
    printf("NULL ptr passed to %s\r\n", __func__);
    return;
  }
  static char const hexdigits_lower[] = "0123456789abcdef";
  for (int i = 0; i < 0x10; i++) {
    if (i == 4 || i == 6 || i == 8 || i == 10) {
      uuid_str[j++] = '-';
    }
    uuid_str[j++] = hexdigits_lower[uuid_arr[i] >> 4];
    uuid_str[j++] = hexdigits_lower[uuid_arr[i] & 0xF];
  }
  uuid_str[j] = '\0';
}

void uuid_parse(const char *uuid_str, uint8_t *uuid_arr) {
  int j = 0;
  if (!uuid_arr || !uuid_str) {
    printf("NULL ptr passed to %s\r\n", __func__);
    return;
  }
  for (int i = 0; i < 36;) {
    char digit[3];
    if (i == 8 || i == 13 || i == 18 || i == 23) {
      if (uuid_str[i] == '-')
        i++;
      else {
        printf("Unexpected char:%c at %d\n", uuid_str[i], i);
        return;
      }
    }
    digit[0] = uuid_str[i++];
    digit[1] = uuid_str[i++];
    digit[2] = '\0';
    uuid_arr[j++] = strtoul(digit, NULL, 16);
  }
}

#define USAGE_OPTS_WIDTH 24
#define USAGE_GAP 2

static void print_option_help(const struct option *opts, int full) {
  size_t pos;
  int pad;

  if (opts->type == OPTION_GROUP) {
    fputc('\n', stderr);
    if (*opts->help)
      fprintf(stderr, "%s\n", opts->help);
    return;
  }

  pos = fprintf(stderr, "    ");
  if (opts->short_name)
    pos += fprintf(stderr, "-%c", opts->short_name);
  else
    pos += fprintf(stderr, "    ");

  if (opts->long_name && opts->short_name)
    pos += fprintf(stderr, ", ");
  if (opts->long_name)
    pos += fprintf(stderr, "--%s", opts->long_name);

  switch (opts->type) {
  case OPTION_ARGUMENT:
    break;
  case OPTION_LONG:
  case OPTION_U64:
  case OPTION_INTEGER:
  case OPTION_UINTEGER:
    pos += fprintf(stderr, " <n>");
    break;
  case OPTION_CALLBACK:
  case OPTION_FILENAME:
  case OPTION_STRING:
    if (opts->argh) {
#if 0 // Argument is not option in this lib usage
      if (opts->flags & PARSE_OPT_OPTARG)
        if (opts->long_name)
          pos += fprintf(stderr, "[=<%s>]", opts->argh);
        else
          pos += fprintf(stderr, "[<%s>]", opts->argh);
      else
#endif
      pos += fprintf(stderr, " <%s>", opts->argh);
    } else {
#if 0 // Argument is not option in this lib usage
      if (opts->flags & PARSE_OPT_OPTARG)
        if (opts->long_name)
          pos += fprintf(stderr, "[=...]");
        else
          pos += fprintf(stderr, "[...]");
      else
#endif
      pos += fprintf(stderr, " ...");
    }
    break;
  default: /* OPTION_{BIT,BOOLEAN,SET_UINT,SET_PTR} */
  case OPTION_END:
  case OPTION_GROUP:
  case OPTION_BIT:
  case OPTION_BOOLEAN:
  case OPTION_INCR:
  case OPTION_SET_UINT:
  case OPTION_SET_PTR:
    break;
  }

  if (pos <= USAGE_OPTS_WIDTH)
    pad = USAGE_OPTS_WIDTH - pos;
  else {
    fputc('\n', stderr);
    pad = USAGE_OPTS_WIDTH;
  }
  fprintf(stderr, "%*s%s\n", pad + USAGE_GAP, "", opts->help);
}

int usage_with_options_internal(const char *const *usagestr,
                                const struct option *opts, int full) {
  if (!usagestr)
    return PARSE_OPT_HELP;

  fprintf(stderr, "\n usage: %s\n", *usagestr++);
  while (*usagestr && **usagestr)
    fprintf(stderr, "    or: %s\n", *usagestr++);
  while (*usagestr) {
    fprintf(stderr, "%s%s\n", **usagestr ? "    " : "", *usagestr);
    usagestr++;
  }

  if (opts->type != OPTION_GROUP)
    fputc('\n', stderr);

  for (; opts->type != OPTION_END; opts++)
    print_option_help(opts, full);

  fputc('\n', stderr);

  return PARSE_OPT_HELP;
}

int parse_options_usage(const char *const *usagestr, const struct option *opts,
                        const char *optstr, bool short_opt) {
  if (!usagestr)
    goto opt;

  fprintf(stderr, "\n usage: %s\n", *usagestr++);
  while (*usagestr && **usagestr)
    fprintf(stderr, "    or: %s\n", *usagestr++);
  while (*usagestr) {
    fprintf(stderr, "%s%s\n", **usagestr ? "    " : "", *usagestr);
    usagestr++;
  }
  fputc('\n', stderr);

opt:
  for (; opts->type != OPTION_END; opts++) {
    if (short_opt) {
      if (opts->short_name == *optstr)
        break;
      continue;
    }

    if (opts->long_name == NULL)
      continue;

    if (!prefixcmp(optstr, opts->long_name))
      break;
    if (!prefixcmp(optstr, "no-") && !prefixcmp(optstr + 3, opts->long_name))
      break;
  }

  if (opts->type != OPTION_END)
    print_option_help(opts, 0);

  return PARSE_OPT_HELP;
}

void usage_with_options(const char *const *usagestr,
                        const struct option *opts) {
  usage_with_options_internal(usagestr, opts, 0);
  exit(129);
}

static int opterror(const struct option *opt, const char *reason, int flags) {
  if (flags & OPT_SHORT)
    return error("switch `%c' %s", opt->short_name, reason);
  if (flags & OPT_UNSET)
    return error("option `no-%s' %s", opt->long_name, reason);
  return error("option `%s' %s", opt->long_name, reason);
}

static int get_arg(struct parse_opt_ctx_t *p, const struct option *opt,
                   int flags, const char **arg) {
  if (p->opt) {
    *arg = p->opt;
    p->opt = NULL;
    // } else if ((opt->flags & PARSE_OPT_LASTARG_DEFAULT) &&
    //            (p->argc == 1 || **(p->argv + 1) == '-')) {
    //   *arg = (const char *)opt->defval;
  } else if (p->argc > 1) {
    p->argc--;
    *arg = *++p->argv;
  } else
    return opterror(opt, "requires a value", flags);
  return 0;
}

static int get_value(struct parse_opt_ctx_t *p, const struct option *opt,
                     int flags) {
  const char *s, *arg = NULL;
  const int unset = flags & OPT_UNSET;
  int err;

  if (unset && p->opt)
    return opterror(opt, "takes no value", flags);

  if (!(flags & OPT_SHORT) && p->opt) {
    switch (opt->type) {
    case OPTION_CALLBACK:
      /* FALLTHROUGH */
    case OPTION_BOOLEAN:
    case OPTION_INCR:
    case OPTION_BIT:
    case OPTION_SET_UINT:
    case OPTION_SET_PTR:
      return opterror(opt, "takes no value", flags);
    case OPTION_END:
    case OPTION_ARGUMENT:
    case OPTION_GROUP:
    case OPTION_STRING:
    case OPTION_FILENAME:
    case OPTION_INTEGER:
    case OPTION_UINTEGER:
    case OPTION_LONG:
    case OPTION_U64:
    default:
      break;
    }
  }

  switch (opt->type) {
  case OPTION_BIT:
    // if (unset)
    //   *(int *)opt->value &= ~opt->defval;
    // else
    //   *(int *)opt->value |= opt->defval;
    return 0;

  case OPTION_BOOLEAN:
    *(bool *)opt->value = unset ? false : true;
    if (opt->set)
      *(bool *)opt->set = true;
    return 0;

  case OPTION_INCR:
    *(int *)opt->value = unset ? 0 : *(int *)opt->value + 1;
    return 0;

  case OPTION_SET_UINT:
    //*(unsigned int *)opt->value = unset ? 0 : opt->defval;
    return 0;

  case OPTION_SET_PTR:
    //*(void **)opt->value = unset ? NULL : (void *)opt->defval;
    return 0;

  case OPTION_STRING:
    if (unset)
      *(const char **)opt->value = NULL;
    // else if (opt->flags & PARSE_OPT_OPTARG && !p->opt)
    //   *(const char **)opt->value = (const char *)opt->defval;
    else
      return get_arg(p, opt, flags, (const char **)opt->value);
    return 0;

  case OPTION_FILENAME:
    err = 0;
    if (unset)
      *(const char **)opt->value = NULL;
    // else if (opt->flags & PARSE_OPT_OPTARG && !p->opt)
    //   *(const char **)opt->value = (const char *)opt->defval;
    else
      err = get_arg(p, opt, flags, (const char **)opt->value);

    if (!err)
      fix_filename(p->prefix, (const char **)opt->value);
    return err;
#if 0
  case OPTION_CALLBACK:
    if (unset)
      return (*opt->callback)(opt, NULL, 1) ? (-1) : 0;
    if (opt->flags & PARSE_OPT_NOARG)
      return (*opt->callback)(opt, NULL, 0) ? (-1) : 0;
    if (opt->flags & PARSE_OPT_OPTARG && !p->opt)
      return (*opt->callback)(opt, NULL, 0) ? (-1) : 0;
    if (get_arg(p, opt, flags, &arg))
      return -1;
    return (*opt->callback)(opt, arg, 0) ? (-1) : 0;
#endif
  case OPTION_INTEGER:
    if (unset) {
      *(int *)opt->value = 0;
      return 0;
    }
    // if (opt->flags & PARSE_OPT_OPTARG && !p->opt) {
    //   *(int *)opt->value = opt->defval;
    //   return 0;
    // }
    if (get_arg(p, opt, flags, &arg))
      return -1;
    *(int *)opt->value = strtol(arg, (char **)&s, 10);
    if (*s)
      return opterror(opt, "expects a numerical value", flags);
    return 0;

  case OPTION_UINTEGER:
    if (unset) {
      *(unsigned int *)opt->value = 0;
      return 0;
    }
    // if (opt->flags & PARSE_OPT_OPTARG && !p->opt) {
    //   *(unsigned int *)opt->value = opt->defval;
    //   return 0;
    // }
    if (get_arg(p, opt, flags, &arg))
      return -1;
    *(unsigned int *)opt->value = strtol(arg, (char **)&s, 10);
    if (*s)
      *(unsigned int *)opt->value = strtol(arg, (char **)&s, 16);
    if (*s)
      return opterror(opt, "expects a numerical value", flags);
    return 0;

  case OPTION_LONG:
    if (unset) {
      *(long *)opt->value = 0;
      return 0;
    }
    // if (opt->flags & PARSE_OPT_OPTARG && !p->opt) {
    //   *(long *)opt->value = opt->defval;
    //   return 0;
    // }
    if (get_arg(p, opt, flags, &arg))
      return -1;
    *(long *)opt->value = strtol(arg, (char **)&s, 10);
    if (*s)
      return opterror(opt, "expects a numerical value", flags);
    return 0;

  case OPTION_U64:
    if (unset) {
      *(uint64_t *)opt->value = 0;
      return 0;
    }
    // if (opt->flags & PARSE_OPT_OPTARG && !p->opt) {
    //   *(uint64_t *)opt->value = opt->defval;
    //   return 0;
    // }
    if (get_arg(p, opt, flags, &arg))
      return -1;
    *(uint64_t *)opt->value = strtoull(arg, (char **)&s, 10);
    if (*s)
      *(uint64_t *)opt->value = strtoull(arg, (char **)&s, 16);
    if (*s)
      return opterror(opt, "expects a numerical value", flags);
    return 0;

  case OPTION_END:
  case OPTION_ARGUMENT:
  case OPTION_GROUP:
  default:
    die("should not happen, someone must be hit on the forehead");
  }
}

static int parse_short_opt(struct parse_opt_ctx_t *p,
                           const struct option *options) {
  for (; options->type != OPTION_END; options++) {
    if (options->short_name == *p->opt) {
      p->opt = p->opt[1] ? p->opt + 1 : NULL;
      return get_value(p, options, OPT_SHORT);
    }
  }
  return -2;
}

static int parse_long_opt(struct parse_opt_ctx_t *p, const char *arg,
                          const struct option *options) {
  const char *arg_end = strchr(arg, '=');
  const struct option *abbrev_option = NULL, *ambiguous_option = NULL;
  int abbrev_flags = 0, ambiguous_flags = 0;

  if (!arg_end)
    arg_end = arg + strlen(arg);

  for (; options->type != OPTION_END; options++) {
    const char *rest;
    int flags = 0;

    if (!options->long_name)
      continue;

    rest = skip_prefix(arg, options->long_name);
    if (options->type == OPTION_ARGUMENT) {
      if (!rest)
        continue;
      if (*rest == '=')
        return opterror(options, "takes no value", flags);
      if (*rest)
        continue;
      p->out[p->cpidx++] = arg - 2;
      return 0;
    }
    if (!rest) {
      if (!prefixcmp(options->long_name, "no-")) {
        /*
         * The long name itself starts with "no-", so
         * accept the option without "no-" so that users
         * do not have to enter "no-no-" to get the
         * negation.
         */
        rest = skip_prefix(arg, options->long_name + 3);
        if (rest) {
          flags |= OPT_UNSET;
          goto match;
        }
        /* Abbreviated case */
        if (!prefixcmp(options->long_name + 3, arg)) {
          flags |= OPT_UNSET;
          goto is_abbreviated;
        }
      }
      /* abbreviated? */
      if (!strncmp(options->long_name, arg, arg_end - arg)) {
      is_abbreviated:
        if (abbrev_option) {
          /*
           * If this is abbreviated, it is
           * ambiguous. So when there is no
           * exact match later, we need to
           * error out.
           */
          ambiguous_option = abbrev_option;
          ambiguous_flags = abbrev_flags;
        }
        if (!(flags & OPT_UNSET) && *arg_end)
          p->opt = arg_end + 1;
        abbrev_option = options;
        abbrev_flags = flags;
        continue;
      }
      /* negated and abbreviated very much? */
      if (!prefixcmp("no-", arg)) {
        flags |= OPT_UNSET;
        goto is_abbreviated;
      }
      /* negated? */
      if (strncmp(arg, "no-", 3))
        continue;
      flags |= OPT_UNSET;
      rest = skip_prefix(arg + 3, options->long_name);
      /* abbreviated and negated? */
      if (!rest && !prefixcmp(options->long_name, arg + 3))
        goto is_abbreviated;
      if (!rest)
        continue;
    }
  match:
    if (*rest) {
      if (*rest != '=')
        continue;
      p->opt = rest + 1;
    }
    return get_value(p, options, flags);
  }

  if (ambiguous_option)
    return error("Ambiguous option: %s "
                 "(could be --%s%s or --%s%s)",
                 arg, (ambiguous_flags & OPT_UNSET) ? "no-" : "",
                 ambiguous_option->long_name,
                 (abbrev_flags & OPT_UNSET) ? "no-" : "",
                 abbrev_option->long_name);
  if (abbrev_option)
    return get_value(p, abbrev_option, abbrev_flags);
  return -2;
}

static void check_typos(const char *arg, const struct option *options) {
  if (strlen(arg) < 3)
    return;

  if (!prefixcmp(arg, "no-")) {
    error("did you mean `--%s` (with two dashes ?)", arg);
    exit(129);
  }

  for (; options->type != OPTION_END; options++) {
    if (!options->long_name)
      continue;
    if (!prefixcmp(options->long_name, arg)) {
      error("did you mean `--%s` (with two dashes ?)", arg);
      exit(129);
    }
  }
}

void parse_options_start(struct parse_opt_ctx_t *ctx, int argc,
                         const char **argv, const char *prefix, int flags) {
  memset(ctx, 0, sizeof(*ctx));
  ctx->argc = argc - 1;
  ctx->argv = argv + 1;
  ctx->out = argv;
  ctx->prefix = prefix;
  ctx->cpidx = ((flags & PARSE_OPT_KEEP_ARGV0) != 0);
  ctx->flags = flags;
  if ((flags & PARSE_OPT_KEEP_UNKNOWN) &&
      (flags & PARSE_OPT_STOP_AT_NON_OPTION))
    die("STOP_AT_NON_OPTION and KEEP_UNKNOWN don't go together");
}

int parse_options_step(struct parse_opt_ctx_t *ctx,
                       const struct option *options,
                       const char *const usagestr[]) {
  int internal_help = !(ctx->flags & PARSE_OPT_NO_INTERNAL_HELP);

  /* we must reset ->opt, unknown short option leave it dangling */
  ctx->opt = NULL;

  for (; ctx->argc; ctx->argc--, ctx->argv++) {
    const char *arg = ctx->argv[0];

    if (*arg != '-' || !arg[1]) {
      if (ctx->flags & PARSE_OPT_STOP_AT_NON_OPTION)
        break;
      ctx->out[ctx->cpidx++] = ctx->argv[0];
      continue;
    }

    if (arg[1] != '-') {
      ctx->opt = arg + 1;
      if (internal_help && *ctx->opt == 'h')
        return usage_with_options_internal(usagestr, options, 0);
      switch (parse_short_opt(ctx, options)) {
      case -1:
        return parse_options_usage(usagestr, options, arg + 1, 1);
      case -2:
        goto unknown;
      default:
        break;
      }
      if (ctx->opt)
        check_typos(arg + 1, options);
      while (ctx->opt) {
        if (internal_help && *ctx->opt == 'h')
          return usage_with_options_internal(usagestr, options, 0);
        arg = ctx->opt;
        switch (parse_short_opt(ctx, options)) {
        case -1:
          return parse_options_usage(usagestr, options, arg, 1);
        case -2:
          /* fake a short option thing to hide the fact that we may have
           * started to parse aggregated stuff
           *
           * This is leaky, too bad.
           */
          ctx->argv[0] = strdup(ctx->opt - 1);
          *(char *)ctx->argv[0] = '-';
          goto unknown;
        default:
          break;
        }
      }
      continue;
    }

    if (!arg[2]) { /* "--" */
      if (!(ctx->flags & PARSE_OPT_KEEP_DASHDASH)) {
        ctx->argc--;
        ctx->argv++;
      }
      break;
    }

    if (internal_help && !strcmp(arg + 2, "help-all"))
      return usage_with_options_internal(usagestr, options, 1);
    if (internal_help && !strcmp(arg + 2, "help"))
      return usage_with_options_internal(usagestr, options, 0);
    if (!strcmp(arg + 2, "list-opts"))
      return PARSE_OPT_LIST_OPTS;
    if (!strcmp(arg + 2, "list-cmds"))
      return PARSE_OPT_LIST_SUBCMDS;
    switch (parse_long_opt(ctx, arg + 2, options)) {
    case -1:
      return parse_options_usage(usagestr, options, arg + 2, 0);
    case -2:
      goto unknown;
    default:
      break;
    }
    continue;
  unknown:
    if (!(ctx->flags & PARSE_OPT_KEEP_UNKNOWN))
      return PARSE_OPT_UNKNOWN;
    ctx->out[ctx->cpidx++] = ctx->argv[0];
    ctx->opt = NULL;
  }
  return PARSE_OPT_DONE;
}

int parse_options_end(struct parse_opt_ctx_t *ctx) {
  memmove(ctx->out + ctx->cpidx, ctx->argv, ctx->argc * sizeof(*ctx->out));
  ctx->out[ctx->cpidx + ctx->argc] = NULL;
  return ctx->cpidx + ctx->argc;
}

static int parse_options_subcommand_prefix(int argc, const char **argv,
                                           const char *prefix,
                                           const struct option *options,
                                           const char *const subcommands[],
                                           const char *usagestr[], int flags) {
  struct parse_opt_ctx_t ctx;

  /* build usage string if it's not provided */
  if (subcommands && !usagestr[0]) {
    struct strbuf buf = STRBUF_INIT;

    strbuf_addf(&buf, "ndctl %s [<options>] {", argv[0]);
    for (int i = 0; subcommands[i]; i++) {
      if (i)
        strbuf_addstr(&buf, "|");
      strbuf_addstr(&buf, subcommands[i]);
    }
    strbuf_addstr(&buf, "}");

    usagestr[0] = strdup(buf.buf);
    strbuf_release(&buf);
  }

  parse_options_start(&ctx, argc, argv, prefix, flags);
  switch (parse_options_step(&ctx, options, usagestr)) {
  case PARSE_OPT_HELP:
    exit(129);
  case PARSE_OPT_DONE:
    break;
  case PARSE_OPT_LIST_OPTS:
    while (options->type != OPTION_END) {
      printf("--%s ", options->long_name);
      options++;
    }
    exit(130);
  case PARSE_OPT_LIST_SUBCMDS:
    if (subcommands)
      for (int i = 0; subcommands[i]; i++)
        printf("%s ", subcommands[i]);
    exit(130);
  default: /* PARSE_OPT_UNKNOWN */
    if (ctx.argv[0][1] == '-') {
      error("unknown option `%s'", ctx.argv[0] + 2);
    } else {
      error("unknown switch `%c'", *ctx.opt);
    }
    usage_with_options(usagestr, options);
  }

  return parse_options_end(&ctx);
}

int parse_options(int argc, const char **argv, const struct option *options,
                  const char *const usagestr[], int flags) {
  return parse_options_subcommand_prefix(argc, argv, NULL, options, NULL,
                                         (const char **)usagestr, flags);
}
