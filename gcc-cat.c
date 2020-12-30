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
 * with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *
 * 03 Mar 14   0.1   - Initial version - MT
 *
 */
 
#include<stdio.h>
#include<string.h>
#include<errno.h>
 
main(int argc, char *argv[]) {
   int count;
   FILE *file;
   int chr;
 
   for(count = 1; count < argc; count++) {
      file = fopen(argv[count], "r");
      if(file == NULL) {
         fprintf(stderr, "%s: %s\n", argv[count], strerror(errno));
         continue;
      }
      while((chr = getc(file)) != EOF)
         fprintf(stdout, "%c", chr);
      fclose(file);
   }
   exit(0);
}

