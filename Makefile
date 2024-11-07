CC = cc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lreadline -lncurses
TARGET = archium
SRC = main.c commands.c utils.c autocomplete.c package_manager.c display.c
INSTALL_DIR = /usr/bin
BUILD_DIR = build
OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(BUILD_DIR)/$(TARGET) $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c archium.h
	$(CC) $(CFLAGS) -c $< -o $@

install: $(BUILD_DIR)/$(TARGET)
	install -m 755 $(BUILD_DIR)/$(TARGET) $(INSTALL_DIR)

uninstall:
	rm -f $(INSTALL_DIR)/$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all install uninstall clean $(BUILD_DIR)
