
/*
 * speechd-up.c - Simple interface between SpeakUp and Speech Dispatcher
 *
 * Copyright (C) 2004, 2006, 2007 Brailcom, o.p.s.
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>
#include <signal.h>
#include <ctype.h>
#include <locale.h>

#include <wchar.h>
#include <wctype.h>

#include <iconv.h>
#include <libspeechd.h>

#include "log.h"
#include "options.h"
#include "configuration.h"

#define BUF_SIZE 1024

#define DTLK_STOP 24
#define DTLK_CMD 1

int fd;
fd_set fd_list;
fd_set fd_write;
SPDConnection *conn;

char *spd_spk_pid_file;

void init_ssml_char_escapes(void);
void spd_spk_reset(int sig);

/* Lifted directly from speechd/src/modules/module_utils.c. */
void xfree(void *data)
{
	if (data != NULL)
		free(data);
}

void
index_marker_callback(size_t msg_id, size_t client_id, SPDNotificationType type,
		      char *index_mark)
{
	//LOG(5,"Index Mark Callback");
	if (index_mark != NULL) {
		write(fd, index_mark, sizeof(index_mark));
	}
}

void speechd_init()
{
	conn = spd_open("speakup", "softsynth", "test", SPD_MODE_THREADED);
	if (conn == 0)
		FATAL(1, "ERROR! Can't connect to Speech Dispatcher!");
	conn->callback_im = index_marker_callback;
	if (spd_set_notification_on(conn, SPD_INDEX_MARKS) == -1)
		LOG(1, "Error turning on Index Mark Callback");
	if (options.language_set != DEFAULT)
		spd_set_language(conn, options.language);
	init_ssml_char_escapes();
}

void speechd_close()
{
	spd_close(conn);
}

int init_speakup_tables()
{
	FILE *fp_char = fopen(options.speakup_characters, "w");
	if (fp_char) {
		int i = 0;

		fprintf(fp_char, "32 space\n");

		for (i = 33; i < 256; i++) {

			fprintf(fp_char, "%d %c\n", i, i);
		}
		fclose(fp_char);
	} else
		return -1;

	fp_char = fopen(options.speakup_chartab, "w");
	if (fp_char) {
		int i = 0;
		for (i = 'a'; i <= 'z'; i++) {
			fprintf(fp_char, "%d\tALPHA\n", i);
		}
		for (i = 'A'; i <= 'Z'; i++) {
			fprintf(fp_char, "%d\tALPHA\n", i);
		}
		for (i = 128; i < 256; i++) {
			fprintf(fp_char, "%d\tALPHA\n", i);
		}
		fclose(fp_char);
	} else
		return -1;

	return 0;
}

