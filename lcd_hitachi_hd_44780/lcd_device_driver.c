#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include "lcd_device_driver.h"

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Diogo Caetano Garcia <diogogarcia@unb.br>, Júlio César Schneider Martins <jschneiderm98@gmail.com>");
MODULE_DESCRIPTION("Device driver to interface with lcd displays that use hitashi_hd_44780");


unsigned short en = 4;
module_param(en, short, 0644);
MODULE_PARM_DESC(en, "Optional, gpio for the EN pin of the lcd. Defaults to gpio 4");
unsigned short rs = 17;
module_param(rs, short, 0644);
MODULE_PARM_DESC(rs, "Optional, gpio for the RS pin of the lcd. Defaults to gpio 17");
unsigned short d4 = 22;
module_param(d4, short, 0644);
MODULE_PARM_DESC(d4, "Optional, gpio for the D4 pin of the lcd. Defaults to gpio 22");
unsigned short d5 = 23;
module_param(d5, short, 0644);
MODULE_PARM_DESC(d5, "Optional, gpio for the D5 pin of the lcd. Defaults to gpio 23");
unsigned short d6 = 24;
module_param(d6, short, 0644);
MODULE_PARM_DESC(d6, "Optional, gpio for the D6 pin of the lcd. Defaults to gpio 24");
unsigned short d7 = 25;
module_param(d7, short, 0644);
MODULE_PARM_DESC(d7, "Optional, gpio for the D7 pin of the lcd. Defaults to gpio 25");

static struct gpio lcd_pins[] = {
		{  4, GPIOF_OUT_INIT_LOW, "EN" },
		{ 17, GPIOF_OUT_INIT_LOW, "RS" },
		{ 22, GPIOF_OUT_INIT_LOW, "D4" },
		{ 23, GPIOF_OUT_INIT_LOW, "D5" },
		{ 24, GPIOF_OUT_INIT_LOW, "D6" },
		{ 25, GPIOF_OUT_INIT_LOW, "D7" },
};

static struct class*  lcd_device_driver_Class  = NULL; ///< The device-driver class struct pointer
static char full_lcd_characters[MAX_CURSOR_POS + 1];
static char lcd_data_buffer[DATA_MESSAGE_SIZE];
static char lcd_config_buffer[CONFIG_MESSAGE_SIZE];

lcd_config config = {
	.cursor_ligado = 1,
	.modo_linha = 1,
	.posicao_cursor = 7
};


lcd_device_manager data_device = {
	.Device_Open = 0,
	.Device_Counter = 0,
	.fops = {
		.read = lcd_data_device_read,
		.write = lcd_data_device_write,
		.open = lcd_data_device_open,
		.release = lcd_data_device_release
	},
	.cdev_data = NULL,
	.driver_device = NULL,
	.dev_number = 0
};

lcd_device_manager config_device = {
	.Device_Open = 0,
	.Device_Counter = 0,
	.fops = {
		.read = lcd_config_device_read,
		.write = lcd_config_device_write,
		.open = lcd_config_device_open,
		.release = lcd_config_device_release
	},
	.cdev_data = NULL,
	.driver_device = NULL,
	.dev_number = 0
};

#define CLEAN_ALL                            0
#define CLEAN_DEVICE_CONFIG                  1
#define CLEAN_DEVICE_DATA                    2
#define CLEAN_CLASS                          3
#define CLEAN_CDEV_CONFIG                    4
#define CLEAN_CDEV_DATA                      5
#define CLEAN_REGION                         6
void lcd_module_clean_level(unsigned int level)
{
	int i;
	if(level<1)
	{
		// Apagar LEDs
		for(i = 0; i < ARRAY_SIZE(lcd_pins); i++)
			gpio_set_value(lcd_pins[i].gpio, 0);
		// Liberar GPIOs dos LEDs
		gpio_free_array(lcd_pins, ARRAY_SIZE(lcd_pins));
	}
	if(level<2) { // Remove o dispositivo de configuracao
		device_destroy(lcd_device_driver_Class, config_device.dev_number);
	}
	if(level<3) { // Remove o dispositivo de dados
		device_destroy(lcd_device_driver_Class, data_device.dev_number);
	}
	if(level<4) // Desfaz o registro da classe de dispositivo e remove a classe de dispositivo
	{
		class_unregister(lcd_device_driver_Class);
		class_destroy(lcd_device_driver_Class);
	}
	if(level<5) // Desaloca a struct cdev para o dispositivo de configuracao
	{
		cdev_del(config_device.cdev_data);
	}
	if(level<6) // Desaloca a struct cdev para o dispositivo de dados
	{
		cdev_del(data_device.cdev_data);
	}
	if(level<7) // Desfaz o registro do major number e da regiao alocada
		unregister_chrdev_region(data_device.dev_number, 1);
}

