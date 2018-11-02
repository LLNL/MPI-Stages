basedir = ../..
libdir = $(basedir)/src
incdir = $(basedir)/inc
binname = ring

all:
	gcc -c -ggdb -I$(incdir) ring.c 
	g++    -pthread -ggdb -L$(libdir) -lrt -o $(binname) ring.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o
