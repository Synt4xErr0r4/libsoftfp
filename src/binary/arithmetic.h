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

#include "../lsp.h"
#include "../misc/arith.h"
#include "../misc/divmnu.h"
#include "../misc/misc.h"
#include "common_source.h"
#include <stdio.h>

#define NWORDS (sizeof z_F / sizeof *z_F)

#define FARITHMETIC_BASIC_CHECK(x)                                                                                     \
    do {                                                                                                               \
        switch (x##_C) {                                                                                               \
            case FCLS_SNAN:                                                                                            \
                FSET_QNAN(x, 1);                                                                                       \
                feraiseexcept(FE_INVALID);                                                                             \
            case FCLS_QNAN:                                                                                            \
                FRETURN(x);                                                                                            \
            default:                                                                                                   \
                break;                                                                                                 \
        }                                                                                                              \
                                                                                                                       \
        if (x##_C == FCLS_DENORMAL)                                                                                    \
            feraiseexcept(FE_DENORM);                                                                                  \
    } while (0)

static inline fsrc_t faddsub(fsrc_t a, fsrc_t b, bool sub) {
    FDECL(x);
    FDECL(y);
    FDECL(z);

    FUNPACK(x, a);
    FUNPACK(y, b);

    if (sub)
        y_S = !y_S;

    FARITHMETIC_BASIC_CHECK(x);
    FARITHMETIC_BASIC_CHECK(y);

    if (x_C == FCLS_INF && y_C == FCLS_INF && x_S != y_S) {
        feraiseexcept(FE_INVALID);
        z_S = 1;
        FQNAN(z);
        goto done;
    }

    if (x_C == FCLS_ZERO && y_C == FCLS_ZERO) {
        z_S = (x_S && y_S) || (fegetround() == FE_DOWNWARD);
        FZERO(z);
        goto done;
    }

    if (x_C == FCLS_INF || y_C == FCLS_ZERO)
        FRETURN(x);

    if (y_C == FCLS_INF || x_C == FCLS_ZERO)
        FRETURN(y);

    z_E = MAX(x_E, y_E);

    int round = x_E == z_E ? ARRAY_RSHIFT(y_F, z_E - y_E) : ARRAY_RSHIFT(x_F, z_E - x_E);

    uint8_t carry = addsub(z_F, x_F, y_F, sizeof x_F / sizeof *x_F, x_S != y_S);

    z_S = x_S ^ carry;

    FROUND_AND_NORMALIZE(z, round);
done:
    FRETURN(z);
}

fsrc_t fadd(fsrc_t a, fsrc_t b) {
    return faddsub(a, b, false);
}

fsrc_t fsub(fsrc_t a, fsrc_t b) {
    return faddsub(a, b, true);
}

fsrc_t fmul(fsrc_t a, fsrc_t b) {
    FDECL(x);
    FDECL(y);
    FDECL(z);

    FUNPACK(x, a);
    FUNPACK(y, b);

    FARITHMETIC_BASIC_CHECK(x);
    FARITHMETIC_BASIC_CHECK(y);

    z_S = x_S ^ y_S;

    if ((x_C == FCLS_INF && y_C == FCLS_ZERO) || (x_C == FCLS_ZERO && y_C == FCLS_INF)) {
        feraiseexcept(FE_INVALID);
        FQNAN(z);
        goto done;
    }

    if (x_C == FCLS_INF || y_C == FCLS_INF) {
        FINF(z);
        goto done;
    }

    if (x_C == FCLS_ZERO || y_C == FCLS_ZERO) {
        FZERO(z);
        goto done;
    }

    z_E = x_E + y_E;

    uint32_t prod[NWORDS * 2] = {0};
    uint32_t mult[NWORDS * 2] = {0};
    size_t shift = 0;
    uint8_t carry;

    memcpy(mult, x_F, sizeof x_F);

    __softfp_mul(prod, mult, y_F, NWORDS);

    int round = ARRAY_RSHIFT(prod, FFRAC);

    memcpy(z_F, prod, sizeof z_F);

    FROUND_AND_NORMALIZE(z, round);
done:
    FRETURN(z);
}

fsrc_t fdiv(fsrc_t a, fsrc_t b) {
    FDECL(x);
    FDECL(y);
    FDECL(z);

    FUNPACK(x, a);
    FUNPACK(y, b);

    FARITHMETIC_BASIC_CHECK(x);
    FARITHMETIC_BASIC_CHECK(y);

    z_S = x_S ^ y_S;

    if ((x_C == FCLS_INF && y_C == FCLS_INF) || (x_C == FCLS_ZERO && y_C == FCLS_ZERO)) {
        feraiseexcept(FE_INVALID);
        FQNAN(z);
        goto done;
    }

    if (x_C == FCLS_INF) {
        FINF(z);
        goto done;
    }

    if (y_C == FCLS_ZERO) {
        feraiseexcept(FE_DIVBYZERO);
        FINF(z);
        goto done;
    }

    if (y_C == FCLS_INF || x_C == FCLS_ZERO) {
        FZERO(z);
        goto done;
    }

    z_E = x_E - y_E;

    uint32_t rem[NWORDS] = {0};
    uint32_t quotient[NWORDS * 2] = {0};
    uint32_t dividend[NWORDS * 2] = {0};

    memcpy(&dividend[NWORDS], x_F, sizeof x_F);

    divmnu(quotient, rem, dividend, y_F, NWORDS * 4, BITS_TO_WORDS(2 * ARRAY_MSB(y_F)));

    int round = ARRAY_RSHIFT(quotient, NWORDS * 32 - FFRAC);

    if (!ARRAY_IS_ZERO(rem))
        round |= BIT_STICKY;

    memcpy(z_F, quotient, sizeof z_F);

    FROUND_AND_NORMALIZE(z, round);
done:
    FRETURN(z);
}

fsrc_t fneg(fsrc_t a) {
    FDECL(x);
    FUNPACK(x, a);

    if (x_C != FCLS_SNAN && x_C != FCLS_QNAN)
        x_S ^= 1;

    FRETURN(x);
}
