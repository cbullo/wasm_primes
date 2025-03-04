#include <chrono>
#include <iostream>

#include "ring_buffer.h"

using StorageType = unsigned int;
const int kFinished = -1;

void primes_worker(int limit, std::atomic<int>& next_value,
                   std::vector<std::atomic<StorageType>>& primes_storage,
                   RingBuffer<int>& output_buffer) {
  std::cout << "WORKDER RUNNING" << std::endl;
  while (true) {
    auto value = next_value.fetch_add(1);
    if (value >= limit) {
      while (!output_buffer.Push(kFinished)) {
      };
      return;
    }

    int storage_index = value / (sizeof(StorageType) * 8);
    int bit_index = value % (sizeof(StorageType) * 8);
    auto prev_value = primes_storage[storage_index].load();
    if (prev_value & (1 << bit_index)) {
      // The value has already been marked by a different thread. Thus it's not
      // a prime.
      continue;
    }

    for (auto i = 2 * value; i < limit; i += value) {
      int storage_index = i / (sizeof(StorageType) * 8);
      int bit_index = i % (sizeof(StorageType) * 8);
      prev_value = primes_storage[storage_index].fetch_or(1 << bit_index);
    }

    while (!output_buffer.Push(value)) {
    };
  }
}