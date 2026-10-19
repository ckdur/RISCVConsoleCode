#ifndef _ADC_H
#define _ADC_H

#include <stdint.h>

// ADCFIFO registers (console,adcfifo0)
#define ADC_REG_DATA        0x00
#define ADC_REG_CTRL        0x04

// ADC_REG_DATA fields
#define ADC_BITS            5
#define ADC_DATA_MASK       ((1 << ADC_BITS) - 1)
#define ADC_FIFO_EMPTY      0x80000000

// ADC_REG_CTRL fields
#define ADC_CTRL_VALID      (1 << 0)
#define ADC_CTRL_RESET      (1 << 8)
#define ADC_CTRL_CPOL       (1 << 16)

// Capture buffer. Stays in RAM after the capture, to be sent through UART later
#define ADC_BUF_SIZE        65536

extern uint8_t adc_buf[ADC_BUF_SIZE];
extern volatile uint32_t adc_count;      // Valid samples in adc_buf

typedef struct {
    uint64_t ticks;         // mtime ticks spent in the capture
    uint32_t empty_polls;   // Reads that found the FIFO empty
    uint8_t min;
    uint8_t max;
} adc_stats_t;

void adc_init(void);
void adc_flush(void);
uint32_t adc_capture(uint8_t* buf, uint32_t n, adc_stats_t* stats);
void adc_report(const uint8_t* buf, uint32_t n, const adc_stats_t* stats);

#endif // _ADC_H
