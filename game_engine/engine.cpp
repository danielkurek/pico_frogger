#include "engine.hpp"
#include "ssd1306-spi.h"
#include <stdio.h>
#include <inttypes.h>
#include <type_traits>
#include "button.hpp"
#include <cstring>

inline void ssd1306_image_blit(ssd1306_t *p, const Image& image, int x_offset, int y_offset){
    ssd1306_image_blit(p, image, x_offset, y_offset, false);
}


// loop = wrap around the image if it is outside of the display
void ssd1306_image_blit(ssd1306_t *p, const Image& image, int x_offset, int y_offset, bool loop){
    for(int i = 0; i < image.width; ++i){
        for(int j = 0; j < image.height; ++j){
            int img_i = i;
            int img_j = j;
            if(image.flip_horizontal){
                img_i = (image.width - 1) - i;
            }
            if(image.flip_vertical){
                img_j = (image.height - 1) - j;
            }
            if(image.data[img_j*image.width + img_i]==1){
                int32_t x = (i+x_offset);
                int32_t y = (j+y_offset);
                if(loop){
                    x = x % p->width;
                    if(x < 0) x += p->width;
                    y = y % p->height;
                    if(y < 0) y += p->height;
                } else{
                    if(x < 0) continue;
                    if(y < 0) continue;
                }

                ssd1306_draw_pixel(p, x, y);
            }
        }
    }
}

GameObject::GameObject(int _x, int _y, const Image image, const char (&name)[5])
    : image_(image), x(_x), y(_y)
{
    strncpy(name_, name, 5);
#ifdef DEBUG_PRINT
    printf("[%s] gameobject constructor", name_);
#endif
}

void GameObject::draw(ssd1306_t *p){
// #ifdef DEBUG_PRINT
//     printf("[%s] drawing at x:%d y:%d\n", name_, x, y);
// #endif
    ssd1306_image_blit(p, image_, x, y);
}

PhysicsObject::PhysicsObject(int x, int y, const int max_x, const int max_y, const Image image, bool loop, const char (&name)[5]) 
    : GameObject(x, y, image, name), loop_(loop), max_x_(max_x), max_y_(max_y)
{
    last_update_ = get_absolute_time(); 
#ifdef DEBUG_PRINT
    printf("[%s] PhysicsObject constructor", name_);
#endif
}

void PhysicsObject::updateTick(absolute_time_t now){
    int64_t time_diff = absolute_time_diff_us(last_update_, now);
    if(time_diff > step_time_us){
        x += motion_vector.x * (time_diff / step_time_us);
        y += motion_vector.y * (time_diff / step_time_us);
        if(loop_){
            if(x >= max_x_) x = x % max_x_;
            else if(x < -image_.width) x = (x % max_x_) + max_x_;
            if(y >= max_y_) y = y % max_y_;
            else if(y < -image_.height) y = (y % max_y_) + max_y_;
        }
        time_diff -= time_diff % step_time_us;
        last_update_ = now;
#ifdef DEBUG_PRINT
        printf("[%s] [%" PRId64 "] update position x:%d y:%d\n", name_, to_us_since_boot(now), x, y);
#endif
    }
}

void PhysicsObject::draw(ssd1306_t *p){
    ssd1306_image_blit(p, image_, x, y, loop_);
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
const Image Frog::frogImage = {5, 5, false, false, Frog::frog_img_data};

Frog::Frog(frog_options_t config) : 
    Frog((config.max_x / 2) - frogImage.width / 2, config.max_y - frogImage.height, config) {}

Frog::Frog(int x, int y, frog_options_t config) : 
    PhysicsObject(x, y, config.max_x, config.max_y, frogImage, false, "frog"),
    btn_up_(config.btn_up_pin, config.debounce_time_us),
    btn_down_(config.btn_down_pin, config.debounce_time_us),
    btn_left_(config.btn_left_pin, config.debounce_time_us),
    btn_right_(config.btn_right_pin, config.debounce_time_us),
    btn_act_(config.btn_act_pin, config.debounce_time_us),
    btn_bck_(config.btn_bck_pin, config.debounce_time_us)
 {}

void Frog::updateTick(absolute_time_t now){
    if(btn_up_.isPressed(now)){
        // -------------------------------------
        // allow clipping by 1 pixel on one side
        // to make the height of the display
        // divisible by 5
        // -------------------------------------
        if(y - image_.height >= -1) y -= image_.height;
    }
    if(btn_down_.isPressed(now)){
        if(y + image_.height <= max_y_ - image_.height) y += image_.height;
    }
    if(btn_left_.isPressed(now)){
        if(x - image_.width >= 0) x -= image_.width;
    }
    if(btn_right_.isPressed(now)){
        if(x + image_.width <= max_x_ - image_.width) x += image_.width;
    }
    if(!(y >= 4 && y <= 28)){
        motion_vector = {0,0};
    }
    PhysicsObject::updateTick(now);
}

GameEngine::GameEngine(int width, int height, frog_options_t& frog_options) : width_(width), height_(height), objects(), cars(), platforms() {
    frog = std::make_shared<Frog>(frog_options);
    objects.push_back(frog);
}


void GameEngine::startGameLoop(ssd1306_t *p){
    last_time_ = get_absolute_time();
    for(;;){
        absolute_time_t now = get_absolute_time();
        for(auto && obj : objects){
            obj->updateTick(now);
        }
        ssd1306_clear(p);
        for(auto && obj : objects){
            obj->draw(p);
        }
        last_time_ = now;
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
        if(!frog->collidesWithObjects(leaves)){
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

void GameEngine::addObject(int x, int y, const Image image, const char (&name)[5]){
    objects.emplace_back(std::make_shared<GameObject>(x, y, image, name));
}
void GameEngine::addCar(int x, int y, const Image image, int step_time_us, MotionVector motion, const char (&name)[5]){
    std::shared_ptr<PhysicsObject> car = std::make_shared<PhysicsObject>(x, y, width_, height_, image, true, name);
    car->step_time_us = step_time_us;
    car->motion_vector = motion;
    objects.push_back(car);
    cars.push_back(car);
}
void GameEngine::addPlatform(int x, int y, const Image image, int step_time_us, MotionVector motion, const char (&name)[5]){
    std::shared_ptr<PhysicsObject> platform = std::make_shared<PhysicsObject>(x, y, width_, height_, image, true, name);
    platform->step_time_us = step_time_us;
    platform->motion_vector = motion;
    objects.push_back(platform);
    platforms.push_back(platform);
}
void GameEngine::addLeaf(int x, int y, const Image image, const char (&name)[5]){
    std::shared_ptr<GameObject> leaf = std::make_shared<GameObject>(x, y, image, name);
    objects.push_back(leaf);
    leaves.push_back(leaf);
}