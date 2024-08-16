#include <linux/types.h>

#ifndef RC522_IOCTL_H
#define RC522_IOCTL_H

    typedef enum rc522_status {
        RC522_UNDEFINED = -1,
        RC522_OK = 0,
        RC522_NOTAGERR = 1,
        RC522_ERR = 2,
        RC522_TIMEOUT = 3,
        RC522_FAILED_UID_CHECK = 4,
        RC522_FAILED_CRC_CHECK = 5,
        RC522_COLLISION = 6,
        RC522_BUFFER_TOO_SMALL = 7,
        RC522_NACK = 0Xff,
    } rc522_status;

    struct rfid_rc522_write_register_dto
    {
        uint8_t addr;
        uint8_t data;
    };

    struct rfid_rc522_write_multiple_register_dto
    {
        uint8_t addr;
        uint8_t data[64];
        uint8_t data_size;
    };

    struct rfid_rc522_req_a_picc_dto
    {
        uint8_t res_size;
        uint8_t res[2];
        rc522_status status;
    };

    struct rfid_rc522_anticollision_dto
    {
        uint8_t uid[5];
        uint8_t res_size;
        rc522_status status;
    };

    struct rfid_rc522_select_tag_dto
    {
        uint8_t uid[5];
        uint8_t res_size;
        uint8_t res[3];
        rc522_status status;
    };

    struct rfid_rc522_authenticate_dto
    {
        uint8_t block_address;
        uint8_t sector_key[6];
        uint8_t uid[5];
        rc522_status status;
    };

    struct rfid_rc522_read_picc_block_dto
    {
        uint8_t block_address;
        uint8_t res_size;
        uint8_t res[16];
        rc522_status status;
    };

    struct rfid_rc522_write_picc_block_dto
    {
        uint8_t block_address;
        uint8_t input[16];
        uint8_t res[1];
        rc522_status status;
    };
    
    #define IOCTL_RC522_READ_REGISTER _IOWR('k', 0xc0, uint8_t *)
    #define IOCTL_RC522_WRITE_REGISTER _IOW('k', 0xc1, struct rfid_rc522_write_register_dto *)
    #define IOCTL_RC522_WRITE_REGISTER_MULTIPLE _IOW('k', 0xc2, struct rfid_rc522_write_multiple_register_dto *)
    #define IOCTL_RC522_REQ_A_PICC _IOR('k', 0xc3, struct rfid_rc522_req_a_picc_dto *)
    #define IOCTL_RC522_ANTICOLLISION _IOWR('k', 0xc4, struct rfid_rc522_anticollision_dto *)
    #define IOCTL_RC522_SELECT_TAG _IOWR('k', 0xc5, struct rfid_rc522_select_tag_dto *)
    #define IOCTL_RC522_AUTHENTICATE _IOWR('k', 0xc6, struct rfid_rc522_authenticate_dto *)
    #define IOCTL_RC522_READ_PICC_BLOCK _IOWR('k', 0xc7, struct rfid_rc522_read_picc_block_dto *)
    #define IOCTL_RC522_WRITE_PICC_BLOCK _IOWR('k', 0xc8, struct rfid_rc522_write_picc_block_dto *)
    #define IOCTL_RC522_STOP_AUTH _IO('k', 0xc9)
#endif