/*
 * cat.c
 *
 * Copyright(C) 2014 - MT
 *
 * A minimal implementation of the ubiquitous 'cat'.
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
 *
 */
 
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* strerror */
#include <unistd.h> /* usleep */
 
static int bflag = 0, delay = 0, nflag = 0, rflag = 0, sflag = 0;
static int line = 0;
 
int main (int argc, char **argv) {
  FILE *file;
  int count, opt;
  int chr, prev, blank;
 
/*
 * Note - To distinguish between options that require an argument and those
 * that do not those that do are followed by a colon.
 */
 
  while ((opt = getopt (argc, argv, "bd:nrs?")) != -1) {
    switch (opt) {
      case 'b': /* Number non blank lines only (overrides -n)*/
        nflag = 1;
        bflag = 1;
        break;
      case 'd':
        delay = atoi(optarg); /* Number of miliseconds to delay */
        break;
      case 'n': /* Number every line */ 
        nflag = 1;
        break;
      case 'r': /* Restart line numbering with each new file */
        rflag = 1;
        nflag = 1;
        break;
      case 's': /* Squeeze blank lines */
        sflag = 1;
        break;
      case '?': /* An error occoured parsing the options.. */
      default:
        exit (1);
      }
  }
 
  for (count = optind; count < argc; count++) {
    if ((file = fopen(argv[count], "r")) == NULL) {
      fprintf(stderr, "%s: %s : %s\n", argv[0], argv[count], strerror(errno));
    } 
    else {
      blank =0;
      if (rflag)
        line =0;
      for (prev = '\n'; (chr = getc(file)) != EOF; prev = chr){
        if (prev == '\n'){
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
        if (delay > 0) 
          fflush(stdout);
          usleep(delay * 1000);
      } 
      fclose(file); 
    }
  }
  exit(0);
}
