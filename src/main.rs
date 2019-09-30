use std::io;

extern crate pancurses;
use pancurses::{Window, endwin, initscr};

mod canvas;
use canvas::Canvas;

fn main() -> io::Result<()> {
  let i = Interface::new();
  // check bounds
  let (rows, cols) = i.size();
  i.window.mvaddch(0, 0, 'A');
  i.window.mvaddch(0, cols - 1, 'B');
  i.window.mvaddch(rows - 1, cols - 1, 'C');
  i.window.refresh();
  // wait for input to quit
  i.window.getch();
  endwin();
  Ok(())
}

struct Interface {
  pub window: Window,
  status_h: i32,
}

impl Interface {
  pub fn new() -> Self {
    let window = initscr();
    let mut i = Interface {
      window,
      status_h: 2,
    };

    i.handle_resize();
    return i;
  }

  /// (rows, cols)
  fn size(&self) -> (i32, i32) {
    self.window.get_max_yx()
  }

  fn canvas_bounds(&self) -> (i32, i32) {
    let (rows, cols) = self.size();
    return (rows - (self.status_h + 1), cols)
  }

  pub fn redraw(&self) {
    let (rows, cols) = self.size();
    // draw boundary between canvas and status window
    self.window.mv(rows - self.status_h, 0);
    self.window.hline('-', cols);
    self.window.refresh();
  }

  pub fn handle_resize(&mut self) {
    self.redraw();
  }
}
