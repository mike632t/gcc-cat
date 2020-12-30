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
 *
 * To Do:            - Default to copying standard input to standard output
 *                     if no arguments are specified on the command line.
 *                   - Warn user if a command line option is ambiguous.
 *                   - Check that source is a valid file...
*/
 
#define NAME         "cat"
#define VERSION      "0.8"
#define BUILD        "0028"
#define AUTHOR       "MT"
#define COPYRIGHT    (__DATE__ +7) /* Extract copyright year from date */
 
#define true         1
#define false        0
 
#define BUFFER_SIZE  16
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
 
char _bflag, _hflag, _nflag, _rflag, _sflag;
char _last; /* Last character read, used to check for a blank lines */
int _line;  /* Current line number */
int _blanks;/* Number of successive blank lines */ 
 
void about() { /* Display help text */
   fprintf(stdout, "Usage: %s [OPTION]... [FILE]...\n", NAME);
   fprintf(stdout, "Concatenate FILE(s)to standard output.\n\n");
   fprintf(stdout, "  -f, --filenames          display filenames\n");
   fprintf(stdout, "  -n, --number             number all output lines \n");
   fprintf(stdout, "  -r, --restart            line numbers start at zero, implies -n\n");
   fprintf(stdout, "  -s, --squeeze-blank      suppress repeated empty output lines\n");
   fprintf(stdout, "      --verbose            display verbose output\n");
   fprintf(stdout, "  -?, --help               display this help and exit\n");
   fprintf(stdout, "      --version            output version information and exit\n\n");
   fprintf(stdout, "With no FILE, or when FILE is -, read standard input.\n");
   exit(0);
}
 
void version() { /* Display version information */
   fprintf(stderr, "%s: Version %s\n", NAME, VERSION);
   fprintf(stderr, "Copyright(C) %s %s\n", COPYRIGHT, AUTHOR);
   fprintf(stderr, "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n");
   fprintf(stderr, "This is free software: you are free to change and redistribute it.\n");
   fprintf(stderr, "There is NO WARRANTY, to the extent permitted by law.\n");
   exit(0);
}
 
int fprintbuf (FILE *_file, int _size, char _buffer[]) {
   int _index;
   for (_index = 0; _index < _size; _index++) {
      if ((_last == '\n') && (_last == _buffer[_index])) {
         _blanks++; /* Count consecutive blank lines */
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
      }
      _last = _buffer[_index]; /* Remember the last character */
   }
   return(fflush(_file)); /* Return status from fflush() */
}
 
int main(int argc, char **argv) {
   FILE *file;
   char flag = false;
   char _buffer[BUFFER_SIZE];
   int _bytes; /* Number of bytes read from file */
   int _size;  /* Number of bytes read into the buffer */
   int _count, _index, _status;
 
   /* Parse command line */
   for (_count = 1; _count < argc && !flag; _count++) {
      if (argv[_count][0] == '-') {
         _index = 1;
         while (argv[_count][_index] != 0) {
            switch (argv[_count][_index]) {
            case 'b': /* Number non empty lines */
               _nflag = true; _bflag = true; break;
            case 'f': /* Print filenames headings */
               _hflag = true; break;          
            case 'n': /* Number lines */
               _nflag = true; break;
            case 'r': /* Restart numbering */
               _rflag = true; _nflag = true; break;
            case 's': /* Squeeze blank lines */
               _sflag = true; break;
            case '?': /* Display help */
               about();               
            case '-': /* '--' terminates command line processing */
               _index = strlen(argv[_count]);
               if (_index == 2) 
                 flag = true; /* '--' terminates command line processing */
               else
                  if (!strncmp(argv[_count], "--version", _index)) {
                     version(); /* Display version information */
                  }
                  else if (!strncmp(argv[_count], "--number-nonblank", _index)) {
                     _nflag = true; _bflag = true;
                  }
                  else if (!strncmp(argv[_count], "--number", _index)) {
                     _nflag = true;
                  }
                  else if (!strncmp(argv[_count], "--squeeze-blank", _index)) {
                     _sflag = true;
                  }
                  else if (!strncmp(argv[_count], "--restart-numbering", _index)) {
                     _rflag = true;
                  }
                  else if (!strncmp(argv[_count], "--show-filenames", _index)) {
                     _hflag = true;
                  }
                  else if (!strncmp(argv[_count], "--help", _index)) {
                     about();
                  }
                  else { /* If we get here then the we have an invalid long option */
                     fprintf(stderr, "%s: invalid option %s\n", argv[0], argv[_count]);
                     fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
                     exit(-1);
               }
               _index--;
               break;
            default: /* If we get here the single letter option is unknown */
               fprintf(stderr, "%s: unknown option -- %c\n", argv[0], argv[_count][_index]);
               fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
               exit(-1);
            }
            _index++;
         }
         if (argv[_count][1] != 0) {
            for (_index = _count; _index < argc - 1; _index++) argv[_index] = argv[_index + 1];
            argc--; _count--;
         }
      }
   }
 
   /* Display files */
   _line = 1;
   for (_count = 1; _count < argc; _count++) {
      _bytes = 0;
      _size = 0;
      if ((file = fopen(argv[_count], "r")) != NULL) {
         while((_size = fread(_buffer, 1, BUFFER_SIZE, file)) > 0 ){ 
            if (_bytes == 0) {
               if (_hflag) fprintf(stdout, "%s:\n", argv[_count]); /* Optionally print filename */
               if (_rflag) _line = 1; /* Optionally reset line numbers */
               _last = '\n';
            }
            _bytes += _size;
            fprintbuf (stdout, _size, _buffer); /* Print buffer */
         }
         fclose(file);
      }
      else {
         _status = errno;
         fprintf(stderr, "%s: %s: %s\n", argv[0], argv[_count], strerror(_status));      
      }
   }
   exit (0);
}
