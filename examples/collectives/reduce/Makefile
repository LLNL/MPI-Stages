binname = reduce

all:
	$(CC) $(CFLAGS) reduce.c
	$(CCPP) $(LDFLAGS) -o $(binname) reduce.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o