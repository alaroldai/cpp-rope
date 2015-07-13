#ifndef DAEDALUS_ROPE_NODE_HEADER
#define DAEDALUS_ROPE_NODE_HEADER

#import <memory>
#import <functional>
#import <vector>
#import <string>
#import <iostream>
#import <numeric>
#import <tuple>
#import <assert.h>
#import <stack>
#import <list>

#import "slice.hpp"
#import "fibonacci.hpp"
#import "measure.hpp"
#import "rope_iter.hpp"
#import "rope_node_type.hpp"
#import "rope_global_conf.hpp"

using std::vector;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::tuple;
using std::cout;
using std::endl;

namespace Daedalus {
    template<
        typename    Item,
        typename    MeasureType>
    class RopeNode {
    public:
        using This = RopeNode<Item, MeasureType>;
        using CallbacksType = MeasureCallbacks<shared_ptr<MeasureType>, Item>;
        
    private:
        using ItemIterType = MeasureIterator<Item, MeasureType, uintptr_t>;
        using Shared = shared_ptr<This>;
        using ItemSlice = shared_ptr<Slice<Item>>;
        
        //TODO: Move the branch/leaf data fields into a union to save memory
        struct BranchData {
            Shared left;
            Shared right;
            
            BranchData(Shared _left, Shared _right)
            :   left(_left),
                right(_right)
            {}
            
            BranchData(const BranchData &other)
            :   left(other.left),
                right(other.right)
            {}

            BranchData()
            :   left(nullptr),
                right(nullptr)
            {
            
            }
        };

        /**
         *  Return a balanced copy of `rope`
         */
        static Shared
        __ropeNodeBalanced(
            Shared rope,
            CallbacksType callbacks)
        {
            if (rope->node_type == RopeNodeTypeLeaf) {
                return rope;
            }
            list<Shared> *leaves = __ropeNodeLeafVector(rope);
            
            if (rope->size / leaves->size() < ROPE_GLOBAL_MAX_LEAF_CAP / 2) {
                auto joinGroupStart = leaves->begin();
                uintptr_t sumSize = leaves->begin() != leaves->end() ? (*(leaves->begin()))->size : 0;
                for (auto it = leaves->begin(); it != leaves->end(); ++it) {
                    auto current_size = (*it)->size;
                    if (sumSize + current_size >= ROPE_GLOBAL_MAX_LEAF_CAP) {
                        list<Shared> joinGroup(joinGroupStart, it);
                        if (joinGroup.size() < 2) {
                            joinGroupStart = it;
                            sumSize = (*it)->leaf_data->size();
                            continue;
                        }

                        list<shared_ptr<Slice<Item>>> slices;
                        for (auto jt = joinGroup.begin(); jt != joinGroup.end(); ++jt) {
                            slices.push_back((*jt)->leaf_data);
                        }
                        auto slice = make_shared<Slice<Item>>(slices);
                        
                        auto insert = make_shared<This>(slice, callbacks);
                        assert(insert->weight == 1);
                        leaves->erase(joinGroupStart, it);
                        leaves->insert(it, make_shared<This>(slice, callbacks));
                        
                        joinGroupStart = it;
                        sumSize = (*it)->leaf_data->size();
                    } else {
                        sumSize += (*it)->leaf_data->size();
                    }
                }
            }
            
            uintptr_t numLeaves = leaves->size();
            uintptr_t blistSize = fibIndex(numLeaves) + 1;
            vector<Shared> blist(blistSize);
            
            uintptr_t j = 0;
            for (auto it = leaves->begin(); it != leaves->end(); ++it, ++j) {
                auto insert = *it;
                bool inserted = false;
                
                while (!inserted) {
                    uintptr_t targetIndex = fibIndex(insert->weight);
                    
                    Shared concatOfLighterNodes = nullptr;
                    
                    for (int i = targetIndex; i >= 0; i--) {
                        uintptr_t idx = targetIndex - i;
                        
                        if (blist.at(idx) == nullptr) {
                            continue;
                        }
                        
                        if (concatOfLighterNodes != nullptr) {
                            concatOfLighterNodes = make_shared<This>(blist.at(idx), concatOfLighterNodes, callbacks);
                        } else {
                            concatOfLighterNodes = blist.at(idx);
                        }
                        blist[idx] = nullptr;
                    }
                    
                    if (concatOfLighterNodes == nullptr) {
                        blist[targetIndex] = insert;
                        inserted = true;
                    } else {
                        insert = make_shared<This>(concatOfLighterNodes, insert, callbacks);
                    }
                }
            }
            
            Shared balanced;
            
            for (uintptr_t i = blistSize; i > 0; i--) {
                uintptr_t idx = blistSize - i;
                if (blist.at(idx)) {
                    if (balanced != nullptr) {
                        balanced = make_shared<This>(blist.at(idx), balanced, callbacks);
                    } else {
                        balanced = blist.at(idx);
                    }
                    blist[idx] = nullptr;
                }
            }
            
            delete leaves;
            
            return balanced;
        }
        
