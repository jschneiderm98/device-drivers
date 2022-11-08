// Codigo para criar um dispositivo de escrita em GPIO

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include "constants.h"

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Diogo Caetano Garcia <diogogarcia@unb.br>, Júlio César Schneider Martins <jschneiderm98@gmail.com>");
MODULE_DESCRIPTION("Ola LCD Device Driver!");

#define DEVICE_NAME "lcd_device_driver"
#define CLASS_NAME  "lcd_device_driver_class"
#define MSG_OK(s) printk(KERN_INFO "%s: %s\n", DEVICE_NAME, s)
#define MSG_BAD(s, err_val) printk(KERN_ERR "%s: %s %ld\n", DEVICE_NAME, s, err_val)

int init_module(void);
void cleanup_module(void);
static int data_device_open(struct inode *, struct file *);
static int data_device_release(struct inode *, struct file *);
static ssize_t data_device_read(struct file *, char *, size_t, loff_t *);
static ssize_t data_device_write(struct file *, const char *, size_t, loff_t *);
static int config_device_open(struct inode *, struct file *);
static int config_device_release(struct inode *, struct file *);
static ssize_t config_device_read(struct file *, char *, size_t, loff_t *);
static ssize_t config_device_write(struct file *, const char *, size_t, loff_t *);
char Send_Nibble(char nibble, char nibble_type);
char Send_Byte(char byte, char byte_type);
void Clear_LCD(void);
void Config_LCD(void);
void Send_String(char *str);
void jiffies_delay(unsigned int n);

#define EN 0
#define RS 1
#define D4 2
#define D5 3
#define D6 4
#define D7 5
#define LCD_LINE_LEN 16

static struct gpio lcd_pins[] = {
		{  4, GPIOF_OUT_INIT_LOW, "EN" },
		{ 17, GPIOF_OUT_INIT_LOW, "RS" },
		{ 22, GPIOF_OUT_INIT_LOW, "D4" },
		{ 23, GPIOF_OUT_INIT_LOW, "D5" },
		{ 24, GPIOF_OUT_INIT_LOW, "D6" },
		{ 25, GPIOF_OUT_INIT_LOW, "D7" },
};

static struct class*  lcd_device_driver_Class  = NULL; ///< The device-driver class struct pointer
static char lcd_text[LCD_LINE_LEN];
unsigned short first_load = 1;
module_param(first_load, short, 0644);
MODULE_PARM_DESC(first_load, "Optional, indicates if it is the first load of the lcd, 0 - not 1 - yes, defaults to 1");

my_device data_device = {
	.Device_Open = 0,
	.Device_Counter = 0,
	.fops = {
		.read = data_device_read,
		.write = data_device_write,
		.open = data_device_open,
		.release = data_device_release
	},
	.cdev_data = NULL,
	.driver_device = NULL,
	.dev_number = 0
};

my_device config_device = {
	.Device_Open = 0,
	.Device_Counter = 0,
	.fops = {
		.read = config_device_read,
		.write = config_device_write,
		.open = config_device_open,
		.release = config_device_release
	},
	.cdev_data = NULL,
	.driver_device = NULL,
	.dev_number = 0
};

#define CLEAN_ALL                            0
#define CLEAN_REGION_CDEV_CLASS_DATA_CONFIG  1
#define CLEAN_REGION_CDEV_CLASS_DATA         2
#define CLEAN_REGION_CDEV_CLASS              3
#define CLEAN_REGION_CDEV                    4
#define CLEAN_REGION                         5
void module_clean_level(unsigned int level)
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
	if(level<2) { // Remove o dispositivo
		device_destroy(lcd_device_driver_Class, config_device.dev_number);
	}
	if(level<3) { // Remove o dispositivo
		device_destroy(lcd_device_driver_Class, data_device.dev_number);
	}
	if(level<4) // Desfaz o registro da classe de dispositivo e remove a classe de dispositivo
	{
		class_unregister(lcd_device_driver_Class);
		class_destroy(lcd_device_driver_Class);
	}
	if(level<5) // Desfaz o registro da classe de dispositivo e remove a classe de dispositivo
	{
		cdev_del(data_device.cdev_data);
	}
	if(level<6) // Desfaz o registro do major number
		unregister_chrdev_region(data_device.dev_number, 1);
}

