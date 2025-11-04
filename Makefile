CC = gcc

CFLAGS = -Wall -Wextra -O3 -march=native -flto -DNDEBUG
LDFLAGS = -lreadline -ldl -ldl -lpthread -flto

RELEASE_CFLAGS = -Wall -Wextra -O2 -mtune=generic -flto -DNDEBUG -s
RELEASE_LDFLAGS = -lreadline -ldl -ldl -lpthread -flto -s

DEBUG_CFLAGS = -Wall -Wextra -O0 -g3 -DDEBUG -fsanitize=address,undefined -fno-omit-frame-pointer
DEBUG_LDFLAGS = -lreadline -ldl -ldl -lpthread -fsanitize=address,undefined

ANALYSIS_FLAGS = -Wall -Wextra -Wformat=2 -Wshadow -Wstrict-prototypes -Wmissing-prototypes -fanalyzer

BUILD_DIR = build
SRC_DIR = src
DESTDIR = /usr/local
VERSION = $(shell cat .VERSION)
TARNAME = archium-$(VERSION)

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
TARGET = $(BUILD_DIR)/archium
VERSION_HEADER = $(SRC_DIR)/include/version.h

.PHONY: all clean install uninstall install-completions test debug release release-static format version-header check analyze profile benchmark

all: $(BUILD_DIR) version-header $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

version-header:
	@echo "#ifndef VERSION_H" > $(VERSION_HEADER)
	@echo "#define VERSION_H" >> $(VERSION_HEADER)
	@echo "#define ARCHIUM_VERSION \"$(VERSION)\"" >> $(VERSION_HEADER)
	@echo "#endif" >> $(VERSION_HEADER)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(SRC_DIR)/include -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

install: $(TARGET)
	install -D $(TARGET) $(DESTDIR)/bin/archium

install-completions:
	./install-completions.sh --all

uninstall:
	rm -f $(DESTDIR)/bin/archium

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(VERSION_HEADER)

debug: clean
	@mkdir -p $(BUILD_DIR)
	@$(MAKE) version-header
	$(CC) $(DEBUG_CFLAGS) -I$(SRC_DIR)/include -c $(SRC_DIR)/main.c -o $(BUILD_DIR)/main.o
	$(CC) $(DEBUG_CFLAGS) -I$(SRC_DIR)/include -c $(SRC_DIR)/autocomplete.c -o $(BUILD_DIR)/autocomplete.o
	$(CC) $(DEBUG_CFLAGS) -I$(SRC_DIR)/include -c $(SRC_DIR)/commands.c -o $(BUILD_DIR)/commands.o
	$(CC) $(DEBUG_CFLAGS) -I$(SRC_DIR)/include -c $(SRC_DIR)/config.c -o $(BUILD_DIR)/config.o
	$(CC) $(DEBUG_CFLAGS) -I$(SRC_DIR)/include -c $(SRC_DIR)/display.c -o $(BUILD_DIR)/display.o
	$(CC) $(DEBUG_CFLAGS) -I$(SRC_DIR)/include -c $(SRC_DIR)/error_handling.c -o $(BUILD_DIR)/error_handling.o
	$(CC) $(DEBUG_CFLAGS) -I$(SRC_DIR)/include -c $(SRC_DIR)/package_manager.c -o $(BUILD_DIR)/package_manager.o
	$(CC) $(DEBUG_CFLAGS) -I$(SRC_DIR)/include -c $(SRC_DIR)/plugin.c -o $(BUILD_DIR)/plugin.o
	$(CC) $(DEBUG_CFLAGS) -I$(SRC_DIR)/include -c $(SRC_DIR)/utils.c -o $(BUILD_DIR)/utils.o
	$(CC) $(OBJ) -o $(TARGET) $(DEBUG_LDFLAGS)
	@echo "$(TARGET)"

info:
	@echo "Source files: $(SRC)"
	@echo "Object files: $(OBJ)"
	@echo "Target: $(TARGET)"
	@echo "Build directory: $(BUILD_DIR)"
	@echo "Install directory: $(DESTDIR)/bin"
	@echo "Current CFLAGS: $(CFLAGS)"
	@echo "Current LDFLAGS: $(LDFLAGS)"

release: clean
	@mkdir -p $(BUILD_DIR)
	@$(MAKE) version-header
	$(CC) $(RELEASE_CFLAGS) -I$(SRC_DIR)/include -c $(SRC_DIR)/main.c -o $(BUILD_DIR)/main.o
	$(CC) $(RELEASE_CFLAGS) -I$(SRC_DIR)/include -c $(SRC_DIR)/autocomplete.c -o $(BUILD_DIR)/autocomplete.o
	$(CC) $(RELEASE_CFLAGS) -I$(SRC_DIR)/include -c $(SRC_DIR)/commands.c -o $(BUILD_DIR)/commands.o
	$(CC) $(RELEASE_CFLAGS) -I$(SRC_DIR)/include -c $(SRC_DIR)/config.c -o $(BUILD_DIR)/config.o
	$(CC) $(RELEASE_CFLAGS) -I$(SRC_DIR)/include -c $(SRC_DIR)/display.c -o $(BUILD_DIR)/display.o
	$(CC) $(RELEASE_CFLAGS) -I$(SRC_DIR)/include -c $(SRC_DIR)/error_handling.c -o $(BUILD_DIR)/error_handling.o
	$(CC) $(RELEASE_CFLAGS) -I$(SRC_DIR)/include -c $(SRC_DIR)/package_manager.c -o $(BUILD_DIR)/package_manager.o
	$(CC) $(RELEASE_CFLAGS) -I$(SRC_DIR)/include -c $(SRC_DIR)/plugin.c -o $(BUILD_DIR)/plugin.o
	$(CC) $(RELEASE_CFLAGS) -I$(SRC_DIR)/include -c $(SRC_DIR)/utils.c -o $(BUILD_DIR)/utils.o
	$(CC) $(OBJ) -o $(TARGET) $(RELEASE_LDFLAGS)
	mkdir -p $(BUILD_DIR)/release
	cp $(TARGET) $(BUILD_DIR)/release/archium
	tar -czvf $(BUILD_DIR)/$(TARNAME).tar.gz -C $(BUILD_DIR) release
	@echo "Release built: $(BUILD_DIR)/$(TARNAME).tar.gz"

format:
	clang-format -i $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/include/*.h)

test: $(TARGET)
	@test -x $(TARGET) # I'll add tests eventually... probably

check: version-header
	@mkdir -p $(BUILD_DIR)/analysis
	$(CC) $(ANALYSIS_FLAGS) -I$(SRC_DIR)/include -fsyntax-only $(wildcard $(SRC_DIR)/*.c) 2> $(BUILD_DIR)/analysis/check.log || true
	@if [ -s $(BUILD_DIR)/analysis/check.log ]; then \
		echo "Found issues:"; \
		cat $(BUILD_DIR)/analysis/check.log; \
	else \
		echo "No issues found."; \
	fi

analyze: check
	@if command -v clang-tidy >/dev/null 2>&1; then \
		clang-tidy $(wildcard $(SRC_DIR)/*.c) -- $(CFLAGS) -I$(SRC_DIR)/include 2>/dev/null | head -20; \
	fi
