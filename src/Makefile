#$Id: Makefile,v 1.8 1994/08/24 18:52:04 berg Exp $

all: init
	$(MAKE) make $@

# The only real thing that can be made right now is:

init:
	cd ..; $(MAKE) make init

.PRECIOUS: Makefile

Makefile makefile Makefiles makefiles: init

procmail lockfile formail multigram mailstat setid ../autoconf.h \
autoconf.h: init
	$(MAKE) make $@
