#ifndef ROPE_ROPE_ITER_H
#define ROPE_ROPE_ITER_H

#import <list>

#import "rope_node_type.hpp"

using std::list;
using std::function;
using std::shared_ptr;

namespace Rope {
    /**
     *  Forward-declare RopeNode so that the iterator can access it.
     */
    template<typename Item, typename MeasureType>
    class RopeNode;
    
    template<typename Item, typename MeasureType, typename IterMeasureType>
    class MeasureIterator
    {
    private:
        using __RopeNode = RopeNode<Item, MeasureType>;
        using This = MeasureIterator<Item, MeasureType, IterMeasureType>;

        struct IterNode
        {
            __RopeNode *rope;
            uintptr_t  target;
            
            IterNode(__RopeNode *rope, uintptr_t target)
            :   rope(rope),
                target(target)
            {}
        };

    public:
        using CallbacksType = IteratorCallbacks<IterMeasureType, Item>;

        /**
         *  Callbacks used to create and join measures
         */
        CallbacksType callbacks;
        
        /**
         *  Callback used to get a measureable value from a rope node.
         */
        function<IterMeasureType (const __RopeNode &)> get_measureable;
        
        /**
         *  Stack of iteration nodes marking the path through the tree to the current item.
         */
        list<IterNode> nodes;
        
        /**
         * Get the most concrete node in the
         */
        IterNode current_node() const
        {
            return nodes.back();
        }
        
        void push_to_leaf()
        {
            __RopeNode *rope = nullptr;
            uintptr_t target = 0;
            assert(current_node().rope != nullptr);
            while (rope = current_node().rope,
                   target = current_node().target,
                   rope->node_type == RopeNodeTypeBranch) {
                int lcap =  rope->branch_data.left != nullptr
                          ? callbacks.predicate(get_measureable(*rope->branch_data.left))
                          : 0;
                if (target < lcap) {
                    // Push down left
                    assert(rope->branch_data.left != nullptr);
                    
                    nodes.push_back(IterNode(rope->branch_data.left.get(), target));
                } else {
                    int ccap = callbacks.predicate(get_measureable(*rope));
                    int rcap = rope->branch_data.right != nullptr ? callbacks.predicate(get_measureable(*rope->branch_data.right)) : 0;
                    target -= lcap;
                    target += (lcap + rcap) - ccap;

                    // Push down right
                    assert(rope->branch_data.right != nullptr);
                    
                    nodes.push_back(IterNode(rope->branch_data.right.get(), target));
                }
            }
        }
        
        uintptr_t raw_index() const
        {
            uintptr_t ret = 0;
            
            for (auto it = nodes.begin(); it != nodes.end(); ++ it) {
                if (it->rope->node_type == RopeNodeTypeBranch) {
                    auto next = (++it)->rope;
                    --it;
                    if (next == it->rope->branch_data.right.get()) {
                        ret += it->rope->branch_data.left->size;
                    }
                } else {
                    int acc = callbacks.index(*current_node().rope->leaf_data, current_node().target);
                    ret += acc > 0 ? acc : 0;
                }
            }
            return ret;
        }
        
        void advance(int n)
        {
            if (n == 0) {
                return;
            }
            push_to_leaf();

            if (nodes.empty()) {
                return;
            }
            
            while (current_node().target + n >= callbacks.predicate(get_measureable(*current_node().rope))) {
                if (nodes.size() == 1) {
                    break;
                }
                nodes.pop_back();
            }
            
            if (nodes.size() == 1 && current_node().target + n >= callbacks.predicate(get_measureable(*current_node().rope))) {
                n = callbacks.predicate(get_measureable(*current_node().rope)) - current_node().target;
            }
            
            for (auto it = nodes.begin(); it != nodes.end(); ++it) {
                (*it).target += n;
            }
            
            push_to_leaf();
        }
        
        void retreat(int n)
        {
            push_to_leaf();

            if (nodes.empty()) {
                return;
            }
            
            while (current_node().target < n) {
                if (nodes.size() == 1) {
                    break;
                }
                nodes.pop_back();
            }
            
            for (auto it = nodes.begin(); it != nodes.end(); ++it) {
                (*it).target -= n;
            }
            
            push_to_leaf();
        }
        
        This &operator++()
        {
            advance(1);
            return *this;
        }
        
        This operator++(int n)
        {
            MeasureIterator ret = *this;
            ret.advance(n);
            return ret;
        }
        
