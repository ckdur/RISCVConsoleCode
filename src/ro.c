#include <stdint.h>
#include <platform.h>
#include <kprintf/kprintf.h>
#include <spi.h>

#include "main.h"
#include "adc.h"
#include "ro.h"

// Write one register of the chip SPI (asicone_sg13g2/digital/rtl/spix/spi.v).
// Frame, sampled on the rising SCLK while CS is low: write, read, addr[2:0], data[7:0], all MSB
// first. The register is written on the 14th rising edge, so the frame is padded to 16 clocks.
// The chip resets its bit counter only on SCLK edges with CS high, so a dummy byte with CS
// off follows every frame.
// NOTE: The chip SPI shares its reset with the ADC (active low here, active high in the ADC):
// adc_rst must be high, or the registers are cleared.
void ro_spix_write(int addr, uint8_t val) {
    uint16_t f = (1 << 15) | (0 << 14) | ((addr & 0x7) << 11) | (val << 3);

    spi[(SPI_REG_FMT >> 2)] = SPI_FMT_LEN(8) | SPI_FMT_PROTO(SPI_PROTO_S) | SPI_FMT_ENDIAN(SPI_ENDIAN_MSB) | SPI_FMT_DIR(SPI_DIR_RX);
    spi[(SPI_REG_CSMODE >> 2)] = SPI_CSMODE_HOLD;
    spi_xfer(spi, f >> 8);
    spi_xfer(spi, f & 0xff);
    spi[(SPI_REG_CSMODE >> 2)] = SPI_CSMODE_AUTO;

    // Clocks with CS high, to reset the bit counter of the chip
    spi[(SPI_REG_CSMODE >> 2)] = SPI_CSMODE_OFF;
    spi_xfer(spi, 0x0);
    spi[(SPI_REG_CSMODE >> 2)] = SPI_CSMODE_AUTO;
}

void ro_init(void) {
    spi_init(spi);
    // Hold adc_rst high for the whole RO session: releases the reset of the chip SPI
    adc[ADC_REG_CTRL >> 2] = ADC_CTRL_RESET;
    // Dividers in reset, DUT_gate low
    rocnt[RO_REG_CTRL >> 2] = 0;
    rocnt[RO_REG_CMD >> 2] = 0;
}

// Configure both ROs through SPI and restart the dividers.
// NOTE: MISO does not work on the chip, the writes cannot be read back
void ro_config(uint8_t cfg101, uint8_t cfg13) {
    uint32_t ctrl = rocnt[RO_REG_CTRL >> 2];
    rocnt[RO_REG_CTRL >> 2] = ctrl & ~RO_CTRL_RSTB;
    ro_spix_write(RO_SPI_REG(RO_101), cfg101);
    ro_spix_write(RO_SPI_REG(RO_13), cfg13);
    rocnt[RO_REG_CTRL >> 2] = ctrl | RO_CTRL_RSTB;
}

// Count the rising edges of the RO outputs during gate bus cycles
void ro_measure(uint32_t gate, uint32_t* counts) {
    rocnt[RO_REG_GATE >> 2] = gate;
    rocnt[RO_REG_CMD >> 2] = RO_CMD_START;
    while(!(rocnt[RO_REG_CMD >> 2] & RO_CMD_DONE));
    for(int i = 0; i < RO_N; i++)
        counts[i] = rocnt[RO_REG_COUNT(i) >> 2];
}

static void ro_report(const char* name, uint32_t gate, int reps) {
    static const char* ro_name[RO_N] = {"RO101", "RO13"};
    uint32_t counts[RO_N];

    kprintf("%s (gate %d cycles at %d Hz):\r\n", name, gate, tlclk_freq);
    for(int r = 0; r < reps; r++) {
        ro_measure(gate, counts);
        for(int i = 0; i < RO_N; i++) {
            // Pad frequency, and ring frequency (the pad is the ring / 32)
            uint64_t fpad = (uint64_t)counts[i] * (uint64_t)tlclk_freq / gate;
            kprintf("  %s: %d edges, pad %ld Hz, ring %ld Hz\r\n", ro_name[i], counts[i],
                (long)fpad, (long)(fpad * RO_DIV));
        }
    }
}

