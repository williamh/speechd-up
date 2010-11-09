
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dotconf.h>

#include "log.h"
#include "configuration.h"
#include "options.h"

extern struct spd_options options;

/*
 * Declare the dotconf error handler and callbacks.
 */
static FUNC_ERRORHANDLER(errorhandler);
static DOTCONF_CB(cb_dontInitTables);
static DOTCONF_CB(cb_language);
static DOTCONF_CB(cb_logFile);
static DOTCONF_CB(cb_logLevel);
static DOTCONF_CB(cb_speakupCharacters);
static DOTCONF_CB(cb_speakupChartab);
static DOTCONF_CB(cb_speakupCoding);
static DOTCONF_CB(cb_speakupDevice);

/*
 * Initialize the array of configuration options.
 */
static const configoption_t configOptions[] = {
	{"DontInitTables", ARG_TOGGLE, cb_dontInitTables, NULL, CTX_ALL,},
	{"Language", ARG_STR, cb_language, NULL, CTX_ALL,},
	{"LogFile", ARG_STR, cb_logFile, NULL, CTX_ALL,},
	{"LogLevel", ARG_INT, cb_logLevel, NULL, CTX_ALL,},
	{"SpeakupCharacters", ARG_STR, cb_speakupCharacters, NULL, CTX_ALL,},
	{"SpeakupChartab", ARG_STR, cb_speakupChartab, NULL, CTX_ALL,},
	{"SpeakupCoding", ARG_STR, cb_speakupCoding, NULL, CTX_ALL,},
	{"SpeakupDevice", ARG_STR, cb_speakupDevice, NULL, CTX_ALL,},
	LAST_OPTION
};

static FUNC_ERRORHANDLER(errorhandler)
{
	LOG(2, "Configuration file error: %s\n", msg);
	return 0;
}

static DOTCONF_CB(cb_dontInitTables)
{
	if (options.dont_init_tables_set != COMMAND_LINE) {
		LOG(3, "setting %s to %i\n", cmd->name, cmd->data.value);
		options.dont_init_tables = cmd->data.value;
		options.dont_init_tables_set = CONFIG_FILE;
		LOG(3, "setting %s has value %i\n", cmd->name, cmd->data.value);
	}
	return NULL;
}

static DOTCONF_CB(cb_language)
{
	assert(cmd->data.str);
	if (options.language_set != COMMAND_LINE) {
		LOG(3, "setting %s to %s\n", cmd->name, cmd->data.str);
		free(options.language);
		options.language = strdup(cmd->data.str);
		options.language_set = CONFIG_FILE;
		LOG(3, "setting %s has value %s\n", cmd->name, cmd->data.str);
	}
	return NULL;
}

static DOTCONF_CB(cb_logFile)
{
	assert(cmd->data.str);
	if (options.log_file_name_set != COMMAND_LINE) {
		LOG(3, "setting %s to %s\n", cmd->name, cmd->data.str);
		free(options.log_file_name);
		options.log_file_name = strdup(cmd->data.str);
		options.log_file_name_set = CONFIG_FILE;
		LOG(3, "setting %s has value %s\n", cmd->name, cmd->data.str);
	}
	return NULL;
}

static DOTCONF_CB(cb_logLevel)
{
	if ((cmd->data.value < 1) || (cmd->data.value > 5))
		FATAL(-1, "Log level must be between 1 and 5");
	if (options.log_level_set != COMMAND_LINE) {
		LOG(3, "setting %s to %i\n", cmd->name, cmd->data.value);
		options.log_level = cmd->data.value;
		options.log_level_set = CONFIG_FILE;
		LOG(3, "setting %s has value %i\n", cmd->name, cmd->data.value);
	}
	return NULL;
}

static DOTCONF_CB(cb_speakupCharacters)
{
	assert(cmd->data.str);
	if (options.speakup_characters_set != COMMAND_LINE) {
		LOG(3, "setting %s to %s\n", cmd->name, cmd->data.str);
		free(options.speakup_characters);
		options.speakup_characters = strdup(cmd->data.str);
		options.speakup_characters_set = CONFIG_FILE;
		LOG(3, "setting %s has value %s\n", cmd->name, cmd->data.str);
	}
	return NULL;
}

static DOTCONF_CB(cb_speakupChartab)
{
	assert(cmd->data.str);
	if (options.speakup_chartab_set != COMMAND_LINE) {
		LOG(3, "setting %s to %s\n", cmd->name, cmd->data.str);
		free(options.speakup_chartab);
		options.speakup_chartab = strdup(cmd->data.str);
		options.speakup_chartab_set = CONFIG_FILE;
		LOG(3, "setting %s has value %s\n", cmd->name, cmd->data.str);
	}
	return NULL;
}

static DOTCONF_CB(cb_speakupCoding)
{
	assert(cmd->data.str);
	if (options.speakup_coding_set != COMMAND_LINE) {
		LOG(3, "setting %s to %s\n", cmd->name, cmd->data.str);
		free(options.speakup_coding);
		options.speakup_coding = strdup(cmd->data.str);
		options.speakup_coding_set = CONFIG_FILE;
		LOG(3, "setting %s has value %s\n", cmd->name, cmd->data.str);
	}
	return NULL;
}

static DOTCONF_CB(cb_speakupDevice)
{
	assert(cmd->data.str);
	if (options.speakup_device_set != COMMAND_LINE) {
		LOG(3, "setting %s to %s\n", cmd->name, cmd->data.str);
		free(options.speakup_device);
		options.speakup_device = strdup(cmd->data.str);
		options.speakup_device_set = CONFIG_FILE;
		LOG(3, "setting %s has value %s\n", cmd->name, cmd->data.str);
	}
	return NULL;
}

void load_configuration(void)
{
	configfile_t *configfile;

	configfile = dotconf_create(options.config_file_name, configOptions,
				    NULL, CASE_INSENSITIVE);
	if (!configfile) {
		LOG(0, "Error opening config file\n");
		exit(1);
	}
	configfile->errorhandler = (dotconf_errorhandler_t) errorhandler;
	if (dotconf_command_loop(configfile) == 0)
		FATAL(-1, "Error reading config file\n");
	dotconf_cleanup(configfile);
	LOG(1, "Configuration has been read from \"%s\"",
	    options.config_file_name);
}
