use rustfft::{FftPlanner, num_complex::Complex};
use std::sync::Arc;

pub fn fft(mut data: Vec<Complex<f64>>, inverse: bool) -> Vec<Complex<f64>> {
    let mut planner = FftPlanner::new();
    let fft = if inverse {
        planner.plan_fft_inverse(data.len())
    }
    else {
        planner.plan_fft_forward(data.len())
    };

    fft.process(&mut data);
    data
}

pub fn poly_mul(a: &[Complex<f64>], b: &[Complex<f64>]) -> Vec<Complex<f64>> {
    let size = a.len().max(b.len()) * 2;
    let mut a_ext = a.to_vec();
    let mut b_ext = b.to_vec();

    a_ext.resize(size, Complex::new(0.0, 0.0));
    b_ext.resize(size, Complex::new(0.0, 0.0));

    let a_fft = fft(a_ext, false);
    let b_fft = fft(b_ext, false);

    let result_fft: Vec<Complex<f64>> = a_fft.iter()
        .zip(&b_fft)
        .map(|(x, y)| x * y)
        .collect();

    fft(result_fft, true)
}

