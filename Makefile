################################################################################
#                                 User Section
################################################################################

CXX ?= c++

# flags
CXXFLAGS += -Wall -Wextra -pedantic
CXXFLAGS += -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable
#CXXFLAGS += -Wno-unused-parameter

CXXFLAGS += -std=c++17 -g -pthread 

CXXFLAGS += -O3 -funroll-loops

#CXXFLAGS += -DDEBUG 

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
trddir = third-party
tstdir = tests
docdir = docs

HDRFLAGS = -I$(trddir) -I$(incdir)

# detect files
headers = $(wildcard $(incdir)/*.h) $(wildcard $(incdir)/*/*.h)
sources = $(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*/*.cc)

# determine files
objects = $(patsubst %.cc, %.o, $(subst $(srcdir), $(blddir), $(sources)))

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
$(blddir)/%.o: $(srcdir)/%.cc $(headers) | directories
	$(CXX) $(HDRFLAGS) -c $(CXXFLAGS) $< -o $@

$(blddir)/*/%.o: $(srcdir)/*/%.cc $(headers) | directories
	$(CXX) $(HDRFLAGS) -c $(CXXFLAGS) $< -o $@

### directory rules

directories:
	@mkdir -p $(bindir)
	@mkdir -p $(libdir)
	@mkdir -p $(blddir)
	@mkdir -p $(blddir)/mpi
	@mkdir -p $(blddir)/interfaces
	@mkdir -p $(blddir)/engines
	@mkdir -p $(blddir)/checkpoints
	@mkdir -p $(blddir)/transports
	@mkdir -p $(blddir)/matchers
	@mkdir -p $(blddir)/deciders
	
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
