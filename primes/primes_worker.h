#include <chrono>

#include "ring_buffer.h"

using StorageType = unsigned int;
const int kFinished = -1;

inline int StorageIndex(int value) { return value / (sizeof(StorageType) * 8); }

inline int BitIndex(int value) { return value % (sizeof(StorageType) * 8); }

void PrimesWorker(int limit, std::atomic<int>& next_value,
                  std::vector<std::atomic<StorageType>>& primes_storage,
                  RingBuffer<int>& output_buffer) {
  while (true) {
    auto value = next_value.fetch_add(1);
    if (value >= limit) {
      while (!output_buffer.Push(kFinished)) {
      };
      return;
    }

    int storage_index = StorageIndex(value);
    int bit_index = BitIndex(value);
    auto prev_value = primes_storage[storage_index].load();
    if (prev_value & (1 << bit_index)) {
      // The value has already been marked by a different thread. Thus it's not
      // a prime.
      continue;
    }

    for (auto i = 2 * value; i < limit; i += value) {
      int storage_index = StorageIndex(i);
      int bit_index = BitIndex(i);
      prev_value = primes_storage[storage_index].fetch_or(1 << bit_index);
    }

    while (!output_buffer.Push(value)) {
    };
  }
}