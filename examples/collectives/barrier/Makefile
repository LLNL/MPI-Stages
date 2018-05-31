binname = barrier

all:
	$(CC) $(CFLAGS) barrier.c
	$(CCPP) $(LDFLAGS) -lrt -o $(binname) barrier.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o