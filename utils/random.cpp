#include "random.h"

#include <random>

int rand_int(int min, int max) {
    static std::random_device rd;
    static std::default_random_engine engine{rd()};
    return std::uniform_int_distribution<>{min, max}(engine);
}