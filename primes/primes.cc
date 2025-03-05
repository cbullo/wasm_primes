#include <atomic>
#include <deque>
#include <functional>
#include <thread>
#include <vector>

// TODO: Get rid of Emscripten here
#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif
#include "primes_worker.h"

void SpawnWorkers(int number_of_workers, int limit,
                  std::atomic<int> &next_value,
                  std::vector<std::atomic<StorageType>> &primes_storage,
                  RingBuffer<int> &output_buffer) {
  for (int i = 0; i < number_of_workers; ++i) {
    std::thread(PrimesWorker, limit, std::ref(next_value),
                std::ref(primes_storage), std::ref(output_buffer))
        .detach();
  }

  // TODO: Move to a higher-level
#ifdef __EMSCRIPTEN__
  // Emscripten is not POSIX compatible - execution needs to return to the main
  // loop for the threads to actually start.
  emscripten_sleep(1);
#endif
}

void FindPrimes(int limit, int number_of_workers,
                std::function<void(int)> prime_callback) {
  int array_size = limit / sizeof(StorageType) + 1;
  static_assert(std::atomic<StorageType>::is_always_lock_free);
  std::vector<std::atomic<StorageType>> primes_storage(array_size);
  std::atomic<int> next_value = 2;

  RingBuffer<int> output_buffer;
  std::deque<int> primes_queue;

  SpawnWorkers(number_of_workers, limit, next_value, primes_storage,
               output_buffer);

  int finished_workers = 0;
  int last_processed = 2;

  while (true) {
    int candidate_prime;
    // Empty the buffer as quickly as possible.
    while (output_buffer.Pop(candidate_prime)) {
      if (candidate_prime == kFinished) {
        ++finished_workers;
      } else {
        primes_queue.push_back(candidate_prime);
      }
    }

    if (!primes_queue.empty()) {
      candidate_prime = primes_queue.front();
      primes_queue.pop_front();

      while (last_processed <= candidate_prime) {
        int storage_index = StorageIndex(last_processed);
        int bit_index = BitIndex(last_processed);

        if (!(primes_storage[storage_index] & (1 << bit_index))) {
          prime_callback(last_processed);
        }
        ++last_processed;
      }
    }

    if (finished_workers == number_of_workers && primes_queue.empty()) {
      prime_callback(-1);
      break;
    }
  }
}
