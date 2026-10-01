#include <main.h>
#include <kprintf/kprintf.h>
#include <clkutils/clkutils.h>
#include <spi.h>
#include <stdint.h>
#include <string.h>

#include "main.h"

volatile uint32_t* spi = (uint32_t*)0;
volatile uint32_t* gpio = (uint32_t*)0;
volatile uint32_t* adc = (uint32_t*)0;

static inline void put_gpio(int i, int val) {
    if(val) gpio[(GPIO_OUTPUT_VAL >> 2)] |= (1 << i);
    else gpio[(GPIO_OUTPUT_VAL >> 2)] &= ~(1 << i);
}

static inline int get_gpio(int i) {
    return ((gpio[(GPIO_INPUT_VAL >> 2)]) >> i) & 0x1;
}

static void write_spix(int addr, uint8_t val) {
    // Push something
    spi[(SPI_REG_CSMODE >> 2)] = SPI_CSMODE_HOLD;
    // Send a dummy of six
    spi[(SPI_REG_FMT >> 2)] = SPI_FMT_LEN(5) | SPI_FMT_PROTO(SPI_PROTO_S) | SPI_FMT_ENDIAN(SPI_ENDIAN_MSB)  | SPI_FMT_DIR(SPI_DIR_RX);

    // Reverse the bits from MSB to LSB
    int addr_inv = ((addr & 0x1) << 2) | (addr & 0x2) | ((addr & 0x4) >> 2);

    // Please note that this is MSB to LSB
    //            write    , read     ,  addr[0:2]
    spi_xfer(spi, (0 << 7) | (1 << 6) | (addr_inv << 3));
    spi[(SPI_REG_FMT >> 2)] = SPI_FMT_LEN(8) | SPI_FMT_PROTO(SPI_PROTO_S) | SPI_FMT_ENDIAN(SPI_ENDIAN_MSB)  | SPI_FMT_DIR(SPI_DIR_RX);

    spi_xfer(spi, val);
    spi[(SPI_REG_CSMODE >> 2)] = SPI_CSMODE_AUTO;

    // Push something while not activated
    spi[(SPI_REG_CSMODE >> 2)] = SPI_CSMODE_OFF;
    spi_xfer(spi, 0x0);
    spi[(SPI_REG_CSMODE >> 2)] = SPI_CSMODE_AUTO;
}

void test() {
    kputs("\r\n\n\nWelcome! Doing the test!\r\n\n");

    if(!gpio) {
        kputs("No GPIO detected!\r\n");
    }
    if(!spi) {
        kputs("No SPI detected!\r\n");
    }
    if(!adc) {
        kputs("No ADC detected!\r\n");
    }

    if(spi && gpio) {
        spi_init(spi);

        gpio[(GPIO_INPUT_EN >> 2)] = 0;
        gpio[(GPIO_OUTPUT_EN >> 2)] = (1 << 14) | (1 << 15);
        gpio[(GPIO_PULLUP_EN >> 2)] = 0;

        int resetval = 0;
        put_gpio(15, 1); // Set Reset to 1
        put_gpio(15, 0); // Set Reset to 0
        put_gpio(15, 1); // Set Reset to 1
        put_gpio(15, 0); // Set Reset to 0

        put_gpio(14, 1); // Set Go in 1

        if(0) { // Commented out
            put_gpio(15, 1); // Set Reset to 1
            put_gpio(15, 0); // Set Reset to 0
            put_gpio(15, 1); // Set Reset to 1
            put_gpio(15, 0); // Set Reset to 0

            write_spix(0, 0x6); // Activate RO13 (0b0110 = 0x6)
            write_spix(1, 0x6); // Activate RO13 (0b0110 = 0x6)

            kputs("spix done, waiting for 1 second...\r\n");

            // Wait for 1000 ms 
            clkutils_delay_ns(1000000000, 1000000000 / timescale_freq);
        }
    }

    if(adc) {
        kputs("ADC detected!\r\n");
        adc[1] = 0x001; // Enable the ADC
        adc[1] = 0x101; // Enable the ADC
        adc[1] = 0x001; // Enable the ADC
        while(1) {
            int32_t adcval = adc[0];
            if(adcval & 0x80000000) continue; // Fifo empty
            kputs("ADC value: ");
            kput_hex(adcval);
            kputs("\r\n");
        }
    }
    kputs("Finished!\r\n");
}