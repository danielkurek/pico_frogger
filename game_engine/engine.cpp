#include "engine.hpp"
#include "ssd1306-spi.h"

void ssd1306_image_blit(ssd1306_t *p, const Image& image, int x_offset, int y_offset){
    for(int i = 0; i < image.width; ++i){
        for(int j = 0; j < image.height; ++j){
            if(image.data[i*image.width + j]==1){
                ssd1306_draw_pixel(p, i+x_offset, j+y_offset);
            }
        }
    }
}

PhysicsObject::PhysicsObject(int x, int y, const Image image, const std::vector<GameObject*>& collision_group) 
    : GameObject(x, y, image)
{

}