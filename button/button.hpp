
#ifndef BUTTON_HPP_
#define BUTTON_HPP_

#include <pico/stdlib.h>

enum class ButtonState{
    Released,
    Debouncing,
    Pressed
};
class Button{
    public:
        Button(uint pin, uint debounce_time_us);
        bool isPressed(absolute_time_t pressed); // should be called as frequently as possible
    private:
        uint pin_;
        uint debounce_time_;
        ButtonState state_;
        absolute_time_t last_update_;
};

#endif