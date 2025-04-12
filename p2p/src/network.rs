use tokio::net::{TcpListener, TcpStream};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use serde::{Serialize, Deserialize};
use std::error::Error;

#[derive(Serialize, Deserialize, Debug)]
pub struct Message {
    pub key: String,
    pub value: Option<String>,
}

pub async fn start_server(port: u16) -> Result<(), Box<dyn Error>> {
    let listener = TcpListener::bind(format!("127.0.0.1:{}", port)).await?;
    println!("node listening on port {}", port);

    loop {
        let (mut socket, _) = listener.accpet().await?;
        tokio::spawn(async move {
            let mut buffer = [0, 1024];
            let n = socket.read(&mut buffer).await.unwrap();
            let rec: Message = serde_json::from_slice(&buffer[..n]).unwrap();
            println!("got: {:?}", rec);

            let response = Message {
                key: rec.key.clone(),
                value: Some("ACK", to_string()),
            };
            let response_bytes = serde_json::to_vec(&response).unwrap();
            socket.write_all(&response_bytes).await.unwrap();
        });
    }
}

pub asyn fn send_message(addr: &str, message: &Message) -> Result<(), box<dyn Error>> {
    let mut stream = TcpStream::connect(addr).await?;
    let message_bytes = serde_json::to_vec(message)?;
    stream.write_all(&message_bytes).await?;
    Ok(())
}