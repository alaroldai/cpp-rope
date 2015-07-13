#ifndef ROPE_ROPE_NODE_TYPE_H
#define ROPE_ROPE_NODE_TYPE_H

namespace Rope {
    /**
     *  Represents the node type of a rope node
     */
    enum RopeNodeType : char {
        RopeNodeTypeLeaf = 0,
        RopeNodeTypeBranch = 1
    };
}

#endif // ROPE_ROPE_NODE_TYPE_H
