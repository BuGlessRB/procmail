#$Id: Makefile,v 1.6 1992/10/20 15:36:45 berg Exp $

# change BASENAME to your home directory if need be
BASENAME = /usr/local

# You can predefine ARCHITECTURE to a bin directory suffix
#ARCHITECTURE=.sun4

BINDIR	  = $(BASENAME)/bin$(ARCHITECTURE)
MANDIR	  = $(BASENAME)/man
# MAN1SUFFIX for regular utility manuals
MAN1SUFFIX= 1
# MAN5SUFFIX for file-format descriptions
MAN5SUFFIX= 5
MAN1DIR	  = $(MANDIR)/man$(MAN1SUFFIX)
MAN5DIR	  = $(MANDIR)/man$(MAN5SUFFIX)

# Things that can be made are:

# init (or makefiles)	Performs some preliminary sanity checks on your system
#			and generates Makefiles accordingly
# bins			Preinstalls only the binaries to ./new
# mans			Preinstalls only the man pages to ./new
# all			Does both
# install.bin		Installs the binaries from ./new to $(BINDIR)
# install.man		Installs the man pages from ./new to $(MAN[15]DIR)
# install		Does both
# recommend		Show some recommended suid/sgid modes
# suid			Impose the modes shown by 'make recommend'
# clean			Attempts to restore the package to pre-make state
# realclean		Attempts to restore the package to pre-make-init state
# deinstall		Removes any previously installed binaries and man
#			pages from your system by careful surgery
# autoconf.h		Will list your system's anomalies
# procmail		Preinstalls just all procmail related stuff to ./new
# formail		Preinstalls just all formail related stuff to ./new
# lockfile		Preinstalls just all lockfile related stuff to ./new

########################################################################
# Only edit below this line if you *think* you know what you are doing #
########################################################################

# Makefile.0 - mark, don't (re)move this, a sed script needs it

# Directory for the standard include files
USRINCLUDE = /usr/include

CFLAGS0 = -O #-ansi -pedantic -Wid-clash-6
LDFLAGS0= -s

CFLAGS1 = $(CFLAGS0)
LDFLAGS1= $(LDFLAGS0) #-lcposix

####CC	= cc # gcc
O	= o
RM	= /bin/rm -f
INSTALL = cp
DEVNULL = /dev/null

SUBDIRS = src man
BINSS	= procmail lockfile formail mailstat
MANS1S	= procmail formail lockfile
MANS5S	= procmailrc procmailex

# Makefile - mark, don't (re)move this, a sed script needs it

HIDEMAKE=$(MAKE)

all: init
	$(HIDEMAKE) make $@

make:
	@/bin/sh -c "exit 0"

init:
	/bin/sh ./initmake "$(SHELL)" "$(RM)" $(USRINCLUDE) $(DEVNULL) \
	 "$(HIDEMAKE)" $(O) "$(CC)" "$(CFLAGS1)" "$(LDFLAGS1)" "$(BINSS)" \
	 "$(MANS1S)" "$(MANS5S)" "$(SUBDIRS)"

makefiles makefile Makefiles Makefile: init

bins mans install.bin install.man install recommend suid clean realclean \
deinstall autoconf.h procmail formail lockfile: init
	$(HIDEMAKE) make $@
