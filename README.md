![Logo](logo.svg)

![Workflow status](https://github.com/madmann91/overture/actions/workflows/build-test-action.yml/badge.svg)

# Overture

A small data structures and algorithm library in C, mostly header-only. This contains the following
algorithms and data structures:

- Vector,
- Span,
- Hash set,
- Hash map,
- Priority queue,
- Unique stack,
- Strings and string views,
- Graph with various traversal algorithms,
- String pool,
- Memory pool,
- Union-find,
- Heap sort,
- Minstd0 random generator,
- FNV-1a hash function,
- Log and error message system,
- Command-line argument parsing,
- Testing framework with process isolation,
- ANSI terminal code helpers,
- Allocation routines,
- Simple IO routines,
- Bit manipulation routines

This library is not intended to be used for public APIs, as there is no attempt at using namespaces
to protect from name collisions.

## Building

This project requires a C23-compliant compiler and CMake. The following commands build the project:

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=<Debug|Release|Coverage> ..
    make

## Testing

Once built, the project can be tested via:

    make test

or, alternatively, when trying to debug memory leaks or other memory errors with a memory checker:

    make memcheck

## Documentation

The project supports the doxygen code documentation generator. It can be invoked manually from the
`doc` directory or by typing `make doc` using the CMake-generated Makefile (see instructions above
to build using CMake).

## License

This project is distributed under the [MIT license](LICENSE.txt).
