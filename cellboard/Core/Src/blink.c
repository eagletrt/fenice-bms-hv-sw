/**
 * @file blink.c
 * @brief Blink a led as many times as the index of the current cellboard
 * 
 * @date Feb 21, 2024
 * 
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "blink.h"

#include <stdbool.h>

#include "../../../fenice_config.h"
#include "main.h"

Blinky blinker;
uint16_t pattern[2 * (CELLBOARD_COUNT - 1)];

void blink_init(void) {
    // If the index is 0 the led is always on
    size_t pattern_size = 0;
    if (cellboard_index > 0) {
        // Create pattern
        for (size_t i = 0; i < cellboard_index; ++i) {
            pattern[pattern_size++] = 150; // On (in ms)
            pattern[pattern_size++] = 250; // Off (in ms)
        }
        pattern[pattern_size - 1] += 750; // Pause between patterns (in ms)
    }

    blinky_init(
        &blinker,
        pattern,
        pattern_size,
        true,
        BLINKY_HIGH
    );

    if (cellboard_index == 0)
        blinky_enable(&blinker, false);
}

BlinkyState blink_routine(uint32_t t) {
    return blinky_routine(&blinker, t);
}
