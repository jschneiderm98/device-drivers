#include "rc522_controller.h"


void write_to_register(mfrc522_registers reg, char *data, size_t data_size) {
    char *buffer = malloc(data_size+1);
    memcpy(buffer+1, data, (data_size+1)*sizeof(char));
    buffer[0] = format_address_to_spi(reg, address_write_mode);
    spi_write(buffer, data_size+1);
    free(buffer);
}

void write_to_register_int(mfrc522_registers reg, int data) {
    char *buf = malloc(1);
    buf[0] = data;
    write_to_register(reg, buf, 1);
    free(buf);
}

void read_from_registers(mfrc522_registers reg, char **data, size_t data_size) {
    char *buffer = malloc(data_size+2);
    memcpy(buffer+1, *data, (data_size+1)*sizeof(char));
    buffer[0] = format_address_to_spi(reg, address_read_mode);
    buffer[data_size+1] = 0x00;
    spi_read(buffer, data_size+1);
    free(buffer);
    *data = buffer;
}

char read_from_register(mfrc522_registers reg) {
    char *buf = malloc(2);
    buf[0] = format_address_to_spi(reg, address_read_mode);
    buf[1] = 0x00;
    spi_read(buf, 2);
    printf("%x %x\n", buf[0] & 0xff, buf[1] & 0xff);
    free(buf);
    return buf[1];
}

void antenna_on() 
{
  char *buf = malloc(1);
  read_from_register(TxControlReg);
  if ((buf[0] & 0x03) != 0x03) {
    write_to_register_int(TxControlReg, buf[0] | 0x03);
  }
}

void init() {
    write_to_register_int(TModeReg, 0x8D);
    write_to_register_int(TPrescalerReg, 0x3E);
    write_to_register_int(TReloadRegMSB, 0);
    write_to_register_int(TReloadRegLSB, 30);

    write_to_register_int(TxASKReg, 0x40);
    write_to_register_int(ModeReg, 0x3D);
    antenna_on();
}

char format_address_to_spi(mfrc522_registers reg, adress_bit_mode mode) {
    if(mode == address_write_mode)
        return (char) (reg << 1) & 0x7E;
    return (char) ((reg << 1) & 0x7E) | 0x80;
}
