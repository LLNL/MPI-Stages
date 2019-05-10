# ExaMPI

## Getting Started

### Building
1. make
2. source runtime/environment

ExaMPI requires a compiler which supports C++17. GCC7 and up functions.

Currently no install procedure is in place. Cmake will be used in future to provide this functionality.
Installation can be done via manually moving the lib, include & runtime directories to the desired location.

### Usage

0. source /runtime/environment
1. mpicc main.c -o main
2. mpiexec -n N ./main

## Getting Involved

Currently submit an issue via the issue tracker on this github repository.

## Contributing

We do closed source development for the moment. Please fork off this public repository to contribute and submit pull requests.

Contributions must be MIT/BSD like licenses.

A vague style guide is present under docs/. Astyle is used to iterate the code and style it according to
the determined standard. `make style` can be used to apply the style.

## How to cite

This work can be cited via:

* https://dl.acm.org/citation.cfm?id=3236385
* https://www.sciencedirect.com/science/article/pii/S0167819118303260

## Notes
1. Every time a new shell is opened you must run `./runtime/environment` in order to force your system to use ExaMPI's mpicc and mpiexec.

## License

View the LICENSE-\* and COPYRIGHT files.

LLNL release number: LLNL-CODE-770619
