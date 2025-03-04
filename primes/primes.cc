#include <atomic>
#include <deque>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

#include "emscripten.h"
#include "primes_worker.h"

std::vector<std::thread> spawn_workers(
    int number_of_workers, int limit, std::atomic<int> &next_value,
    std::vector<std::atomic<StorageType>> &primes_storage,
    RingBuffer<int> &output_buffer) {
  std::vector<std::thread> thread_pool;
  for (int i = 0; i < number_of_workers; ++i) {
    thread_pool.push_back(
        std::thread(primes_worker, limit, std::ref(next_value),
                    std::ref(primes_storage), std::ref(output_buffer)));
  }

  emscripten_sleep(0);
  return thread_pool;
}

void join_workers(std::vector<std::thread> &thread_pool) {
  for (auto &thread : thread_pool) {
    thread.join();
  }
}

void find_primes(int limit, int number_of_workers,
                 std::function<void(int)> prime_callback) {
  int array_size = limit / sizeof(StorageType) + 1;
  static_assert(std::atomic<StorageType>::is_always_lock_free);
  std::vector<std::atomic<StorageType>> primes_storage(array_size);
  std::atomic<int> next_value = 2;

  RingBuffer<int> output_buffer;
  std::deque<int> primes_queue;

  auto thread_pool = spawn_workers(number_of_workers, limit, next_value,
                                   primes_storage, output_buffer);

  int finished_workers = 0;
  int last_processed = 2;

  while (true) {
    int candidate_prime;
    // Empty the buffer as quickly as possible.
    while (output_buffer.Pop(candidate_prime)) {
      // std::cout << "candidate " << candidate_prime << std::endl;
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
        int storage_index = last_processed / (8 * sizeof(StorageType));
        int bit_index = last_processed % (8 * sizeof(StorageType));

        if (!(primes_storage[storage_index] & (1 << bit_index))) {
          //std::cout << "FOUND PRIME " << last_processed << std::endl;
          prime_callback(last_processed);
        }
        ++last_processed;
      }
    }

    if (finished_workers == number_of_workers && primes_queue.empty()) {
      break;
    }

    // std::cout << next_value << " " << finished_workers << std::endl;
  }

  join_workers(thread_pool);
}
