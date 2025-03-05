#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <functional>
#include <unordered_set>
#include <vector>

#include "primes.h"

// Helper function to generate reference prime numbers up to `limit`
std::unordered_set<int> GenerateReferencePrimes(int limit) {
  std::vector<bool> is_prime(limit + 1, true);
  std::unordered_set<int> primes;

  is_prime[0] = is_prime[1] = false;  // 0 and 1 are not primes
  for (int i = 2; i <= limit; ++i) {
    if (is_prime[i]) {
      primes.insert(i);
      for (int j = 2 * i; j <= limit; j += i) {
        is_prime[j] = false;
      }
    }
  }
  return primes;
}

// Test case
TEST(FindPrimesTest, GeneratesAllPrimesUpTo500000AndRespectsTermination) {
  constexpr int limit = 500000;
  constexpr int workers =
      4;  // Arbitrary number of workers for parallelism test

  std::unordered_set<int> found_primes;
  std::atomic<bool> finished_called(false);

  auto callback = [&](int prime) -> int {
    if (finished_called.load()) {
      ADD_FAILURE() << "Callback was called after returning -1";
      return -1;
    }
    if (prime == -1) {
      finished_called.store(true);
      return -1;
    }
    found_primes.insert(prime);
    return 0;  // Continue receiving primes
  };

  FindPrimes(limit, workers, callback);

  std::unordered_set<int> expected_primes = GenerateReferencePrimes(limit);

  EXPECT_EQ(found_primes, expected_primes);
}
