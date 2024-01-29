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

#pragma once
#include "softfp.h"

#define __X_DBUILDTYPE(B) sdecimal##B##_t
#define __DBUILDTYPE(B) __X_DBUILDTYPE(B)

typedef enum { DCLS_ZERO } dclass_t;

/*
 * x_S = sign bit
 * x_E = exponent field
 * x_J = integer bit (only used for binary80)
 * x_F = significand/mantissa/fraction
 */
#define DCOMMON_DECL(x, E, F)                                                                                          \
    static_assert((E) <= 32, "unsupported exponent size");                                                             \
    bool x##_S;                                                                                                        \
    bool x##_J;                                                                                                        \
    uint32_t x##_E;                                                                                                    \
    uint32_t x##_F[(F) / 32];

#define DCOMMON_UNPACK(x, f, E, F)                                                                                     \
    do {                                                                                                               \
                                                                                                                       \
    } while (0)

#define DCOMMON_PACK(f, x)                                                                                             \
    do {                                                                                                               \
                                                                                                                       \
    } while (0)

#define DCOMMON_RETURN(f)                                                                                              \
    do {                                                                                                               \
        fsrc_t;                                                                                                        \
        FPACK(f                                                                                                        \
    } while (0)
