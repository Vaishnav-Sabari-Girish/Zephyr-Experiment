#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <string.h>
#include "display_hal.h"

LOG_MODULE_REGISTER(oled_uart_shell, LOG_LEVEL_INF);

/*
 * The Command Handler 
*/
static int cmd_oled_msg(const struct shell *sh, size_t argc, char **argv) {
  if (argc < 2) {
    shell_print(sh, "Usage: oled msg <Your Text Here>");
    return -EINVAL;
  }

  char buffer[128] = {0};
  int pos = 0;

  /* Loop through all the typed words and combine into 1 string */
  for (size_t i = 1; i < argc; i++) {
    pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%s ", argv[i]);
  }

  display_hal_clear();

  display_hal_set_font_size(24);
  display_hal_print("MESSAGE", 0, 0);

  display_hal_set_font_size(16);
  display_hal_print(buffer, 0, 32);

  display_hal_finalize();

  shell_print(sh, "Success! Sent to OLED: %s", buffer);

  return 0;
}

/*
 * Command to clear screen
*/
static int cmd_oled_clear(const struct shell *sh, size_t argc, size_t **argv) {
  display_hal_clear();
  display_hal_set_font_size(24);
  display_hal_print("MESSAGE", 0, 0);
  display_hal_finalize();
  shell_print(sh, "OLED cleared");
  return 0;
}

/*
 * Register the subcommands
*/
SHELL_STATIC_SUBCMD_SET_CREATE(sub_oled,
                               SHELL_CMD_ARG(msg, NULL, "Print message to OLED (eg: oled msg Hello World", cmd_oled_msg, 2, 255),
                               SHELL_CMD(clear, NULL, "Clear the OLED screen", cmd_oled_clear),
                               SHELL_SUBCMD_SET_END
);

/*
 * Register the root command (oled)
*/
SHELL_CMD_REGISTER(oled, &sub_oled, "OLED display commands", NULL);

int main(void)
{
  if (display_hal_init() != 0) {
    LOG_ERR("Display init failed");
    return -1;
  }

  display_hal_clear();
  display_hal_set_font_size(24);
  display_hal_print("MESSAGE", 0, 0);
  display_hal_finalize();

  // No while(1) loop because the shell automatically
  // runs in a background thread

  return 0;
}
