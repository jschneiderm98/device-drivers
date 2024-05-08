#include "spi_interface.h"

static int spiFd;

void spi_setup()
{
  setup_reset_pin();
  spiFd = wiringPiSPISetup (SPI_CHAN, SPI_SPEED);
  if (spiFd < 0)
  {
    fprintf (stderr, "Falha ao alocar barramento SPI: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void setup_reset_pin() {
  wiringPiSetup();
  pinMode(RC522_RST_PIN, OUTPUT);
  digitalWrite(RC522_RST_PIN, HIGH);
}

void cleanup_spi() {
  pinMode(RC522_RST_PIN, INPUT);
}

void spi_read(uint8_t *data, size_t size) {
  if (size > MAX_SIZE) size = MAX_SIZE;

  if (wiringPiSPIDataRW(SPI_CHAN, data, size) == -1)
	{
	  printf ("Falha na comunicação SPI, função de leitura: %s\n", strerror (errno)) ;
	  exit(EXIT_FAILURE);
	}
}

void spi_write(uint8_t *data, size_t size) {
  if (size > MAX_SIZE) size = MAX_SIZE;

  if (wiringPiSPIDataRW(SPI_CHAN, data, size) == -1)
	{
	  printf ("Falha na comunicação SPI, função de escrita: %s\n", strerror (errno)) ;
	  exit(EXIT_FAILURE);
	}
}