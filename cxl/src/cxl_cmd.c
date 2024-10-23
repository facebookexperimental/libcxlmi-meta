// SPDX-License-Identifier: LGPL-2.1-or-later

/* std includes */
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>

/* libcxlmi includes */
#include <ccan/endian/endian.h>
#include <cxlmi/private.h>
#include <libcxlmi.h>

/* vendor includes */
#include "cxl_cmd.h"
#include <parse_option.h>
#include <util_main.h>
#include <vendor_commands.h>
#include <vendor_types.h>

/* Length of UUID in bytes */
#define LOG_UUID_LEN 0x10

#define member_size(type, member) (sizeof(((type *)0)->member))
#define CXL_MAX_RECORDS_TO_DUMP 20
#define CXL_DRAM_EVENT_GUID "601dcbb3-9c06-4eab-b8af-4e9bfb5c9624"
#define CXL_MEM_MODULE_EVENT_GUID "fe927475-dd59-4339-a586-79bab113b774"

static const uint8_t cel_uuid[LOG_UUID_LEN] = {
    0x0d, 0xa9, 0xc0, 0xb5, 0xbf, 0x41, 0x4b, 0x78,
    0x8f, 0x79, 0x96, 0xb1, 0x62, 0x3b, 0x3f, 0x17};

static const uint8_t ven_dbg[LOG_UUID_LEN] = {
    0x5e, 0x18, 0x19, 0xd9, 0x11, 0xa9, 0x40, 0x0c,
    0x81, 0x1f, 0xd6, 0x07, 0x19, 0x40, 0x3d, 0x86};

#define FW_VERSION_LEN 0x10

int cxl_cmd_identify(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_memdev_identify identify;

  rc = cxlmi_cmd_memdev_identify(ep, NULL, &identify);
  if (!rc) {
    printf("Identify payload info: %s\n", get_devname(ep));
    printf("    out size: 0x%lx\n", sizeof(identify));

    printf("%s info\n", get_devname(ep));
    printf("    fw revision: ");
    for (int i = 0; i < FW_VERSION_LEN; ++i)
      printf("%02x ", identify.fw_revision[i]);
    printf("\n");
    printf("    total_capacity: %lu MB (%lu GB)\n",
           le64_to_cpu(identify.total_capacity),
           (le64_to_cpu(identify.total_capacity)) / 4);
    printf("    volatile_capacity: %lu MB (%lu GB)\n",
           le64_to_cpu(identify.volatile_capacity),
           (le64_to_cpu(identify.volatile_capacity)) / 4);
    printf("    persistent_capacity: %lu MB (%lu GB)\n",
           le64_to_cpu(identify.persistent_capacity),
           (le64_to_cpu(identify.persistent_capacity)) / 4);
    printf("    partition_align: %lu MB (%lu GB)\n",
           le64_to_cpu(identify.partition_align),
           (le64_to_cpu(identify.partition_align)) / 4);
    printf("    info_event_log_size: %d\n",
           le16_to_cpu(identify.info_event_log_size));
    printf("    warning_event_log_size: %d\n",
           le16_to_cpu(identify.warning_event_log_size));
    printf("    failure_event_log_size: %d\n",
           le16_to_cpu(identify.failure_event_log_size));
    printf("    fatal_event_log_size: %d\n",
           le16_to_cpu(identify.fatal_event_log_size));
    printf("    lsa_size: %d\n", le32_to_cpu(identify.lsa_size));
    for (int i = 0; i < 3; ++i)
      printf("    poison_list_max_mer[%d]: %d\n", i,
             identify.poison_list_max_mer[i]);
    printf("    inject_poison_limit: %d\n",
           le16_to_cpu(identify.inject_poison_limit));
    printf("    poison_caps: %d\n", identify.poison_caps);
    printf("    qos_telemetry_caps: %d\n", identify.qos_telemetry_caps);
  }

  return rc;
}