int __init lcd_init_driver(void)
{
	int ret, i, result, err;

	result = alloc_chrdev_region(&data_device.dev_number, 0, 2, DEVICE_NAME);
	if(result < 0)
	{
		MSG_BAD("Error while allocating char device region", (long int)result);
		return result;
	}
	config_device.dev_number = MKDEV(MAJOR(data_device.dev_number), MINOR(data_device.dev_number) + 1);

	data_device.cdev_data = cdev_alloc();
	if(!data_device.cdev_data) {
		lcd_module_clean_level(CLEAN_REGION); 
		MSG_BAD("Error while allocating data-cdev", 0L);
		return -1;
	}

	config_device.cdev_data = cdev_alloc();
	if(!config_device.cdev_data) {
		lcd_module_clean_level(CLEAN_CDEV_DATA); 
		MSG_BAD("Error while allocating config-cdev", 0L);
		return -1;
	}

	cdev_init(data_device.cdev_data, &data_device.fops);
	cdev_init(config_device.cdev_data, &config_device.fops);

	err = cdev_add(data_device.cdev_data, data_device.dev_number, 1);
	if (err < 0)
	{
		lcd_module_clean_level(CLEAN_CDEV_CONFIG);
		MSG_BAD("Error while adding cdev", (long int)err);
		return err;
	}

	err = cdev_add(config_device.cdev_data, config_device.dev_number, 1);
	if (err < 0)
	{
		lcd_module_clean_level(CLEAN_CDEV_CONFIG);
		MSG_BAD("Error while adding cdev", (long int)err);
		return err;
	}

	// Registrar a classe do dispositivo
	lcd_device_driver_Class = class_create(THIS_MODULE, CLASS_NAME);
	if(IS_ERR(lcd_device_driver_Class)) // Se houve erro no registro
	{
		lcd_module_clean_level(CLEAN_CDEV_CONFIG);
		unregister_chrdev_region(data_device.dev_number, 1);
		MSG_BAD("falhou em registrar a classe do dispositivo", PTR_ERR(lcd_device_driver_Class));
		return PTR_ERR(lcd_device_driver_Class); // Correct way to return an error on a pointer
	}
	MSG_OK("classe do dispositivo registrada corretamente");

	// Registrar o device driver
	data_device.driver_device = device_create(lcd_device_driver_Class, NULL, data_device.dev_number, NULL, DEVICE_NAME "!data");
	if(IS_ERR(data_device.driver_device)) // Se houve erro no registro
	{
		lcd_module_clean_level(CLEAN_CLASS);
		MSG_BAD("falhou em criar o device driver de dados", PTR_ERR(data_device.driver_device));
		return PTR_ERR(data_device.driver_device);
	}
	MSG_OK("dispositivo de dados criado corretamente");

	config_device.driver_device = device_create(lcd_device_driver_Class, NULL, config_device.dev_number, NULL, DEVICE_NAME "!config");
	if(IS_ERR(config_device.driver_device)) // Se houve erro no registro
	{
		lcd_module_clean_level(CLEAN_DEVICE_DATA);
		MSG_BAD("falhou em criar o device driver de configuracao", PTR_ERR(config_device.driver_device));
		return PTR_ERR(config_device.driver_device);
	}
	MSG_OK("dispositivo de configuracao criado corretamente");

	// Registrar GPIOs para LEDs, ligar LEDs

	lcd_pins[EN].gpio = en;
	lcd_pins[RS].gpio = rs;
	lcd_pins[D4].gpio = d4;
	lcd_pins[D5].gpio = d5;
	lcd_pins[D6].gpio = d6;
	lcd_pins[D7].gpio = d7;

	ret = gpio_request_array(lcd_pins, ARRAY_SIZE(lcd_pins));
	if(ret) {
		lcd_module_clean_level(CLEAN_DEVICE_CONFIG);
		MSG_BAD("não conseguiu acesso ao GPIO", (long int)ret);
	}
	else
		MSG_OK("modulo carregado");

	for(i=0; i < MAX_CURSOR_POS; i++)
		full_lcd_characters[i] = ' ';
	full_lcd_characters[MAX_CURSOR_POS] = '\0';

	lcd_config_device();
	lcd_send_string("Ola LCD");
	lcd_register_values_change("Ola LCD", 0, 7);

	return ret;
}

void __exit lcd_exit_driver(void)
{
	lcd_clear_screen();
	lcd_module_clean_level(CLEAN_ALL);
	MSG_OK("modulo descarregado");
}

int lcd_device_open(lcd_device_manager *dev) {
	if(dev->Device_Open) return -EBUSY;
	dev->Device_Open++; // Trava acesso ao device
	dev->Device_Counter = 0; // Conta quantos caracteres foram lidos
	try_module_get(THIS_MODULE); // Incrementa o contador de uso do modulo
	return 0;
}

