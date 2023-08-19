CC = gcc
LIBS = -lncurses
TARGET = ils

$(TARGET): ils.c
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

all: $(TARGET)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run