cppsrc = $(wildcard *.cpp)
csrc = $(wildcard *.c)
cppobj = $(cppsrc:.cpp=.cpp.o)
cobj = $(csrc:.c=.c.o)
alib = exampi.a

CFLAGS = -std=gnu11 -Iinc
CXXFLAGS = -std=gnu++11 -Iinc

$(alib): $(cppobj) $(cobj)
	$(AR) rcs $@ $^

%.cpp.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS)

%.c.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(cppobj) $(cobj)


