# HiKey 970 tools and utilities

A collection of tools and utilities for the HiKey 970 board (https://www.96boards.org/product/hikey970).

## Build AOSP

The Makefile in `build_kernel` can be used to build AOSP. The kernel is built from sources and combined with a pre-built AOSP into an image that can be flashed onto the HiKey 970 board.

Install dependencies

```
$ sudo apt install git-core gnupg flex bison gperf build-essential zip curl zlib1g-dev gcc-multilib g++-multilib libc6-dev-i386 lib32ncurses5-dev x11proto-core-dev libx11-dev lib32z-dev ccache libgl1-mesa-dev libxml2-utils xsltproc unzip python-mako openjdk-8-jdk uuid-dev build-essential libssl-dev
```

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
