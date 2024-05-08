#include "rc522_controller.h"


void write_to_register_multiple(mfrc522_registers reg, uint8_t *data, size_t data_size) {
    uint8_t *buffer = malloc(data_size+1);// byte0 - address, bytes 1-n - data
    memcpy(buffer+1, data, data_size);
    buffer[0] = format_address_to_byte(reg, address_write_mode);
    spi_write(buffer, data_size+1);
    free(buffer);
}

void write_to_register(mfrc522_registers reg, uint8_t data) {
    uint8_t *buf = malloc(2); // byte0 - address, byte 1 - data
    buf[0] = format_address_to_byte(reg, address_write_mode);
    buf[1] = data;
    spi_write(buf, 2);
    free(buf);
}

void read_from_register_multiple(mfrc522_registers reg, uint8_t **res, size_t num_reads) {
    uint8_t *buffer = malloc(num_reads+1);
    for (size_t i = 0; i < num_reads; i++)
    {
        buffer[i] = format_address_to_byte(reg, address_read_mode);
    }
    buffer[num_reads] = 0x00;

    spi_read(buffer, num_reads+1);
    *res = buffer;
}

void read_from_registers(mfrc522_registers reg, uint8_t **data, size_t data_size) {
    uint8_t *buffer = malloc(data_size+2);
    memcpy(buffer+1, *data, (data_size+1));
    buffer[0] = format_address_to_byte(reg, address_read_mode);
    buffer[data_size+1] = 0x00;
    spi_read(buffer, data_size+1);
    *data = buffer;
}

uint8_t read_from_register(mfrc522_registers reg) {
    uint8_t *buf = malloc(2), res;
    buf[0] = format_address_to_byte(reg, address_read_mode);
    buf[1] = 0x00;
    spi_read(buf, 2);
    res = buf[1];
    free(buf);
    return res;
}

void reset() {
    write_to_register(CommandReg, SoftReset);
}

void antenna_on() 
{
  uint8_t txControlRegByte = read_from_register(TxControlReg);
  if ((txControlRegByte & 0b00000011) != 0b00000011) {
    set_bits_in_reg(TxControlReg, 0b00000011);
  }
}

uint8_t init() {
    reset();

    //configure timeout for communication with PICC
    write_to_register(TModeReg, 0x8D);
    write_to_register(TPrescalerReg, 0x3E);
    write_to_register(TReloadRegLSB, 0x1E);
    write_to_register(TReloadRegMSB, 0x0);

    write_to_register(TxASKReg, 0x40); // 100% ASK modulation
    write_to_register(ModeReg, 0x3D);
    antenna_on();
    
    usleep(4000);

    return read_from_register(VersionReg);
}

void cleanup() {
    cleanup_spi();
}

uint8_t format_address_to_byte(mfrc522_registers reg, adress_bit_mode mode) {
    if(mode == address_write_mode)
        return (uint8_t) (reg << 1) & 0x7E;
    return (uint8_t) ((reg << 1) & 0x7E) | 0x80;
}

void set_bits_in_reg(mfrc522_registers reg, uint8_t bits_to_set) {
    uint8_t reg_current_byte = read_from_register(reg);
    write_to_register(reg, reg_current_byte | bits_to_set);
}

void clear_bits_in_reg(mfrc522_registers reg, uint8_t bits_to_set) {
    uint8_t reg_current_byte = read_from_register(reg);
    write_to_register(reg, reg_current_byte & (~bits_to_set));
}

int isCrcOperationDone() {
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

void self_test() {
    reset();
    usleep(10000);

    uint8_t *buffer = malloc(25);
    for (size_t i = 0; i < 25; i++)
    {   
        buffer[i] = 0x00;
    }
    
    //write the data to FIFO Buffer
    write_to_register_multiple(FIFODataReg, buffer, 25);
    set_bits_in_reg(FIFOLevelReg, 0x80);

    write_to_register(AutoTestReg, 0x09);
    write_to_register(FIFODataReg, 0x00);
    write_to_register(CommandReg, CalcCRC);

    usleep(100000);
    free(buffer);
    read_from_register_multiple(FIFODataReg, &buffer, 64);
    printf("\n");
    for (size_t i = 1, count = 1; i < 65; i++, count++)
    {   
        //printf(BYTE_TO_BINARY_PATTERN " ", BYTE_TO_BINARY(buffer[i]));
        printf("%02x ", buffer[i]);
        if(count >= 8) {
            count = 0;
            printf("\n");
        }
    }
    printf("\n");
    init();
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
        printf("Error detected, errorRegValue: %x\n", errorRegValue);
        return RC522_ERR;
    }
    if(read_from_register(ComIrqReg) & irqEn & 0x01) return RC522_NOTAGERR;

    *response_size = read_from_register(FIFOLevelReg);
    uint8_t last_bits = read_from_register(ControlReg) & 0b00000111;
    uint8_t *buffer;
    if(*response_size > 0) {
        read_from_register_multiple(FIFODataReg, &buffer, *response_size+1);
        *response = malloc(*response_size);
        *response_size_bits = last_bits ? ((*response_size) - 1) * 8 + last_bits : (*response_size)*8;
        memcpy(*response, buffer+1, *response_size);
        free(buffer);
    }

    if(!should_check_crc) return RC522_OK;

    if (*response_size < 3 || !is_crc_from_picc_valid(*response, *response_size)) return RC522_FAILED_CRC_CHECK;
     
    return RC522_OK;
}

