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

#define VENDORS_BANKS 8
#define VENDORS_ITEMS 128
#define SPD_MODULE_SERIAL_NUMBER_LEN (328 - 325 + 1) // 4 Bytes

extern const char *vendors[VENDORS_BANKS][VENDORS_ITEMS];
extern void IntToString(u8 *String, u8 *Integer, u8 SizeInByte);
extern char *decode_ddr4_module_type(u8 *bytes);
extern float ddr4_mtb_ftb_calc(unsigned char b1, signed char b2);
extern int decode_ddr4_module_speed(u8 *bytes);
extern int decode_ddr4_module_size(u8 *bytes);
extern char *decode_ddr4_manufacturer(u8 *bytes);
extern int decode_ram_type(u8 *bytes);
extern const char *ram_types[];

int cxl_cmd_dimm_spd_read(struct cxlmi_endpoint *ep, uint32_t spd_id,
                          uint32_t offset, uint32_t num_bytes) {
  int rc;
  struct cxlmi_cmd_dimm_spd_read_req spd_read_req;
  struct cxlmi_cmd_dimm_spd_read_rsp *spd_data = NULL;
  u8 serial[9];
  int buswidth;
  int ram_type;

  spd_read_req.spd_id = cpu_to_le32(spd_id);
  spd_read_req.offset = cpu_to_le32(offset);
  spd_read_req.num_bytes = cpu_to_le32(num_bytes);
  if (num_bytes) {
    spd_data = calloc(1, sizeof(spd_data->dimm_spd_data));
    if (!spd_data) {
      printf("Failed to allocate memory\r\n");
      return -ENOMEM;
    }
  }

  rc = cxlmi_cmd_dimm_spd_read(ep, NULL, &spd_read_req, spd_data);
  if (!rc) {
    ram_type = decode_ram_type(spd_data->dimm_spd_data);

    printf("=========================== DIMM SPD READ Data: %s "
           "============================\n",
           get_devname(ep));
    printf("Output Payload:");
    for (int i = 0; i < num_bytes; i++) {
      if (i % 16 == 0) {
        printf("\n%04x  %02x ", i + offset, spd_data->dimm_spd_data[i]);
      } else {
        printf("%02x ", spd_data->dimm_spd_data[i]);
      }
    }
    printf("\n\n");

    // Decoding SPD data for only DDR4 SDRAM.

    buswidth = 8 << (spd_data->dimm_spd_data[13] & 7);

    printf("\n\n====== DIMM SPD DECODE ============\n");
    printf("Total Width: %s\n", "TBD");
    printf("Data Width: %d bits\n", buswidth);
    printf("Size: %d GB\n", decode_ddr4_module_size(spd_data->dimm_spd_data));
    printf("Form Factor: %s\n", "TBD");
    printf("Set: %s\n", "TBD");
    printf("Locator: %s\n", "DIMM_X");
    printf("Bank Locator: %s\n", "_Node1_ChannelX_DimmX");
    printf("Type: %s\n", ram_types[ram_type]);
    printf("Type Detail: %s\n",
           decode_ddr4_module_type(spd_data->dimm_spd_data));
    printf("Speed: %d MT/s\n",
           decode_ddr4_module_speed(spd_data->dimm_spd_data));
    printf("Manufacturer: %s\n",
           decode_ddr4_manufacturer(spd_data->dimm_spd_data));
    IntToString(serial, &spd_data->dimm_spd_data[325],
                SPD_MODULE_SERIAL_NUMBER_LEN);
    printf("Serial Number: %s\n", serial);
    printf("Asset Tag: %s\n", "TBD");
  }

  if (spd_data)
    free(spd_data);

  return rc;
}

int cxl_cmd_dimm_slot_info(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_dimm_slot_info dimm_slot_info;
  int offset = 0;
  int indent = 2;
  char silk_screen_char;
  u8 *dimm_slots;

  rc = cxlmi_cmd_dimm_slot_info(ep, NULL, &dimm_slot_info);
  if (!rc) {
    dimm_slots = (uint8_t *)&dimm_slot_info;
    printf("=========================== DIMM SLOT INFO : %s  "
           "============================\n",
           get_devname(ep));
    printf("Output Payload:\n");
    for (int i = 0; i < sizeof(dimm_slot_info); i++) {
      if (i % 16 == 0) {
        printf("\n%04x  %02x ", i + offset, dimm_slots[i]);
      } else {
        printf("%02x ", dimm_slots[i]);
      }
    }
    printf("\n\n");

    // Decoding slot info data.
    printf("\n\n====== DIMM SLOTS INFO DECODE ============\n");

    printf("Number of DIMM Slots: %d\n", dimm_slot_info.num_dimm_slots);
    printf("DIMM SPD Index: 0\n");
    printf("%*sDIMM Present: 0x%x\n", indent + 2, "",
           dimm_slot_info.slot0_dimm_present);
    silk_screen_char = dimm_slot_info.slot0_dimm_silk_screen;
    printf("%*sDIMM Silk Screen: %c\n", indent + 2, "", silk_screen_char);
    printf("%*sChannel ID: 0x%x\n", indent + 2, "",
           dimm_slot_info.slot0_channel_id);
    printf("%*sI2C Address: 0x%x\n", indent + 2, "",
           dimm_slot_info.slot0_spd_i2c_addr);
    printf("DIMM SPD Index: 1\n");
    printf("%*sDIMM Present: 0x%x\n", indent + 2, "",
           dimm_slot_info.slot1_dimm_present);
    silk_screen_char = dimm_slot_info.slot1_dimm_silk_screen;
    printf("%*sDIMM Silk Screen: %c\n", indent + 2, "", silk_screen_char);
    printf("%*sChannel ID: 0x%x\n", indent + 2, "",
           dimm_slot_info.slot1_channel_id);
    printf("%*sI2C Address: 0x%x\n", indent + 2, "",
           dimm_slot_info.slot1_spd_i2c_addr);
    printf("DIMM SPD Index: 2\n");
    printf("%*sDIMM Present: 0x%x\n", indent + 2, "",
           dimm_slot_info.slot2_dimm_present);
    silk_screen_char = dimm_slot_info.slot2_dimm_silk_screen;
    printf("%*sDIMM Silk Screen: %c\n", indent + 2, "", silk_screen_char);
    printf("%*sChannel ID: 0x%x\n", indent + 2, "",
           dimm_slot_info.slot2_channel_id);
    printf("%*sI2C Address: 0x%x\n", indent + 2, "",
           dimm_slot_info.slot2_spd_i2c_addr);
    printf("DIMM SPD Index: 3\n");
    printf("%*sDIMM Present: 0x%x\n", indent + 2, "",
           dimm_slot_info.slot3_dimm_present);
    silk_screen_char = dimm_slot_info.slot3_dimm_silk_screen;
    printf("%*sDIMM Silk Screen: %c\n", indent + 2, "", silk_screen_char);
    printf("%*sChannel ID: 0x%x\n", indent + 2, "",
           dimm_slot_info.slot3_channel_id);
    printf("%*sI2C Address: 0x%x\n", indent + 2, "",
           dimm_slot_info.slot3_spd_i2c_addr);

    printf("\n\n");
  }

  return rc;
}

