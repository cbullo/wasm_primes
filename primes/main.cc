#include <atomic>
#include <cmath>
#include <functional>
#include <mutex>
#include <thread>

#include "emscripten.h"
#include "primes.h"

typedef void (*PrimesCallback)(int prime);

int main() { return 0; }

extern "C" {
EMSCRIPTEN_KEEPALIVE
void findPrimesJS(int limit, int threads, PrimesCallback callback) {
  using namespace std::placeholders;
  const int kMinReportSize = 256;

  std::vector<int> read_buffer;
  std::vector<int> write_buffer;
  std::atomic<int> write_count = 0;
  std::atomic<bool> finished = false;
  std::mutex buffers_mutex;

  read_buffer.reserve(sqrt(limit));
  write_buffer.reserve(sqrt(limit));

  auto internal_callback = [&write_buffer, &write_count, &buffers_mutex,
                            &finished](int prime) {
    bool locked = false;
    if (write_count >= kMinReportSize) {
      buffers_mutex.lock();
      locked = true;
    }
    if (prime == -1) {
      finished.store(true);
    } else {
      write_buffer.push_back(prime);
      write_count++;
    }
    if (locked) {
      buffers_mutex.unlock();
    }
  };

  std::thread processing_thread(FindPrimes, limit, threads,
                                std::bind(internal_callback, _1));
  processing_thread.detach();

  while (true) {
    bool local_finished = finished.load();
    if (write_count >= kMinReportSize || local_finished) {
      const std::lock_guard<std::mutex> lock(buffers_mutex);
      write_buffer.swap(read_buffer);
      write_count.store(0);
    }

    if (callback) {
      for (auto prime : read_buffer) {
        callback(prime);
      }
    }
    read_buffer.resize(0);

    emscripten_sleep(1);

    if (local_finished) {
      break;
    }
  }
}
}
