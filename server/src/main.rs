use std::io::{self, prelude::*};
use std::net::{TcpListener, TcpStream};
use std::sync::{Arc, Mutex};
use std::thread;

mod canvas;
use canvas::Canvas;

fn main() -> io::Result<()> {
    let mut canvas = Canvas::new(3, 3);

    canvas.insert("foobar");

    let canvas = Arc::new(Mutex::new(canvas));

    let listener = TcpListener::bind("127.0.0.1:5555")?;

    // accept connections and process them in parallel
    loop {
        let (stream, addr) = listener.accept().unwrap();
        println!("New client: {}", addr);
        let canvas = Arc::clone(&canvas);

        thread::spawn(move || {
            handle_stream(stream, &canvas);
        });
    }
}

pub fn handle_stream(mut stream: TcpStream, canvas: &Mutex<Canvas>) {
    writeln!(stream, "{}", canvas.lock().unwrap()).unwrap();
    canvas.lock().unwrap().set(0, 1, 'X');
    thread::sleep_ms(5000);
}
