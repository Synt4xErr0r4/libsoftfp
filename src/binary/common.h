/* MIT License
 *
 * Copyright (c) 2024 Thomas Kasper
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once
#include "softfp.h"

#include "../misc/misc.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define __X_FBUILDTYPE(B) sbinary##B##_t
#define __FBUILDTYPE(B) __X_FBUILDTYPE(B)

#define __X_FBUILDTYPE_COMPLEX(B) scbinary##B##_t
#define __FBUILDTYPE_COMPLEX(B) __X_FBUILDTYPE_COMPLEX(B)

typedef enum {
    FCLS_ZERO,     // E=0 J=0 F=0
    FCLS_DENORMAL, // E=0 J=0 F!=0
    FCLS_PSEUDO,   // E=0 J=1
    FCLS_NORMAL,   // E>0 E<emax
    FCLS_INF,      // E=emax F=0
    FCLS_SNAN,     // E=emax MSB(F)=0
    FCLS_QNAN,     // E=emax MSB(F)=1
    FCLS_ILLEGAL   // E>0 J=0      [here treated as sNaN]
} fclass_t;

#define FSPECIALEXP(E) ((1 << (E)) - 1)
#define FMAXEXP(E) ((1 << ((E) -1)) - 1)
#define FMINEXP(E) (2 - (1 << ((E) -1)))
#define FDENORMMINEXP(E, F) (FMINEXP(E) - (F))
#define FBIAS(E) FMAXEXP((E))

#define FCOMMON_GET_NTH(x, n) (x##_F[BITS_TO_WORDS((n)) - 1] & (1 << ((n) % 32)))
#define FCOMMON_SET_NTH(x, n, v) __set_nth_bit(&x##_F[BITS_TO_WORDS((n)) - 1], (n) % 32, (v) &1)

static inline void __set_nth_bit(uint32_t *arr, uint32_t bit, bool val) {
    if (val)
        *arr |= 1 << bit;
    else
        *arr &= ~(1 << bit);
}

#define FCOMMON_GET_JBIT(x, F) FCOMMON_GET_NTH(x, (F))
#define FCOMMON_SET_JBIT(x, F, v) FCOMMON_SET_NTH(x, (F), (v))

#define FCOMMON_GET_QNAN(x, F) FCOMMON_GET_NTH(x, (F) -1)
#define FCOMMON_SET_QNAN(x, F, v) FCOMMON_SET_NTH(x, (F) -1, (v))

/*
 * x_S = sign bit
 * x_E = exponent field
 * x_F = significand/mantissa/fraction (at least FRAC+JBIT+1 bits are allocated)
 * x_C = classification
 */
#define FCOMMON_DECL(x, E, J, F)                                                                                       \
    _Static_assert((E) < 31, "unsupported exponent size");                                                             \
    _Static_assert((((E) + (F) + (J) + 1) % 8) == 0, "illegal bit count");                                             \
    _Static_assert((E) + (J) + (F) + 1 <= 8 * sizeof(fsrc_t), "bit count type mismatch");                              \
    bool x##_S;                                                                                                        \
    int32_t x##_E;                                                                                                     \
    uint32_t x##_F[BITS_TO_WORDS((F) + (J)) + ((((F) + (J)) % 32) != 0)];                                              \
    fclass_t x##_C;

/* TODO not verified on big-endian systems */

