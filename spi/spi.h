#ifndef SPI_H
#define SPI_H

#include <inttypes.h>

void spi_init(uint32_t volatile* spi);
uint8_t spi_xfer(uint32_t volatile* spi, uint8_t d);

#endif
