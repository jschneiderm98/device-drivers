#include <linux/kernel.h>
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

int isCrcOperationDone(void) {
    uint8_t divIrqRegByte = read_from_register(DivIrqReg);
    return divIrqRegByte & 0x04;
}

rc522_status calculate_crc(uint8_t *data, size_t data_size, uint8_t *result) {
    uint16_t i;
    write_to_register(CommandReg, Idle);
    clear_bits_in_reg(DivIrqReg, 0x04);
    set_bits_in_reg(FIFOLevelReg, 0x80);

    //write the data to FIFO Buffer
    write_to_register_multiple(FIFODataReg, data, data_size);

    write_to_register(CommandReg, CalcCRC);

    i = 5000;
    while (i>0) {
        if(isCrcOperationDone()) {
            write_to_register(CommandReg, Idle);
            result[0] = read_from_register(CRCResultRegLSB);
            result[1] = read_from_register(CRCResultRegMSB);
            return RC522_OK;
        }
        i--;
    }
    write_to_register(CommandReg, Idle);
    return RC522_TIMEOUT;
}

uint8_t is_crc_from_picc_valid(uint8_t *data, uint8_t data_size) {
    uint8_t result[2];
    uint8_t data_size_without_crc = data_size - 2;
    uint8_t crc_position = data_size - 2;

    calculate_crc(data, data_size_without_crc, result);
    
    set_bits_in_reg(FIFOLevelReg, 0x80);
    if(data[crc_position] != result[0] || data[crc_position+1] != result[1]) return 0;
    return 1;
}

