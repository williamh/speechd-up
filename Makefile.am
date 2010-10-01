# Process this file with automake to produce Makefile.in

bin_PROGRAMS = speechd-up
speechd_up_CFLAGS = -Wall
speechd_up_CPPFLAGS = -DLOGPATH=\"$(logpath)\" \
	-DPIDPATH=\"$(pidpath)\" \
	-DSYS_CONF=\"$(sysconfdir)\"
speechd_up_LDADD = $(DOTCONF_LIBS)
speechd_up_SOURCES = speechd-up.c\
	options.c \
	options.h \
	log.c \
	log.h \
	configuration.c \
	configuration.h

dist_sysconf_DATA = speechd-up.conf

info_TEXINFOS = speechd-up.texi
speechd_up_TEXINFOS = gpl.texi fdl.texi

EXTRA_DIST = build.sh
