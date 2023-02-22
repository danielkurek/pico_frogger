#ifndef ENGINE_H_
#define ENGINE_H_

#include <pico/stdlib.h>
#include "ssd1306-spi.h"
#include <vector>

struct Image{
    uint8_t *data;
    uint8_t width;
    uint8_t height;
};

void ssd1306_image_blit(ssd1306_t *p, const Image& image, int x_offset, int y_offset);

class GameObject{
    public:
        GameObject(int x, int y, const Image image);
        void updateTick(int ms_since_last_update) {}
        void draw(){}
        void changeImage(const Image image){}
        int x;
        int y;
    private:
        Image image;
};

class PhysicsObject : public GameObject{
    public:
        PhysicsObject(int x, int y, const Image image, const std::vector<GameObject*>& collision_group);
};

class GameEngine{
    public:
        GameEngine(int width, int height){}

        void start_gameloop(void* (step)){}

    private:
        std::vector<GameObject> _objects;
        std::vector<GameObject*> _collisionGroup;
        int width;
        int height;
};


#endif