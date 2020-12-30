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
 * 29 Mar 20   0.6   - Added a boolean type definition and defined true and
 *                     false - MEJT
 * 03 Jul 20   0.7   - Tidied up formatting - MT
 *                   - Changed program name to use defined text - MT
 *                   - Added  separate routines to display program  version
 *                     and help text - MT
 * 09 Jul 20   0.8   - Now uses fread() instead of fgetc() which results in
 *                     a four or five fold increase in performance - MT
 *                   - Moved code to print contents of buffer to a separate
 *                     function  (depends on global variables to keep track
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
 *                     generic 'busy wait' in _wait().
 *                   - Moved all the preprocessor include directives to the
 *                     top and tidied up the conditional code blocks  since
 *                     the  code  would not compile using Visual C  6.0  if
 *                     'windows.h'was included in '_wait()' - MT
 *                   - Added  a  timestamp to the version  information  and 
 *                     removed  the copyright macro and just used  __DATE__
 *                     instead - MT
 *
 * To Do:            - Default to copying standard input to standard output
 *                     if no arguments are specified on the command line.
 *                   - Add delay between digits when printing line numbers.
 */

#define NAME         "cat"
#define VERSION      "0.10"
#define BUILD        "0036"
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
 
char _bflag, _hflag, _nflag, _rflag, _sflag, _dflag = false;
char _last; /* Last character read, used to check for a blank lines */
int _line;  /* Current line number */
int _blanks;/* Number of successive blank lines */
 
void _version() { /* Display version information */
   fprintf(stderr, "%s: Version %s", NAME, VERSION);
   fprintf(stderr, " (%c%c %c%c%c %s %s)", __DATE__[4], __DATE__[5],
      __DATE__[0], __DATE__[1], __DATE__[2], __DATE__ +9, __TIME__ );
   fprintf(stderr,"\n");
   fprintf(stderr, "Copyright(C) %s %s\n", __DATE__ +7, AUTHOR);
   fprintf(stderr, "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n");
   fprintf(stderr, "This is free software: you are free to change and redistribute it.\n");
   fprintf(stderr, "There is NO WARRANTY, to the extent permitted by law.\n");
   exit(0);
}
 
#if defined(VMS) || defined(MSDOS) || defined (WIN32) /* Use DEC/Microsoft command line options */
   void _about() { /* Display help text */
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
   void _about() { /* Display help text */
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
 
void _error(const char *_fmt, ...) { /* Print formatted error message */
   va_list _args;
   va_start(_args, _fmt);
   fprintf(stderr, "%s : ", NAME);
   vfprintf(stderr, _fmt, _args);
   va_end(_args);
}
 
int _wait(long _delay) { /* wait for milliseconds */
#if defined(linux)/* Use usleep() function */
   return (usleep(_delay * 1000));
#elif defined(WIN32) /* Use usleep() function */
   Sleep(_delay);
   return (0);
#elif defined(VMS) /* Use VMS LIB$WAIT */
   float _seconds;
   _seconds = _delay / 1000.0;
   return (lib$wait(&_seconds));
#else /* Use a portable but very inefficent busy loop */
   struct timeb _start, _end;
   ftime(&_start);
   ftime(&_end);
   while ((1000 * (_end.time - _start.time) + _end.millitm - _start.millitm) < _delay) {
      ftime(&_end);
   }
   printf(".");
   return(0);
#endif
}

int _isfile(char *_name) {
   struct stat _file_d;
      stat(_name, &_file_d);
   return ((_file_d.st_mode & S_IFMT) == S_IFREG);
}
 
int _isdir(char *_name) {
   struct stat _file_d;
   stat(_name, &_file_d);
   return ((_file_d.st_mode & S_IFMT) == S_IFDIR);
}
 
int _fprintbuf (FILE *_file, int _size, char _buffer[]) {
   int _index;
   for (_index = 0; _index < _size; _index++) {
      if ((_last == '\n') && (_last == _buffer[_index])) {
         _blanks++; /* Count consecutive belank lines */
      } else {
         _blanks = 0;
      }
      if ((!_sflag) || (_blanks < 2)) {
         if ((_last == '\n') && !(_buffer[_index] == '\n' && _bflag)) {
            if (_nflag) {
               fprintf(_file, "%6d\t", _line); /* Print line number */
            }
            _line++;
         }
         fprintf(_file, "%c", _buffer[_index]);
         if (_dflag > 0) { /* Optionally add a delay between characters. */
            fflush(_file);
            _wait(8); /* An 8 ms delay equates to approximately 2400 baud */
         }
      }
      _last = _buffer[_index]; /* Remember the last character */
   }
   return(fflush(_file)); /* Return status from fflush() */
}

int main(int argc, char **argv) {
   FILE *file;
   char _buffer[BUFFER_SIZE];
   int _bytes; /* Number of bytes read from file */
   int _size;  /* Number of bytes read into the buffer */
   int _count, _index;
#if defined(VMS) || defined(MSDOS) || defined (WIN32) /* Parse DEC/Microsoft style command line options */
   for (_count = 1; _count < argc; _count++) {
      if (argv[_count][0] == '/') {
         for (_index = 0; argv[_count][_index]; _index++) /* Convert option to uppercase */
            if (argv[_count][_index] >= 'a' && argv[_count][_index] <= 'z')
               argv[_count][_index] = argv[_count][_index] - 32;
         if (!strncmp(argv[_count], "/VERSION", _index)) {
            _version(); /* Display version information */
         } else if (!strncmp(argv[_count], "/DELAY", _index)) {
            _dflag = true;
         } else if (!strncmp(argv[_count], "/NUMBER", _index)) {
            _nflag = true;
         } else if (!strncmp(argv[_count], "/IGNORE", _index)) {
            _nflag = true; _bflag = true;
         } else if (!strncmp(argv[_count], "/SKIP", _index)) {
            _sflag = true;
         } else if (!strncmp(argv[_count], "/RESTART", _index)) {
            _rflag = true;
         } else if (!strncmp(argv[_count], "/HEADER", _index)) {
            if (strlen(argv[_count]) < 4) { /* Check option is not ambigious */
               _error("option '%s' is ambiguous; please specify '/HEADER' or '/HELP'.\n", argv[_count]);
               exit(-1);
            }
            _hflag = true;
         } else if (!strncmp(argv[_count], "/HELP", _index)) {
            _about();
         } else if (!strncmp(argv[_count], "/?", _index)) {
            _about();
         } else { /* If we get here then the we have an invalid option */
            _error("invalid option %s\nTry '%s /help' for more information.\n", argv[_count] , NAME);
            exit(-1);
         }
         if (argv[_count][1] != 0) {
            for (_index = _count; _index < argc - 1; _index++) argv[_index] = argv[_index + 1];
            argc--; _count--;
         }
      }
   }
#else /* Parse UNIX style command line options */
   int _abort; /* Stop processing command line */
   for (_count = 1; _count < argc && (_abort != true); _count++) {
      if (argv[_count][0] == '-') {
         _index = 1;
         while (argv[_count][_index] != 0) {
            switch (argv[_count][_index]) {
            case 'b': /* Number non empty lines */
               _nflag = true; _bflag = true; break;
            case 'd': /* Wait between printing characters  */
               _dflag = true; break;
            case 'f': /* Print filenames headings */
               _hflag = true; break;
            case 'n': /* Number lines */
               _nflag = true; break;
            case 'r': /* Restart numbering */
               _rflag = true; _nflag = true; break;
            case 's': /* Squeeze blank lines */
               _sflag = true; break;
            case '?': /* Display help */
               _about();
            case '-': /* '--' terminates command line processing */
               _index = strlen(argv[_count]);
               if (_index == 2)
                 _abort = true; /* '--' terminates command line processing */
               else
                  if (!strncmp(argv[_count], "--version", _index)) {
                     _version(); /* Display version information */
                  } else if (!strncmp(argv[_count], "--delay", _index)) {
                     _dflag = true;
                  } else if (!strncmp(argv[_count], "--number", _index)) {
                     _nflag = true;
                  } else if (!strncmp(argv[_count], "--number-nonblank", _index)) {
                     _nflag = true; _bflag = true;
                  } else if (!strncmp(argv[_count], "--squeeze-blank", _index)) {
                     if (strlen(argv[_count]) < 4) { /* Check option is not ambigious */
                        _error("option '%s' is ambiguous; please specify '--squeeze-blank' or '--show-filenames'.\n", argv[_count]);
                        exit(-1);
                     }
                     _sflag = true;
                  } else if (!strncmp(argv[_count], "--restart-numbering", _index)) {
                     _rflag = true;
                  } else if (!strncmp(argv[_count], "--show-filenames", _index)) {
                     _hflag = true;
                  } else if (!strncmp(argv[_count], "--help", _index)) {
                     _about();
                  } else { /* If we get here then the we have an invalid long option */
                     _error("%s: invalid option %s\nTry '%s --help' for more information.\n", argv[_count][_index] , NAME);
                     exit(-1);
                  }
               _index--; /* Leave index pointing at end of string (so argv[_count][_index] = 0) */
               break;
            default: /* If we get here the single letter option is unknown */
               _error("unknown option -- %c\nTry '%s --help' for more information.\n", argv[_count][_index] , NAME);
               exit(-1);
            }
            _index++; /* Parse next letter in options */
         }
         if (argv[_count][1] != 0) {
            for (_index = _count; _index < argc - 1; _index++) argv[_index] = argv[_index + 1];
            argc--; _count--;
         }
      }
   }
#endif
   _line = 1;
   for (_count = 1; _count < argc; _count++) { /* Display each file */
      _bytes = 0;
      if (_isdir(argv[_count])) { /* Check that argument is not a directory */
         _error("%s: %s\n", argv[_count], strerror(21));
      } else {
         if ((file = fopen(argv[_count], "r")) != NULL) {
            while((_size = fread(_buffer, 1, BUFFER_SIZE, file)) > 0 ){
               if (_bytes == 0) {
                  if (_hflag) fprintf(stdout, "%s:\n", argv[_count]); /* Optionally print filename */
                  if (_rflag) _line = 1; /* Optionally reset line numbers */
                  _last = '\n';
               }
               _bytes += _size;
               _fprintbuf (stdout, _size, _buffer); /* Print buffer */
            }
            fclose(file);
         } else {
#if defined(VMS) /* Use VAX-C extension (avoids potential ACCVIO) */
             _error("%s: %s\n", argv[_count], strerror(errno, vaxc$errno)); 
#else
             _error("%s: %s\n", argv[_count], strerror(errno));
#endif
         }
      }
   }
   exit (0);
}
