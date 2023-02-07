import os
from lcd_driver import lcd_config, lcd_data, ConfigOperation

def waitForResult(expected_result: str) -> None:
  os.system('clear')
  print(f'Resultado esperado: {expected_result}')
  print('Digite para continuar')
  input()

print('Iniciando testes')
lcd_config(ConfigOperation.LIMPAR_DISPLAY)
lcd_config(ConfigOperation.MODO_UMA_LINHA)
lcd_data('Teste')
lcd_config(ConfigOperation.MOVER_CURSOR_SEGUNDA_LINHA)
lcd_data('N deve aparecer')
waitForResult('a palavra "Teste" na primeira linha do display e nada na segunda')

lcd_config(ConfigOperation.LIMPAR_DISPLAY)
waitForResult('a linha deve ter sido limpa')

lcd_config(ConfigOperation.LIMPAR_DISPLAY)
lcd_config(ConfigOperation.MODO_DUAS_LINHAS)
lcd_data("Teste")
lcd_config(ConfigOperation.MOVER_CURSOR_SEGUNDA_LINHA)
lcd_data("Teste2")
waitForResult('a palavra "Teste" na primeira linha do display e "Teste2" na segunda')

lcd_config(ConfigOperation.LIMPAR_SEGUNDA_LINHA)
waitForResult('a segunda linha deve ter sido limpa')

lcd_config(ConfigOperation.MOVER_CURSOR_SEGUNDA_LINHA)
lcd_data("Teste2")
waitForResult('a segunda linha deve ter sido escrita novamente')

lcd_config(ConfigOperation.LIMPAR_PRIMEIRA_LINHA)
waitForResult('a primeira linha deve ter sido limpa')

lcd_config(ConfigOperation.LIMPAR_DISPLAY)
lcd_config(ConfigOperation.CURSOR_NAO_VISIVEL)
waitForResult('O cursor não deve ser visível')

lcd_config(ConfigOperation.LIMPAR_DISPLAY)
lcd_config(ConfigOperation.CURSOR_VISIVEL)
waitForResult('O cursor deve ser visível')

lcd_config(ConfigOperation.LIMPAR_DISPLAY)
