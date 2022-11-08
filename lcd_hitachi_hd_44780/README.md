# Device driver lcd

Device driver para integração raspberry pi 3b com módulos lcd com driver hitashi hd44780.

## Estrura de comunicação

São criado dois arquivos no diretório `/dev/lcd_device_driver`. O primeiro é responsável por escrever dados `/dev/lcd_device_driver/data`, que também pode ser lido para receber o valor atual. O segundo é `/dev/lcd_device_driver/config` que é utilizado para realizar configurações no display.

## Créditos

Versão inicial criada e uso autorizado por [Diogo Caetano Garcia](https://github.com/DiogoCaetanoGarcia/Sistemas_Embarcados/tree/c04a3e19722d61e9c35284f77ed8be101d53e990/5_T%C3%B3picos_avan%C3%A7ados/5.6_Aplica%C3%A7%C3%B5es/2_LCD_device_driver)
