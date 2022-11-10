#!/bin/bash

sudo rmmod lcd_device_driver
make clean
dmesg | tail -1