int cxl_cmd_read_ddr_temp(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_read_ddr_temp ddr_temp;

  rc = cxlmi_cmd_read_ddr_temp(ep, NULL, &ddr_temp);
  if (!rc) {
    printf("DDR DIMM temperature info: %s\n", get_devname(ep));
    for (int idx = 0; idx < DDR_MAX_DIMM_CNT; idx++) {
      printf("dimm_id : 0x%x\n", ddr_temp.ddr_dimm_temp_info[idx].dimm_id);
      printf("spd_idx: 0x%x\n", ddr_temp.ddr_dimm_temp_info[idx].spd_idx);
      printf("dimm temp: %f\n", ddr_temp.ddr_dimm_temp_info[idx].dimm_temp);
      printf("ddr temperature is %s\n\n",
             ddr_temp.ddr_dimm_temp_info[idx].ddr_temp_valid ? "valid"
                                                             : "invalid");
    }
  }

  return rc;
}

int cxl_cmd_health_counters_clear(struct cxlmi_endpoint *ep, uint32_t bitmask) {
  int rc;
  struct cxlmi_cmd_health_counters_clear health_counters_clear;

  health_counters_clear.bitmask = cpu_to_le32(bitmask);
  rc = cxlmi_cmd_health_counters_clear(ep, NULL, &health_counters_clear);

  return rc;
}

int cxl_cmd_health_counters_get(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_health_counters_get health_counters;

  rc = cxlmi_cmd_health_counters_get(ep, NULL, &health_counters);
  if (!rc) {
    printf("============================= get health counters :%s  "
           "==============================\n",
           get_devname(ep));
    printf("0: CRITICAL_OVER_TEMPERATURE_EXCEEDED = %d\n",
           le32_to_cpu(health_counters.critical_over_temperature_exceeded));
    printf(
        "1: OVER_TEMPERATURE_WARNING_LEVEL_EXCEEDED = %d\n",
        le32_to_cpu(health_counters.over_temperature_warning_level_exceeded));
    printf("2: CRITICAL_UNDER_TEMPERATURE_EXCEEDED = %d\n",
           le32_to_cpu(health_counters.critical_under_temperature_exceeded));
    printf(
        "3: UNDER_TEMPERATURE_WARNING_LEVEL_EXCEEDED = %d\n",
        le32_to_cpu(health_counters.under_temperature_warning_level_exceeded));
    printf("4: POWER_ON_EVENTS = %d\n",
           le32_to_cpu(health_counters.power_on_events));
    printf("5: POWER_ON_HOURS = %d\n",
           le32_to_cpu(health_counters.power_on_hours));
    printf("6: CXL_MEM_LINK_CRC_ERRORS = %d\n",
           le32_to_cpu(health_counters.cxl_mem_link_crc_errors));
    printf("7: CXL_IO_LINK_LCRC_ERRORS = %d\n",
           le32_to_cpu(health_counters.cxl_io_link_lcrc_errors));
    printf("8: CXL_IO_LINK_ECRC_ERRORS = %d\n",
           le32_to_cpu(health_counters.cxl_io_link_ecrc_errors));
    printf("9: NUM_DDR_COR_ECC_ERRORS = %d\n",
           le32_to_cpu(health_counters.num_ddr_correctable_ecc_errors));
    printf("10: NUM_DDR_UNCOR_ECC_ERRORS = %d\n",
           le32_to_cpu(health_counters.num_ddr_uncorrectable_ecc_errors));
    printf("11: LINK_RECOVERY_EVENTS = %d\n",
           le32_to_cpu(health_counters.link_recovery_events));
    printf("12: TIME_IN_THROTTLED = %d\n",
           le32_to_cpu(health_counters.time_in_throttled));
    printf("13: RX_RETRY_REQUEST = %d\n",
           le32_to_cpu(health_counters.rx_retry_request));
    printf("14: RCMD_QS0_HI_THRESHOLD_DETECT = %d\n",
           le32_to_cpu(health_counters.rcmd_qs0_hi_threshold_detect));
    printf("15: RCMD_QS1_HI_THRESHOLD_DETECT = %d\n",
           le32_to_cpu(health_counters.rcmd_qs1_hi_threshold_detect));
    printf("16: NUM_PSCAN_COR_ECC_ERRORS = %d\n",
           le32_to_cpu(health_counters.num_pscan_correctable_ecc_errors));
    printf("17: NUM_PSCAN_UNCOR_ECC_ERRORS = %d\n",
           le32_to_cpu(health_counters.num_pscan_uncorrectable_ecc_errors));
    printf("18: NUM_DDR_DIMM0_COR_ECC_ERRORS = %d\n",
           le32_to_cpu(health_counters.num_ddr_dimm0_correctable_ecc_errors));
    printf("19: NUM_DDR_DIMM0_UNCOR_ECC_ERRORS = %d\n",
           le32_to_cpu(health_counters.num_ddr_dimm0_uncorrectable_ecc_errors));
    printf("20: NUM_DDR_DIMM1_COR_ECC_ERRORS = %d\n",
           le32_to_cpu(health_counters.num_ddr_dimm1_correctable_ecc_errors));
    printf("21: NUM_DDR_DIMM1_UNCOR_ECC_ERRORS = %d\n",
           le32_to_cpu(health_counters.num_ddr_dimm1_uncorrectable_ecc_errors));
    printf("22: NUM_DDR_DIMM2_COR_ECC_ERRORS = %d\n",
           le32_to_cpu(health_counters.num_ddr_dimm2_correctable_ecc_errors));
    printf("23: NUM_DDR_DIMM2_UNCOR_ECC_ERRORS = %d\n",
           le32_to_cpu(health_counters.num_ddr_dimm2_uncorrectable_ecc_errors));
    printf("24: NUM_DDR_DIMM3_COR_ECC_ERRORS = %d\n",
           le32_to_cpu(health_counters.num_ddr_dimm3_correctable_ecc_errors));
    printf("25: NUM_DDR_DIMM3_UNCOR_ECC_ERRORS = %d\n",
           le32_to_cpu(health_counters.num_ddr_dimm3_uncorrectable_ecc_errors));
  }

  return rc;
}

