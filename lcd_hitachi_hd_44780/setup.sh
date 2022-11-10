#!/bin/bash
make
sudo insmod lcd_device_driver.ko
dmesg | tail -1
sudo chmod 777 /dev/lcd_device_driver/data 
sudo chmod 777 /dev/lcd_device_driver/config 
