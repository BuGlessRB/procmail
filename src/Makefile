#$Id: Makefile,v 1.7 1994/05/26 14:12:19 berg Exp $

HIDEMAKE=$(MAKE)

all: init
	$(HIDEMAKE) make $@

# The only real thing that can be made right now is:

init:
	cd ..; $(MAKE) make init

.PRECIOUS: Makefile

Makefile makefile Makefiles makefiles: init

procmail lockfile formail multigram mailstat setid ../autoconf.h \
autoconf.h: init
	$(HIDEMAKE) make $@
