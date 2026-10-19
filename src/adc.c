#include <stdint.h>
#include <platform.h>
#include <kprintf/kprintf.h>

#include "main.h"
#include "adc.h"

uint8_t adc_buf[ADC_BUF_SIZE];
volatile uint32_t adc_count = 0;

static inline uint64_t mtime_now(void) {
    return *(volatile uint64_t*)(CLINT_CTRL_ADDR + CLINT_MTIME);
}

void adc_init(void) {
    adc[ADC_REG_CTRL >> 2] = ADC_CTRL_CPOL;                                     // Clock polarity inverted
    adc[ADC_REG_CTRL >> 2] = ADC_CTRL_CPOL | ADC_CTRL_VALID;                    // Enable the ADC
    adc[ADC_REG_CTRL >> 2] = ADC_CTRL_CPOL | ADC_CTRL_VALID | ADC_CTRL_RESET;   // Assert the reset while enabling the ADC
    adc[ADC_REG_CTRL >> 2] = ADC_CTRL_CPOL | ADC_CTRL_VALID;                    // Deassert the reset while enabling the ADC
}

// Discard whatever accumulated in the FIFO, so the capture starts with fresh samples
void adc_flush(void) {
    while(!(adc[ADC_REG_DATA >> 2] & ADC_FIFO_EMPTY));
}

// Fill buf with n samples as fast as possible. Nothing is printed in the loop,
// so the only limit is the MMIO read latency.
// NOTE: The FIFO is small and drops samples when full. If empty_polls is ~0,
// the CPU did not keep up with the ADC and samples were likely lost.
uint32_t adc_capture(uint8_t* buf, uint32_t n, adc_stats_t* stats) {
    volatile uint32_t* data = &adc[ADC_REG_DATA >> 2];
    uint32_t i = 0;
    uint32_t empty = 0;

    adc_flush();
    uint64_t t0 = mtime_now();
    while(i < n) {
        uint32_t v = *data;
        if(v & ADC_FIFO_EMPTY) {
            empty++;
            continue;
        }
        buf[i++] = v & ADC_DATA_MASK;
    }
    uint64_t t1 = mtime_now();

    if(stats) {
        stats->ticks = t1 - t0;
        stats->empty_polls = empty;
        stats->min = ADC_DATA_MASK;
        stats->max = 0;
        for(uint32_t k = 0; k < n; k++) {
            if(buf[k] < stats->min) stats->min = buf[k];
            if(buf[k] > stats->max) stats->max = buf[k];
        }
    }
    return i;
}

void adc_report(const uint8_t* buf, uint32_t n, const adc_stats_t* stats) {
    uint64_t us = stats->ticks * 1000000UL / timescale_freq;
    uint64_t rate = us ? (uint64_t)n * 1000000UL / us : 0;

    kprintf("ADC capture: %d samples in %d us (%d samples/s)\r\n", n, (uint32_t)us, (uint32_t)rate);
    kprintf("Empty polls: %d\r\n", stats->empty_polls);
    kprintf("Min: %d, Max: %d\r\n", stats->min, stats->max);
    kputs("First samples:");
    for(uint32_t k = 0; k < n && k < 32; k++) {
        if(!(k % 16)) kputs("\r\n");
        kprintf("%d ", buf[k]);
    }
    kputs("\r\n");
}
