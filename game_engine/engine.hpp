#ifndef ENGINE_H_
#define ENGINE_H_

#include <pico/stdlib.h>
#include "ssd1306-spi.h"
#include <vector>
#include <stdio.h>
#include <memory>

struct Image{
    uint8_t width;
    uint8_t height;
    uint8_t* data;
};

void ssd1306_image_blit(ssd1306_t *p, const Image& image, int x_offset, int y_offset);

class GameObject{
    public:
        GameObject(int _x, int _y, const Image image);
        virtual void updateTick(absolute_time_t now) { }
        void draw(ssd1306_t *p);
        void changeImage(const Image image) { _image = image; }
        int getWidth() { return _image.width; }
        int getHeight() { return _image.height; }
        int x;
        int y;
    protected:
        Image _image;
};

class PhysicsObject : public GameObject{
    public:
        PhysicsObject(int x, int y, const Image image, bool loop);
        virtual void updateTick(absolute_time_t now) override;
        void setMotionVector(int _x_step, int _y_step);
        void setStepTimeUs(int time_us);
        template<typename T>
        std::shared_ptr<T> collidesWithObjects(const std::vector<std::shared_ptr<T>>& collision_group);
        int getMotionX() { return x_step; }
        int getMotionY() { return y_step; }
    private:
        int step_time_us = 1000;
        int x_step = 0;
        int y_step = 0;
        bool _loop;
        absolute_time_t last_update;
};

class Frog : public PhysicsObject {
    public:
        Frog();
        static uint8_t frog_img_data [];

        static const Image frogImage;
};

class GameEngine{
    public:
        GameEngine(int width, int height);
        void start_gameloop(ssd1306_t *p);

        std::vector<std::shared_ptr<GameObject>> objects;
        std::vector<std::shared_ptr<GameObject>> cars;
        std::vector<std::shared_ptr<PhysicsObject>> platforms;
        std::vector<std::shared_ptr<GameObject>> leaves;
    private:
        int _width;
        int _height;
        absolute_time_t _last_time;
        std::shared_ptr<Frog> frog;
};


#endif