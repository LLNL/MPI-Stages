# ExaMPI

# Getting Started

# Getting Involved

# Contributing

## Build Instructions
0. `git checkout develop` (Temporarily need to be on the develop branch)
1. `make`

(will change later with cmake)

## Usage
1. `source ./runtime/environment`
2. `mpicc main.c -o main`
3. `mpiexec -n N ./main`


mpiexec has many options, access via `mpiexec --help`

`-n N` specifies the number of concurrent processes to launch

## Notes
1. If you are using OSX, you must install gcc. By default, using the gcc command on OSX calls to clang which is currently unsupported.
2. Every time a new shell is opened you must run `./runtime/environment` in order to force your system to use ExaMPI's mpicc and mpiexec.

# License
LLNL release number: LLNL-CODE-770619

