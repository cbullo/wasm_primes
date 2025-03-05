#include <functional>

#ifdef __EMSCRIPTEN
// TODO: Get rid of Emscripten here
#include "emscripten.h"
#endif

void FindPrimes(int limit, int number_of_workers,
                std::function<void(int)> prime_callback);
