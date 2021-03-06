
#ifndef STRING_MATCH_BM_TUNED_H
#define STRING_MATCH_BM_TUNED_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stdint.h"
#include <memory>
#include <assert.h>

#include "StringMatch.h"
#include "AlgorithmWrapper.h"
#include "jstd/scoped_ptr.h"

//
// See: http://www-igm.univ-mlv.fr/~lecroq/string/tunedbm.html#SECTION00195
//

namespace StringMatch {

template <typename CharTy>
class BMTunedImpl {
public:
    typedef BMTunedImpl<CharTy>     this_type;
    typedef CharTy                  char_type;
    typedef std::size_t             size_type;
    typedef typename jstd::uchar_traits<CharTy>::type
                                    uchar_type;

    static const size_t kMaxAscii = 256;

private:
    bool alive_;
    int bmBc_[kMaxAscii];

public:
    BMTunedImpl() : alive_(false) {}
    ~BMTunedImpl() {
        this->destroy();
    }

    static const char * name() { return "BM Tuned"; }
    static bool need_preprocessing() { return true; }

    bool is_alive() const {
        return this->alive_;
    }

    void destroy() {
        this->alive_ = false;
    }

    /* Preprocessing bad characters. */
    static void preBmBc(const char * pattern, size_type length, int * bmBc) {
        assert(pattern != nullptr);
        assert(bmBc != nullptr);

        for (size_type i = 0; i < kMaxAscii; ++i) {
            bmBc[i] = (int)length;
        }
        for (Long i = 0; i < ((Long)length - 1); ++i) {
            bmBc[(uchar_type)pattern[i]] = (int)((Long)length - 1 - i);
        }
        assert(bmBc[(uchar_type)pattern[length - 1]] > 0);
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) SM_NOEXCEPT {
        assert(pattern != nullptr);

        /* Preprocessing bad characters. */
        int * bmBc = (int *)&this->bmBc_[0];
        this_type::preBmBc(pattern, length, bmBc);

        this->alive_ = true;
        return true;
    }

    /* Searching */
    SM_NOINLINE_DECLARE(Long)
    search(const char_type * text_start, size_type text_len,
           const char_type * pattern, size_type pattern_len) const {
        assert(text_start != nullptr);
        assert(pattern != nullptr);

        if (unlikely(pattern_len == 0))
            return 0;

        if (likely(pattern_len <= text_len)) {
            int * bmBc = (int *)&this->bmBc_[0];
            assert(bmBc != nullptr);

            Long last = (Long)pattern_len - 1;
            Long last_char = (Long)(uchar_type)pattern[last];
            Long shift = (Long)pattern_len;
            bmBc[last_char] = 0;
            assert(shift > 0);

            jstd::scoped_array<char_type> text_new(new char_type[text_len + pattern_len + 1]);
            char_type * text = text_new.get();
            if (text != nullptr) {
                memcpy((void *)text, (const void *)text_start, text_len);
                memset((void *)(text + text_len), (int)(uchar_type)pattern[last],
                                pattern_len * sizeof(char_type));
                *(text + text_len + pattern_len) = char_type('\0');
            }

            const Long source_last = (Long)(text_len - pattern_len);
            const Long pattern_last = (Long)pattern_len - 1;
            Long source_offset = 0;
            do {
                register const char_type * source = text + source_offset + pattern_last;
                register const char_type * cursor = pattern + pattern_last;
                assert(source >= text && source < (text + text_len));

                Long k = bmBc[(uchar_type)*source];
                while (k != 0) {
                    source += k;
                    source_offset += k;
                    k = bmBc[(uchar_type)*source];
                    source += k;
                    source_offset += k;
                    k = bmBc[(uchar_type)*source];
                    source += k;
                    source_offset += k;
                    k = bmBc[(uchar_type)*source];
                }

                source--;
                cursor--;

                while (likely(cursor >= pattern)) {
                    if (likely(*source != *cursor)) {
                        break;
                    }
                    source--;
                    cursor--;
                }

                if (likely(cursor >= pattern)) {
                    source_offset += shift;
                }
                else {
                    // Has found
                    assert(source_offset >= 0 && source_offset < (Long)text_len);
                    return source_offset;
                }
            } while (likely(source_offset <= source_last));
        }

        return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< BMTunedImpl<char> >    BMTuned;
} // namespace AnsiString

namespace UnicodeString {
    typedef AlgorithmWrapper< BMTunedImpl<wchar_t> > BMTuned;
} // namespace UnicodeString

} // namespace StringMatch

#endif // STRING_MATCH_BM_TUNED_H