#define FCOMMON_UNPACK(x, f, B, E, J, F)                                                                               \
    do {                                                                                                               \
        const uint8_t *const arr = (uint8_t *) &(f);                                                                   \
        const size_t BYTES = BITS_TO_BYTES((B));                                                                       \
        const size_t E_BYTES = BITS_TO_BYTES((E) + 1);                                                                 \
        const size_t F_UNUSED = (F) % 32;                                                                              \
                                                                                                                       \
        x##_S = (arr[BYTES - 1] & 0x80) == 0x80;                                                                       \
                                                                                                                       \
        memcpy(&x##_E, &arr[BYTES - E_BYTES], E_BYTES);                                                                \
                                                                                                                       \
        x##_E >>= (F) % 8;                                                                                             \
        x##_E &= ~(UINT32_MAX << (E));                                                                                 \
                                                                                                                       \
        memcpy(x##_F, arr, BITS_TO_BYTES((F)));                                                                        \
                                                                                                                       \
        if (F_UNUSED)                                                                                                  \
            x##_F[BITS_TO_WORDS((F)) - 1] &= ~(UINT32_MAX << F_UNUSED);                                                \
                                                                                                                       \
        FCOMMON_CLASSIFY(x, (E), (J), (F));                                                                            \
    } while (0)

#define FCOMMON_PACK(x, f, B, E, J, F)                                                                                 \
    do {                                                                                                               \
        uint8_t *const arr = (uint8_t *) &(f);                                                                         \
        const size_t F_BYTES = BITS_TO_BYTES((F));                                                                     \
        const size_t E_BYTES = BITS_TO_BYTES((E));                                                                     \
                                                                                                                       \
        x##_E = (x##_E + FBIAS((E))) & ((1 << (E)) - 1);                                                               \
                                                                                                                       \
        if (!(J))                                                                                                      \
            FCOMMON_SET_JBIT(x, (F), 0);                                                                               \
                                                                                                                       \
        if ((F) % 32) {                                                                                                \
            x##_F[BITS_TO_WORDS((F)) - 1] |= x##_E << ((F) % 32);                                                      \
            x##_E >>= (F) % 32;                                                                                        \
            if (E_BYTES - 1)                                                                                           \
                memcpy(&arr[F_BYTES], &x##_E, E_BYTES - 1);                                                            \
        } else                                                                                                         \
            memcpy(arr, x##_F + F_BYTES, E_BYTES);                                                                     \
                                                                                                                       \
        memcpy(arr, x##_F, MIN(sizeof x##_F, sizeof(f)));                                                              \
                                                                                                                       \
        if (x##_S)                                                                                                     \
            arr[BITS_TO_BYTES((B)) - 1] |= 0x80;                                                                       \
        else                                                                                                           \
            arr[BITS_TO_BYTES((B)) - 1] &= 0x7F;                                                                       \
    } while (0)

#define FCOMMON_RETURN(x, B, E, J, F)                                                                                  \
    do {                                                                                                               \
        fsrc_t r;                                                                                                      \
        FCOMMON_PACK(x, r, (B), (E), (J), (F));                                                                        \
        return r;                                                                                                      \
    } while (0)

#define FCOMMON_CLASSIFY(x, E, J, F)                                                                                   \
    do {                                                                                                               \
        bool __frac_zero = ARRAY_IS_ZERO(x##_F);                                                                       \
        if (!x##_E) {                                                                                                  \
            if (__frac_zero)                                                                                           \
                x##_C = FCLS_ZERO;                                                                                     \
            else if ((J) && FCOMMON_GET_JBIT(x, (F))) {                                                                \
                x##_C = FCLS_PSEUDO;                                                                                   \
                x##_E = 1 - FBIAS((E));                                                                                \
            } else {                                                                                                   \
                int32_t __frac_shift = (F) -ARRAY_MSB(x##_F);                                                          \
                x##_C = FCLS_DENORMAL;                                                                                 \
                x##_E = 1 - FBIAS((E)) - __frac_shift;                                                                 \
                ARRAY_LSHIFT(x##_F, __frac_shift);                                                                     \
            }                                                                                                          \
        } else if ((J) && !FCOMMON_GET_JBIT(x, (F)))                                                                   \
            x##_C = FCLS_ILLEGAL;                                                                                      \
        else if (x##_E == FSPECIALEXP((E))) {                                                                          \
            x##_E -= FBIAS((E));                                                                                       \
            if (__frac_zero)                                                                                           \
                x##_C = FCLS_INF;                                                                                      \
            else if (FCOMMON_GET_QNAN(x, (F)))                                                                         \
                x##_C = FCLS_QNAN;                                                                                     \
            else                                                                                                       \
                x##_C = FCLS_SNAN;                                                                                     \
        } else {                                                                                                       \
            x##_C = FCLS_NORMAL;                                                                                       \
            x##_E -= FBIAS((E));                                                                                       \
            if (!(J))                                                                                                  \
                FCOMMON_SET_JBIT(x, (F), 1);                                                                           \
        }                                                                                                              \
    } while (0)

#define FCOMMON_INF(x, E)                                                                                              \
    do {                                                                                                               \
        memset(x##_F, 0, sizeof x##_F);                                                                                \
        x##_E = FSPECIALEXP((E)) - FBIAS((E));                                                                         \
    } while (0)

#define FCOMMON_QNAN(x, E, F)                                                                                          \
    do {                                                                                                               \
        FCOMMON_INF(x, (E));                                                                                           \
        FCOMMON_SET_QNAN(z, (F), 1);                                                                                   \
        FCOMMON_SET_JBIT(z, (F), 1);                                                                                   \
    } while (0)

#define FCOMMON_ZERO(x, E)                                                                                             \
    do {                                                                                                               \
        memset(x##_F, 0, sizeof x##_F);                                                                                \
        x##_E = -FBIAS((E));                                                                                           \
    } while (0)

#define FCOMMON_ROUND_AND_NORMALIZE(x, round_ctrl, E, F)                                                               \
    do {                                                                                                               \
        int __round = (round_ctrl);                                                                                    \
        bool __should_round = SHOULD_ROUND(x##_S, __round);                                                            \
                                                                                                                       \
        do {                                                                                                           \
            bool __should_normalize = false;                                                                           \
            int32_t __msb;                                                                                             \
                                                                                                                       \
            if (__should_round && ARRAY_INCREMENT(x##_F)) {                                                            \
                __should_normalize = true;                                                                             \
                __msb = BITS_TO_WORDS((F));                                                                            \
                feraiseexcept(FE_INEXACT);                                                                             \
            } else {                                                                                                   \
                __msb = ARRAY_MSB(x##_F);                                                                              \
                __should_normalize = __msb != (F);                                                                     \
            }                                                                                                          \
                                                                                                                       \
            int32_t __shift = __msb - (F);                                                                             \
            x##_E += __shift;                                                                                          \
                                                                                                                       \
            if (x##_E > FMAXEXP((E))) { /* overflow to inf */                                                          \
                FCOMMON_INF(x, (E));                                                                                   \
                feraiseexcept(FE_OVERFLOW);                                                                            \
                break;                                                                                                 \
            }                                                                                                          \
                                                                                                                       \
            if (x##_E < FDENORMMINEXP((E), (F))) { /* underflow to 0 */                                                \
                FZERO(x);                                                                                              \
                feraiseexcept(FE_UNDERFLOW);                                                                           \
                break;                                                                                                 \
            }                                                                                                          \
                                                                                                                       \
            if (x##_E < FMINEXP((E))) { /* denormal result */                                                          \
                __shift += FMINEXP((E)) - x##_E;                                                                       \
                x##_E = FMINEXP((E));                                                                                  \
            }                                                                                                          \
                                                                                                                       \
            __round = ARRAY_RSHIFT(x##_F, __shift);                                                                    \
        } while ((__should_round = SHOULD_ROUND(x##_S, __round)));                                                     \
    } while (0);
