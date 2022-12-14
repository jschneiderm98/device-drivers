# Device Drivers

Esse projeto consiste no desenvolvimento de device-drivers para Raspberry Pi por meio de LKMs(Loadable Kernel Modules), para interação com módulos externos à Raspberry Pi.

## Requisitos

```bash
sudo apt install raspberrypi-kernel-headers libncurses5-dev bc bison flex libssl-dev python2 rpi-update
sudo rpi-update
sudo reboot
```

```bash
sudo wget https://raw.githubusercontent.com/notro/rpi-source/master/rpi-source -O /usr/bin/rpi-source
sudo chmod +x /usr/bin/rpi-source
python2 /usr/bin/rpi-source -q --tag-update
python2 /usr/bin/rpi-source
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
| 03 | 12/12/2022 | Iniciar escrita do TCC, escrever introdução |
| 04 | 19/12/2022 | Realizar revisão bibliográfica |
| 05 | 26/12/2022 | Natal |
| 06 | 02/01/2023 | Realizar revisão bibliográfica |
| 07 | 09/01/2023 | Escrever Proposta de tema |
| 08 | 16/01/2023 | Escrever resultados e discussão |
| 09 | 23/01/2023 | Escrever Passos futuros e cronograma |
| 10 | 30/01/2023 | Revisão do texto |
| 11 | 06/02/2023 | Revisão do texto |
| 12 | 13/02/2023 | Revisão do texto |
| 13 | 20/02/2023 | Defesa na banca de TCC |
