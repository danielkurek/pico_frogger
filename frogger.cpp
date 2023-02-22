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

void animation(void) {
    ssd1306_t disp;
    disp.external_vcc=false;
    if(!ssd1306_init(&disp, 128, 64, spi1, SSD1306_SPI_DC, SSD1306_SPI_CSN, SSD1306_SPI_RES)){
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        sleep_ms(50);
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(50);
    }
    static uint8_t img [] = {
        1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,0,0,0,0,0,0,0,0,0,0,0,1,
        1,0,1,1,1,1,1,1,1,1,1,0,1,
        1,0,1,0,0,0,0,0,0,0,1,0,1,
        1,0,1,0,1,1,1,1,1,0,1,0,1,
        1,0,1,0,1,1,1,1,1,0,1,0,1,
        1,0,1,0,1,1,1,1,1,0,1,0,1,
        1,0,1,0,1,1,1,1,1,0,1,0,1,
        1,0,1,0,1,1,1,1,1,0,1,0,1,
        1,0,1,0,0,0,0,0,0,0,1,0,1,
        1,0,1,1,1,1,1,1,1,1,1,0,1,
        1,0,0,0,0,0,0,0,0,0,0,0,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1
    };
    Image image = {img, 13, 13};

    ssd1306_clear(&disp);
    for(;;){
        for(int x=0; x<127-image.width; x+=3) {
            for(int y=0; y<63-image.height; ++y) {
                ssd1306_image_blit(&disp, image, x, y);
                ssd1306_show(&disp);
                sleep_ms(SLEEPTIME);
                ssd1306_clear(&disp);
            }
        }
    }
}


int main()
{
    stdio_init_all();
    setup_gpios();
    animation();

    // // Set up our UART
    // uart_init(UART_ID, BAUD_RATE);
    // // Set the TX and RX pins by using the function select on the GPIO
    // // Set datasheet for more information on function select
    // gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    // gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    

    // // GPIO initialisation.
    // // We will make this GPIO an input, and pull it up by default
    // gpio_init(GPIO);
    // gpio_set_dir(GPIO, GPIO_IN);
    // gpio_pull_up(GPIO);
    

    // // Example of using the HW divider. The pico_divider library provides a more user friendly set of APIs 
    // // over the divider (and support for 64 bit divides), and of course by default regular C language integer
    // // divisions are redirected thru that library, meaning you can just use C level `/` and `%` operators and
    // // gain the benefits of the fast hardware divider.
    // int32_t dividend = 123456;
    // int32_t divisor = -321;
    // // This is the recommended signed fast divider for general use.
    // divmod_result_t result = hw_divider_divmod_s32(dividend, divisor);
    // printf("%d/%d = %d remainder %d\n", dividend, divisor, to_quotient_s32(result), to_remainder_s32(result));
    // // This is the recommended unsigned fast divider for general use.
    // int32_t udividend = 123456;
    // int32_t udivisor = 321;
    // divmod_result_t uresult = hw_divider_divmod_u32(udividend, udivisor);
    // printf("%d/%d = %d remainder %d\n", udividend, udivisor, to_quotient_u32(uresult), to_remainder_u32(uresult));

    // // SPI initialisation. This example will use SPI at 1MHz.
    // spi_init(SPI_PORT, 1000*1000);
    // gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    // gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    // gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    // gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // // Chip select is active-low, so we'll initialise it to a driven-high state
    // gpio_set_dir(PIN_CS, GPIO_OUT);
    // gpio_put(PIN_CS, 1);
    

    // // I2C Initialisation. Using it at 400Khz.
    // i2c_init(I2C_PORT, 400*1000);
    
    // gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    // gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    // gpio_pull_up(I2C_SDA);
    // gpio_pull_up(I2C_SCL);


    // // Interpolator example code
    // interp_config cfg = interp_default_config();
    // // Now use the various interpolator library functions for your use case
    // // e.g. interp_config_clamp(&cfg, true);
    // //      interp_config_shift(&cfg, 2);
    // // Then set the config 
    // interp_set_config(interp0, 0, &cfg);

    // // Timer example code - This example fires off the callback after 2000ms
    // add_alarm_in_ms(2000, alarm_callback, NULL, false);





    return 0;
}
