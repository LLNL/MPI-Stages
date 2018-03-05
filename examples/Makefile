dirs = hello ring $(wildcard collectives/*) check pseudo waitall

export basedir = $(realpath ..)
export libdir = $(basedir)/src
export incdir = $(basedir)/inc

export CC = gcc
export CCPP = g++
export CFLAGS = -c -ggdb -I$(incdir)
export LDFLAGS = -pthread -L$(libdir) -lexampi

default:
	$(foreach dir,$(dirs),$(MAKE) -C $(dir);)

clean:
	$(foreach dir,$(dirs),$(MAKE) -C $(dir) clean;)
