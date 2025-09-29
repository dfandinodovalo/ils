CC=gcc
CFLAGS=-Wall -Wextra -I/usr/include/ncursesw
LDFLAGS=-lncursesw

SOURCES=main.c screen_manager.c file_utils.c ui_renderer.c
OBJECTS=$(SOURCES:.c=.o)
BUILD_DIR=build
EXECUTABLE=ils

# Los objetos ahora estarán en build/
OBJECTS_BUILD=$(addprefix $(BUILD_DIR)/,$(OBJECTS))

all: $(BUILD_DIR) $(EXECUTABLE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXECUTABLE): $(OBJECTS_BUILD)
	$(CC) $(OBJECTS_BUILD) -o $@ $(LDFLAGS)

# Regla para compilar .c en /build/*.o
$(BUILD_DIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(BUILD_DIR)/*.o $(EXECUTABLE)