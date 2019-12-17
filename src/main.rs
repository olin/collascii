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
  i.window.mvaddch(0, cols as i32 - 1, 'B');
  i.window.mvaddch(rows as i32 - 1, cols as i32 - 1, 'C');
  i.window.refresh();
  // wait for input to quit
  i.window.getch();
  endwin();
  Ok(())
}

struct Interface {
  pub window: Window,
  status_h: usize,
  canvas: Canvas,
  view: (usize, usize),
}

impl Interface {
  pub fn new() -> Self {
    let window = initscr();
    let mut i = Interface {
      window,
      status_h: 2,
      canvas: Canvas::new(100, 100),
      view: (0, 0),
    };

    i.handle_resize();
    return i;
  }

  /// (rows, cols)
  fn size(&self) -> (usize, usize) {
    let (y, x) = self.window.get_max_yx();
    (y as usize, x as usize)
  }

  fn canvas_bounds(&self) -> (usize, usize) {
    let (rows, cols) = self.size();
    return (
      rows - (self.status_h + 1),
      cols)
  }

  fn draw_canvas(&self) {
    let (min_y, min_x) = self.view;
    let (max_y, max_x) = self.canvas_bounds();
    for y in min_y..max_y {
      for x in min_x..max_x {
        self.window.mvaddch(y as i32, x as i32, self.canvas.get(x, y));
      }
    }
  }

  pub fn redraw(&self) {
    let (rows, cols) = self.size();
    // draw boundary between canvas and status window
    self.window.mv((rows - self.status_h) as i32, 0);
    self.window.hline('-', cols as i32);
    self.window.refresh();
  }

  pub fn handle_resize(&mut self) {
    self.redraw();
  }
}
