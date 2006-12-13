/*
 * options.h - Defines possible command line options
 *
 * Copyright (C) 2003, 2006 Brailcom, o.p.s.
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
 * $Id: options.h,v 1.8 2006-12-13 18:00:09 hanke Exp $
 */

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PACKAGE "speechd-up"

#define MODE_DAEMON 1
#define MODE_SINGLE 0

#define DEFAULT 0
#define COMMAND_LINE 1
#define CONFIG_FILE 2

struct {
    int log_level;
    int log_level_set;
    char *log_file_name;
    int log_file_name_set;
    int spd_spk_mode;
    char *speakup_device;
    int speakup_device_set;
    char *speakup_coding;
    int speakup_coding_set;
    char *language;
    int language_set;
    int probe_mode;
    int dont_init_tables;
    int dont_init_tables_set;
} options;

static struct option spd_long_options[] = {
    {"run-daemon", 0, 0, 'd'},
    {"run-single", 0, 0, 's'},
    {"log-level", 1, 0, 'l'},
    {"log-file", 1, 0, 'L'},
    {"device", 1, 0, 'D'},
    {"coding", 1, 0, 'c'},
    {"language", 1, 0, 'i'},
    {"synthesis", 1, 0, 'S'},
    {"dont-init-tables", 0, 0, 't'},
    {"probe", 0, 0, 'p'},
    {"version", 0, 0, 'v'},
    {"help", 0, 0, 0},
    {0, 0, 0, 0}
};

static char* spd_short_options = "dsvhpti:l:L:D:S:c:";
