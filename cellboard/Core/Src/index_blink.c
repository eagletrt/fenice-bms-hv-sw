#include "index_blink.h"
#include "cellboard_config.h"
#include "main.h"

blink_t led_blink;

void index_blink_init(GPIO_TypeDef *led_port, uint16_t led_pin, bool repeat) {
  led_blink.index = 0;
  led_blink.port = led_port;
  led_blink.pin = led_pin;
  led_blink.time = HAL_GetTick();

  HAL_GPIO_WritePin(led_port, led_pin, GPIO_PIN_SET);

  uint16_t blink_pattern[2*CELLBOARD_COUNT] = {0};
  uint8_t pattern_count = 0, cellboard_count = 0;

  while (cellboard_count < cellboard_index) {
      blink_pattern[pattern_count++] = 150;  // On
      blink_pattern[pattern_count++] = 250;  // Off
      cellboard_count++;
  }
  blink_pattern[pattern_count-1] = 1000;  // Big off

  BLINK_SET_PATTERN(led_blink, blink_pattern, pattern_count);
  BLINK_SET_ENABLE(led_blink, true);
  BLINK_SET_REPEAT(led_blink, repeat);
}