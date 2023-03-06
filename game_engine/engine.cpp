#include "engine.hpp"
#include "ssd1306-spi.h"
#include <stdio.h>
#include <inttypes.h>
#include <type_traits>
#include "button.hpp"
#include <cstring>

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

GameObject::GameObject(int _x, int _y, const Image image, const char (&name)[5])
    : _image(image), x(_x), y(_y)
{
    strncpy(_name, name, 5);
#ifdef DEBUG_PRINT
    printf("[%s] gameobject constructor", _name);
#endif
}

void GameObject::draw(ssd1306_t *p){
// #ifdef DEBUG_PRINT
//     printf("[%s] drawing at x:%d y:%d\n", _name, x, y);
// #endif
    ssd1306_image_blit(p, _image, x, y);
}

PhysicsObject::PhysicsObject(int x, int y, const int max_x, const int max_y, const Image image, bool loop, const char (&name)[5]) 
    : GameObject(x, y, image, name), _loop(loop), _max_x(max_x), _max_y(max_y)
{
    last_update = get_absolute_time(); 
#ifdef DEBUG_PRINT
    printf("[%s] PhysicsObject constructor", _name);
#endif
}

void PhysicsObject::updateTick(absolute_time_t now){
    int64_t time_diff = absolute_time_diff_us(last_update, now);
    if(time_diff > step_time_us){
        x += motion_vector.x * (time_diff / step_time_us);
        y += motion_vector.y * (time_diff / step_time_us);
        if(_loop){
            if(x >= _max_x) x = x % _max_x;
            else if(x < -_image.width) x = _max_x - ((x+_image.width) % _max_x) - 1;
            if(y >= _max_y) y = y % _max_y;
            else if(y < -_image.height) y = _max_y - (y % _max_y) - 1;
        }
        time_diff -= time_diff % step_time_us;
        last_update = now;
#ifdef DEBUG_PRINT
        printf("[%s] [%" PRId64 "] update position x:%d y:%d\n", _name, to_us_since_boot(now), x, y);
#endif
    }
}

template<typename T>
std::shared_ptr<T> PhysicsObject::collidesWithObjects(const std::vector<std::shared_ptr<T>>& collision_group){
    static_assert(std::is_base_of<GameObject, T>::value, "T must be derived from GameObject");
    int x1 = x;
    int x2 = x + getWidth() - 1;
    int y1 = y;
    int y2 = y + getHeight() - 1;
    for(auto && other : collision_group){
        int other_x1 = other->x;
        int other_x2 = other->x + other->getWidth() - 1;
        int other_y1 = other->y;
        int other_y2 = other->y + other->getHeight() - 1;
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
    Frog((config.max_x / 2) - frogImage.width / 2, config.max_y - frogImage.height, config) {}

Frog::Frog(int x, int y, frog_options_t config) : 
    PhysicsObject(x, y, config.max_x, config.max_y, frogImage, false, "frog"),
    btn_up(config.btn_up_pin, config.debounce_time_us),
    btn_down(config.btn_down_pin, config.debounce_time_us),
    btn_left(config.btn_left_pin, config.debounce_time_us),
    btn_right(config.btn_right_pin, config.debounce_time_us),
    btn_act(config.btn_act_pin, config.debounce_time_us),
    btn_bck(config.btn_bck_pin, config.debounce_time_us)
 {}

void Frog::updateTick(absolute_time_t now){
    if(btn_up.isPressed(now)){
        // -------------------------------------
        // allow clipping by 1 pixel on one side
        // to make the height of the display
        // divisible by 5
        // -------------------------------------
        if(y - _image.height >= -1) y -= _image.height;
    }
    if(btn_down.isPressed(now)){
        if(y + _image.height <= _max_y - _image.height) y += _image.height;
    }
    if(btn_left.isPressed(now)){
        if(x - _image.width >= 0) x -= _image.width;
    }
    if(btn_right.isPressed(now)){
        if(x + _image.width <= _max_x - _image.width) x += _image.width;
    }
    if(!(y >= 4 && y <= 28)){
        motion_vector = {0,0};
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
        ssd1306_clear(p);
        for(auto && obj : objects){
            obj->draw(p);
        }
        _last_time = now;
        bool game_over = checkCollisions(p);
        ssd1306_show(p);
        if(game_over) break;
    }
}

bool GameEngine::checkCollisions(ssd1306_t *p){
    if(cars.size() > 0 && frog->y >= 34 && frog->y <= 58){
        if(frog->collidesWithObjects(cars)){
            // game over
            ssd1306_draw_string(p, 5, 20, 2, "GAME OVER!");
#ifdef DEBUG_PRINT
            printf("game over - cars\n");
#endif
            return true;
        }
    }
    else if(platforms.size() > 0 && frog->y >= 4 && frog->y <= 28){
        std::shared_ptr<PhysicsObject> other = frog->collidesWithObjects(platforms);
        if(other){
            frog->motion_vector = other->motion_vector;
            frog->step_time_us = other->step_time_us;
            frog->synchronize(other->getLastUpdate());
        } else {
            // game over
            ssd1306_draw_string(p, 5, 20, 2, "GAME OVER!");
#ifdef DEBUG_PRINT
            printf("game over - platforms\n");
#endif
            return true;
        }
    } else if(leaves.size() > 0 && frog->y <= 3){
        if(frog->collidesWithObjects(leaves)){
            ssd1306_draw_string(p, 20, 20, 2, "VICTORY!");
            // victory
#ifdef DEBUG_PRINT
            printf("victory\n");
#endif
            return true;
        } else {
            // game over
            ssd1306_draw_string(p, 5, 20, 2, "GAME OVER!");
#ifdef DEBUG_PRINT
            printf("game over - leaves\n");
#endif
            return true;
        }
    }
    return false;
}

void GameEngine::add_object(int x, int y, const Image image, const char (&name)[5]){
    objects.emplace_back(std::make_shared<GameObject>(x, y, image, name));
}
void GameEngine::add_car(int x, int y, const Image image, int step_time_us, MotionVector motion, const char (&name)[5]){
    std::shared_ptr<PhysicsObject> car = std::make_shared<PhysicsObject>(x, y, _width, _height, image, true, name);
    car->step_time_us = step_time_us;
    car->motion_vector = motion;
    objects.push_back(car);
    cars.push_back(car);
}
void GameEngine::add_platform(int x, int y, const Image image, int step_time_us, MotionVector motion, const char (&name)[5]){
    std::shared_ptr<PhysicsObject> platform = std::make_shared<PhysicsObject>(x, y, _width, _height, image, true, name);
    platform->step_time_us = step_time_us;
    platform->motion_vector = motion;
    objects.push_back(platform);
    platforms.push_back(platform);
}
void GameEngine::add_leaf(int x, int y, const Image image, const char (&name)[5]){
    std::shared_ptr<GameObject> leaf = std::make_shared<GameObject>(x, y, image, name);
    objects.push_back(leaf);
    leaves.push_back(leaf);
}