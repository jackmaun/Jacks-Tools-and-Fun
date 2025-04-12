use rand::Rng;
use rustfft::num_complex::Complex;
use crate::fft::poly_mul;

const MODULUS: u64 = 257;  // small prime
const VECTOR_SIZE: usize = 64;

// random matrix
fn generate_matrix() -> Vec<Vec<Complex<f64>>> {
    let mut rng = rand::thread_rng();
    (0..VECTOR_SIZE).map(|_| {
        (0..VECTOR_SIZE).map(|_| {
            Complex::new(rng.gen_range(0.0..MODULUS as f64), 0.0)
        }).collect()
    }).collect()
}

// SWIFFT of an input vector
pub fn swifft_hash(input: &[u64]) -> Vec<u64> {
    let matrix = generate_matrix();
    let mut transformed: Vec<Complex<f64>> = vec![Complex::new(0.0, 0.0); VECTOR_SIZE];

    for i in 0..VECTOR_SIZE {
        let row_poly = &matrix[i];
        let input_poly: Vec<Complex<f64>> = input.iter()
            .map(|&x| Complex::new(x as f64, 0.0))
            .collect();

        let result_poly = poly_mul(row_poly, &input_poly);
        transformed[i] = result_poly.iter().sum();  // sum the FFT results
    }

    // mod to get hash
    transformed.iter().map(|c| (c.re as u64) % MODULUS).collect()
}