int cxl_cmd_pmic_vtmon_info(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_pmic_vtmon_info pmic_vtmon_info;

  rc = cxlmi_cmd_pmic_vtmon_info(ep, NULL, &pmic_vtmon_info);
  if (!rc) {
    printf("=========================== PMIC VTMON SLOT INFO : %s "
           "============================\n",
           get_devname(ep));
    for (int i = 0; i < MAX_PMIC; i++) {
      printf("pmic name: %s\n", pmic_vtmon_info.pmic_data[i].pmic_name);
      printf("vin: %f\n", pmic_vtmon_info.pmic_data[i].vin);
      printf("vout: %f\n", pmic_vtmon_info.pmic_data[i].vout);
      printf("iout: %f\n", pmic_vtmon_info.pmic_data[i].iout);
      printf("powr: %f\n", pmic_vtmon_info.pmic_data[i].powr);
      printf("temp: %f\n", pmic_vtmon_info.pmic_data[i].temp);
    }
  }

  return rc;
}

extern char ltssm_state_name[][20];

int cxl_cmd_read_ltssm_states(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_read_ltssm_states read_ltssm_states;
  uint32_t *ltssm_val;
  uint32_t offset = 0;
  uint32_t curr_state;

  rc = cxlmi_cmd_read_ltssm_states(ep, NULL, &read_ltssm_states);
  if (!rc) {
    printf("LTSSM STATE CHANGES: %s \n", get_devname(ep));
    ltssm_val = read_ltssm_states.ltssm_states;
    if ((ltssm_val[offset] == ltssm_val[offset + 1]) &&
        (ltssm_val[offset + 1] == 0x0)) {
      printf("ltssm state changes are not collected\n");
    } else {
      while (offset < LTSSM_STATE_DUMP_COUNT_MAX) {
        if ((ltssm_val[offset] == ltssm_val[offset + 1]) &&
            (ltssm_val[offset + 1] == 0x0))
          break;
        curr_state = ltssm_val[offset++];
        printf("ltssm state val = 0x%x, %s\n", curr_state,
               ltssm_state_name[curr_state]);
      }
    }
  }

  return rc;
}

int cxl_cmd_pcie_eye_run(struct cxlmi_endpoint *ep, uint8_t lane,
                         uint8_t sw_scan, uint8_t ber) {
  int rc;
  struct cxlmi_cmd_pcie_eye_run_req pcie_eye_run_in;
  struct cxlmi_cmd_pcie_eye_run_rsp pcie_eye_run_out;

  pcie_eye_run_in.lane = lane;
  pcie_eye_run_in.sw_scan = sw_scan;
  pcie_eye_run_in.ber = ber;

  rc = cxlmi_cmd_pcie_eye_run(ep, NULL, &pcie_eye_run_in, &pcie_eye_run_out);
  if (!rc) {
    printf("pcie eye is running: %s\n", get_devname(ep));
  } else {
    printf("pcie eye already running OR fault, error : %d on %s\n",
           pcie_eye_run_out.pcie_eye_run_status, get_devname(ep));
  }

  return rc;
}

int cxl_cmd_pcie_eye_status(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_pcie_eye_status pcie_eye_status;

  rc = cxlmi_cmd_pcie_eye_status(ep, NULL, &pcie_eye_status);
  if (!rc) {
    printf("%s : %s\n",
           pcie_eye_status.pcie_eye_status
               ? "PCIE EYE SW IS RUNNING"
               : "PCIE EYE SW IS NOT RUNNING/FINISHED",
           get_devname(ep));
    if (pcie_eye_status.error)
      printf("pcie eye run error %d:\n", pcie_eye_status.error);
  }

  return rc;
}

#define NUM_EYESCOPE_VERT_VALS 511
#define TOTAL_EYESCOPE_VERT_VALS ((NUM_EYESCOPE_VERT_VALS * 2) + 1)
#define VERT_SKIP 15

extern void display_rx_setttings(struct rx_settings *settings);
extern void
display_merged_eye_results(struct eyescope_results *eyescope_results);

int cxl_cmd_pcie_eye_get(struct cxlmi_endpoint *ep, uint32_t sw_scan,
                         uint32_t ber) {
  int rc = 0;

  if (sw_scan) {
    struct cxlmi_cmd_pcie_eye_get_sw_req pcie_eye_get_sw_in;
    struct cxlmi_cmd_pcie_eye_get_sw_rsp pcie_eye_get_sw_out;
    printf(" PCIE_EYE SW :%s \n", get_devname(ep));
    for (int i = 0; i < TOTAL_EYESCOPE_VERT_VALS; i += VERT_SKIP) {
      pcie_eye_get_sw_in.offset = i;
      rc = cxlmi_cmd_pcie_eye_get_sw(ep, NULL, &pcie_eye_get_sw_in,
                                     &pcie_eye_get_sw_out);
      if (!rc) {
        printf("%s\n", pcie_eye_get_sw_out.pcie_eye_data);
      } else {
        printf("pcie_eye_get read failed or sw_scan not enabled: %s\n",
               get_devname(ep));
        goto abort;
      }
    }
    if (ber) {
      struct cxlmi_cmd_pcie_eye_get_sw_ber pcie_eye_get_sw_ber;
      rc = cxlmi_cmd_pcie_eye_get_sw_ber(ep, NULL, &pcie_eye_get_sw_ber);
      if (!rc) {
        printf("Extrapolation for BER at 1e-12 :%s \n", get_devname(ep));
        if (pcie_eye_get_sw_ber.vert_margin > 18 &&
            pcie_eye_get_sw_ber.horiz_margin > 0.2) {
          printf("Eye Height and width margins are > 0.2UI and 18mV, Test "
                 "PASSED\n");
          printf("Eye width margin at 1e-12 is %f UI\n",
                 pcie_eye_get_sw_ber.horiz_margin);
          printf("Eye height margin at 1e-12 is %f mV\n",
                 pcie_eye_get_sw_ber.vert_margin);
        } else {
          printf("Eye Height and width margins are not greater than 0.2UI and "
                 "18mV, Test FAILED\n");
        }
      }
    }
  } else {
    struct cxlmi_cmd_pcie_eye_get_hw pcie_eye_get_hw;
    rc = cxlmi_cmd_pcie_eye_get_hw(ep, NULL, &pcie_eye_get_hw);
    if (!rc) {
      printf(" PCIE_EYE HW :%s \n", get_devname(ep));
      if (pcie_eye_get_hw.eyescope_request_status) {
        printf("eyescope request status: PASS\n");
        display_rx_setttings(&pcie_eye_get_hw.rx_settings);
        display_merged_eye_results(&pcie_eye_get_hw.eyescope_results);
      } else {
        printf("eyescope request status: FAIL\n");
      }
    } else {
      printf("pcie_eye_get read failed hw scan not enabled\n");
      goto abort;
    }
  }

abort:
  return rc;
}

