basedir = ../..
libdir = $(basedir)/src
incdir = $(basedir)/inc
binname = abort

all:
	gcc -c -ggdb -I$(incdir) abort.c 
	g++    -pthread -ggdb -L$(libdir) -o $(binname) abort.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o