int cxl_cmd_get_supported_logs(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_get_supported_logs *gsl;

  gsl = calloc(1,
               sizeof(*gsl) + CXLMI_MAX_SUPPORTED_LOGS * sizeof(*gsl->entries));
  if (!gsl) {
    printf("Failed to allocate memory\r\n");
    return -ENOMEM;
  }

  rc = cxlmi_cmd_get_supported_logs(ep, NULL, gsl);
  if (!rc) {
    printf("Get Supported Logs Response : %s\n", get_devname(ep));
    printf("Entries: %d\n", gsl->num_supported_log_entries);
    for (int i = 0; i < gsl->num_supported_log_entries; i++) {
      char uuid[40];
      uuid_unparse(gsl->entries[i].uuid, uuid);
      printf("\tentries[%d] uuid: %s, size: %d\n", i, uuid,
             gsl->entries[i].log_size);
    }
  }

  free(gsl);

  return rc;
}

int cxl_cmd_get_log(struct cxlmi_endpoint *ep, const char *log_uuid,
                    uint32_t log_size) {
  uint32_t max_payload = log_size;
  struct cxlmi_cmd_get_log_req in;
  struct cxlmi_cmd_get_log_cel_rsp *ret;
  int i, rc;

  if (!log_uuid) {
    printf("%s: Please specify log uuid argument\n", get_devname(ep));
    return CXLMI_RET_INPUT; //-EINVAL;
  }
  uuid_parse(log_uuid, in.uuid);

  // Check of size is 0. If so, sould we query supported logs and assume size?
  if (log_size == 0) {
    struct cxlmi_cmd_get_supported_logs *gsl;
    int j = 0;

    gsl = calloc(1, sizeof(*gsl) +
                        CXLMI_MAX_SUPPORTED_LOGS * sizeof(*gsl->entries));
    if (!gsl) {
      printf("Failed to allocate memory\r\n");
      return -ENOMEM;
    }

    rc = cxlmi_cmd_get_supported_logs(ep, NULL, gsl);
    if (!rc) {
      for (i = 0; i < gsl->num_supported_log_entries; i++) {
        for (j = 0; j < sizeof(gsl->entries[i].uuid); j++) {
          if (gsl->entries[i].uuid[j] != in.uuid[j])
            break;
        }
        if (j == LOG_UUID_LEN) {
          max_payload = gsl->entries[i].log_size;
          printf("Log available (using non-size size), size=%d\n", max_payload);
        }
      }
    }

    free(gsl);
  }

  in.offset = 0;
  in.length = max_payload;
  ret = calloc(1, sizeof(*ret) + max_payload);
  if (!ret) {
    printf("Failed to allocate memory\r\n");
    return -ENOMEM;
  }

  rc = cxlmi_cmd_get_log_cel(ep, NULL, &in, ret);
  if (rc)
    goto done;
  printf("Log payload info: %s\n", get_devname(ep));
  printf("    out size: 0x%x\n", in.length);

  /* Check if CEL logs */
  for (i = 0; i < sizeof(in.uuid); i++) {
    if (in.uuid[i] != cel_uuid[i])
      break;
  }
  if (i == LOG_UUID_LEN) {
    uint32_t cel_size = max_payload / sizeof(*ret);
    printf("    no_cel_entries size: %d\n", cel_size);

    for (int e = 0; e < cel_size; ++e) {
      printf("    cel_entry[%d] opcode: 0x%x, effect: 0x%x\n", e,
             (ret[e].opcode), (ret[e].command_effect));
    }
    goto done;
  }
  /* Check if Vendor Logs*/
  for (i = 0; i < sizeof(in.uuid); i++) {
    if (in.uuid[i] != ven_dbg[i])
      break;
  }
  if (i == LOG_UUID_LEN) {
    printf("    number of received bytes: %d\n", max_payload);
    printf("%s\n", (char *)ret);
    goto done;
  }

  /* If control reached here, it means unsupport log is provided*/
  printf("Invalid log UUID:%s\n", log_uuid);

done:
  free(ret);

  return rc;
}

