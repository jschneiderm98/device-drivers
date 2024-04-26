CC = gcc -g

main.out: main.o rc522_controller.o spi_interface.o
	${CC} main.o rc522_controller.o spi_interface.o -o main.out -lwiringPi
main.o: main.c
	${CC} -c main.c
rc522_controller.o: rc522_controller.c
	${CC} -c rc522_controller.c
spi_interface.o: spi_interface.c
	${CC} -c spi_interface.c

clean:
	rm -f *.out *.o *.bin *.txt *.html