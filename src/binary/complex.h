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

#if FSTDCOMPLEX == 1
#include <complex.h>

#define Re(x) creal(x)
#define Im(x) cimag(x)

#define Complex(x, y) ((x) + (y) *I)

#else
#define Re(x) (x).Re
#define Im(x) (x).Im

#define Complex(x, y) ((fcomplex_t){(x), (y)})
#endif

static inline fsrc_t __copysign(fsrc_t a, fsrc_t b) {
    FDECL(x);
    FDECL(y);
    FUNPACK(x, a);
    FUNPACK(y, b);

    x_S = y_S;

    FRETURN(x);
}

fcomplex_t fmulc(fsrc_t a, fsrc_t b, fsrc_t c, fsrc_t d) {
    fsrc_t ac = fmul(a, c);
    fsrc_t bd = fmul(b, d);
    fsrc_t ad = fmul(a, d);
    fsrc_t bc = fmul(b, c);
    fsrc_t x = fsub(ac, bd);
    fsrc_t y = fadd(ad, bc);

    FDECL(fa);
    FDECL(fb);
    FDECL(fc);
    FDECL(fd);
    FDECL(fac);
    FDECL(fbd);
    FDECL(fad);
    FDECL(fbc);
    FDECL(fx);
    FDECL(fy);

    FUNPACK(fx, x);
    FUNPACK(fy, y);

    if (fx_C == FCLS_SNAN || fy_C == FCLS_SNAN)
        feraiseexcept(FE_INVALID);

    if ((fx_C == FCLS_QNAN || fx_C == FCLS_SNAN) && (fy_C == FCLS_QNAN || fy_C == FCLS_SNAN)) {
        int recalc = 0;

        fsrc_t v1;
        fsrc_t v0;

        FDECL(f1);
        FDECL(f0);

        FZERO(f1);
        FZERO(f0);

        f1_E = FBIAS(FEXP);
        FSET_JBIT(f1, 1);

        FPACK(f1, v1);
        FPACK(f0, v0);

        FUNPACK(fa, a);
        FUNPACK(fb, b);
        FUNPACK(fc, c);
        FUNPACK(fd, d);
        FUNPACK(fac, ac);
        FUNPACK(fbd, bd);
        FUNPACK(fad, ad);
        FUNPACK(fbc, bc);

        if (fa_C == FCLS_INF || fb_C == FCLS_INF) {
            a = __copysign(fa_C == FCLS_INF ? v1 : v0, a);
            b = __copysign(fb_C == FCLS_INF ? v1 : v0, b);

            if (fc_C == FCLS_QNAN || fc_C == FCLS_SNAN)
                c = __copysign(v0, c);

            if (fd_C == FCLS_QNAN || fd_C == FCLS_SNAN)
                d = __copysign(v0, d);

            recalc = 1;
        }

        if (fc_C == FCLS_INF || fd_C == FCLS_INF) {
            c = __copysign(fc_C == FCLS_INF ? v1 : v0, c);
            d = __copysign(fd_C == FCLS_INF ? v1 : v0, d);

            if (fa_C == FCLS_QNAN || fa_C == FCLS_SNAN)
                a = __copysign(v0, a);

            if (fb_C == FCLS_QNAN || fb_C == FCLS_SNAN)
                b = __copysign(v0, b);

            recalc = 1;
        }

        if (!recalc && (fac_C == FCLS_INF || fbd_C == FCLS_INF || fad_C == FCLS_INF || fbc_C == FCLS_INF)) {
            if (fa_C == FCLS_QNAN || fa_C == FCLS_SNAN)
                a = __copysign(v0, a);

            if (fb_C == FCLS_QNAN || fb_C == FCLS_SNAN)
                b = __copysign(v0, b);

            if (fc_C == FCLS_QNAN || fc_C == FCLS_SNAN)
                c = __copysign(v0, c);

            if (fd_C == FCLS_QNAN || fd_C == FCLS_SNAN)
                d = __copysign(v0, d);

            recalc = 1;
        }

        if (recalc) {
            ac = fmul(a, c);
            bd = fmul(b, d);
            ad = fmul(a, d);
            bc = fmul(b, c);

            fsrc_t inf;

            FDECL(finf);
            FINF(finf);
            FPACK(finf, inf);

            x = fmul(inf, fsub(ac, bd));
            y = fmul(inf, fadd(ad, bc));
        }
    }

    return Complex(x, y);
}

