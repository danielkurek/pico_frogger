#include <stdio.h>
#include "pico/stdlib.h"
// #include "hardware/uart.h"
// #include "hardware/gpio.h"
// #include "hardware/divider.h"
// #include "hardware/spi.h"
#include "hardware/spi.h"
// #include "hardware/pio.h"
// #include "hardware/interp.h"
// #include "hardware/timer.h"
// #include "hardware/watchdog.h"
// #include "hardware/clocks.h"

#include "pico_ssd1306/ssd1306-spi.h"
#include "game_engine/engine.hpp"
#include <memory>
#include <inttypes.h>

#define SLEEPTIME 25

// // UART defines
// // By default the stdout UART is `uart0`, so we will use the second one
// #define UART_ID uart1
// #define BAUD_RATE 9600

// // Use pins 4 and 5 for UART1
// // Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
// #define UART_TX_PIN 4
// #define UART_RX_PIN 5

// // GPIO defines
// // Example uses GPIO 2
// #define GPIO 2


// // SPI Defines
// // We are going to use SPI 0, and allocate it to the following GPIO pins
// // Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
// #define SPI_PORT spi0
// #define PIN_MISO 16
// #define PIN_CS   17
// #define PIN_SCK  18
// #define PIN_MOSI 19

// // I2C defines
// // This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// // Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
// #define I2C_PORT i2c0
// #define I2C_SDA 8
// #define I2C_SCL 9


// int64_t alarm_callback(alarm_id_t id, void *user_data) {
//     // Put your timeout handler code in here
//     return 0;
// }

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
    gpio_init(SSD1306_SPI_CSN);
    gpio_put(SSD1306_SPI_CSN, 0);
    gpio_set_dir(SSD1306_SPI_CSN, GPIO_OUT);
    gpio_init(SSD1306_SPI_DC);
    gpio_put(SSD1306_SPI_DC, 0);
    gpio_set_dir(SSD1306_SPI_DC, GPIO_OUT);
    gpio_init(SSD1306_SPI_RES);
    gpio_put(SSD1306_SPI_RES, 0);
    gpio_set_dir(SSD1306_SPI_RES, GPIO_OUT);

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

void game_start(void) {
    ssd1306_t disp;
    disp.external_vcc=false;
    if(!ssd1306_init(&disp, SSD1306_WIDTH, SSD1306_HEIGHT, spi1, SSD1306_SPI_DC, SSD1306_SPI_CSN, SSD1306_SPI_RES)){
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        sleep_ms(50);
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(50);
    }
    static uint8_t truck_img_data [] = {
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,0,1,0,1,0,1,0,1,0,1,
        1,1,1,1,1,0,1,0,1,0,1,0,1,1,
        1,1,1,1,0,1,0,1,0,1,0,1,0,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1
    };
    Image truck_img = {14, 5, truck_img_data};
    static uint8_t platform_img_data [] = {
        0,0,1,1,1,1,1,1,1,1,1,1,0,0,
        0,1,1,0,1,0,1,1,0,1,0,1,1,0,
        1,1,0,1,0,1,0,0,1,0,1,0,1,1,
        0,1,1,0,1,0,1,1,0,1,0,1,1,0,
        0,0,1,1,1,1,1,1,1,1,1,1,0,0
    };
    Image platform_img = {14, 5, platform_img_data};
    ssd1306_clear(&disp);
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
    printf("Boot up");
    sleep_ms(5000);
    printf("Start");
#endif
    engine.add_car(SSD1306_WIDTH - 1, SSD1306_HEIGHT - Frog::frogImage.height - truck_img.height, truck_img, 75000, {-1, 0}, "car1");
    engine.add_platform(SSD1306_WIDTH,28-platform_img.height+1,platform_img, 75000, {-1, 0}, "plt1");

    engine.start_gameloop(&disp);
}


int main()
{
    stdio_init_all();
    setup_gpios();
    game_start();

    return 0;
}
