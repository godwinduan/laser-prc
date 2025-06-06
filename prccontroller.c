#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

// Pin to toggle the MCP4822 ADC to output both ADCs at once
#define PIN_LDAC 20

#define COMMAND_BUF_SIZE 100
#define NUM_CHANNELS 2

const char delimiter[] = ",";
char command_buf[COMMAND_BUF_SIZE];
uint16_t input_buf[NUM_CHANNELS];
uint16_t output_buf[NUM_CHANNELS];

static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}

int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    spi_set_format(SPI_PORT, 16, 0, 0, SPI_MSB_FIRST); // 16 bits sent to the DAC at a time
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    cs_deselect();
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    // Initialize ADC
    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);

    // Initialize DAC
    gpio_set_dir(PIN_LDAC, GPIO_OUT);
    // gpio_put(PIN_LDAC, 0); // If simultaneous DAC output isn't needed, use this instead
    gpio_put(PIN_LDAC, 1);


    while (true) {
        int i = 0;
        gets(command_buf);

        // Parse input values for the DAC
        char *tok = strtok(command_buf, delimiter);
        while (tok != NULL)
        {
            input_buf[i] = atoi(tok);
            tok = strtok(NULL, delimiter);
            i++;
        }

        // Send input values back out as a test
        //output_buf[0] = input_buf[0];
        //output_buf[1] = input_buf[1];

        // 
        // Initialization bits for the MCP4822 (12-BIT DAC)

        // A/B: DACA or DACB Selection bit
        // 1 = Write to DACB
        // 0 = Write to DACA
        uint16_t ab_selection_bit = 0 << 15; // First write to DAC A

        // GA: Output Gain Selection bit
        // 1 = 1x (VOUT = VREF * D/4096)
        // 0 = 2x (VOUT = 2 * VREF * D/4096), where internal VREF = 2.048V
        uint16_t ga_selection_bit = 1 << 13;

        // SHDN: Output Shutdown Control bit
        // 1 = Active mode operation. VOUT is available.
        // 0 = Shutdown the selected DAC channel. Analog output is not available at the channel that was shut down. VOUT pin is connected to 500 kOhm typical)
        uint16_t shdn_selection_bit = 1 << 12;

        // Add the 4 initialization bits sent to DAC
        input_buf[0] = input_buf[0] | ab_selection_bit | ga_selection_bit | shdn_selection_bit;
        ab_selection_bit = 1 << 15; // Next write to DAC B
        input_buf[1] = input_buf[1] | ab_selection_bit | ga_selection_bit | shdn_selection_bit;

        // Write to DACA
        cs_select();
        spi_write16_blocking(SPI_PORT, &(input_buf[0]), 1);
        cs_deselect();

        // pause needed(?) datasheet says CS must be high for at least 15ns
        // sleep_us(1);

        // Write to DACB
        cs_select();
        spi_write16_blocking(SPI_PORT, &(input_buf[1]), 1);
        cs_deselect();

        // Pulse LDAC low to update 2 DAC outputs simultaneously
        // pause needed(?) datasheet says 40ns needed after CS deselect
        gpio_put(PIN_LDAC, 0);
        sleep_us(1); // Datasheet says 100ns min pulse width
        gpio_put(PIN_LDAC, 1);

        // Wait a bit for signal to stabilize
        sleep_us(10);
        //sleep_ms(1);

        // Read ADCs
        adc_select_input(0);
        output_buf[0] = adc_read();
        adc_select_input(1);
        output_buf[1] = adc_read();
        
        // Send ADC readings back to PC
        for (i = 0; i < NUM_CHANNELS; i++)
        {
            printf("%u", output_buf[i]);
            if (i + 1 < NUM_CHANNELS)
                printf(",");
        }
        printf("\n");

    }
}
