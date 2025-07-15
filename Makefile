CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -lreadline -ldl -ldl -lpthread

BUILD_DIR = build
SRC_DIR = src
DESTDIR = /usr/local
VERSION = $(shell cat VERSION)
TARNAME = archium-$(VERSION)

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
TARGET = $(BUILD_DIR)/archium
VERSION_HEADER = $(SRC_DIR)/version.h

.PHONY: all clean install uninstall install-completions test debug release format version-header

all: $(BUILD_DIR) version-header $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

version-header:
	@echo "#ifndef VERSION_H" > $(VERSION_HEADER)
	@echo "#define VERSION_H" >> $(VERSION_HEADER)
	@echo "#define ARCHIUM_VERSION \"$(VERSION)\"" >> $(VERSION_HEADER)
	@echo "#endif" >> $(VERSION_HEADER)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)
	@echo "Build complete at $(TARGET)"

install: $(TARGET)
	install -D $(TARGET) $(DESTDIR)/bin/archium
	@echo "Archium installed to $(DESTDIR)/bin/archium"

install-completions:
	./install-completions.sh --all

uninstall:
	rm -f $(DESTDIR)/bin/archium
	@echo "Archium uninstalled from $(DESTDIR)/bin/archium"

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(VERSION_HEADER)

debug:
	@echo "Source files: $(SRC)"
	@echo "Object files: $(OBJ)"
	@echo "Target: $(TARGET)"
	@echo "Build directory: $(BUILD_DIR)"
	@echo "Install directory: $(DESTDIR)/bin"

release: clean all
	strip $(TARGET)
	mkdir -p $(BUILD_DIR)/release
	cp $(TARGET) $(BUILD_DIR)/release/archium
	tar -czvf $(BUILD_DIR)/$(TARNAME).tar.gz -C $(BUILD_DIR) release

format:
	clang-format -i $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*.h)

test: $(TARGET)
	@echo "Not implemented yet."
