#$Id: Makefile,v 1.3 1992/11/11 13:58:08 berg Exp $

HIDEMAKE=$(MAKE)

all: init
	$(HIDEMAKE) make $@

# The only real thing that can be made right now is:

init:
	cd ..; $(MAKE) make init

Makefile makefile Makefiles makefiles: init

procmail.1 procmailrc.5 procmailex.5 lockfile.1 formail.1: init
	$(HIDEMAKE) make $@
