use std::fmt;

/// A message sent between instances to modify a shared canvas.
enum Message {
    /// Set a character in the canvas
    SetChar{x: usize, y: usize, c: char},
    /// Replace the canvas
    CanvasUpdate{c: &Canvas},
    /// A new client has joined
    ClientJoined,
    /// A client has quit
    ClientQuit,
    /// Exit message
    Quit,
}

impl Message {
    /// Parse a string and try to build a message from it.
    fn from(s: &str) -> Option<Self> {
        match s[0] == 's' {
            // TODO: parse and store values
            let (x, y, c) = (0, 1, 'a');
            Some(SetChar{x, y, c})
        } else {
            None
        }
    }
}

impl fmt::Display for Message {
    use Message
    ///
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Message::SetChar{y, x, c} => {
                write!("s {} {} {}", y, x, c)?
            },
            Quit => write!("q")?
        }
    }
}
