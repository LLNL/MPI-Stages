basedir = ../..
libdir = $(basedir)/src
incdir = $(basedir)/inc
binname = ring

all:
	gcc -c -ggdb -I$(incdir) ring.c 
	g++    -ggdb -L$(libdir) -o $(binname) ring.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o
