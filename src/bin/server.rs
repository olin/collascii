use std::io::{self, prelude::*};
use std::net::{TcpListener, TcpStream, Shutdown};
use std::sync::{Arc, Mutex};
use std::thread;
use std::collections::HashMap;
use std::fmt;

mod canvas;
use canvas::Canvas;

fn main() -> io::Result<()> {
    let mut canvas = Canvas::new(3, 3);

    canvas.insert("foobar");

    let canvas = Arc::new(Mutex::new(canvas));
    let clients = Arc::new(Mutex::new(Clients::new()));

    let listener = TcpListener::bind("127.0.0.1:5555")?;

    // accept connections and process them in parallel
    loop {
        let (stream, addr) = listener.accept().unwrap();
        println!("New client: {}", addr);
        let canvas = Arc::clone(&canvas);
        let uid = clients.add(stream.try_clone().unwrap().shutdown(Shutdown::Read));
        let clients = Arc::clone(&clients);

        thread::spawn(move || {
            handle_stream(stream, &canvas, &clients);
        });
    }
}

pub fn handle_stream(mut stream: TcpStream, canvas: &Mutex<Canvas>) {
    writeln!(stream, "{}", canvas.lock().unwrap()).unwrap();
    canvas.lock().unwrap().set(0, 1, 'X');
    thread::sleep_ms(5000);
}


/// Unique identifier of a client
type ClientUid = u8;

/// Queue of connected network clients
struct Clients {
    list: HashMap<ClientUid, TcpStream>,
}

impl Clients {
    pub fn new() -> Self {
        Clients {
            list: HashMap::new()
        }
    }

    /// Send a message to all clients
    pub fn broadcast(&mut self, msg: fmt::Arguments) -> io::Result<()> {
        for (uid, stream) in self.list.iter_mut() {
            stream.write_fmt(msg)?
        }
        Ok(())
    }

    /// Send a message to all clients but one (usually the sender)
    pub fn send(&mut self, client: ClientUid, msg: fmt::Arguments) -> io::Result<()> {
        for (&uid, stream) in self.list.iter_mut() {
            if uid == client {
                continue;
            }
            stream.write_fmt(msg)?
        }
        Ok(())
    }

    /// Add a client to the queue, returning the uid
    pub fn add(&mut self, client: TcpStream) -> ClientUid {
        let uid = self.get_new_uid();
        if self.list.insert(uid, client).is_some() {
            panic!("Uid should not exist in map!")
        }
        return uid;
    }

    /// Remove a client from the queue
    pub fn remove(&mut self, client: ClientUid) -> Option<TcpStream> {
        self.list.remove(&client)
    }

    /// Get a new uid for a client
    ///
    /// Warning: this will panic if the max uid is the maximum u8.
    fn get_new_uid(&self) -> ClientUid {
        match self.list.keys().max() {
            None => 1,
            Some(max_uid) => max_uid + 1,
        }
    }
}
