
/*
 * speechd-up.c - Simple interface between SpeakUp and Speech Dispatcher
 *
 * Copyright (C) 2004 Brailcom, o.p.s.
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
 * $Id: speechd-up.c,v 1.3 2004-01-29 00:12:42 hanke Exp $
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>
#include <signal.h>
#include <iconv.h>

#include <libspeechd.h>


#include "options.h"

#define BUF_SIZE 1024

#define DTLK_RESET 7
#define DTLK_STOP 24
#define DTLK_CMD 1

int fd, conn;

FILE *logfile;

char *spd_spk_pid_file;

void spd_spk_reset(int sig);

void
DBG(int level, char *format, ...)
{
  assert(format != NULL);
  assert(logfile != NULL);

  if(level <= LOG_LEVEL) {
    va_list args;
    int i;
		
    va_start(args, format);
    {
      {
	/* Print timestamp */
	time_t t;
	char *tstr;
	t = time(NULL);
	tstr = (char*) strdup(ctime(&t));
	/* Remove the trailing \n */
	assert(strlen(tstr)>1);
	tstr[strlen(tstr)-1] = 0;
	fprintf(logfile, "[%s] speechd: ", tstr);
      }
      for(i=1;i<level;i++){
	fprintf(logfile, " ");
      }
      vfprintf(logfile, format, args);
      fprintf(logfile, "\n");
      fflush(logfile);
    }
    va_end(args);

  }				
}

#define FATAL(status, format...) { DBG(0, format); exit(status); }

void
speechd_init()
{
  conn = spd_open("speakup", "softsynth", NULL);
  if (conn == 0) FATAL(1, "ERROR! Can't connect to Speech Dispatcher!");
}

void
speechd_close()
{
  spd_close(conn);
}

int
process_command(char command, unsigned int param, int pm)
{
  int val;
  int ret;

  switch(command)
    {
    case 's':
      val = (param - 5) * 20;
      assert((val >= -100) && (val <= +100));
      DBG(5, "[rate %d]", val);
      ret = spd_set_voice_rate(conn, val);
      if (ret == -1) perror("ERROR: Invalid rate!");
      break;
    case 'p': 
      if ((pm == 1) && (param == 3))
	{
	  DBG(5, "[capital letters on]", val);
	  spd_set_capital_letters(conn, SPD_CAP_SPELL);
	}
      else if ((pm == -1) && (param == 3))
	{
	  DBG(5, "[capital letters off]", val);
	  spd_set_capital_letters(conn, SPD_CAP_NONE);	  
	}
      else
	{
	  val = (param - 5) * 20;
	  assert((val >= -100) && (val <= +100));
	  DBG(5, "[pitch %d]", val);
	  ret = spd_set_voice_pitch(conn, val);
	  if (ret == -1) DBG(1, "ERROR: Can't set pitch!");
	}
      break;
    case 'v': 
      DBG(3, "[volume setting not supported yet]", val);
      break;
    case 'x': 
      DBG(3, "[tone setting not supported]", val);
      break;
    case 'b': 
      switch(param){
      case 0:
	DBG(5, "[punctuation all]", val);
	ret = spd_set_punctuation(conn, SPD_PUNCT_ALL);
	break;
      case 1:
      case 2:
	DBG(5, "[punctuation some]", val);
	ret = spd_set_punctuation(conn, SPD_PUNCT_SOME);
	break;
      case 3:
	DBG(5, "[punctuation none]", val);
	ret = spd_set_punctuation(conn, SPD_PUNCT_NONE);
	break;
      default: perror("ERROR: Invalid punctuation mode!");
      }
      if (ret == -1) perror("ERROR: Can't set punctuation mode");
      break;
    default:
      DBG(3, "ERROR: [%c: this command is not supported]", command);
    }

}

