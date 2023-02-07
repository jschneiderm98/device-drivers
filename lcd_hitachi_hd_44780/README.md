# Device driver lcd

Device driver para integração raspberry pi 3b com módulos lcd que possuem driver hitashi hd44780.

## Parâmetros do device driver

Durante a intalação do módulo do kernel, é possível passar argumentos de linha de comando para configurar as portas gpio que serão utilizadas. As portas configuráveis são `EN`, `RS`, `D4`, `D5`, `D6`, `D7`. A seguir um exemplo de uso.

```bash
sudo insmod lcd_device_driver.ko en=16 d4=26
```

Os valores padrão de cada gpio são indicados abaixo.
<center>

| Porta LCD | Pino GPIO |
| - | - |
| EN | 4 |
| RS | 17 |
| D4 | 22 |
| D5 | 23 |
| D6 | 24 |
| D7 | 25 |

</center>

## Estrura de comunicação

São criado dois arquivos no diretório `/dev/lcd_device_driver`. O primeiro é responsável por escrever dados `/dev/lcd_device_driver/data`, que também pode ser lido para receber o valor atual. O segundo é `/dev/lcd_device_driver/config` que é utilizado para realizar configurações no display, é possível também ler esse arquivo para obter as informações atuais do display.

### Comandos de configuração

Ao escrever no arquivo `/dev/lcd_device_driver/config` o device driver utilizará apenas o primeiro carácter, a seguir é mostrada uma tabela com operações suportadas e o carácter que deve ser escrito.

| Comando | Carácter | Descrição |
| - | - | - |
| Modo uma linha | '1' | Modo que apenas a linha de cima do display é utilizada |Q
| Modo duas linhas | '2' | Modo que ambas as linhas do display são utilizadas |
| Cursor vísivel | '3' | Torna a posição atual do cursor vísivel, piscando no display |
| Cursor não vísivel | '4' | Desabilita o cursor vísivel |
| Limpar display | '5' | Remove qualquer carácter escrito no display |
| Retornar cursor | '6' | Retorna o cursor para a primeira posição do display |
| Ir para segunda linha | '7' | Move o cursor para a primeira posição da segunda linha do display |
| Limpar primeira linha | '8' | Limpa a primeira linha (0 - 39) e posiciona o cursor no começo da primeira linha |
| Limpar segunda linha | '9' | Limpa a segunda linha (40 - 79) e posiciona o cursor no começo da segunda linha |

### Helper

Foi criado um script shell chamado `lcd` para automatizar tarefas de build e facilitar a escrita nos arquivos do device driver. Para instruções de uso basta executar

```
./lcd help
```

## Créditos

Versão inicial criada e uso autorizado por [Diogo Caetano Garcia](https://github.com/DiogoCaetanoGarcia/Sistemas_Embarcados/tree/c04a3e19722d61e9c35284f77ed8be101d53e990/5_T%C3%B3picos_avan%C3%A7ados/5.6_Aplica%C3%A7%C3%B5es/2_LCD_device_driver)
