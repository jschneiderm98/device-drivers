#include "spi_interface.h"

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


void write_to_register(mfrc522_registers reg, char *data, size_t data_size);
void write_to_register_int(mfrc522_registers reg, int data);
void read_from_registers(mfrc522_registers reg, char **data, size_t data_size);
char read_from_register(mfrc522_registers reg);
void antenna_on();
void init();
char format_address_to_spi(mfrc522_registers reg, adress_bit_mode mode);