        /**
         *  Append all the leaf nodes of a rope to some vector
         *
         *  NOTE - this is NOT a shared_ptr, you must delete it yourself.
         */
        static list<Shared> *
        __ropeNodeLeafVector(
            Shared rope,
            list<Shared> *prepend)
        {
            if (rope->node_type == RopeNodeTypeLeaf) {
                if (rope->size == 0) {
                    return prepend;
                }
                prepend->push_back(rope);
                return prepend;
            }
            __ropeNodeLeafVector(rope->branch_data.left, prepend);
            __ropeNodeLeafVector(rope->branch_data.right, prepend);
            return prepend;
        }
        
        /**
         *  Construct a vector of all the leaf nodes of a rope
         *
         *  NOTE - this is NOT a shared_ptr, you must delete it yourself.
         */
        static list<Shared> *
        __ropeNodeLeafVector(Shared rope)
        {
            auto vec = new list<Shared>;
            return __ropeNodeLeafVector(rope, vec);
        }

    public:
        /**
         *  The node type
         */
        RopeNodeType node_type;

        /**
         *  Branch data, if any
         */
        BranchData branch_data;

        /**
         *  Leaf data, if any
         */
        ItemSlice leaf_data;

        /**
         *  The number of items stored within the scope of this node.
         */
        uintptr_t size;

        /**
         *  The number of leaf nodes contained within the scope of this node.
         */
        uintptr_t weight;

        /**
         *  An arbitary measure of the items within the scope of this node.
         */
        shared_ptr<MeasureType> measure;


        void each_chunk(std::function<void (Item const *s, uintptr_t l)> f) {
            switch(node_type) {
                case RopeNodeTypeLeaf: {
                    f(&*leaf_data->begin(), leaf_data->size());
                    break;
                }
                case RopeNodeTypeBranch: {
                    if (branch_data.left) branch_data.left->each_chunk(f);
                    if (branch_data.right) branch_data.right->each_chunk(f);
                }
            }
        }
        
        /**
         *  Common logic for initialization once a slice has been created
         */
        void initWithSlice(ItemSlice const &slice, CallbacksType const &callbacks)
        {
            uintptr_t slice_size = slice->size();
            if (slice_size >= ROPE_GLOBAL_MAX_LEAF_CAP) {
                uintptr_t lcap = slice_size / 2;
                
                auto lhs_slice = make_shared<Slice<Item>>(*slice, 0, lcap);
                auto rhs_slice = make_shared<Slice<Item>>(*slice, lcap, slice_size - lcap);
                
                auto lhs = make_shared<This>(lhs_slice, callbacks);
                auto rhs = make_shared<This>(rhs_slice, callbacks);
                
                leaf_data = nullptr;
                branch_data = BranchData(lhs, rhs);
                measure = callbacks.join(branch_data.left->measure, branch_data.right->measure);
                size = lhs->size + rhs->size;
                weight = branch_data.left->weight + branch_data.right->weight;
                node_type = RopeNodeTypeBranch;
                return;
            }
            
            leaf_data = slice;
            branch_data = BranchData();
            measure = callbacks.accumulate(*leaf_data);
            size = leaf_data->size();
            weight = 1;
            node_type = RopeNodeTypeLeaf;
        }

        /**
         *  Construct a rope from a vector. Can be more efficient as a slice can be created directly
         *  without need to create a new vector and copy elements into it.
         */
        void initWithVector(shared_ptr<vector<Item>> vector, CallbacksType const &callbacks)
        {
            auto slice = make_shared<Slice<Item>>(
                                                vector,
                                                vector->begin(),
                                                vector->end());
            

            initWithSlice(slice, callbacks);
        }

