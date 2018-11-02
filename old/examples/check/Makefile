#basedir = ../..
#libdir = $(basedir)/src
#incdir = $(basedir)/inc
binname = check

all:
	$(CC) $(CFLAGS) check.c
	$(CCPP) $(LDFLAGS) -lrt -o $(binname) check.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o
