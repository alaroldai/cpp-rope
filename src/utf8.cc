#import "utf8.hpp"

#import <sstream>
#import <cstring>
#import <iostream>
#import <cassert>

namespace Daedalus {

    UTF8Measure::UTF8Measure()
    {
        memset(pre, 0, 4);
        memset(post, 0, 4);
        count = 0;
    }

    UTF8Measure::UTF8Measure(const char &c)
    {
        memset(pre, 0, 4);
        memset(post, 0, 4);
        if (c & 0x80) {
            // `c` is part of a multibyte sequence
            count = 0;
            pre[0] = c;
            post[0] = c;
        } else {
            // `c` is a single-byte sequence
            count = c != '\0';
        }
    }

    UTF8Measure::UTF8Measure(int d)
    {
        memset(pre, 0, 4);
        memset(post, 0, 4);
        count = d; 
    }


    UTF8Measure::UTF8Measure(UTF8Measure const &left, UTF8Measure const &right)
    {
        count = left.count + right.count;
        memcpy(pre, left.pre, 4);
        memcpy(post, right.post, 4);
        
        char extra[8];
        memset(extra, 0, 8);

        memcpy(extra, left.post, 4);
        int extralen = strnlen(extra, 4);
        memcpy(extra + extralen, right.pre, 4);
        extralen = strnlen(extra, 8);
        
        // Search for a byte in 'extra' with at least two leading set bits.
        for (char *c = extra; *c != '\0'; c++) {
            int leading = 8 - fls(~*c);
            if (leading > 1) {
                int llen = c - extra;
                int rlen = extralen - llen - leading;
            
                llen = llen > 0 ? llen : 0;
                rlen = rlen > 0 ? rlen : 0;
            
                if (left.count == 0) {
                    memset(pre, 0, 4);
                    memcpy(pre, extra, llen);
                }
                
                if (right.count == 0) {
                    memset(post, 0, 4);
                    memcpy(post, c + leading, rlen);
                }
                
                count ++;
                
                return;
            }
        }
        
        if (left.count == 0) {
            memset(pre, 0, 4);
            memcpy(pre, extra, 4);
        }
        
        if (right.count == 0) {
            memset(post, 0, 4);
            memcpy(post, extra, 4);
        }
    }

    uintptr_t UTF8Measure::getCount(const UTF8Measure::Shared &m)
    {
        return m->count;
    }

    UTF8Measure::Shared UTF8Measure::identity()
    {
        return std::make_shared<UTF8Measure>();
    }

    UTF8Measure::Shared UTF8Measure::add(const UTF8Measure::Shared &lhs, const UTF8Measure::Shared &rhs)
    {
        return std::make_shared<UTF8Measure>(*lhs, *rhs);
    }

    UTF8Measure::Shared UTF8Measure::accumulate(const Slice<char> &vec)
    {
        auto acc = identity();
        auto tmp = identity();

        for (auto it = vec.begin(); it != vec.end(); ++it) {
            UTF8Measure rhs(*it);
            new (acc.get()) UTF8Measure(*tmp, rhs);
            new (tmp.get()) UTF8Measure(*acc);
        }

        return acc;
    }

    uintptr_t UTF8Measure::index(const Slice<char> &vec, uintptr_t target)
    {
        uintptr_t acc = -1;
        auto it = vec.begin();
        auto end = vec.end();
        auto accm = identity();
        auto tmp = identity();
        
        auto current = accm->count;
        
        while (current <= target) {
            if (it == end) {
                ++acc;
                break;
            }
            new (accm.get()) UTF8Measure(*tmp, UTF8Measure(*it));
            new (tmp.get()) UTF8Measure(*accm);
            ++acc;
            ++it;
            current = accm->count;
        } 
        return acc;
    }

#pragma mark - LineMeasure

    LineMeasure::LineMeasure() : lpartial(true), count(0) {}

    LineMeasure::LineMeasure(const char &c) {
        if (c == '\n') {
            count = 1;
            lpartial = false;
        } else {
            count = 0;
            lpartial = true;
        }
    }

    LineMeasure::LineMeasure(int d) {
        count = d;
        lpartial = false;
    }

    LineMeasure::LineMeasure(LineMeasure const &left, LineMeasure const &right) {
        count = left.count + right.count;
        lpartial = left.lpartial;
    }

    uintptr_t LineMeasure::getCount(const LineMeasure::Shared &m) {
        return m->count + (m->lpartial ? 1 : 0);
    }

    LineMeasure::Shared LineMeasure::identity() {
        return std::make_shared<LineMeasure>();
    }

    LineMeasure::Shared LineMeasure::add(const LineMeasure::Shared &lhs, const LineMeasure::Shared &rhs) {
        return std::make_shared<LineMeasure>(*lhs, *rhs);
    }

    LineMeasure::Shared LineMeasure::accumulate(const Slice<char> &vec) {
        auto acc = identity();
        auto tmp = identity();

        for (auto it = vec.begin(); it != vec.end(); ++it) {
            LineMeasure rhs(*it);
            new (acc.get()) LineMeasure(*tmp, rhs);
            new (tmp.get()) LineMeasure(*acc);
        }

        return acc;
    }

    uintptr_t LineMeasure::index(const Slice<char> &vec, uintptr_t target) {
        if (target == 0) return 0;
        uintptr_t acc = -1;
        auto it = vec.begin();
        auto end = vec.end();
        auto accm = identity();
        auto tmp = identity();

        auto current = accm->count;

        while (current < target) {
            if (it == end) {
                ++acc;
                break;
            }
            new (accm.get()) LineMeasure(*tmp, LineMeasure(*it));
            new (tmp.get()) LineMeasure(*accm);
            ++acc;
            ++it;
            current = accm->count;
        }
        assert(acc != -1);
        return acc + 1;
    }
};
