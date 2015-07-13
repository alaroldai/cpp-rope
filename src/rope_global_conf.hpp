#ifndef ROPE_ROPE_GLOBAL_CONF_H
#define ROPE_ROPE_GLOBAL_CONF_H

#import <unistd.h>

namespace Rope
{
    static uintptr_t ROPE_GLOBAL_MAX_LEAF_CAP = sysconf(_SC_PAGESIZE);
}

#endif // ROPE_ROPE_GLOBAL_CONF_H
