use std::fmt;

use crate::canvas::Canvas;

use std::io::{self, BufRead};

/// A message sent between instances to modify a shared canvas.
enum Message {
    /// Set a character in the canvas
    SetChar { x: usize, y: usize, c: char },
    /// Replace the canvas
    CanvasUpdate { c: Canvas },
    // /// A new client has joined
    // ClientJoined,
    // /// A client has quit
    // ClientQuit,
    /// Exit message
    Quit,
}

impl Message {
    /// Parse a string and try to build a message from it.
    // pub fn from(s: &str) -> Option<Self> {
    //     let s = s.chars();
    //     use Message::{Quit, SetChar};
    //     let f = s.next().unwrap();
    //     if f == 's' {
    //         // TODO: parse and store values
    //         let (x, y, c) = (0, 1, 'a');
    //         Some(SetChar { x, y, c })
    //     } else if f == 'q' {
    //         Some(Quit)
    //     } else {
    //         None
    //     }
    // }

    fn parse_Quit(s: &str) -> Option<Self> {
        unimplemented!()
    }

    fn parse_SetChar(s: &str) -> Option<Self> {
        unimplemented!()
    }

    pub fn from_reader<R>(source: &mut R) -> Result<Self, io::Error>
    where
        R: BufRead,
    {
        let mut line = String::new();
        let parse_error = || io::Error::from(io::ErrorKind::InvalidData);
        let size = source.read_line(&mut line)?;
        if line.len() < 1 {
            return Err(parse_error());
        }
        let mut vals: Vec<&str> = line.split(' ').collect();
        let prefix = vals[0];

        match prefix {
            // SetChar
            "s" => {
                if vals.len() != 4 {
                    return Err(parse_error());
                }
                let y: usize = vals[1].parse().map_err(|_| parse_error())?;
                let x: usize = vals[2].parse().map_err(|_| parse_error())?;
                let c: char = vals[3].parse().map_err(|_| parse_error())?;
                Ok(Message::SetChar { y, x, c })
            }
            // CanvasUpdate
            "c" => {
                if vals.len() != 3 {
                    return Err(parse_error());
                }
                let height: usize = vals[1].parse().map_err(|_| parse_error())?;
                let width: usize = vals[2].parse().map_err(|_| parse_error())?;
                let mut canvas = Canvas::new(width, height);
                // load data into canvas
                let bytes_to_read = width * height;
                let mut buf = vec![0u8; bytes_to_read];
                source.read_exact(&mut buf);
                let buf = String::from_utf8(buf).map_err(|_| parse_error())?;
                canvas.insert(&buf);
                Ok(Message::CanvasUpdate { c: canvas })
            }
            // Quit
            "q" => Ok(Message::Quit),
            _ => Err(parse_error()),
        }
    }
}

impl Into<String> for Message {
    fn into(self) -> String {
        format!("{}", self)
    }
}

impl fmt::Display for Message {
    // use Message
    ///
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Message::SetChar { y, x, c } => write!(f, "s {} {} {}", y, x, c)?,
            Message::CanvasUpdate { c } => write!(f, "c {} {} \n{}", c.height(), c.width(), c)?,
            Message::Quit => write!(f, "q")?,
        }
        Ok(())
    }
}
