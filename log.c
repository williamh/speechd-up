/*
 * log.c - Logging for speechd-up
 *
 * Copyright (C) 2001, 2002, 2003, 2006 Brailcom, o.p.s.
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>

#include "options.h"
#include "log.h"

FILE *logfile;

extern struct spd_options options;

void LOG(int level, char *format, ...)
{
	assert(format != NULL);
	assert(logfile != NULL);

	if (level <= options.log_level) {
		va_list args;
		int i;

		va_start(args, format);
		{
			{
				/* Print timestamp */
				time_t t;
				char *tstr;
				t = time(NULL);
				tstr = (char *)strdup((char *)ctime(&t));
				if (tstr == NULL) {
					fprintf(stderr,
						"strdup(ctime) failed, can't log");
					return;
				}
				/* Remove the trailing \n */
				assert(strlen(tstr) > 1);
				tstr[strlen(tstr) - 1] = 0;
				fprintf(logfile, "[%s] speechd: ", tstr);
				free(tstr);
			}
			for (i = 1; i < level; i++) {
				fprintf(logfile, " ");
			}
			vfprintf(logfile, format, args);
			fprintf(logfile, "\n");
			fflush(logfile);
		}
		va_end(args);
	}
}