int cxl_cmd_get_cxl_link_status(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_get_cxl_link_status cxl_link_status;

  rc = cxlmi_cmd_get_cxl_link_status(ep, NULL, &cxl_link_status);
  if (!rc) {
    printf("CXL Link Status : %s\n", get_devname(ep));
    printf("Link is in CXL%0.1f mode\n", cxl_link_status.cxl_link_status);
    printf("Negotiated link width: x%d\n", cxl_link_status.link_width);
    printf("Negotiated link speed: Gen%d\n", cxl_link_status.link_speed);
    printf("ltssm state: %s, code 0x%x\n",
           ltssm_state_name[cxl_link_status.ltssm_val],
           cxl_link_status.ltssm_val);
  }

  return rc;
}

int cxl_cmd_get_device_info(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_get_device_info device_info;

  rc = cxlmi_cmd_get_device_info(ep, NULL, &device_info);
  if (!rc) {
    printf("Device Info: %s\n", get_devname(ep));
    printf("Device id: 0x%x\n", device_info.device_id);
    printf("Revision id: 0x%x\n", device_info.revision_id);
  }

  return rc;
}

int cxl_cmd_get_ddr_bw(struct cxlmi_endpoint *ep, uint32_t timeout,
                       uint32_t iterations) {
  int rc;
  struct cxlmi_cmd_get_ddr_bw_req get_ddr_bw_in = {0, 0};
  struct cxlmi_cmd_get_ddr_bw_rsp get_ddr_bw_out;
  float total_peak_bw = 0;

  get_ddr_bw_in.timeout = timeout;
  get_ddr_bw_in.iterations = iterations;

  rc = cxlmi_cmd_get_ddr_bw(ep, NULL, &get_ddr_bw_in, &get_ddr_bw_out);
  if (!rc) {
    printf("DDR GET BW : %s\n", get_devname(ep));
    for (int i = 0; i < DDR_MAX_SUBSYS; i++) {
      printf("ddr%d peak bandwidth = %f GB/s\n", i, get_ddr_bw_out.peak_bw[i]);
      total_peak_bw += get_ddr_bw_out.peak_bw[i];
    }
    printf("total peak bandwidth = %f GB/s\n", total_peak_bw);
  }

  return rc;
}

int cxl_cmd_ddr_margin_run(struct cxlmi_endpoint *ep, uint8_t slice_num,
                           uint8_t rd_wr_margin, uint8_t ddr_id) {
  int rc;
  struct cxlmi_cmd_ddr_margin_run ddr_margin_run = {};

  ddr_margin_run.slice_num = slice_num;
  ddr_margin_run.rd_wr_margin = rd_wr_margin;
  ddr_margin_run.ddr_id = ddr_id;

  rc = cxlmi_cmd_ddr_margin_run(ep, NULL, &ddr_margin_run);
  if (!rc) {
    printf("DDR MARGIN RUN : %s\n", get_devname(ep));
  }

  return rc;
}

int cxl_cmd_ddr_margin_status(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_ddr_margin_status ddr_margin_status;

  rc = cxlmi_cmd_ddr_margin_status(ep, NULL, &ddr_margin_status);
  if (!rc) {
    printf("DDR MARGIN STATUS : %s\n", get_devname(ep));
    printf("%s\n", ddr_margin_status.run_status
                       ? "DDR MARGIN IS RUNNING"
                       : "DDR MARGIN IS NOT RUNNING/FINISHED");
  }

  return rc;
}

int cxl_cmd_ddr_margin_get(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_ddr_margin_get ddr_margin_get;

  rc = cxlmi_cmd_ddr_margin_get(ep, NULL, &ddr_margin_get);
  if (!rc) {
    printf("DDR MARGIN GET : %s\n", get_devname(ep));
    printf(
        "SliceNo,bitNo, VrefLv, MinDelay, MaxDelay, MinDly(ps), MaxDly(ps)\n");

    for (int i = 0; i < ddr_margin_get.row_count; i++) {
      printf("%d,%d,%d,%d,%d,%3.2f,%3.2f\n",
             ddr_margin_get.ddr_margin_slice_data[i].slicenumber,
             ddr_margin_get.ddr_margin_slice_data[i].bitnumber,
             ddr_margin_get.ddr_margin_slice_data[i].vreflevel,
             ddr_margin_get.ddr_margin_slice_data[i].margin_low,
             ddr_margin_get.ddr_margin_slice_data[i].margin_high,
             ddr_margin_get.ddr_margin_slice_data[i].min_delay_ps,
             ddr_margin_get.ddr_margin_slice_data[i].max_delay_ps);
    }
  }

  return rc;
}

int cxl_cmd_reboot_mode_set(struct cxlmi_endpoint *ep, uint8_t reboot_mode) {
  int rc;
  struct cxlmi_cmd_reboot_mode_set reboot_mode_set;

  reboot_mode_set.reboot_mode = reboot_mode;

  rc = cxlmi_cmd_reboot_mode_set(ep, NULL, &reboot_mode_set);
  if (!rc) {
    printf("REBOOT MODE SET : %s\n", get_devname(ep));
  }

  return rc;
}

int cxl_cmd_curr_cxl_boot_mode_get(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_curr_cxl_boot_mode_get curr_cxl_boot_mode;

  rc = cxlmi_cmd_curr_cxl_boot_mode_get(ep, NULL, &curr_cxl_boot_mode);
  if (!rc) {
    printf("CURR CXL BOOT MODE : %s\n", get_devname(ep));
    if (curr_cxl_boot_mode.curr_cxl_boot == CXL_IO_MEM_MODE)
      printf("CXL_IO_MEM_MODE\n");
    else if (curr_cxl_boot_mode.curr_cxl_boot == CXL_IO_MODE)
      printf("CXL_IO_MODE\n");
    else
      printf("Invalid Mode\n");
  }

  return rc;
}

extern void display_error_count(struct ddr_controller_errors *ddr_ctrl_err,
                                ddr_subsys ddr_id);

int cxl_cmd_get_ddr_ecc_err_info(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_get_ddr_ecc_err_info get_ddr_ecc_err_info;

  rc = cxlmi_cmd_get_ddr_ecc_err_info(ep, NULL, &get_ddr_ecc_err_info);
  if (!rc) {
    printf("GET DDR ECC ERR INFO : %s\n", get_devname(ep));
    for (int i = 0; i < DDR_MAX_SUBSYS; i++) {
      display_error_count(get_ddr_ecc_err_info.ddr_ctrl_err, i);
    }
  }

  return rc;
}

