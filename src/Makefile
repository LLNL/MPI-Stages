cppsrc = $(wildcard *.cpp) $(wildcard mpi/*.cpp) $(wildcard transport/*.cpp)
csrc = $(wildcard *.c) $(wildcard mpi/*.c)
cppobj = $(cppsrc:.cpp=.cpp.o)
cobj = $(csrc:.c=.c.o)
alib = libexampi.a

CFLAGS = -std=gnu11 -Iinc -I../inc
CXXFLAGS = -std=gnu++11 -Iinc -I../inc -Itransport

$(alib): $(cppobj) $(cobj)
	$(AR) rcs $@ $^

%.cpp.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS)

%.c.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(cppobj) $(cobj)


