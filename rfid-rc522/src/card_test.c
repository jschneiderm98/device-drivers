#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <signal.h>

#include "rfid_rc522_drive_ioctl.h"

int dev;

void intHandler(int dummy) {
	close(dev);
	exit(0);
}

rc522_status request_picc(int file_id) {
	struct rfid_rc522_req_a_picc_dto dto;

	ioctl(file_id, IOCTL_RC522_REQ_A_PICC, &dto);
	printf("req_a_picc status %u\n", dto.status);
	if(dto.status != 0) return dto.status;
	for (size_t i = 0; i < dto.res_size; i++)
	{
		printf("req_a_picc[%d] = %x\n", i, dto.res[i]);
	}
	return dto.status;
}

rc522_status anticollision(int file_id, uint8_t uid[5]) {
	struct rfid_rc522_anticollision_dto dto_anticollision;

	ioctl(file_id, IOCTL_RC522_ANTICOLLISION, &dto_anticollision);
	printf("anticollision status %u\n", dto_anticollision.status);
	if(dto_anticollision.status != 0) return dto_anticollision.status;
		printf("uid bytes: ");
	for (size_t i = 0; i < dto_anticollision.res_size; i++)
	{
		printf("%x ", dto_anticollision.uid[i]);
	}
	printf("\n");
	memcpy(uid, dto_anticollision.uid, 5);
	return dto_anticollision.status;
}

rc522_status select_tag(int file_id, uint8_t uid[5]) {
	struct rfid_rc522_select_tag_dto dto_select_tag;

	memcpy(dto_select_tag.uid, uid, 5);
	ioctl(file_id, IOCTL_RC522_SELECT_TAG, &dto_select_tag);
	printf("dto_select_tag status %u\n", dto_select_tag.status);
	if(dto_select_tag.status != 0) return dto_select_tag.status;
	return dto_select_tag.status;
}

rc522_status auth_in_sector(int file_id, uint8_t sector, uint8_t uid[5], uint8_t sector_key[6]) {
	struct rfid_rc522_authenticate_dto dto_auth;

	memcpy(dto_auth.uid, uid, 5);
	memcpy(dto_auth.sector_key, sector_key, 6);
	dto_auth.block_address = sector*4 + 3;
	ioctl(file_id, IOCTL_RC522_AUTHENTICATE, &dto_auth);
	return dto_auth.status;
}

rc522_status read_block(int file_id, uint8_t block_address) {
	struct rfid_rc522_read_picc_block_dto dto_read;

	dto_read.block_address = block_address;
	ioctl(file_id, IOCTL_RC522_READ_PICC_BLOCK, &dto_read);

	if(dto_read.status != 0) return dto_read.status;

	printf("bloco %u: |", block_address);
	for (size_t i = 0; i < dto_read.res_size; i++)
	{
		if(block_address != 0) {
			printf("%c", dto_read.res[i]);
			continue;
		}
		printf("%u", dto_read.res[i]);
	}
	printf("|\n");
	return dto_read.status;
}

rc522_status write_block(int file_id, uint8_t block_address, char *data) {
	struct rfid_rc522_write_picc_block_dto dto_write;

	memcpy(dto_write.input, data, 16);
	dto_write.block_address = block_address;
	ioctl(file_id, IOCTL_RC522_WRITE_PICC_BLOCK, &dto_write);
	printf("write_block status %u\n", dto_write.status);
	return dto_write.status;
}

void read_all_blocks(int file_id, uint8_t *uid, uint8_t *sector_key) {
	rc522_status status;
	for (size_t sector = 0; sector < 16; sector++)
	{
		uint16_t cont_auth = 0;
		do
		{
			status = auth_in_sector(file_id, sector, uid, sector_key);
			cont_auth++;
		} while (status != RC522_OK && cont_auth < 256);
		if(cont_auth >= 256) {
			printf("Não foi possível autenticar em setor %u, rc522_status: %u\n", sector, status);
			continue;
			}	
		for (size_t block_in_sector = 0; block_in_sector < 3; block_in_sector++)
		{
			size_t current_block = sector*4 + block_in_sector;
			uint16_t cont_read = 0;
			do
			{
				status = read_block(file_id, current_block);
				cont_read++;
			} while (status != RC522_OK && cont_read < 256);
			if(cont_read >= 256) {
				printf("Não foi possível ler o bloco %u, rc522_status: %u\n", current_block, status);
			}	
		}
	}
}

int main() {
	rc522_status status;
	uint8_t key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	uint8_t uid[5];
	uint16_t cont_read = 0;
	uint16_t cont_auth = 0;

	dev = open("/dev/rfid_rc522_driver", O_NONBLOCK);
	if(dev == -1) {
		printf("Não foi possível abrir arquivo\n");
		return -1;
	}
	signal(SIGINT, intHandler);

	ioctl(dev, IOCTL_RC522_STOP_AUTH);

	status = request_picc(dev);
	if(status != RC522_OK) {
		printf("Não foi possível requisitar uma tag, rc522_status: %u\n", status);
		return -1;
	}
	
	status = anticollision(dev, uid);
	if(status != RC522_OK) {
		printf("Não foi possível realizar protocolo anticolisão, rc522_status: %u\n", status);
		return -1;
	}

	status = select_tag(dev, uid);
	if(status != RC522_OK) {
		printf("Não foi possível realizar selecionar tag, rc522_status: %u\n", status);
		return -1;
	}

	read_all_blocks(dev, uid, key);
	printf("read all %u\n", status);
	
	status = auth_in_sector(dev, 2, uid, key);
	printf("auth again %u\n", status); 
	write_block(dev, 8, "8depois depois depois");
	printf("write done\n");

	status = auth_in_sector(dev, 4, uid, key);
	printf("auth again %u\n", status); 
	write_block(dev, 16, "16depois depois depois");
	printf("write done\n");

	status = auth_in_sector(dev, 8, uid, key);
	printf("auth again %u\n", status); 
	write_block(dev, 32, "32depois depois depois");
	printf("write done\n");

	read_all_blocks(dev, uid, key);
	printf("read all done\n");

	ioctl(dev, IOCTL_RC522_STOP_AUTH);

	close(dev);
	return 0;
}