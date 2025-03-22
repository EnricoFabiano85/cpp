#include <numeric>
#include <vector>
#include <iostream>
#include <experimental/simd>

int main()
{
  std::vector<float> a = {1.0f, 2.0f, 3.0f, 4.0f};
  std::vector<float> b = {5.0f, 6.0f, 7.0f, 8.0f};
  std::vector<float> result(4);

  std::experimental::simd<float> simd_a(&a[0], std::experimental::element_aligned_tag{});
  std::experimental::simd<float> simd_b(&b[0], std::experimental::element_aligned_tag{});
  std::experimental::simd<float> simd_result = simd_a + simd_b;
  simd_result.copy_to(&result[0], std::experimental::element_aligned_tag{});

  for (float val : result) {
      std::cout << val << " "; // Output: 6 8 10 12
  }
  std::cout << std::endl;
}