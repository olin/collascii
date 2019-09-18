use std::vec::Vec;
use std::fmt;


pub struct Canvas {
    width: usize,
    height: usize,
    rows: Vec<Vec<char>>,
}

impl Canvas {
    pub fn new(width: usize, height: usize) -> Self {
        let fill = ' ';  // initial character to fill canvas with
        let mut rows = Vec::with_capacity(height as usize);
        for _ in 0..height {
            let mut v = Vec::with_capacity(width as usize);
            v.resize(width, fill);
            rows.push(v);
        }
        Canvas {
            width,
            height,
            rows,
        }
    }

    pub fn width(&self) -> usize {
        self.width
    }

    pub fn height(&self) -> usize {
        self.width
    }

    pub fn get(&self, x: usize, y: usize) -> char {
        assert!(self.is_in(x,y));
        self.rows[y][x]
    }

    pub fn set(&mut self, x: usize, y: usize, val: char) {
        assert!(self.is_in(x,y));
        self.rows[y][x] = val;
    }

    fn is_in(&self, x: usize, y: usize) -> bool {
        x < self.width && y < self.height
    }

    pub fn insert(&mut self, s: &str) -> usize {
        let mut x = 0;
        let mut y = 0;
        let mut count = 0;
        for c in s.chars() {
            if x >= self.width() {
                x = 0;
                y += 1;
            }
            if y >= self.height() {
                break;
            }
            self.set(x, y, c);
            x += 1;
            count += 1;
        }
        count
    }
}

impl fmt::Display for Canvas {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        for (i, row) in self.rows.iter().enumerate() {
            for cell in row {
                write!(f, "{}", cell)?
            }
            if i < self.height - 1 {
                write!(f, "\n")?
            }
        }
        Ok(())
    }
}

// TODO
// impl From<&str> for Canvas {
//     pub fn from(s: &str) -> self {

//     }
// }

#[cfg(test)]
mod test {
    use super::Canvas;

    #[test]
    fn basics() {
        let mut c = Canvas::new(3, 4);
        // set upper left corner
        c.set(0, 0, 'A');
        assert_eq!('A', c.get(0, 0));

        // set lower right corner
        c.set(2, 3, 'B');
        assert_eq!('B', c.get(2, 3));
    }

    #[test]
    fn insert() {
        let s = "ABCDEFGH";

        let mut small = Canvas::new(2, 2);
        assert_eq!(4, small.insert(s), "Input string should be truncated");
        assert_eq!('A', small.get(0, 0));
        assert_eq!('B', small.get(1, 0));
        assert_eq!('C', small.get(0, 1));
        assert_eq!('D', small.get(1, 1));

        let mut large = Canvas::new(3, 3);
        assert_eq!(8, large.insert(s));
        let coords = [
            ('A', 0, 0),
            ('C', 2, 0),
            ('H', 1, 2),
            (' ', 2, 2),
        ];
        for &(c, x, y) in coords.into_iter() {
            assert_eq!(c, large.get(x, y), "wrong value at ({}, {})", x, y)
        }

        let mut just_right = Canvas::new(4, 2);
        assert_eq!(8, just_right.insert(s));
        let coords = [
            ('A', 0, 0),
            ('C', 2, 0),
            ('H', 3, 1),
        ];
        for &(c, x, y) in coords.into_iter() {
            assert_eq!(c, just_right.get(x, y), "wrong value at ({}, {})", x, y)
        }
    }
}
