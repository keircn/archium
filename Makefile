CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -lreadline

SRCS = main.c commands.c package_manager.c utils.c autocomplete.c display.c error_handling.c
OBJS = $(SRCS:.c=.o)
TARGET = archium

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(TARGET)
	@mkdir -p $(DESTDIR)/usr/local/bin
	@mkdir -p $(DESTDIR)/var/log
	@install -m 755 $(TARGET) $(DESTDIR)/usr/local/bin/
	@touch $(DESTDIR)/var/log/archium.log
	@touch $(DESTDIR)/var/log/archium_error.log
	@chmod 644 $(DESTDIR)/var/log/archium.log
	@chmod 644 $(DESTDIR)/var/log/archium_error.log
	@echo "Installation complete!"

uninstall:
	@rm -f $(DESTDIR)/usr/local/bin/$(TARGET)
	@rm -f $(DESTDIR)/var/log/archium.log
	@rm -f $(DESTDIR)/var/log/archium_error.log
	@echo "Uninstallation complete!"

clean:
	rm -f $(OBJS) $(TARGET)
