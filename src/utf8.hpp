#ifndef DAEDALUS_UTF8_H
#define DAEDALUS_UTF8_H

#import <cstdint>
#import <memory>

#import "measure.hpp"
#import "slice.hpp"

namespace Daedalus {

    /**
     *  Represents a measure of a number of UTF8-encoded characters,
     *  along with leading and trailing bytes up to 4 bytes at each end
     */
    class UTF8Measure : public Measure<char>
    {
    private:
        using Shared = std::shared_ptr<UTF8Measure>;

    public:
        char pre[4];
        uintptr_t count;
        char post[4];

        /**
         *  Construct an empty UTF8Measure. ('', 0, '')
         */
        UTF8Measure();

        /**
         *  Construct a UTF8Measure from a single byte.
         *  ('', 1, '') if the byte is a valid UTF8 rune, otherwise (c, 0, c);
         */
        UTF8Measure(const char &c);

        /**
         *  Construct a UTF8Measure given a specified number of runes.
         *  ('', d, '')
         */
        UTF8Measure(int d);

        /**
         *  Construct a UTF8Measure by joining two existing measures
         *  count = left.count + right.count + 1 (assuming left.post + right.post is a valid sequence)
         *  pre = left.pre
         *  post = right.post
         */
        UTF8Measure(UTF8Measure const &left, UTF8Measure const &right);

        #pragma mark - Callbacks

        /**
         *  Callback function used to access `count` through a shared pointer
         */
        static uintptr_t
        getCount(const Shared &m);

        /**
         *  Callback function used to create an empty UTF8Measure
         */
        static Shared
        identity();

        /**
         *  Callback function used to join UTF8Measure instances
         */
        static Shared
        add(const Shared &lhs, const Shared &rhs);

        static Shared
        accumulate(const Slice<char> &vec);

        static uintptr_t
        index(const Slice<char> &vec, uintptr_t target);
    };

    /**
     *  Represents the number of line beginnings in a chunk of text
     */
    class LineMeasure : public Measure<char>
    {
    private:
        using Shared = std::shared_ptr<LineMeasure>;

    public:
        bool lpartial;
        uintptr_t count;

        /**
         *  Construct an empty LineMeasure. ('', 0, '')
         */
        LineMeasure();

        /**
         *  Construct a LineMeasure from a single byte.
         */
        LineMeasure(const char &c);

        /**
         *  Construct a LineMeasure given a specified number of runes.
         */
        LineMeasure(int d);

        /**
         *  Construct a UTF8Measure by joining two existing measures
         *  count = left.count + right.count + 1 (assuming left.post + right.post is a valid sequence)
         *  pre = left.pre
         *  post = right.post
         */
        LineMeasure(LineMeasure const &left, LineMeasure const &right);

        #pragma mark - Callbacks

        /**
         *  Callback function used to access `count` through a shared pointer
         */
        static uintptr_t
        getCount(const Shared &m);

        /**
         *  Callback function used to create an empty UTF8Measure
         */
        static Shared
        identity();

        /**
         *  Callback function used to join UTF8Measure instances
         */
        static Shared
        add(const Shared &lhs, const Shared &rhs);

        static Shared
        accumulate(const Slice<char> &vec);

        static uintptr_t
        index(const Slice<char> &vec, uintptr_t target);
    };
    
    class BytesMeasure : public Measure<char>
    {
    private:
        size_t bytes;
        using Shared = std::shared_ptr<BytesMeasure>;
        
    public:
        
        BytesMeasure(BytesMeasure const &lhs, BytesMeasure const &rhs) : bytes(lhs.bytes + rhs.bytes) { }
        BytesMeasure(size_t bytes) : bytes(bytes) { }
        BytesMeasure() : bytes(0) { }
        
        static Shared add(const Shared &lhs, const Shared &rhs) {
            return std::make_shared<BytesMeasure>(*lhs, *rhs);
        }
        
        static Shared identity() {
            return std::make_shared<BytesMeasure>();
        };
        
        static Shared accumulate(const Slice<char> &vec) {
            return std::make_shared<BytesMeasure>(vec.size());
        }
        
        static uintptr_t
        index(const Slice<char> &vec, uintptr_t target) {
            return target;
        }
        
        static uintptr_t getCount(const Shared &m) {
            return m->bytes;
        }
    };

};

#endif
