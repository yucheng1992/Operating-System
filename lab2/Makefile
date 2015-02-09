# 
# NYU CS 202 - Spring 2015 - Lab 2
#   (Derived from Eddie Kohler's UCLA shell lab.)
# 

CC = gcc
CFLAGS = -g -W -Wall -Werror -Wno-unused
LAB = 2
DISTDIR = lab$(LAB)
V = @

all: cs202sh

cs202sh: cmdparse.o cmdrun.o main.o
	@echo + link $@
	$(V)$(CC) $(CFLAGS) -lpthread -o $@ cmdparse.o cmdrun.o main.o

cmdrun.o: cmdrun.c cmdparse.h cmdrun.h
	@echo + cc cmdrun.c
	$(V)$(CC) $(CFLAGS) -c cmdrun.c

cmdparse.o: cmdparse.c cmdparse.h
	@echo + cc cmdparse.c
	$(V)$(CC) $(CFLAGS) -c cmdparse.c

main.o: main.c cmdparse.h cmdrun.h
	@echo + cc main.c
	$(V)$(CC) $(CFLAGS) -c main.c

handin:
	$(V)/bin/bash ./check-lab.sh . || (rm -rf $(DISTDIR)-handin; false)
	$(MAKE) -C .. handin

clean:
	@echo + clean
	$(V)rm -rf *.o *~ *.bak core *.core cs202sh freecheck

realclean: clean
	@echo + realclean
	$(V)rm -rf $(DISTDIR) $(DISTDIR).tgz

test: cs202sh always
	/usr/bin/perl -w ./tester.pl

always:
	@:

.PHONY: always clean realclean test handin