int init_module(void)
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
		module_clean_level(CLEAN_REGION); 
		MSG_BAD("Error while allocating data-cdev", 0L);
		return -1;
	}

	config_device.cdev_data = cdev_alloc();
	if(!config_device.cdev_data) {
		module_clean_level(CLEAN_REGION); 
		MSG_BAD("Error while allocating config-cdev", 0L);
		return -1;
	}

	cdev_init(data_device.cdev_data, &data_device.fops);
	cdev_init(config_device.cdev_data, &config_device.fops);

	err = cdev_add(data_device.cdev_data, data_device.dev_number, 1);
	if (err < 0)
	{
		module_clean_level(CLEAN_REGION);
		MSG_BAD("Error while adding cdev", (long int)err);
		return err;
	}

	err = cdev_add(config_device.cdev_data, config_device.dev_number, 1);
	if (err < 0)
	{
		module_clean_level(CLEAN_REGION);
		MSG_BAD("Error while adding cdev", (long int)err);
		return err;
	}

	// Registrar a classe do dispositivo
	lcd_device_driver_Class = class_create(THIS_MODULE, CLASS_NAME);
	if(IS_ERR(lcd_device_driver_Class)) // Se houve erro no registro
	{
		module_clean_level(CLEAN_REGION_CDEV);
		unregister_chrdev_region(data_device.dev_number, 1);
		MSG_BAD("falhou em registrar a classe do dispositivo", PTR_ERR(lcd_device_driver_Class));
		return PTR_ERR(lcd_device_driver_Class); // Correct way to return an error on a pointer
	}
	MSG_OK("classe do dispositivo registrada corretamente");

	// Registrar o device driver
	data_device.driver_device = device_create(lcd_device_driver_Class, NULL, data_device.dev_number, NULL, DEVICE_NAME "!data");
	if(IS_ERR(data_device.driver_device)) // Se houve erro no registro
	{
		module_clean_level(CLEAN_REGION_CDEV_CLASS);
		MSG_BAD("falhou em criar o device driver", PTR_ERR(data_device.driver_device));
		return PTR_ERR(data_device.driver_device);
	}
	MSG_OK("dispositivo criado corretamente");

	config_device.driver_device = device_create(lcd_device_driver_Class, NULL, config_device.dev_number, NULL, DEVICE_NAME "!config");
	if(IS_ERR(config_device.driver_device)) // Se houve erro no registro
	{
		module_clean_level(CLEAN_REGION_CDEV_CLASS_DATA);
		MSG_BAD("falhou em criar o device driver", PTR_ERR(config_device.driver_device));
		return PTR_ERR(config_device.driver_device);
	}
	MSG_OK("dispositivo criado corretamente");

	// Registrar GPIOs para LEDs, ligar LEDs
	ret = gpio_request_array(lcd_pins, ARRAY_SIZE(lcd_pins));
	if(ret) {
		module_clean_level(CLEAN_REGION_CDEV_CLASS_DATA_CONFIG);
		MSG_BAD("não conseguiu acesso ao GPIO", (long int)ret);
	}
	else
		MSG_OK("modulo carregado");

	for(i=0; i<LCD_LINE_LEN; i++)
		lcd_text[i] = '\0';
	lcd_text[0] = 'O';
	lcd_text[1] = 'l';
	lcd_text[2] = 'a';
	lcd_text[3] = ' ';
	lcd_text[4] = 'L';
	lcd_text[5] = 'C';
	lcd_text[6] = 'D';
	Config_LCD();
	Send_String(lcd_text);

	return ret;
}

void cleanup_module(void)
{
	Clear_LCD();
	module_clean_level(CLEAN_ALL);
	MSG_OK("modulo descarregado");
}

static int device_open(my_device dev) {
	if(dev.Device_Open) return -EBUSY;
	dev.Device_Open++; // Trava acesso ao device
	dev.Device_Counter = 0; // Conta quantos caracteres foram lidos
	try_module_get(THIS_MODULE); // Incrementa o contador de uso do modulo
	return 0;
}

static int data_device_open(struct inode *inode, struct file *file)
{
	return device_open(data_device);
}

