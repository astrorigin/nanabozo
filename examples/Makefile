# nanabozo examples Makefile

NAME = nanabozo
DESTDIR = /usr/local

ALLEXAMPLES = Makefile.ex MemStream.cxx basic.php buffered_output.php function.php

.DEFAULT_GOAL := void

.PHONY: install uninstall void

$(DESTDIR)/share/doc/$(NAME)/examples:
	mkdir -p $@

install: $(DESTDIR)/share/doc/$(NAME)/examples $(ALLEXAMPLES)
	cp -f $(ALLEXAMPLES) $<

uninstall:
	rm -rf $(DESTDIR)/share/doc/$(NAME)/examples

void: ;

# vi: sw=4 ts=4 noet
