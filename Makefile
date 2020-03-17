# nanabozo Makefile

TARGET = nanabozo
CC = cc
CFLAGS = -g -Wall -O3
PREFIX = /usr/local
INPUTSIZE = 512

$(TARGET): nanabozo.c
	$(CC) $(CFLAGS) -DINPUTSIZE=$(INPUTSIZE) -o $@ nanabozo.c

$(PREFIX)/bin:
	mkdir -p $@

$(PREFIX)/bin/$(TARGET): $(PREFIX)/bin $(TARGET)
	cp -f $(TARGET) $(PREFIX)/bin

.PHONY: clean install uninstall

clean:
	rm -f $(TARGET)

install: $(PREFIX)/bin/$(TARGET)

uninstall:
	rm -f $(PREFIX)/bin/$(TARGET)

# vi: sw=4 ts=4 sts=4 noet ai
