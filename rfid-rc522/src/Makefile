obj-m := rfid_rc522_device_driver.o 
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
msg:
	dmesg | tail -20
