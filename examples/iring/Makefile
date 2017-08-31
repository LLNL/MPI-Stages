basedir = ../..
libdir = $(basedir)/src
incdir = $(basedir)/inc
binname = iring

all:
	gcc -c -ggdb -I$(incdir) iring.c 
	g++    -ggdb -L$(libdir) -o $(binname) iring.o -lexampi -lpthread

clean:
	rm -f $(binname)
	rm -f *.o
