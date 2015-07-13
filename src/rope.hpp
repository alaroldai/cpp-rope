#ifndef ROPE_ROPE_HEADER
#define ROPE_ROPE_HEADER

#import <memory>
#import <functional>

#import "rope_node.hpp"

using std::shared_ptr;
using std::function;
using std::make_shared;
using std::get;
using std::ostream;
using std::cout;
using std::endl;
using std::tuple;
using std::make_tuple;

namespace Rope {

    template<typename Item, typename MeasureType>
    class Rope {
    private:
        using This     = Rope<Item, MeasureType>;
        using NodeType = RopeNode<Item, MeasureType>;
        
    public:
        using MeasureIterType = MeasureIterator<Item, MeasureType, shared_ptr<MeasureType>>;
        using ItemIterType = MeasureIterator<Item, MeasureType, uintptr_t>;
        using CallbacksType = MeasureCallbacks<shared_ptr<MeasureType>, Item>;
        using PredicateType = function<uintptr_t (const shared_ptr<MeasureType> &)>;

        shared_ptr<NodeType> rootNode;
        
        Rope<Item, MeasureType>(CallbacksType const &callbacks)
        :   rootNode(make_shared<NodeType>(callbacks))
        {}
        
        template<typename Container>
        Rope<Item, MeasureType>(Container const &other, CallbacksType const &callbacks)
        :   rootNode(make_shared<NodeType>(other, callbacks))
        {}
        
        Rope<Item, MeasureType>(shared_ptr<NodeType> const &root)
        :   rootNode(root)
        {}
        
        uintptr_t size() const {
            return rootNode->size;
        }
        
        MeasureType const &measure() const {
            return *rootNode->measure;
        }

        void each_chunk(std::function<void (Item const *s, uintptr_t l)> f) {
            rootNode->each_chunk(f);
        }

        MeasureIterType begin(typename MeasureIterType::CallbacksType const &callbacks) const
        {
            return MeasureIterType(rootNode.get(), 0, callbacks);
        }

        ItemIterType begin_items() const
        {
            return ItemIterType(rootNode.get(), 0);
        }

        MeasureIterType end(typename MeasureIterType::CallbacksType const &callbacks) const
        {
            return begin(callbacks) + callbacks.predicate(rootNode->measure) + 1;
        }

        ItemIterType end_items() const
        {
            auto lhs = begin_items();
            return lhs + rootNode->size + 1;
        }
        
        This substr(MeasureIterType const &begin, MeasureIterType const &end, CallbacksType const &callbacks) const
        {
            return This(rootNode->substr(begin, end, callbacks));
        }
        
        
        This concat(This const &other, CallbacksType const &callbacks) const {
            return This(make_shared<NodeType>(rootNode, other.rootNode, callbacks));
        }
        
        This &balance(CallbacksType const &callbacks) {
            rootNode = NodeType::balanced(rootNode, callbacks);
            return *this;
        }
        
        tuple<This, This> splitAfter(MeasureIterType const &splitPoint, CallbacksType const &callbacks) const
        {
            auto result = rootNode->splitAfter(splitPoint, callbacks);
            return make_tuple(This(get<0>(result)), This(get<1>(result)));
        }

        tuple<This, This> splitBefore(MeasureIterType const &splitPoint, CallbacksType const &callbacks) const
        {
            auto result = rootNode->splitBefore(splitPoint, callbacks);
            return make_tuple(This(get<0>(result)), This(get<1>(result)));
        }
        
        void __log() const
        {
            rootNode->__log();
            cout << endl;
        }

    };
    
    
    template<
        typename    Item,
        typename    MeasureType>
        ostream &operator<<
    (   ostream &lhs,
        Rope<Item, MeasureType> const &rhs)
    {
        auto it = rhs.begin_items();
        auto end = rhs.end_items();
        
        for (; it < end; ++ it) {
            auto r = *it;
            lhs << r;
        }
        return lhs;
    }
};

#endif // ROPE_ROPE_HEADER

