CC = gcc

main.out: main.o rc522_controller.o spi_interface.o
	gcc main.o rc522_controller.o spi_interface.o -o main.out -lwiringPi
main.o: main.c
	gcc -c main.c
rc522_controller.o: rc522_controller.c
	gcc -c rc522_controller.c
spi_interface.o: spi_interface.c
	gcc -c spi_interface.c

clean:
	rm -f *.out *.o *.bin *.txt *.html