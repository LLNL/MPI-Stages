binname = waitall

all:
	$(CC) $(CFLAGS) -std=c99 waitall.c
	$(CCPP) $(LDFLAGS) -o $(binname) waitall.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o