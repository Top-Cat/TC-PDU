#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/rmt.h"
#include "frequency.h"

#define FREQ_PIN GPIO_NUM_39
#define FREQ_CHANNEL RMT_CHANNEL_0

float frequency;
gpio_mode_t mode = GPIO_MODE_INPUT;
static TimerHandle_t rmt_timer;

static void rmt_timer_callback(TimerHandle_t xTimer) {
    if (mode == GPIO_MODE_DISABLE) {
        // Callback never happened :(
        mode = GPIO_MODE_INPUT;
        frequency = 0;
    } else {
        mode = GPIO_MODE_DISABLE;
    }
    gpio_set_direction(FREQ_PIN, mode);
}

void rmt_rx_task(void *arg) {
    RingbufHandle_t rb = NULL;
    rmt_get_ringbuf_handle(FREQ_CHANNEL, &rb);
    rmt_rx_start(FREQ_CHANNEL, true);

    while (rb) {
        size_t rx_size = 0;
        rmt_item32_t *items = (rmt_item32_t *)xRingbufferReceive(rb, &rx_size, portMAX_DELAY);
        if (items) {
            size_t num_items = rx_size / sizeof(rmt_item32_t);
            if (num_items > 2) {
                float newFreq = 0;
                uint16_t count = num_items - 2;
                double mult = 2e6 / count;
                for (int i = 0; i < count; i++) {
                    const rmt_item32_t *s = &items[i + 1];
                    newFreq += mult / (s->duration0 + s->duration1);
                }
                frequency = newFreq;
            }

            mode = GPIO_MODE_INPUT;
            gpio_set_direction(FREQ_PIN, mode);
            vRingbufferReturnItem(rb, (void *)items);
        }
    }

    vTaskDelete(NULL);
}

void Frequency::init() {
  uint8_t blocks = 2;
  rmt_config_t rmt_rx_config = {
    .rmt_mode = RMT_MODE_RX,
    .channel = FREQ_CHANNEL,
    .gpio_num = FREQ_PIN,
    .clk_div = 40, // 2MHz
    .mem_block_num = blocks,
    .rx_config = {
      .idle_threshold = 40000, // 20ms
      .filter_ticks_thresh = 200, // ~1us
      .filter_en = true,
    }
  };

  ESP_ERROR_CHECK(rmt_config(&rmt_rx_config));
  ESP_ERROR_CHECK(rmt_driver_install(FREQ_CHANNEL, 64 * blocks * sizeof(rmt_item32_t), 0));

  // Timer manually toggles GPIO to force RMT to end
  // We only have enough blocks for 128 / f seconds
  //  - 2.56 at 50Hz
  //  - 2.13 at 60Hz
  rmt_timer = xTimerCreate("rmt_timer", 1000 / portTICK_PERIOD_MS, true, NULL, rmt_timer_callback);
  xTimerStart(rmt_timer, 0);

  // Create task to wait on RMT results
  xTaskCreate(rmt_rx_task, "rmt_rx_task", 4096, NULL, 10, NULL);
}

float Frequency::getFrequency() {
  return frequency;
}

Frequency freq;
