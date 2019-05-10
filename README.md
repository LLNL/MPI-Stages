# ExaMPI

## Getting Started

### Building
1. make
2. source runtime/environment

Currently no install procedure is in place. Cmake will be used in future to provide this functionality.
Installation can be done via manually moving the lib & include directories to the desired location.

### Usage

0. source /runtime/environment
1. mpicc main.c -o main
2. mpiexec -n N ./main

## Getting Involved

Currently submit an issue via the issue tracker on this github repository.

## Contributing

We do closed source development for the moment. Please fork off this public repository to contribute and submit pull requests.

A vague style guide is present under docs/. Astyle is used to iterate the code and style it according to
the determined standard. `make style` can be used to apply the style.

## Notes
1. Every time a new shell is opened you must run `./runtime/environment` in order to force your system to use ExaMPI's mpicc and mpiexec.

## License
LLNL release number: LLNL-CODE-770619
