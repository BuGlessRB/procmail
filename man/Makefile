#$Id: Makefile,v 1.6 1994/05/26 14:11:57 berg Exp $

HIDEMAKE=$(MAKE)

all: init
	$(HIDEMAKE) make $@

# The only real thing that can be made right now is:

init:
	cd ..; $(MAKE) make init

.PRECIOUS: Makefile

Makefile makefile Makefiles makefiles: init

procmail.1 procmailrc.5 procmailsc.5 procmailex.5 lockfile.1 formail.1: init
	$(HIDEMAKE) make $@
