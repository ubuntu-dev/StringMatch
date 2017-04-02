#ifndef JIMI_STRING_MATCH_H
#define JIMI_STRING_MATCH_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#if defined(_M_X64) || defined(_M_ARM) || defined(_M_ARM64)
    #define StringMatch_Unaligned __unaligned
#else
    #define StringMatch_Unaligned
#endif

#ifdef __cplusplus
    extern "C++" {
        template <typename CountOfType, size_t SizeOfArray>
        char(*StringMatch_CountOf_Helper(StringMatch_Unaligned CountOfType(&_Array)[SizeOfArray]))[SizeOfArray];

        #define StringMatch_CountOf(_Array) (sizeof(*StringMatch_CountOf_Helper(_Array)) + 0)
    }
#else
    #define StringMatch_CountOf(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

namespace StringMatch {
namespace AnsiString {

class kmp {
private:
    const char * pattern_;
    size_t pattern_len_;
    const char * text_;
    size_t text_len_;

    int * kmpNext_;

public:
    kmp() : pattern_(nullptr), pattern_len_(0),
            text_(nullptr), text_len_(0), kmpNext_(nullptr) {
        init();
    }
    ~kmp() {
        release();
    }

    void pattern(const char * pattern, size_t length) {
        return preprocessing(pattern, strlen(pattern));
    }

    void pattern(const char * pattern) {
        return preprocessing(pattern, strlen(pattern));
    }

    void pattern(const std::string & pattern) {
        return preprocessing(pattern.c_str(), pattern.size());
    }

    int match(const char * text, size_t length) {
        return search(text, length);
    }

    int match(const char * text) {
        return search(text, strlen(text));
    }

    int match(const std::string & text) {
        return search(text.c_str(), text.size());
    }

    void display(int index_of) {
        printf("text     = \"%s\", text_len = %" PRIuPTR "\n", text_, text_len_);
        printf("pattern  = \"%s\", pattern_len = %" PRIuPTR "\n", pattern_, pattern_len_);
        printf("index_of = %d\n", index_of);
        printf("\n");
    }

private:
    void init() {
    }

    void release() {
        if (kmpNext_ != nullptr) {
            delete[] kmpNext_;
            kmpNext_ = nullptr;
        }
    }

    void preprocessing(const char * pattern, size_t length) {
        assert(pattern != nullptr);
        pattern_ = pattern;
        pattern_len_ = length;

        int * kmpNext = new int[length + 1];
        if (kmpNext != nullptr) {
            kmpNext[0] = -1;
            kmpNext[1] = 0;
            for (size_t index = 1; index < length; ++index) {
                if (pattern[index] == pattern[kmpNext[index - 1]]) {
                    kmpNext[index + 1] = kmpNext[index] + 1;
                }
                else {
                    kmpNext[index + 1] = 0;
                }
            }
        }
        if (kmpNext_ != nullptr)
            delete[] kmpNext_;
        kmpNext_ = kmpNext;
    }

    int search(const char * text, size_t length) {
        text_ = text;
        text_len_ = length;

        assert(text != nullptr);
        assert(pattern_ != nullptr);

        size_t pattern_len = pattern_len_;
        if (length < pattern_len) {
            return -1;
        }

        register const char * target = text;
        register const char * pattern = pattern_;

        int pos = -1;        
        if ((size_t)target & (size_t)pattern) {
            const char * target_end = text + (length - pattern_len);
            const char * pattern_end = pattern_ + pattern_len;
            do {
                if (*target != *pattern) {
                    int search_index = (int)(pattern - pattern_);
                    if (search_index == 0) {
                        target++;
                        if (target >= target_end) {
                            return -1;
                        }
                    }
                    else {
                        assert(search_index >= 1);
                        int search_offset = kmpNext_[search_index];
                        int target_offset = search_index - search_offset;
                        assert(target_offset >= 1);
                        pattern = pattern_ + search_offset;
                        target = target + target_offset;
                        if (target >= target_end) {
                            return -1;
                        }
                    }
                }
                else {
                    target++;
                    pattern++;
                    if (pattern >= pattern_end) {
                        assert((target - text) >= (ptrdiff_t)pattern_len);
                        pos = (int)((target - text) - (ptrdiff_t)pattern_len);
                        assert(pos >= 0);
                        break;
                    }
                    assert(target < (text + length));
                }
            } while (1);
        }
        return pos;
    }
};

} // namespace AnsiString
} // namespace StringMatch

#endif // JIMI_STRING_MATCH_H
