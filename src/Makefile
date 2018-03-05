cppsrc = $(wildcard *.cpp) $(wildcard mpi/*.cpp) $(wildcard transport/*.cpp)
csrc = $(wildcard *.c) $(wildcard mpi/*.c)
cppobj = $(cppsrc:.cpp=.cpp.o)
cobj = $(csrc:.c=.c.o)
alib = libexampi.a

CFLAGS = -std=gnu11 -Iinc -I../inc -Wall -Werror
CXXFLAGS = -std=c++11 -pthread -Iinc -I../inc -Itransport -Wall -Werror

$(alib): $(cppobj) $(cobj)
	$(AR) rcs $@ $^

%.cpp.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS)

%.c.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(cppobj) $(cobj)


