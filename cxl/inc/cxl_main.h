// SPDX-License-Identifier: LGPL-2.1-or-later
#ifndef __CXL_MAIN_H__
#define __CXL_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

#define STR_HELP "help"

/* standard commands */
#define STR_IDENTIFY "id-cmd"
#define STR_GET_SUPPORTED_LOGS "get-supported-logs"
#define STR_GET_LOG "get-log"
#define STR_GET_ALERT_CONFIG "get-alert-config"
#define STR_SET_ALERT_CONFIG "set-alert-config"
#define STR_GET_HEALTH_INFO "get-health-info"
#define STR_GET_FW_INFO "get-fw-info"
#define STR_UPDATE_FW "update-fw"
#define STR_GET_TIMESTAMP "get-timestamp"
#define STR_SET_TIMESTAMP "set-timestamp"
#define STR_GET_EVENT_RECORDS "get-event-records"
#define STR_CLEAR_EVENT_RECORDS "clear-event-records"
#define STR_GET_EVENT_INTERRUPT_POLICY "get-event-interrupt-policy"
#define STR_SET_EVENT_INTERRUPT_POLICY "set-event-interrupt-policy"

#ifdef __cplusplus
}
#endif
#endif /* __CXL_MAIN_H__ */
