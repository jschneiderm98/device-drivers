#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define	SPI_CHAN 0
#define	MAX_SIZE 2000
#define SPI_SPEED 1000000

void spi_setup();
void spi_read(char *buffer, size_t size);
void spi_write(char *data, size_t size);