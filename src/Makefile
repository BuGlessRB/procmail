#$Id: Makefile,v 1.2 1992/10/02 14:39:26 berg Exp $

HIDEMAKE=$(MAKE)

all: init
	$(HIDEMAKE) make $@

# The only real thing that can be made right now is:

init:
	cd ..; $(MAKE) make init

Makefile makefile Makefiles makefiles: init

procmail lockfile formail mailstat ../autoconf.h autoconf.h: init
	$(HIDEMAKE) make $@
