## Process this file with automake to produce Makefile.in

bin_PROGRAMS = speechd_up

speechd_up_SOURCES = speechd-up.c options.c options.h
speechd_up_CFLAGS = -DPIDPATH=\"@pidpath@\"
speechd_up_LDADD = -lspeechd

EXTRA_DIST= ChangeLog NEWS TODO AUTHORS COPYING INSTALL README