int cxl_cmd_i2c_read(struct cxlmi_endpoint *ep, uint16_t slave_addr,
                     uint8_t reg_addr, uint8_t num_bytes) {
  int rc;
  struct cxlmi_cmd_i2c_read_req i2c_read_in;
  struct cxlmi_cmd_i2c_read_rsp i2c_read_out;

  i2c_read_in.slave_addr = slave_addr;
  i2c_read_in.reg_addr = reg_addr;
  i2c_read_in.num_bytes = num_bytes;

  if (num_bytes > I2C_MAX_SIZE_NUM_BYTES) {
    rc = EINVAL;
    printf("%s: Max number of bytes supported is %d, cmd submission failed: %d "
           "(%s)\n",
           get_devname(ep), I2C_MAX_SIZE_NUM_BYTES, rc, strerror(-rc));
    return -EINVAL;
  }

  rc = cxlmi_cmd_i2c_read(ep, NULL, &i2c_read_in, &i2c_read_out);
  if (!rc) {
    printf("i2c read success : %s\n", get_devname(ep));
    printf("i2c read output:");
    for (int i = 0; i < i2c_read_out.num_bytes; i++) {
      printf("0x%x\t", i2c_read_out.buf[i]);
    }
    printf("\n");
  }

  return rc;
}

int cxl_cmd_i2c_write(struct cxlmi_endpoint *ep, uint16_t slave_addr,
                      uint8_t reg_addr, uint8_t data) {
  int rc;
  struct cxlmi_cmd_i2c_write i2c_write;
  i2c_write.slave_addr = slave_addr;
  i2c_write.reg_addr = reg_addr;
  i2c_write.data = data;

  rc = cxlmi_cmd_i2c_write(ep, NULL, &i2c_write);
  if (!rc) {
    printf("i2c write success : %s\n", get_devname(ep));
  }

  return rc;
}

int cxl_cmd_get_ddr_latency(struct cxlmi_endpoint *ep, uint32_t measure_time) {
  int rc;
  struct cxlmi_cmd_get_ddr_latency_req get_ddr_latency_in;
  struct cxlmi_cmd_get_ddr_latency_rsp get_ddr_latency_out;

  get_ddr_latency_in.measure_time = measure_time;

  rc = cxlmi_cmd_get_ddr_latency(ep, NULL, &get_ddr_latency_in,
                                 &get_ddr_latency_out);
  if (!rc) {
    printf("Get DDR Latency : %s\n", get_devname(ep));
    for (int ddr_id = 0; ddr_id < DDR_MAX_SUBSYS; ddr_id++) {
      printf("\nDDR%d Latency:\n", ddr_id);
      printf(
          "readLat: %lu, rdSampleCnt: %u\n, writeLat: %lu, wrSampleCnt: %u\n",
          get_ddr_latency_out.ddr_lat_op[ddr_id].readlat,
          get_ddr_latency_out.ddr_lat_op[ddr_id].rdsamplecnt,
          get_ddr_latency_out.ddr_lat_op[ddr_id].writelat,
          get_ddr_latency_out.ddr_lat_op[ddr_id].wrsamplecnt);

      printf("Average Latency:\n");
      printf("Avg Read Latency  : %f ns \n Avg Write Latency : %f ns \n",
             get_ddr_latency_out.ddr_lat_op[ddr_id].avg_rdlatency,
             get_ddr_latency_out.ddr_lat_op[ddr_id].avg_wrlatency);
    }
  }

  return rc;
}

extern void
display_membridge_errors(struct cxlmi_cmd_get_membridge_errors *membridge_err);

int cxl_cmd_get_membridge_errors(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_get_membridge_errors get_membridge_errors;

  rc = cxlmi_cmd_get_membridge_errors(ep, NULL, &get_membridge_errors);
  if (!rc) {
    printf("Get Membrige Errors : %s\n", get_devname(ep));
    display_membridge_errors(&get_membridge_errors);
  }

  return rc;
}

int cxl_cmd_hpa_to_dpa(struct cxlmi_endpoint *ep, uint64_t hpa_address) {
  int rc;
  struct cxlmi_cmd_hpa_to_dpa_req hpa_to_dpa_in;
  struct cxlmi_cmd_hpa_to_dpa_rsp hpa_to_dpa_out;

  hpa_to_dpa_in.hpa_address = hpa_address;

  rc = cxlmi_cmd_hpa_to_dpa(ep, NULL, &hpa_to_dpa_in, &hpa_to_dpa_out);
  if (!rc) {
    printf("dpa address:0x%lx\n", hpa_to_dpa_out.dpa_address);
  }
  return rc;
}

int cxl_cmd_start_ddr_ecc_scrub(struct cxlmi_endpoint *ep) {
  int rc;

  rc = cxlmi_cmd_start_ddr_ecc_scrub(ep, NULL);
  if (!rc) {
    printf("DDR ECC Scrub Started: %s\n", get_devname(ep));
  }

  return rc;
}

int cxl_cmd_ddr_ecc_scrub_status(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_ddr_ecc_scrub_status ddr_ecc_scrub_status;

  rc = cxlmi_cmd_ddr_ecc_scrub_status(ep, NULL, &ddr_ecc_scrub_status);
  if (!rc) {
    printf("DDR ECC Scrub Status: %s\n", get_devname(ep));
    for (int subsys = DDR_CTRL0; subsys < DDR_MAX_SUBSYS; subsys++) {
      printf("DDR-%d %s\n", subsys,
             ddr_ecc_scrub_status.ecc_scrub_status[subsys]
                 ? "ECC SCRUB IS IN PROGRESS"
                 : "DDR SCRUB IS NOT RUNNING/FINISHED");
    }
  }

  return rc;
}

extern void
display_ddr_init_status(struct cxlmi_cmd_ddr_init_status *ddr_init_status);

int cxl_cmd_ddr_init_status(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_ddr_init_status ddr_init_status;

  rc = cxlmi_cmd_ddr_init_status(ep, NULL, &ddr_init_status);
  if (!rc) {
    printf("DDR INIT Status: %s\n", get_devname(ep));

    display_ddr_init_status(&ddr_init_status);
  }

  return rc;
}

extern void
display_membridge_stats(struct cxlmi_cmd_get_membridge_stats *membridge_stats);

int cxl_cmd_get_membridge_stats(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_get_membridge_stats membridge_stats;

  rc = cxlmi_cmd_get_membridge_stats(ep, NULL, &membridge_stats);
  if (!rc) {
    printf("Membrige Stats: %s\n", get_devname(ep));

    display_membridge_stats(&membridge_stats);
  }

  return rc;
}

