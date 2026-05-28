#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/smf.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(smf_logger, LOG_LEVEL_INF);

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});
static struct gpio_callback button_cb_data;

// Define the event for state transitions 
K_EVENT_DEFINE(smf_events);
#define BTN_PRESS_EVT BIT(0)

/*
  * State Machine Context
  * This wrapper holds the zephyr smf_ctx and any custom variables
  * your states might need to share
*/
struct s_object {
  struct smf_ctx ctx;
};
static struct s_object smf_context;

// Define the states 
enum smf_states {
  STATE_IDLE,
  STATE_ACTIVE,
};

static const struct smf_state states[];

/*
  * IDLE state functions
*/
static void idle_entry(void *o) {
  LOG_INF("Enter IDLE state -> LED OFF");
  gpio_pin_set_dt(&led, 0);
}

static enum smf_state_result idle_run(void *o) {
  // Block the state machine until button is pressed 
  uint32_t events = k_event_wait(&smf_events, BTN_PRESS_EVT, false, K_FOREVER);

  if (events & BTN_PRESS_EVT) {
    k_event_clear(&smf_events, BTN_PRESS_EVT);
    smf_set_state(SMF_CTX(&smf_context), &states[STATE_ACTIVE]);
  }

  return SMF_EVENT_HANDLED;
}

/*
  * ACTIVE state functions
*/  
static void active_entry(void *o) {
  LOG_INF("Enter ACTIVE state -> LED ON");
  gpio_pin_set_dt(&led, 1);
}

static enum smf_state_result active_run(void *o) {
  // Block the state machine until button is pressed 
  uint32_t events = k_event_wait(&smf_events, BTN_PRESS_EVT, false, K_FOREVER);

  if (events & BTN_PRESS_EVT) {
    k_event_clear(&smf_events, BTN_PRESS_EVT);
    smf_set_state(SMF_CTX(&smf_context), &states[STATE_IDLE]);
  }

  return SMF_EVENT_HANDLED;
}

// Populate the states table 
static const struct smf_state states[] = {
  [STATE_IDLE]   = SMF_CREATE_STATE(idle_entry, idle_run, NULL, NULL, NULL),
  [STATE_ACTIVE]   = SMF_CREATE_STATE(active_entry, active_run, NULL, NULL, NULL),
};

// ISR
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  k_event_post(&smf_events, BTN_PRESS_EVT);
}

int main(void)
{
  int ret;

  if (!gpio_is_ready_dt(&led) || !gpio_is_ready_dt(&button)) {
    LOG_ERR("Hardware not ready");
    return 0;
  }

  ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
  if (ret < 0) {
    return 0;
  }

  ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
  if (ret < 0) {
    return 0;
  }

  ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
  if (ret != 0) {
    return 0;
  }

  gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
  gpio_add_callback(button.port, &button_cb_data);

  smf_set_initial(SMF_CTX(&smf_context), &states[STATE_IDLE]);

  LOG_INF("SMF initialized. Press button to transition states");

  while (1) {
    ret = smf_run_state(SMF_CTX(&smf_context));
    if (ret) {
      LOG_ERR("SMF Error: %d", ret);
      break;
    }
  }

  return 0;
}
