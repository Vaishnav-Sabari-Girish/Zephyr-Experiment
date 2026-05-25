#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/kvss/nvs.h>
#include <zephyr/logging/log.h>

/* Modern Zephyr Devicetree macros for Flash Partitions */

#define NVS_PARTITION_DEVICE DEVICE_DT_GET(DT_MTD_FROM_FIXED_PARTITION(DT_NODELABEL(storage_partition)))
#define NVS_PARTITION_OFFSET DT_REG_ADDR(DT_NODELABEL(storage_partition))

/* Assign id '1' to our boot counter variable */
#define BOOT_COUNT_ID        1

static struct nvs_fs fs;

LOG_MODULE_REGISTER(nvs_logger, LOG_LEVEL_DBG);

int main(void)
{
  int rc;
  uint32_t boot_count = 0;
  struct flash_pages_info info;

  LOG_INF("Starting NVS Boot counter example");

  /* Assign device and check readiness */
  fs.flash_device = NVS_PARTITION_DEVICE;
  if (!device_is_ready(fs.flash_device)) {
    LOG_ERR("Flash device %s is not ready", fs.flash_device->name);
    return -1;
  }

  /* Assign partition offset */
  fs.offset = NVS_PARTITION_OFFSET;

  /* Ask the hardware how big its memory pages are */
  rc = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
  if (rc) {
    LOG_ERR("Unable to get flash pages info");
    return -1;
  }

  /* Configure the NVS file system */
  fs.sector_size = info.size;
  fs.sector_count = 3U;    /* Allocate 3 flash sectors for NVS to use */

  /* Mount the NVS system */
  rc = nvs_mount(&fs);
  if (rc) {
    LOG_ERR("Mounting NVS file system failed");
    return -1;
  }

  /* Read previous boot count from memory */
  rc = nvs_read(&fs, BOOT_COUNT_ID, &boot_count, sizeof(boot_count));

  if (rc > 0) {
    LOG_INF("Found existing boot count: %d", boot_count);
  } else {
    LOG_INF("No boot count found at id. Starting fresh from 0");
  }

  /* Increment and save back to flash */
  boot_count++;
  rc = nvs_write(&fs, BOOT_COUNT_ID, &boot_count, sizeof(boot_count));

  if (rc < 0) {
    LOG_ERR("Failed to write flash memory");
  } else {
    LOG_INF("Successfully saved new boot count: %d", boot_count);
    LOG_INF("Press the RESET button to see it increment");
  }

  return 0;
}
