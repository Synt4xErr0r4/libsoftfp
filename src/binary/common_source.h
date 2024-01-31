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
#include "common.h"

#define fsrc_t __FBUILDTYPE(FBITS)
#define fcomplex_t __FBUILDTYPE_COMPLEX(FBITS)

#define FSIG (FFRAC + FJBIT)

#define FSET_QNAN(x, v) FCOMMON_SET_QNAN(x, FFRAC, (v))
#define FGET_QNAN(x) FCOMMON_GET_QNAN(x, FFRAC)

#define FSET_JBIT(x, v) FCOMMON_SET_JBIT(x, FFRAC, (v))
#define FGET_JBIT(x) FCOMMON_GET_JBIT(x, FFRAC)

#define FDECL(x) FCOMMON_DECL(x, FEXP, FJBIT, FFRAC)
#define FUNPACK(x, f) FCOMMON_UNPACK(x, f, FBITS, FEXP, FJBIT, FSIG)
#define FPACK(x, f) FCOMMON_PACK(x, f, FBITS, FEXP, FJBIT, FSIG)
#define FRETURN(x) FCOMMON_RETURN(x, FBITS, FEXP, FJBIT, FSIG)

#define FCLASSIFY(x) FCOMMON_CLASSIFY(x, FEXP, FJBIT, FFRAC)

#define FQNAN(x) FCOMMON_QNAN(x, FEXP, FFRAC)
#define FINF(x) FCOMMON_INF(x, FEXP)
#define FZERO(x) FCOMMON_ZERO(x, FEXP)

#define FROUND_AND_NORMALIZE(x, round) FCOMMON_ROUND_AND_NORMALIZE(x, round, FEXP, FFRAC)

#if FCAST_ONLY == 0
#include "arithmetic.h"
#include "compare.h"
#include "complex.h"
#include "intconv.h"
#endif
