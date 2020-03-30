# nanabozo Makefile

NAME = nanabozo
VERSION = 0.1-alpha
CC = cc
CFLAGS = -g -Wall -O3
DESTDIR = /usr/local
INPUTSIZE = 512

ALLFILES = CMakeLists.txt LICENSE.txt Makefile README.rst \
		   debian examples nanabozo.1 nanabozo.c

export DESTDIR
export NAME

.PHONY: all clean distclean install srcpack uninstall

.DEFAULT_GOAL := all

$(NAME): nanabozo.c
	$(CC) $(CFLAGS) -DINPUTSIZE=$(INPUTSIZE) -o $@ $<

$(DESTDIR)/bin:
	mkdir -p $@

$(DESTDIR)/bin/$(NAME): $(DESTDIR)/bin $(NAME)
	cp -f $(NAME) $<

$(DESTDIR)/share/man/man1:
	mkdir -p $@

$(NAME).1.gz: nanabozo.1
	gzip -9 -c $< > $@

$(DESTDIR)/share/man/man1/$(NAME).1.gz: $(DESTDIR)/share/man/man1 $(NAME).1.gz
	cp -f $(NAME).1.gz $<

$(DESTDIR)/share/doc/$(NAME):
	mkdir -p $@

$(DESTDIR)/share/doc/$(NAME)/README.rst: $(DESTDIR)/share/doc/$(NAME) README.rst
	cp -f README.rst $<

$(NAME)-$(VERSION).tar.xz: $(ALLFILES)
	mkdir $(NAME)-$(VERSION)
	cp -r $(ALLFILES) $(NAME)-$(VERSION)
	tar --remove-files -cJf $(NAME)-$(VERSION).tar.xz $(NAME)-$(VERSION)

all: $(NAME) $(NAME).1.gz

clean: distclean
distclean:
	rm -rf $(NAME) $(NAME).1.gz $(NAME)-*.tar.xz

install: $(DESTDIR)/bin/$(NAME) $(DESTDIR)/share/man/man1/$(NAME).1.gz \
	$(DESTDIR)/share/doc/$(NAME)/README.rst
	cd examples && $(MAKE) install

srcpack: $(NAME)-$(VERSION).tar.xz

uninstall:
	rm -f $(DESTDIR)/bin/$(NAME)
	rm -rf $(DESTDIR)/share/doc/$(NAME)
	rm -f $(DESTDIR)/share/man/man1/$(NAME).1.gz

# vi: sw=4 ts=4 noet
