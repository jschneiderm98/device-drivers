#include <linux/device.h>

#define MODO_COMANDO 0
#define COMANDO_NIBBLE_INICIAL_MODO_4_BITS 0x2
#define COMANDO_1_LINE 0x20
#define COMANDO_1_LINE_CHAR '1'
#define COMANDO_2_LINE 0x28
#define COMANDO_2_LINE_CHAR '2'
#define COMANDO_CURSOR_VISIVEL 0x0F
#define COMANDO_CURSOR_VISIVEL_CHAR '3'
#define COMANDO_CURSOR_NAO_VISIVEL 0x0C
#define COMANDO_CURSOR_NAO_VISIVEL_CHAR '4'
#define COMANDO_LIMPAR_DISPLAY 0x01
#define COMANDO_LIMPAR_DISPLAY_CHAR '5'
#define COMANDO_RETORNAR_CURSOR 0x02
#define COMANDO_RETORNAR_CURSOR_CHAR '6'
#define COMANDO_CURSOR_SEGUNDO_LINHA 0xC0
#define COMANDO_CURSOR_SEGUNDO_LINHA_CHAR '7'
#define COMANDO_INVALIDO 0x0
#define COMANDO_INICIAL 0x3
#define MODO_DADO 1

typedef struct my_device
{
    int Device_Open;    // Device aberto? Usado para prevenir acesso multiplo ao device
    int Device_Counter; // Posicao do arquivo para leitura e escrita
    struct device* driver_device;
    struct cdev* cdev_data; 
    struct file_operations fops;
    dev_t dev_number;
} my_device;

typedef struct lcd_config
{
    int posicao_cursor;  //indica posição autal do cursor 0-15 primeira linha, 40-55 segunda linha
    int cursor_ligado;  // indica se o cursor esta ligado no display, 0 - false, 1 - true
    int modo_linha; // indica quantas linhas estão atividas no display 1 - 1 linha, 2 - 2 linhas
} lcd_config;