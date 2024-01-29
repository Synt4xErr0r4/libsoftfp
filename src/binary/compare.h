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

#define CMP_NAN 2

static inline int cmpimpl(fsrc_t a, fsrc_t b) {
    FDECL(x);
    FDECL(y);

    FUNPACK(x, a);
    FUNPACK(y, b);

    if (x_C == FCLS_SNAN || y_C == FCLS_SNAN) {
        feraiseexcept(FE_INVALID);
        return CMP_NAN;
    }

    if (x_C == FCLS_QNAN || y_C == FCLS_QNAN)
        return CMP_NAN;

    if (x_C == FCLS_INF) {
        if (y_C != FCLS_INF || x_S != y_S)
            return x_S ? -1 : 1;

        return 0;
    }

    if (y_C == FCLS_INF)
        return y_S ? 1 : -1;

    if (x_C == FCLS_ZERO)
        return y_C == FCLS_ZERO ? 0 : y_S ? 1 : -1;

    if (x_S > y_S)
        return -1;

    if (x_S < y_S)
        return 1;

    if (x_E > y_E)
        return 1;

    if (x_E < y_E)
        return -1;

    while (true) {
        int msb_x = ARRAY_MSB(x_F);
        int msb_y = ARRAY_MSB(y_F);

        if (msb_x == msb_y) {
            if (msb_x == -1)
                return 0;

            FCOMMON_SET_NTH(x, msb_x, 0);
            FCOMMON_SET_NTH(y, msb_y, 0);
            continue;
        }

        if (x_S)
            return msb_x > msb_y ? -1 : 1;

        return msb_x > msb_y ? 1 : -1;
    }
}

int fcmp(fsrc_t a, fsrc_t b) {
    int cmp = cmpimpl(a, b);

    if (cmp == CMP_NAN)
        return 1;

    return cmp;
}

int funord(fsrc_t a, fsrc_t b) {
    FDECL(x);
    FDECL(y);

    FUNPACK(x, a);
    FUNPACK(y, b);

    if (x_C == FCLS_SNAN || y_C == FCLS_SNAN) {
        feraiseexcept(FE_INVALID);
        return 1;
    }

    return x_C == FCLS_QNAN || y_C == FCLS_QNAN;
}

int feq(fsrc_t a, fsrc_t b) {
    int cmp = cmpimpl(a, b);
    return cmp == CMP_NAN ? 1 : cmp;
}

int fne(fsrc_t a, fsrc_t b) {
    int cmp = cmpimpl(a, b);
    return cmp == CMP_NAN ? 1 : cmp;
}

int fge(fsrc_t a, fsrc_t b) {
    int cmp = cmpimpl(a, b);
    return cmp == CMP_NAN ? -1 : cmp;
}

int flt(fsrc_t a, fsrc_t b) {
    int cmp = cmpimpl(a, b);
    return cmp == CMP_NAN ? 1 : cmp;
}

int fle(fsrc_t a, fsrc_t b) {
    int cmp = cmpimpl(a, b);
    return cmp == CMP_NAN ? 1 : cmp;
}

int fgt(fsrc_t a, fsrc_t b) {
    int cmp = cmpimpl(a, b);
    return cmp == CMP_NAN ? -1 : cmp;
}
