#include <functional>
#include <iostream>
#include <thread>

#include "emscripten.h"
#include "primes.h"

typedef void (*PrimesCallback)(int prime);

int main() {
  // find_primes(30000000, 16, [](int prime) {
  //   std::cout << "Found prime: " << prime << std::endl;
  //   //std::cout.flush();
  // });
}

void callback_and_sleep(PrimesCallback callback, int prime) {
  if (callback) {
    (*callback)(prime);
  }
  // emscripten_sleep(0);
}

extern "C" {
EMSCRIPTEN_KEEPALIVE
void find_primes_js(int limit, int threads, PrimesCallback callback) {
  using namespace std::placeholders;
  std::thread processing_thread(find_primes, limit, threads,
                                std::bind(callback_and_sleep, callback, _1));

  while (pthread_tryjoin_np(processing_thread.native_handle(), nullptr)) {
    emscripten_sleep(0);
  };
}
}
