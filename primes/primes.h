#include <functional>

void find_primes(int limit, int number_of_workers,
                 std::function<void(int)> prime_callback);