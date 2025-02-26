mod fft;
mod swifft;

fn main() {
    let input_data: Vec<u64> = vec![42, 13, 88, 99, 5, 72, 37, 50, 29, 61, 7, 14, 20, 11, 9, 33,
                                     19, 80, 55, 23, 44, 78, 12, 90, 1, 5, 27, 31, 67, 45, 39, 22,
                                     15, 73, 58, 92, 21, 60, 83, 41, 3, 48, 36, 17, 66, 30, 62, 8,
                                     49, 6, 89, 53, 18, 24, 16, 56, 74, 26, 47, 34, 10, 95, 54, 87];

    let hash_result = swifft::swifft_hash(&input_data);

    println!("SWIFFT Hash Output: {:?}", hash_result);
}

