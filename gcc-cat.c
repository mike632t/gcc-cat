/*
 * cat.c
 *
 * Copyright(C) 2019 - MT
 *
 * A not quite so minimal implementation of the ubiquitous 'cat'.
 *
 * Implements a subset of the GNU 'cat' functionality.
 * 
 * Deliberatly parses the command line without using 'getopt' or 'argparse'
 * to maximize portability.
 *
 * This  program is free software: you can redistribute it and/or modify it
 * under  the terms of the GNU General Public License as published  by  the
 * Free  Software Foundation, either version 3 of the License, or (at  your
 * option) any later version.
 *
 * This  program  is distributed in the hope that it will  be  useful,  but
 * WITHOUT   ANY   WARRANTY;   without even   the   implied   warranty   of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You  should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * 03 Mar 14   0.1   - Initial version - MT
 * 04 Apr 18   0.2   - Added some command line options including the option
 *                     to restart line numbering at the start of every file
 *                     add a delay when printing each character - MT
 * 08 Mar 19   0.3   - Removed  delay  option and rewrote argument  parsing
 *                     code to comply with the ANSI C standard - MT
 *                   - Changed flags to boolean values - MT
 *                   - Does NOT insert a newline between files - MT
 * 10 Mar 19         - Added  an  option to print the filename as a  header
 *                     before displaying the file - MT
 *                   - Allows  standard input to be specified as the  input
 *                     stream, implementing this change involved moving the
 *                     code  to  display the contents of each file  into  a
 *                     seperate routine - MT
 *                   - Implemented conditional compilation options for unix
 *                     and other platforms so delimiter for the the command
 *                     line options is correct - MT
 * 12 Mar 19         - Defined boolean values as integers and changed error
 *                     message  on  file open to allow code to be  compiled
 *                     with older compilers - MT
 *                   - Removed conditional compilation options - MT
 *                   - Fixed  bug  in option parsing routine to allow  more
 *                     than  one  option  to  be  specified  in  a   single
 *                     parameter - MT
 *                   - Prints header after the fle is opened - MT
 *                   - Updated description - MT
 * 14 Mar 19         - Uses  errno to print error message (it is  necessary
 *                     to  assign errno to a local vairable before  calling
 *                     strerror(), don't really know why but this does seem
 *                     to depend on the compiler!) - MT
 * 16 Mar 19         - Fixed a bug in the option parser that any characters
 *                     immediatly  following the '--' were ignored and were
 *                     not treated as an invalid option - MT
 *                   - Mofified DEBUG macro - MT
 *                   - Option delimiter no longer defined in a macro - MT
 *             0.4   - Added the capability to parse long options including
 *                     those that are only partly complete and used this to
 *                     add the ability to display the program version using
 *                     a macro to return the copyright year - MT
 *             0.5   - Added  the long versions of each command line option
 *                     including '--version' and '--help' and modified  the
 *                     error  messages  to suggest using '--help'  when  an
 *                     invalid command line option is specified - MT
 *                   - Removed '--verbose' option - MT
 *                   - Removed DATE macro - MT
 * 29 Mar 20   0.6   - Added a boolean type defination and defined true and
 *                     false - MEJT
 * 03 Jul 20   0.7   - Tidied up formatting - MT
 *                   - Changed progran name to use defined text - MT
 *                   - Added  seperate routines to display program  version
 *                     and help text - MT
 * 09 Jul 20   0.8   - Now uses fread() instead of fgetc() which results in
 *                     a four or five fold inrease in performance - MT
 *                   - Moved code to print contents of buffer to a seperate
 *                     function  (depends on glabal vairables to keep track
 *                     of line number, blank lines, and the last  character
 *                     printed - MT
 * 11 Jul 20   0.9   - Checks that the path is not a directory - MT
 * 25 Jul 20         - Can  now  parse  Microsoft/DEC  style  command  line
 *                     options (allows partial completion) - MT
 * 30 Jul 20         - Conditionally  uses VMS specific version of strerror
 *                     (avoids access violation on VAX-C) - MT
 * 31 Jul 20         - Renamed  local functions to avoid  naming  conflicts
 *                     with builtin functions - MT
 *                   - Reinstated the option to insert a delay between each
 *                     character using either a generic busy wait loop or a
 *                     platform specific routine for linux and VMS - MT
 * 03 Aug 20   0.10  - Now uses the windows Sleep() function instead of the
 *                     generic 'busy wait' in i_wait().
 *                   - Moved all the preprocessor include directives to the
 *                     top and tidied up the conditional code blocks  since
 *                     the  code  would not compile using Visual C  6.0  if
 *                     'windows.h'was included in 'i_wait()' - MT
 *                   - Added  a  timestamp to the version  information  and 
 *                     removed  the copyright macro and just used  __DATE__
 *                     instead - MT
 * 24 Aug 20         - Added type prefixes to vairable names - MT
 * 21 Sep 20   0.11  - Fixed  bug in argument parser caused by an undefined
 *                     value for i_abort on the first pass through the loop
 *                     (only affected Tru64 UNIX) - MT
 * 23 Nov 23   0.12  - Fixed  another bug in argument parser that cuased  a
 *                     segfault if an invalid long argument was included on
 *                     the command line - MT
 *                   - Fixed about() and version() to use stdout - MT
 *
 * To Do:            - Default to copying standard input to standard output
 *                     if no arguments are specified on the command line.
 *                   - Add delay between digits when printing line numbers.
 */

