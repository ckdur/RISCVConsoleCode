#include <main.h>
#include <kprintf/kprintf.h>
#include <clkutils/clkutils.h>
#include <spi.h>
#include "bls12_381_test.h"
#include "dilithium_test.h"

uint8_t spi_config[] = {
    // Check signature
    0xBA,
    // SET_DIV (FD = F / DIV)
    6,
    // FCW (F = REF * (FCW+1))
    0x8, // about 240MHz can go 
    // KI
    0x1, 0xFF,
    // KP
    0x0, 0xFF
};
const int spi_config_siz = sizeof(spi_config)/sizeof(uint8_t);

volatile uint32_t* spi_pll = (uint32_t*)0;
volatile uint32_t* gpio = (uint32_t*)0;

// The order of GPIOs are:
/*
    "0", // sf_rst_n (OUT)
    "1", // pll_out_div (IN)
    "2", // pll_sel (OUT)
    "3", // pll_err (IN)
    "4", // pll_locked (IN)
    "5", // pll_rst_n (OUT)
    "6", // pll_mux (IN)
*/

static inline void put_gpio(int i, int val) {
    if(val) gpio[(GPIO_OUTPUT_VAL >> 2)] |= (1 << i);
    else gpio[(GPIO_OUTPUT_VAL >> 2)] &= ~(1 << i);
}

static inline int get_gpio(int i) {
    return ((gpio[(GPIO_INPUT_VAL >> 2)]) >> i) & 0x1;
}

void test() {
    while(spi_pll && gpio) {
        spi_init(spi_pll);
        gpio[(GPIO_INPUT_EN >> 2)] = (1 << 1) | (1 << 3) | (1 << 4) | (1 << 6);
        gpio[(GPIO_OUTPUT_EN >> 2)] = (1 << 0) | (1 << 2) | (1 << 5);
        gpio[(GPIO_PULLUP_EN >> 2)] = 0;

        // Mux into the external clock
        put_gpio(2, 0);

        // Trigger all the resets
        put_gpio(0, 0);
        put_gpio(5, 0);

        // Release the shift register only
        put_gpio(0, 1);

        // Push something
        spi_pll[(SPI_REG_CSMODE >> 2)] = SPI_CSMODE_HOLD;
        // Send a dummy of six
        spi_pll[(SPI_REG_FMT >> 2)] = SPI_FMT_LEN(5) | SPI_FMT_PROTO(SPI_PROTO_S) | SPI_FMT_ENDIAN(SPI_ENDIAN_MSB)  | SPI_FMT_DIR(SPI_DIR_RX);
        spi_xfer(spi_pll, 0x0);
        spi_pll[(SPI_REG_FMT >> 2)] = SPI_FMT_LEN(8) | SPI_FMT_PROTO(SPI_PROTO_S) | SPI_FMT_ENDIAN(SPI_ENDIAN_MSB)  | SPI_FMT_DIR(SPI_DIR_RX);
        // Send the normal string
        uint8_t back;
        for(int i = 0; i < spi_config_siz; i++) 
            back = spi_xfer(spi_pll, spi_config[i]);
        spi_pll[(SPI_REG_CSMODE >> 2)] = SPI_CSMODE_AUTO;
        spi_pll[(SPI_REG_CSMODE >> 2)] = SPI_CSMODE_OFF;
        spi_xfer(spi_pll, 0x0);
        spi_pll[(SPI_REG_CSMODE >> 2)] = SPI_CSMODE_AUTO;
        
        if(back != 0xd0/*0x74*/) {
            // Failed signature
            kprintf("Failed signature in the SPI config (%x)\r\n", (unsigned int) back);
        //    continue;
        }
        
        // Release the PLL reset
        put_gpio(5, 1);

        // Wait for either the locked, or the error
        int locked, err;
        do {
            locked = get_gpio(4);
            err = get_gpio(3);
        } while (!locked && !err);

        if(!err) {
            // If there was no error, put the pll clock
            put_gpio(2, 1);
        }
        else {
            kputs("ERROR: PLL did not lock\r\n");
            continue;
        }
        break;
    }
    dilithium_test();
    bls12_381_test();
}
