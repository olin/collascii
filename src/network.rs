//! Network protocol-related structures
use std::fmt;

use crate::canvas::Canvas;

use std::io::{self, BufRead};

/// A message sent between instances to modify a shared canvas.
#[derive(Debug, PartialEq, Clone)]
pub enum Message {
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
    /// Parse a readable buffer and try to build a message from it.
    pub fn from_reader<R>(source: &mut R) -> Result<Self, io::Error>
    where
        R: BufRead,
    {
        let mut line = String::new();
        let size = source.read_line(&mut line)?;
        let parse_error = |msg: &str| {
            io::Error::new(
                io::ErrorKind::InvalidData,
                format!("Parse Error: {}: {:?}", msg, line.clone()),
            )
        };
        if line.len() == 0 {
            return Ok(Message::Quit);
        }
        let vals: Vec<&str> = line.split_ascii_whitespace().collect();
        let prefix = vals[0];

        match prefix {
            // SetChar
            "s" => {
                if vals.len() != 4 {
                    return Err(parse_error(&format!(
                        "Expected 4 arguments for SetChar, got {}",
                        vals.len()
                    )));
                }
                let y: usize = vals[1]
                    .parse()
                    .map_err(|_| parse_error("Invalid y value"))?;
                let x: usize = vals[2]
                    .parse()
                    .map_err(|_| parse_error("Invalid x value"))?;
                let c: char = vals[3]
                    .parse()
                    .map_err(|_| parse_error("Invalid c value"))?;
                Ok(Message::SetChar { y, x, c })
            }
            // CanvasUpdate
            "cs" => {
                if vals.len() != 3 {
                    return Err(parse_error(&format!(
                        "Expected 4 arguments for CanvasUpdate, got {}",
                        vals.len()
                    )));
                }
                let height: usize = vals[1]
                    .parse()
                    .map_err(|_| parse_error("Invalid height value"))?;
                let width: usize = vals[2]
                    .parse()
                    .map_err(|_| parse_error("Invalid width value"))?;
                let mut canvas = Canvas::new(width, height);
                // load data into canvas
                let bytes_to_read = width * height;
                let mut buf = vec![0u8; bytes_to_read];
                source.read_exact(&mut buf);
                let buf = String::from_utf8(buf)
                    .map_err(|e| parse_error(&format!("Couldn't parse canvas contents: {}", e)))?;
                canvas.insert(&buf);
                Ok(Message::CanvasUpdate { c: canvas })
            }
            // Quit
            "q" => Ok(Message::Quit),
            _ => Err(parse_error("Unknown command")),
        }
    }
}

impl Into<String> for Message {
    fn into(self) -> String {
        format!("{}", self)
    }
}

impl fmt::Display for Message {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Message::SetChar { y, x, c } => writeln!(f, "s {} {} {}", y, x, c)?,
            Message::CanvasUpdate { c } => {
                writeln!(f, "cs {} {} \n{}", c.height(), c.width(), c.serialize())?
            }
            Message::Quit => writeln!(f, "q")?,
        }
        Ok(())
    }
}

#[cfg(test)]
mod test {
    use super::Canvas;
    use super::Message;

    /// Check parsing of individual messages
    #[test]
    fn parse() {
        // good test cases
        let mut c1 = Canvas::new(3, 2);
        c1.insert("X1234");
        let msg_test_cases = [
            // SetChar
            (Message::SetChar { y: 3, x: 2, c: 'a' }, "s 3 2 a"),
            (Message::SetChar { y: 1, x: 0, c: 'Z' }, "s 1 0 Z"),
            // Canvas
            (Message::CanvasUpdate { c: c1 }, "c 2 3\nX1234 "),
            // Quit
            (Message::Quit, "q"),
        ];

        // parse them individually
        for (expected, input) in msg_test_cases.iter() {
            let parsed = Message::from_reader(&mut input.as_bytes());
            assert!(parsed.is_ok());
            assert_eq!(expected, &parsed.unwrap());
        }

        // Concat all messages into a big stream and read it
        let (expecteds, inputs): (Vec<_>, Vec<_>) = msg_test_cases.iter().cloned().unzip();
        let blob = inputs.iter().fold(String::new(), |mut acc, input| {
            acc.push('\n');
            acc.push_str(input);
            acc
        });
        let mut reader = blob.as_bytes();
        for expected in expecteds.iter() {
            let parsed = Message::from_reader(&mut reader);
            assert!(parsed.is_ok());
            assert_eq!(expected, &parsed.unwrap());
        }
    }
}
