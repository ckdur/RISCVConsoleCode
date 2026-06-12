#include "platform.h"
#include "encoding.h"
#include <inttypes.h>
#include "spi.h"

void spi_init(uint32_t volatile* spi) {
    spi[(SPI_REG_SCKDIV >> 2)] = 5;
    spi[(SPI_REG_SCKMODE >> 2)] = SPI_SCK_POL;
    spi[(SPI_REG_CSMODE >> 2)] = 0; // 0 Auto, 2 Hold, 3 Off
}

uint8_t spi_xfer(uint32_t volatile* spi, uint8_t d)
{
	int32_t r;
    spi[(SPI_REG_TXFIFO >> 2)] = d;
	do {
		r = spi[(SPI_REG_RXFIFO >> 2)];
	} while (r < 0);
	return r;
}