int cxl_cmd_get_alert_config(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_memdev_get_alert_config alert_config;

  rc = cxlmi_cmd_memdev_get_alert_config(ep, NULL, &alert_config);
  if (!rc) {
    printf("Alert Config Summary : %s\n", get_devname(ep));

    printf("    valid_alerts: 0x%x\n", alert_config.valid_alerts);
    printf("    programmable_alerts: 0x%x\n", alert_config.programmable_alerts);
    printf("    life_used_critical_alert_threshold: 0x%x\n",
           alert_config.life_used_critical_alert_threshold);
    printf("    life_used_prog_warn_threshold: 0x%x\n",
           alert_config.life_used_programmable_warning_threshold);
    printf("    dev_over_temp_crit_alert_threshold: 0x%x\n",
           alert_config.device_over_temperature_critical_alert_threshold);
    printf("    dev_under_temp_crit_alert_threshold: 0x%x\n",
           alert_config.device_under_temperature_critical_alert_threshold);
    printf("    dev_over_temp_prog_warn_threshold: 0x%x\n",
           alert_config.device_over_temperature_programmable_warning_threshold);
    printf(
        "    dev_under_temp_prog_warn_threshold: 0x%x\n",
        alert_config.device_under_temperature_programmable_warning_threshold);
    printf("    corr_vol_mem_err_prog_warn_thresold: 0x%x\n",
           alert_config
               .corrected_volatile_mem_error_programmable_warning_threshold);
    printf("    corr_pers_mem_err_prog_warn_threshold: 0x%x\n",
           alert_config
               .corrected_persistent_mem_error_programmable_warning_threshold);
  }

  return rc;
}

int cxl_cmd_set_alert_config(struct cxlmi_endpoint *ep,
                             uint32_t alert_prog_threshold,
                             uint32_t device_temp_threshold,
                             uint32_t mem_error_threshold) {
  int rc;
  struct cxlmi_cmd_memdev_set_alert_config alert_config;

  alert_prog_threshold = cpu_to_be32(alert_prog_threshold);
  device_temp_threshold = cpu_to_be32(device_temp_threshold);
  mem_error_threshold = cpu_to_be32(mem_error_threshold);

  alert_config.valid_alert_actions = ((alert_prog_threshold >> 8) & 0xff);
  alert_config.enable_alert_actions = ((alert_prog_threshold >> 16) & 0xff);
  alert_config.life_used_programmable_warning_threshold =
      ((alert_prog_threshold >> 24) & 0xff);
  alert_config.rsvd1 = 0;

  alert_config.device_over_temperature_programmable_warning_threshold =
      cpu_to_le16(be16_to_cpu(((device_temp_threshold) & 0xffff)));
  alert_config.device_under_temperature_programmable_warning_threshold =
      cpu_to_le16(be16_to_cpu((((device_temp_threshold) >> 16) & 0xffff)));

  alert_config.corrected_volatile_mem_error_programmable_warning_threshold =
      cpu_to_le16(be16_to_cpu((mem_error_threshold & 0xffff)));
  alert_config.corrected_persistent_mem_error_programmable_warning_threshold =
      cpu_to_le16(be16_to_cpu(((mem_error_threshold >> 16) & 0xffff)));

  printf("alert_config settings: %s \n", get_devname(ep));
  printf("    valid_alert_actions: 0x%x\n", alert_config.valid_alert_actions);
  printf("    enable_alert_actions: 0x%x\n", alert_config.enable_alert_actions);
  printf("    life_used_prog_warn_threshold: 0x%x\n",
         alert_config.life_used_programmable_warning_threshold);
  printf(
      "    dev_over_temp_prog_warn_threshold: 0x%x\n",
      le16_to_cpu(
          alert_config.device_over_temperature_programmable_warning_threshold));
  printf("    dev_under_temp_prog_warn_threshold: 0x%x\n",
         le16_to_cpu(
             alert_config
                 .device_under_temperature_programmable_warning_threshold));
  printf("    corr_vol_mem_err_prog_warn_thresold: 0x%x\n",
         le16_to_cpu(
             alert_config
                 .corrected_volatile_mem_error_programmable_warning_threshold));
  printf(
      "    corr_pers_mem_err_prog_warn_threshold: 0x%x\n",
      le16_to_cpu(
          alert_config
              .corrected_persistent_mem_error_programmable_warning_threshold));

  rc = cxlmi_cmd_memdev_set_alert_config(ep, NULL, &alert_config);
  if (rc)
    return rc;

  printf("command completed successfully\n");
  return rc;
}