int cxl_cmd_ddr_err_inj_en(struct cxlmi_endpoint *ep, uint32_t ddr_id,
                           uint32_t err_type, uint64_t ecc_fwc_mask) {
  int rc;
  struct cxlmi_cmd_ddr_err_inj_en ddr_err_inj_en;
  ddr_err_inj_en.ddr_id = ddr_id;
  ddr_err_inj_en.err_type = err_type;
  ddr_err_inj_en.ecc_fwc_mask = ecc_fwc_mask;

  rc = cxlmi_cmd_ddr_err_inj_en(ep, NULL, &ddr_err_inj_en);
  if (!rc) {
    printf("Error injection enabled on DDR%d: %s\n", ddr_id, get_devname(ep));
  }

  return rc;
}

int cxl_cmd_trigger_coredump(struct cxlmi_endpoint *ep) {
  int rc;

  rc = cxlmi_cmd_trigger_coredump(ep, NULL);
  if (!rc) {
    printf("Triggered simulated coredump: %s\n", get_devname(ep));
  }

  return rc;
}

int cxl_cmd_ddr_stats_run(struct cxlmi_endpoint *ep, uint8_t ddr_id,
                          uint32_t monitor_time, uint32_t loop_count) {
  int rc;
  struct cxlmi_cmd_ddr_stats_run ddr_stats_run;
  ddr_stats_run.ddr_id = ddr_id;
  ddr_stats_run.monitor_time = monitor_time;
  ddr_stats_run.loop_count = loop_count;

  rc = cxlmi_cmd_ddr_stats_run(ep, NULL, &ddr_stats_run);
  if (!rc) {
    printf("DDR Stats run on DDR%d: %s\n", ddr_id, get_devname(ep));
  }

  return rc;
}

#define MAX_CXL_TRANSFER_SZ (16 * 1024)

extern void display_pmon_stats(ddr_stats_data_t *disp_stats,
                               uint32_t loop_count);
extern void display_cs_pm_stats(ddr_stats_data_t *disp_stats,
                                uint32_t loop_count);
extern void display_cs_bank_pm_stats(ddr_stats_data_t *disp_stats,
                                     uint32_t loop_count);
extern void display_mc_pm_stats(ddr_stats_data_t *disp_stats,
                                uint32_t loop_count);

int cxl_cmd_ddr_stats_get(struct cxlmi_endpoint *ep) {
  int rc = 0;
  int total_bytes = 0, bytes_to_cpy = 0, bytes_copied = 0;
  ddr_stats_data_t *ddr_stats_start = NULL;
  unsigned char *buf = NULL;
  struct cxlmi_cmd_ddr_stats_status ddr_stats_status;

  rc = cxlmi_cmd_ddr_stats_status(ep, NULL, &ddr_stats_status);
  if (!rc) {
    printf("DDR stats get : %s\n", get_devname(ep));
    printf("%s\n", ddr_stats_status.run_status
                       ? "DDR STATS IS BUSY"
                       : "DDR STATS IS NOT RUNNING/FINISHED");

    printf("Loop Count = %d\n", ddr_stats_status.loop_count);
    if (ddr_stats_status.run_status) {
      return -EBUSY;
    }

    total_bytes = sizeof(ddr_stats_data_t) * ddr_stats_status.loop_count;
    buf = (unsigned char *)malloc(total_bytes);
    if (!buf) {
      printf("Failed to allocate memory\r\n");
      return -ENOMEM;
    }

    ddr_stats_start = (ddr_stats_data_t *)buf;

    while (bytes_copied < total_bytes) {
      bytes_to_cpy = (total_bytes - bytes_copied) < MAX_CXL_TRANSFER_SZ
                         ? (total_bytes - bytes_copied)
                         : MAX_CXL_TRANSFER_SZ;
      struct cxlmi_cmd_ddr_stats_get_req ddr_stats_get_in;
      ddr_stats_get_in.offset = bytes_copied;
      ddr_stats_get_in.transfer_sz = bytes_to_cpy;
      cxlmi_cmd_ddr_stats_get_rsp_t ddr_stats_get_out = buf + bytes_copied;
      rc = cxlmi_cmd_ddr_stats_get(ep, NULL, &ddr_stats_get_in,
                                   &ddr_stats_get_out);
      bytes_copied = bytes_copied + bytes_to_cpy;
      if (rc < 0)
        goto out;
    }

    display_pmon_stats(ddr_stats_start, ddr_stats_status.loop_count);
    display_cs_pm_stats(ddr_stats_start, ddr_stats_status.loop_count);
    display_cs_bank_pm_stats(ddr_stats_start, ddr_stats_status.loop_count);
    display_mc_pm_stats(ddr_stats_start, ddr_stats_status.loop_count);
  }
out:
  if (ddr_stats_start) {
    free(ddr_stats_start);
    ddr_stats_start = NULL;
  }

  return rc;
}

int cxl_cmd_ddr_param_set(struct cxlmi_endpoint *ep, uint32_t ddr_interleave_sz,
                          uint32_t ddr_interleave_ctrl_choice) {
  int rc;
  struct cxlmi_cmd_ddr_param_set ddr_param;
  ddr_param.ddr_inter.ddr_interleave_sz = ddr_interleave_sz;
  ddr_param.ddr_inter.ddr_interleave_ctrl_choice = ddr_interleave_ctrl_choice;

  rc = cxlmi_cmd_ddr_param_set(ep, NULL, &ddr_param);
  if (!rc) {
    printf("DDR param set: %s\n", get_devname(ep));
  }

  return rc;
}

int cxl_cmd_ddr_param_get(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_ddr_param_get ddr_param;

  rc = cxlmi_cmd_ddr_param_get(ep, NULL, &ddr_param);
  if (!rc) {
    printf("DDR param get: %s\n", get_devname(ep));
    printf("ddr_interleave_sz: %d\n", ddr_param.ddr_inter.ddr_interleave_sz);
    printf("ddr_interleave_ctrl_choice: %d\n",
           ddr_param.ddr_inter.ddr_interleave_ctrl_choice);
  }

  return rc;
}

extern void print_ddr_phy_pll_status(uint32_t instance,
                                     struct ddr_phy_pll_status *phy_pll_status);
extern void
print_ddr_training_status(uint32_t instance,
                          struct ddr_dimm_training_status *dimm_tr_status);
extern void
print_margin_vref_low_high(uint32_t instance,
                           struct ddr_dimm_training_status *dimm_tr_status);
extern void print_margin_rdlvl_delay_window(
    int instance, struct ddr_dimm_training_status *dimm_tr_status);
extern void print_margin_wrdqlvl_delay_window(
    int instance, struct ddr_dimm_training_status *dimm_tr_status);
extern void print_err_status(int instance,
                             struct ddr_dimm_training_status *dimm_tr_status);

