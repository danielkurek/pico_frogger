#include "button.hpp"
#include <pico/stdlib.h>

Button::Button(uint pin, uint debounce_time_us) 
    : _pin(pin), _debounce_time(debounce_time_us), 
      _state(ButtonState::Released), _last_update(0) 
{
    gpio_init(_pin);
    gpio_pull_down(_pin);
    gpio_set_dir(_pin, false);
}

/**
 * Button needs to be `debounce_time` HIGH to return true
 * returns true only one time when the button is held
 */
bool Button::isPressed(absolute_time_t now){
    auto diff = absolute_time_diff_us(_last_update, now);
    bool pressed = gpio_get(_pin);
    if(!pressed){
        _last_update = now;
        _state = ButtonState::Released;
        return false;
    }
    if(diff < _debounce_time){
        return false;
    }
    switch(_state){
        case ButtonState::Released:
            _state = ButtonState::Debouncing;
            _last_update = now;
            return false;
            break;
        case ButtonState::Debouncing:
            _state = ButtonState::Pressed;
            _last_update = now;
            return true;
            break;
        case ButtonState::Pressed:
            return false;
            break;
        default:
            return false;
    }
}