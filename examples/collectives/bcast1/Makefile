basedir = ../../..
libdir = $(basedir)/src
incdir = $(basedir)/inc
binname = bcast1

all:
	$(CC) $(CFLAGS) bcast1.c
	$(CCPP) $(LDFLAGS) -lrt -o $(binname) bcast1.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o