int cxl_cmd_ddr_dimm_level_training_status(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_dimm_level_training_status training_status;

  rc = cxlmi_cmd_dimm_level_training_status(ep, NULL, &training_status);
  if (!rc) {
    printf("DIMM Level Training Status: %s\n", get_devname(ep));

    for (int i = DDR_CTRL0; i < DDR_MAX_SUBSYS; i++) {
      printf("dimm:%d level training status\n", i);
      print_ddr_phy_pll_status(
          i, &training_status.dimm_training_status[i].phy_pll_status);
      print_ddr_training_status(i, &training_status.dimm_training_status[i]);
      print_margin_vref_low_high(i, &training_status.dimm_training_status[i]);
      print_margin_rdlvl_delay_window(i,
                                      &training_status.dimm_training_status[i]);
      print_margin_wrdqlvl_delay_window(
          i, &training_status.dimm_training_status[i]);
      print_err_status(i, &training_status.dimm_training_status[i]);
    }
  }

  return rc;
}

int cxl_cmd_viral_inj_en(struct cxlmi_endpoint *ep, uint32_t viral_type) {
  int rc;
  struct cxlmi_cmd_viral_inj_en vira_inj_en;
  vira_inj_en.viral_type = viral_type;

  rc = cxlmi_cmd_viral_inj_en(ep, NULL, &vira_inj_en);
  if (!rc) {
    printf("Viral Injection enabled: %s\n", get_devname(ep));
  }

  return rc;
}

int cxl_cmd_mem_ll_err_inj_en(struct cxlmi_endpoint *ep, uint32_t en_dis,
                              uint32_t ll_err_type) {
  int rc;
  struct cxlmi_cmd_mem_ll_err_inj_en mem_ll_err_inj_en;
  mem_ll_err_inj_en.en_dis = en_dis;
  mem_ll_err_inj_en.ll_err_type = ll_err_type;

  rc = cxlmi_cmd_mem_ll_err_inj_en(ep, NULL, &mem_ll_err_inj_en);
  if (!rc) {
    printf("Mem LL err Injection enabled: %s\n", get_devname(ep));
  }

  return rc;
}

int cxl_cmd_pci_err_inj_en(struct cxlmi_endpoint *ep, uint32_t en_dis,
                           uint32_t type, uint32_t err, uint32_t count,
                           uint32_t opt1, uint32_t opt2) {
  int rc;
  struct cxlmi_cmd_pci_err_inj_en pci_err_inj;
  pci_err_inj.en_dis = en_dis;
  pci_err_inj.err_type = type;
  pci_err_inj.err_subtype = err;
  pci_err_inj.count = count;
  pci_err_inj.opt_param1 = opt1;
  pci_err_inj.opt_param2 = opt2;

  rc = cxlmi_cmd_pci_err_inj_en(ep, NULL, &pci_err_inj);
  if (!rc) {
    printf("PCI Error Injection enabled: %s\n", get_devname(ep));
  }

  return rc;
}

int cxl_cmd_core_volt_set(struct cxlmi_endpoint *ep, float core_volt) {
  int rc;
  struct cxlmi_cmd_core_volt_set core_volt_in;
  core_volt_in.core_volt = core_volt;

  rc = cxlmi_cmd_core_volt_set(ep, NULL, &core_volt_in);
  if (!rc) {
    printf("Core Volt set: %s\n", get_devname(ep));
  }

  return rc;
}

int cxl_cmd_core_volt_get(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_core_volt_get core_volt_out;

  rc = cxlmi_cmd_core_volt_get(ep, NULL, &core_volt_out);
  if (!rc) {
    printf("Core Volt get: %s\n", get_devname(ep));
    printf("Core Voltage: %f V\n", core_volt_out.core_volt);
  }

  return rc;
}

int cxl_cmd_ddr_cont_scrub_status(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_ddr_cont_scrub_status ddr_cont_scrub_status;

  rc = cxlmi_cmd_ddr_cont_scrub_status(ep, NULL, &ddr_cont_scrub_status);
  if (!rc) {
    printf("DDR cont scrub status: %s\n", get_devname(ep));
    printf("%s\n", ddr_cont_scrub_status.cont_scrub_status
                       ? "CONTINUOUS SCRUB IS ON"
                       : "CONTINUOUS SCRUB IS OFF");
  }

  return rc;
}

int cxl_cmd_ddr_cont_scrub_set(struct cxlmi_endpoint *ep,
                               uint32_t cont_scrub_status) {
  int rc;
  struct cxlmi_cmd_ddr_cont_scrub_set ddr_cont_scrub_in;
  ddr_cont_scrub_in.cont_scrub_status = cont_scrub_status;

  rc = cxlmi_cmd_ddr_cont_scrub_set(ep, NULL, &ddr_cont_scrub_in);
  if (!rc) {
    printf("DDR cont scrub set: %s\n", get_devname(ep));
  }

  return rc;
}

int cxl_cmd_ddr_page_select_set(struct cxlmi_endpoint *ep,
                                uint8_t page_select_option) {
  int rc;
  struct cxlmi_cmd_ddr_page_select_set page_select_in;
  page_select_in.pp_select.page_policy_reg_val = page_select_option;

  rc = cxlmi_cmd_ddr_page_select_set(ep, NULL, &page_select_in);
  if (!rc) {
    printf("DDR Page Policy set: %s\n", get_devname(ep));
  }

  return rc;
}

int cxl_cmd_ddr_page_select_get(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_ddr_page_select_get page_select_out;

  rc = cxlmi_cmd_ddr_page_select_get(ep, NULL, &page_select_out);
  if (!rc) {
    printf("DDR Page Policy get: %s\n", get_devname(ep));
    printf("Page_Policy_Reg_Value is selected for %s\n",
           (page_select_out.pp_select.page_policy_reg_val) ? "open" : "close");
  }

  return rc;
}

int cxl_cmd_ddr_hppr_set(struct cxlmi_endpoint *ep,
                         uint64_t hppr_enable_option) {
  int rc;
  struct cxlmi_cmd_ddr_hppr_set hppr_in;
  hppr_in.enable = hppr_enable_option;

  rc = cxlmi_cmd_ddr_hppr_set(ep, NULL, &hppr_in);
  if (!rc) {
    printf("DDR HPPR set: %s\n", get_devname(ep));
  }

  return rc;
}

int cxl_cmd_ddr_hppr_get(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_ddr_hppr_get hppr_out;

  rc = cxlmi_cmd_ddr_hppr_get(ep, NULL, &hppr_out);
  if (!rc) {
    printf("DDR HPPR get: %s\n", get_devname(ep));
    printf("DDR[0] HPPR is %s\n",
           (hppr_out.hppr_enable[0] == 1) ? "Enabled" : "Disabled");
    printf("DDR[1] HPPR is %s\n",
           (hppr_out.hppr_enable[1] == 1) ? "Enabled" : "Disabled");
  }

  return rc;
}

