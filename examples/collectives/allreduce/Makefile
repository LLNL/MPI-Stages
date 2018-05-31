binname = allreduce

all:
	$(CC) $(CFLAGS) -std=c99 allreduce.c
	$(CCPP) $(LDFLAGS) -lrt -o $(binname) allreduce.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o