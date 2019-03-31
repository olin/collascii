# COLLASCII

_A collaborative ascii canvas_

Team members:
[Matthew Beaudouin-Lafon](https://github.com/MatthewBeaudouinLafon),
[Evan New-Schmidt](https://github.com/newsch/)

## Project Goal

A terminal-based version of [asciiflow](http://asciiflow.com/).

### MVP

- An editor where you move with arrow keys and press a character to change a cell.
- A client/server model that allows other people to connect to and edit your instance, given an IP and port.
- Featuring a status bar a la vim.

### Additional Features (ordered by priority)

- Different modes to support more features (eg. box mode)
  - Drawing straight lines
  - Drawing boxes
  - Free line (move cursor and choose best ascii character to make a "smooth" line)
  - Fill with character (paint bucket)
- panning across larger canvases
- an r/theplace
- layers, bringing it closer still to industry standards like Photoshop™
- mouse support (`ncurses` supposedly makes this easier)
- colors
- Interactive tic-tac-toe with a physical device
  - draw lines, move cursor to one of 9 places, draw whole X or O
  - Enter Tic-Tac-Toe mode by drawing board?
- Importing images as ascii

## Learning Goals

### Evan

I'd like to work with 3rd-party libraries, implement a concurrent networked program, and learn how to write a terminal interface.

### Matt

I'm shooting for a project that says "Look I can write C, but not for important things". Learning goals include writing good apis, writing good C, designing a decent CLI, and learning about networking hands on. 

## What's Needed

- text editor (of sorts) for drawing and displaying the canvas
  - [this project](https://github.com/SelinaWang/SoftSysZis/blob/master/reports/report.md) built a text editor following [this guide](https://viewsourcecode.org/snaptoken/kilo/index.html) (they **did not** use `ncurses`)
  - [this project](https://github.com/vickymmcd/SillyString/blob/master/reports/report.md) used `ncurses` to build an interactive escape the room game and found [NCURSES introduction](https://invisible-island.net/ncurses/ncurses-intro.html) and [NCURSES tutorial](http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/) useful.
  - [this tutorial](https://cheukyin699.github.io/tutorial/c++/2015/02/01/ncurses-editor-tutorial-01.html) covers creating a text-editor in `ncurses`
- a network protocol for sending updates to the canvas between users
  - Right now my inclination is to work with an HTTP/[ARPA Internet Text Messages](https://tools.ietf.org/html/rfc822) related format over TCP, and go from there.

## First Steps

- Evan: Read Chapters 11 (Sockets and Networking) and 12 (Threads) of HFC for more info on networking in C. _Done when they're read and a written summary is commented in Trello._
- Evan: Write a spec and implementation of a single-threaded server/client that can send an ascii canvas back and forth. _Done when a pull-request with the code is approved and merged._