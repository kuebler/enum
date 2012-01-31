#
# Author: <kuebler@informatik.uni-tuebingen.de>
#

CC = gcc
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
RM = rm
LIBS = -L/opt/local/lib -Lpicosat -lgmp -lpicosat
INCLUDE = -I/opt/local/include -Ipicosat


all: enum tags

#bailleuxboufkhad: bailleuxboufkhad.o array.o solver.o
#	$(CC) -o $@ $^

enum: parse.o solver.o array.o bailleuxboufkhad.o picosat_binding.o
	$(CC) $(LIBS) -o $@ $^

.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) $<

clean: cleartags
	$(RM) -f *.o
	$(RM) -f bailleuxboufkhad
	$(RM) -f enum

cleartags:
	$(RM) -f tags

tags:
	ctags --recurse .
