# HiKey 970 tools and utilities

A collection of tools and utilities for the HiKey 970 board (https://www.96boards.org/product/hikey970).

## Build AOSP

The Makefile in `build_kernel` can be used to build AOSP. The kernel is built from sources and combined with a pre-built AOSP into an image that can be flashed onto the HiKey 970 board.

To build it:

```
$ cd build_kernel
$ make bootloader
$ make all
```

The images are created in the `out` folder.

## TODO

* documentation
* automate making the executables