        /**
         *  Construct an empty rope
         */
        RopeNode<
            Item,
            MeasureType>
        (   CallbacksType const &callbacks)
        :   node_type(RopeNodeTypeLeaf),
            branch_data(BranchData(nullptr, nullptr)),
            leaf_data(make_shared<Slice<Item>>()),
            size(0),
            weight(1),
            measure(callbacks.identity())
        {}
        
        /**
         *  Construct a rope from a slice
         */
        RopeNode<Item, MeasureType>(
            ItemSlice const &slice,
            CallbacksType const &callbacks)
        {
            initWithSlice(slice, callbacks);
        }
        
        /**
         *  Construct a rope from a container (e.g., a string or list)
         */
        template<typename Container>
        RopeNode<
            Item,
            MeasureType>
        (   Container const &other,
            CallbacksType const &callbacks)
        :   branch_data()
        {
            auto vec = make_shared<vector<Item>>();
            for (auto it = other.begin(); it != other.end(); ++it) {
                vec->push_back(*it);
            }
            
            initWithVector(vec, callbacks);
        }

        
        /**
         *  Construct a rope from a C array
         */
        RopeNode<Item, MeasureType>(Item const *buf, size_t const buflen, CallbacksType const &callbacks)
        {
            auto vec = make_shared<vector<Item>>();
            for (size_t i = 0; i < buflen; ++i) {
                vec->push_back(*(buf + i));
            }
            initWithVector(vec, callbacks);
        }
        
        RopeNode<Item, MeasureType>(string &other, CallbacksType const &callbacks)
        :   RopeNode<Item, MeasureType>(other.c_str(), strlen(other.c_str()), callbacks)
        {}
        
        /**
         *  Construct a rope by joining two other ropes.
         *  The arguments must be wrapped in `shared_ptr`s
         */
        RopeNode<
            Item,
            MeasureType>
        (   Shared const &left,
            Shared const &right,
            CallbacksType const &callbacks)
        :   node_type(RopeNodeTypeBranch),
            branch_data(left, right),
            size(left->size + right->size),
            weight(left->weight + right->weight),
            measure(callbacks.join(left->measure, right->measure))
        {}
        
        /**
         *  Construct a rope from a substring, as specified by a pair of iterators
         *  [begin, end)
         */
        template<typename IterMeasureType1, typename IterMeasureType2>
        RopeNode<Item, MeasureType>(
            const MeasureIterator<Item, MeasureType, IterMeasureType1> &_begin,
            const MeasureIterator<Item, MeasureType,  IterMeasureType2> &_end,
            CallbacksType const &callbacks)
        {
            auto begin = ItemIterType(
                _begin.nodes.front().rope,
                _begin.raw_index() > 0 ? _begin.raw_index() : 0);
            auto end = ItemIterType(
                _end.nodes.front().rope,
                _end.raw_index() > 0 ? _end.raw_index() : 0);
            
            assert(begin.nodes.front().rope == end.nodes.front().rope);
            assert(begin <= end);
            
            begin.push_to_leaf();
            end.push_to_leaf();
            
            while (true) {
                This &src = *begin.nodes.front().rope;
                if (src.node_type == RopeNodeTypeLeaf) {
                    node_type = RopeNodeTypeLeaf;
                    
                    auto start = begin.raw_index();
                    auto length = end.raw_index() - start;
                    
                    leaf_data = make_shared<Slice<Item>>(
                        *src.leaf_data,
                        start,
                        length);
                    branch_data = BranchData(nullptr, nullptr);
                    size = leaf_data->size();
                    weight = 1;
                    measure = callbacks.accumulate(*leaf_data);
                    break;
                }
                
                // Other is a branch
                end.nodes.pop_front();
                begin.nodes.pop_front();
                
                if ((*begin.nodes.begin()).rope != (*end.nodes.begin()).rope) {
                    if (end.raw_index() >= 0) {
                        node_type = RopeNodeTypeBranch;
                        
                        auto begin_front = begin.nodes.front();
                        auto end_front = end.nodes.front();
                        
                        auto lbegin = ItemIterType(begin_front.rope, begin.raw_index());
                        lbegin.push_to_leaf();
                        auto lend = ItemIterType(begin_front.rope, begin_front.rope->size);
                        lend.push_to_leaf();
                        branch_data.left = make_shared<This>(lbegin, lend, callbacks);
                        
                        auto rbegin = ItemIterType(end_front.rope, 0);
                        rbegin.push_to_leaf();
                        auto rend = ItemIterType(end_front.rope, end.raw_index());
                        rend.push_to_leaf();
                        
                        branch_data.right = make_shared<This>(rbegin, rend, callbacks);
                        measure = callbacks.join(branch_data.left->measure, branch_data.right->measure);
                        size = branch_data.left->size + branch_data.right->size;
                        weight = branch_data.left->weight + branch_data.right->weight;
                        break;
                    } else {
                        end = ItemIterType(begin.nodes.front().rope,
                                           begin.nodes.front().rope->size + 1);
                        end.push_to_leaf();
                        continue;
                    }
                }
            };
        }
        
