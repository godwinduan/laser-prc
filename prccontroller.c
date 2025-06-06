#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

const size_t data_size = sizeof(uint16_t);

#define COMMAND_BUF_SIZE 100
#define NUM_CHANNELS 2

char command_buf[COMMAND_BUF_SIZE];
uint16_t input_buf[NUM_CHANNELS];
uint16_t output_buf[NUM_CHANNELS];

int main()
{
    stdio_init_all();

    output_buf[0] = (uint16_t)129;
    output_buf[1] = (uint16_t)3928;
    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    //printf("System ready.\n");

    while (true) {
        gets(command_buf);

        /*
        if (strcmp(command_buf, "one") == 0)
            printf("got one\n");

        if (strcmp(command_buf, "two") == 0)
            printf("got two\n");

        */

        if (strcmp(command_buf, "one") == 0)
            fwrite(output_buf, sizeof(uint16_t), NUM_CHANNELS, stdout);

        if (strcmp(command_buf, "two") == 0)
        {
            output_buf[0]++;
            output_buf[1]++;
            fwrite(output_buf, sizeof(uint16_t), NUM_CHANNELS, stdout);
        }

        if (strcmp(command_buf, "three") == 0)
        {
            printf("hey\n");
        }

        if (strcmp(command_buf, "four") == 0)
        {
            printf("ho");
        }

        char tester[10] = "tester";
        tester[9] = 'r';
        char *ptr = tester;

        uint16_t tester2[10] = {1,2,3,4,5,6,7,8,9,10};
        void *ptr2 = tester2;

        if (strcmp(command_buf, "five") == 0)
        {
            for(int i = 0; i < 10; i++)
                putchar((char) *(ptr2 + i));
        }

    }
}