int
parse_buf(char *buf, size_t bytes)
{ 
  char helper[16];
  char cmd_type;
  int  n, m;
  unsigned int param;
  int pm;
  int ret;

  iconv_t cd;
  int i;
  int enc_bytes;
  char *pi, *po;
  int bytes_left = BUF_SIZE;
  int in_bytes;

  char text[BUF_SIZE];

  assert (bytes <= BUF_SIZE);

  cd = iconv_open("utf-8", SPEAKUP_CODING);
  if (cd == (iconv_t) -1) FATAL(1, "Requested character set conversion not possible"
			"by iconv: %s!", strerror(errno));

  pi = buf;
  po = text;
  m = 0;

  for(i = 0; i < bytes - 1; i++)
    {

      /* Reset speechd connection */
      if (buf[i] == DTLK_RESET){
	spd_spk_reset(0);
	return 0;
      }
 
      /* Stop speaking */
      else if (buf[i] == DTLK_STOP)
	{
	  DBG(5, "[stop]");
	  spd_stop(conn);
	  pi = &(buf[i+1]);
	  po = text;
	}

      /* Process a command */
      else if (buf[i] == DTLK_CMD)
	{

	  /* If there is some text before this command, say it */
	  if (m > 0)
	    {	     
	      DBG(5, "text: |%s|", text);
	      DBG(5, "[speaking (2)]");
	      spd_say(conn, SPD_TEXT, text);
	      m = 0;
	    }
      
	  /* If the digit is signed integer, read the sign.  We have
	     to do it this way because +3, -3 and 3 seem to be three
	     different thinks in this protocol */
	  i++;
	  if (buf[i] == '+') i++, pm = 1;
	  else if (buf[i] == '-') i++, pm = -1;
	  else pm = 0;		/* No sign */

	  /* Read the numerical parameter (one or more digits) */
	  n = 0;
	  for (i; (i < bytes - 1) && (n<15); i++)
	    {
	      if (!isdigit(buf[i]))
		{
		  helper[n] = 0;
		  cmd_type = buf[i];
		  break;
		}
	      helper[n++] = buf[i];	  
	    }
	  param = strtol(helper, NULL, 10);
	
	  /* Now when we have the command (cmd_type) and it's
	     parameter, let's communicate it to speechd */
	  process_command(cmd_type, param, pm);
	  pi = &(buf[i+1]);
	  po = text;
	}
      else
	{
	  /* This is ordinary text, so put the byte into our text
	     buffer for later synthesis. */
	  in_bytes = 1;
	  enc_bytes = iconv(cd, &pi, &in_bytes, &po, &bytes_left);
	  if (enc_bytes == -1) DBG(1,"unknown character in input"); /*;*/
	  else{
	    m++;
	    *po = 0;
	  }	  
	}
    }

  iconv_close(cd);

  /* Finally, say the text we read from /dev/softsynth*/
  assert(m>=0);
  
  if (m != 0){
    DBG(5,"text: |%s %d|", text, m);
    DBG(5, "[speaking]");
    // text[m] = 0;
    spd_say(conn, SPD_TEXT, text);
    DBG(5,"---");
  }

  return 0;
}

void
spd_spk_terminate(int sig)
{
  DBG(1, "Terminating...");
  /* TODO: Resolve race  */
  speechd_close();
  close(fd);
  fclose(logfile);
  exit(1);
}

void
spd_spk_reset(int sig)
{
  speechd_close();
  speechd_init();
}

int
create_pid_file()
{
    FILE *pid_file;
    int pid_fd;
    struct flock lock;
    int ret;    
      
    /* If the file exists, examine it's lock */
    pid_file = fopen(spd_spk_pid_file, "r");
    if (pid_file != NULL){
        pid_fd = fileno(pid_file);

        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = 1;
        lock.l_len = 3;

        /* If there is a lock, exit, otherwise remove the old file */
        ret = fcntl(pid_fd, F_GETLK, &lock);
        if (ret == -1){
            fprintf(stderr, "Can't check lock status of an existing pid file.\n");
            return -1;
        }

        fclose(pid_file);
        if (lock.l_type != F_UNLCK){
            fprintf(stderr, "Speechd-Up already running.\n");
            return -1;
        }

        unlink(spd_spk_pid_file);        
    }    
    
    /* Create a new pid file and lock it */
    pid_file = fopen(spd_spk_pid_file, "w");
    if (pid_file == NULL){
        fprintf(stderr, "Can't create pid file in %s, wrong permissions?\n",
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
    if (ret == -1){
        fprintf(stderr, "Can't set lock on pid file.\n");
        return -1;
    }

    return 0;
}

void
destroy_pid_file()
{
    unlink(spd_spk_pid_file);
}


int
main (int argc, char *argv[])
{
  int i;
  size_t chars_read;
  unsigned char buf[BUF_SIZE];
  fd_set fd_list;

  options_set_default();
  options_parse(argc, argv);

  if (!strcmp(PIDPATH, ""))
    spd_spk_pid_file = (char*) strdup("/var/run/speechd-up.pid");
  else
    spd_spk_pid_file = (char*) strdup(PIDPATH"speechd.pid");
  
  if (create_pid_file() == -1) exit(1);


  logfile = fopen(LOG_FILE_NAME, "w+");
  if (logfile == NULL){
    fprintf(stderr, "ERROR: Can't open logfile in %s! Wrong permissions?\n", LOG_FILE_NAME);
    exit(1);
  }

  /* Fork, set uid, chdir, etc. */
  if (SPD_SPK_MODE == MODE_DAEMON){
    daemon(0,0);	   
    /* Re-create the pid file under this process */
    destroy_pid_file();
    if (create_pid_file() == -1) return 1;
  }

  /* Register signals */
  (void) signal(SIGINT, spd_spk_terminate);	
  (void) signal(SIGHUP, spd_spk_reset);

  DBG(1,"Speechd-speakup starts!");

  if ((fd = open(SPEAKUP_DEVICE, O_RDONLY)) < 0) {
    FATAL(2, "ERROR! Unable to open soft synth device (%s)\n", SPEAKUP_DEVICE);
    return -1;
  }
  if ( fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) == -1) {
    FATAL(5, "fcntl() failed");
    return (-1);
  }

  speechd_init();

  while (1){
    FD_ZERO(&fd_list);
    FD_SET(fd, &fd_list);
    if ( select(fd+1, &fd_list, NULL, NULL, NULL) < 0 ) {
      if (errno == EINTR) continue;
      FATAL(5, "select() failed");
      close (fd);
      return -1;
    }
    chars_read = read(fd, buf, BUF_SIZE);
    if (chars_read < 0) {
      FATAL(5, "read() failed");
      close (fd);
      return -1;
    }
    buf[chars_read-1] = 0;

    parse_buf(buf, chars_read);
  }

  return 0;
}
 

