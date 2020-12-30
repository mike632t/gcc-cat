/*
 * cat.c
 *
 * Copyright(C) 2018 - MT
 *
 * Concatenate files and print on the standard output.
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
 * with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *
 * 03 Apr 18   0.1   - Initial version - MEJT
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
int main(int argc, char *argv[]) {
  FILE *fp;
  char *prog = argv[0];
  int c;
 
  while (--argc > 0)
    if ((fp = fopen(*++argv, "r")) == NULL) {
      fprintf(stderr, "%s: can't open %s\n", prog, *argv);
      exit(1);
    } else {
      while ((c = getc(fp)) != EOF) {
        putc(c, stdout);
      }
      fclose(fp);
    }
  if (ferror(stdout)) {
    fprintf(stderr, "%s: error writing stdout\n", prog);
    exit(2);
  }
  exit(0);
}

