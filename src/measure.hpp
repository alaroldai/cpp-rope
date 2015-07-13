#ifndef ROPE_MEASURE_H
#define ROPE_MEASURE_H

#import <functional>

#import "slice.hpp"

using std::function;

namespace Rope {

    /**
     *  Structure used to provide monoid-style operations for rope measurements
     */
    template <typename M, typename T>
    class MeasureCallbacks
    {
    public:
        using joinf_type = function<M (M const &, M const &)>;
        using identityf_type = function<M ()>;
        using accumulatef_type = function<M (const Slice<T> &)>;

        joinf_type join;            // Used to join two measurements (e.g.: (1, 1) -> 2)
        identityf_type identity;    // Returns a measure that is the identity for `join` (e.g.: 0)

        accumulatef_type accumulate;

        MeasureCallbacks(
            joinf_type          joinf,
            identityf_type      identityf,
            accumulatef_type    accumulatef)
        :   join(joinf),
            identity(identityf),
            accumulate(accumulatef)
        {}

        virtual ~MeasureCallbacks() {}

        template <typename A>
        static joinf_type
        lift_join(A (*f)(A const &, A const &))
        {
            return joinf_type(reinterpret_cast<M (*)(M const &, M const &)>(f));
        }

        template <typename A>
        static joinf_type
        lift_join(function<A (A const &, A const &)> f)
        {
            using a_joinf_type = A (A const &, A const &);
            return lift_join(f. template target<a_joinf_type>());
        }

        template <typename A>
        static identityf_type
        lift_identity(A (*f)())
        {
            return identityf_type(reinterpret_cast<M (*)()>(f));
        }

        template <typename A>
        static identityf_type
        lift_identity(function<A ()> f)
        {
            using a_identityf_type = A ();
            return lift_identity(f. template target<a_identityf_type>());
        }

        template <typename A>
        static accumulatef_type
        lift_accumulate(A (*f)(const Slice<T> &))
        {
            return accumulatef_type(reinterpret_cast<M (*)(const Slice<T> &)>(f));
        }

        template <typename A>
        static accumulatef_type
        lift_accumulate(function<A (const Slice<T> &)> f)
        {
            using a_accumulatef_type = A (const Slice<T> &);
            return lift_accumulate(f. template target<a_accumulatef_type>());
        }
    };

    template<typename M, typename T>
    class IteratorCallbacks
    {
    public:
        using indexf_type = function<uintptr_t (Slice<T> const &, uintptr_t)>;
        using predicatef_type = function<uintptr_t (M const &)>;

        indexf_type index;
        predicatef_type predicate;

        IteratorCallbacks(indexf_type indexf, predicatef_type predicatef)
        :   index(indexf),
            predicate(predicatef)
        {}

        static indexf_type
        lift_index(uintptr_t (*f)(const Slice<T> &, uintptr_t))
        {
            return indexf_type(f);
        }

        static indexf_type
        lift_index(function<uintptr_t (const Slice<T> &, uintptr_t)> f)
        {
            return f;
        }

        template <typename A>
        static predicatef_type
        lift_predicate(uintptr_t (*f)(A const &))
        {
            return predicatef_type(reinterpret_cast<uintptr_t (*)(M const &)>(f));
        }

        template <typename A>
        static predicatef_type
        lift_predicate(function<uintptr_t (A const &)> f)
        {
            using a_predicatef_type = uintptr_t (A const &);
            return predicate_index(f. template target<a_predicatef_type>());
        } 
    };
    
    template<typename T>
    class Measure
    {
    public:
        virtual ~Measure<T>() {}
    };
};

#endif // ROPE_MEASURE_H
