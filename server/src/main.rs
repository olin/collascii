use std::io::{self, prelude::*};
use std::net::{TcpListener, TcpStream};

mod canvas;
use canvas::Canvas;

fn main() -> io::Result<()> {
    let mut canvas = Canvas::new(3, 3);

    canvas.insert("foobar");

    let listener = TcpListener::bind("127.0.0.1:5555")?;

    // accept connections and process them serially
    for stream in listener.incoming() {
        let mut stream = stream.unwrap();
        writeln!(stream, "{}", canvas);
    }
    Ok(())
}

fn manage_
