binname = bcast0

all:
	$(CC) $(CFLAGS) bcast0.c
	$(CCPP) $(LDFLAGS) -lrt -o $(binname) bcast0.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o
