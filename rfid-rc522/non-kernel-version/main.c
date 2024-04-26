#include <stdio.h>
#include "rc522_controller.h"

int main(int argc, char const *argv[])
{
    uint8_t data[10] = {0x93, 0x70, 0x32, 0x52, 0xa1}, result[2];
    uint8_t *res, res_size, res_size_bits;
    printf("Realizando o setup do SPI\n");
    spi_setup();
    init();

    request_picc(PICC_REQIDL, &res, &res_size, &res_size_bits);
    printf("res_size: %u\n", res_size);
    printf("res_size: %u\n", res_size_bits);
    //calculate_crc(data, 5, result);
    //self_test();
    printf("Fim\n");
    return 0;
}
