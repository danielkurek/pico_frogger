#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "pico_ssd1306/ssd1306-spi.h"
#include "game_engine/engine.hpp"
#include <memory>
#include <inttypes.h>

#define SLEEPTIME 25

#define SSD1306_SPI_RES 7
#define SSD1306_SPI_DC 8
#define SSD1306_SPI_CSN 9
#define SSD1306_SPI_SCK 10
#define SSD1306_SPI_TX 11

#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64

void setup_gpios(void) {
    spi_init(spi1, 10 * 1024 * 1024);
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(SSD1306_SPI_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SSD1306_SPI_TX, GPIO_FUNC_SPI);
    const uint additional_spi_pins_mask = 1 << SSD1306_SPI_CSN | 1 << SSD1306_SPI_DC | 1 << SSD1306_SPI_RES;
    gpio_init_mask(additional_spi_pins_mask);
    // gpio_pull_up(SSD1306_SPI_CSN);
    // gpio_pull_up(SSD1306_SPI_DC);
    // gpio_pull_up(SSD1306_SPI_RES);
    gpio_set_dir_out_masked(additional_spi_pins_mask);
    gpio_put_masked(additional_spi_pins_mask, 0);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

void draw_number(ssd1306_t *p, int x, int y, int number){
    do{
        int remainder = number % 10;
        number = number / 10;
        ssd1306_draw_char(p, x, y, 1, '0' + remainder);
        x -= 6;
    }while(number > 0);
}

void game_start(ssd1306_t *disp) {
    static uint8_t truck_img_data [] = {
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,0,1,0,1,0,1,0,1,0,1,
        1,1,1,1,1,0,1,0,1,0,1,0,1,1,
        1,1,1,1,0,1,0,1,0,1,0,1,0,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1
    };
    Image truck_img = {14, 5, false, false, truck_img_data};
    static uint8_t platform_img_data [] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,1,1,1,1,1,1,1,1,1,1,1,1,0,
        1,0,0,0,0,0,0,0,0,0,0,0,0,1,
        0,1,1,1,1,1,1,1,1,1,1,1,1,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };
    Image platform_img = {14, 5, false, false, platform_img_data};

    static uint8_t leaf_img_data [] = {
        1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,
        1,0,1,0,1,0,1
    };
    Image leaf_img = {7, 4, false, false, leaf_img_data};
    ssd1306_clear(disp);
    frog_options_t frog_options = {
        btn_up_pin: 0,
        btn_down_pin: 1,
        btn_left_pin: 2,
        btn_right_pin: 3,
        btn_act_pin: 5,
        btn_bck_pin: 4,
        debounce_time_us: 10000,
        max_x: SSD1306_WIDTH,
        max_y: SSD1306_HEIGHT
    };
    GameEngine engine{SSD1306_WIDTH, SSD1306_HEIGHT, frog_options};
#ifdef DEBUG_PRINT
    printf("Start");
#endif
    int cars_offset[] = {1, 50, 100, 75, 25};
    size_t cars_length = sizeof(cars_offset) / sizeof(cars_offset[0]);
    for(int i = 0; i < cars_length; i++){
        MotionVector motion {-1, 0};
        truck_img.flip_horizontal = false;
        if(i % 2 == 1){
            motion.x *= -1;
            truck_img.flip_horizontal = !truck_img.flip_horizontal;
        }
        char name[5] = "car0";
        name[3] = '0' + i;
        int height = SSD1306_HEIGHT - (i+1) * Frog::frogImage.height - truck_img.height;
        engine.addCar(SSD1306_WIDTH - cars_offset[i], height, truck_img, 75000, motion, std::move(name));
    }

    int platforms_offset[] = {0, 30, 20, 75, 50};
    size_t platforms_length = sizeof(platforms_offset) / sizeof(platforms_offset[0]);
    for(int i = 0; i < platforms_length; i++){
        MotionVector motion {-1, 0};
        if(i % 2 == 1){
            motion.x *= -1;
        }
        char name[5] = "plt0";
        name[3] = '0' + i;
        int height = SSD1306_HEIGHT - (cars_length + 2 + i) * Frog::frogImage.height - platform_img.height;
        engine.addPlatform(SSD1306_WIDTH - platforms_offset[i], height, platform_img, 75000, motion, std::move(name));
    }

    for(int x = 0; x < SSD1306_WIDTH; x += SSD1306_WIDTH / 7){
        char name[5] = "lef0";
        name[3] = '0' + x;
        engine.addLeaf(x, 0, leaf_img, "leaf");
    }

    engine.startGameLoop(disp);
}


int main()
{
    stdio_init_all();
    setup_gpios();
#ifdef DEBUG_PRINT
    sleep_ms(3000);
    printf("Boot up");
#endif
    Button btn_start {5, 10000};
    ssd1306_t disp;
    disp.external_vcc=false;
    if(!ssd1306_init(&disp, SSD1306_WIDTH, SSD1306_HEIGHT, spi1, SSD1306_SPI_DC, SSD1306_SPI_CSN, SSD1306_SPI_RES)){
        while(true){
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
            sleep_ms(50);
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            sleep_ms(50);
        }
    }
    ssd1306_clear(&disp);
    ssd1306_draw_string(&disp, 20, 15, 2, "FROGGER");
    ssd1306_draw_string(&disp, 7, 35, 1, "Press ACT to start");
    ssd1306_show(&disp);
    while(true){
        absolute_time_t now = get_absolute_time();
        if(btn_start.isPressed(now))
            game_start(&disp);
    }
    return 0;
}
