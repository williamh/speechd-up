#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([speechd-up], [0.5-dev])
AM_INIT_AUTOMAKE([foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
AC_CONFIG_SRCDIR([speechd-up.c])
AC_CONFIG_HEADERS([config.h])

AC_ARG_VAR([logpath], [Absolute path to a log file])
AC_ARG_VAR([pidpath], [Absolute path to a pid file])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
PKG_CHECK_MODULES([DOTCONF], [dotconf])
AC_SUBST([DOTCONF_CFLAGS])
AC_SUBST([DOTCONF_LIBS])
AC_SEARCH_LIBS([spd_open], [speechd], [],
	[AC_MSG_FAILURE([unable to find libspeechd])])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h locale.h stdlib.h string.h unistd.h wchar.h wctype.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([select strdup strerror strtol])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
