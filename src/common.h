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

#define SOFTFP_FENV 1

#ifdef __FPFUN_PREFIX
# undef __FPFUN_PREFIX
#endif

#if FDEC == 1 || (defined TDEC && TDEC == 1)
# if FDPD == 1 || (defined TDPD && TDPD == 1)
#  define __FPFUN_PREFIX __dpd_
# else
#  define __FPFUN_PREFIX __bid_
# endif
#else
# define __FPFUN_PREFIX __
#endif

#define __XX_FPFUN(a, b, c, d, e) a ## b ## c ## d ## e
#define __X_FPFUN(a, b, c, d, e) __XX_FPFUN(a, b, c, d, e)
#define __FPFUN(a, b, c, d) __X_FPFUN(__FPFUN_PREFIX, a, b, c, d)

#define __FPFUN_DEFAULT(pre, suf) __FPFUN(pre, FID, suf, /**/)
#define __FPFUN_COMPLEX(kind)     __FPFUN(kind, FCID, 3, /**/)
#define __FPFUN_CONVERT(kind)     __FPFUN(kind, FID, TID, 2)

#ifdef TDEFINED
# ifdef CONV_TGT
#  undef CONV_TGT
#  undef CONV_TRUNC
#  undef fconv
# endif

# if TDEC == 1
#  include "decimal/common_target.h"
#  define CONV_TGT d
# else
#  include "binary/common_target.h"
#  define CONV_TGT f
# endif

# if (FBITS > TBITS) || (FBITS == TBITS && FDEC == 1)
#  define CONV_TRUNC 1
#  define fconv __FPFUN_CONVERT(trunc)
# else
#  define CONV_TRUNC 0
#  define fconv __FPFUN_CONVERT(extend)
# endif

# if CONV_SRC == f && CONV_TGT == f
#  include "cast/f2f.h"
# elif CONV_SRC == f && CONV_TGT == d
#  include "cast/f2d.h"
# elif CONV_SRC == d && CONV_TGT == f
#  include "cast/d2f.h"
# elif CONV_SRC == d && CONV_TGT == d
#  include "cast/d2d.h"
# else
#  error "Invalid conversion config"
# endif

#else

# ifdef fadd
#  error "fadd shouldn't be defined here"
# endif

# define fadd      __FPFUN_DEFAULT(add, 3)
# define fsub      __FPFUN_DEFAULT(sub, 3)
# define fmul      __FPFUN_DEFAULT(mul, 3)
# define fdiv      __FPFUN_DEFAULT(div, 3)
# define fneg      __FPFUN_DEFAULT(neg, 2)
# define ffixi32   __FPFUN_DEFAULT(fix, si)
# define ffixi64   __FPFUN_DEFAULT(fix, di)
# define ffixu32   __FPFUN_DEFAULT(fixuns, si)
# define ffixu64   __FPFUN_DEFAULT(fixuns, di)
# define ffixbit   __FPFUN_DEFAULT(fix, bitint)
# define ffloati32 __FPFUN_DEFAULT(floatsi, /**/)
# define ffloati64 __FPFUN_DEFAULT(floatdi, /**/)
# define ffloatu32 __FPFUN_DEFAULT(floatunsi, /**/)
# define ffloatu64 __FPFUN_DEFAULT(floatundi, /**/)
# define ffloatbit __FPFUN_DEFAULT(floatbitint, /* */)
# define fcmp      __FPFUN_DEFAULT(cmp, 2)
# define funord    __FPFUN_DEFAULT(unord, 2)
# define feq       __FPFUN_DEFAULT(eq, 2)
# define fne       __FPFUN_DEFAULT(ne, 2)
# define fge       __FPFUN_DEFAULT(ge, 2)
# define flt       __FPFUN_DEFAULT(lt, 2)
# define fle       __FPFUN_DEFAULT(le, 2)
# define fgt       __FPFUN_DEFAULT(gt, 2)

# if FDEC == 1
#  include "decimal/common_source.h"
#  define CONV_SRC d
# else
#  define fmulc  __FPFUN_COMPLEX(mul)
#  define fdivc  __FPFUN_COMPLEX(div)
#  define fcmulc __FPFUN_COMPLEX(cmul)
#  define fcdivc __FPFUN_COMPLEX(cdiv)
#  include "binary/common_source.h"
#  define CONV_SRC f
# endif
#endif