#define NAME         "cat"
#define VERSION      "0.11"
#define BUILD        "0037"
#define AUTHOR       "MT"

#define true         1
#define false        0

#define BUFFER_SIZE  512

#include <stdio.h>
#include <stdlib.h>  /* exit */
#include <stdarg.h>
#include <string.h>
#include <errno.h>
 
#if defined(linux)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#elif defined(WIN32)
#include <windows.h>
#include <sys/stat.h>
#elif defined(VMS)
#include <stat.h>
#include <timeb.h>
#include <lib$routines.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#endif
 
char b_bflag, b_hflag, b_nflag, b_rflag, b_sflag, b_bflag = false;
char c_last; /* Last character read, used to check for a blank lines */
int i_line;  /* Current line number */
int i_blanks;/* Number of successive blank lines */
 
void v_version() { /* Display version information */
   fprintf(stdout, "%s: Version %s ", NAME, VERSION);
   if (__DATE__[4] == ' ') fprintf(stdout, "(0"); else fprintf(stdout, "(%c", __DATE__[4]);
   fprintf(stdout, "%c %c%c%c %s %s)", __DATE__[5], __DATE__[0], __DATE__[1], __DATE__[2], &__DATE__[9], __TIME__ );
   fprintf(stdout,"\n");
   fprintf(stdout, "Copyright(C) %s %s\n", __DATE__ +7, AUTHOR);
   fprintf(stdout, "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n");
   fprintf(stdout, "This is free software: you are free to change and redistribute it.\n");
   fprintf(stdout, "There is NO WARRANTY, to the extent permitted by law.\n");
   exit(0);
}
 
#if defined(VMS) || defined(MSDOS) || defined (WIN32) /* Use DEC/Microsoft command line options */
   void v_about() { /* Display help text */
      fprintf(stdout, "Usage: %s [OPTION]... [FILE]...\n", NAME);
      fprintf(stdout, "Concatenate FILE(s)to standard output.\n\n");
      fprintf(stdout, "  /delay                   delay 8ms between each byte\n");
      fprintf(stdout, "  /header                  display filenames\n");
      fprintf(stdout, "  /number                  number all output lines \n");
      fprintf(stdout, "  /restart                 line numbers start at zero, implies -n\n");
      fprintf(stdout, "  /skip                    skip over repeated blank lines\n");
      fprintf(stdout, "  /version                 output version information and exit\n\n");
      fprintf(stdout, "  /?, /help                display this help and exit\n");
      exit(0);
   }
#else
   void v_about() { /* Display help text */
      fprintf(stdout, "Usage: %s [OPTION]... [FILE]...\n", NAME);
      fprintf(stdout, "Concatenate FILE(s)to standard output.\n\n");
      fprintf(stdout, "  -d, --delay              delay 8ms between each byte\n");
      fprintf(stdout, "  -f, --filenames          display filenames\n");
      fprintf(stdout, "  -n, --number             number all output lines \n");
      fprintf(stdout, "  -r, --restart            line numbers start at zero, implies -n\n");
      fprintf(stdout, "  -s, --squeeze-blank      suppress repeated blank lines\n");
      fprintf(stdout, "  -?, --help               display this help and exit\n");
      fprintf(stdout, "      --version            output version information and exit\n\n");
      fprintf(stdout, "With no FILE, or when FILE is -, read standard input.\n");
      exit(0);
   }
#endif
 
