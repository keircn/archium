CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -lreadline -lncurses

BUILD_DIR = build
SRC = main.c commands.c error_handling.c utils.c package_manager.c display.c autocomplete.c
OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)
TARGET = $(BUILD_DIR)/archium

.PHONY: all clean install uninstall

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)
	@echo "Build complete! Binary is at: $(TARGET)"

install: $(TARGET)
	install -D $(TARGET) $(DESTDIR)/usr/local/bin/archium

uninstall:
	rm -f $(DESTDIR)/usr/local/bin/archium

clean:
	rm -rf $(BUILD_DIR)

# Debug target to show variables
debug:
	@echo "Source files: $(SRC)"
	@echo "Object files: $(OBJ)"
	@echo "Target: $(TARGET)"
