CC = gcc
CFLAGS = -Wall -Wextra -O3 `sdl2-config --cflags`
LDFLAGS = `sdl2-config --libs` -lSDL2_image
TARGET = metricpet
SRC = metricpet.c
BIN_DIR = /usr/local/bin

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	sudo mv $(TARGET) $(BIN_DIR)

.PHONY: all clean install

