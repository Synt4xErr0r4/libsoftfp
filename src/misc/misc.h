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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef FE_DENORM
#ifdef __FE_DENORM
#define FE_DENORM __FE_DENORM
#else
#define FE_DENORM 0
#endif
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
#define X86 1
#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* ceil(a / b) */
#define CEILDIV(a, b) (((a) + ((b) -1)) / (b))

#define BITS_TO_BYTES(bits) CEILDIV((bits), 8)
#define BYTES_TO_WORDS(bytes) CEILDIV((bytes), 4)
#define BITS_TO_WORDS(bits) CEILDIV((bits), 32)

#define ARRAY_MSB(arr) __softfp_bitscan((arr), sizeof(arr) / sizeof *(arr), true)
#define ARRAY_LSB(arr) __softfp_bitscan((arr), sizeof(arr) / sizeof *(arr), false)

// forward/reverse bitscan (LSB/MSB) array of n words
int32_t __softfp_bitscan(uint32_t *arr, size_t n, bool reverse);

// reverse bitscan (MSB) array of n bytes
int32_t __softfp_revbitscan(uint8_t *arr, size_t n);

#define ARRAY_LSHIFT(arr, shift) __softfp_arr_shift((arr), sizeof(arr) / sizeof *(arr), (shift))
#define ARRAY_RSHIFT(arr, shift) __softfp_arr_shift((arr), sizeof(arr) / sizeof *(arr), -(int32_t) (shift))

#define BIT_GUARD (1 << 2)
#define BIT_ROUND (1 << 1)
#define BIT_STICKY (1 << 0)

int __softfp_arr_shift(uint32_t *arr, size_t n, int32_t shift);

#define MEM_IS_ZERO(mem, n) __softfp_mem_chk_zero((mem), (n))
#define ARRAY_IS_ZERO(arr) MEM_IS_ZERO((arr), sizeof(arr))

bool __softfp_mem_chk_zero(const void *vp, size_t n);

#define SHOULD_ROUND(sign, round) __softfp_should_round((sign), (round))

bool __softfp_should_round(bool sign, int round);

#define ARRAY_INCREMENT(arr) __softfp_arr_inc((arr), sizeof(arr) / sizeof *(arr))

int __softfp_arr_inc(uint32_t *arr, size_t n);

#ifndef SOFTFP_HAS_FENV
#include <fenv.h>
#pragma STDC FENV_ACCESS ON
#endif
