#include <linux/device.h>
#include <linux/fs.h>

// Constantes gerais do projeto

#define DEVICE_NAME "lcd_device_driver"
#define CLASS_NAME  "lcd_device_driver_class"
#define LCD_LINE_LEN 16
#define MAX_CURSOR_POS 80
#define FIRST_LINE_FIRST_POS 0
#define SECOND_LINE_FIRST_POS 40
#define EMPTY_LCD_LINE "                                        "
#define CONFIG_MESSAGE_SIZE 48
#define DATA_MESSAGE_SIZE 84

// Log function macros

#define MSG_OK(s) printk(KERN_INFO "%s: %s\n", DEVICE_NAME, s)
#define MSG_BAD(s, err_val) printk(KERN_ERR "%s: %s %ld\n", DEVICE_NAME, s, err_val)

// Constantes para comando do display lcd

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
#define COMANDO_CURSOR_SEGUNDO_LINHA 0xC0
#define COMANDO_LIMPAR_PRIMEIRA_LINHA_CHAR '8'
#define COMANDO_LIMPAR_SEGUNDO_LINHA_CHAR '9'
#define COMANDO_INVALIDO 0x0
#define COMANDO_INICIAL 0x3
#define MODO_DADO 1

// Constantes da posição de cada bit no array de gpio's

#define EN 0
#define RS 1
#define D4 2
#define D5 3
#define D6 4
#define D7 5

typedef struct lcd_device_manager
{
    int Device_Open;    // Device aberto? Usado para prevenir acesso multiplo ao device
    int Device_Counter; // Posicao do arquivo para leitura e escrita
    struct device* driver_device;
    struct cdev* cdev_data; 
    struct file_operations fops;
    dev_t dev_number;
} lcd_device_manager;

typedef struct lcd_config
{
    int posicao_cursor;  //indica posição autal do cursor 0-39 primeira linha, 40-79 segunda linha, max -> 80
    int cursor_ligado;  // indica se o cursor esta ligado no display, 0 - false, 1 - true
    int modo_linha; // indica quantas linhas estão atividas no display 1 - 1 linha, 2 - 2 linhas
} lcd_config;

void lcd_module_clean_level(unsigned int level);
int lcd_init_driver(void);
void lcd_exit_driver(void);
int lcd_device_open(lcd_device_manager *dev);
int lcd_device_release(lcd_device_manager *dev);
int lcd_data_device_open(struct inode *, struct file *);
int lcd_data_device_release(struct inode *, struct file *);
ssize_t lcd_data_device_read(struct file *, char *, size_t, loff_t *);
ssize_t lcd_data_device_write(struct file *, const char *, size_t, loff_t *);
int lcd_config_device_open(struct inode *, struct file *);
int lcd_config_device_release(struct inode *, struct file *);
ssize_t lcd_config_device_read(struct file *, char *, size_t, loff_t *);
ssize_t lcd_config_device_write(struct file *, const char *, size_t, loff_t *);
void lcd_limpar_linha(char comando);
char lcd_select_configuration(char indicador);
char lcd_send_nibble(char nibble, char nibble_type);
char lcd_send_byte(char byte, char byte_type);
void lcd_clear_screen(void);
void lcd_config_device(void);
void lcd_force_4bit_mode(void);
void lcd_send_string(char *str);
void lcd_jiffies_delay(unsigned int n);
void lcd_register_values_change(char *str, int start, int length);
void lcd_clear_values(void);