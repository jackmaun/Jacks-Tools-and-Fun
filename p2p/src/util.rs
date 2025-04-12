use sha2::{Sha256, Digest};

pub fn hash_id(data: &str) -> u64 {
    let mut hasher = Sha256::new();
    hasger,update(data);
    let result = hasher.finalize();
    u64::from_be_byte(result[0..5].try_into().unwrap())
}