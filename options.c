/*
 * options.c - Parse and process possible command line options
 *
 * Copyright (C) 2003,2004, 2006, 2007 Brailcom, o.p.s.
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
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "options.h"

static struct option spd_long_options[] = {
	{"run-daemon", 0, 0, 'd'},
	{"run-single", 0, 0, 's'},
	{"log-level", 1, 0, 'l'},
	{"log-file", 1, 0, 'L'},
	{"config-file", 1, 0, 'C'},
	{"device", 1, 0, 'D'},
	{"coding", 1, 0, 'c'},
	{"language", 1, 0, 'i'},
	{"synthesis", 1, 0, 'S'},
	{"dont-init-tables", 0, 0, 't'},
	{"probe", 0, 0, 'p'},
	{"version", 0, 0, 'v'},
	{"help", 0, 0, 'h'},
	{0, 0, 0, 0}
};

static char *spd_short_options = "dsvhpti:l:L:C:D:S:c:";

struct spd_options options;

#define SPD_OPTION_SET_INT(param) \
    val = strtol(optarg, &tail_ptr, 10); \
    if(tail_ptr != optarg){ \
        param = val; \
    }

void options_print_help(char *argv[])
{
	assert(argv);
	assert(argv[0]);

	printf
	    ("Usage: %s [-{d|s}] [-l {1|2|3|4|5}] [-L=logfile] [-c=encoding] | [-v] | [-h]\n",
	     argv[0]);
	printf
	    ("SpeechD-Up -- Interface between Speech Dispatcher and SpeakUp (GNU GPL)\n\n");
	printf("-d, --run-daemon     -      Run as a daemon\n"
	       "-s, --run-single     -      Run as single application\n"
	       "-l, --log-level      -      Set log level (1..5)\n"
	       "-L, --log-file       -      Set log file to path\n"
	       "-D, --device         -      Specify the device name of Speakup software synthesis\n"
	       "-i, --language       -      Set default language for speech output\n"
	       "-c, --coding         -      Specify the default encoding to use\n"
	       "-t, --dont-init-tables -    Don't rewrite /proc tables for optimal software synthesis\n"
	       "-p, --probe          -      Initialize everything and try to say some message\n"
	       "                            but don't connect to SpeakUp. For testing purposes.\n"
	       "-v, --version        -      Report version of this program\n"
	       "-h, --help           -      Print this info\n\n"
	       "Copyright (C) 2003,2005 Brailcom, o.p.s.\n"
	       "This is free software; you can redistribute it and/or modify it\n"
	       "under the terms of the GNU General Public License as published by\n"
	       "the Free Software Foundation; either version 2, or (at your option)\n"
	       "any later version. Please see COPYING for more details.\n\n"
	       "Please report bugs to %s\n\n", PACKAGE_BUGREPORT);
}

void options_print_version(void)
{
	printf("%s %s\n", PACKAGE, VERSION);
	printf("Copyright (C) 2002-2003 Brailcom, o.p.s.\n"
	       "GNU Emacs comes with ABSOLUTELY NO WARRANTY.\n"
	       "You may redistribute copies of Emacs\n"
	       "under the terms of the GNU General Public License.\n"
	       "For more information about these matters, see the file named COPYING.\n");
}

void options_set_default(void)
{
	options.spd_spk_mode = MODE_DAEMON;
	options.log_level = 3;
	options.log_level_set = DEFAULT;
	if (!strcmp(LOGPATH, ""))
		options.log_file_name = strdup("/var/log/speechd-up.log");
	else
		options.log_file_name = strdup(LOGPATH "/speechd-up.log");
	options.log_file_name_set = DEFAULT;
	options.config_file_name = strdup(SYS_CONF "/speechd-up.conf");
	options.speakup_device = strdup("/dev/softsynth");
	options.speakup_device_set = DEFAULT;
	options.speakup_chartab =
	    strdup("/sys/accessibility/speakup/i18n/chartab");
	options.speakup_chartab_set = DEFAULT;
	options.speakup_characters =
	    strdup("/sys/accessibility/speakup/i18n/characters");
	options.speakup_device_set = DEFAULT;
	options.speakup_coding = strdup("iso-8859-1");
	options.speakup_coding_set = DEFAULT;
	options.language = strdup("en");
	options.language_set = DEFAULT;
	options.probe_mode = 0;
	options.dont_init_tables = 0;
	options.dont_init_tables_set = DEFAULT;
}

void options_parse(int argc, char *argv[])
{
	char *tail_ptr;
	int c_opt;
	int option_index;
	int val;

	assert(argc > 0);
	assert(argv);

	while (1) {
		option_index = 0;

		c_opt =
		    getopt_long(argc, argv, spd_short_options, spd_long_options,
				&option_index);

		if (c_opt == -1)
			break;

		switch (c_opt) {
		case 'd':
			options.spd_spk_mode = MODE_DAEMON;
			break;
		case 's':
			options.spd_spk_mode = MODE_SINGLE;
			break;
		case 'l':
			SPD_OPTION_SET_INT(options.log_level);
			options.log_level_set = COMMAND_LINE;
			break;
		case 'L':
			if (options.log_file_name != 0)
				free(options.log_file_name);
			options.log_file_name = strdup(optarg);
			options.log_file_name_set = COMMAND_LINE;
			break;
		case 'C':
			if (options.config_file_name != 0)
				free(options.config_file_name);
			options.config_file_name = strdup(optarg);
			break;
		case 'D':
			if (options.speakup_device != 0)
				free(options.speakup_device);
			options.speakup_device = strdup(optarg);
			options.speakup_device_set = COMMAND_LINE;
			break;
		case 'c':
			if (options.speakup_coding != 0)
				free(options.speakup_coding);
			options.speakup_coding = strdup(optarg);
			options.speakup_coding_set = COMMAND_LINE;
			break;
		case 'i':
			if (options.language != 0)
				free(options.language);
			options.language = strdup(optarg);
			options.language_set = COMMAND_LINE;
		case 'v':
			options_print_version();
			exit(0);
			break;
		case 'h':
			options_print_help(argv);
			exit(0);
			break;
		case 'p':
			options.probe_mode = 1;
			break;
		case 't':
			options.dont_init_tables = 1;
			options.dont_init_tables_set = COMMAND_LINE;
			break;
		default:
			printf("Error: Unrecognized option\n\n");
			options_print_help(argv);
			exit(1);
		}
	}
}
