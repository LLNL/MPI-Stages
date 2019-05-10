# ExaMPI

## Getting Started

### Building
1. make
2. source runtime/environment

Currently no install procedure is in place. Cmake will be used in future to provide this functionality.

### Usage

0. source /runtime/environment
1. mpicc main.c -o main
2. mpiexec -n N ./main

## Getting Involved

## Contributing

mpiexec has options, access via `mpiexec --help`

## Notes
1. Every time a new shell is opened you must run `./runtime/environment` in order to force your system to use ExaMPI's mpicc and mpiexec.

## License
LLNL release number: LLNL-CODE-770619