rc522_status req_a_picc(uint8_t **res, uint8_t *res_size, uint8_t *res_size_bits) {
    uint8_t req_mode = PICC_REQIDL;
    write_to_register(BitFramingReg, 0x07);
    //write_to_register(BitFramingReg, 0x07);
    //write_to_register(TxModeReg, 0x00);
    //write_to_register(RxModeReg, 0x00);
    //write_to_register(ModWidthReg, 0x26);

    //clear_bits_in_reg(CollReg, 0x80);

    rc522_status status = send_command(Transceive, &req_mode, 1, res, res_size, res_size_bits, 0);

    if(status != RC522_OK) return status;
    if(*res_size_bits != 0x10) return RC522_ERR;
    return RC522_OK;
}

uint8_t is_uid_valid_picc(uint8_t *uid) {
    uint8_t check = 0;
    for (size_t i = 0; i < 4; i++)
    {
        check = check ^ uid[i];
    }
    return check == uid[4];
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
    uint8_t *buffer = malloc(9);
    buffer[0] = PICC_SElECTTAG;
    buffer[1] = 0x70;
    memcpy(buffer+2, uid, 5);
    
    calculate_crc(buffer, 7, buffer+7);

    rc522_status status = send_command(Transceive, buffer, 9, res, res_size, res_size_bits, 0);
    free(buffer);

    if(status != RC522_OK) return status;
    if(*res_size != 3) return RC522_ERR;
    return RC522_OK;
}

rc522_status authenticate(uint8_t **res, uint8_t *res_size, uint8_t *res_size_bits, picc_auth_commands auth_mode, uint8_t block_address, uint8_t *sector_key, uint8_t sector_key_size, uint8_t *uid) {
    uint8_t buffer_size = 2 + sector_key_size + 4;
    // size = 2 + sector_key_size + 4;1 for the auth_mode and 1 for address, and 4 for the first 4 bytes of uid
    uint8_t *buffer = malloc(buffer_size);
    buffer[0] = auth_mode;
    buffer[1] = block_address;
    memcpy(buffer+2, sector_key, sector_key_size);
    memcpy(buffer+2+sector_key_size, uid, 4);

    rc522_status status = send_command(MFAuthent, buffer, buffer_size, res, res_size, res_size_bits, 0);
    free(buffer);

    if(status != RC522_OK) return status;
    if(!(read_from_register(Status2Reg) & 0x08)) return RC522_ERR;
    return RC522_OK;
}

rc522_status read_block(uint8_t **res, uint8_t *res_size, uint8_t *res_size_bits, uint8_t blockAddr) {
    uint8_t *buffer = malloc(4);
    rc522_status status;
    buffer[0] = PICC_READ;
    buffer[1] = blockAddr;
    status = calculate_crc(buffer, 2, buffer+2);
    if(status != RC522_OK) return status;

    status = send_command(Transceive, buffer, 4, res, res_size, res_size_bits, 1);
    free(buffer);
    if(status != RC522_OK) return status;
    *res_size = (*res_size) - 2;//ignore crc bits
    //if(*res_size != 16) return RC522_ERR;
    return RC522_OK;
}

rc522_status write_block(uint8_t *data, uint8_t data_size, uint8_t **res, uint8_t *res_size, uint8_t *res_size_bits, uint8_t blockAddr) {
    uint8_t *buffer = malloc(4);
    rc522_status status;
    buffer[0] = PICC_WRITE;
    buffer[1] = blockAddr;
    status = calculate_crc(buffer, 2, buffer+2);
    if(status != RC522_OK) return status;

    status = send_command(Transceive, buffer, 4, res, res_size, res_size_bits, 0);
    free(buffer);
    if(status != RC522_OK) return status;
    buffer = malloc(18); // 16 bytes of data + 2 of crc
    free(*res);
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

void stop_authentication() {
    clear_bits_in_reg(Status2Reg, 0x08);
}

uint64_t convert_uid_to_number(uint8_t *uid) {
      uint64_t id = 0;
      for (size_t i = 0; i < 5; i++)
      {
        id = id * 256 + uid[i];
      }
      return id;
}