int lcd_data_device_open(struct inode *inode, struct file *file)
{
	snprintf(lcd_data_buffer, DATA_MESSAGE_SIZE, "|%s|\n", full_lcd_characters);
	return lcd_device_open(&data_device);
}

int lcd_config_device_open(struct inode *inode, struct file *file)
{
	snprintf(
		lcd_config_buffer, 
		CONFIG_MESSAGE_SIZE, 
		"posicao_cursor:%02d;cursor_ligado:%d;modo_linha:%d\n", 
		config.posicao_cursor, config.cursor_ligado, config.modo_linha
	);
	return lcd_device_open(&config_device);
}

int lcd_device_release(lcd_device_manager *dev)
{
	dev->Device_Open--; // Libera acesso ao device
	module_put(THIS_MODULE); // Decrementa o contador de uso do modulo
	return 0;
}

int lcd_data_device_release(struct inode *inode, struct file *file)
{
	return lcd_device_release(&data_device);
}

int lcd_config_device_release(struct inode *inode, struct file *file)
{
	return lcd_device_release(&config_device);
}

ssize_t lcd_data_device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	int actual_length;
	// End of file
	if(data_device.Device_Counter >= DATA_MESSAGE_SIZE)
		return 0;
	actual_length = length + data_device.Device_Counter > DATA_MESSAGE_SIZE ? (DATA_MESSAGE_SIZE - data_device.Device_Counter) : length;

	snprintf(buffer, actual_length, "%s", lcd_data_buffer+data_device.Device_Counter);
	data_device.Device_Counter += actual_length;
	return actual_length;
}

ssize_t lcd_config_device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	int actual_length = length + config_device.Device_Counter > CONFIG_MESSAGE_SIZE ? (CONFIG_MESSAGE_SIZE - config_device.Device_Counter) : length;

	if (config_device.Device_Counter > 0) 
		return 0;
	snprintf(buffer, actual_length, "%s", lcd_config_buffer+config_device.Device_Counter);
	config_device.Device_Counter = actual_length;
	return actual_length;
}

ssize_t lcd_data_device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	int new_cursor_pos, actual_length;
	char local_buff[MAX_CURSOR_POS+1];
	//lcd_clear_screen();
	if (config.posicao_cursor >= MAX_CURSOR_POS) {
		return len;
	}
	new_cursor_pos = config.posicao_cursor + len;
	actual_length = new_cursor_pos > MAX_CURSOR_POS ? (MAX_CURSOR_POS - config.posicao_cursor) : len;
	new_cursor_pos = config.posicao_cursor + actual_length;
	snprintf(local_buff, actual_length+1, buff);
	lcd_send_string(local_buff);
	lcd_register_values_change(local_buff, config.posicao_cursor, actual_length);
	config.posicao_cursor = new_cursor_pos % MAX_CURSOR_POS;
	return len;
}

void lcd_limpar_linha(char comando) {
	if (comando == COMANDO_LIMPAR_PRIMEIRA_LINHA_CHAR) {
		lcd_send_byte(COMANDO_RETORNAR_CURSOR, MODO_COMANDO);
		lcd_send_string(EMPTY_LCD_LINE);
		lcd_send_byte(COMANDO_RETORNAR_CURSOR, MODO_COMANDO);
		config.posicao_cursor = 0;
		lcd_register_values_change(EMPTY_LCD_LINE, FIRST_LINE_FIRST_POS, LCD_LINE_LEN);
		return;
	}
	lcd_send_byte(COMANDO_CURSOR_SEGUNDO_LINHA, MODO_COMANDO);
	lcd_send_string(EMPTY_LCD_LINE);
	lcd_send_byte(COMANDO_CURSOR_SEGUNDO_LINHA, MODO_COMANDO);
	config.posicao_cursor = 40;
	lcd_register_values_change(EMPTY_LCD_LINE, SECOND_LINE_FIRST_POS, SECOND_LINE_FIRST_POS + LCD_LINE_LEN);
}