        This &operator+=(int n)
        {
            advance(n);
            return *this;
        }
        
        friend This operator+(uintptr_t n, const This &rhs)
        {
            This ret = rhs;
            ret.advance(n);
            return ret;
        }
        
        friend This operator+(const This &lhs, uintptr_t n)
        {
            This ret = lhs;
            ret.advance(n);
            return ret;
        }
        
        This &operator--()
        {
            retreat(1);
            return *this;
        }
        
        This operator--(int n)
        {
            This ret = *this;
            ret.retreat(n);
            return ret;
        }
        
        This &operator-=(int n)
        {
            retreat(n);
            return *this;
        }
        
        friend This operator-(uintptr_t n, const This &rhs)
        {
            This ret = rhs;
            ret.retreat(n);
            return ret;
        }
        
        friend This operator-(const This &lhs, uintptr_t n)
        {
            This ret = lhs;
            ret.retreat(n);
            return ret;
        }
        
        friend uintptr_t operator-(This &lhs, This &rhs)
        {
            assert(lhs.nodes.front().rope == rhs.nodes.front().rope);
            return lhs.nodes.front().target - rhs.nodes.front().target;
        }
        
        Item operator*()
        {
            push_to_leaf();
            auto acc = callbacks.index(
                            *(current_node().rope->leaf_data),
                            current_node().target);
            return *(current_node().rope->leaf_data->begin() + (int)acc);
        }
        
        Item *operator->()
        {
            push_to_leaf();
            return &*(current_node().rope->leaf_data.begin() + current_node().target);
        }
        
        friend bool operator==(const This &lhs, const This &rhs)
        {
            assert(lhs.nodes.front().rope == rhs.nodes.front().rope);
            return lhs.raw_index() == rhs.raw_index();
        }
        
        friend bool operator!=(const This &lhs, const This &rhs)
        {
            return !(lhs == rhs);
        }
        
        friend bool operator<(const This &lhs, const This &rhs)
        {
            assert(lhs.nodes.front().rope == rhs.nodes.front().rope);
            return lhs.nodes.front().target < rhs.nodes.front().target;
        }
        
        friend bool operator>(const This &lhs, const This &rhs)
        {
            assert(lhs.nodes.front().rope == rhs.nodes.front().rope);
            return lhs.nodes.front().target > rhs.nodes.front().target;
        }
        
        friend bool operator<=(const This &lhs, const This &rhs)
        {
            assert(lhs.nodes.front().rope == rhs.nodes.front().rope);
            return lhs.nodes.front().target <= rhs.nodes.front().target;
        }
        
        friend bool operator>=(const This &lhs, const This &rhs)
        {
            assert(lhs.nodes.front().rope == rhs.nodes.front().rope);
            return lhs.nodes.front().target >= rhs.nodes.front().target;
        }
        
        MeasureIterator<Item, MeasureType, shared_ptr<MeasureType>>(
            __RopeNode *root,
            uintptr_t position,
            IteratorCallbacks<shared_ptr<MeasureType>, Item> callbacks)
        :   callbacks(callbacks),
            get_measureable([] (const __RopeNode &rope) -> shared_ptr<MeasureType> { return rope.measure; })
        {
            nodes.push_back(IterNode(root, position));
        }
        
        MeasureIterator<Item, MeasureType, uintptr_t>(
            __RopeNode *root,
            uintptr_t position)
        :   callbacks(
                // index
                [] (const Slice<Item> &vec, int target) {
                    int acc = -1;
                    auto it = vec.begin();
                    auto end = vec.end();
                    auto accm = 0;
                    while (accm - 1 < target) {
                        if (it == end) {
                            ++acc;
                            break;
                        }
                        accm += 1;
                        ++acc;
                        ++it;
                    }
                    return acc;
                },
                // increment predicate
                [] (const uintptr_t &size) -> uintptr_t { return size; }),
            get_measureable([] (const __RopeNode &rope) -> uintptr_t { return rope.size; })
        {
            nodes.push_back(IterNode(root, 0));
            advance(position);
        }
        
        template<typename I, typename M, typename IMT>
        MeasureIterator<I, M, IMT>
        (MeasureIterator<I, M, IMT> &other)
        :   callbacks(other.callbacks),
            get_measureable(other.get_measureable),
            nodes(other.nodes)
        {}
    };
};

#endif // ROPE_ROPE_ITER_H
