# ----------------------------------------------------------------------------
# Makefile for building tapp
#
# Copyright 2010 FriendlyARM (http://www.arm9.net/)
#

ifndef DESTDIR
DESTDIR			   ?= /tmp/FriendlyARM/mini6410/rootfs
endif

CFLAGS				= -Wall -O2
CC					= arm-linux-g++
INSTALL				= install

TARGET				= float-test.soft float-test.vfp


all: $(TARGET)

float-test.soft: float-test.c
	$(CC) $(CFLAGS) -mfloat-abi=soft $< -o $@

float-test.vfp: float-test.c
	$(CC) $(CFLAGS) $< -o $@

install: $(TARGET)
	$(INSTALL) $^ $(DESTDIR)/usr/bin

clean distclean:
	rm -rf *.o $(TARGET)


# ----------------------------------------------------------------------------

.PHONY: $(PHONY) install clean distclean

# End of file
# vim: syntax=make

