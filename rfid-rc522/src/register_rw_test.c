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

int main() {
	const uint8_t VERSION_REG_ADDR = 0x37;
	const uint8_t FIFO_DATA_REG_ADDR = 0x09;
	const uint8_t FIFO_LEVEL_REG_ADDR = 0x0A;
	uint8_t read_ioctl_data;
	struct rfid_rc522_write_register_dto write_dto;
	struct rfid_rc522_write_multiple_register_dto write_multiple_dto;

	dev = open("/dev/rfid_rc522_driver", O_NONBLOCK);
	if(dev == -1) {
		printf("Não foi possível abrir arquivo\n");
		return -1;
	}
	signal(SIGINT, intHandler);
	read_ioctl_data = VERSION_REG_ADDR;
	ioctl(dev, IOCTL_RC522_READ_REGISTER, &read_ioctl_data);
	printf("VersionReg Value: 0x%x\n", read_ioctl_data);

	read_ioctl_data = FIFO_LEVEL_REG_ADDR;
	ioctl(dev, IOCTL_RC522_READ_REGISTER, &read_ioctl_data);
	printf("FIFOLevelReg Value(inicial): %u\n", read_ioctl_data);

	write_dto.addr = FIFO_DATA_REG_ADDR;
	write_dto.data = 11;
	ioctl(dev, IOCTL_RC522_WRITE_REGISTER, &write_dto);

	read_ioctl_data = FIFO_LEVEL_REG_ADDR;
	ioctl(dev, IOCTL_RC522_READ_REGISTER, &read_ioctl_data);
	printf("FIFOLevelReg Value(apos inserir um byte): %u\n", read_ioctl_data);

	write_multiple_dto.addr = FIFO_DATA_REG_ADDR;
	write_multiple_dto.data[0] = 12;
	write_multiple_dto.data[1] = 22;
	write_multiple_dto.data[2] = 32;
	write_multiple_dto.data[3] = 42;
	write_multiple_dto.data_size = 4;
	ioctl(dev, IOCTL_RC522_WRITE_REGISTER_MULTIPLE, &write_multiple_dto);
	
	read_ioctl_data = FIFO_LEVEL_REG_ADDR;
	ioctl(dev, IOCTL_RC522_READ_REGISTER, &read_ioctl_data);
	printf("FIFOLevelReg Value(apos mais 4 bytes): %u\n", read_ioctl_data);

	read_ioctl_data = FIFO_DATA_REG_ADDR;
	ioctl(dev, IOCTL_RC522_READ_REGISTER, &read_ioctl_data);
	printf("FIFO_DATA_REG_ADDR(primeiro valor inserido) Value: %u\n", read_ioctl_data);

	read_ioctl_data = FIFO_DATA_REG_ADDR;
	ioctl(dev, IOCTL_RC522_READ_REGISTER, &read_ioctl_data);
	printf("FIFO_DATA_REG_ADDR(segundo valor inserido) Value: %u\n", read_ioctl_data);

	read_ioctl_data = FIFO_DATA_REG_ADDR;
	ioctl(dev, IOCTL_RC522_READ_REGISTER, &read_ioctl_data);
	printf("FIFO_DATA_REG_ADDR(terceiro valor inserido) Value: %u\n", read_ioctl_data);

	close(dev);
	return 0;
}