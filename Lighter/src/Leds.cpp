#include "Leds.hpp"

#include <future>
#include <inttypes.h>
#include <memory>
#include <stdio.h>
#include <thread>
#include <vector>

namespace Leds {

    void FlashBang(
        uint8_t brightness,
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
            [&]{
                static const struct {
                    size_t milliseconds;
                    uint8_t brightness;
                } script[] = {
                    {0, 31},
                    {250, 8},
                    {100, 7},
                    {100, 6},
                    {100, 5},
                    {100, 4},
                    {100, 3},
                    {100, 2},
                    {100, 1},
                    {100, 0},
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
