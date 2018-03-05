#basedir = ../..
#libdir = $(basedir)/src
#incdir = $(basedir)/inc
binname = hello

all:
	#gcc -c -ggdb -I$(incdir) hello.c 
	$(CC) $(CFLAGS) hello.c
	$(CCPP) $(LDFLAGS) -o $(binname) hello.o -lexampi
	#g++    -pthread -ggdb -L$(libdir) -o $(binname) hello.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o