int cxl_cmd_get_health_info(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_memdev_get_health_info health_info;

  rc = cxlmi_cmd_memdev_get_health_info(ep, NULL, &health_info);
  if (!rc) {
    printf("Device Health Info : %s\n", get_devname(ep));
    printf("    health_state: 0x%x\n", health_info.health_status);
    printf("    media_status: 0x%x\n", health_info.media_status);
    printf("    additional_status: 0x%x\n", health_info.additional_status);
    printf("    life_used: 0x%x\n", health_info.life_used);
    printf("    device_temp: 0x%x\n", health_info.device_temperature);
    printf("    dirty_shutdown_count: 0x%x\n",
           health_info.dirty_shutdown_count);
    printf("    corr_vol_mem_err_count: 0x%x\n",
           health_info.corrected_volatile_error_count);
    printf("    corr_pers_mem_err_count: 0x%x\n",
           health_info.corrected_persistent_error_count);
  }

  return rc;
}

int cxl_cmd_get_dev_fw_info(struct cxlmi_endpoint *ep, bool is_os) {
  if (is_os)
    return cxl_cmd_get_os_fw_info(ep);
  else
    return cxl_cmd_get_fw_info(ep);
}

#define SLOT_MASK 0x07
int cxl_cmd_get_fw_info(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_get_fw_info fw_info;
  uint8_t slotmask = SLOT_MASK;

  rc = cxlmi_cmd_get_fw_info(ep, NULL, &fw_info);
  if (!rc) {
    printf("================================= %s : get fw info "
           "==================================\r\n",
           get_devname(ep));
    printf("FW Slots Supported: %d\n", fw_info.slots_supported);

    printf("Active FW Slot: %x\n", fw_info.slot_info & slotmask);
    printf("Staged FW Slot: %x\n", (fw_info.slot_info >> 3) & slotmask);
    printf("FW Activation Capabilities: %x\n", fw_info.caps);
    printf("Slot 1 FW Revision: %s\n", fw_info.fw_rev1);
    printf("Slot 2 FW Revision: %s\n", fw_info.fw_rev2);
    printf("Slot 3 FW Revision: %s\n", fw_info.fw_rev3);
    printf("Slot 4 FW Revision: %s\n", fw_info.fw_rev4);
  }

  return rc;
}

int cxl_cmd_get_os_fw_info(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_get_fw_info fw_info;
  uint8_t slotmask = SLOT_MASK;

  rc = cxlmi_cmd_get_os_fw_info(ep, NULL, &fw_info);
  if (!rc) {
    printf("================================= %s : get fw info "
           "==================================\r\n",
           get_devname(ep));
    printf("OS Slots Supported: %d\n", fw_info.slots_supported);

    printf("Active OS Slot: %x\n", fw_info.slot_info & slotmask);
    printf("Staged OS Slot: %x\n", (fw_info.slot_info >> 3) & slotmask);
    printf("OS Activation Capabilities: %x\n", fw_info.caps);
    printf("Slot 1 OS Revision: %s\n", fw_info.fw_rev1);
    printf("Slot 2 OS Revision: %s\n", fw_info.fw_rev2);
    printf("Slot 3 OS Revision: %s\n", fw_info.fw_rev3);
    printf("Slot 4 OS Revision: %s\n", fw_info.fw_rev4);
  }

  return rc;
}

#define FW_BYTE_ALIGN 128
#define FW_BLOCK_SIZE 128
#define INITIATE_TRANSFER 1
#define CONTINUE_TRANSFER 2
#define END_TRANSFER 3
#define ABORT_TRANSFER 4

const char *TRANSFER_FW_ERRORS[15] = {"Success",
                                      "Background Command Started",
                                      "Invalid Parameter",
                                      "Unsupported",
                                      "Internal Error",
                                      "Retry Required",
                                      "Busy",
                                      "Media Disabled",
                                      "FW Transfer in Progress",
                                      "FW Transfer Out of Order",
                                      "FW Authentication Failed",
                                      "Invalid Slot",
                                      "Aborted",
                                      "Invalid Security State",
                                      "Invalid Payload Length"};

typedef unsigned char fwblock[FW_BLOCK_SIZE];

