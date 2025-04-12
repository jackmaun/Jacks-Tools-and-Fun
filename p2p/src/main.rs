mod node;
mod network;
mod util;

use node::Node;
use network:: {start_server, send_message, Message};
use util::hash_id;
use tokio::task;
use plotters::prelude::*;
use std::f64::consts::PI;


fn vis_ring(nodes: &[Node], filename: &str) {
    let root = BitMapBackend::new(filename, (800, 800)).into_drawing_area();
    root.fill(&WHITE).unwrap();

    let center = (400.0, 400.0);
    let radius = 300.0;

    for(i, node) in nodes.iter().enumerated() {
        let angle = 2.0 * PI * (i as f64) / (nodes.len() as f64);
        let x = center.0 + radius * angle.cos();
        let y = center.1 + radius * angle.sin();

        root.draw(&Circle::new((x as i32, y as i32), 10, &BLUE)).unwrap();
        root.draw(&Text::new(
            format!("{}", node.id),
            (x as i32, y as i32 - 20),
            ("sans-serif", 15).into_font(),
        ))
        .unwrap();

        for &finger in &node.finger_table {
            let target_angle = 2.0 * PI * (finger as f64) / (nodes.len() as f64);
            let target_x = center.0 + radius * target_angle.cos();
            let target_y = center.1 + radius * target_angle.sin();

            root.draw(&PathElement::new(
                vec![(x, y), (target_x, target_y)],
                &BLACK,
            ))
            .unwrap();
        }
    }

    root.present().unwrap();
}

#[tokio::main]
async fn main() {
    let node1_id = hash_id("node1");
    let node2_id = hash_id("node2");
    let node3_id = hash_id("node3");

    let mut node1 = Node::new(node1_id);
    let mut node2 = Node::new(node2_id);
    let mut node3 = Node::new(node3_id);

    node1.add_finger(node2_id);
    node2.add_finger(node3_id);
    node3.add_finger(node1_id);

    let nodes = vec![node1.clone(), node2.clone(), node3.clone()];
    visualize_ring(&nodes, "chord_ring.png");
    println!("Chord ring visualization saved to chord_ring.png");

    task::spawn(async {
        start_server(8080).await.unwrap();
    });

    task::spawn(async {
        start_server(8081).await.unwrap();
    });

    tokio::time::sleep(std::time::Duration::from_secs(1)).await; // Wait for servers to start
    let message = Message {
        key: "key1".to_string(),
        value: Some("value1".to_string()),
    };

    send_message("127.0.0.1:8081", &message).await.unwrap();
}