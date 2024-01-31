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
#include "../misc/misc.h"
#include "common.h"

void __softfp_dpd_unpack(bool *restrict sign, uint32_t significand[], int32_t *restrict exponent,
                         dclass_t *restrict class, void *restrict data, size_t ncomb, size_t nsig);

void __softfp_bid_unpack(bool *restrict sign, uint32_t significand[], int32_t *restrict exponent,
                         dclass_t *restrict class, void *restrict data, size_t ncomb, size_t nsig);

void __softfp_dpd_pack(bool sign, uint32_t significand[], int32_t exponent, dclass_t class, void *restrict data,
                       size_t ncomb, size_t nsig);

void __softfp_bid_pack(bool sign, uint32_t significand[], int32_t exponent, dclass_t class, void *restrict data,
                       size_t ncomb, size_t nsig);
