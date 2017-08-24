basedir = ../../..
libdir = $(basedir)/src
incdir = $(basedir)/inc
binname = bcast1

all:
	gcc -c -ggdb -I$(incdir) bcast1.c 
	g++    -pthread -ggdb -L$(libdir) -o $(binname) bcast1.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o
