#$Id: Makefile,v 1.4 1993/08/25 15:30:04 berg Exp $

HIDEMAKE=$(MAKE)

all: init
	$(HIDEMAKE) make $@

# The only real thing that can be made right now is:

init:
	cd ..; $(MAKE) make init

Makefile makefile Makefiles makefiles: init

procmail lockfile formail multigram mailstat ../autoconf.h autoconf.h: init
	$(HIDEMAKE) make $@
