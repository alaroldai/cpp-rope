#ifndef DAEDALUS_ROPE_NODE_TYPE_H
#define DAEDALUS_ROPE_NODE_TYPE_H

namespace Daedalus {
    /**
     *  Represents the node type of a rope node
     */
    enum RopeNodeType : char {
        RopeNodeTypeLeaf = 0,
        RopeNodeTypeBranch = 1
    };
}

#endif // DAEDALUS_ROPE_NODE_TYPE_H