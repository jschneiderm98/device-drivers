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
        RC522_FAILED_CRC_CHECK = 5
    } rc522_status;

    struct rfid_rc522_write_register_dto
    {
        uint8_t addr;
        uint8_t data;
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
        rc522_status status;
    };
    
    #define IOCTL_RC522_READ_REGISTER _IOWR('a', 0, uint8_t *)
    #define IOCTL_RC522_WRITE_REGISTER _IOW('a', 1, struct rfid_rc522_write_register_dto *)
    #define IOCTL_RC522_REQ_A_PICC _IOR('a', 2, struct rfid_rc522_req_a_picc_dto *)
    #define IOCTL_RC522_ANTICOLLISION _IOWR('a', 3, struct rfid_rc522_anticollision_dto *)
    #define IOCTL_RC522_SELECT_TAG _IOWR('a', 4, struct rfid_rc522_select_tag_dto *)
    #define IOCTL_RC522_AUTHENTICATE _IOWR('a', 5, struct rfid_rc522_authenticate_dto *)
    #define IOCTL_RC522_READ_PICC_BLOCK _IOWR('a', 6, struct rfid_rc522_read_picc_block_dto *)
    #define IOCTL_RC522_WRITE_PICC_BLOCK _IOWR('a', 7, struct rfid_rc522_write_picc_block_dto *)
    #define IOCTL_RC522_STOP_AUTH _IO('a', 7)
#endif