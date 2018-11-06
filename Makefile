################################################################################
#                                 User Section
################################################################################

# compiler/linker choice
CXX = g++ 
LD =

# flags
CXXFLAGS = -std=c++11 -pthread -Wall
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
default: static

help:
	@echo "make static"
	@echo "make shared"

# target building rules
shared: $(shlib)

static: $(alib)

$(alib): $(objects)
	$(AR) rcs $@ $^

$(shlib): $(objects)
	@echo "SHARED LIBRARY NOT IMPLEMENTED"

# object building rules
$(blddir)/%.o: $(srcdir)/%.cpp $(headers) | directories
	$(CXX) -I$(incdir) -c $(CXXFLAGS) $< -o $@

$(blddir)/*/%.o: $(srcdir)/*/%.cpp $(headers) | directories
	$(CXX) -I$(incdir) -c $(CXXFLAGS) $< -o $@

### directory rules
directories:
	@echo "Creating build directories."
	@mkdir -p $(bindir)
	@mkdir -p $(libdir)
	@mkdir -p $(blddir)
	@mkdir -p $(blddir)/mpi

### clean rules
.PHONY: style
style:
	@echo "Formatting all header files and source files."
	@astyle --project "include/*" "src/*"

.PHONY: clean
clean:
	@echo "Removing all build directories."
	@-rm -rf $(bindir)
	@-rm -rf $(blddir)
	@-rm -rf $(libdir)
