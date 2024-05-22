# Device Drivers

Esse projeto consiste no desenvolvimento de device-drivers para Raspberry Pi por meio de LKMs(Loadable Kernel Modules), para interação com módulos externos à Raspberry Pi.

## Requisitos

```bash
sudo apt install raspberrypi-kernel-headers libncurses5-dev bc bison flex libssl-dev python2 rpi-update
sudo rpi-update
sudo reboot
```

```bash
sudo wget https://raw.githubusercontent.com/RPi-Distro/rpi-source/master/rpi-source -O /usr/local/bin/rpi-source
sudo chmod +x /usr/local/bin/rpi-source
/usr/local/bin/rpi-source -q --tag-update
rpi-source
sudo ln -s /usr/src/linux-headers-$(uname -r)/ /lib/modules/$(uname -r)/build
```

## Carregando os módulos

* Primeiro realize o build do módulo

    ```bash
    make
    ```

* Então instale o módulo

    ```bash
    sudo insmod modulo.ko
    ```

* Para realizar comandos é utilizado
 o arquivo no diretório `/dev`. Exs.:
  * Leitura

    ```bash
    sudo cat /dev/modulo
    ```
  * Escrita

    Para realizar a escrita é necessário alterar a permissão do arquivo

    ```bash
    sudo chmod 777 /dev/modulo
    ```

    E então

    ```bash
    sudo echo valor > /dev/modulo
    ```
* Por fim resta remover o módulo e limpar os arquivos gerados durante o build

    ```bash
    sudo rmmod modulo
    ```

    ```bash
    make clean
    ```

## TCC

O desenvolvimento desse projeto será realizado como um TCC(Trabalho de conclusão de curso).

### Aluno: [Júlio César Schneider Martins](https://github.com/jschneiderm98)
### Orientador: [Diogo Caetano Garcia](https://github.com/DiogoCaetanoGarcia)

## Cronograma TCC1

| Semana | Data inicial | Tarefa |
| - | - | - |
| 01 | 28/11/2022 | Estudar outros módulos para o desenvolvimento de device drivers. |
| 02 | 05/12/2022 | Planejar desenvolvimento dos device drivers do(s) módulo(s) escolhido(s)  |
| 03 | 12/12/2022 | Iniciar escrita do TCC1, escrever introdução |
| 04 | 19/12/2022 | Realizar revisão bibliográfica |
| 05 | 26/12/2022 | Natal |
| 06 | 02/01/2023 | Realizar revisão bibliográfica |
| 07 | 09/01/2023 | Escrever Proposta de tema |
| 08 | 16/01/2023 | Escrever resultados e discussão |
| 09 | 23/01/2023 | Escrever Passos futuros e cronograma |
| 10 | 30/01/2023 | Revisão do texto |
| 11 | 06/02/2023 | Revisão do texto |
| 12 | 13/02/2023 | Revisão do texto |
| 13 | 20/02/2023 | Defesa na banca de TCC1 |

## Cronograma TCC2
| Semana | Data inicial | Tarefa | Concluido |
| - | - | - | - |
| 01.0 | 18/03/2024 | Revisão do contéudo estudado no TCC1 | <center>:white_check_mark:</center> |
| 02.1 | 25/03/2024 | Estudo sobre funcionamento do módulo RFID-RC522 | <center>:white_check_mark:</center> |
| 02.2 | 28/03/2024 | Estudo sobre o uso de comunicação serial em módulos do kernel | <center>:white_check_mark:</center> |
| 03 | 01/04/2024 | Montagem do circuto e testes iniciais utilizando linguagem de alto nível (Python) com o módulo RFID-RC522 | <center>:white_check_mark:</center> |
| 04 | 08/04/2024 | Desenvolvimento de um driver no espaço de usuário utilizando a linguagem C para o módulo RFID-RC522 | <center>:white_check_mark:</center> |
| 05 | 15/04/2024 | Desenvolvimento de um driver no espaço de usuário utilizando a linguagem C para o módulo RFID-RC522 | <center>:white_check_mark:</center> |
| 06 | 22/04/2024 | Desenvolvimento de um driver no espaço de usuário utilizando a linguagem C para o módulo RFID-RC522 | <center>:white_check_mark:</center> |
| 07 | 29/04/2024 | Desenvolvimento do device-driver no kernel para o módulo RFID-RC522, interface SPI básica | <center>:white_check_mark:</center> |
| 08 | 06/05/2024 | Desenvolvimento do device-driver no kernel para o módulo RFID-RC522, interface SPI básica | <center>:white_check_mark:</center> |
| 09 | 13/05/2024 | Desenvolvimento do device-driver no kernel para o módulo RFID-RC522, leitura e escrita de registradores | <center>:white_check_mark:</center> |
| 10 | 20/05/2024 | Desenvolvimento do device-driver no kernel para o módulo RFID-RC522, Self test RC522 | <center>:white_check_mark:</center> |
| 11 | 27/05/2024 | Desenvolvimento do device-driver no kernel para o módulo RFID-RC522, leitura de dispositivos RFID | <center>:black_square_button:</center> |
| 12 | 03/06/2024 | Desenvolvimento do device-driver no kernel para o módulo RFID-RC522, escrita de dispositivos RFID | <center>:black_square_button:</center> |
| 13 | 10/06/2024 | Continuação da escrita dos Resultados e Discussões |<center>:black_square_button:</center>  |
| 14 | 17/06/2024 | Continuação da escrita dos Materiais e métodos e Desenvolvimento dos Drivers | <center>:black_square_button:</center> |
| 15 | 24/07/2024 | Continuação da escrita dos Introdução | <center>:black_square_button:</center> |
| 16 | 01/07/2024 | Continuação da escrita da Conclusão e do Resumo | <center>:black_square_button:</center> |
| 17 | 08/07/2024 | Revisão textual e Defesa na banca de TCC2 | <center>:black_square_button:</center> |
