# simple makefile compiling the examples

CC = cc
C++ = c++

.DEFAULT_GOAL := build

basic.c: basic.php
	nanabozo --main --html $< $@

basic.cgi: basic.c
	$(CC) -o $@ $<

buffered_output.cpp: buffered_output.php
	nanabozo -p ms.input -f ms.inputf $< $@

buffered_output.cgi: buffered_output.cpp
	$(C++) -o $@ $<

function.c: function.php
	nanabozo $< $@

function.cgi: function.c
	$(CC) -o $@ $<

.PHONY: build clean

build: basic.cgi buffered_output.cgi function.cgi

clean:
	rm -f *.c *.cpp *.cgi

# vi: sw=4 ts=4 noet ft=make