int cxl_cmd_update_device_fw(struct cxlmi_endpoint *ep, bool is_os,
                             struct _update_fw_params *fw_params) {
  struct stat fileStat;
  int filesize;
  int fd;
  int num_blocks;
  int num_read;
  int size;
  const int max_retries = 10;
  int retry_count;
  uint32_t offset;
  fwblock *rom_buffer;
  uint32_t opcode;
  int sleep_time = 1;
  int percent_to_print = 0;
  uint16_t std_opcode = ((FIRMWARE_UPDATE << 8) | TRANSFER);

  int rc;
  FILE *rom;

  /* check file passed and get fw size */
  rom = fopen(fw_params->filepath, "rb");
  if (rom == NULL) {
    printf("Error: File open returned %s\nCould not open file %s\n",
           strerror(errno), fw_params->filepath);
    return -ENOENT;
  }

  printf("Rom filepath: %s\n", fw_params->filepath);
  fd = fileno(rom);
  rc = fstat(fd, &fileStat);
  if (rc != 0) {
    fclose(rom);
    return -ENOENT;
  }

  filesize = fileStat.st_size;

  num_blocks = filesize / FW_BLOCK_SIZE;
  if (filesize % FW_BLOCK_SIZE != 0) {
    num_blocks++;
  }

  /* Allocate mem for FW filesize */
  rom_buffer = (fwblock *)malloc(filesize);
  if (!rom_buffer) {
    printf("Failed to allocate memory\r\n");
    fclose(rom);
    return -ENOMEM;
  }

  num_read = fread(rom_buffer, 1, filesize, rom);
  if (filesize != num_read) {
    printf("Number of bytes read: %d\nNumber of bytes expected: %d\n", num_read,
           num_blocks);
    free(rom_buffer);
    fclose(rom);
    return -ENOENT;
  }

  offset = 0;

  if (is_os) {
    printf("firmware update selected for OS Image\n");
    // Vistara opcode for OS(boot1) image update
    opcode = ((VENDOR_CMD_OEM_MGMT << 8) | TRANSFER_OS);
  } else {
    if (fw_params->hbo) {
      // Pioneer vendor opcode for hbo-transfer-fw
      opcode = ((VENDOR_CMD_OEM_MGMT << 8) | TRANSFER_FW);
    } else {
      // Spec defined transfer-fw
      opcode = ((FIRMWARE_UPDATE << 8) | TRANSFER);
    }
  }

  struct cxlmi_cmd_transfer_fw *transfer_fw_input = NULL;
  struct cxlmi_cmd_hbo_status_fields hbo_status;

  /* Trasfer chunks of FW in blocks */
  for (int i = 0; i < num_blocks; i++) {
    offset = i * (FW_BLOCK_SIZE / FW_BYTE_ALIGN);

    if ((i * 100) / num_blocks >= percent_to_print) {
      printf("%d percent complete. Transfering block %d of %d at offset 0x%x\n",
             percent_to_print, i, num_blocks, offset);
      percent_to_print = percent_to_print + 10;
    }
    size = FW_BLOCK_SIZE;
    if (i == num_blocks - 1 && filesize % FW_BLOCK_SIZE != 0) {
      size = filesize % FW_BLOCK_SIZE;
    }

    transfer_fw_input = calloc(1, sizeof(*transfer_fw_input) + size);
    if (!transfer_fw_input) {
      printf("Failed to allocate memory\r\n");
      rc = -ENOMEM;
      goto out;
    }

    if (i == 0)
      transfer_fw_input->action = INITIATE_TRANSFER;
    else if (i == num_blocks - 1)
      transfer_fw_input->action = END_TRANSFER;
    else
      transfer_fw_input->action = CONTINUE_TRANSFER;
    transfer_fw_input->slot = fw_params->slot;

    transfer_fw_input->offset = offset;
    memcpy(transfer_fw_input->data, &rom_buffer[i], size);

    if (opcode == std_opcode) {
      rc = cxlmi_cmd_transfer_fw(ep, NULL, transfer_fw_input);
    } else {
      rc = cxlmi_cmd_vendor_transfer_fw(ep, NULL, transfer_fw_input, opcode);
    }
    retry_count = 0;
    sleep_time = 10;
    while (rc != 0) {
      if (retry_count > max_retries) {
        printf("Maximum %d retries exceeded while transferring block %d\n",
               max_retries, i);
        goto abort;
      }

      sleep(sleep_time);

      if (opcode == std_opcode) {
        rc = cxlmi_cmd_transfer_fw(ep, NULL, transfer_fw_input);
      } else {
        rc = cxlmi_cmd_vendor_transfer_fw(ep, NULL, transfer_fw_input, opcode);
      }
      retry_count++;
    }

    if (rc != 0) {
      printf("transfer_fw failed on %d of %d\n", i, num_blocks);
      goto abort;
    }

    rc = cxlmi_cmd_get_hbo_status(ep, NULL, &hbo_status);
    retry_count = 0;
    sleep_time = 10;
    while (rc != 0) {
      if (retry_count > max_retries) {
        printf("Maximum %d retries exceeded for hbo_status of block %d\n",
               max_retries, i);
        goto abort;
      }

      sleep(sleep_time);

      rc = cxlmi_cmd_get_hbo_status(ep, NULL, &hbo_status);
      retry_count++;
    }

    if (rc != 0) {
      printf("transfer_fw failed on %d of %d\n", i, num_blocks);
      goto abort;
    }

    if (fw_params->mock) {
      goto abort;
    }
    if (transfer_fw_input) {
      free(transfer_fw_input);
      transfer_fw_input = NULL;
    }
  }

  goto out;
abort:
  sleep(2.0);

  transfer_fw_input->action = ABORT_TRANSFER;
  transfer_fw_input->offset = FW_BLOCK_SIZE;
  if (opcode == std_opcode) {
    rc = cxlmi_cmd_transfer_fw(ep, NULL, transfer_fw_input);
  } else {
    rc = cxlmi_cmd_vendor_transfer_fw(ep, NULL, transfer_fw_input, opcode);
  }

out:
  if (transfer_fw_input) {
    free(transfer_fw_input);
    transfer_fw_input = NULL;
  }
  free(rom_buffer);
  fclose(rom);

  return rc;
}

