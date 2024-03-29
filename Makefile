# nanabozo Makefile

# debug build:  NDEBUG=0 make -e

NAME = nanabozo
VERSION = 0.1-alpha
NDEBUG ?= 1
CC = cc
ifeq ($(NDEBUG),1)
CFLAGS = -Wall -Wextra -O3
else
CFLAGS = -g -Og -Wall -Wextra -fsanitize=address -fno-omit-frame-pointer
endif
DESTDIR = /usr/local
INPUTSIZE = 512

ALLFILES = CMakeLists.txt LICENSE.txt Makefile README.rst \
		   examples nanabozo.1 nanabozo.c

export DESTDIR
export NAME

.PHONY: build clean distclean re install install-all install-doc \
	install-ex install-man srcpack uninstall

.DEFAULT_GOAL := build

$(NAME): nanabozo.c
	$(CC) $(CFLAGS) -DINPUTSIZE=$(INPUTSIZE) -o $@ $<
ifeq ($(NDEBUG),1)
	strip --strip-unneeded --remove-section=.comment --remove-section=.note $@
endif

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

README.rst.gz: README.rst
	gzip -9 -c $< > $@

$(DESTDIR)/share/doc/$(NAME)/README.rst.gz: $(DESTDIR)/share/doc/$(NAME) README.rst.gz
	cp -f README.rst.gz $<

$(NAME)-$(VERSION).tar.xz: $(ALLFILES)
	mkdir $(NAME)-$(VERSION)
	cp -rf $(ALLFILES) $(NAME)-$(VERSION)
	tar --remove-files -cJf $(NAME)-$(VERSION).tar.xz $(NAME)-$(VERSION)
	rm -rf $(NAME)-$(VERSION)

build: $(NAME)

clean: distclean
distclean:
	rm -rf $(NAME) $(NAME).1.gz $(NAME)-*.tar.xz README.rst.gz

re: clean build

install: $(DESTDIR)/bin/$(NAME)

install-doc: $(DESTDIR)/share/doc/$(NAME)/README.rst.gz

install-ex:
	$(MAKE) -C examples install

install-man: $(DESTDIR)/share/man/man1/$(NAME).1.gz

install-all: install install-doc install-ex install-man

srcpack: $(NAME)-$(VERSION).tar.xz

uninstall:
	rm -f $(DESTDIR)/bin/$(NAME)
	rm -rf $(DESTDIR)/share/doc/$(NAME)
	rm -f $(DESTDIR)/share/man/man1/$(NAME).1.gz

# vi: sw=4 ts=4 noet
