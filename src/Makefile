#$Id: Makefile,v 1.1 1992/09/28 14:28:02 berg Exp $

HIDEMAKE=$(MAKE)

all: init
	$(HIDEMAKE) $@

# The only real thing that can be made right now is:

init:
	cd ..; $(MAKE) init

Makefile makefile Makefiles makefiles: init

procmail lockfile formail mailstat ../autoconf.h autoconf.h: init
	$(HIDEMAKE) $@
