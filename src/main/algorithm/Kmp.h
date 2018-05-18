
#ifndef STRING_MATCH_KMP_H
#define STRING_MATCH_KMP_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdio.h>
#include <stdlib.h>
#include "basic/stdint.h"
#include <string>
#include <memory>
#include <assert.h>

#include "StringMatch.h"
#include "AlgorithmWrapper.h"
#include "support/StringRef.h"

namespace StringMatch {

template <typename CharTy>
class KmpImpl {
public:
    typedef KmpImpl<CharTy>     this_type;
    typedef CharTy              char_type;
    typedef std::size_t         size_type;

private:
    std::unique_ptr<int[]> kmp_next_;

public:
    KmpImpl() : kmp_next_() {}
    ~KmpImpl() {
        this->destroy();
    }

    static const char * name() { return "Kmp"; }
    static bool need_preprocessing() { return true; }

    bool is_alive() const {
        return (this->kmp_next_.get() != nullptr);
    }

    void destroy() {
        this->kmp_next_.reset();
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) {
        assert(pattern != nullptr);
        int * kmp_next = new int[length + 1];
        if (kmp_next != nullptr) {
            kmp_next[0] = -1;
            kmp_next[1] = 0;
            for (size_t index = 1; index < length; ++index) {
                if (pattern[index] == pattern[kmp_next[index - 1]]) {
                    kmp_next[index + 1] = kmp_next[index] + 1;
                }
                else {
                    kmp_next[index + 1] = 0;
                }
            }
        }
        this->kmp_next_.reset(kmp_next);
        return (kmp_next != nullptr);
    }

    /* Searching */
    int search(const char_type * text, size_type text_len,
               const char_type * pattern_in, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern_in != nullptr);

        int * kmp_next = this->kmp_next_.get();
        assert(kmp_next != nullptr);

        if (pattern_len <= text_len) {
            register const char * target = text;
            register const char * pattern = pattern_in;

            const char * target_end = text + (text_len - pattern_len);
            const char * pattern_end = pattern + pattern_len;
            do {
                if (*target != *pattern) {
                    int search_index = (int)(pattern - pattern_in);
                    if (search_index == 0) {
                        target++;
                        if (target > target_end) {
                            // Not found
                            return Status::NotFound;
                        }
                    }
                    else {
                        assert(search_index >= 1);
                        int search_offset = kmp_next[search_index];
                        int target_offset = search_index - search_offset;
                        assert(target_offset >= 1);
                        pattern = pattern_in + search_offset;
                        target = target + target_offset;
                        if (target > target_end) {
                            // Not found
                            return Status::NotFound;
                        }
                    }
                }
                else {
                    target++;
                    pattern++;
                    if (pattern >= pattern_end) {
                        // Has found
                        assert((target - text) >= (intptr_t)pattern_len);
                        int pos = (int)((target - text) - (intptr_t)pattern_len);
                        assert(pos >= 0);
                        return pos;
                    }
                    assert(target < (text + text_len));
                }
            } while (1);
        }

        return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< KmpImpl<char> >    Kmp;
} // namespace AnsiString

namespace UnicodeString {
    typedef AlgorithmWrapper< KmpImpl<wchar_t> > Kmp;
} // namespace UnicodeString

} // namespace StringMatch

#endif // STRING_MATCH_KMP_H