# nanabozo Makefile

TARGET = nanabozo
CC = cc
OPTS = -Wall -O3
INSTALLDIR = $(HOME)/bin
INPUTSIZE = 512

$(TARGET): nanabozo.c
	$(CC) $(OPTS) -DINPUTSIZE=$(INPUTSIZE) -o $@ nanabozo.c

$(INSTALLDIR):
	mkdir -p $@

$(INSTALLDIR)/$(TARGET): $(INSTALLDIR) $(TARGET)
	cp -f $(TARGET) $(INSTALLDIR)

.PHONY: clean install uninstall

clean:
	rm -f $(TARGET)

install: $(INSTALLDIR)/$(TARGET)

uninstall:
	rm -f $(INSTALLDIR)/$(TARGET)

# vi: sw=4 ts=4 sts=4 noet ai
