#include <stdio.h>
#include "rc522_controller.h"

int main(int argc, char const *argv[])
{
    printf("Realizando o setup do SPI\n");
    spi_setup();
    printf("Primeira leitura\n");
    read_from_register(Status1Reg);
    printf("Escrita 1\n");
    write_to_register_int(TPrescalerReg, 0x3E);
    printf("Segunda leitura\n");
    read_from_register(TPrescalerReg);
    printf("Terceira leitura\n");
    read_from_register(TPrescalerReg);
    printf("Escrita 2\n");
    write_to_register_int(TPrescalerReg, 0x3F);
    printf("Quarta leitura\n");
    read_from_register(TPrescalerReg);
    printf("Quarta leitura\n");
    read_from_register(TPrescalerReg);
    printf("Fim");
    return 0;
}
