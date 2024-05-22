#include <linux/init.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include "rfid_rc522_device_driver.h"

static struct spi_device *rc522_spi_dev;

static struct gpio rc522_rst_pin = {  RC522_RST_PIN, GPIOF_OUT_INIT_HIGH, "EN" };

uint8_t format_address_to_byte(mfrc522_registers reg, adress_bit_mode mode) {
    if(mode == address_write_mode)
        return (uint8_t) (reg << 1) & 0x7E;
    return (uint8_t) ((reg << 1) & 0x7E) | 0x80;
}

void write_to_register(mfrc522_registers reg, uint8_t data) {
    uint8_t *message = kmalloc(2, GFP_KERNEL);
    message[0] = format_address_to_byte(reg, address_write_mode);
    message[1] = data;

    struct spi_transfer t = {
        .tx_buf = message,
        .len = 2,
    };

    struct spi_message spi_m;
    spi_message_init(&spi_m);
    spi_message_add_tail(&t, &spi_m);

    int ret = spi_sync(rc522_spi_dev, &spi_m);
    
    if (ret < 0) {
        printk(KERN_ERR "Erro ao enviar dados via SPI\n");
    }
    kfree(message);
}

void write_to_register_multiple(mfrc522_registers reg, uint8_t *data, size_t data_size) {
    uint8_t *buffer = kmalloc(data_size+1, GFP_KERNEL);// byte0 - address, bytes 1-n - data
    memcpy(buffer+1, data, data_size);
    buffer[0] = format_address_to_byte(reg, address_write_mode);

    struct spi_transfer t = {
        .tx_buf = buffer,
        .len = data_size + 1,
    };

    struct spi_message spi_m;
    spi_message_init(&spi_m);
    spi_message_add_tail(&t, &spi_m);

    int ret = spi_sync(rc522_spi_dev, &spi_m);
    
    if (ret < 0) {
        printk(KERN_ERR "Erro ao enviar dados via SPI\n");
    }

    kfree(buffer);
}

uint8_t read_from_register(mfrc522_registers reg) {
    uint8_t *message_res = kmalloc(2, GFP_KERNEL);
    uint8_t *message = kmalloc(2, GFP_KERNEL);
    message[0] = format_address_to_byte(reg, address_read_mode);
    message[1] = 0x00;

    struct spi_transfer t = {
        .tx_buf = message,
        .rx_buf = message_res,
        .len = 2,
    };

    struct spi_message spi_m;
    spi_message_init(&spi_m);
    spi_message_add_tail(&t, &spi_m);

    int ret = spi_sync(rc522_spi_dev, &spi_m);
    
    if (ret < 0) {
        printk(KERN_ERR "Erro ao ler dados via SPI\n");
        return ret;
    }
    uint8_t res = message_res[1];
    kfree(message_res);
    kfree(message);
    return res;
}

void read_from_register_multiple(mfrc522_registers reg, uint8_t **res, size_t num_reads) {
    uint8_t *buffer = kmalloc(num_reads+1, GFP_KERNEL);
    (*res) = kmalloc(num_reads+1, GFP_KERNEL);
    uint8_t reg_formatted = format_address_to_byte(reg, address_read_mode);

    for (size_t i = 0; i < num_reads; i++)
    {
        buffer[i] = reg_formatted;
    }
    buffer[num_reads] = 0x00;

    struct spi_transfer t = {
        .tx_buf = buffer,
        .rx_buf = (*res),
        .len = num_reads+1,
    };

    struct spi_message spi_m;
    spi_message_init(&spi_m);
    spi_message_add_tail(&t, &spi_m);

    int ret = spi_sync(rc522_spi_dev, &spi_m);
    
    if (ret < 0) {
        printk(KERN_ERR "Erro ao ler dados via SPI\n");
    }
    (*res) = (*res) + 1;
    kfree(buffer);
}

void set_bits_in_reg(mfrc522_registers reg, uint8_t bits_to_set) {
    uint8_t reg_current_byte = read_from_register(reg);
    write_to_register(reg, reg_current_byte | bits_to_set);
}

void clear_bits_in_reg(mfrc522_registers reg, uint8_t bits_to_set) {
    uint8_t reg_current_byte = read_from_register(reg);
    write_to_register(reg, reg_current_byte & (~bits_to_set));
}

void rc522_reset(void) {
    write_to_register(CommandReg, SoftReset);
}

void rc522_antenna_on(void) 
{
  uint8_t txControlRegByte = read_from_register(TxControlReg);
  if ((txControlRegByte & 0b00000011) != 0b00000011) {
    set_bits_in_reg(TxControlReg, 0b00000011);
  }
}

