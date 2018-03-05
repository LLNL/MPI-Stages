binname = waitall

all:
	$(CC) $(CFLAGS) waitall.c
	$(CCPP) $(LDFLAGS) -o $(binname) waitall.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o