################################################################################
#                                 User Section
################################################################################

# compiler/linker choice
CXX = g++ 
LD =

# flags
CXXFLAGS = -std=c++11 -pthread -Wall -Werror
LDFLAGS = 

# target definitions
shlib = $(libdir)/libexampi.so
alib = $(libdir)/libexampi.a

################################################################################
#                               Automatic Section
################################################################################

# directory definitions
bindir = bin
libdir = lib
blddir = build
srcdir = src
incdir = include
tstdir = tests
docdir = docs

# detect files
headers = $(wildcard $(incdir)/*.h) $(wildcard $(incdir)/*/*.h)
sources = $(wildcard $(srcdir)/*.cpp) $(wildcard $(srcdir)/*/*.cpp)

# determine files
objects = $(patsubst %.cpp, %.o, $(subst $(srcdir), $(blddir), $(sources)))

### rules
#default: $(alib)
default: static

help:
	@echo "make static"
	@echo "make shared"

#$(target): directories $(headers) $(objects)
#	$(AR) $(LDFLAGS) -o %@

#shared: $(shlib)

static: $(alib)

$(alib): $(objects)
	$(AR) rcs $@ $^

$(blddir)/%.o: $(srcdir)/%.cpp $(headers) | directories
	$(CXX) -I$(incdir) -c $(CXXFLAGS) $< -o $@

$(blddir)/*/%.o: $(srcdir)/*/%.cpp $(headers) | directories
	$(CXX) -I$(incdir) -c $(CXXFLAGS) $< -o $@

### directory rules
directories:
	mkdir -p $(bindir)
	mkdir -p $(libdir)
	mkdir -p $(blddir)
	mkdir -p $(blddir)/mpi

### clean rules
.PHONY: clean
clean:
	@-rm -rf $(bindir)
	@-rm -rf $(blddir)
	@-rm -rf $(libdir)
