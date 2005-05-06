## Process this file with automake to produce Makefile.in

bin_PROGRAMS = speechd_up

speechd_up_SOURCES = speechd-up.c options.c options.h
speechd_up_CFLAGS = -DPIDPATH=\"@pidpath@\"
speechd_up_LDADD = -lspeechd
COMPILE = $(CC) $(speechd_up_CFLAGS) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
EXTRA_DIST= ChangeLog NEWS TODO AUTHORS COPYING INSTALL README

