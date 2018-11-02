binname = sendrecv

all:
	$(CC) $(CFLAGS) -std=c99 sendrecv.c
	$(CCPP) $(LDFLAGS) -lrt -o $(binname) sendrecv.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o