#define SCALBN(x, n)                                                                                                   \
    do {                                                                                                               \
        x##_E += (n);                                                                                                  \
                                                                                                                       \
        if (x##_E > FMAXEXP(FEXP))                                                                                     \
            FINF(x);                                                                                                   \
        else if (x##_E < FDENORMMINEXP(FEXP, FFRAC))                                                                   \
            FZERO(x);                                                                                                  \
    } while (0)

fcomplex_t fdivc(fsrc_t a, fsrc_t b, fsrc_t c, fsrc_t d) {
    FDECL(fa);
    FDECL(fb);
    FDECL(fc);
    FDECL(fd);
    FDECL(fac);
    FDECL(fbd);
    FDECL(fad);
    FDECL(fbc);
    FDECL(fx);
    FDECL(fy);

    FUNPACK(fc, c);
    FUNPACK(fd, d);

    int32_t ilogb;
    bool logb_inf = false;

    if (fd_C == FCLS_QNAN || fd_C == FCLS_SNAN) {
        if (fc_C == FCLS_QNAN || fc_C == FCLS_SNAN || fc_C == FCLS_INF) {
            logb_inf = fc_C == FCLS_INF;
            goto nonfinite;
        }
    } else if (fc_C == FCLS_QNAN || fc_C == FCLS_SNAN) {
        if (fd_C == FCLS_INF) {
            logb_inf = true;
            goto nonfinite;
        }
    } else {
        if (fcmp(c, d) > 0) {
            if (fc_C == FCLS_INF) {
                logb_inf = true;
                goto nonfinite;
            }

            ilogb = fc_E;
        } else {
            if (fd_C == FCLS_INF) {
                logb_inf = true;
                goto nonfinite;
            }

            ilogb = fd_E;
        }
    }

    SCALBN(fc, -ilogb);
    SCALBN(fd, -ilogb);

    FPACK(fc, c);
    FPACK(fd, d);

nonfinite:;
    fsrc_t ac = fmul(a, c);
    fsrc_t bd = fmul(b, d);
    fsrc_t ad = fmul(a, d);
    fsrc_t bc = fmul(b, c);

    fsrc_t denom = fadd(fmul(c, c), fmul(d, d));

    fsrc_t x = fdiv(fadd(ac, bd), denom);
    fsrc_t y = fdiv(fsub(bc, ad), denom);

    FUNPACK(fx, x);
    FUNPACK(fy, y);

    SCALBN(fx, -ilogb);
    SCALBN(fy, -ilogb);

    FPACK(fx, x);
    FPACK(fy, y);

    if ((fx_C == FCLS_SNAN || fc_C == FCLS_QNAN) && (fy_C == FCLS_SNAN || fd_C == FCLS_QNAN)) {
        FUNPACK(fa, a);
        FUNPACK(fb, b);
        FUNPACK(fc, c);
        FUNPACK(fd, d);

        FDECL(fdenom);
        FUNPACK(fdenom, denom);

        fsrc_t inf;

        FDECL(finf);
        FINF(finf);
        FPACK(finf, inf);

        fsrc_t v1;
        fsrc_t v0;

        FDECL(f1);
        FDECL(f0);

        FZERO(f1);
        FZERO(f0);

        f1_E = FBIAS(FEXP);
        FSET_JBIT(f1, 1);

        FPACK(f1, v1);
        FPACK(f0, v0);

        if (fdenom_C == FCLS_ZERO &&
            (!(fx_C == FCLS_SNAN || fx_C == FCLS_QNAN) || !(fy_C == FCLS_SNAN || fy_C == FCLS_QNAN))) {
            x = fmul(__copysign(inf, c), a);
            y = fmul(__copysign(inf, c), b);
        } else if ((fa_C == FCLS_INF || fb_C == FCLS_INF) &&
                   (fc_C != FCLS_QNAN && fc_C != FCLS_SNAN && fc_C != FCLS_INF) &&
                   (fd_C != FCLS_QNAN && fd_C != FCLS_SNAN && fd_C != FCLS_INF)) {
            a = __copysign(fa_C == FCLS_INF ? v1 : v0, a);
            b = __copysign(fb_C == FCLS_INF ? v1 : v0, b);
            x = fmul(inf, fadd(fmul(a, c), fmul(b, d)));
            y = fmul(inf, fsub(fmul(b, c), fmul(a, d)));
        } else if (logb_inf && (fa_C != FCLS_QNAN && fa_C != FCLS_SNAN && fa_C != FCLS_INF) &&
                   (fb_C != FCLS_QNAN && fb_C != FCLS_SNAN && fb_C != FCLS_INF)) {
            c = __copysign(fc_C == FCLS_INF ? v1 : v0, c);
            d = __copysign(fd_C == FCLS_INF ? v1 : v0, d);
            x = fmul(v0, fadd(fmul(a, c), fmul(b, d)));
            y = fmul(v0, fsub(fmul(b, c), fmul(a, d)));
        }
    }

    return Complex(x, y);
}

fcomplex_t fcmulc(fcomplex_t a, fcomplex_t b) {
    return fmulc(Re(a), Im(a), Re(b), Im(b));
}

fcomplex_t fcdivc(fcomplex_t a, fcomplex_t b) {
    return fdivc(Re(a), Im(a), Re(b), Im(b));
}