void process_command(char command, unsigned int param, int pm)
{
	int val, ret = 0;
	static int currate = 5, curpitch = 5;

	LOG(5, "cmd: %c, param: %d, rel: %d", command, param, pm);
	if (pm != 0)
		pm *= param;

	switch (command) {

	case '@':		/* Reset speechd connection */
		LOG(5, "resetting speech dispatcher connection");
		spd_spk_reset(0);
		break;

	case 'b':		/* set punctuation level */
		switch (param) {
		case 0:
			LOG(5, "[punctuation all]");
			ret = spd_set_punctuation(conn, SPD_PUNCT_ALL);
			ret = spd_set_capital_letters(conn, SPD_CAP_SPELL);
			break;
		case 1:
		case 2:
			LOG(5, "[punctuation some]");
			ret = spd_set_punctuation(conn, SPD_PUNCT_SOME);
			break;
		case 3:
			LOG(5, "[punctuation none]");
			ret = spd_set_punctuation(conn, SPD_PUNCT_NONE);
			break;
		default:
			LOG(1, "ERROR: Invalid punctuation mode!");
		}
		if (ret == -1)
			LOG(1, "ERROR: Can't set punctuation mode");
		break;

	case 'o':		/* set voice */
		switch (param) {
		case 0:
			LOG(5, "[Voice MALE1]");
			ret = spd_set_voice_type(conn, SPD_MALE1);
			break;
		case 1:
			LOG(5, "[Voice MALE2]");
			ret = spd_set_voice_type(conn, SPD_MALE2);
			break;
		case 2:
			LOG(5, "[Voice MALE3]");
			ret = spd_set_voice_type(conn, SPD_MALE3);
			break;
		case 3:
			LOG(5, "[Voice FEMALE1]");
			ret = spd_set_voice_type(conn, SPD_FEMALE1);
			break;
		case 4:
			LOG(5, "[Voice FEMALE2]");
			ret = spd_set_voice_type(conn, SPD_FEMALE2);
			break;
		case 5:
			LOG(5, "[Voice FEMALE3]");
			ret = spd_set_voice_type(conn, SPD_FEMALE3);
			break;
		case 6:
			LOG(5, "[Voice CHILD_MALE]");
			ret = spd_set_voice_type(conn, SPD_CHILD_MALE);
			break;
		case 7:
			LOG(5, "[Voice CHILD_FEMALE]");
			ret = spd_set_voice_type(conn, SPD_CHILD_FEMALE);
			break;
		default:
			LOG(1, "ERROR: Invalid voice %d!", param);
			break;
		}
		if (ret == -1)
			LOG(1, "ERROR: Can't set voice!");
		break;

	case 'p':		/* set pitch command */
		if (pm)
			curpitch += pm;
		else
			curpitch = param;
		val = (curpitch - 5) * 20;
		assert((val >= -100) && (val <= +100));
		LOG(5, "[pitch %d, param: %d]", val, param);
		ret = spd_set_voice_pitch(conn, val);
		if (ret == -1)
			LOG(1, "ERROR: Can't set pitch!");
		break;

	case 's':		/* speech rate */
		if (pm)
			currate += pm;
		else
			currate = param;
		val = (currate * 22) - 100;
		assert((val >= -100) && (val <= +100));
		LOG(5, "[rate %d, param: %d]", val, param);
		ret = spd_set_voice_rate(conn, val);
		if (ret == -1)
			LOG(1, "ERROR: Invalid rate!");
		break;

	case 'f':
		LOG(3, "WARNING: [frequency setting not supported,"
		    "use rate instead]");
		break;

	case 'v':
		LOG(3, "[volume setting not supported yet]");
		break;

	case 'x':
		LOG(3, "[tone setting not supported]");
		break;
	default:
		LOG(3, "ERROR: [%c: this command is not supported]", command);
	}
}

/* Say a single character.

UGLY HACK: Since currently this can either be a character
encountered while moving the cursor, a single character generated
by the application, I use the SSIP CHAR command for it. CHAR
generally gives more information and gives better result
for pressing keys. However, there might be some bad side-effects
in the situations while the characters are generated by reding
characters when moving with the cursor. It is currently impossible
to distinguish KEYs from CHARacters in Speakup.
*/
int say_single_character(char *character)
{
	int ret;
	char cmd[13];

	if (!strcmp(character, "\n"))
		return 0;

	LOG(5, "Saying single character: |%s|", character);

	/* It seems there is a bug in some versions of libspeechd
	   in function spd_say_char() */
	snprintf(cmd, 12, "CHAR %s", character);
	ret = spd_execute_command(conn, "SET SELF PRIORITY TEXT");
	if (ret != 0)
		return ret;
	ret = spd_execute_command(conn, cmd);
	if (ret != 0)
		return ret;

	return 0;
}

const char *ssml_less_than = "&lt;";
const char *ssml_greater_than = "&gt;";
const char *ssml_ampersand = "&amp;";
const char *ssml_single_quote = "&apos;";
const char *ssml_double_quote = "&quot;";

const char *ssml_entities[128];
int ssml_entity_lengths[128];

/*
  init_ssml_char_escapes: initialize the tables that describe SSML character
  escapes. */

void init_ssml_char_escapes(void)
{
	int i = 0;
	for (i = 0; i < (sizeof(ssml_entities) / sizeof(char *)); i++) {
		ssml_entity_lengths[i] = 0;
		ssml_entities[i] = NULL;
	}
	ssml_entities['<'] = ssml_less_than;
	ssml_entity_lengths['<'] = strlen(ssml_less_than);
	ssml_entities['>'] = ssml_greater_than;
	ssml_entity_lengths['>'] = strlen(ssml_greater_than);
	ssml_entities['&'] = ssml_ampersand;
	ssml_entity_lengths['&'] = strlen(ssml_ampersand);
	ssml_entities['\''] = ssml_single_quote;
	ssml_entity_lengths['\''] = strlen(ssml_single_quote);
	ssml_entities['\"'] = ssml_double_quote;
	ssml_entity_lengths['\"'] = strlen(ssml_double_quote);
}

