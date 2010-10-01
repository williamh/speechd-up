
/*
 * configuration.c -- Dotconf configuration for SpeechD-Up
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
#include <string.h>
#include <dotconf.h>
#include <assert.h>

#include "log.h"
#include "configuration.h"
#include "options.h"

/* == CONFIGURATION MANAGEMENT FUNCTIONS */

/* Add dotconf configuration option */
configoption_t *add_config_option(configoption_t * options,
				  int *num_config_options, char *name, int type,
				  dotconf_callback_t callback, info_t * info,
				  unsigned long context)
{
	configoption_t *opts;

	(*num_config_options)++;
	opts = (configoption_t *) realloc(options, (*num_config_options)
					  * sizeof(configoption_t));
	opts[*num_config_options - 1].name = strdup(name);
	opts[*num_config_options - 1].type = type;
	opts[*num_config_options - 1].callback = callback;
	opts[*num_config_options - 1].info = info;
	opts[*num_config_options - 1].context = context;
	return opts;
}

/* Free all configuration options. */
void free_config_options(configoption_t * opts, int *num)
{
	int i = 0;

	if (opts == NULL)
		return;

	for (i = 0; i <= (*num) - 1; i++) {
		free((char *)opts[i].name);
	}
	free(opts);
	*num = 0;
	opts = NULL;
}

/* == CALLBACK DEFINITION MACROS == */

#define DOTCONF_CB_STR(name, arg) \
   DOTCONF_CB(cb_ ## name) \
   { \
       assert(cmd->data.str != NULL); \
       if (options.arg ## _set != COMMAND_LINE){ \
           LOG(3, "Setting " #name " to %s", cmd->data.str); \
           options.arg = strdup(cmd->data.str); \
           options.arg ## _set = CONFIG_FILE; \
           LOG(3, "Setting " #name " set to %d", options.arg ## _set); \
       } \
       return NULL; \
   }

#define DOTCONF_CB_INT(name, arg, cond, str) \
   DOTCONF_CB(cb_ ## name) \
   { \
       int val = cmd->data.value; \
       if (!(cond)) FATAL(-1, str); \
       if (options.arg ## _set != COMMAND_LINE){ \
            LOG(3, "Setting " #name " to %d", val); \
            options.arg = val; \
            options.arg ## _set = CONFIG_FILE; \
       } \
       return NULL; \
   }

/* == CALLBACK DEFINITIONS == */
DOTCONF_CB_INT(LogLevel, log_level, ((val >= 0) && (val <= 5)),
	       "Log level must be between 1 (least verbose) and 5 (very verbose, "
	       "only for debugging).");
DOTCONF_CB_STR(LogFile, log_file_name);
DOTCONF_CB_STR(SpeakupDevice, speakup_device);
DOTCONF_CB_STR(SpeakupChartab, speakup_chartab);
DOTCONF_CB_STR(SpeakupCharacters, speakup_characters);
DOTCONF_CB_STR(SpeakupCoding, speakup_coding);
DOTCONF_CB_STR(Language, language);
DOTCONF_CB_INT(DontInitTables, dont_init_tables,
	       (val == 0) || (val == 1),
	       "DontInitTables must be either 0 or 1.");

DOTCONF_CB(cb_unknown)
{
	LOG(2, "Unknown option in configuration!");
	return NULL;
}

/* == LOAD CALLBACKS == */

#define ADD_CONFIG_OPTION(name, arg_type) \
   options = add_config_option(options, num_options, #name, arg_type, cb_ ## name, 0, 0);

#define ADD_LAST_OPTION() \
   options = add_config_option(options, num_options, "", 0, NULL, NULL, 0);

configoption_t *load_config_options(int *num_options)
{
	configoption_t *options = NULL;

	ADD_CONFIG_OPTION(LogLevel, ARG_INT);
	ADD_CONFIG_OPTION(LogFile, ARG_STR);
	ADD_CONFIG_OPTION(SpeakupDevice, ARG_STR);
	ADD_CONFIG_OPTION(SpeakupCoding, ARG_STR);
	ADD_CONFIG_OPTION(SpeakupChartab, ARG_STR);
	ADD_CONFIG_OPTION(SpeakupCharacters, ARG_STR);
	ADD_CONFIG_OPTION(Language, ARG_STR);
	ADD_CONFIG_OPTION(DontInitTables, ARG_TOGGLE);

	ADD_LAST_OPTION();

	return options;

}
