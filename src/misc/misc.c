/* MIT License
 *
 * Copyright (c) 2023 Thomas Kasper
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "misc.h"

#include <string.h>

#ifdef X86
#include <immintrin.h>
#endif

bool __softfp_mem_chk_zero(const void *vp, size_t n) {
    const uint8_t *arr = (const uint8_t *) vp;

    while (n--)
        if (*arr++)
            return false;

    return true;
}

int32_t __softfp_bitscan(uint32_t *arr, size_t n, bool reverse) {
    if (reverse)
        for (size_t i = n; i > 0; --i) {
            size_t v = arr[i - 1];

            if (!v)
                continue;

#ifdef X86 /* use `bsr` or `lzcnt` instruction */
            return _bit_scan_reverse(v) + 32 * (i - 1);
#else
            for (size_t j = 32; j > 0; --j)
                if (v & (1 << (j - 1)))
                    return j - 1 + 32 * (i - 1);
#endif
        }
    else
        for (size_t i = 0; i < n; ++i) {
            size_t v = arr[i];

            if (!v)
                continue;

#ifdef X86 /* use `bsf` or `tzcnt` instruction */
            return _bit_scan_forward(v) + 32 * i;
#else
            for (size_t j = 0; j < 32; ++j)
                if (v & (1 << j))
                    return j + 32 * i;
#endif
        }

    return -1; // no bit found
}

int32_t __softfp_revbitscan(uint8_t *arr, size_t n) {
    for (size_t i = n; i > 0; --i) {
        size_t v = arr[i - 1];

        if (!v)
            continue;

#ifdef X86 /* use `bsr` or `lzcnt` instruction */
        return _bit_scan_reverse(v) + 8 * (i - 1);
#else
        for (size_t j = 8; j > 0; --j)
            if (v & (1 << (j - 1)))
                return j - 1 + 8 * (i - 1);
#endif
    }

    return -1; // no bit found
}

int __softfp_arr_shift(uint32_t *arr, size_t n, int32_t shift) {
    if (shift == 0 || !n || !arr)
        return 0;

    if (shift < 0) { // shift right
        shift = -shift;

        bool sticky = false; // OR of all bits shifted out (except round bit)
        bool round = false;  // most significand bit shifted out

        size_t wordshift = shift / 32;
        size_t bitshift = shift % 32;

        for (size_t i = 0; i < n; ++i) {
            size_t val = arr[i];

            if (shift > 32 * (i + 1)) {
                if (!sticky)
                    sticky = !!val;
                continue;
            }

            if (shift > 32 * i) {
                size_t bits = bitshift ? bitshift - 1 : 31;

                round = !!(val & 1 << bits);

                if (!sticky)
                    sticky = !!(val & ((1 << bits) - 1));

                if (bitshift)
                    arr[i - wordshift] = val >> bitshift;

                continue;
            }

            if (bitshift) {
                arr[i - wordshift - 1] |= val << (32 - bitshift);
                arr[i - wordshift - 0] = val >> bitshift;
            } else
                arr[i - wordshift] = val;
        }

        if (bitshift)
            arr[n - wordshift - 1] &= (1 << (32 - bitshift)) - 1;

        for (size_t i = 0; i < wordshift; ++i)
            arr[n - i - 1] = 0;

        return (sticky ? BIT_STICKY : 0) | (round ? BIT_ROUND : 0) | ((arr[0] & 1) ? BIT_GUARD : 0);
    } else {                   // shift left
        if (shift >= 32 * n) { // shifting more bits than available
            memset(arr, 0, n * sizeof(uint32_t));
            return 0;
        }

        size_t wordshift = shift / 32;
        size_t bitshift = shift % 32;

        for (size_t i = wordshift; i < n; ++i) {
            size_t val = arr[n - i - 1];

            if (bitshift) {
                arr[n - i + wordshift - 1] = val << bitshift;

                if (i != wordshift)
                    arr[n - i + wordshift - 0] |= val >> (32 - bitshift);
            } else
                arr[n - i + wordshift - 1] = val;
        }

        for (size_t i = 0; i < wordshift; ++i)
            arr[i] = 0;

        return 0;
    }
}

bool __softfp_should_round(bool sign, int rounding_mode) {
    bool guard = !!(rounding_mode & BIT_GUARD);
    bool round = !!(rounding_mode & BIT_ROUND);
    bool sticky = !!(rounding_mode & BIT_STICKY);

    switch (fegetround()) {
        case FE_TONEAREST: // round to nearest, ties to even
            return (round && sign) || (guard && round);

        case FE_DOWNWARD: // toward -inf
            return sign && (round || sticky);

        case FE_UPWARD: // toward +inf
            return !sign && (round || sticky);

        case FE_TOWARDZERO: // toward 0
        default:
            return false;
    }
}

int __softfp_arr_inc(uint32_t *arr, size_t n) {
    uint8_t carry = 1;
    for (size_t i = 0; i < n; ++i) {
        uint32_t out;
#ifdef X86
        carry = _addcarry_u32(carry, arr[i], 0, &out);
#else
        out = arr[i];

        if (carry) {
            ++out;
            if (out)
                carry = 0;
        }
#endif
        arr[i] = out;
    }
    return carry;
}
