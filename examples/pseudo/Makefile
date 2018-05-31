basedir = ../..
libdir = $(basedir)/src
incdir = $(basedir)/inc
binname = ring-check

all:
	gcc -c -ggdb -I$(incdir) ring-check.c 
	g++    -pthread -ggdb -L$(libdir) -lrt -o $(binname) ring-check.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o
