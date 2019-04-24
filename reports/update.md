# COLLASCII

_A collaborative ascii canvas_

Team members:
[Matthew Beaudouin-Lafon](https://github.com/MatthewBeaudouinLafon),
[Evan New-Schmidt](https://github.com/newsch),
[Adam Novotny](https://github.com/labseven)

## Project Goal

Our goal is to make a collaborative ascii drawing application, through a terminal interface. Think of it like the google docs of [asciiflow](http://asciiflow.com/), but in a terminal.
Why? Because it's funny, questionable, and a little bit artsy. Oh and a great learning experience too.

### MVP

We have reached some of our MVP vision:

The MVP is designed to be a relatively low bar to ensure we get to a good place.
- An editor where you move with arrow keys and press a character to change a cell.
  _We have a working interface that you can edit an ASCII canvas with._
- Featuring a status bar a la vim.
  _There's a status bar there but it doesn't tell you that much info yet. We don't have visual feedback for switching between modes yet._
- A client/server model that allows other people to connect to and edit your instance, given an IP and port.
  _We have a protocol defined and an example client/server, but it isn't integrated with the frontend and we're having second thoughts on the protocol._


### Done

Here is an actual screenshot of our application (terminal made smaller for demo):
```
┌────────────────────────────┐
│                            │
│                            │
│        hello world!        │
│                            │
│                            │
│                            │
├────────────────────────────┤
│Insert mode                 │
│                            │
└────────────────────────────┘
```
The top window is a canvas in which you can type in. The bottom window is a status bar, through which you can change input modes (insert, box, line, select, etc.).


The frontend of the application uses [ncurses](https://www.gnu.org/software/ncurses/) to draw to the terminal. This allows us to print any character anywhere in the terminal with abstracted windows. For Collasciii, we have a _canvas_ window in which the user draws, and a _status_ window for switching modes and changing options. When drawing, the input is written to our own canvas struct, which is then written to the ncurses canvas window. This gives us more flexibility in to handle changes from another user on the network.

A _mode_ is represented by its function (written in `fe_modes.h`), as well as an entry in the `MODE_ID` enum. They are stored in a function array such that the `MODE_ID` enum indexes into the array with the associated mode function, effectively mapping the enum to functions. This way, instead of running a big ugly switch statement in the main loop, we can keep track of the current mode and call the function through the function array.

_Function array_
```C
int (*mode_functions[])(State *, WINDOW *, WINDOW *) = {
    mode_picker,
    mode_insert,
    mode_free_line,
};
```

_Mode enum in the same order_
```C
typedef enum {
  MODE_PICKER,
  MODE_INSERT,
  MODE_FREE_LINE,

  // ^ add your mode above
  LAST,  // used to get number of elements
} Mode_ID;
```

_Call the correct function through the array_
```C
(main while loop)
    (other stuff)
    mode_functions[state->current_mode](state, canvas_win, status_win); // Mode function call
(end)
```

The frontend has a state struct to neatly keep track of variables during the main while loop. This includes the input character, previous input, cursor position and mode. This state is then passed into the mode function.

Our application mostly follows the [MVC](https://en.wikipedia.org/wiki/Model%E2%80%93view%E2%80%93controller) pattern.

The **Model** is a Canvas struct. It holds the all the ascii in a matrix. All changes are written to the canvas.

```c
typedef struct {
  int num_cols, num_rows;
  char **rows;
} Canvas;
```

The **Controller** is split up into a few files:
`cursor.c` and `fe_modes.c`. Cursor is knows its x and y position, and has functions to move it. `fe_modes` switches between input modes, and defines what each mode does. The most important mode is "insert mode" which writes the typed characters and moves the cursor forward.

Finally the file `frontend.c` wraps up all of these features and presents them in a **View**. It draws the windows, status bar, and canvas onto the terminal window, and listens for key presses.


### Doing

- **Integrating networking with the frontend** (Evan). I made an HTTP-esque protocol to send and request canvases between instances. Integration with the front end has been more complex than I thought, and we've talked about some better protocols for collaborating. With the most-recent pull request the frontend and network code use the same data structure to store the canvas, so the final steps for integrating the current protocol are to add a mutex for reads/writes, run the server as a separate thread, and run the client in the same thread as the frontend interface. _This will be done when [pull request #4](https://github.com/olin/SoftSysCollascii/pull/4) is merged in, and if we decide to use another protocol that will be another task._
- **File reading/writing** (Evan). I wrote some extensions of the canvas API for reading from and writing to files, but it needs to be integrated with the new interface framework and decisions need to be made regarding how canvases are used in the frontend. Currently there is one large canvas that you start in the center of, but a reasonable txt export/save should probably crop to the artwork. _Done when [pull request #23](https://github.com/olin/SoftSysCollascii/pull/4) is merged in._
- **UI for changing modes** (Matt). Right now there's a framework for different modes to be implemented, but there isn't an interface for the user to select them. After that, we’ll add a few modes such as free line (already mostly implemented), shapes (straight line, box, circle, etc.), fill, etc. _Done when a pull request is merged in implementing an interface for mode selection._


## Additional Features (ordered by priority)
This list of features is designed such that no matter where we stop, we'll be happy with what we have.
- Different modes to support more features (eg. box mode)
  - Drawing straight lines
  - Drawing boxes
  - Free line (move cursor and choose best ascii character to make a "smooth" line)
  - Fill with character (paint bucket)
- "Editing" layer for tools before committing them to the main canvas
- Select > copy/cut > paste
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

I'd like to work with 3rd-party libraries, implement a concurrent networked program, and learn how to write a terminal interface. I'm looking forward to writing thoughtful APIs that are used in multiple ways, and above all else I want to create something that people will enjoy using.

### Matt

I'm shooting for a project that says "Look I can write C, but not for important things". Learning goals include writing good apis, writing good C, designing a decent CLI, and learning about networking hands on.

### Adam

I want to make something vaguely useful, but more importantly something that sparks joy. Through this project I want to learn to connect many parts together (Model/Controller/Networking) on a nontrivial application.

## Links

- https://github.com/olin/softsyscollascii
- We use the [GitHub project board](https://github.com/olin/SoftSysCollascii/projects/1) to keep track of issues.
- ([The obsolete team Trello board lives here](https://trello.com/b/IIGIut0G/collascii))