int cxl_cmd_ddr_hppr_addr_info_set(struct cxlmi_endpoint *ep, uint8_t ddr_id,
                                   uint8_t chip_select, uint8_t bank_group,
                                   uint8_t bank, uint32_t row) {
  int rc;
  struct cxlmi_cmd_ddr_hppr_addr_info_set hppr_addr_info_in;
  hppr_addr_info_in.hppr_addr_info.ddr_id = ddr_id;
  hppr_addr_info_in.hppr_addr_info.chip_select = chip_select;
  hppr_addr_info_in.hppr_addr_info.bank = bank;
  hppr_addr_info_in.hppr_addr_info.bank_group = bank_group;
  hppr_addr_info_in.hppr_addr_info.row = row;

  rc = cxlmi_cmd_ddr_hppr_addr_info_set(ep, NULL, &hppr_addr_info_in);
  if (!rc) {
    printf("DDR HPPR addr info set: %s\n", get_devname(ep));
  }

  return rc;
}

int cxl_cmd_ddr_hppr_addr_info_get(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_ddr_hppr_addr_info_get hppr_addr_info_out;

  rc = cxlmi_cmd_ddr_hppr_addr_info_get(ep, NULL, &hppr_addr_info_out);
  if (!rc) {
    printf("DDR HPPR addr info get: %s\n", get_devname(ep));
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 8; j++) {
        printf("DDR[%d],id[%d],ch=%d,cs=%d,bg=0x%02x,b=0x%02x,r=0x%08x,ppr_"
               "state=%d\n",
               hppr_addr_info_out.hppr_addr_info[i][j].ddr_id, j,
               hppr_addr_info_out.hppr_addr_info[i][j].channel,
               hppr_addr_info_out.hppr_addr_info[i][j].chip_select,
               hppr_addr_info_out.hppr_addr_info[i][j].bank_group,
               hppr_addr_info_out.hppr_addr_info[i][j].bank,
               hppr_addr_info_out.hppr_addr_info[i][j].row,
               hppr_addr_info_out.hppr_addr_info[i][j].ppr_state);
      }
    }
  }

  return rc;
}

int cxl_cmd_ddr_hppr_addr_info_clear(struct cxlmi_endpoint *ep, uint8_t ddr_id,
                                     uint8_t channel_id) {
  int rc;
  struct cxlmi_cmd_ddr_hppr_addr_info_clear hppr_addr_info_clear_in;
  hppr_addr_info_clear_in.ddr_id = ddr_id;
  hppr_addr_info_clear_in.channel_id = channel_id;

  rc = cxlmi_cmd_ddr_hppr_addr_info_clear(ep, NULL, &hppr_addr_info_clear_in);
  if (!rc) {
    printf("DDR HPPR addr info cleared: %s\n", get_devname(ep));
  }

  return rc;
}

int cxl_cmd_ddr_ppr_get_status(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_ddr_ppr_get_status ppr_status;

  rc = cxlmi_cmd_ddr_ppr_get_status(ep, NULL, &ppr_status);
  if (!rc) {
    printf("DDR PPR status for: %s\n", get_devname(ep));
    printf("DDR PPR Status is %d\n", ppr_status.status);
  }

  return rc;
}

int cxl_cmd_ddr_refresh_mode_set(struct cxlmi_endpoint *ep,
                                 uint8_t refresh_mode) {
  int rc;
  struct cxlmi_cmd_ddr_refresh_mode_set ddr_refresh_mode;
  ddr_refresh_mode.ddr_refresh_val = refresh_mode;

  rc = cxlmi_cmd_ddr_refresh_mode_set(ep, NULL, &ddr_refresh_mode);
  if (!rc) {
    printf("DDR refresh mode set: %s\n", get_devname(ep));
  }

  return rc;
}

int cxl_cmd_ddr_refresh_mode_get(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_ddr_refresh_mode_get ddr_refresh_mode;

  rc = cxlmi_cmd_ddr_refresh_mode_get(ep, NULL, &ddr_refresh_mode);
  if (!rc) {
    printf("DDR refresh mode get: %s\n", get_devname(ep));
    printf("REFRESH MODE IS SELECTED TO %dxRefresh mode\n",
           (ddr_refresh_mode.ddr_refresh_val == 0)
               ? 1
               : ddr_refresh_mode.ddr_refresh_val);
  }

  return rc;
}

extern void
display_cxl_error_info(struct cxlmi_cmd_cxl_err_cntr_get *cxl_err_cnt);

int cxl_cmd_cxl_err_cntr_get(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_cxl_err_cntr_get cxl_err_cntr;

  rc = cxlmi_cmd_cxl_err_cntr_get(ep, NULL, &cxl_err_cntr);
  if (!rc) {
    printf("CXL err cntr for: %s\n", get_devname(ep));
    display_cxl_error_info(&cxl_err_cntr);
  }

  return rc;
}

int cxl_cmd_ddr_freq_get(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_ddr_freq_get ddr_freq;

  rc = cxlmi_cmd_ddr_freq_get(ep, NULL, &ddr_freq);
  if (!rc) {
    printf("DDR freq for: %s\n", get_devname(ep));
    printf("DDR Operating Frequency: %f MHz\n", ddr_freq.ddr_freq);
  }

  return rc;
}

int cxl_cmd_ddr_init_err_info_get(struct cxlmi_endpoint *ep) {
  int rc;
  struct cxlmi_cmd_ddr_init_err_info_get ddr_init_err_info;

  rc = cxlmi_cmd_ddr_init_err_info_get(ep, NULL, &ddr_init_err_info);
  if (!rc) {
    printf("DDR Init error info for: %s\n", get_devname(ep));
    for (int i = 0; i < DDR_MAX_SUBSYS; i++) {
      printf("BIST error details for DDR (%d) \n", i);
      printf("DDR BIST error count %d\n",
             ddr_init_err_info.ddr_bist_err_info[i].ddr_bist_err_cnt);
      printf("DDR BIST error info (col)%d\n",
             ddr_init_err_info.ddr_bist_err_info[i].ddr_bist_err_info_col);
      printf("DDR BIST error info (row)%d\n",
             ddr_init_err_info.ddr_bist_err_info[i].ddr_bist_err_info_row);
      printf("DDR BIST error info (bank)%d\n",
             ddr_init_err_info.ddr_bist_err_info[i].ddr_bist_err_info_bank);
      printf("DDR BIST error info (cs)%d\n",
             ddr_init_err_info.ddr_bist_err_info[i].ddr_bist_err_info_cs);
    }
  }

  return rc;
}
