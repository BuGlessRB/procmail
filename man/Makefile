#$Id: Makefile,v 1.4 1993/11/29 17:22:34 berg Exp $

HIDEMAKE=$(MAKE)

all: init
	$(HIDEMAKE) make $@

# The only real thing that can be made right now is:

init:
	cd ..; $(MAKE) make init

Makefile makefile Makefiles makefiles: init

procmail.1 procmailrc.5 procmailsc.5 procmailex.5 lockfile.1 formail.1: init
	$(HIDEMAKE) make $@
