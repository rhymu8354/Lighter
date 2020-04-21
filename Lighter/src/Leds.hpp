#pragma once

#include <stdint.h>

namespace Leds {

    extern void TurnOn(
        uint8_t brightness,
        uint8_t red,
        uint8_t green,
        uint8_t blue
    );
    extern void FlashBang(
        uint8_t brightness,
        uint8_t red,
        uint8_t green,
        uint8_t blue
    );
    extern void TurnOff();

}