/* ssml_escape_text: escape characters that are special to SSML. */
char *ssml_escape_text(char *text)
{
	int old_text_length = strlen(text);
	int escaped_text_length = 0;
	int escape_pos = 0;
	int i;
	char *escaped_text = NULL;

	/*
	   Make two passes over the text.  The first pass determines how much memory
	   to allocate.  The second pass copies text into a newly-allocated buffer,
	   escaping special characters as necessary.  After the second pass finishes,
	   terminate the buffer with a NUL character, and return it to the caller.
	 */

	for (i = 0; i < old_text_length; i++) {
		int current_char = text[i];
		if ((current_char >= 0) && (current_char <= 127) &&
		    (ssml_entity_lengths[current_char] != 0))
			escaped_text_length +=
			    ssml_entity_lengths[current_char];
		else
			escaped_text_length += 1;
	}
	escaped_text = malloc(escaped_text_length + 1);
	if (escaped_text != NULL) {
		for (i = 0; i < old_text_length; i++) {
			int current_char = text[i];
			if ((current_char >= 0) && (current_char <= 127) &&
			    (ssml_entity_lengths[current_char] != 0)) {
				strcpy(&(escaped_text[escape_pos]),
				       ssml_entities[current_char]);
				escape_pos += ssml_entity_lengths[current_char];
			} else
				escaped_text[escape_pos++] = current_char;
		}
		escaped_text[escape_pos] = '\0';
	}

	return escaped_text;
}

char *recode_text(char *text)
{
	iconv_t cd;
	size_t in_bytes, out_bytes, enc_bytes;
	char *utf8_text, *out_p;

	utf8_text = malloc(4 * strlen(text) + 1);
	if (utf8_text == NULL) {
		LOG(1, "ERROR: Charset conversion failed, reason: %s",
		    strerror(errno));
		return NULL;
	}

	out_p = utf8_text;
	out_bytes = 4 * strlen(text);
	in_bytes = strlen(text);

	/* Initialize ICONV for charset conversion */
	cd = iconv_open("utf-8", options.speakup_coding);
	if (cd == (iconv_t) - 1)
		FATAL(1, "Requested character set conversion not possible"
		      "by iconv: %s!", strerror(errno));

	enc_bytes = iconv(cd, &text, &in_bytes, &out_p, &out_bytes);
	if (enc_bytes == -1) {
		LOG(1, "ERROR: Charset conversion failed, reason: %s",
		    strerror(errno));
		utf8_text = NULL;
	} else {
		*out_p = 0;
		LOG(5, "Recoded text: |%s|", utf8_text);
	}

	iconv_close(cd);

	return utf8_text;
}

/*
  speak_string: send a string containing more than one printable character 
  to Speech Dispatcher.  */

int speak_string(char *text)
{
	char *utf8_text = NULL;
	char *escaped_utf8_text = NULL;
	char *ssml_text = NULL;
	int ret = 0;
	utf8_text = recode_text(text);
	if (utf8_text == NULL)
		ret = -1;
	else {
		escaped_utf8_text = ssml_escape_text(utf8_text);

		if (escaped_utf8_text == NULL) {
			LOG(1, "No memory available to hold current string.");
			ret = -1;
		} else {
			int bufsize = strlen(escaped_utf8_text) + 16;
			ssml_text = malloc(bufsize);
			if (ssml_text == NULL)
				ret = -1;
			else {
				snprintf(ssml_text, bufsize,
					 "<speak>%s</speak>",
					 escaped_utf8_text);
				LOG(5, "Sending to speechd as text: |%s|",
				    ssml_text);
				ret = spd_say(conn, SPD_MESSAGE, ssml_text);
			}
		}
	}
	xfree(utf8_text);
	xfree(escaped_utf8_text);
	xfree(ssml_text);
	return ret;
}

int speak(char *text)
{
	/* Check whether text contains more than one
	   printable character. If so, use spd_say,
	   otherwise use say_single_character.

	   Returns 0 on success, -2 on speechd communication
	   errors, -1 on other errors.
	 */

	int printables = 0;
	int i;
	char *utf8_text = NULL;
	int spd_ret = 0, ret = 0;
	char character[2];

	assert(text);
	for (i = 0; i <= strlen(text) - 1; i++) {
		if (!isspace(text[i])) {
			if (printables == 0)
				sprintf(character, "%c", text[i]);
			printables++;
		}
	}

	LOG(5, "Text before recoding: |%s|", text);

	if (printables == 1) {
		utf8_text = recode_text(character);
		LOG(5, "Sending to speechd as character: |%s|", utf8_text);
		spd_ret = say_single_character(utf8_text);
	} else if (printables > 1) {
		spd_ret = speak_string(text);
	}
	/* Else printables is 0, nothing to do. */

	if (spd_ret != 0)
		ret = -2;
	xfree(utf8_text);
	return ret;
}

