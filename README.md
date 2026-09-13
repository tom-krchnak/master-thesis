# salasym

Symbolic executor for SALA

## How to use

I use `CMakeLists.txt` with `CMakePresets.json` to build the project and all the dependencies.

But the commands are ugly so I have a simple `Makefile` wrapper:
```shell
# (optional) if you have newer llvm (>=22) and boost (>1.87)
make patch

# configure cmake, install salac locally
make init

# build salasym (default: release)
make salasym
# make salasym PRESET=debug

# run salasym with given source file
make verify FILE=data/simple.c
```

Instead of using cmake, it's possible to run the commands directly:

```shell
# (optional) install salac locally
cmake --preset release
cmake --build --preset release --target salac
cmake --install build/release/binsalac

# build salasym ('release' can be changed to 'debug')
cmake --preset release
cmake --build --preset release --target salasym

# compile the file and run salasym
./build/release/install/bin/salac.py --input FILE --output OUT --opt 2
./build/release/salasym/salasym OUT/FILE.json
```
