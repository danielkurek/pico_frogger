#include "button.hpp"
#include <pico/stdlib.h>

Button::Button(uint pin, uint debounce_time_us) 
    : pin_(pin), debounce_time_(debounce_time_us), 
      state_(ButtonState::Released)
{
    last_update_ = get_absolute_time();
    gpio_init(pin_);
    gpio_pull_down(pin_);
    gpio_set_dir(pin_, false);
}

/**
 * Button needs to be `debounce_time` HIGH to return true
 * returns true only one time when the button is held
 */
bool Button::isPressed(absolute_time_t now){
    auto diff = absolute_time_diff_us(last_update_, now);
    bool pressed = gpio_get(pin_);
    if(!pressed){
        last_update_ = now;
        state_ = ButtonState::Released;
        return false;
    }
    if(diff < debounce_time_){
        return false;
    }
    switch(state_){
        case ButtonState::Released:
            state_ = ButtonState::Debouncing;
            last_update_ = now;
            return false;
            break;
        case ButtonState::Debouncing:
            state_ = ButtonState::Pressed;
            last_update_ = now;
            return true;
            break;
        case ButtonState::Pressed:
            return false;
            break;
        default:
            return false;
    }
}