#import <cstdint>

namespace Rope {
    /**
     *  Calculate which index in the Fibbonacci sequence a number appears at.
     *  If `fibNum` is not a fibbonacci number, returns the largest fibbonacci number in [0, `fibNum`)
     */
    uintptr_t fibIndex(uintptr_t fibnum);
}
