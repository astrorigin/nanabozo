# nanabozo Makefile

TARGET = nanabozo
CC = cc
CFLAGS = -g -Wall -O3
PREFIX = /usr/local
INPUTSIZE = 512

.DEFAULT_GOAL := all

$(TARGET): nanabozo.c
	$(CC) $(CFLAGS) -DINPUTSIZE=$(INPUTSIZE) -o $@ $<

$(PREFIX)/bin:
	mkdir -p $@

$(PREFIX)/bin/$(TARGET): $(PREFIX)/bin $(TARGET)
	cp -f $(TARGET) $<

$(PREFIX)/share/man/man1:
	mkdir -p $@

$(TARGET).1.gz: nanabozo.1
	gzip -c $< > $@

$(PREFIX)/share/man/man1/$(TARGET).1.gz: $(PREFIX)/share/man/man1 $(TARGET).1.gz
	cp -f $(TARGET).1.gz $<

.PHONY: all clean install uninstall

all: $(TARGET) $(TARGET).1.gz

clean:
	rm -f $(TARGET) $(TARGET).1.gz

install: $(PREFIX)/bin/$(TARGET) $(PREFIX)/share/man/man1/$(TARGET).1.gz

uninstall:
	rm -f $(PREFIX)/bin/$(TARGET)
	rm -f $(PREFIX)/share/man/man1/$(TARGET).1.gz

# vi: sw=4 ts=4 noet