void v_error(const char *s_fmt, ...) { /* Print formatted error message */
   va_list t_args;
   va_start(t_args, s_fmt);
   fprintf(stderr, "%s : ", NAME);
   vfprintf(stderr, s_fmt, t_args);
   va_end(t_args);
   exit(-1);
}
 
int i_wait(long l_delay) { /* wait for milliseconds */
#if defined(linux)/* Use usleep() function */
   return (usleep(l_delay * 1000));
#elif defined(WIN32) /* Use usleep() function */
   Sleep(l_delay);
   return (0);
#elif defined(VMS) /* Use VMS LIB$WAIT */
   float f_seconds;
   f_seconds = l_delay / 1000.0;
   return (lib$wait(&f_seconds));
#else /* Use a portable but very inefficent busy loop */
   struct timeb t_start, t_end;
   ftime(&t_start);
   ftime(&t_end);
   while ((1000 * (t_end.time - t_start.time) + t_end.millitm - t_start.millitm) < l_delay) {
      ftime(&t_end);
   }
   return(0);
#endif
}

int i_isfile(char *s_name) {
   struct stat t_file_d;
      stat(s_name, &t_file_d);
   return ((t_file_d.st_mode & S_IFMT) == S_IFREG);
}
 
int i_isdir(char *s_name) {
   struct stat t_file_d;
   stat(s_name, &t_file_d);
   return ((t_file_d.st_mode & S_IFMT) == S_IFDIR);
}
 
int i_fprintbuf (FILE *h_file, int i_size, char a_buffer[]) {
   int i_index;
   for (i_index = 0; i_index < i_size; i_index++) {
      if ((c_last == '\n') && (c_last == a_buffer[i_index])) {
         i_blanks++; /* Count consecutive belank lines */
      } else {
         i_blanks = 0;
      }
      if ((!b_sflag) || (i_blanks < 2)) {
         if ((c_last == '\n') && !(a_buffer[i_index] == '\n' && b_bflag)) {
            if (b_nflag) {
               fprintf(h_file, "%6d\t", i_line); /* Print line number */
            }
            i_line++;
         }
         fprintf(h_file, "%c", a_buffer[i_index]);
         if (b_bflag > 0) { /* Optionally add a delay between characters. */
            fflush(h_file);
            i_wait(8); /* An 8 ms delay equates to approximately 2400 baud */
         }
      }
      c_last = a_buffer[i_index]; /* Remember the last character */
   }
   return(fflush(h_file)); /* Return status from fflush() */
}

