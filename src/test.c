#include <main.h>
#include <kprintf/kprintf.h>
#include <clkutils/clkutils.h>
#include <spi.h>
#include <stdint.h>
#include <string.h>

#include "main.h"
#include "adc.h"
#include "ro.h"

// The ADC measurement is finished. It cannot run together with the ROs: the ADC reset is the
// (inverted) reset of the chip SPI that holds the RO configuration
#ifndef TEST_ADC
#define TEST_ADC 0
#endif

volatile uint32_t* spi = (uint32_t*)0;
volatile uint32_t* gpio = (uint32_t*)0;
volatile uint32_t* adc = (uint32_t*)0;
volatile uint32_t* rocnt = (uint32_t*)0;

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
    if(!rocnt) {
        kputs("No RO counter detected!\r\n");
    }

    if(TEST_ADC && adc) {
        kputs("ADC detected!\r\n");
        adc_init();

        adc_stats_t stats;
        adc_count = adc_capture(adc_buf, ADC_BUF_SIZE, &stats);
        adc_report(adc_buf, adc_count, &stats);
        // TODO: Send adc_buf[0..adc_count) through UART
    }

    if(spi && adc && rocnt) {
        ro_test();
        ro_diag();
        ro_scan();
    }
    kputs("Finished!\r\n");
}
