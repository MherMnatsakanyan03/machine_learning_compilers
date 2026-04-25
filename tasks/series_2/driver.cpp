#include <chrono>
#include <cmath>
#include <iostream>
#include <string>

typedef void (*gemm_kernel_func)(float const *, float const *, float *, uint k);

extern "C" {
void gemm_1_32_6(float const *a, float const *b, float *c, uint k);
void gemm_64_32_6(float const *a, float const *b, float *c, uint k);
void gemm_1_31_6(float const *a, float const *b, float *c, uint k);
void gemm_64_31_6(float const *a, float const *b, float *c, uint k);
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

int main() {
  srand48(time(NULL));
  uint reps = 50000;

  // =========================================================
  // TEST 1: The K=1 Micro-Kernel
  // =========================================================
  float a_k1[32 * 1];
  float b_k1[1 * 6];
  float c_k1[32 * 6];
  float c_ref_k1[32 * 6];

  fill_random(a_k1, 32 * 1);
  fill_random(b_k1, 1 * 6);
  fill_random(c_k1, 32 * 6);
  for (int i = 0; i < 32 * 6; i++)
    c_ref_k1[i] = c_k1[i]; // Sync C and C_ref

  benchmark_kernel("gemm_32_6_1", gemm_1_32_6, a_k1, b_k1, c_k1, c_ref_k1, 32,
                   6, 1, reps);

  // =========================================================
  // TEST 2: The K=64 Kernel
  // =========================================================
  float a_k64[32 * 64];
  float b_k64[64 * 6];
  float c_k64[32 * 6];
  float c_ref_k64[32 * 6];
  reps = 5000;

  fill_random(a_k64, 32 * 64);
  fill_random(b_k64, 64 * 6);
  fill_random(c_k64, 32 * 6);
  for (int i = 0; i < 32 * 6; i++)
    c_ref_k64[i] = c_k64[i]; // Sync C and C_ref

  benchmark_kernel("gemm_32_6_64", gemm_64_32_6, a_k64, b_k64, c_k64, c_ref_k64,
                   32, 6, 64, reps);

  // =========================================================
  // TEST 3: The K=1, a=31 Micro-Kernel
  // =========================================================
  float a_k1_31[31 * 1];
  float b_k1_31[1 * 6];
  float c_k1_31[31 * 6];
  float c_ref_k1_31[31 * 6];
  reps = 50000;

  fill_random(a_k1_31, 31 * 1);
  fill_random(b_k1_31, 1 * 6);
  fill_random(c_k1_31, 31 * 6);
  for (int i = 0; i < 31 * 6; i++)
    c_ref_k1_31[i] = c_k1_31[i]; // Sync C and C_ref

  benchmark_kernel("gemm_31_6_1", gemm_1_31_6, a_k1_31, b_k1_31, c_k1_31,
                   c_ref_k1_31, 31, 6, 1, reps);

  // =========================================================
  // TEST 4: The K=64, a=31 Kernel
  // =========================================================
  float a_k64_31[31 * 64];
  float b_k64_31[64 * 6];
  float c_k64_31[31 * 6];
  float c_ref_k64_31[31 * 6];
  reps = 5000;

  fill_random(a_k64_31, 31 * 64);
  fill_random(b_k64_31, 64 * 6);
  fill_random(c_k64_31, 31 * 6);
  for (int i = 0; i < 31 * 6; i++)
    c_ref_k64_31[i] = c_k64_31[i]; // Sync C and C_ref

  benchmark_kernel("gemm_31_6_64", gemm_64_31_6, a_k64_31, b_k64_31, c_k64_31,
                   c_ref_k64_31, 31, 6, 64, reps);

  return EXIT_SUCCESS;
}