int parse_buf(char *buf, size_t bytes)
{
	char helper[20];
	char cmd_type = ' ';
	int n, m;
	unsigned int param;
	int pm;

	int i;
	//char *mark_tag="<mark name=\"%s\">";
	char *pi, *po;

	char text[BUF_SIZE];

	assert(bytes <= BUF_SIZE);

	pi = buf;
	po = text;
	m = 0;

	po = text;

	for (i = 0; i < bytes; i++) {
		/* Stop speaking */
		if (buf[i] == DTLK_STOP) {
			spd_cancel(conn);
			LOG(5, "[stop]");
			pi = &(buf[i + 1]);
			po = text;
			m = 0;
		}

		/* Process a command */
		else if (buf[i] == DTLK_CMD) {
			/* If the digit is signed integer, read the sign.  We have
			   to do it this way because +3, -3 and 3 seem to be three
			   different things in this protocol */
			i++;
			if (buf[i] == '+')
				i++, pm = 1;
			else if (buf[i] == '-')
				i++, pm = -1;
			else
				pm = 0;	/* No sign */

			/* Read the numerical parameter (one or more digits) */
			n = 0;
			while (isdigit(buf[i]) && n < 15)
				helper[n++] = buf[i++];
			if (n) {
				helper[n] = 0;
				param = strtol(helper, NULL, 10);
				cmd_type = buf[i];
			}

			if (cmd_type == 'i') {
				LOG(5, "Insert Index %d", param);
				sprintf(helper, "<mark name=\"%d\"/>", param);
				strcpy(po, helper);
				pi = &(buf[i + 1]);
				po += strlen(helper) * sizeof(char);
				*po = 0;
			} else {
				/* If there is some text before this command, say it */
				if (m > 0) {
					*po = '\0';
					LOG(5, "text: |%s|", text);
					LOG(5, "[speaking (2)]");
					speak(text);
					m = 0;
				}
				/* Now when we have the command (cmd_type) and it's
				   parameter, let's communicate it to speechd */
				process_command(cmd_type, param, pm);
				pi = &(buf[i + 1]);
				po = text;
			}
		} else {
			/* This is ordinary text, so put the byte into our text
			   buffer for later synthesis. */
			m++;
			*po = *pi;
			pi += sizeof(char);
			po += sizeof(char);
		}
	}
	*po = 0;

	/* Finally, say the text we read from /dev/softsynth */
	assert(m >= 0);

	if (m != 0) {
		LOG(5, "text: |%s %d|", text, m);
		LOG(5, "[speaking]");
		speak(text);
		LOG(5, "---");
	}

	return 0;
}

void spd_spk_terminate(int sig)
{
	LOG(1, "Terminating...");
	/* TODO: Resolve race  */
	speechd_close();
	close(fd);
	fclose(logfile);
	exit(1);
}

void spd_spk_reset(int sig)
{
	speechd_close();
	speechd_init();
}

int create_pid_file()
{
	FILE *pid_file;
	int pid_fd;
	struct flock lock;
	int ret;

	/* If the file exists, examine it's lock */
	pid_file = fopen(spd_spk_pid_file, "r");
	if (pid_file != NULL) {
		pid_fd = fileno(pid_file);

		lock.l_type = F_WRLCK;
		lock.l_whence = SEEK_SET;
		lock.l_start = 1;
		lock.l_len = 3;

		/* If there is a lock, exit, otherwise remove the old file */
		ret = fcntl(pid_fd, F_GETLK, &lock);
		if (ret == -1) {
			fprintf(stderr,
				"Can't check lock status of an existing pid file.\n");
			return -1;
		}

		fclose(pid_file);
		if (lock.l_type != F_UNLCK) {
			fprintf(stderr, "Speechd-Up already running.\n");
			return -1;
		}

		unlink(spd_spk_pid_file);
	}

	/* Create a new pid file and lock it */
	pid_file = fopen(spd_spk_pid_file, "w");
	if (pid_file == NULL) {
		fprintf(stderr,
			"Can't create pid file in %s, wrong permissions?\n",
			spd_spk_pid_file);
		return -1;
	}
	fprintf(pid_file, "%d\n", getpid());
	fflush(pid_file);

	pid_fd = fileno(pid_file);
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 1;
	lock.l_len = 3;

	ret = fcntl(pid_fd, F_SETLK, &lock);
	if (ret == -1) {
		fprintf(stderr, "Can't set lock on pid file.\n");
		return -1;
	}

	return 0;
}

