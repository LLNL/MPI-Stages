basedir = ../..
libdir = $(basedir)/src
incdir = $(basedir)/inc
binname = maxminloc

all:
	gcc -c -ggdb -I$(incdir) maxminloc.c 
	g++    -pthread -ggdb -L$(libdir) -lrt -o $(binname) maxminloc.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o