        ~RopeNode() {}
        
        template<typename IterMeasureType>
        Shared
        substr(
            MeasureIterator<Item, MeasureType, IterMeasureType> const &_begin,
            MeasureIterator<Item, MeasureType, IterMeasureType> const &_end,
            CallbacksType const &callbacks)
        {
            MeasureIterator<Item, MeasureType, IterMeasureType> begin = _begin;
            MeasureIterator<Item, MeasureType, IterMeasureType> end = _end;
            
            begin.push_to_leaf();
            end.push_to_leaf();
            return make_shared<This>(begin, end, callbacks);
        }
        
        template<typename IterMeasureType>
        tuple<Shared, Shared>
        splitAfter(
            MeasureIterator<Item, MeasureType, IterMeasureType> const &it,
            CallbacksType const &callbacks)
        {
            return splitBefore(it + 1, callbacks);
        }

        template<typename IterMeasureType>
        tuple<Shared, Shared>
        splitBefore(
            MeasureIterator<Item, MeasureType, IterMeasureType> const &it,
            CallbacksType const &callbacks)
        {            
            Shared left = nullptr, right = nullptr;
            
            for (auto nit = it.nodes.begin(); nit != it.nodes.end(); ++nit) {
                This *src = nit->rope;

                if (src->node_type == RopeNodeTypeLeaf) {
                    auto mid = src->leaf_data->begin() + it.callbacks.index(*src->leaf_data, nit->target);
                    auto lhs_length = mid - src->leaf_data->begin();
                    auto rhs_length = src->leaf_data->end() - mid;
                    
                    if (left != nullptr) {
                        left = make_shared<This>(
                            left,
                            make_shared<This>(
                                make_shared<Slice<Item>>(*src->leaf_data, 0, lhs_length),
                                callbacks),
                            callbacks);
                    } else {
                        left = make_shared<This>(make_shared<Slice<Item>>(*src->leaf_data, 0, lhs_length), callbacks);
                    }
                    
                    if (right != nullptr) {
                        right = make_shared<This>(
                            make_shared<This>(
                                make_shared<Slice<Item>>(*src->leaf_data, lhs_length, rhs_length),
                                callbacks),
                            right,
                            callbacks);
                    } else {
                        auto slice = *src->leaf_data;
                        right = make_shared<This>(make_shared<Slice<Item>>(slice, lhs_length, rhs_length), callbacks);
                    }
                } else {
                    This *next = (++nit)->rope;
                    --nit;

                    if (next == src->branch_data.left.get()) {
                        if (right != nullptr) {
                            right = make_shared<This>(src->branch_data.right, right, callbacks);
                        } else {
                            right = src->branch_data.right;
                        }
                    } else {
                        if (left != nullptr) {
                            left = make_shared<This>(left, src->branch_data.left, callbacks);
                        } else {
                            left = src->branch_data.left;
                        }
                    }
                }
            }

            return make_tuple(left != nullptr ? left : make_shared<This>(callbacks),
                                   right != nullptr ? right : make_shared<This>(callbacks));
        }

        static Shared
        balanced(Shared const &rope, CallbacksType const &callbacks)
        {
            return __ropeNodeBalanced(rope, callbacks);
        }
        
        void __log()
        {
            if (node_type == RopeNodeTypeBranch) {
                cout << "(";
                branch_data.left->__log();
                cout << ")(";
                branch_data.right->__log();
                cout << ")";
            } else {
                cout << "[ leaf of " << size << " ]";
            }
            fflush(stdout);
        }
    };
};

#endif // DAEDALUS_ROPE_NODE_HEADER
