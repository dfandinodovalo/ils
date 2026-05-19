CC = gcc
CFLAGS = -Wall -Wextra -O2 -I/usr/include/ncursesw
LDFLAGS = -lncursesw

SOURCES = main.c screen_manager.c file_utils.c ui_renderer.c
BUILD_DIR = build
OBJECTS = $(addprefix $(BUILD_DIR)/, $(SOURCES:.c=.o))
EXECUTABLE = ils

.PHONY: all clean run

all: $(BUILD_DIR) $(EXECUTABLE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/main.o: screen_manager.h file_utils.h ui_renderer.h
$(BUILD_DIR)/file_utils.o: file_utils.h
$(BUILD_DIR)/screen_manager.o: screen_manager.h
$(BUILD_DIR)/ui_renderer.o: ui_renderer.h file_utils.h screen_manager.h

run: all
	./$(EXECUTABLE)

clean:
	rm -rf $(BUILD_DIR) $(EXECUTABLE)
