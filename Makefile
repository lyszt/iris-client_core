CC = gcc
CFLAGS = -Iinclude -Wall -g
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = build/program

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p build
	$(CC) $(OBJ) -o $(TARGET)

clean:
	rm -f src/*.o $(TARGET)