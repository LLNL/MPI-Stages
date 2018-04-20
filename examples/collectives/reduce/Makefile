binname = reduce

all:
	$(CC) $(CFLAGS) -std=c99 reduce.c
	$(CCPP) $(LDFLAGS) -o $(binname) reduce.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o