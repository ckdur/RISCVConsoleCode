#ifndef _RO_H
#define _RO_H

#include <stdint.h>

// ROCOUNTER registers (console,rocounter0)
#define RO_REG_CTRL         0x00
#define RO_REG_GATE         0x04
#define RO_REG_CMD          0x08
#define RO_REG_SEQ          0x0C
#define RO_REG_LEVEL        0x10
#define RO_REG_COUNT(i)     (0x20 + 4 * (i))

// RO_REG_CTRL fields
#define RO_CTRL_RSTB        (1 << 0)            // RO_RST_B pin (the /32 dividers reset when 0)
#define RO_CTRL_GATE(i)     (1 << (8 + (i)))    // DUT_gate pin of each RO

// RO_REG_CMD fields
#define RO_CMD_START        (1 << 0)            // W: start a window. R: busy
#define RO_CMD_DONE         (1 << 1)
#define RO_CMD_CONTINUOUS   (1 << 8)

// Counter channels
#define RO_101              0
#define RO_13               1
#define RO_N                2

// Chip SPI registers of each RO (R[7:0] is RO 101, R[15:8] is RO 13)
#define RO_SPI_REG(i)       ((i) == RO_101 ? 0 : 1)

// Bits of the chip SPI register of each RO
#define RO_CFG_N_RO_CONTROL (1 << 0)
#define RO_CFG_RO_CONTROL   (1 << 1)
#define RO_CFG_DUT_FOOTER   (1 << 2)
#define RO_CFG_DUT_HEADER   (1 << 3)            // PMOS header, active low

// Ring closed, DUT header and footer on
#define RO_CFG_OSC          (RO_CFG_RO_CONTROL | RO_CFG_DUT_FOOTER)
// Ring broken (stress / I-V mode, DUT_gate drives the DUT)
#define RO_CFG_OFF          (RO_CFG_N_RO_CONTROL)

// The RO output pad is the ring divided by 32 on chip
#define RO_DIV              32

void ro_spix_write(int addr, uint8_t val);
void ro_init(void);
void ro_config(uint8_t cfg101, uint8_t cfg13);
void ro_measure(uint32_t gate, uint32_t* counts);
void ro_test(void);
void ro_diag(void);
void ro_scan(void);

#endif // _RO_H
