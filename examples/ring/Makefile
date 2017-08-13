basedir = ../..
libdir = $(basedir)/src
incdir = $(basedir)/inc
binname = ring

all:
	g++ -ggdb -I$(incdir) -L$(libdir) -o $(binname) ring.c -lexampi

clean:
	rm -f $(binname)
	rm -f *.o
