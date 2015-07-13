//
//  slice.h
//  rope
//
//  Created by Alastair Daivis on 2/12/2014.
//
//

#ifndef rope_slice_h
#define rope_slice_h

#import <vector>
#import <list>

namespace Rope {
    

    /**
     *  Provides easy slice behaviour over std::vector (i.e., subvectors)
     */
    template<typename ItemType> class Slice {
    public:
        using Storage = std::vector<ItemType>;
        using IterType = typename Storage::const_iterator;

    private:
        std::shared_ptr<Storage>    store;
        IterType                    istart;     // iterator into `store` marking the beginning of the slice
        IterType                    iend;       // iterator into `store` marking the end of the slice
        
    public:
        
        IterType begin() const { return istart; }

        IterType end() const { return iend; }

        typename std::iterator_traits<IterType>::difference_type
        size() const { return std::distance(istart, iend); }
        
        /**
         *  Construct an empty slice with new storage
         */
        Slice<ItemType> ()
        :   store(std::make_shared<Storage>()),
            istart(store->begin()),
            iend(store->end())
        {}
        
        /**
         *  Construct a new slice from existing storage
         */
        Slice<ItemType>
        (   std::shared_ptr<Storage>   s,
            IterType                   begin,
            IterType                   end)
        :   store(s),
            istart(begin),
            iend(end)
        {}

        /**
         *  Construct a new slice as a slice of another slice
         */
        Slice<ItemType>
        (   Slice<ItemType> const & other,
            int                     start_inset,
            int                     length)
        :   store(other.store),
            istart(other.istart + start_inset),
            iend(other.istart + start_inset + length)
        {}
        
        Slice<ItemType>
        (   std::list<std::shared_ptr<Slice<ItemType>>> others)
        :   store(std::make_shared<Storage>())
        {
            uintptr_t total_size = 0;
            for (auto it = others.begin(); it != others.end(); ++it) {
                total_size += (*it)->size();
            };
            
            store->reserve(total_size);
            
            for (auto it = others.begin(); it != others.end(); ++it) {
                store->insert(store->end(), (*it)->istart, (*it)->iend);
            };
            istart = store->begin();
            iend = store->end();
        }
        
        ~Slice<ItemType>()
        {}
        
        template<typename Item>
        friend std::ostream &operator<<
        (   std::ostream &lhs,
            Slice<Item> const &rhs)
        {
            auto it = rhs.istart;
            auto end = rhs.iend;
            
            for (; it != end; ++ it) {
                lhs << *it;
            }
            return lhs;
        }
        
    };
}

#endif
