# nanabozo Makefile

TARGET = nanabozo
CC = cc
BINDIR = $(HOME)/bin
INPUTSIZE = 512

$(TARGET):
	$(CC) -Wall -DINPUTSIZE=$(INPUTSIZE) -o $@ nanabozo.c

$(BINDIR):
	mkdir -p $@

$(BINDIR)/$(TARGET): $(BINDIR) $(TARGET)
	cp -f $(TARGET) $(BINDIR)

.PHONY:
clean:
	rm -f $(TARGET)

install: $(BINDIR)/$(TARGET)

uninstall:
	rm -f $(BINDIR)/$(TARGET)

# vi: sw=4 ts=4 sts=4 noet ai
