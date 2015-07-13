#import "fibonacci.hpp"

#import <cmath>

static const double PHI = 1.61803398874989484820;

uintptr_t Rope::fibIndex(uintptr_t fibnum) {
    return floor(log(fibnum * sqrt(5) + 0.5) / log(PHI));
}
