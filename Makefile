HEADERS=rpi_ws281x/clk.h rpi_ws281x/dma.h rpi_ws281x/gpio.h rpi_ws281x/pwm.h rpi_ws281x/ws2811.h rpi_ws281x/mailbox.h rpi_ws281x/rpihw.h
OBJ=main.o rpi_ws281x/dma.o rpi_ws281x/pwm.o rpi_ws281x/ws2811.o rpi_ws281x/mailbox.o rpi_ws281x/rpihw.o
TARGET=/usr/local/bin/rpilights
CFLAGS=-O -w -fstack-protector -lm

all: $(TARGET)

$(TARGET): $(OBJ)
	sudo cc -o $(TARGET) $(CFLAGS) $(OBJ)
	sudo chmod +s $(TARGET)
	sudo chmod 666 /dev/mem

clean:
	sudo rm -rf $(OBJ) $(TARGET)

main.o: main.c $(HEADERS)

dma.o: dma.c $(HEADERS)

pwm.o: pwm.c $(HEADERS)

ws2811.o: ws2811.c $(HEADERS)

mailbox.o: mailbox.c $(HEADERS)

rpihw.o: rpihw.c $(HEADERS)
