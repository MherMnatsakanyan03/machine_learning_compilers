#include <iostream>
#include <vector>

// External assembly function declaration
extern "C" void transpose_cab(float* input, float* output, int a, int b, int c);

int main() {
    const int a = 4;
    const int b = 8;
    const int c = 5;
    
    // Input tensor (cab)
    float matrix[a * b * c] = {0};

    for (int k = 0; k < c; ++k) {
        for (int i = 0; i < b; ++i) {
            for (int j = 0; j < a; ++j) {
                int idx = k*b*a + i * a + j;
                matrix[idx] = static_cast<float>(idx + 1);
            }
        }
    }
    
    // Output tensor (transposed cba)
    float result[a * b * c] = {0};

    std::cout << "Original Matrices:" << std::endl;
    for (int k = 0; k < c; ++k) {
        for (int i = 0; i < b; ++i) {
            for (int j = 0; j < a; ++j) {
                std::cout << matrix[k*b*a + i * a + j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    // Call the assembly function
    transpose_cab(matrix, result, a, b, c);

    std::cout << "\nTransposed Matrices:" << std::endl;
    for (int k = 0; k < c; ++k) {
        for (int i = 0; i < a; ++i) {
            for (int j = 0; j < b; ++j) {
                std::cout << result[k*a*b + i * b + j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}
