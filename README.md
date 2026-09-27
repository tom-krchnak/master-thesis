# salasym

Symbolic executor for SALA

```shell
# optional compatibility patch for Boost >1.87
make patch

# configure, build, and install the compiler locally
# - run only once
make salac

# configure and build salasym (default: release)
# - run after every change
make
# make PRESET=debug

# compile C and run salasym
make verify FILE=path/to/program.c
# or run an already compiled SALA program
./impl/build/release/salasym/salasym path/to/program.json

# remove all build artifacts for fresh start
make clean
```
