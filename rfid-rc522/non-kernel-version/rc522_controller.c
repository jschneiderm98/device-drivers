#include "rc522_controller.h"


void write_to_register_multiple(mfrc522_registers reg, uint8_t *data, size_t data_size) {
    uint8_t *buffer = malloc(data_size+1);// byte0 - address, bytes 1-n - data
    memcpy(buffer+1, data, data_size+1);
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
    write_to_register(TxControlReg, txControlRegByte | 0b00000011);
  }
}

void init() {
    reset();

    write_to_register(TModeReg, 0x8D);
    write_to_register(TPrescalerReg, 0x3E);
    write_to_register(TReloadRegLSB, 30);
    write_to_register(TReloadRegMSB, 0);

    write_to_register(TxASKReg, 0x40);
    write_to_register(ModeReg, 0x3D);
    antenna_on();
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

void idle_command() {
    set_bits_in_reg(CommandReg, Idle);
}

void reset_fifo_buffer() {
    set_bits_in_reg(FIFOLevelReg, 0b10000000);
}

int operationDidNotTimeout(struct timeval start, struct timeval current, int timeout_in_us) {
    int elapsed_time = ((current.tv_sec - start.tv_sec) * 1000000) + (current.tv_usec - start.tv_usec);
    return elapsed_time < timeout_in_us;
}

int isCrcOperationDone() {
    uint8_t divIrqRegByte = read_from_register(DivIrqReg);
    return divIrqRegByte & 0b00000100;
}

void calculate_crc(uint8_t *data, size_t data_size, uint8_t *result) {
    struct timeval start, current;
    idle_command();
    clear_bits_in_reg(DivIrqReg, 0b00000100);
    reset_fifo_buffer();

    //write the data to FIFO Buffer
    write_to_register_multiple(FIFODataReg, data, data_size);

    write_to_register(CommandReg, CalcCRC);
    gettimeofday(&start,NULL);
    gettimeofday(&current,NULL);
    while (operationDidNotTimeout(start, current, 10000)) {
        printf("Passou no while\n");
        if(isCrcOperationDone()) {
            idle_command();
            result[0] = read_from_register(CRCResultRegLSB);
            result[1] = read_from_register(CRCResultRegMSB);
            printf("Result in crc calc MSB: "BYTE_TO_BINARY_PATTERN", LSB: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(result[1]), BYTE_TO_BINARY(result[0]));
            return;
        }
        usleep(500);
        gettimeofday(&current,NULL);
    }
    printf("Timeout in crc calc");
    //timeout
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
    reset_fifo_buffer();

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
    idle_command();
    reset();
}

int isCommandError(uint8_t waitIrq) {
    uint8_t comIrqRegByte = read_from_register(ComIrqReg);
    return (comIrqRegByte & 0b00000001) || (comIrqRegByte & waitIrq);
}

void send_command(uint8_t command, uint8_t *data, size_t data_size, uint8_t **response, uint8_t *response_size, uint8_t *response_size_bits) {
    uint8_t irqEn = 0x00;
    uint8_t waitIrq = 0x00;
    struct timeval start, current;

    if(command == MFAuthent) {
        irqEn = 0x12;
        waitIrq = 0x10;
    }

    if(command == Transceive) {
        irqEn = 0x77;
        waitIrq = 0x30;
    }

    idle_command();
    clear_bits_in_reg(ComIrqReg, 0b01111111);
    reset_fifo_buffer();
    //write_to_register(ComlEnReg, irqEn | 0b10000000);
    
    write_to_register_multiple(CommandReg, data, data_size);

    if(command == Transceive) {
        set_bits_in_reg(BitFramingReg, 0b10000000);
    }

    write_to_register(CommandReg, command);

    gettimeofday(&start,NULL);
    gettimeofday(&current,NULL);

    while (operationDidNotTimeout(start, current, 10000) && !(isCommandError(waitIrq))) {
        usleep(500);
        gettimeofday(&current,NULL);
    }

    clear_bits_in_reg(BitFramingReg, 0b10000000);
    //reset_fifo_buffer();
    //idle_command();
    uint8_t errorRegValue = read_from_register(ErrorReg); // ErrorReg[7..0] bits are: WrErr TempErr reserved BufferOvfl CollErr CRCErr ParityErr ProtocolErr
    if (errorRegValue & 0x13) 
    {  // BufferOvfl ParityErr ProtocolErr
        return;
    }
    (*response_size) = read_from_register(FIFOLevelReg);
    uint8_t last_bits = read_from_register(ControlReg) & 0b00000111;
    uint8_t *buffer;
    (*response_size_bits) = last_bits ? ((*response_size) - 1) * 8 + last_bits : (*response_size) * 8;
    read_from_register_multiple(FIFODataReg, &buffer, (*response_size));
    (*response) = malloc((*response_size) + 1);
    memcpy(buffer+1, (*response), (*response_size));
}

void request_picc(uint8_t request_mode, uint8_t **res, uint8_t *res_size, uint8_t *res_size_bits) {
    write_to_register(BitFramingReg, 0x07);
    send_command(Transceive, &request_mode, 1, res, res_size, res_size_bits);
    for (size_t i = 0; i < (*res_size); i++)
    {
        printf("%x\n", (*res)[i]);
    }
}