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
 *                     false - MT
 * 03 Jul 20   0.7   - Tidied up formatting - MT
 *                   - Changed program name to use defined text - MT
 *                   - Added  separate routines to display program  version
 *                     and help text - MT
 *
 * To Do:            - Default to copying standard input to standard output
 *                     if no arguments are specified on the command line.
 *                   - Warn user if a command line option is ambiguous.
 *                   - Use a buffer.
 */
 
#define NAME         "cat"
#define VERSION      "0.7"
#define BUILD        "0026"
#define AUTHOR       "MT"
#define COPYRIGHT    (__DATE__ +7) /* Extract copyright year from date */
 
#define DEBUG(code)  do {if (__DEBUG__) {code;}} while(0)
#define __DEBUG__    0

#define true         1
#define false        0
 
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

typedef char bool;
 
static bool bflag = false, hflag = false, nflag = false, rflag = false, sflag = false;
static int line = 0;
static int chr, prev, blank;
 
void about() { /* Display help text */
   fprintf(stdout, "Usage: %s [OPTION]... [FILE]...\n"
      "Concatenate FILE(s)to standard output.\n\n" 
      "  -f, --filenames          display filenames\n"
      "  -n, --number             number all output lines \n"
      "  -r, --restart            line numbers start at zero, implies -n\n" 
      "  -s, --squeeze-blank      suppress repeated empty output lines\n"
      "      --verbose            display verbose output\n"
      "  -?, --help               display this help and exit\n"
      "      --version            output version information and exit\n\n"
      "With no FILE, or when FILE is -, read standard input.\n", 
      NAME);
   exit(0);
}

void version() { /* Display version information */
   fprintf(stderr, "%s: Version %s\n"
      "Copyright(C) %s %s\n"
      "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n"
      "This is free software: you are free to change and redistribute it.\n"
      "There is NO WARRANTY, to the extent permitted by law.\n", 
      NAME, VERSION, COPYRIGHT, AUTHOR);
   exit(0);
}
 
int main(int argc, char **argv) {
   FILE *file;
   int errnum;
   int count, index;
   bool flag = true;
 
   /* Parse command line */
   for (index = 1; index < argc && flag; index++) {
      if (argv[index][0] == '-') {
         count = 1;
         while (argv[index][count] != 0) {
            switch (argv[index][count]) {
            case 'b': /* Number non empty lines */
               nflag = true; bflag = true; break;
            case 'f': /* Print filenames headings */
               hflag = true; break;          
            case 'n': /* Number lines */
               nflag = true; break;
            case 'r': /* Restart numbering */
               rflag = true; nflag = true; break;
            case 's': /* Squeeze blank lines */
               sflag = true; break;
            case '?': /* Display help */
               about();               
            case '-': /* '--' terminates command line processing */
               DEBUG(fprintf(stderr, "%s:%0d: argv[%0d][%0d]: '%c'\n", argv[0], __LINE__, index, count, argv[index][count]));
               count = strlen(argv[index]);
               if (count == 2) 
                 flag = false; /* '--' terminates command line processing */
               else
                  if (!strncmp(argv[index], "--version", count)) {
                     version(); /* Display version information */
                  }
                  else if (!strncmp(argv[index], "--number-nonblank", count)) {
                     nflag = true; bflag = true;
                  }
                  else if (!strncmp(argv[index], "--number", count)) {
                     nflag = true;
                  }
                  else if (!strncmp(argv[index], "--squeeze-blank", count)) {
                     sflag = true;
                  }
                  else if (!strncmp(argv[index], "--restart-numbering", count)) {
                     rflag = true;
                  }
                  else if (!strncmp(argv[index], "--show-filenames", count)) {
                     hflag = true;
                  }
                  else if (!strncmp(argv[index], "--help", count)) {
                     about();
                  }
                  else { /* If we get here then the we have an invalid long option */
                     fprintf(stderr, "%s: invalid option %s\n", argv[0], argv[index]);
                     fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
                     exit(-1);
               }
               count--;
               break;
            default: /* If we get here the single letter option is unknown */
               fprintf(stderr, "%s: unknown option -- %c\n", argv[0], argv[index][count]);
               fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
               exit(-1);
            }
            count++;
         }
         if (argv[index][1] != 0) {
            for (count = index; count < argc - 1; count++) argv[count] = argv[count + 1];
            argc--; index--;
         }
      }
   }
 
   /* Display files */
   prev = '\n';
   for (count = 1; count < argc; count++) {
      if ((file = fopen(argv[count], "r")) != NULL) {
         if (hflag) {
            if (prev != '\n') fprintf(stdout, "\n");
            fprintf(stdout, "%s:\n", argv[count]);
            prev = '\n';
         }
         blank = 0;
         if (rflag) line = 0;
         while ((chr = getc(file)) != EOF) {
            if (prev == '\n') {
            ++blank;
            if (sflag && chr == '\n' && blank > 1)
               continue;
            if (nflag)
               if (!bflag || chr != '\n')
                  fprintf(stdout, "%6d\t", ++line);
            } 
         else 
            blank = 0;
         fprintf(stdout, "%c", chr);
         fflush(stdout);
         prev = chr;
         }
         fclose(file);
      }
      else {
         errnum = errno;
         fprintf(stderr, "%s: %s: %s\n", argv[0], argv[count], strerror(errnum));      
      }
   }
}
