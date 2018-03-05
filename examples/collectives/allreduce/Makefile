binname = allreduce

all:
	$(CC) $(CFLAGS) allreduce.c
	$(CCPP) $(LDFLAGS) -o $(binname) allreduce.o -lexampi

clean:
	rm -f $(binname)
	rm -f *.o