# COLLASCII

[![Build Status](https://travis-ci.com/olin/collascii.svg?branch=master)](https://travis-ci.com/olin/collascii)

_A collaborative ascii canvas_

[![asciicast](https://asciinema.org/a/248508.svg)](https://asciinema.org/a/248508)

For more examples of what's possible, [check out issue #21](https://github.com/olin/SoftSysCollascii/issues/21#partial-timeline).
Add your own creations!

## Getting Started

You can **download a `collascii` executable from the [Releases page](https://github.com/olin/SoftSysCollascii/releases)**
or build your own version locally (see [Building](#building)).

COLLASCII currently supports Linux and MacOS (and Windows, using [WSL](https://docs.microsoft.com/en-us/windows/wsl/about)),
but it should run anywhere that NCURSES, BSD sockets, and POSIX threads do.

### Usage

Run the executable:

```shell
./collascii
```

This will open the editor view. Move the cursor with the arrow keys, and type to
insert text. Switch between input modes with `<TAB>`, and exit with `<CTRL+C>`.

To export your art, you can save it a file with `<CTRL+S>`, or copy it off of
the screen. Most terminals support some sort of block select, which makes this a
little easier.

For `gnome-terminal` on Ubuntu (and some others):

- `CTRL+click`, drag, and release to highlight a block of text
- `CTRL+SHIFT+C` to copy to your clipboard
- paste it wherever you want! (including within COLLASCII)

### Installing Dependencies

Building and using COLLASCII requires [the NCURSES library](https://invisible-island.net/ncurses/).

On Ubuntu, you can install it with `sudo apt install libncurses5` (use
`libncurses5-dev` if you're looking to develop).

### Building

`cd` to `src/` and run `make`. A `collascii` executable should be produced.

### Development

_First, a word on organization:_ source code lives in `src/`, class reports live
in `reports/`.

#### `DEBUG`

`#ifdef DEBUG` is used throughout the project. It can be enabled globally by
defining `DEBUG` in the `Make` environment, in Bash:

```shell
DEBUG=1 make collascii
```

or Fish:

```shell
env DEBUG=1 make collascii
```

This will also turn off compiler optimization and add debugging info to the
executable.

**NOTE: Run `make clean` before you enable this for the first time - `make`
won't know to recompile the source code hasn't changed.**

Because the NCURSES interface uses the terminal, any printing to `stdout` and
`stderr` will normally write directly onto the window in an unpleasant manner.
We've come up with a couple of workarounds: when `DEBUG` or `LOG_TO_FILE` is
`#define`d for `frontend.c`, `stderr` is reconnected to the file `out.txt`, so
text output can still be read without having to `fprintf` to a custom file.

This output can be read live from another terminal with the command:

```shell
tail -f out.txt
```

There are two macros defined in [`util.h`](`util.h`) to help with getting output
to `stderr`, `eprintf` and `logd`.

- `eprintf` is analogous to `printf` except it prints to `stderr` instead of
  `stdout`.
- `logd` behaves like `eprintf` when `DEBUG` is defined, but does nothing if it
  isn't, so you can easily toggle debug statements without overwriting the
  screen.

### Tests

Test files are built with [the `minunit` framework](https://github.com/siu/minunit),
placed under `src/`, and named `library_test.c` for the `library` tested.

#### Running Tests

- `make test` to compile, run, and remove all tests
- `make .run-foo_test.c` to compile, run, and remove a specific test
- `make foo_test` to compile a specific test
