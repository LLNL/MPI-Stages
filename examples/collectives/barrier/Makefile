binname = barrier

all:
	$(CC) $(CFLAGS) barrier.c
	$(CCPP) $(LDFLAGS) -o $(binname) barrier.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o