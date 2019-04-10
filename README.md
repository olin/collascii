# COLLASCII

_A collaborative ascii canvas_

## Getting Started

Source code lives in `src/`, class reports live in `reports/`.

### Installing Dependencies

Building COLLASCII requires [the `ncurses` library](https://invisible-island.net/ncurses/).

On Ubuntu, you can install it with `sudo apt install libncurses5-dev`.

### Building

`cd` to `src/` and run `make`.

### Tests

Test files are built with [the `minunit` framework](https://github.com/siu/minunit), placed under `src/`, and named `library_test.c` for the `library` tested.

#### Running Tests

- `make test` to compile, run, and remove all tests
- `make .run-foo_test.c` to compile, run, and remove a specific test
- `make foo_test` to compile a specific test
