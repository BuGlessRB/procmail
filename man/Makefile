#$Id: Makefile,v 1.1 1992/09/28 14:27:54 berg Exp $

HIDEMAKE=$(MAKE)

all: init
	$(HIDEMAKE) $@

# The only real thing that can be made right now is:

init:
	cd ..; $(MAKE) init

Makefile makefile Makefiles makefiles: init

procmail.1 procmailrc.5 procmailex.5 lockfile.1 formail.1: init
	$(HIDEMAKE) $@
