<!-- TODO: double-check for typos -->
# COLLASCII: A collaborative ascii canvas

_The Future Editor of Yesterday, Tomorrow_


Team members:
[Matthew Beaudouin-Lafon](https://github.com/MatthewBeaudouinLafon),
[Evan New-Schmidt](https://github.com/newsch),
[Adam Novotny](https://github.com/labseven)

## Overview

We made a collaborative ascii drawing application with a terminal interface. Think of it like the google docs of [asciiflow](http://asciiflow.com/), but in a terminal.
Why? Because it's funny, questionable, and a little bit artsy. Oh and a great learning experience too.

#### Featuring:

We are pleased to present a fully-featured collaborative ascii editor, that enables you to create beautiful walls of text.

<!-- TODO: Embed Video Here -->

Highlighted features:
- Collaboration over the network
- File read/write to import/export artwork
- 3 drawing modes:
  - Insert: Type characters onto the canvas
  - Line drawing: make continuous lines with arrow keys
  - Brush: use mouse or arrows to paint onto the canvas
- A slick API for implementing new modes


#### Synergetic Resources:
These are a few applications that interface well with collascii:
* [Cowsay](https://en.wikipedia.org/wiki/Cowsay)
* [Figlet](https://www.askapache.com/online-tools/figlet-ascii/)
* [Svgbob](https://github.com/ivanceras/svgbob)
* [Asciifier](https://github.com/newsch/asciifier)

Please let us know of any projects that we should know about.


<!-- TODO: update this w/ the latest and greatest -->
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

## Selected Walkthroughs

The frontend of the application uses [NCURSES](https://www.gnu.org/software/ncurses/) to draw to the terminal. This allows us to print any character anywhere in the terminal with a coordinate system and abstracted "windows". For Collascii, we have a _canvas_ window in which the user draws, and a _status_ window for switching modes and changing options. When drawing, the input is written to our own `Canvas` struct, which is then drawn to the NCURSES canvas window. Separating the model and view gives us flexibility to handle special input modes and changes from networked users.

A _mode_ is represented by its function (written in `fe_modes.h`), name, and an entry in the `MODE_ID` enum. They are stored in an array such that the `MODE_ID` enum indexes into the array with the associated mode function, effectively mapping the enum to functions. This way, instead of running a big ugly switch statement in the main loop, we can keep track of the current mode and call mode functions in a standardized way.

Mode functions are called for three reasons:
- Once when initially selected by the user, to set up any necessary structures and update the visual interface.
- When the mode is selected and a new keypress is ready for interpretation.
- When the function is switched away from by the user.

Each mode function is called with two parameters, a `reason_t` enum that captures the three cases mentioned above, and the `State` struct with the current state, which the function can read and write to. A mode's main way of interfacing is by changing the state and exiting, but they also also write to the `mode_win` `msg_win`

Looking at the relevant section of `fe_modes.h`, we can see several components of this system:
- the `reason_t` enum mentioned above
- the mode function prototype, which ensures that all mode functions are the same type
- declarations of mode functions using the protoype
- a struct definition for mode information
- the external declaration of the function array, built from the mode info type

_`fe_modes.h` declarations:_
```C
typedef enum { START, NEW_KEY, END } reason_t;

// Prototype for mode functions. Note that this is NOT a function pointer type
// (use `mode_function_t*` for that). https://stackoverflow.com/a/5195682
typedef int mode_function_t(reason_t, State *);

mode_function_t mode_picker;
mode_function_t mode_insert;
mode_function_t mode_pan;
mode_function_t mode_free_line;
mode_function_t mode_brush;

typedef struct {
  char *name;
  char *description;
  mode_function_t *mode_function;
} editor_mode_t;

extern editor_mode_t modes[];
```

_Definition of the modes array in `fe_modes.c`:_
```C
editor_mode_t modes[] = {
  {"Mode Selector", "", mode_picker},
  {"Insert", "", mode_insert},
  {"Pan", "", mode_pan},
  {"Free-Line", "", mode_free_line},
  {"Brush", "", mode_brush},
};
```

_Separately, the `Mode_ID` enum used to to select functions, in the same order:_
```C
typedef enum {
  MODE_PICKER,
  MODE_INSERT,
  MODE_PAN,
  MODE_FREE_LINE,
  MODE_BRUSH,

  // ^ add your mode above
  LAST,  // used to get number of elements
} Mode_ID;
```

Passing keypress events from NCURSES and providing them to the relevant mode is handled by the function `master_handler`. It also intercepts global commands (like mode switching and file read/write).


The frontend has a `State` struct to neatly keeps track of variables inside the main while loop. This includes the input character, previous input, cursor position and mode. This state is then passed into the mode function.

Our application mostly follows the [MVC](https://en.wikipedia.org/wiki/Model%E2%80%93view%E2%80%93controller) pattern.

In addition to the `State`, our **model** is stored `Canvas` struct. `Canvas` holds the all the characters in a matrix, and has an API for reading/writing it.

```c
typedef struct {
  int num_cols, num_rows;
  char **rows;
} Canvas;
```

The **Controller** is split up into a few files:
- `cursor.c`, `fe_modes.c`. Cursor knows its x and y position, and has functions to move it. `fe_modes` switches between input modes, and defines what each mode does. The most important mode is "insert mode" which writes the typed characters and moves the cursor forward.
- The network client (`network.c`) also modifies the canvas based on commands from the server. The `network` and `stdin` streams are monitored and processed by a single thread.

Finally the file `frontend.c` wraps up all of these features and presents them in a **View**. It draws the windows, status bar, and canvas onto the terminal window, and listens for key presses.

#### Networking

Networking was heavily inspired by ["Simple Chat Server in C"](https://github.com/yorickdewid/Chat-Server/blob/master/chat_server.c) and ["Simple Chat Client in C"](https://github.com/dtolj/simple-chat-client-server).

The protocol has only a few functions:
- `s y x c` sets the spot `(x,y)` to the character `c`, and sends this to all other clients (It is `y x` to standardize to ncurses)
- `c` makes the server respond with the entire canvas
- `q` closes the connection and makes the sessions offline only

The chat server has the master copy of the canvas, and tracks changes as all clients update it with the set command. It sends a serialized canvas to new clients. The set command sends updates to all clients (including the author) to preserve a consistent timeline of canvas changes. The commands are sent over a TCP socket delimeted by a newline. With [fdopen()](https://linux.die.net/man/3/fdopen) we can treat the socket as a file and use convenient functions like getline().

## Future
We found this project very exciting and are happy with the current state of the project. There are many unresolved [issues](https://github.com/olin/SoftSysCollascii/issues) on the github repo, and we have kept adding feature requests and bug reports.

## Learning Goals Reflection

### Evan

> I'd like to work with 3rd-party libraries, implement a concurrent networked program, and learn how to write a terminal interface. I'm looking forward to writing thoughtful APIs that are used in multiple ways, and above all else I want to create something that people will enjoy using.

I have found this project very motivating. Implementing features that were useful (and things I wanted to use) was  very empowering. Designing and writing internal APIs in the context of the larger program was also pretty fun. It was very rewarding to make something useful, and see it grow in ability and complexity. I got to use my networking knowledge for an initial implementation, but for the latter half I enjoyed learning how to use NCURSES and modify our internal APIs to advance the project.


### Matt

> I want to make something that says "Look I can write C, but not for important things". Learning goals include writing good apis, writing good C, designing a decent CLI, and learning about networking hands on.

I think the final result matches my first goal fairly well. If you're reading this and think "boy we should hire this guy to write all of our C", then I suppose I did not. In all seriousness, I'm quite proud of the final result. It's surprisingly fun to mess around with, and I enjoy talking about it.
From a technical standpoint, I got what I wanted out of doing this project. At the nuts and bolts level, I noticed that we didn't have to constantly refactor everything, which is a sign that we designed our apis decently well from the start. We didn't quite reach my grand ideas for a beautiful command line interface, but it's usable enough to get the point across. While I didn't touch much of the networking code myself, talking about the implementation with Evan and Adam sufficiently scratched my learning itch. On the meta software engineering level, this project was a good exercise in using a good GitHub workflow. In summary, this is a project I'm pretty proud of and it definitely solidified my knowledge of C.

### Adam

> I want to make something vaguely useful, but more importantly something that sparks joy. Through this project I want to learn to connect many parts together (Model/Controller/Networking) on a nontrivial application.

I found great motivation by working on a project that sparks joy. Seeing my edits improve the interface, make development smoother, and create a functional collaborative editing has been very motivating. I was fascinated by the different ways people solve consistency in collaborative text editing (seems like [Operational Transformations](http://www.codecommit.com/blog/java/understanding-and-applying-operational-transformation) are the future.), though we did not need to implement anything more complicated than a queue. I have a much better understanding about what makes go a compelling language to use, and am excited to go back to the future with memory management and list comprehension. As a group, we adhered to effective software development strategies that otherwise tend to slip during final project time. Maintaining a structured git workflow, refactoring code, and making sure everything is properly documented helped us stay productive and it made me feel good to be making a quality product. I am excited to show off collascii and see what uses it inspires.

## Links

### Learning
- Guide to NCURSES that was very useful: https://www.tldp.org/HOWTO/NCURSES-Programming-HOWTO/index.html
- NCURSES manual (not always on the first page of search results): https://invisible-island.net/ncurses/man/ncurses.3x.html
-

### Project-Related

- https://github.com/olin/softsyscollascii
- We used a [GitHub project board](https://github.com/olin/SoftSysCollascii/projects/1) to keep track of progress.
- ([The obsolete team Trello board lives here](https://trello.com/b/IIGIut0G/collascii))
