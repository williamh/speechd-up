/*
 * options.h - Defines possible command line options
 *
 * Copyright (C) 2003 Brailcom, o.p.s.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id: options.h,v 1.4 2005-07-04 09:35:11 hanke Exp $
 */

#include <getopt.h>

#define PACKAGE "speechd-up"
#define VERSION "0.0"

int LOG_LEVEL;
char *LOG_FILE_NAME;

int SPD_SPK_MODE;
#define MODE_DAEMON 1
#define MODE_SINGLE 0

char *SPEAKUP_DEVICE;
char *SPEAKUP_CODING;

int PROBE_MODE;

static struct option spd_long_options[] = {
    {"run-daemon", 0, 0, 'd'},
    {"run-single", 0, 0, 's'},
    {"log-level", 1, 0, 'l'},
    {"log-file", 1, 0, 'L'},
    {"device", 1, 0, 'D'},
    {"coding", 1, 0, 'c'},
    {"synthesis", 1, 0, 'S'},
    {"probe", 0, 0, 'p'},
    {"version", 0, 0, 'v'},
    {"help", 0, 0, 0},
    {0, 0, 0, 0}
};

static char* spd_short_options = "dsvhpl:L:D:S:c:";
