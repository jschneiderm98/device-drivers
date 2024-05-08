#include "spi_interface.h"
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

typedef enum mfrc522_registers {
    // Command and status registers
    Reserved00 = 0x00,
    CommandReg = 0x01,
    ComlEnReg = 0x02,
    DivlEnReg = 0x03,
    ComIrqReg = 0x04,
    DivIrqReg = 0x05,
    ErrorReg = 0x06,
    Status1Reg = 0x07,
    Status2Reg = 0x08,
    FIFODataReg = 0x09,
    FIFOLevelReg = 0x0A,
    WaterLevelReg = 0x0B,
    ControlReg = 0x0C,
    BitFramingReg = 0x0D,
    CollReg = 0x0E,
    Reserved0F = 0x0F,
    // Commands registers
    Reserved10 = 0x10,
    ModeReg = 0x11,
    TxModeReg = 0x12,
    RxModeReg = 0x13,
    TxControlReg = 0x14,
    TxASKReg = 0x15,
    TxSelReg = 0x16,
    RxSelReg = 0x17,
    RxThresholdReg = 0x18,
    DemodReg = 0x19,
    Reserved1A = 0x1A,
    Reserved1B = 0x1B,
    MfTxReg = 0x1C,
    MfRxReg = 0x1D,
    Reserved1E = 0x1E,
    SerialSpeedReg = 0x1F,
    // Configuration registers
    CRCResultRegMSB = 0x21,
    CRCResultRegLSB = 0x22,
    Reserved23 = 0x23,
    ModWidthReg = 0x24,
    Reserved25 = 0x25,
    RFCfgReg = 0x26,
    GsNReg = 0x27,
    CWGsPReg = 0x28,
    ModGsPReg = 0x29,
    TModeReg = 0x2A,
    TPrescalerReg = 0x2B,
    TReloadRegMSB = 0x2C,
    TReloadRegLSB = 0x2D,
    TCounterValRegMSB = 0x2E,
    TCounterValRegLSB = 0x2F,
    // Test registers
    Reserved30 = 0x30,
    TestSel1Reg = 0x31,
    TestSel2Reg = 0x32,
    TestPinEnReg = 0x33,
    TestPinValueReg = 0x34,
    TestBusReg = 0x35,
    AutoTestReg = 0x36,
    VersionReg = 0x37,
    AnalogTestReg = 0x38,
    TestDAC1Reg = 0x39,
    TestDAC2Reg = 0x3A,
    Reserved3C = 0x3C,
    Reserved3D = 0x3D,
    Reserved3E = 0x3E,
    Reserved3F = 0x3F,
} mfrc522_registers;

typedef enum adress_bit_mode {
    address_read_mode = 1,
    address_write_mode = 0,
} adress_bit_mode;

typedef enum rc522_commands {
    Idle = 0b0000,
    Mem = 0b0001,
    Generate_RandomID = 0b0010,
    CalcCRC = 0b0011,
    Transmit = 0b0100,
    NoCmdChange = 0b0111,
    Receive = 0b1000,
    Transceive = 0b1100,
    Reserved = 0b1101,
    MFAuthent = 0b1110,
    SoftReset = 0b1111,
} rc522_commands;

typedef enum rc522_status {
    RC522_OK = 0,
    RC522_NOTAGERR = 1,
    RC522_ERR = 2,
    RC522_TIMEOUT = 3,
    RC522_FAILED_UID_CHECK = 4,
    RC522_FAILED_CRC_CHECK = 5
} rc522_status;

typedef enum picc_transceive_commands {
    PICC_REQIDL = 0x26,
    PICC_REQALL = 0x52,
    PICC_ANTICOLL = 0x93,
    PICC_SElECTTAG = 0x93,
    PICC_READ = 0x30,
    PICC_WRITE = 0xA0,
    PICC_DECREMENT = 0xC0,
    PICC_INCREMENT = 0xC1,
    PICC_RESTORE = 0xC2,
    PICC_TRANSFER = 0xB0,
    PICC_HALT = 0x50,
} picc_transceive_commands;

typedef enum picc_auth_commands {
    PICC_AUTHENT1A = 0x60,
    PICC_AUTHENT1B = 0x61,
} picc_auth_commands;


void write_to_register_multiple(mfrc522_registers reg, uint8_t *data, size_t data_size);
void write_to_register(mfrc522_registers reg, uint8_t data);
void read_from_registers(mfrc522_registers reg, uint8_t **data, size_t data_size);
uint8_t read_from_register(mfrc522_registers reg);
void set_bits_in_reg(mfrc522_registers reg, uint8_t bits_to_set);
void clear_bits_in_reg(mfrc522_registers reg, uint8_t bits_to_set);
void antenna_on();
uint8_t init();
void cleanup();
uint8_t format_address_to_byte(mfrc522_registers reg, adress_bit_mode mode);
rc522_status calculate_crc(uint8_t *data, size_t data_size, uint8_t *result);
void self_test();
rc522_status req_a_picc(uint8_t **res, uint8_t *res_size, uint8_t *res_size_bits);
rc522_status anticollision(uint8_t **res, uint8_t *res_size, uint8_t *res_size_bits);
rc522_status select_tag(uint8_t **res, uint8_t *res_size, uint8_t *res_size_bits, uint8_t *uid);
rc522_status authenticate(uint8_t **res, uint8_t *res_size, uint8_t *res_size_bits, picc_auth_commands auth_mode, uint8_t block_address, uint8_t *sector_key, uint8_t sector_key_size, uint8_t *uid);
rc522_status read_block(uint8_t **res, uint8_t *res_size, uint8_t *res_size_bits, uint8_t blockAddr);
uint64_t convert_uid_to_number(uint8_t *uid);
void stop_authentication();