int main(int argc, char **argv) {
   FILE *h_file;
   char a_buffer[BUFFER_SIZE];
   int i_bytes; /* Number of bytes read from file */
   int i_size;  /* Number of bytes read into the buffer */
   int i_count, i_index;
#if defined(VMS) || defined(MSDOS) || defined (WIN32) /* Parse DEC/Microsoft style command line options */
   for (i_count = 1; i_count < argc; i_count++) {
      if (argv[i_count][0] == '/') {
         for (i_index = 0; argv[i_count][i_index]; i_index++) /* Convert option to uppercase */
            if (argv[i_count][i_index] >= 'a' && argv[i_count][i_index] <= 'z')
               argv[i_count][i_index] = argv[i_count][i_index] - 32;
         if (!strncmp(argv[i_count], "/VERSION", i_index)) {
            v_version(); /* Display version information */
         } else if (!strncmp(argv[i_count], "/DELAY", i_index)) {
            b_bflag = true;
         } else if (!strncmp(argv[i_count], "/NUMBER", i_index)) {
            b_nflag = true;
         } else if (!strncmp(argv[i_count], "/IGNORE", i_index)) {
            b_nflag = true; b_bflag = true;
         } else if (!strncmp(argv[i_count], "/SKIP", i_index)) {
            b_sflag = true;
         } else if (!strncmp(argv[i_count], "/RESTART", i_index)) {
            b_rflag = true;
         } else if (!strncmp(argv[i_count], "/HEADER", i_index)) {
            if (strlen(argv[i_count]) < 4) { /* Check option is not ambigious */
               v_error("option '%s' is ambiguous; please specify '/HEADER' or '/HELP'.\n", argv[i_count]);
            }
            b_hflag = true;
         } else if (!strncmp(argv[i_count], "/HELP", i_index)) {
            v_about();
         } else if (!strncmp(argv[i_count], "/?", i_index)) {
            v_about();
         } else { /* If we get here then the we have an invalid option */
            v_error("invalid option %s\nTry '%s /help' for more information.\n", argv[i_count] , NAME);
         }
         if (argv[i_count][1] != 0) {
            for (i_index = i_count; i_index < argc - 1; i_index++) argv[i_index] = argv[i_index + 1];
            argc--; i_count--;
         }
      }
   }
#else /* Parse UNIX style command line options */
   char b_abort = false; /* Stop processing command line */
   for (i_count = 1; i_count < argc && (b_abort != true); i_count++) {
      if (argv[i_count][0] == '-') {
         i_index = 1;
         while (argv[i_count][i_index] != 0) {
            switch (argv[i_count][i_index]) {
            case 'b': /* Number non empty lines */
               b_nflag = true; b_bflag = true; break;
            case 'd': /* Wait between printing characters  */
               b_bflag = true; break;
            case 'f': /* Print filenames headings */
               b_hflag = true; break;
            case 'n': /* Number lines */
               b_nflag = true; break;
            case 'r': /* Restart numbering */
               b_rflag = true; b_nflag = true; break;
            case 's': /* Squeeze blank lines */
               b_sflag = true; break;
            case '?': /* Display help */
               v_about();
            case '-': /* '--' terminates command line processing */
               i_index = strlen(argv[i_count]);
               if (i_index == 2)
                 b_abort = true; /* '--' terminates command line processing */
               else
                  if (!strncmp(argv[i_count], "--version", i_index)) {
                     v_version(); /* Display version information */
                  } else if (!strncmp(argv[i_count], "--delay", i_index)) {
                     b_bflag = true;
                  } else if (!strncmp(argv[i_count], "--number", i_index)) {
                     b_nflag = true;
                  } else if (!strncmp(argv[i_count], "--number-nonblank", i_index)) {
                     b_nflag = true; b_bflag = true;
                  } else if (!strncmp(argv[i_count], "--squeeze-blank", i_index)) {
                     if (strlen(argv[i_count]) < 4) { /* Check option is not ambigious */
                        v_error("option '%s' is ambiguous; please specify '--squeeze-blank' or '--show-filenames'.\n", argv[i_count]);
                     }
                     b_sflag = true;
                  } else if (!strncmp(argv[i_count], "--restart-numbering", i_index)) {
                     b_rflag = true;
                  } else if (!strncmp(argv[i_count], "--show-filenames", i_index)) {
                     b_hflag = true;
                  } else if (!strncmp(argv[i_count], "--help", i_index)) {
                     v_about();
                  } else { /* If we get here then the we have an invalid long option */
                     v_error("invalid option %s\nTry '%s --help' for more information.\n", argv[i_count], NAME);
                  }
               i_index--; /* Leave index pointing at end of string (so argv[i_count][i_index] = 0) */
               break;
            default: /* If we get here the single letter option is unknown */
               v_error("unknown option -- %c\nTry '%s --help' for more information.\n", argv[i_count][i_index] , NAME);
            }
            i_index++; /* Parse next letter in options */
         }
         if (argv[i_count][1] != 0) {
            for (i_index = i_count; i_index < argc - 1; i_index++) argv[i_index] = argv[i_index + 1];
            argc--; i_count--;
         }
      }
   }
#endif
   i_line = 1;
   for (i_count = 1; i_count < argc; i_count++) { /* Display each file */
      i_bytes = 0;
      if (i_isdir(argv[i_count])) { /* Check that argument is not a directory */
         v_error("%s: %s\n", argv[i_count], strerror(21));
      } else {
         if ((h_file = fopen(argv[i_count], "r")) != NULL) {
            while((i_size = fread(a_buffer, 1, BUFFER_SIZE, h_file)) > 0 ){
               if (i_bytes == 0) {
                  if (b_hflag) fprintf(stdout, "%s:\n", argv[i_count]); /* Optionally print filename */
                  if (b_rflag) i_line = 1; /* Optionally reset line numbers */
                  c_last = '\n';
               }
               i_bytes += i_size;
               i_fprintbuf (stdout, i_size, a_buffer); /* Print buffer */
            }
            fclose(h_file);
         } else {
#if defined(VMS) /* Use VAX-C extension (avoids potential ACCVIO) */
             v_error("%s: %s\n", argv[i_count], strerror(errno, vaxc$errno)); 
#else
             v_error("%s: %s\n", argv[i_count], strerror(errno));
#endif
         }
      }
   }
   exit (0);
}
