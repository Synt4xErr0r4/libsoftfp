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

/*
 * since the macros FBITS, FEXP et al. are defined in the dynamically created config header files, the LSP does not
 * know about them. Therefore, we define some dummy values here to make the LSP happy.
 */

#ifdef __SOFTFP_LSP

#include <stdbool.h>
#include <stdint.h>

#if defined __x86_64__ || defined _M_X64 || defined i386 || defined __i386__ || defined __i386 || defined _M_IX86
#include <immintrin.h>
#endif

#ifndef FDEFINED

#define FDEC 0
#define FCAST_ONLY 0

#define FBITS 15
#define FEXP 5
#define FJBIT 0
#define FFRAC 10
#define FID _
#define FCID _

#define FSTDCOMPLEX 0

#define FDPD 0

#define FBITS 15
#define FCOMB 5
#define FSIGN 10
#define FID _

#endif

#ifndef TDEFINED

#define TDEC 0

#define TBITS 15
#define TEXP 5
#define TJBIT 0
#define TFRAC 10
#define TID _

#define TDPD 0

#define TBITS 15
#define TCOMB 5
#define TSIGN 10
#define TID _

#endif

// dummy types
typedef void *sbinary15_t;

typedef struct {
    void *Re;
    void *Im;
} scbinary15_t;

typedef void *sdecimal15_t;

#endif