uint8_t is_uid_valid_picc(uint8_t *uid) {
    uint8_t check = 0;
    for (size_t i = 0; i < 4; i++)
    {
        check = check ^ uid[i];
    }
    return check == uid[4];
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

rc522_status send_command(uint8_t command, uint8_t *data, size_t data_size, uint8_t **response, uint8_t *response_size, uint8_t *response_size_bits, uint8_t should_check_crc) {
    uint8_t irqEn = 0x00;
    uint8_t waitIrq = 0x00;

    //uint8_t tx_last_bits = valid_bits ? valid_bits : 0;
    //uint8_t bit_framing = tx_last_bits;

    if(command == MFAuthent) {
        irqEn = 0x12;
        waitIrq = 0x10;
    }

    if(command == Transceive) {
        irqEn = 0x77;
        waitIrq = 0x30;
    }
    write_to_register(CommandReg, Idle);
    write_to_register(ComlEnReg, irqEn | 0x80);
    clear_bits_in_reg(ComIrqReg, 0x80);
    set_bits_in_reg(FIFOLevelReg, 0x80);

    write_to_register_multiple(FIFODataReg, data, data_size);

    write_to_register(CommandReg, command);

    if(command == Transceive) {
        set_bits_in_reg(BitFramingReg, 0b10000000);
    }

    int i = 2000;

    while (1) {
        uint8_t comIrqRegByte = read_from_register(ComIrqReg);
        if (i-- < 1 || comIrqRegByte & waitIrq) break;
    }

    clear_bits_in_reg(BitFramingReg, 0b10000000);

    if(i < 1) return RC522_TIMEOUT;

    uint8_t errorRegValue = read_from_register(ErrorReg); // ErrorReg[7..0] bits are: WrErr TempErr reserved BufferOvfl CollErr CRCErr ParityErr ProtocolErr
    if (errorRegValue & 0x13) 
    {  // BufferOvfl ParityErr ProtocolEr
        printk(KERN_ERR"Error detected, errorRegValue: %x\n", errorRegValue);
        return RC522_ERR;
    }
    if(read_from_register(ComIrqReg) & irqEn & 0x01) return RC522_NOTAGERR;

    *response_size = read_from_register(FIFOLevelReg);
    uint8_t last_bits = read_from_register(ControlReg) & 0b00000111;
    if(*response_size > 0) {
        read_from_register_multiple(FIFODataReg, response, *response_size);
        *response_size_bits = last_bits ? ((*response_size) - 1) * 8 + last_bits : (*response_size)*8;
    }

    if(!should_check_crc) return RC522_OK;

    if (*response_size < 3 || !is_crc_from_picc_valid(*response, *response_size)) return RC522_FAILED_CRC_CHECK;
     
    return RC522_OK;
}

rc522_status req_a_picc(uint8_t **res, uint8_t *res_size, uint8_t *res_size_bits) {
    uint8_t req_mode = PICC_REQIDL;
    write_to_register(BitFramingReg, 0x07);

    rc522_status status = send_command(Transceive, &req_mode, 1, res, res_size, res_size_bits, 0);

    if(status != RC522_OK) return status;
    if(*res_size_bits != 0x10) return RC522_ERR;
    return RC522_OK;
}

rc522_status anticollision(uint8_t **res, uint8_t *res_size, uint8_t *res_size_bits) {
    uint8_t data[2] = { PICC_ANTICOLL, 0x20 };
    write_to_register(BitFramingReg, 0x00);
    rc522_status status = send_command(Transceive, data, 2, res, res_size, res_size_bits, 0);

    if(status != RC522_OK) return status;
    if(*res_size != 5) return RC522_ERR;
    if(!is_uid_valid_picc(*res)) return RC522_FAILED_UID_CHECK;

    return RC522_OK;
}

rc522_status select_tag(uint8_t **res, uint8_t *res_size, uint8_t *res_size_bits, uint8_t *uid) {
    uint8_t *buffer = kmalloc(9, GFP_KERNEL);
    buffer[0] = PICC_SElECTTAG;
    buffer[1] = 0x70;
    memcpy(buffer+2, uid, 5);
    
    calculate_crc(buffer, 7, buffer+7);

    rc522_status status = send_command(Transceive, buffer, 9, res, res_size, res_size_bits, 0);
    kfree(buffer);

    if(status != RC522_OK) return status;
    if(*res_size != 3) return RC522_ERR;
    return RC522_OK;
}

rc522_status authenticate(uint8_t **res, uint8_t *res_size, uint8_t *res_size_bits, picc_auth_commands auth_mode, uint8_t block_address, uint8_t *sector_key, uint8_t sector_key_size, uint8_t *uid) {
    uint8_t buffer_size = 2 + sector_key_size + 4;
    // size = 2 + sector_key_size + 4;1 for the auth_mode and 1 for address, and 4 for the first 4 bytes of uid
    uint8_t *buffer = kmalloc(buffer_size, GFP_KERNEL);
    buffer[0] = auth_mode;
    buffer[1] = block_address;
    memcpy(buffer+2, sector_key, sector_key_size);
    memcpy(buffer+2+sector_key_size, uid, 4);

    rc522_status status = send_command(MFAuthent, buffer, buffer_size, res, res_size, res_size_bits, 0);
    kfree(buffer);

    if(status != RC522_OK) return status;
    if(!(read_from_register(Status2Reg) & 0x08)) return RC522_ERR;
    return RC522_OK;
}

void stop_authentication(void) {
    clear_bits_in_reg(Status2Reg, 0x08);
}

rc522_status read_block(uint8_t **res, uint8_t *res_size, uint8_t *res_size_bits, uint8_t blockAddr) {
    uint8_t *buffer = kmalloc(4, GFP_KERNEL);
    rc522_status status;
    buffer[0] = PICC_READ;
    buffer[1] = blockAddr;
    status = calculate_crc(buffer, 2, buffer+2);
    if(status != RC522_OK) return status;

    status = send_command(Transceive, buffer, 4, res, res_size, res_size_bits, 1);
    kfree(buffer);
    if(status != RC522_OK) return status;
    *res_size = (*res_size) - 2;//ignore crc bits
    //if(*res_size != 16) return RC522_ERR;
    return RC522_OK;
}

rc522_status write_block(uint8_t *data, uint8_t data_size, uint8_t **res, uint8_t *res_size, uint8_t *res_size_bits, uint8_t blockAddr) {
    uint8_t *buffer = kmalloc(4, GFP_KERNEL);
    rc522_status status;
    buffer[0] = PICC_WRITE;
    buffer[1] = blockAddr;
    status = calculate_crc(buffer, 2, buffer+2);
    if(status != RC522_OK) return status;

    status = send_command(Transceive, buffer, 4, res, res_size, res_size_bits, 0);
    kfree(buffer);
    if(status != RC522_OK) return status;
    buffer = kmalloc(18, GFP_KERNEL); // 16 bytes of data + 2 of crc
    kfree(*res);
    *res_size = 0;
    *res_size_bits = 0;
    if(data_size < 16) {
        memcpy(buffer, data, data_size);
        for (uint8_t i = data_size; i < 16; i++)
        {
            buffer[i] = ' ';
        }
    }
    else {
        memcpy(buffer, data, 16);
    }
    status = calculate_crc(buffer, 16, buffer+16);
    if(status != RC522_OK) return status;

    status = send_command(Transceive, buffer, 18, res, res_size, res_size_bits, 1);
    //if(*res_size != 16) return RC522_ERR;
    return RC522_OK;
}

/* static int rc522_spi_probe(struct spi_device *spi)
{
    printk(KERN_INFO "Probe Chamado\n");
    rc522_spi_dev = spi;

    uint8_t version = rc522_pcd_setup();
    printk(KERN_INFO "Version byte 0x%x\n", version);

    rc522_status status = -1;
    uint8_t *req_res, *uid, *select_tag_res, *auth_res;
    uint8_t res_size = 0, res_size_bits = 0;
    uint8_t key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    
    uint16_t i = 300;
    while(i > 0) {
        status = req_a_picc(&req_res, &res_size, &res_size_bits);
        if(status != RC522_OK) {
            i--;
            continue;
        }
        for (size_t i = 0; i < res_size; i++)
        {
            printk(KERN_INFO "req_a_picc[%d] = %x", i, req_res[i]);
        }

        res_size = 0, res_size_bits = 0;
        status = anticollision(&uid, &res_size, &res_size_bits);
        if(status != RC522_OK) {
            i--;
            continue;
        }
        for (size_t i = 0; i < res_size; i++)
        {
            printk(KERN_INFO "anticollision[%d] = %u", i, uid[i]);
        }

        res_size = 0, res_size_bits = 0;
        printk(KERN_INFO "-1");
        status = select_tag(&select_tag_res, &res_size, &res_size_bits, uid);
        if(status != RC522_OK) {
            i--;
            continue;
        }
        for (size_t i = 0; i < res_size; i++)
        {
            printk(KERN_INFO "select_tag[%d] = %u", i, select_tag_res[i]);
        }

        res_size = 0, res_size_bits = 0;
        status = authenticate(&auth_res, &res_size, &res_size_bits, PICC_AUTHENT1A, 11, key, 6, uid);
        if(status != RC522_OK) {
            i--;
            continue;
        }
        for (size_t i = 0; i < res_size; i++)
        {
            printk(KERN_INFO "auth_res[%d] = %u", i, auth_res[i]);
        }

        uint8_t buffer[16] = {'N','o','v','a','E','s','c','r','i','t','a','R','C','5','2','2'};
        uint8_t buffer_size = 16;
        uint8_t max_data_access = buffer_size / 16;
        uint8_t complete_data = max_data_access > 3;
        uint8_t block_addres[3] = {8, 9, 10};
        max_data_access = complete_data ? 3 : max_data_access;
        printk(KERN_INFO "write_data");
        for (uint8_t i = 0; i < max_data_access; i++)
        {
            uint8_t *block_data;
            uint8_t block_res_size = 0, block_res_size_bits = 0;
            uint8_t *buffer_pointer = buffer + i*16;
            
            block_res_size = 0, block_res_size_bits = 0;
            if(i == max_data_access && !complete_data) {
                printk(KERN_INFO "write_block 1");
                status = write_block(buffer_pointer, buffer_size % 16, &block_data, &block_res_size, &block_res_size_bits, block_addres[i]);
            }
            else {
                printk(KERN_INFO "write_block 2");
                status = write_block(buffer_pointer, 16, &block_data, &block_res_size, &block_res_size_bits, block_addres[i]);
            }

            if(status != RC522_OK) {
                printk(KERN_INFO "status not okay");
                i--;
                continue;
            }
        }

        stop_authentication();

        break;
    }

    return 0;
} */

static int rc522_spi_probe(struct spi_device *spi)
{
    printk(KERN_INFO "Probe Chamado\n");
    rc522_spi_dev = spi;

    uint8_t version = rc522_pcd_setup();
    printk(KERN_INFO "Version byte 0x%x\n", version);

    rc522_status status = -1;
    uint8_t *req_res, *uid, *select_tag_res, *auth_res;
    uint8_t res_size = 0, res_size_bits = 0;
    uint8_t key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    
    uint16_t i = 200;
    while(i > 0) {
        status = req_a_picc(&req_res, &res_size, &res_size_bits);
        if(status != RC522_OK) {
            i--;
            continue;
        }
        for (size_t i = 0; i < res_size; i++)
        {
            printk(KERN_INFO "req_a_picc[%d] = %x", i, req_res[i]);
        }

        res_size = 0, res_size_bits = 0;
        status = anticollision(&uid, &res_size, &res_size_bits);
        if(status != RC522_OK) {
            i--;
            continue;
        }
        for (size_t i = 0; i < res_size; i++)
        {
            printk(KERN_INFO "anticollision[%d] = %u", i, uid[i]);
        }

        res_size = 0, res_size_bits = 0;
        printk(KERN_INFO "-1");
        status = select_tag(&select_tag_res, &res_size, &res_size_bits, uid);
        if(status != RC522_OK) {
            i--;
            continue;
        }
        for (size_t i = 0; i < res_size; i++)
        {
            printk(KERN_INFO "select_tag[%d] = %u", i, select_tag_res[i]);
        }

        res_size = 0, res_size_bits = 0;
        status = authenticate(&auth_res, &res_size, &res_size_bits, PICC_AUTHENT1A, 11, key, 6, uid);
        if(status != RC522_OK) {
            i--;
            continue;
        }
        for (size_t i = 0; i < res_size; i++)
        {
            printk(KERN_INFO "auth_res[%d] = %u", i, auth_res[i]);
        }

        uint8_t buffer_size = 0;
        uint8_t *buffer = kmalloc(16*3, GFP_KERNEL);
        uint8_t block_addres[3] = {8, 9, 10};
        printk(KERN_INFO "1");
        for (uint8_t i = 0; i < 3; i++)
        {
            uint8_t *block_data;
            uint8_t block_res_size = 0, block_res_size_bits = 0;
            uint8_t *buffer_pointer = buffer + (i * 16);

            block_res_size = 0, block_res_size_bits = 0;
            status = read_block(&block_data, &block_res_size, &block_res_size_bits, block_addres[i]);
            if(status != RC522_OK) {
                i--;
                continue;
            }
            printk(KERN_INFO "2");
            printk(KERN_INFO "block_data[%u]:",i);
            for (size_t i = 0; i < block_res_size; i++)
            {
                printk(KERN_CONT "%c", block_data[i]);
            }
            memcpy(buffer_pointer, block_data, block_res_size);
            printk(KERN_INFO "block_res_size: %u", block_res_size);
            buffer_size = buffer_size + block_res_size;
            printk(KERN_INFO "4");
        }
        printk(KERN_INFO "5");
        printk(KERN_INFO "Read: ");
        for (size_t i = 0; i < buffer_size; i++)
        {
            printk(KERN_CONT "%c", buffer[i]);
        }

        stop_authentication();

        break;
    }

    return 0;
}

static void rc522_spi_remove(struct spi_device *spi)
{
    // Limpar qualquer recurso alocado
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

int __init rc522_spi_init(void)
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

void __exit rc522_spi_exit(void)
{
    printk(KERN_INFO "Tirando registo do spi driver\n");
    spi_unregister_driver(&rc522_spi_driver);
    printk(KERN_INFO "Desativando pino de reset\n");
    gpio_set_value(rc522_rst_pin.gpio, 0);
    printk(KERN_INFO "Liberando pino de reset\n");
    gpio_free(rc522_rst_pin.gpio);
    printk(KERN_INFO "Modulo retirado\n");
}

module_init(rc522_spi_init);
module_exit(rc522_spi_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Julio Cesar Schneider Martins <jschneiderm98@gmail.com>");
MODULE_DESCRIPTION("Device driver to interface with RFID RC522");