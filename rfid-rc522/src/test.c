#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

#include "rfid_rc522_drive_ioctl.h"

int main() {

	struct rfid_rc522_req_a_picc_dto dto;
	struct rfid_rc522_anticollision_dto dto_anticollision;
	struct rfid_rc522_select_tag_dto dto_select_tag;
	struct rfid_rc522_authenticate_dto dto_auth;
	struct rfid_rc522_read_picc_block_dto dto_read;
	struct rfid_rc522_write_picc_block_dto dto_write;
	uint8_t key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	int dev = open("/dev/rfid_rc522_driver", O_WRONLY);
	if(dev == -1) {
		printf("Não foi possível abrir arquivo\n");
		return -1;
	}

	ioctl(dev, IOCTL_RC522_REQ_A_PICC, &dto);
	printf("req_a_picc status %u\n", dto.status);
	if(dto.status != 0) return 0;
	for (size_t i = 0; i < dto.res_size; i++)
	{
		printf("req_a_picc[%d] = %x\n", i, dto.res[i]);
	}

	ioctl(dev, IOCTL_RC522_ANTICOLLISION, &dto_anticollision);
	printf("anticollision status %u\n", dto_anticollision.status);
	if(dto_anticollision.status != 0) return 0;
	for (size_t i = 0; i < dto_anticollision.res_size; i++)
	{
		printf("anticollision[%d] = %x\n", i, dto_anticollision.uid[i]);
	}

	memcpy(dto_select_tag.uid, dto_anticollision.uid, 5);
	ioctl(dev, IOCTL_RC522_SELECT_TAG, &dto_select_tag);
	printf("dto_select_tag status %u\n", dto_select_tag.status);
	if(dto_select_tag.status != 0) return 0;
	for (size_t i = 0; i < dto_select_tag.res_size; i++)
	{
		printf("dto_select_tag[%d] = %x\n", i, dto_select_tag.res[i]);
	}

	memcpy(dto_auth.uid, dto_anticollision.uid, 5);
	memcpy(dto_auth.sector_key, key, 6);
	dto_auth.block_address = 11;
	ioctl(dev, IOCTL_RC522_AUTHENTICATE, &dto_auth);
	printf("dto_auth status %u\n", dto_auth.status);
	if(dto_auth.status != 0) return 0;

	dto_read.block_address = 8;
	ioctl(dev, IOCTL_RC522_READ_PICC_BLOCK, &dto_read);

	printf("before: dto_read status %u\n", dto_read.status);
	if(dto_read.status != 0) return 0;
    printf("before: data in block %u: ", dto_read.block_address);
	for (size_t i = 0; i < dto_read.res_size; i++)
    {
        printf("%c", dto_read.res[i]);
    }
    printf("\n");

	memcpy(dto_write.input, "teste teste abcd", 16);
	dto_write.block_address = 8;
	ioctl(dev, IOCTL_RC522_WRITE_PICC_BLOCK, &dto_write);

	printf("dto_write status %u\n", dto_write.status);
	if(dto_write.status != 0) return 0;

	dto_read.block_address = 8;
	dto_read.res_size = 0;
	dto_read.status = -1;
	ioctl(dev, IOCTL_RC522_READ_PICC_BLOCK, &dto_read);

	printf("after: dto_read status %u\n", dto_read.status);
	if(dto_read.status != 0) return 0;
    printf("after: data in block %u: ", dto_read.block_address);
	for (size_t i = 0; i < dto_read.res_size; i++)
    {
        printf("%c", dto_read.res[i]);
    }
    printf("\n");

	ioctl(dev, IOCTL_RC522_STOP_AUTH);


	close(dev);
	return 0;
}