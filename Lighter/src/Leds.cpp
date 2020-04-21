#include "Leds.hpp"

#include <future>
#include <inttypes.h>
#include <memory>
#include <stdio.h>
#include <thread>
#include <vector>

namespace Leds {

    void FlashBang(
        uint8_t red,
        uint8_t green,
        uint8_t blue
    ) {
        static std::future< void > flashBangFuture;
        if (flashBangFuture.valid()) {
            flashBangFuture.wait();
        }
        flashBangFuture = std::async(
            std::launch::async,
            [red, green, blue]{
                static const struct {
                    size_t milliseconds;
                    uint8_t brightness;
                } script[] = {
                    {0, 31},
                    {250, 8},
                    {50, 7},
                    {50, 6},
                    {50, 5},
                    {50, 4},
                    {50, 3},
                    {50, 2},
                    {50, 1},
                    {50, 0},
                };
                constexpr auto numSteps = sizeof(script) / sizeof(*script);
                for (size_t i = 0; i < numSteps; ++i) {
                    const auto& step = script[i];
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(step.milliseconds)
                    );
                    TurnOn(
                        step.brightness,
                        red,
                        green,
                        blue
                    );
                }
            }
        );
    }

}
