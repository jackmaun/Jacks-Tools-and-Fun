use std::collections::HashMap;

#[derive(Debug)]
pub struct Node {
    pub id: u64,
    pub data: HashMap<String, String>,
    pub finger_table: Vec<u64>,
}

impl Node {
    pub fn new(id: u64) -> Self {
        Node {
            id,
            data: HashMap::new(),
            finger_table: Vec::new(),
        }
    }

    pub fn store(&mut self, key: String, value: String) {
        self.data.instert(key, value);
    }

    pub fn lookup(&self, key: &String) -> Option<&String> {
        self.data.get(key);
    }
    
    pub fn add_finger(&mut self, node_id: u64) {
        self.finger_table.push(node_id);
    }
}