void destroy_pid_file()
{
	unlink(spd_spk_pid_file);
}

int main(int argc, char *argv[])
{
	size_t chars_read;
	char buf[BUF_SIZE];
	int ret;

	options_set_default();
	options_parse(argc, argv);

	if (!strcmp(PIDPATH, ""))
		spd_spk_pid_file = (char *)strdup("/var/run/speechd-up.pid");
	else
		spd_spk_pid_file = (char *)strdup(PIDPATH "/speechd-up.pid");

	if (create_pid_file() == -1)
		exit(1);

	logfile = stdout;
	load_configuration();

	logfile = fopen(options.log_file_name, "w+");
	if (logfile == NULL) {
		fprintf(stderr,
			"ERROR: Can't open logfile in %s! Wrong permissions?\n",
			options.log_file_name);
		exit(1);
	}

	/* Fork, set uid, chdir, etc. */
	if (options.spd_spk_mode == MODE_DAEMON) {
		daemon(0, 0);
		/* Re-create the pid file under this process */
		destroy_pid_file();
		if (create_pid_file() == -1)
			return 1;
	}

	/* Register signals */
	(void)signal(SIGINT, spd_spk_terminate);
	(void)signal(SIGHUP, spd_spk_reset);

	LOG(1, "Speechd-speakup starts!");

	if (!options.probe_mode) {
		if ((fd = open(options.speakup_device, O_RDWR)) < 0) {
			LOG(1,
			    "Error while openning the device in read/write mode %d,%s",
			    errno, strerror(errno));
			LOG(1, "Trying to open the device in the old way.");
			if ((fd = open(options.speakup_device, O_RDONLY)) < 0) {
				LOG(1,
				    "Error while openning the device in read mode %d,%s",
				    errno, strerror(errno));
				FATAL(2,
				      "ERROR! Unable to open soft synth device (%s)\n",
				      options.speakup_device);
				return -1;
			} else {
				LOG(1,
				    "It seems you are using an older version of Speakup "
				    "that doesn't support index marking. This is not a problem "
				    "but some more advanced functions of Speakup might not work "
				    "until you upgrade Speakup.");
			}

		}
		if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) == -1) {
			FATAL(5, "fcntl() failed");
			return (-1);
		}
	}

	speechd_init();

	if (options.probe_mode) {
		LOG(1,
		    "This is just a probe mode. Not trying to read Speakup's device.\n");
		LOG(1, "Trying to say something on Speech Dispatcher\n");
		spd_say(conn, SPD_MESSAGE,
			"Hello! It seems SpeechD-Up works correctly!\n");
		LOG(1, "Trying to close connection to Speech Dispatcher\n");
		spd_close(conn);
		LOG(1, "SpeechD-Up is terminating correctly in probe mode");
		return 0;
	}

	ret = spd_set_data_mode(conn, SPD_DATA_SSML);
	if (ret) {
		LOG(1,
		    "ERROR: This version of Speech Dispatcher doesn't support SSML mode.\n"
		    "Please use a newer version of Speech Dispatcher (at least 0.5)");
		FATAL(6, "SSML not supported in Speech Dispatcher");
	}

	if (!options.dont_init_tables) {
		ret = init_speakup_tables();
		if (ret) {
			LOG(1,
			    "ERROR: It was not possible to init Speakup /proc tables for\n"
			    "characters and character types."
			    "This error might appear because you use an old version of Speakup!"
			    "If your instalation of Speakup is new:In order for internationalization\n"
			    "or correct Speech Dispatcher support (like sound icons) to be\n"
			    "working, you need to set each entry in /proc/speakup/characters\n"
			    "except for space to its value and each entry in /proc/speakup/chartab"
			    "which represents a valid speakable character to ALPHA.\n");
		}
	}

	while (1) {
		FD_ZERO(&fd_list);
		FD_SET(fd, &fd_list);
		if (select(fd + 1, &fd_list, NULL, NULL, NULL) < 0) {
			if (errno == EINTR)
				continue;
			FATAL(5, "select() failed");
			close(fd);
			return -1;
		}
		chars_read = read(fd, buf, BUF_SIZE);
		if (chars_read < 0) {
			FATAL(5, "read() failed");
			close(fd);
			return -1;
		}
		buf[chars_read] = 0;
		LOG(5, "Main loop characters read = %d : (%s)", chars_read,
		    buf);
		parse_buf(buf, chars_read);
	}

	return 0;
}