uint8_t rc522_pcd_setup(void) {
    rc522_reset();

    //configure timeout for communication with PICC
    write_to_register(TModeReg, 0x8D);
    write_to_register(TPrescalerReg, 0x3E);
    write_to_register(TReloadRegLSB, 0x1E);
    write_to_register(TReloadRegMSB, 0x0);

    write_to_register(TxASKReg, 0x40); // 100% ASK modulation
    write_to_register(ModeReg, 0x3D);
    rc522_antenna_on();

    return read_from_register(VersionReg);
}

void rc522_self_test(void) {
    rc522_reset();

    uint8_t *buffer = kmalloc(25, GFP_KERNEL);
    for (uint8_t i = 0; i < 25; i++)
    {   
        buffer[i] = 0x00;
    }
    
    //write the data to FIFO Buffer
    write_to_register_multiple(FIFODataReg, buffer, 25);
    
    set_bits_in_reg(FIFOLevelReg, 0x80);

    write_to_register(AutoTestReg, 0x09);
    write_to_register(FIFODataReg, 0x00);
    write_to_register(CommandReg, CalcCRC);
    usleep_range(15000, 25000);
    kfree(buffer);
    read_from_register_multiple(FIFODataReg, &buffer, 64);
    for (size_t i = 0; i < 8; i++)
    {   
        printk(
            KERN_INFO "0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x", 
            buffer[i*8], 
            buffer[(i*8)+1], 
            buffer[(i*8)+2], 
            buffer[(i*8)+3], 
            buffer[(i*8)+4], 
            buffer[(i*8)+5], 
            buffer[(i*8)+6], 
            buffer[(i*8)+7]
        );
    }
    rc522_reset();
}

static int rc522_spi_probe(struct spi_device *spi)
{
    printk(KERN_INFO "Probe Chamado\n");
    rc522_spi_dev = spi;

    //uint8_t version = rc522_pcd_setup();
    //printk(KERN_INFO "Version byte 0x%x\n", version);
    rc522_self_test();

    return 0;
}

static void rc522_spi_remove(struct spi_device *spi)
{
    // Limpar qualquer recurso alocado
    rc522_spi_dev = NULL;
}

static const struct spi_device_id rc522_spi_idtable[] = {
    {"rc522", 2},
    {},
};

MODULE_DEVICE_TABLE(spi, rc522_spi_idtable);

static const struct of_device_id rc522_spi_of_match[] = {
    {
        .compatible = "mytest,rc522",
    },
    {},
};

MODULE_DEVICE_TABLE(of, rc522_spi_of_match);

static struct spi_driver rc522_spi_driver = {
    .driver = {
        .name = "rc522_spi_device",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(rc522_spi_of_match),
    },
    .probe = rc522_spi_probe,
    .remove = rc522_spi_remove,
    .id_table = rc522_spi_idtable,
};

static int __init rc522_spi_init(void)
{
    int ret;
    printk(KERN_INFO "Iniciando GPIO Reset\n");
    ret = gpio_request(rc522_rst_pin.gpio, rc522_rst_pin.label);
    if(ret) {
		MSG_BAD("Não foi possível obter acesso ao GPIO.", (long int)ret);
        return ret;
    }
    printk(KERN_INFO "GPIO Reset iniciado, setando como output com valor 1\n");
    gpio_direction_output(rc522_rst_pin.gpio, 1);
    printk(KERN_INFO "GPIO Reset configurado com sucesso\n");

    printk(KERN_INFO "Iniciando driver SPI\n");
    // Registrar a SPI device
    ret = spi_register_driver(&rc522_spi_driver);
    if (ret < 0) {
        printk(KERN_ERR "Erro ao registrar o driver SPI\n");
        gpio_set_value(rc522_rst_pin.gpio, 0);
        gpio_free(rc522_rst_pin.gpio);
        return ret;
    }
    printk(KERN_INFO "Driver SPI iniciado\n");

    
    return 0;
}

static void __exit rc522_spi_exit(void)
{
    spi_unregister_driver(&rc522_spi_driver);
    gpio_set_value(rc522_rst_pin.gpio, 0);
    gpio_free(rc522_rst_pin.gpio);
}

module_init(rc522_spi_init);
module_exit(rc522_spi_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Julio Cesar Schneider Martins <jschneiderm98@gmail.com>");
MODULE_DESCRIPTION("Device driver to interface with RFID RC522");