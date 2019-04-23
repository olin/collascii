### Done

(backend)

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


### Doing

On the frontend, we’re currently working on the UI for changing modes. After that, we’ll add a few modes such as free line (already mostly implemented), shapes (straight line, box, circle, etc.), fill, etc.


_prompt:_
```
Your project update should answer the following questions (note that some are the same as in the proposal):


1) What is the goal of your project; for example, what do you plan to make, and what should it do?  Identify a lower bound you are confident you can achieve and a stretch goal that is more ambitious.

2) What are your learning goals; that is, what do you intend to achieve by working on this project?

3) What have you done to get started?  Have you found the resources you need, do you have a plan to find them, or do you need help?  If you found any resources you think I should add to the list on the class web page, please email them to me.

4) What are you working on now?  Describe at least three concrete tasks that you are working on, and identify which members of the team are working on them.  For each one, what is the "definition of done"; that is, what will you produce to demonstrate that the task is done?

Audience: Think of your update as a rough draft of your final report.  Target an external audience that wants to know what you did and why.  More specifically, think about students in future versions of SoftSys who might want to work on a related project.  Also think about people who might look at your online portfolio to see what you know, what you can do, and how well you can communicate.



Submission Mechanics

1) In your project report, you should already have a folder called "reports" that contains a Markdown document called "proposal.md".  Make a copy of "proposal.md" called "update.md"

2) At the top of this document, give your project a meaningful name that at least suggests the topic of the project.  Do not call it "Software Systems Project 2".  List the complete names of all members of the team.

3) Answer the questions in the Content section, above. Use typesetting features to indicate the organization of the document.  Do not include the questions as part of your document.

4) At the end of the document, include a link to your Trello board and a link to the repository on GitHub.

5) Complete your update, view it on GitHub, and copy the GitHub URL.  Then paste the URL in the submission space below.
```

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
  _There's a status bar there but it doesn't tell you that much info yet._
- A client/server model that allows other people to connect to and edit your instance, given an IP and port.
  _We have a protocol defined and an example client/server, but it isn't integrated with the frontend and we're having second thoughts on the protocol._


### Done

backend: servers clients and canvases (adam), oh my

- panning across larger canvases

frontend: ncurses, oh my (matt)




### Doing

Integrating networking with frontend: Evan

File reading/writing: Evan

Frontend mode switching, free line mode (matt)


## Additional Features (ordered by priority)
This list of features is designed such that no matter where we stop, we'll be happy with what we have.
- Different modes to support more features (eg. box mode)
  - Drawing straight lines
  - Drawing boxes
  - Free line (move cursor and choose best ascii character to make a "smooth" line)
  - Fill with character (paint bucket)
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

I want to make something vaguely useful, but more importantly something that sparks joy.
