# Makefile for ex1-q1 winter 2019A
CFLAGS = -Wall
LDFLAGS = -lm # not really needed for this exercise
CC = gcc -std=c99
ECHO = echo "going to compile for target $@"
OS := $(shell uname)
ifneq (,$(findstring CYGWIN_NT,$(OS)))
EXT = .exe
else
EXT =
endif
PROG = ex3_q2$(EXT)

all: $(PROG) test

ex3_q2_given.o: ex3_q2_given.c
	echo going to compile subst.o...
	gcc -std=gnu99 -Wall -c ex3_q2_given.c ex3_q2_given.h

ex3_q2.o: ex3_q2.c 
	echo going to compile ex3_q2.o...
	gcc -std=gnu99 -Wall -c ex3_q2.c ex3_q2.h

$(PROG): ex3_q2.o ex3_q2_given.o
	echo 'going to build (link) $(PROG)...'
	gcc -o $(PROG) -Wall ex3_q2.o -lm ex3_q2_given.o

clean:
	rm -vf *.o ${PROG} *.log *.exe
	
test:
	@echo going to run test...
	# make sure you have input file in.txt
	# so that the following command works.
	./ex3_q2 > out.log 2> err.log 
	