int cxl_cmd_get_timestamp(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_get_timestamp ts;

  rc = cxlmi_cmd_get_timestamp(ep, NULL, &ts);
  if (rc)
    return rc;
  else {
    printf("timestamp: 0x%lx\n", le64_to_cpu(ts.timestamp));
  }

  return rc;
}

int cxl_cmd_set_timestamp(struct cxlmi_endpoint *ep, uint64_t timestamp) {
  int rc;
  struct cxlmi_cmd_set_timestamp ts;

  ts.timestamp = cpu_to_le64(timestamp);
  printf("setting timestamp to: 0x%lx\n", le64_to_cpu(ts.timestamp));
  rc = cxlmi_cmd_set_timestamp(ep, NULL, &ts);
  if (rc)
    return rc;
  else {
    printf("command completed successfully\n");
  }
  return rc;
}

int cxl_cmd_get_event_records(struct cxlmi_endpoint *ep, uint8_t type) {
  int rc;
  struct cxlmi_cmd_get_event_records_rsp *event_records;
  struct cxlmi_cmd_get_event_records_req event_req;
  int indent = 2;

  event_records =
      calloc(1, sizeof(*event_records) + CXL_MAX_RECORDS_TO_DUMP *
                                             sizeof(struct cxlmi_event_record));
  if (!event_records) {
    printf("Failed to allocate memory\r\n");
    return -ENOMEM;
  }

  event_req.event_log = type;
  rc = cxlmi_cmd_get_event_records(ep, NULL, &event_req, event_records);
  if (!rc) {
    printf("cxl_dram_event_record size: 0x%lx\n",
           member_size(struct cxlmi_event_record, data));
    printf("cxl_memory_module_record size: 0x%lx\n",
           member_size(struct cxlmi_event_record, data));
    printf("cxl_event_record size: 0x%lx\n", sizeof(struct cxlmi_event_record));
    printf("cxl_get_event_record_info size: 0x%lx\n",
           sizeof(struct cxlmi_cmd_get_event_records_rsp));
    printf("========= Get Event Records Info : %s =========\n",
           get_devname(ep));
    printf("%*sout size: 0x%lx\n", indent, "", sizeof(*event_records));
    printf("%*sflags: 0x%x\n", indent, "", event_records->flags);
    printf("%*soverflow_err_cnt: 0x%x\n", indent, "",
           (event_records->overflow_err_count));
    printf("%*sfirst_overflow_evt_ts: 0x%lx\n", indent, "",
           (event_records->first_overflow_timestamp));
    printf("%*slast_overflow_evt_ts: 0x%lx\n", indent, "",
           (event_records->last_overflow_timestamp));
    printf("%*sevent_record_count: 0x%x\n", indent, "",
           (event_records->record_count));

    for (int rec = 0; rec < event_records->record_count; rec++) {
      char uuid[40];
      struct cxlmi_event_record *record = &event_records->records[rec];
      uuid_unparse(record->uuid, uuid);
      if (strcmp(uuid, CXL_DRAM_EVENT_GUID) == 0)
        printf("%*sEvent Record: %d (DRAM guid: %s)\n", indent, "", rec, uuid);
      else if (strcmp(uuid, CXL_MEM_MODULE_EVENT_GUID) == 0)
        printf("%*sEvent Record: %d (Memory Module Event guid: %s)\n", indent,
               "", rec, uuid);
      else
        printf("%*sEvent Record: %d (uuid: %s)\n", indent, "", rec, uuid);

      printf("%*sevent_record_length: 0x%x\n", indent + 2, "", record->length);
      printf("%*sevent_record_flags: 0x%02x%02x%02x\n", indent + 2, "",
             record->flags[0], record->flags[1], record->flags[2]);
      printf("%*sevent_record_handle: 0x%x\n", indent + 2, "",
             le16_to_cpu(record->handle));
      printf("%*srelated_event_record_handle: 0x%x\n", indent + 2, "",
             le16_to_cpu(record->related_handle));
      printf("%*sevent_record_ts: 0x%lx\n", indent + 2, "",
             le64_to_cpu(record->timestamp));

      /* TODO: Add a loop to print dram event_record data */
    }
  }

  free(event_records);

  return rc;
}

