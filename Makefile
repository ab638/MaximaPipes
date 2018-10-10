# Makefile for expand.c and factor.c
#

all: expand factor

expand:
	gcc -g -o expand expand.c
factor:
	gcc -g -o factor factor.c
clean:
	rm -f *.o expand factor *.*~
