#include <iostream>

#include "primes.h"

int main() {
  find_primes(30000000, 16, [](int prime) {
    std::cout << "Found prime: " << prime << std::endl;
    //std::cout.flush();
  });
}