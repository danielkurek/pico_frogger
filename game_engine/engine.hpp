#ifndef ENGINE_H_
#define ENGINE_H_

#include <pico/stdlib.h>
#include "ssd1306-spi.h"
#include <vector>
#include <stdio.h>
#include <memory>
#include "button.hpp"

struct Image{
    uint8_t width;
    uint8_t height;
    bool flip_horizontal;
    bool flip_vertical;
    uint8_t* data;
};

struct MotionVector{
    int x;
    int y;
};

void ssd1306_image_blit(ssd1306_t *p, const Image& image, int x_offset, int y_offset);
void ssd1306_image_blit(ssd1306_t *p, const Image& image, int x_offset, int y_offset, bool loop);

class GameObject{
    public:
        GameObject(int _x, int _y, const Image image, const char (&name)[5]);
        virtual void updateTick(absolute_time_t now) { }
        virtual void draw(ssd1306_t *p);
        void changeImage(const Image image) { _image = image; }
        int getWidth() { return _image.width; }
        int getHeight() { return _image.height; }
        int x;
        int y;
    protected:
        Image _image;
        char _name[5];
};

class PhysicsObject : public GameObject{
    public:
        PhysicsObject(int x, int y, const int max_x, const int max_y, const Image image, bool loop, const char (&name)[5]);
        virtual void updateTick(absolute_time_t now) override;
        virtual void draw(ssd1306_t *p) override;
        template<typename T>
        std::shared_ptr<T> collidesWithObjects(const std::vector<std::shared_ptr<T>>& collision_group);
        MotionVector motion_vector {0,0};
        int step_time_us = 1000;
        absolute_time_t getLastUpdate() { return last_update; }
        void synchronize(absolute_time_t other) { last_update = other; }
    private:
        bool _loop;
        absolute_time_t last_update;
    protected:
        const int _max_x;
        const int _max_y;
};

struct frog_options_t{
    uint btn_up_pin;
    uint btn_down_pin;
    uint btn_left_pin;
    uint btn_right_pin;
    uint btn_act_pin;
    uint btn_bck_pin;
    uint debounce_time_us;
    int max_x;
    int max_y;
};

class Frog : public PhysicsObject {
    public:
        Frog(frog_options_t config);
        Frog(int x, int y, frog_options_t config);
        void updateTick(absolute_time_t now) override;
        static uint8_t frog_img_data [];
        static const Image frogImage;
    private:
        Button btn_up;
        Button btn_down;
        Button btn_left;
        Button btn_right;
        Button btn_act;
        Button btn_bck;
};

class GameEngine{
    public:
        GameEngine(int width, int height, frog_options_t& frog_options);
        void start_gameloop(ssd1306_t *p);

        void add_object(int x, int y, const Image image, const char (&name)[5]);
        void add_car(int x, int y, const Image image, int step_time_us, MotionVector motion, const char (&name)[5]);
        void add_platform(int x, int y, const Image image, int step_time_us, MotionVector motion, const char (&name)[5]);
        void add_leaf(int x, int y, const Image image, const char (&name)[5]);

        std::vector<std::shared_ptr<GameObject>> objects;
        std::vector<std::shared_ptr<PhysicsObject>> cars;
        std::vector<std::shared_ptr<PhysicsObject>> platforms;
        std::vector<std::shared_ptr<GameObject>> leaves;
    private:
        bool checkCollisions(ssd1306_t *p);
        int _width;
        int _height;
        absolute_time_t _last_time;
        std::shared_ptr<Frog> frog;
};


#endif