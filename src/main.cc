#import "rope.hpp"

#import <iostream>
#import <fstream>
#import <sstream>
#import <memory>

#import <ctime>

#import "utf8.hpp"

#define ROPE_TEST_PRINT 1

using std::shared_ptr;
using std::dynamic_pointer_cast;
using std::cout;
using std::get;
using std::endl;
using std::ostringstream;
using std::clock_t;
using std::clock;
using std::ifstream;
using std::string;
using std::cerr;

using UTF8Measure = Rope::UTF8Measure;

using CRope = Rope::Rope<
    char,
    Rope::Measure<char>>;

using GenericMeasureCallbacks = Rope::MeasureCallbacks<shared_ptr<Rope::Measure<char>>, char>;
using GenericIteratorCallbacks = Rope::IteratorCallbacks<shared_ptr<Rope::Measure<char>>, char>;

GenericMeasureCallbacks callbacks = GenericMeasureCallbacks(
    GenericMeasureCallbacks::lift_join(UTF8Measure::add),
    GenericMeasureCallbacks::lift_identity(UTF8Measure::identity),
    GenericMeasureCallbacks::lift_accumulate(UTF8Measure::accumulate)
);

GenericIteratorCallbacks iterCallbacks(
    GenericIteratorCallbacks::lift_index(UTF8Measure::index),
    GenericIteratorCallbacks::lift_predicate(UTF8Measure::getCount)
);


auto getCount = [](shared_ptr<Rope::Measure<char>> const &m) {
    return Rope::UTF8Measure::getCount(dynamic_pointer_cast<Rope::UTF8Measure>(m));
};

void raw_index_test(CRope rope)
{
    int i = 0;
    for (auto it = rope.begin(iterCallbacks); it < rope.end(iterCallbacks); ++it, ++i) {
        if (ROPE_TEST_PRINT) {
            cout << "raw index of character at index " << i << ": " << it.raw_index() << endl;
        }
    }
}

void split_after_test(CRope rope)
{
    int i = 0;
    for (auto it = rope.begin(iterCallbacks); it < rope.end(iterCallbacks); ++it, ++i) {
        auto expected = rope.begin(iterCallbacks) + i;
        assert(it == expected);
        auto split = rope.splitAfter(it, callbacks);
        auto lhs = get<0>(split), rhs = get<1>(split);
        if (ROPE_TEST_PRINT) {
            cout << "split after begin + " << i << ":\t\"" << lhs << "\" -> \"" << rhs << "\"" << endl;
        }
    }
}

uintptr_t split_before_test(CRope rope)
{
    int i = 0;
    for (auto it = rope.begin(iterCallbacks); it < rope.end(iterCallbacks); ++it, ++i) {
        it.push_to_leaf();
        auto split = rope.splitBefore(it, callbacks);
        if (ROPE_TEST_PRINT) {
            cout << "Split before index " << i << ": \"";
            cout << get<0>(split) << "\" -> \"" << get<1>(split) << "\"" << endl;
        }
    }
    return i;
}

void split_and_concat_tests(CRope rope)
{
    for (auto it = rope.begin(iterCallbacks); it < rope.end(iterCallbacks); ++it) {
        auto split = rope.splitBefore(it, callbacks);
        if (ROPE_TEST_PRINT) {
            cout << get<0>(split) << " + " << get<1>(split) << " -> " << get<0>(split).concat(get<1>(split), callbacks) << endl;
        }
    }
}

void build_by_concat_tests(CRope rope)
{
    for (char c = 'a'; c != 'z' + 1; ++c) {
        rope = rope.concat(CRope((ostringstream() << c).str(), callbacks), callbacks).balance(callbacks);
        if (ROPE_TEST_PRINT) {
            cout << rope << endl;
        }
    }
    
    for (int i = 0; i < 20; i++) {
        auto s = (ostringstream() << i).str();
        auto rhs = CRope(s, callbacks);
        auto result = rope.concat(rhs, callbacks).balance(callbacks);
        if (ROPE_TEST_PRINT) {
            cout << rope << " + " << rhs << " -> " << result << endl;
        }
        rope = result;
    }
}

void split_after_end_test(CRope rope)
{
    auto split = rope.splitAfter(rope.end(iterCallbacks), callbacks);
    if (ROPE_TEST_PRINT) {
        cout << "Split after end: \"" << get<0>(split) << "\" -> \"" << get<1>(split) << "\"" << endl;
    }
}

void tests_with_rope(CRope rope)
{
    raw_index_test(rope);
    split_after_test(rope);
    split_before_test(rope);
    split_after_end_test(rope);
    split_and_concat_tests(rope);
}

void run_tests()
{
    string msg = u8"インターネットに接続していることを確認し";
    
    CRope rope(msg, callbacks);

    tests_with_rope(rope);

    build_by_concat_tests(CRope(callbacks));
}

void speed_test()
{

    string s = "a";

    cout << "numSplits, elapsed, elapsed / numSplits" << endl;

    while (s.size() < Rope::ROPE_GLOBAL_MAX_LEAF_CAP * 4) {
        cerr << (double)s.size() * 100 / (double)(Rope::ROPE_GLOBAL_MAX_LEAF_CAP * 4) << endl;

        CRope rope(s, callbacks);
        rope.balance(callbacks);

        clock_t start = clock();

        auto numSplits = split_before_test(rope) ?: 1;

        clock_t end = clock();
        auto elapsed = (end - start) / (double)(CLOCKS_PER_SEC / 1000000);

        cout << rope.size() << ", " << elapsed << ", " << "--, " << elapsed / numSplits << endl;

        s += "a";
    }
}

void file_speed_test(int argc, char **argv)
{
    if (argc < 2) {
        cout << "Expected a filename for argument 1" << endl;
        return;
    }

    string *file_contents = new string;

    ifstream file(argv[1]);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            *file_contents += line;
        }
    }

    CRope rope(*file_contents, callbacks);
    
    delete file_contents;

    clock_t start = clock();

    cout << "numSplits, elapsed, elapsed / numSplits" << endl;
    uintptr_t i = 0;
    for (auto it = rope.begin(iterCallbacks); i < 10000 && it < rope.end(iterCallbacks); ++it, ++i) {

        it.push_to_leaf();
        
        clock_t start_split = clock();

        auto split = rope.splitBefore(it, callbacks);

        auto elapsed = (clock() - start_split) / (double)(CLOCKS_PER_SEC / 1000000);
        
        cout << i << ", " << elapsed << ", " << elapsed << endl;
    }

    double elapsed = (double)(clock() - start) / (double)(CLOCKS_PER_SEC);
    cout << i << " operations completed, " << (elapsed > 0 ? (float)i / (float)elapsed : -1) << " ops / s" << endl;
}


int main(int argc, char **argv)
{
    // file_speed_test(argc, argv);
    
    // speed_test();

    run_tests();
}
