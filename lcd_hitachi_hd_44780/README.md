# Device driver lcd

Device driver para integração raspberry pi 3b com módulos lcd com driver hitashi hd44780.

## Estrura de comunicação

São criado dois arquivos no diretório `/dev/lcd_device_driver`. O primeiro é responsável por escrever dados `/dev/lcd_device_driver/data`, que também pode ser lido para receber o valor atual. O segundo é `/dev/lcd_device_driver/config` que é utilizado para realizar configurações no display.