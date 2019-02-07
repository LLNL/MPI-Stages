################################################################################
#                                 User Section
################################################################################

CXX ?= c++

# flags
CXXFLAGS += -Wall -Wextra -pedantic -Wno-unused-parameter
CXXFLAGS += -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable
CXXFLAGS += -std=c++17 -g -pthread -O2

CXXFLAGS += -DDEBUG 
#CXXFLAGS += -Wfatal-errors

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
	$(CXX) -I$(incdir) -c $(CXXFLAGS) $< -o $@

$(blddir)/*/%.o: $(srcdir)/*/%.cc $(headers) | directories
	$(CXX) -I$(incdir) -c $(CXXFLAGS) $< -o $@

### directory rules

directories:
	@mkdir -p $(bindir)
	@mkdir -p $(libdir)
	@mkdir -p $(blddir)
	@mkdir -p $(blddir)/mpi
	@mkdir -p $(blddir)/interfaces
	@mkdir -p $(blddir)/engines
	@mkdir -p $(blddir)/checkpoints
	@mkdir -p $(blddir)/matchers

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
