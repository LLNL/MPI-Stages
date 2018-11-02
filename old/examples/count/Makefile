basedir = ../..
libdir = $(basedir)/src
incdir = $(basedir)/inc
binname = count

all:
	gcc -c -ggdb -I$(incdir) getcount.c 
	g++    -pthread -ggdb -L$(libdir) -lrt -o $(binname) getcount.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o