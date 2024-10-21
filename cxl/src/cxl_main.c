// SPDX-License-Identifier: LGPL-2.1-or-later

/* std includes */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* libcxlmi includes */
#include <libcxlmi.h>

/* vendor includes */
#include "cxl_cmd.h"
#include "cxl_main.h"
#include <util_main.h>
#include <vendor_commands.h>

/* List of support commands and respective handlers */
struct cmd_struct commands[] = {
    {STR_HELP, cmd_print_help},
    {STR_IDENTIFY, cmd_identify},
    {STR_GET_SUPPORTED_LOGS, cmd_get_supported_logs},
    {STR_GET_LOG, cmd_get_log},
    {STR_GET_ALERT_CONFIG, cmd_get_alert_config},
    {STR_SET_ALERT_CONFIG, cmd_set_alert_config},
    {STR_GET_HEALTH_INFO, cmd_get_health_info},
    {STR_GET_FW_INFO, cmd_get_fw_info},
    {STR_GET_TIMESTAMP, cmd_get_timestamp},
    {STR_SET_TIMESTAMP, cmd_set_timestamp},
    {STR_GET_EVENT_RECORDS, cmd_get_event_records},
    {STR_CLEAR_EVENT_RECORDS, cmd_clear_event_records},
    {STR_GET_EVENT_INTERRUPT_POLICY, cmd_get_event_interrupt_policy},
    {STR_SET_EVENT_INTERRUPT_POLICY, cmd_set_event_interrupt_policy},

};

const char cxl_usage_string[] = "cxl COMMAND [ARGS]";

int cmd_print_help(int argc, const char **argv, struct cxlmi_ctx *ctx) {
  fprintf(stderr, "%s\n", cxl_usage_string);
  fprintf(stderr, "Usage: %s <cmd> <device>\n", "cxl");
  fprintf(stderr, "<device> device_name (ex: mem0)\n");
  fprintf(stderr, "%s --list-cmds to see all available commands\n", "cxl");

  return 0;
}

int main(int argc, const char **argv) {
  struct cxlmi_ctx *ctx = NULL;
  struct cxlmi_endpoint *ep = NULL;
  int rc = EXIT_FAILURE;

  if (argc < 2) {
    cmd_print_help(argc, argv, NULL);
    goto exit;
  }

  ctx = cxlmi_new_ctx(stdout, DEFAULT_LOGLEVEL);
  if (!ctx) {
    fprintf(stderr, "cannot create new context object\n");
    goto exit;
  }

  /* Look for flags.. */
  argv++;
  argc--;
  main_handle_options(&argv, &argc, cxl_usage_string, commands,
                      ARRAY_SIZE(commands));

  /* NOTE: only 1 endpoint, but might add more */
  if (argc > 1) {
    if (!strncmp(argv[1], "mem", strlen("mem"))) {
      /* TODO: Move this logic to cmd_action
      to support iterating all endpoints like cxlcli tool*/
      ep = cxlmi_open(ctx, argv[1]);
      if (!ep) {
        fprintf(stderr, "cannot open '%s' endpoint\n", argv[1]);
        goto exit_free_ctx;
      }
    }
  }
  if (argc < 1) {
    cmd_print_help(argc, argv, NULL);
    goto exit_free_ctx;
  }

  main_handle_internal_command(argc, argv, ctx, commands, ARRAY_SIZE(commands));

  if (ep)
    cxlmi_close(ep);
exit_free_ctx:
  if (ctx)
    cxlmi_free_ctx(ctx);
exit:
  return rc;
}
