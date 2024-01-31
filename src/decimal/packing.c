#include "packing.h"
#include "../misc/arith.h"
#include "common.h"

#include <string.h>

#ifdef X86
#include <immintrin.h>
#endif

static inline void unpack(bool *restrict sign, uint8_t significand[], int32_t *restrict exponent,
                          dclass_t *restrict class, uint8_t *restrict data, size_t ncomb, size_t nsig, bool dpd) {

    const size_t nbytes = BITS_TO_BYTES(ncomb + nsig + 1);

    const uint32_t bias = 3 * (1 << (ncomb - 6)) - 1; // = (3 * 2^expbits) / 2 - 1

    size_t sigbytes = nsig / 8;
    size_t sigbits = nsig % 8;

    uint8_t msb = data[nbytes - 1];

    const size_t ndigits = nsig / 10 * 3 + 1;
    uint8_t digits[ndigits];

    uint32_t rawexp = 0;
    size_t offset = 0;

    /*
     * S = sign bit
     * E = exponent bits
     * F = significand bits
     * Q = quiet bit
     * P = payload bits
     * x = don't care
     *
     * BID:
     *
     * S00EEEEE...EFFF   exponent = 00EEE...EEE
     * S01EEEEE...EFFF   exponent = 01EEE...EEE
     * S10EEEEE...EFFF   exponent = 10EEE...EEE
     * S1100EEE...EEEF   exponent = 00EEE...EEE
     * S1101EEE...EEEF   exponent = 01EEE...EEE
     * S1110EEE...EEEF   exponent = 10EEE...EEE
     *
     * DPD:
     *
     * S00FFFEE...EEEE   exponent = 00EEE...EEE
     * S01FFFEE...EEEE   exponent = 01EEE...EEE
     * S10FFFEE...EEEE   exponent = 10EEE...EEE
     * S1100FEE...EEEE   exponent = 00EEE...EEE
     * S1101FEE...EEEE   exponent = 01EEE...EEE
     * S1110FEE...EEEE   exponent = 10EEE...EEE
     *
     * common:
     *
     * S11110xx...xxxx   +-Inf
     * S11111QP...PPPP   NaN (with payload P; qNaN if Q=0, sNaN if Q=1)
     */

    *sign = !!(msb & 0x80);

    uint32_t exp = msb & 0x60;
    uint8_t shift;

    if (exp == 0x60) {
        exp = msb & 0x18;

        if (exp == 0x18) {
            if (msb & 0x04) {
                *class = (msb & 0x02) ? DCLS_SNAN : DCLS_QNAN;
                *exponent = DEXP_NAN;
                *sign = 0; // ignore sign bit for NaN

                if (dpd)
                    goto unpack_dpd;
                else
                    goto unpack_bid;
            }

            *class = DCLS_INF;
            *exponent = DEXP_INF;
            return;
        }

        *class = DCLS_NORMAL;
        exp >>= 3;

        if (dpd) {
            digits[ndigits - 1] = ((msb >> 2) & 1) | 0x8;
            goto unpack_dpd_exp;
        }

        shift = 1;
    } else {
        *class = DCLS_NORMAL;
        exp >>= 5;

        if (dpd) {
            digits[ndigits - 1] = (msb >> 2) & 0x7;
            goto unpack_dpd_exp;
        }

        shift = 3;
    }

unpack_bid_exp:;
    for (size_t i = sigbytes; i < nbytes; ++i) {
        uint8_t byte = data[i];

        if (i == sigbytes) {
            if (shift + sigbits > 8) {
                significand[sigbytes] |= byte;

                offset = shift + sigbits - 8;
                byte = data[++i];

                significand[sigbytes + 1] |= byte & (0xFF << offset);
            } else {
                offset = sigbits + shift;

                significand[sigbytes] |= byte & (0xFF << offset);

                if (shift == 1) {
                    if (sigbits < 5)
                        significand[sigbytes] |= 0x8 << sigbits;
                    else
                        significand[sigbytes + 1] |= 0x1 << (sigbits - 5);
                }
            }

            byte >>= offset;

            offset = 8 - offset;
            rawexp = byte;
            continue;
        }

        if (i == nbytes - 1) {
            if (shift == 1) {
                rawexp |= (byte & 0x07) << offset;
                offset += 3;
            } else {
                rawexp |= (byte & 0x1F) << offset;
                offset += 5;
            }
        } else {
            rawexp |= byte << offset;
            offset += 8;
        }
    }

    *exponent = ((exp << offset) | rawexp) - bias;

unpack_bid:
    memcpy(significand, data, sigbytes);

    if (sigbits)
        significand[sigbytes] |= data[sigbytes] & ((1 << sigbits) - 1);

    // TODO bounds check

    return;

unpack_dpd_exp:;
    for (size_t i = sigbytes; i < nbytes; ++i) {
        uint8_t byte = data[i];

        if (i == sigbytes) {
            byte >>= sigbits;
            byte <<= offset;
            offset += sigbits;
        } else if (i == nbytes - 1) {
            byte &= 0x03;
            byte <<= offset;
            offset += 2;
        } else {
            byte <<= offset;
            offset += 8;
        }

        rawexp |= byte;
    }

    *exponent = ((exp << offset) | rawexp) - bias;

unpack_dpd:;
    register size_t digit = 0;

    for (size_t i = 0; i < nsig / 10; ++i) {
        size_t offset = 10 * i;

        size_t bitoff = offset % 8;
        size_t byteoff = offset / 8;

        register uint16_t block = 0;

        size_t bit = 0;

        while (bit < 10) {
            uint8_t byte = data[byteoff];

            block |= (byte >> bitoff) << bit;

            bit += 8 - bitoff;

            bitoff = 0;
            ++byteoff;
        }

        uint8_t mode;

#ifdef USE_PEXT
#undef USE_PEXT
#endif

#if defined X86 && !defined X86_NO_BMI2
#define USE_PEXT
        mode = _pext_u32(block, 0xE);
#else
        mode = (block >> 1) & 0x7;
#endif

        switch (mode) {
            case 0:
            case 1:
            case 2:
            case 3:
#ifdef USE_PEXT
                digits[digit++] = _pext_u32(block, 0x7);   // --- --- 0 111
                digits[digit++] = _pext_u32(block, 0x70);  // --- 111 0 ---
                digits[digit++] = _pext_u32(block, 0x380); // 111 --- 0 ---
#else
                digits[digit++] = block & 0x7;
                digits[digit++] = (block >> 4) & 0x7;
                digits[digit++] = (block >> 7) & 0x7;
#endif
                break;

            case 4:
#ifdef USE_PEXT
                digits[digit++] = _pext_u32(block, 0x1) | 0x8; // --- --- 100 x
                digits[digit++] = _pext_u32(block, 0x70);      // --- xxx 100 -
                digits[digit++] = _pext_u32(block, 0x380);     // xxx --- 100 -
#else
                digits[digit++] = 0x8 | (block & 0x1);
                digits[digit++] = (block >> 4) & 0x7;
                digits[digit++] = (block >> 7) & 0x7;
#endif
                break;

            case 5:
#ifdef USE_PEXT
                digits[digit++] = _pext_u32(block, 0x61);       // --- xx- 101 x
                digits[digit++] = _pext_u32(block, 0x10) | 0x8; // --- --x 101 -
                digits[digit++] = _pext_u32(block, 0x380);      // xxx --- 101 -
#else
                digits[digit++] = (block & 0x1) | ((block >> 4) & 0x6);
                digits[digit++] = 0x8 | ((block >> 4) & 0x1);
                digits[digit++] = (block >> 7) & 0x7;
#endif
                break;

            case 6:
#ifdef USE_PEXT
                digits[digit++] = _pext_u32(block, 0x301);      // xx- --- 110 x
                digits[digit++] = _pext_u32(block, 0x70);       // --- xxx 110 -
                digits[digit++] = _pext_u32(block, 0x80) | 0x8; // --x --- 110 -
#else
                digits[digit++] = (block & 0x1) | ((block >> 7) & 0x6);
                digits[digit++] = (block >> 4) & 0x7;
                digits[digit++] = 0x8 | ((block >> 7) & 0x1);
#endif
                break;

            case 7:
#ifdef USE_PEXT
                mode = _pext_u32(block, 0x60); // --- xx- 111 -
#else
                mode = (block >> 5) & 0x3;
#endif

                switch (mode) {
                    case 0:
#ifdef USE_PEXT
                        digits[digit++] = _pext_u32(block, 0x301);      // xx- 00- 111 x
                        digits[digit++] = _pext_u32(block, 0x10) | 0x8; // --- 00x 111 -
                        digits[digit++] = _pext_u32(block, 0x80) | 0x8; // --x 00- 111 -
#else
                        digits[digit++] = (block & 0x1) | ((block >> 7) & 0x6);
                        digits[digit++] = 0x8 | ((block >> 4) & 0x1);
                        digits[digit++] = 0x8 | ((block >> 7) & 0x1);
#endif
                        break;

                    case 1:
#ifdef USE_PEXT
                        digits[digit++] = _pext_u32(block, 0x1) | 0x8;  // --- 01- 111 x
                        digits[digit++] = _pext_u32(block, 0x310);      // xx- 01x 111 -
                        digits[digit++] = _pext_u32(block, 0x80) | 0x8; // --x 01- 111 -
#else
                        digits[digit++] = 0x8 | (block & 0x1);
                        digits[digit++] = ((block >> 4) & 0x1) | ((block >> 7) & 0x6);
                        digits[digit++] = 0x8 | ((block >> 7) & 0x1);
#endif
                        break;

                    case 2:
#ifdef USE_PEXT
                        digits[digit++] = _pext_u32(block, 0x1) | 0x8;  // --- 10- 111 x
                        digits[digit++] = _pext_u32(block, 0x10) | 0x8; // --- 10x 111 -
                        digits[digit++] = _pext_u32(block, 0x380);      // xxx 10- 111 -
#else
                        digits[digit++] = 0x8 | (block & 0x1);
                        digits[digit++] = 0x8 | ((block >> 4) & 0x1);
                        digits[digit++] = (block >> 7) & 0x7;
#endif
                        break;

                    case 3:
#ifdef USE_PEXT
                        digits[digit++] = _pext_u32(block, 0x1) | 0x8;  // --- 11- 111 x
                        digits[digit++] = _pext_u32(block, 0x10) | 0x8; // --- 11x 111 -
                        digits[digit++] = _pext_u32(block, 0x80) | 0x8; // --x 11- 111 -
#else
                        digits[digit++] = 0x8 | (block & 0x1);
                        digits[digit++] = 0x8 | ((block >> 4) & 0x1);
                        digits[digit++] = 0x8 | ((block >> 7) & 0x1);
#endif
                        break;
                }
                break;
        }
    }

dpd_to_bid:;
    const size_t payloadwords = BITS_TO_WORDS(nsig + 4) + ((nsig + 4) % 32 != 0);
    const size_t bufwords = CEILDIV(payloadwords, 2);

    uint32_t *accumulator = (uint32_t *) significand;
    memset(accumulator, 0, payloadwords * sizeof(uint32_t));

    uint32_t input[bufwords];
    uint32_t output[bufwords * 2];

    uint32_t x10[bufwords];
    uint32_t mult[bufwords * 2];
    uint32_t old_mult[bufwords * 2];

    memset(input, 0, sizeof input);
    memset(x10, 0, sizeof x10);
    memset(mult, 0, sizeof mult);

    x10[0] = 10;
    mult[0] = 1;

    for (size_t i = 0; i < ndigits; ++i) {
        input[0] = digits[i];

        // accumulator += <current digit> * mult

        memcpy(old_mult, mult, sizeof mult);
        memset(output, 0, sizeof mult);

        __softfp_mul(output, mult, input, bufwords);
        __softfp_add(accumulator, accumulator, output, payloadwords);

        // mult *= 10

        memset(mult, 0, sizeof mult);

        __softfp_mul(mult, old_mult, x10, bufwords);
    }
}

