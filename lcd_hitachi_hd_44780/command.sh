#!/bin/bash

help()
{
    echo Usage:
    echo "./command.sh [operation] [value]"
    echo "operation -> "
    echo "comando: configuration operation writes in /dev/lcd_device_driver/config"
    echo "escrita: write operation writes in /dev/lcd_device_driver/data"
    echo "ler_dado: reads last written value from /dev/lcd_device_driver/data"
    echo "ler_config: reads current configuration from /dev/lcd_device_driver/config"
}

verify_second_argument()
{
    if [ $1 -lt 2 ]; then
        echo "Must pass second argument. Usage: pass 'help' as argument"
        false
        return $?
    fi
    true
}


case $1 in
    "help")
        help
        exit;;
    "comando")
        if verify_second_argument $#; then
            sudo echo -n $2 > /dev/lcd_device_driver/config
        fi
        exit;;
    "escrita")
        if verify_second_argument $#; then
            sudo echo -n $2 > /dev/lcd_device_driver/data
        fi
        exit;;
    "ler_dado")
        sudo cat /dev/lcd_device_driver/data
        exit;;
    "ler_config")
        sudo cat /dev/lcd_device_driver/config
        exit;;
esac
