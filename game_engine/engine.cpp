#include "engine.hpp"
#include "ssd1306-spi.h"
#include <stdio.h>
#include <inttypes.h>
#include <type_traits>
#include "button.hpp"

void ssd1306_image_blit(ssd1306_t *p, const Image& image, int x_offset, int y_offset){
    for(int i = 0; i < image.width; ++i){
        for(int j = 0; j < image.height; ++j){
            if(image.data[j*image.width + i]==1){
                if(i+x_offset < 0) continue;
                if(j+y_offset < 0) continue;
                ssd1306_draw_pixel(p, i+x_offset, j+y_offset);
            }
        }
    }
}

GameObject::GameObject(int _x, int _y, const Image image){
    _image = image;
    x = _x;
    y = _y;
    printf("gameobject constructor");
}

void GameObject::draw(ssd1306_t *p){
    printf("drawing\n");
    ssd1306_image_blit(p, _image, x, y);
}

PhysicsObject::PhysicsObject(int x, int y, const Image image, bool loop) 
    : GameObject(x, y, image), _loop(loop)
{
    last_update = get_absolute_time(); 
    printf("PhysicsObject constructor");
}

void PhysicsObject::updateTick(absolute_time_t now){
    int64_t time_diff = absolute_time_diff_us(last_update, now);
    if(time_diff > step_time_us){
        x += x_step * (time_diff / step_time_us);
        y += y_step * (time_diff / step_time_us);
        if(_loop){
            if(x > 127) x = x % 128;
            else if(x < -_image.width) x = 127 - ((x+_image.width) % 128);
            if(y > 63) y = y % 64;
            else if(y < -_image.height) y = 63 - (y % 64);
        }
        time_diff -= time_diff % step_time_us;
        last_update = now;
    }
}

void PhysicsObject::setMotionVector(int _x_step, int _y_step){
    x_step = _x_step;
    y_step = _y_step;
}
void PhysicsObject::setStepTimeUs(int time_us){
    step_time_us = time_us;
}
template<typename T>
std::shared_ptr<T> PhysicsObject::collidesWithObjects(const std::vector<std::shared_ptr<T>>& collision_group){
    static_assert(std::is_base_of<GameObject, T>::value, "T must be derived from GameObject");
    int x1 = x;
    int x2 = x + getWidth();
    int y1 = y;
    int y2 = y + getHeight();
    for(auto && other : collision_group){
        int other_x1 = other->x;
        int other_x2 = other->x + other->getWidth();
        int other_y1 = other->y;
        int other_y2 = other->y + other->getHeight();
        if(x2 >= other_x1 && other_x2 >= x1 && y2 >= other_y1 && other_y2 >= y1){
            return other;
        }
    }
    return nullptr;
}

uint8_t Frog::frog_img_data [] = {
            0,1,1,1,0,
            1,0,1,0,1,
            0,1,1,1,0,
            1,1,1,1,1,
            0,1,1,1,0,
        };
const Image Frog::frogImage = {5,5,Frog::frog_img_data};

Frog::Frog(frog_options_t config) : 
    PhysicsObject((127/2)-frogImage.width, 63-frogImage.height, frogImage, false),
    btn_up(config.btn_up_pin, config.debounce_time_us),
    btn_down(config.btn_down_pin, config.debounce_time_us),
    btn_left(config.btn_left_pin, config.debounce_time_us),
    btn_right(config.btn_right_pin, config.debounce_time_us),
    btn_act(config.btn_act_pin, config.debounce_time_us),
    btn_bck(config.btn_bck_pin, config.debounce_time_us)
 {}

void Frog::updateTick(absolute_time_t now){
    if(btn_up.isPressed(now)){
        if(y - _image.height >= 0) y -= _image.height;
    }
    if(btn_down.isPressed(now)){
        if(y + _image.height <= 63 - _image.height) y += _image.height;
    }
    if(btn_left.isPressed(now)){
        if(x - _image.width >= 0) x -= _image.width;
    }
    if(btn_right.isPressed(now)){
        if(x + _image.width <= 127 - _image.width) x += _image.width;
    }
    PhysicsObject::updateTick(now);
}

GameEngine::GameEngine(int width, int height, frog_options_t& frog_options) : _width(width), _height(height), objects(), cars(), platforms() {
    frog = std::make_shared<Frog>(frog_options);
    objects.push_back(frog);
}


void GameEngine::start_gameloop(ssd1306_t *p){
    _last_time = get_absolute_time();
    for(;;){
        absolute_time_t now = get_absolute_time();
        for(auto && obj : objects){
            obj->updateTick(now);
        }
        bool game_over = checkCollisions();
        ssd1306_clear(p);
        for(auto && obj : objects){
            printf("x:%d y:%d\n", obj->x, obj->y);
            obj->draw(p);
        }
        _last_time = now;
        ssd1306_show(p);
        if(game_over) break;
    }
}

bool GameEngine::checkCollisions(){
    if(frog->y > 25){
        if(frog->collidesWithObjects(cars)){
            // game over
            printf("game over - cars");
            return true;
        }
    }
    else if(frog->y > 5){
        std::shared_ptr<PhysicsObject> other = frog->collidesWithObjects(platforms);
        if(other){
            frog->setMotionVector(other->getMotionX(), other->getMotionY());
            frog->setStepTimeUs(75000);
        } else {
            // game over
            printf("game over - platforms");
            return true;
        }
    } else {
        if(frog->collidesWithObjects(leaves)){
            // victory
            printf("victory");
            return true;
        } else {
            // game over
            printf("game over - leaves");
            return true;
        }
    }
    return false;
}