void __softfp_dpd_unpack(bool *restrict sign, uint32_t significand[], int32_t *restrict exponent,
                         dclass_t *restrict class, void *restrict data, size_t ncomb, size_t nsig) {
    unpack(sign, (uint8_t *) significand, exponent, class, (uint8_t *) data, ncomb, nsig, true);
}

void __softfp_bid_unpack(bool *restrict sign, uint32_t significand[], int32_t *restrict exponent,
                         dclass_t *restrict class, void *restrict data, size_t ncomb, size_t nsig) {
    unpack(sign, (uint8_t *) significand, exponent, class, (uint8_t *) data, ncomb, nsig, false);
}

void __softfp_dpd_pack(bool sign, uint32_t significand[], int32_t exponent, dclass_t class, void *restrict data,
                       size_t ncomb, size_t nsig) {
    (void) sign;
    (void) significand;
    (void) exponent;
    (void) class;
    (void) data;
    (void) ncomb;
    (void) nsig;
}

void __softfp_bid_pack(bool sign, uint32_t significand[], int32_t exponent, dclass_t class, void *restrict data,
                       size_t ncomb, size_t nsig) {
    (void) sign;
    (void) significand;
    (void) exponent;
    (void) class;
    (void) data;
    (void) ncomb;
    (void) nsig;
}
