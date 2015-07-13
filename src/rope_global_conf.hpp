#ifndef DAEDALUS_ROPE_GLOBAL_CONF_H
#define DAEDALUS_ROPE_GLOBAL_CONF_H

#import <unistd.h>

namespace Daedalus
{
//    static uintptr_t ROPE_GLOBAL_MAX_LEAF_CAP = sysconf(_SC_PAGESIZE);
//    static uintptr_t ROPE_GLOBAL_MAX_LEAF_CAP = 0x1000;
    static uintptr_t ROPE_GLOBAL_MAX_LEAF_CAP = 256;
}

#endif // DAEDALUS_ROPE_GLOBAL_CONF_H