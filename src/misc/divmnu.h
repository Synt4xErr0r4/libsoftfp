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

#include <stdint.h>

int __softfp_divmnu(uint16_t q[], uint16_t r[], const uint16_t u[], const uint16_t v[], int m, int n);

/**
 * @param q[m-n+1] quotient
 * @param r[n] remainder
 * @param u[m] dividend
 * @param v[n] divisor
 * @param m number of 32-bit words in u
 * @param n number of 32-bit words in v (m >= n)
 */
static inline int divmnu(uint32_t q[], uint32_t r[], const uint32_t u[], const uint32_t v[], int m, int n) {
    return __softfp_divmnu((uint16_t *) q, (uint16_t *) r, (const uint16_t *) u, (const uint16_t *) v, m * 2, n * 2);
}
