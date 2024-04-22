#include <linux/init.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include "rfid_rc522_device_driver.h"

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Júlio César Schneider Martins <jschneiderm98@gmail.com>");
MODULE_DESCRIPTION("Device driver to interface with RFID RC522");


struct spi_board_info rc222_spi_master_info = 
{
  .modalias     = "rfid_rc522_spi_driver",
  .max_speed_hz = 1000000,
  .bus_num      = SPI_BUS_NUM,
  .chip_select  = 0,
  .mode         = SPI_MODE_0
};

static struct spi_device *rc222_spi_slave_device;

int rc522_write( uint8_t data )
{
  uint8_t rx  = 0x00;
  if(!rc222_spi_slave_device) {
    MSG_BAD("Slave device not avaible to write\n", 0L);
    return -1
  }
  
  struct spi_transfer	transfer_data = 
  {
    .tx_buf	= &data,
    .rx_buf = &rx,
    .len		= 1,
  };

  spi_sync_transfer( rc222_spi_slave_device, &tr, 1 );
  
  return 0;
}

int __init rc522_init_driver(void)
{
  int err;
  struct spi_master *master_device;
  
  master_device = spi_busnum_to_master( rc222_spi_master_info.bus_num );
  if( master_device == NULL )
  {
    MSG_BAD("No master in bus_num\n", 0L);
    return -ENODEV;
  }
   
  // create a new slave device, given the master and device info
  rc222_spi_slave_device = spi_new_device( master_device, &rc222_spi_slave_device_info );
  if( rc222_spi_slave_device == NULL ) 
  {
    MSG_BAD("Error while creating slave device\n", 0L);
    return -ENODEV;
  }
  
  // 8-bits in a word
  rc222_spi_slave_device->bits_per_word = 8;

  // setup the SPI slave device
  err = spi_setup( rc222_spi_slave_device );
  if( err )
  {
    MSG_BAD("Error in setup of slave device\n", (long int) err);
    spi_unregister_device( rc222_spi_slave_device );
    return -ENODEV;
  }
  
  MSG_OK("SPI driver Registered\n");
  return 0;
}

int __exit rc522_exit_driver(void)
{ 
  if( rc222_spi_slave_device )
  {
    spi_unregister_device( rc222_spi_slave_device );    // Unregister the SPI slave
    MSG_OK("SPI driver Unregistered\n");
    return 0;
  }
}
 
module_init(rc522_init_driver);
module_exit(rc522_exit_driver);