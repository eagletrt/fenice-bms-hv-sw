#include "blink.h"

#include <string.h>

#define index_blink_run() blink_run(&led_blink);

extern blink_t led_blink;
void index_blink_init(GPIO_TypeDef *led_port, uint16_t led_pin, bool repeat);