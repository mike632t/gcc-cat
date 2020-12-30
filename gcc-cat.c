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
 */
 
#define VERSION       "0.4"
#define BUILD         "0018"
#define AUTHOR        "MT"
#define DATE          "16 Mar 19"
#define COPYRIGHT     (__DATE__ +7) /* Extract copyright year from date */
 
#define DEBUG(code)   do {if (ENABLE_DEBUG) {code;}} while(0)
#define ENABLE_DEBUG  0
 
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
  
static int bflag = 0, hflag = 0, nflag = 0, rflag = 0, sflag = 0;
static int line = 0;
static int chr, prev, blank;
 
int cat(FILE *file) {
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
    prev = chr;
  }
  return(fflush(stdout));
}
 
int main(int argc, char **argv) {
  FILE *file;
  int errnum;
   
  int count, index;
  int flag = 1;
 
  for (index = 1; index < argc && flag; index++) {
    if (argv[index][0] == '-') {
      count = 1;
      while (argv[index][count] != 0) {
        switch (argv[index][count]) { 
          case 'b': /* Number non empty lines */
            nflag = 1; bflag = 1; break;
          case 'h': /* Print filenames headings */
            hflag = 1; break;          
          case 'n': /* Number lines */
            nflag = 1; break;
          case 'r': /* Restart numbering */
            rflag = 1; nflag = 1; break;
          case 's': /* Squeeze blank lines */
            sflag = 1; break;
          case '-': /* Check for long options */
            DEBUG(fprintf(stderr, "%s:%0d: argv[%0d][%0d]: '%c'\n", argv[0], __LINE__, index, count, argv[index][count]));
            count = strlen(argv[index]);
            if (count == 2) 
              flag = 0; /* '--' terminates command line processing */
            else
              if (!strncmp(argv[index], "--version", count)) { /* Display version information */
                fprintf(stderr, "%s: Version %s\n", argv[0], VERSION);
                fprintf(stderr, "Copyright(C) %s %s\n", COPYRIGHT, AUTHOR);
                fprintf(stderr, "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n");
                fprintf(stderr, "This is free software: you are free to change and redistribute it.\n");
                fprintf(stderr, "There is NO WARRANTY, to the extent permitted by law.\n");
                exit(0);
              }
              else {
                fprintf(stderr, "%s: invalid option %s\n", argv[0], argv[index]);
                fprintf(stderr, "Usage: %s [-v] [--version] [file...]\n", argv[0]);
                exit(-1);
              }
            count--;
            break;
          default: /* If we get here the option is invalid */
            fprintf(stderr, "%s: unknown option -- %c\n", argv[0], argv[index][count]);
              fprintf(stderr, "Usage: %s [-v] [--version] [file...]\n", argv[0]);
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
 
  DEBUG(fprintf(stderr, "%s: Version %s (%s)\n", argv[0], VERSION, DATE));

  prev = '\n';
  for (count = 1; count < argc; count++)
    if (strcmp("-", argv[count]) == 0) {
      if (hflag) {
        if (prev != '\n') fprintf(stdout, "\n");
        fprintf(stdout, "stdin:\n");
        prev = '\n';
      }
      cat(stdin);
    }
    else {
      if ((file = fopen(argv[count], "r")) != NULL) {
        if (hflag) {
          if (prev != '\n') fprintf(stdout, "\n");
          fprintf(stdout, "%s:\n", argv[count]);
          prev = '\n';
        }
        cat(file);
        fclose(file);
      }
      else {
        errnum = errno;
        fprintf(stderr, "%s: %s: %s\n", argv[0], argv[count], strerror(errnum));      
      }
    }
  exit(0);
}
