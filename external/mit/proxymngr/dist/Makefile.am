# 
#  Copyright 2005  Red Hat, Inc.
# 
#  Permission to use, copy, modify, distribute, and sell this software and its
#  documentation for any purpose is hereby granted without fee, provided that
#  the above copyright notice appear in all copies and that both that
#  copyright notice and this permission notice appear in supporting
#  documentation, and that the name of Red Hat not be used in
#  advertising or publicity pertaining to distribution of the software without
#  specific, written prior permission.  Red Hat makes no
#  representations about the suitability of this software for any purpose.  It
#  is provided "as is" without express or implied warranty.
# 
#  RED HAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
#  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
#  EVENT SHALL RED HAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
#  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
#  DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
#  TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
#  PERFORMANCE OF THIS SOFTWARE.

bin_PROGRAMS = proxymngr

CONFIG_DIR = $(libdir)/X11/proxymngr

AM_CFLAGS =						\
	$(CWARNFLAGS)					\
	$(PROXYMNGR_CFLAGS)				\
	-DXVENDORNAME=\"The\ X\.Org\ Foundation\"	\
	-DXORG_RELEASE=\"Release\ $(VERSION)\"		\
	-DCONFIG_FILE=\"$(CONFIG_DIR)/proxymngr/pmconfig\"

proxymngr_LDADD = $(PROXYMNGR_LIBS)

proxymngr_SOURCES =	\
        config.c \
        config.h \
        main.c \
        pmdb.c \
        pmdb.h \
        pmint.h

appman_PRE = \
        proxymngr.man

configdir = $(CONFIG_DIR)
config_DATA = pmconfig

pmconfig: pmconfig.cpp
	$(SED) -e s/LBXPROXY/`echo @LBXPROXY@ | sed -e s/\\\\//\\\\\\\\\\\\//g`/ < $(srcdir)/pmconfig.cpp > $@

EXTRA_DIST = pmconfig.cpp
CLEANFILES = pmconfig

appmandir = $(APP_MAN_DIR)

appman_DATA = $(appman_PRE:man=@APP_MAN_SUFFIX@)

EXTRA_DIST += $(appman_PRE)
CLEANFILES += $(appman_DATA)

SUFFIXES = .$(APP_MAN_SUFFIX) .man

# String replacements in MAN_SUBSTS now come from xorg-macros.m4 via configure
.man.$(APP_MAN_SUFFIX):
	$(AM_V_GEN)$(SED) $(MAN_SUBSTS) < $< > $@

EXTRA_DIST += ChangeLog
MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog
