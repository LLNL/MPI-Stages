basedir = ../..
libdir = $(basedir)/src
incdir = $(basedir)/inc
binname = hello

all:
	g++ -I$(incdir) -L$(libdir) -o $(binname) hello.c -lexampi

clean:
	rm -f $(binname)
	rm -f *.o
