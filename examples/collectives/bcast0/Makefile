basedir = ../../..
libdir = $(basedir)/src
incdir = $(basedir)/inc
binname = bcast0

all:
	gcc -c -ggdb -I$(incdir) bcast0.c 
	g++    -ggdb -L$(libdir) -o $(binname) bcast0.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o
