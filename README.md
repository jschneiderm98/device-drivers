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

