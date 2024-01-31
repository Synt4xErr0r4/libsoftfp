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

#pragma once

#include "../lsp.h"
#include "common_source.h"

// TODO these functions are a bit janky, maybe rewrite them
// TODO check FPEs

#define BASE_CONV2INT(nanval, bits, unsigned_)                                                                         \
    do {                                                                                                               \
        FUNPACK(x, a);                                                                                                 \
                                                                                                                       \
        if (x_C == FCLS_ZERO)                                                                                          \
            goto zeroval;                                                                                              \
                                                                                                                       \
        if (x_C == FCLS_SNAN || x_C == FCLS_QNAN || x_C == FCLS_INF) {                                                 \
            feraiseexcept(FE_INVALID);                                                                                 \
            goto errval;                                                                                               \
        }                                                                                                              \
                                                                                                                       \
        if (x_E < 0 || (unsigned_ && x_S)) {                                                                           \
            feraiseexcept(FE_INEXACT);                                                                                 \
            goto zeroval;                                                                                              \
        }                                                                                                              \
                                                                                                                       \
        if (x_E >= (bits)) {                                                                                           \
            feraiseexcept(FE_INEXACT);                                                                                 \
            goto errval;                                                                                               \
        }                                                                                                              \
                                                                                                                       \
        int round = ARRAY_RSHIFT(x_F, FFRAC - x_E);                                                                    \
        bool truncated = SHOULD_ROUND(x_S, round);                                                                     \
                                                                                                                       \
        if (ARRAY_MSB(x_F) >= (bits)) {                                                                                \
            feraiseexcept(FE_INEXACT);                                                                                 \
            goto errval;                                                                                               \
        }                                                                                                              \
                                                                                                                       \
        if (truncated)                                                                                                 \
            feraiseexcept(FE_INEXACT);                                                                                 \
    } while (0)

#define CONV2INT(nanval, type, unsigned_)                                                                              \
    do {                                                                                                               \
        FDECL(x);                                                                                                      \
        BASE_CONV2INT(nanval, sizeof(type) * 8, unsigned_);                                                            \
                                                                                                                       \
        type ret = 0;                                                                                                  \
        memcpy(&ret, x_F, MIN(sizeof ret, sizeof x_F));                                                                \
                                                                                                                       \
        if (!unsigned_ && (ret > 0) == x_S)                                                                            \
            ret = -ret;                                                                                                \
                                                                                                                       \
        return ret;                                                                                                    \
    errval:                                                                                                            \
        return (nanval);                                                                                               \
    zeroval:                                                                                                           \
        return 0;                                                                                                      \
    } while (0)

int32_t ffixi32(fsrc_t a) {
    CONV2INT(INT32_MIN, int32_t, false);
}

int64_t ffixi64(fsrc_t a) {
    CONV2INT(INT64_MIN, int64_t, false);
}

uint32_t ffixu32(fsrc_t a) {
    CONV2INT(0, uint32_t, true);
}

uint64_t ffixu64(fsrc_t a) {
    CONV2INT(0, uint64_t, true);
}

#define BASE_CONV2FLOAT(unsigned_, nbytes, is_zero, a_negative, a_abs, value)                                          \
    do {                                                                                                               \
        FDECL(z);                                                                                                      \
                                                                                                                       \
        if ((is_zero)) {                                                                                               \
            FZERO(z);                                                                                                  \
            z_S = 0;                                                                                                   \
            goto done;                                                                                                 \
        }                                                                                                              \
                                                                                                                       \
        if (!unsigned_ && (a_negative)) {                                                                              \
            z_S = 1;                                                                                                   \
            a_abs;                                                                                                     \
        } else                                                                                                         \
            z_S = 0;                                                                                                   \
                                                                                                                       \
        int32_t msb = __softfp_revbitscan((uint8_t *) (value), (nbytes));                                              \
                                                                                                                       \
        if (msb > FMAXEXP(FEXP)) {                                                                                     \
            FINF(z);                                                                                                   \
            goto done;                                                                                                 \
        }                                                                                                              \
                                                                                                                       \
        int round;                                                                                                     \
                                                                                                                       \
        if (sizeof z_F < (nbytes)) {                                                                                   \
            uint32_t buffer[(nbytes)];                                                                                 \
            memcpy(buffer, (value), (nbytes));                                                                         \
            round = ARRAY_LSHIFT(buffer, FFRAC - msb);                                                                 \
            memcpy(z_F, buffer, sizeof z_F);                                                                           \
        } else {                                                                                                       \
            memset(z_F, 0, sizeof z_F);                                                                                \
            memcpy(z_F, (value), (nbytes));                                                                            \
            round = ARRAY_LSHIFT(z_F, FFRAC - msb);                                                                    \
        }                                                                                                              \
                                                                                                                       \
        bool truncated = SHOULD_ROUND(z_S, round);                                                                     \
                                                                                                                       \
        if (truncated)                                                                                                 \
            feraiseexcept(FE_INEXACT);                                                                                 \
                                                                                                                       \
        z_E = msb;                                                                                                     \
                                                                                                                       \
    done:                                                                                                              \
        FRETURN(z);                                                                                                    \
    } while (0)

#define CONV2FLOAT(unsigned_) BASE_CONV2FLOAT(unsigned_, sizeof a, a == 0, a < 0, a = -a, &a)

fsrc_t ffloati32(int32_t a) {
    CONV2FLOAT(false);
}

fsrc_t ffloati64(int64_t a) {
    CONV2FLOAT(false);
}

fsrc_t ffloatu32(uint32_t a) {
    CONV2FLOAT(true);
}

fsrc_t ffloatu64(uint64_t a) {
    CONV2FLOAT(true);
}

void ffixbit(void *r, int32_t rprec, fsrc_t a) {
    FDECL(x);

    if (!rprec)
        return;

    bool unsigned_ = rprec > 0;

    if (!unsigned_)
        rprec = -rprec;

    BASE_CONV2INT(0, rprec, unsigned_);

    memcpy(r, x_F, BITS_TO_BYTES(rprec));
    return;

errval:
    memset(r, 0, BITS_TO_BYTES(rprec));

    if (!unsigned_)
        ((uint8_t *) r)[0] |= x_S << (rprec & 7);

    return;
zeroval:
    memset(r, 0, BITS_TO_BYTES(rprec));
}

fsrc_t ffloatbit(const void *r, int32_t rprec) {
    if (!rprec) {
        FDECL(z);
        FZERO(z);
        FRETURN(z);
    }

    bool unsigned_ = rprec > 0;

    if (!unsigned_)
        rprec = -rprec;

    BASE_CONV2FLOAT(unsigned_, BITS_TO_BYTES(rprec), MEM_IS_ZERO(r, BITS_TO_BYTES(rprec)),
                    ((uint8_t *) r)[0] & (1 << (rprec & 7)), ((uint8_t *) r)[0] &= ~(1 << (rprec & 7)), r);
}