static int config_device_open(struct inode *inode, struct file *file)
{
	return device_open(config_device);
}

static int device_release(my_device dev)
{
	dev.Device_Open--; // Libera acesso ao device
	module_put(THIS_MODULE); // Decrementa o contador de uso do modulo
	return 0;
}

static int data_device_release(struct inode *inode, struct file *file)
{
	return device_release(data_device);
}

static int config_device_release(struct inode *inode, struct file *file)
{
	return device_release(config_device);
}

static ssize_t data_device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	int i;
	// End of file
	if(data_device.Device_Counter>=LCD_LINE_LEN)
		return 0;
	for(i=0; (i<length)&&(data_device.Device_Counter<LCD_LINE_LEN)&&(lcd_text[data_device.Device_Counter]!='\0'); i++, data_device.Device_Counter++)
		put_user(lcd_text[data_device.Device_Counter], buffer+i);
	return i;
}

static ssize_t config_device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	// End of file
	if(config_device.Device_Counter>=0)
		return 0;
	put_user('1', buffer);
	config_device.Device_Counter++;
	return 1;
}

static ssize_t data_device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	int i;
	char local_buff[LCD_LINE_LEN+1]={'\0'};
	Clear_LCD();
	data_device.Device_Counter = 0;
	for(i=0; (i<len)&&(data_device.Device_Counter<LCD_LINE_LEN); i++, data_device.Device_Counter++)
		local_buff[i] = lcd_text[data_device.Device_Counter] = buff[i];
	local_buff[i] = lcd_text[data_device.Device_Counter] = '\0';
	Send_String(local_buff);
	return i;
}

char select_configuration(char indicador) {
	switch(indicador) {
		case COMANDO_1_LINE_CHAR:
			return COMANDO_1_LINE;
		case COMANDO_2_LINE_CHAR:
			return COMANDO_2_LINE;
		case COMANDO_CURSOR_VISIVEL_CHAR:
			return COMANDO_CURSOR_VISIVEL;
		case COMANDO_CURSOR_NAO_VISIVEL_CHAR:
			return COMANDO_CURSOR_NAO_VISIVEL;
		case COMANDO_LIMPAR_DISPLAY_CHAR:
			return COMANDO_LIMPAR_DISPLAY;
		case COMANDO_RETORNAR_CURSOR_CHAR:
			return COMANDO_RETORNAR_CURSOR;
		case COMANDO_CURSOR_SEGUNDO_LINHA_CHAR:
			return COMANDO_CURSOR_SEGUNDO_LINHA;
		default:
			return COMANDO_INVALIDO;
	}
}

static ssize_t config_device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	char comando = select_configuration(buff[0]);
	Send_Byte(comando, MODO_COMANDO);
	return 1;
}

char Send_Nibble(char nibble, char nibble_type)
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
	jiffies_delay(1);
	return 0;
}

char Send_Byte(char byte, char byte_type)
{
	if(Send_Nibble(byte>>4, byte_type)==-1)
		return -1;
	Send_Nibble(byte & 0xF, byte_type);
	return 0;
}

void Clear_LCD(void)
{
	Send_Byte(COMANDO_LIMPAR_DISPLAY, MODO_COMANDO);
	jiffies_delay(2);
	Send_Byte(COMANDO_RETORNAR_CURSOR, MODO_COMANDO);
	jiffies_delay(2);
}

void Config_LCD(void)
{
	jiffies_delay(1);
	printk(KERN_ALERT "first load: %hi\n", first_load);
	if (first_load)
		Send_Nibble(COMANDO_NIBBLE_INICIAL_MODO_4_BITS, MODO_COMANDO);
	else
		Send_Byte(COMANDO_1_LINE, MODO_COMANDO);
	Send_Byte(COMANDO_CURSOR_VISIVEL, MODO_COMANDO);
	Clear_LCD();
}

void Send_String(char *str)
{
	int i = 0;
	while(str[i]!='\0')
	{
		Send_Byte(str[i], MODO_DADO);
		i++;
	}
}

void jiffies_delay(unsigned int n)
{
	unsigned long t_end = jiffies + n*HZ/100;
	while(jiffies<t_end);
}
