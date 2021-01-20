#
#	gcc-cat.make
#
#	Copyright (C) 2012  MEJT
#	
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#	
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#	
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#	Note - Shell commands must start with a tab character at the beginning
#       of each line NOT spaces..!
#	
#  09 Apr 12   0.1   - Initial version - MEJT
#  17 Aug 13   0.2   - Added make clean and make verify to delete any 
#                      object files and automatically compare the output
#                      against that expected - MEJT
#
PROGRAM = gcc-cat
FILES		= *.c.* LICENSE README.md makefile
SOURCE = $(PROGRAM).c
OBJECT = $(SOURCE:.c=.o)
#OUTPUT = gcc-cat.t
FLAGS	= 
LIBS	= 
#LIBS	= -lm
CC	= gcc
DIFF = diff
#
$(PROGRAM): $(OBJECT) 
#	@echo '*** Linking $(OBJECT)'
	@$(CC) $(OBJECT) -o $(PROGRAM) $(LIBS)
#	Tidy up by deleting ALL object files - this will force all the sources
# 	to be recompiled every time so may not be a good idea on slow machines
#	@echo '*** Deleting object files'
	@rm -f $(OBJECT)
#
#	Doesn't attempt to be selective - just recompiles everything!
$(OBJECT) : $(SOURCE)
#	@echo '*** Compiling $(SOURCE)'
	@$(CC) $(FLAGS) -c $(SOURCE)
#
#  	make clean - Clean up any object files.
clean:
#	@echo '*** Cleaning up object files'
	@rm -f $(OBJECT)
#
backup:
	@tar -czvpf $(PROGRAM)-`date +'%Y%m%d%H%M'`.tar.gz $(FILES)
