Rope
==============

The Rope is a binary-tree based data structure designed for performing string-manipulation operations efficiently on large amounts of text.

This is an implementation of a rope in C++. While there is an existing implementation of ropes in the C++ STL, that
implementation is restricted to ropes using either `char` or `wchar` as the underlying type. Further, the STL
implementation is not unicode-aware, which increases the complexity of working with UTF-8 strings (for example,
iterating by words / code-points). This implementation provides both.

## Implementation details

- This rope does not specify an underlying type, the underlying type must be provided as a template parameter.
- Random-access and slicing operations on the rope are supported by the concept of a "Measure", which is similar ot
  a monoid.
- Pretty much all interaction is via iterators, which are constructed from measure identifiers.

## Measures
The "Measure" type is an abstract class with the following requirements:
- It must be constructable from a vector of the rope's underlying type
- It must be composable (i.e., there must be a function of type (T, T) -> T, such as '+')
- It must have an 'index' function, which, when given a vector and a target measure, will be capable of locating the
  point in the vector at which the accumulative measurement matches the target measure.

In the case of counting bytes (given `char` as the rope's underlying type), those functions might be implemented as:

    using ByteMeasure = uint64_t;
    ByteMeasure byte_measure(std::vector<char> &vec) { return vec.size(); }
    ByteMeasure byte_compose(ByteMeasure &lhs, ByteMeasure &rhs) { return lhs + rhs; }
    uint64_t    byte_index(std::vector<char> &vec, ByteMeasure &measure) { return measure; }