void ro_test(void) {
    uint32_t gate = tlclk_freq; // 1 s

    kputs("RO test\r\n");
    ro_init();

    // Both rings broken: both counts must be 0
    ro_config(RO_CFG_OFF, RO_CFG_OFF);
    ro_report("Both off", gate, 1);

    // One at a time, then both. The count switching on and off is the only proof that the SPI
    // writes reach the chip
    ro_config(RO_CFG_OSC, RO_CFG_OFF);
    ro_report("RO101 on", gate, 1);
    ro_config(RO_CFG_OFF, RO_CFG_OSC);
    ro_report("RO13 on", gate, 1);
    ro_config(RO_CFG_OSC, RO_CFG_OSC);
    ro_report("Both on", gate, 5);

    ro_config(RO_CFG_OFF, RO_CFG_OFF);
    kputs("RO test done\r\n");
}

// Fraction (per mille) of reads of the synchronized RO outputs that are high
static void ro_duty(int n, uint32_t* hi) {
    for(int i = 0; i < RO_N; i++) hi[i] = 0;
    for(int k = 0; k < n; k++) {
        uint32_t l = rocnt[RO_REG_LEVEL >> 2];
        for(int i = 0; i < RO_N; i++) hi[i] += (l >> i) & 1;
    }
    for(int i = 0; i < RO_N; i++) hi[i] = (uint32_t)((uint64_t)hi[i] * 1000 / n);
}

static void ro_diag_line(const char* name, uint32_t gate) {
    uint32_t c[RO_N], d[RO_N];
    ro_measure(gate, c);
    ro_duty(20000, d);
    kprintf("  %s: RO101 %d, RO13 %d edges/%d cycles; duty %d, %d per mille\r\n",
        name, c[RO_101], c[RO_13], gate, d[RO_101], d[RO_13]);
}

