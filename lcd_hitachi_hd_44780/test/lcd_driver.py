from enum import Enum

LCD_DEVICE_DRIVER_CONFIG_PATH = '/dev/lcd_device_driver/config'
LCD_DEVICE_DRIVER_DATA_PATH = '/dev/lcd_device_driver/data'

class ConfigOperation(Enum):
  MODO_UMA_LINHA = '1'
  MODO_DUAS_LINHAS = '2'
  CURSOR_VISIVEL = '3'
  CURSOR_NAO_VISIVEL = '4'
  LIMPAR_DISPLAY = '5'
  MOVER_CURSOR_PRIMEIRA_LINHA = '6'
  MOVER_CURSOR_SEGUNDA_LINHA = '7'
  LIMPAR_PRIMEIRA_LINHA = '8'
  LIMPAR_SEGUNDA_LINHA = '9'

def lcd_config(operation: ConfigOperation):
  with open(LCD_DEVICE_DRIVER_CONFIG_PATH, 'w') as config_device:
    config_device.write(operation.value)

def lcd_data(data: str) -> None:
  with open(LCD_DEVICE_DRIVER_DATA_PATH, 'w') as data_device:
    data_device.write(data)