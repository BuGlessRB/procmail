#$Id: Makefile,v 1.5 1993/12/13 15:52:59 berg Exp $

HIDEMAKE=$(MAKE)

all: init
	$(HIDEMAKE) make $@

# The only real thing that can be made right now is:

init:
	cd ..; $(MAKE) make init

.PRECIOUS: Makefile

Makefile makefile Makefiles makefiles: init

procmail lockfile formail multigram mailstat ../autoconf.h autoconf.h: init
	$(HIDEMAKE) make $@