// Checks that the counts come from the rings through the dividers, and looks for harmonic modes
// (several edges running around the ring) by restarting the rings from different states
void ro_diag(void) {
    uint32_t gate = tlclk_freq / 10; // 100 ms
    uint32_t c[RO_N];

    kputs("RO diagnostics\r\n");
    ro_init();

    ro_config(RO_CFG_OSC, RO_CFG_OSC);
    ro_diag_line("both on        ", gate);
    rocnt[RO_REG_CTRL >> 2] = 0;                                    // dividers in reset
    ro_diag_line("RO_RST_B low   ", gate);
    rocnt[RO_REG_CTRL >> 2] = RO_CTRL_RSTB;
    ro_diag_line("RO_RST_B high  ", gate);

    ro_config(RO_CFG_OSC | RO_CFG_DUT_HEADER, RO_CFG_OSC | RO_CFG_DUT_HEADER);
    ro_diag_line("DUT header off ", gate);
    ro_config(RO_CFG_RO_CONTROL, RO_CFG_RO_CONTROL);
    ro_diag_line("DUT footer off ", gate);
    ro_config(RO_CFG_OSC, RO_CFG_OSC);
    ro_diag_line("both on again  ", gate);

    // Restarts: open the ring with DUT_gate at 0 or 1, or go through the reset value 0x00
    for(int mode = 0; mode < 3; mode++) {
        static const char* mname[3] = {"open, DUT_gate=0", "open, DUT_gate=1", "via 0x00 (rovcel3 pull-up)"};
        kprintf("Restarts %s:\r\n ", mname[mode]);
        for(int r = 0; r < 12; r++) {
            rocnt[RO_REG_CTRL >> 2] = RO_CTRL_RSTB | (mode == 1 ? (RO_CTRL_GATE(0) | RO_CTRL_GATE(1)) : 0);
            if(mode == 2) {
                ro_spix_write(RO_SPI_REG(RO_101), 0x00);
                ro_spix_write(RO_SPI_REG(RO_13), 0x00);
            } else {
                ro_spix_write(RO_SPI_REG(RO_101), RO_CFG_OFF);
                ro_spix_write(RO_SPI_REG(RO_13), RO_CFG_OFF);
            }
            ro_measure(gate / 10, c);                               // 10 ms settle
            ro_spix_write(RO_SPI_REG(RO_101), RO_CFG_OSC);
            ro_spix_write(RO_SPI_REG(RO_13), RO_CFG_OSC);
            ro_measure(gate, c);
            kprintf(" %d/%d", c[RO_101], c[RO_13]);
        }
        kputs("\r\n");
    }
    // Restarts from states where the ring nodes do not alternate: the footer off (DUT output stuck
    // high), the DUT header off, a ring held open for a variable time, or the RO supply rails of the
    // enable stage released at different instants. A harmonic mode shows up as counts in ratios k/k'
    kputs("Restarts from footer off (ms held):\r\n ");
    rocnt[RO_REG_CTRL >> 2] = RO_CTRL_RSTB;
    for(int r = 0; r < 12; r++) {
        ro_spix_write(RO_SPI_REG(RO_101), RO_CFG_RO_CONTROL);
        ro_spix_write(RO_SPI_REG(RO_13), RO_CFG_RO_CONTROL);
        ro_measure(gate / 100 * (r + 1), c);
        ro_spix_write(RO_SPI_REG(RO_101), RO_CFG_OSC);
        ro_spix_write(RO_SPI_REG(RO_13), RO_CFG_OSC);
        ro_measure(gate, c);
        kprintf(" %d/%d", c[RO_101], c[RO_13]);
    }
    kputs("\r\nRestarts, open for a few us only:\r\n ");
    for(int r = 0; r < 12; r++) {
        ro_spix_write(RO_SPI_REG(RO_101), RO_CFG_OFF);
        for(volatile int k = 0; k < r * 7; k++);
        ro_spix_write(RO_SPI_REG(RO_101), RO_CFG_OSC);
        ro_measure(gate, c);
        kprintf(" %d", c[RO_101]);
    }
    kputs("\r\nRestarts through the header-off state:\r\n ");
    for(int r = 0; r < 12; r++) {
        ro_spix_write(RO_SPI_REG(RO_101), (r & 1) ? 0x00 : RO_CFG_N_RO_CONTROL | RO_CFG_DUT_HEADER);
        for(volatile int k = 0; k < r * 50; k++);
        ro_spix_write(RO_SPI_REG(RO_101), RO_CFG_OSC | RO_CFG_DUT_HEADER);
        ro_measure(gate / 10, c);
        ro_spix_write(RO_SPI_REG(RO_101), RO_CFG_OSC);
        ro_measure(gate, c);
        kprintf(" %d", c[RO_101]);
    }
    kputs("\r\n");
    rocnt[RO_REG_CTRL >> 2] = RO_CTRL_RSTB;
    ro_config(RO_CFG_OFF, RO_CFG_OFF);
    kputs("RO diagnostics done\r\n");
}

// Counts of both rings for every value of the 4 configuration bits, with DUT_gate at 0 and 1
void ro_scan(void) {
    uint32_t gate = tlclk_freq / 10;
    uint32_t c[RO_N];
    kputs("RO config scan (cfg: RO101/RO13 edges in 100 ms, DUT_gate=0 | DUT_gate=1)\r\n");
    ro_init();
    for(int cfg = 0; cfg < 16; cfg++) {
        kprintf("  0x%hx:", cfg);
        for(int g = 0; g < 2; g++) {
            rocnt[RO_REG_CTRL >> 2] = RO_CTRL_RSTB | (g ? (RO_CTRL_GATE(0) | RO_CTRL_GATE(1)) : 0);
            ro_spix_write(RO_SPI_REG(RO_101), RO_CFG_OFF);
            ro_spix_write(RO_SPI_REG(RO_13), RO_CFG_OFF);
            ro_measure(gate / 10, c);
            ro_spix_write(RO_SPI_REG(RO_101), cfg);
            ro_spix_write(RO_SPI_REG(RO_13), cfg);
            ro_measure(gate / 10, c);
            ro_measure(gate, c);
            kprintf(" %d/%d%s", c[RO_101], c[RO_13], g ? "" : " |");
        }
        kputs("\r\n");
    }
    rocnt[RO_REG_CTRL >> 2] = RO_CTRL_RSTB;
    ro_config(RO_CFG_OFF, RO_CFG_OFF);
    kputs("RO config scan done\r\n");
}
