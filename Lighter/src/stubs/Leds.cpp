#include "../Leds.hpp"

#include <future>
#include <inttypes.h>
#include <memory>
#include <stdio.h>
#include <thread>
#include <vector>

namespace Leds {

    void TurnOn(
        uint8_t brightness,
        uint8_t red,
        uint8_t green,
        uint8_t blue
    ) {
        printf("LIGHT 'EM UP!\n");
        printf("Brightness: %" PRIu8 "\n", brightness);
        printf("Color: #%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "\n", red, green, blue);
    }

    void TurnOff() {
        printf("THANK YOU, I ALMOST WENT BLIND THERE!\n");
    }

}