int cxl_cmd_clear_event_records(struct cxlmi_endpoint *ep, uint8_t type,
                                uint8_t flags, uint16_t handles) {
  int rc;
  struct cxlmi_cmd_clear_event_records *event_records;
  /* TODO: Currently only 1 event_handle can be passed */
  if (flags) {
    event_records = calloc(1, sizeof(*event_records));
    if (!event_records) {
      printf("Failed to allocate memory\r\n");
      return -ENOMEM;
    }
    event_records->nr_recs = 0;
  } else {
    event_records = calloc(1, sizeof(*event_records) + sizeof(uint16_t));
    if (!event_records) {
      printf("Failed to allocate memory\r\n");
      return -ENOMEM;
    }
    event_records->nr_recs = 1;
    event_records->handles[0] = cpu_to_le16(handles);
  }
  event_records->event_log = type;
  rc = cxlmi_cmd_clear_event_records(ep, NULL, event_records);
  if (!rc) {
    printf("Clear Event Records command completed successfully\n");
  }

  free(event_records);

  return rc;
}

int cxl_cmd_get_event_interrupt_policy(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_get_event_interrupt_policy event_interrupt_policy;

  rc = cxlmi_cmd_get_event_interrupt_policy(ep, NULL, &event_interrupt_policy);
  if (!rc) {
    printf("Get Event Interrupt Policy payload info: %s\n", get_devname(ep));
    printf("    info_event_log_int_settings: 0x%x\n",
           event_interrupt_policy.informational_settings);
    printf("    warning_event_log_int_settings: 0x%x\n",
           event_interrupt_policy.warning_settings);
    printf("    failure_event_log_int_settings: 0x%x\n",
           event_interrupt_policy.warning_settings);
    printf("    fatal_event_log_int_settings: 0x%x\n",
           event_interrupt_policy.fatal_settings);
  }

  return rc;
}

int cxl_cmd_set_event_interrupt_policy(struct cxlmi_endpoint *ep,
                                       uint32_t interrupt_policy) {
  int rc;
  struct cxlmi_cmd_set_event_interrupt_policy event_interrupt_policy;

  /* below is meant for readability, you don't really need this */
  interrupt_policy = cpu_to_be32(interrupt_policy);
  event_interrupt_policy.informational_settings = (interrupt_policy & 0xff);
  event_interrupt_policy.warning_settings = ((interrupt_policy >> 8) & 0xff);
  event_interrupt_policy.failure_settings = ((interrupt_policy >> 16) & 0xff);
  event_interrupt_policy.fatal_settings = ((interrupt_policy >> 24) & 0xff);

  rc = cxlmi_cmd_set_event_interrupt_policy(ep, NULL, &event_interrupt_policy);
  if (rc)
    return rc;

  printf("command completed successfully\n");

  return rc;
}
