#include "spi_interface.h"

static int spiFd;

void spi_setup()
{
  spiFd = wiringPiSPISetup (SPI_CHAN, SPI_SPEED);
  if (spiFd < 0)
  {
    fprintf (stderr, "Falha ao alocar barramento SPI: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void spi_read(char *buffer, size_t size) {
  if (size > MAX_SIZE) size = MAX_SIZE;
  if (wiringPiSPIDataRW(SPI_CHAN, buffer, size) == -1)
	{
	  printf ("Falha na comunicação SPI, função de leitura: %s\n", strerror (errno)) ;
	  exit(EXIT_FAILURE);
	}
}

void spi_write(char *data, size_t size) {
  if (size > MAX_SIZE) size = MAX_SIZE;
  if (wiringPiSPIDataRW(SPI_CHAN, data, size) == -1)
	{
	  printf ("Falha na comunicação SPI, função de escrita: %s\n", strerror (errno)) ;
	  exit(EXIT_FAILURE);
	}
}