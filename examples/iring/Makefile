basedir = ../..
libdir = $(basedir)/src
incdir = $(basedir)/inc
binname = iring

all:
	$(CC) $(CFLAGS) iring.c
	$(CCPP) $(LDFLAGS) -lrt -o $(binname) iring.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o
