#include <linux/device.h>

#define MODO_COMANDO 0
#define COMANDO_1_LINE 0x20
#define COMANDO_2_LINE 0x28
#define COMANDO_CURSOR_VISIVEL 0x0F
#define COMANDO_CURSOR_NAO_VISIVEL 0x0C
#define COMANDO_LIMPAR_DISPLAY 0x01
#define COMANDO_RETORNAR_CURSOR 0x02
#define MODO_DADO 1

typedef struct my_device
{
    int Device_Open;    // Device aberto? Usado para prevenir acesso multiplo ao device
    int Device_Counter; // Posicao do arquivo para leitura e escrita
    struct device* driver_device;
    struct cdev* cdev_data; 
    struct file_operations fops;
} my_device;