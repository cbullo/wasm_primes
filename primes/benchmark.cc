#include <iostream>
#include <chrono>
#include <functional>
#include <vector>
#include <atomic>

#include "primes.h"

void BenchmarkFindPrimes() {
    const int limit = 50000000;
    // List of thread counts to benchmark.
    std::vector<int> thread_counts = {1, 2, 4, 8, 16};

    // Iterate over each thread count.
    for (int workers : thread_counts) {
        // Reset the prime counter.
        std::atomic<size_t> prime_count{0};

        // Create a callback that increments the counter for each prime.
        auto callback = [&](int prime) -> int {
            if (prime == -1) {
                // Termination signal received.
                return -1;
            }
            ++prime_count;
            return 0; // Continue processing.
        };

        // Start timing.
        auto start = std::chrono::steady_clock::now();

        // Call the FindPrimes function.
        FindPrimes(limit, workers, callback);

        // Stop timing.
        auto end = std::chrono::steady_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        // Output the results.
        std::cout << "Workers: " << workers
                  << " | Time: " << elapsed_ms << " ms"
                  << " | Primes found: " << prime_count.load() << std::endl;
    }
}

int main() {
    BenchmarkFindPrimes();
    return 0;
}
