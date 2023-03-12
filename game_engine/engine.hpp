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

inline void ssd1306_image_blit(ssd1306_t *p, const Image& image, int x_offset, int y_offset);
void ssd1306_image_blit(ssd1306_t *p, const Image& image, int x_offset, int y_offset, bool loop);

class GameObject{
    public:
        GameObject(int _x, int _y, const Image image, const char (&name)[5]);
        virtual void updateTick(absolute_time_t now) { }
        virtual void draw(ssd1306_t *p);
        void changeImage(const Image image) { image_ = image; }
        int getWidth() { return image_.width; }
        int getHeight() { return image_.height; }
        int x;
        int y;
    protected:
        Image image_;
        char name_[5];
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
        absolute_time_t getLastUpdate() { return last_update_; }
        void synchronize(absolute_time_t other) { last_update_ = other; }
    private:
        bool loop_;
        absolute_time_t last_update_;
    protected:
        const int max_x_;
        const int max_y_;
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
        Button btn_up_;
        Button btn_down_;
        Button btn_left_;
        Button btn_right_;
        Button btn_act_;
        Button btn_bck_;
};

class GameEngine{
    public:
        GameEngine(int width, int height, frog_options_t& frog_options);
        void startGameLoop(ssd1306_t *p);

        void addObject(int x, int y, const Image image, const char (&name)[5]);
        void addCar(int x, int y, const Image image, int step_time_us, MotionVector motion, const char (&name)[5]);
        void addPlatform(int x, int y, const Image image, int step_time_us, MotionVector motion, const char (&name)[5]);
        void addLeaf(int x, int y, const Image image, const char (&name)[5]);

        std::vector<std::shared_ptr<GameObject>> objects;
        std::vector<std::shared_ptr<PhysicsObject>> cars;
        std::vector<std::shared_ptr<PhysicsObject>> platforms;
        std::vector<std::shared_ptr<GameObject>> leaves;
    private:
        bool checkCollisions(ssd1306_t *p);
        int width_;
        int height_;
        absolute_time_t last_time_;
        std::shared_ptr<Frog> frog;
};


#endif