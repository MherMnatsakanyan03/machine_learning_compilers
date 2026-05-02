#include <omp.h>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>

typedef void (*gemm_kernel_func)(float const *, float const *, float *, uint k);

extern "C" {
void gemm_512_16_16(float const *a, float const *b, float *c, uint k);
}

void fill_random(float *arr, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    arr[i] = (float)drand48();
  }
}

float calc_max_diff(float const *a, float const *b, uint i_m, uint i_n,
                    uint i_ld) {
  float max_diff = 0;

  for (uint m = 0; m < i_m; m++) {
    for (uint n = 0; n < i_n; n++) {
      float diff = a[n * i_ld + m] - b[n * i_ld + m];
      diff = std::abs(diff);

      max_diff = std::max(max_diff, diff);
    }
  }

  return max_diff;
}

void gemm_ref_mnk(float const *a, float const *b, float *c, uint m, uint n,
                  uint k, uint lda, uint ldb, uint ldc) {
  // C = C + A * B
  for (uint j = 0; j < n; j++) {
    for (uint p = 0; p < k; p++) {
      // B is row-major: b[row * ldb + col]
      float b_val = b[p * ldb + j];

      for (uint i = 0; i < m; i++) {
        // A is col-major: a[col * lda + row]
        // C is col-major: c[col * ldc + row]
        c[j * ldc + i] += a[p * lda + i] * b_val;
      }
    }
  }
}

void benchmark_kernel(const std::string &kernel_name, gemm_kernel_func kernel,
                      float const *a, float const *b, float *c, float *c_ref,
                      uint m, uint n, uint k, uint reps) {

  std::cout << "Testing " << kernel_name << " kernel" << std::endl;

  // Assuming tightly packed matrices based on your earlier code
  uint lda = m;
  uint ldb = n; // B is row-major K x N, so leading dim is N
  uint ldc = m;

  // Run reference implementation
  gemm_ref_mnk(a, b, c_ref, m, n, k, lda, ldb, ldc);

  // Run assembly kernel
  kernel(a, b, c, k);

  // Check correctness
  float max_diff = calc_max_diff(c_ref, c, m, n, ldc);
  std::cout << "  maximum difference: " << max_diff << "\n";

  // Time the kernel
  auto start = std::chrono::steady_clock::now();
  for (uint rep = 0; rep < reps; rep++) {
    kernel(a, b, c, k);
  }
  auto stop = std::chrono::steady_clock::now();

  std::chrono::duration<double> duration =
      std::chrono::duration_cast<std::chrono::duration<double>>(stop - start);

  std::cout << "  duration: " << duration.count() << " seconds" << std::endl;

  // Calculate GFLOPS
  double gflops = reps;
  gflops *= m * n * k * 2; // 2 ops (multiply and add) per element
  gflops *= 1.0E-9;
  gflops /= duration.count();

  std::cout << "  GFLOPS: " << gflops << "\n\n";
}

// Notice a_all and b_all are no longer const here so we can fill them randomly!
void benchmark_multicore_gemm(const std::string &kernel_name, gemm_kernel_func kernel, 
                              float *a_all, float *b_all, float *c_all, float *c_ref_all,
                              uint total_blocks, uint m, uint n, uint k, uint reps) {
    
    std::cout << "Testing: " << kernel_name << " (Multi-Core)\n";
    std::cout << "Total 16x16 Blocks: " << total_blocks << "\n";

    fill_random(a_all, total_blocks * m * k);
    fill_random(b_all, total_blocks * k * n);
    fill_random(c_all, total_blocks * m * n);
    for (size_t i = 0; i < total_blocks * m * n; i++) {
        c_ref_all[i] = c_all[i]; // Sync C and C_ref
    }

    // Check just the first block to ensure math is correct
    gemm_ref_mnk(&a_all[0], &b_all[0], &c_ref_all[0], m, n, k, m, n, m);
    kernel(&a_all[0], &b_all[0], &c_all[0], k);
    
    float max_diff = calc_max_diff(&c_ref_all[0], &c_all[0], m, n, m);
    std::cout << "  maximum difference: " << max_diff << "\n";

    // benchmarking
    std::cout << "  Running benchmark on " << omp_get_max_threads() << " threads...\n";
    
    auto start = std::chrono::steady_clock::now();
    
    for (uint rep = 0; rep < reps; rep++) {
        #pragma omp parallel for
        for (uint b = 0; b < total_blocks; b++) {
            // Calculate starting pointers for this specific core's bloc
            float const *a_block = &a_all[b * (m * k)];
            float const *b_block = &b_all[b * (k * n)];
            float *c_block = &c_all[b * (m * n)];

            kernel(a_block, b_block, c_block, k);
        }
    }
    
    auto stop = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration = stop - start;

    // (Total Blocks) * (M * N * K) * (2 ops: multiply + add) * Repetitions
    double total_flops = (double)total_blocks * m * n * k * 2.0 * reps;
    double gflops = (total_flops / 1.0E9) / duration.count();

    std::cout << "  duration: " << duration.count() << " seconds\n";
    std::cout << "  GFLOPS:   " << gflops << "\n\n";
}

int main() {
  srand48(time(NULL));

  // TEST 1: The Single Core Kernel
  uint reps_single = 500000;

  float a[16 * 512];
  float b[512 * 16];
  float c[16 * 16];
  float c_ref[16 * 16];

  fill_random(a, 16 * 512);
  fill_random(b, 512 * 16);
  fill_random(c, 16 * 16);
  for (int i = 0; i < 16 * 16; i++)
    c_ref[i] = c[i]; 

  benchmark_kernel("gemm_512_16_16 (Single)", gemm_512_16_16, a, b, c, c_ref, 16,
                   16, 512, reps_single);


  // TEST 2: The Multi-Core Kernel
  uint total_blocks = 1024;
  uint reps_multi = 1000; 

  float *a_mc = new float[total_blocks * 16 * 512];
  float *b_mc = new float[total_blocks * 512 * 16];
  float *c_mc = new float[total_blocks * 16 * 16];
  float *c_ref_mc = new float[total_blocks * 16 * 16];

  benchmark_multicore_gemm("gemm_512_16_16", gemm_512_16_16, a_mc, b_mc, c_mc, c_ref_mc,
                           total_blocks, 16, 16, 512, reps_multi);

  // Clean up the heap memory
  delete[] a_mc;
  delete[] b_mc;
  delete[] c_mc;
  delete[] c_ref_mc;

  return EXIT_SUCCESS;
}