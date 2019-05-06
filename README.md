# COLLASCII

[![Build Status](https://travis-ci.com/olin/SoftSysCollascii.svg?branch=master)](https://travis-ci.com/olin/SoftSysCollascii)

_A collaborative ascii canvas_

## Getting Started

_First, a word on organization:_

Source code lives in `src/`, class reports live in `reports/`.

Collascii currently supports Linux and MacOS, but it should run anywhere that `ncurses` and POSIX threads do.

You can build your own version of `collascii` (see [Building](#building)) or use a pre-built executable from the [Releases page](https://github.com/olin/SoftSysCollascii/releases).

### Usage

Run the executable:
```shell
./collascii
```

This will open the editor view. Move the cursor with the arrow keys, and type to insert text.

To export your art, you'll need to copy it off of the screen. Most terminals 
support some sort of block select, which makes this a little easier.

For `gnome-terminal` on Ubuntu (and some others):
- `CTRL+click` and drag to highlight a rectangle
- `CTRL+SHIFT+C` to copy to your clipboard

### Installing Dependencies

Building and using COLLASCII requires [the `ncurses` library](https://invisible-island.net/ncurses/).

On Ubuntu, you can install it with `sudo apt install libncurses5` (use `libncurses5-dev` if you're looking to develop).

### Building

`cd` to `src/` and run `make`. A `collascii` executable should be produced.

### Tests

Test files are built with [the `minunit` framework](https://github.com/siu/minunit), placed under `src/`, and named `library_test.c` for the `library` tested.

#### Running Tests

- `make test` to compile, run, and remove all tests
- `make .run-foo_test.c` to compile, run, and remove a specific test
- `make foo_test` to compile a specific test
