#include <main.h>
#include <kprintf/kprintf.h>
#include <clkutils/clkutils.h>
#include <spi.h>
#include "bls12_381_test.h"
#include "dilithium_test.h"

// Run on simulation using: 

// Simulate the comms from FPGA/CHIP
// make -C sims/vcs run-binary-debug CONFIG=TomohiroConfig BINARY=~/Documents/RISCVConsoleCode/build/out.elf LOADMEM=1 TIMEOUT_CYCLES=0 EXTRA_SIM_PREPROC_DEFINES="+define+UNIT_DELAY=1"
// make -C sims/verilator run-binary-debug CONFIG=TomohiroConfig BINARY=~/Documents/RISCVConsoleCode/build/out.elf LOADMEM=1 TIMEOUT_CYCLES=0 EXTRA_SIM_PREPROC_DEFINES="+define+UNIT_DELAY=1" USE_FST=1
// make -C sims/vcs run-binary-debug CONFIG=TomohiroPreChipConfig BINARY=~/Documents/RISCVConsoleCode/build/out.elf LOADMEM=1 TIMEOUT_CYCLES=0 EXTRA_SIM_PREPROC_DEFINES="+define+UNIT_DELAY=1"


// Simulate the MCU all-integrated
// make -C sims/verilator run-binary-debug CONFIG=CryptoMCUConfig BINARY=~/Documents/IKEDA_LAB/RISCVConsoleCode/build/out.elf LOADMEM=1 TIMEOUT_CYCLES=0 EXTRA_SIM_PREPROC_DEFINES="+define+UNIT_DELAY=1" USE_FST=1
// make -C sims/verilator run-binary-debug CONFIG=CryptoMCUConfig BINARY=~/Documents/IKEDA_LAB/RISCVConsoleCode/build/out.elf LOADMEM=1 TIMEOUT_CYCLES=0 EXTRA_SIM_PREPROC_DEFINES="+define+UNIT_DELAY=1 -I$(pwd)/../tsmc7dig/projects/yuto_dilithium_202604/src" USE_FST=1

// Simulate the TLSerial with FPGA-CHIP comms
// make -C sims/verilator run-binary-debug CONFIG=TetheredChipCryptoConfig BINARY=~/Documents/IKEDA_LAB/RISCVConsoleCode/build/out.elf LOADMEM=1 TIMEOUT_CYCLES=0 EXTRA_SIM_PREPROC_DEFINES="+define+UNIT_DELAY=1 -I$(pwd)/../tsmc7dig/projects/yuto_dilithium_202604/src" USE_FST=1

// To implement in FPGA for measurements
// make -C fpga SUB_PROJECT=vcu108 CONFIG=RocketDuranVCU108Config bitstream
// make -C fpga SUB_PROJECT=vcu108 CONFIG=RocketDuran40MHzVCU108Config bitstream
// make -C fpga SUB_PROJECT=vcu108 CONFIG=RocketDuran50MHzVCU108Config bitstream
// make -C fpga SUB_PROJECT=vcu108 CONFIG=RocketDuran100MHzVCU108Config bitstream


uint8_t spi_config[] = {
    // Check signature
    0xBA,
    // SET_DIV
    0x4,
    // FCW
    0x25,
    // KI
    0x0, 0x0,
    // KP
    0x0, 0x0
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
        gpio[(GPIO_INPUT_EN >> 2)] = (1 << 1) | (1 << 3) | (1 << 4) | (1 << 6);
        gpio[(GPIO_OUTPUT_EN >> 2)] = (1 << 0) | (1 << 2) | (1 << 5);
        gpio[(GPIO_PULLUP_EN >> 2)] = 0;

        // Mux into the external clock
        put_gpio(2, 0);

        // Trigger all the resets
        put_gpio(0, 1);
        put_gpio(5, 0);

        // Release the shift register only
        //put_gpio(0, 1);

        // Push something
        spi_pll[(SPI_REG_CSMODE >> 2)] = SPI_CSMODE_HOLD;
        uint8_t back;
        for(int i = 0; i < spi_config_siz; i++) 
            back = spi_xfer(spi_pll, 0xff);
        spi_pll[(SPI_REG_CSMODE >> 2)] = SPI_CSMODE_AUTO;
        
        if(back != spi_config[0]) {
            // Failed signature
            kprintf("Failed signature in the SPI config (%x)\r\n", (unsigned int) back);
            continue;
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
        }
        break;
    }
    dilithium_test();
    bls12_381_test();
}
