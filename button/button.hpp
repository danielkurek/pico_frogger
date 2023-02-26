
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
        Button(uint pin, uint debounce_time_us) : _pin(pin), _debounce_time(debounce_time_us), _state(ButtonState::Released), _last_update(0) {}
        bool isPressed(absolute_time_t pressed); // should be called as frequently as possible
    private:
    uint _pin;
    uint _debounce_time;
    ButtonState _state;
    absolute_time_t _last_update;
};

#endif