CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -lreadline -lncurses

BUILD_DIR = build
SRC_DIR = .
DESTDIR ?= /usr/local

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
TARGET = $(BUILD_DIR)/archium

.PHONY: all clean install uninstall test debug

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)
	@echo "Build complete! Binary is at: $(TARGET)"

install: $(TARGET)
	install -D $(TARGET) $(DESTDIR)/bin/archium
	@echo "Archium installed to $(DESTDIR)/bin/archium"

uninstall:
	rm -f $(DESTDIR)/bin/archium
	@echo "Archium uninstalled from $(DESTDIR)/bin/archium"

clean:
	rm -rf $(BUILD_DIR)
	@echo "Cleaned build directory"

debug:
	@echo "Source files: $(SRC)"
	@echo "Object files: $(OBJ)"
	@echo "Target: $(TARGET)"
	@echo "Build directory: $(BUILD_DIR)"
	@echo "Install directory: $(DESTDIR)/bin"

release: clean $(BUILD_DIR) $(TARGET)
	strip $(TARGET)
	@echo "Release binary built: $(TARGET)"

format:
	clang-format -i $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*.h)
	@echo "Codebase formatted with clang-format"

# TODO: Add tests
test: $(TARGET)
	@echo "Not implemented yet. Please run the binary manually for testing."