char lcd_select_configuration(char indicador) {
	switch(indicador) {
		case COMANDO_1_LINE_CHAR:
			lcd_send_byte(COMANDO_LIMPAR_DISPLAY, MODO_COMANDO);
			lcd_clear_values();
			config.modo_linha = 1;
			return COMANDO_1_LINE;
		case COMANDO_2_LINE_CHAR:
			lcd_send_byte(COMANDO_LIMPAR_DISPLAY, MODO_COMANDO);
			lcd_clear_values();
			config.modo_linha = 2;
			return COMANDO_2_LINE;
		case COMANDO_CURSOR_VISIVEL_CHAR:
			config.cursor_ligado = 1;
			return COMANDO_CURSOR_VISIVEL;
		case COMANDO_CURSOR_NAO_VISIVEL_CHAR:
			config.cursor_ligado = 0;
			return COMANDO_CURSOR_NAO_VISIVEL;
		case COMANDO_LIMPAR_DISPLAY_CHAR:
			lcd_clear_values();
			return COMANDO_LIMPAR_DISPLAY;
		case COMANDO_RETORNAR_CURSOR_CHAR:
			config.posicao_cursor = 0;
			return COMANDO_RETORNAR_CURSOR;
		case COMANDO_CURSOR_SEGUNDO_LINHA_CHAR:
			config.posicao_cursor = 40;
			return COMANDO_CURSOR_SEGUNDO_LINHA;
		case COMANDO_LIMPAR_PRIMEIRA_LINHA_CHAR:
		case COMANDO_LIMPAR_SEGUNDO_LINHA_CHAR:
			lcd_limpar_linha(indicador);
			return COMANDO_INVALIDO;
		default:
			return COMANDO_INVALIDO;
	}
}

ssize_t lcd_config_device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	char comando = lcd_select_configuration(buff[0]);
	if(comando != COMANDO_INVALIDO)
		lcd_send_byte(comando, MODO_COMANDO);
	return 1;
}

char lcd_send_nibble(char nibble, char nibble_type)
{
	if((nibble_type!=MODO_DADO)&&(nibble_type!=MODO_COMANDO))
		return -1;
	gpio_set_value(lcd_pins[EN].gpio, 1);
	gpio_set_value(lcd_pins[RS].gpio, nibble_type);
	gpio_set_value(lcd_pins[D4].gpio, nibble&1);
	gpio_set_value(lcd_pins[D5].gpio, (nibble>>1)&1);
	gpio_set_value(lcd_pins[D6].gpio, (nibble>>2)&1);
	gpio_set_value(lcd_pins[D7].gpio, (nibble>>3)&1);
	gpio_set_value(lcd_pins[EN].gpio, 0);
	lcd_jiffies_delay(1);
	return 0;
}

char lcd_send_byte(char byte, char byte_type)
{
	if(lcd_send_nibble(byte>>4, byte_type)==-1)
		return -1;
	lcd_send_nibble(byte & 0xF, byte_type);
	return 0;
}

void lcd_clear_screen(void)
{
	lcd_send_byte(COMANDO_LIMPAR_DISPLAY, MODO_COMANDO);
	lcd_jiffies_delay(2);
	lcd_send_byte(COMANDO_RETORNAR_CURSOR, MODO_COMANDO);
	lcd_jiffies_delay(2);
}

void lcd_force_4bit_mode(void)
{
	// Manda 3 nibbles 0011, garantindo que o lcd esteja em um modo 8 bits
	// independente de estar no modo 4 bits ou 8 bits anteriormente
	lcd_send_nibble(COMANDO_INICIAL, MODO_COMANDO);
	lcd_send_nibble(COMANDO_INICIAL, MODO_COMANDO);
	lcd_send_nibble(COMANDO_INICIAL, MODO_COMANDO);
	// Manda 1 nibble setando o modo 4 bits
	lcd_send_nibble(COMANDO_NIBBLE_INICIAL_MODO_4_BITS, MODO_COMANDO);
}

void lcd_config_device(void)
{
	lcd_jiffies_delay(1);
	lcd_force_4bit_mode();
	lcd_send_byte(COMANDO_CURSOR_VISIVEL, MODO_COMANDO);
	lcd_send_byte(COMANDO_1_LINE, MODO_COMANDO);
	lcd_clear_screen();
}

void lcd_send_string(char *str)
{
	int i = 0;
	while(str[i]!='\0')
	{
		lcd_send_byte(str[i], MODO_DADO);
		i++;
	}
}

void lcd_jiffies_delay(unsigned int n)
{
	unsigned long t_end = jiffies + n*HZ/100;
	while(jiffies<t_end);
}

void lcd_register_values_change(char *str, int start, int length) {
	int i, new_cursor_pos, actual_length;
	if (start >= MAX_CURSOR_POS) {
		return;
	}
	
	new_cursor_pos = start + length;
	actual_length = new_cursor_pos > MAX_CURSOR_POS ? (MAX_CURSOR_POS - start) : length;
	new_cursor_pos = start + actual_length;

	for(i = 0; i < actual_length && str[i] != '\0' && str[i] != '\x00'; i++) {
		full_lcd_characters[i + start] = str[i];
	}
}

void lcd_clear_values(void) {
	int i;
	for(i=0; i < MAX_CURSOR_POS; i++)
		full_lcd_characters[i] = ' ';
	full_lcd_characters[MAX_CURSOR_POS] = '\0';
	config.posicao_cursor = 0;
}

module_init(lcd_init_driver);
module_exit(lcd_exit_driver);