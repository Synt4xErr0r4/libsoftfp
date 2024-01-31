#include "arith.h"

#include "misc.h"

#ifdef X86
#include <immintrin.h>
#endif

static inline uint32_t addcarry(uint32_t a, uint32_t b, uint8_t *carry) {
    uint32_t c;
#ifdef X86
    *carry = _addcarry_u32(*carry, a, b, &c);
#else
    c = a + b + *carry;

    *carry = (c <= a && c <= b) ? 1 : 0; // overflow check
#endif
    return c;
}

static inline uint32_t subborrow(uint32_t a, uint32_t b, uint8_t *carry) {
    uint32_t c;
#ifdef X86
    *carry = _subborrow_u32(*carry, a, b, &c);
#else
    c = a - b - *carry;

    *carry = ((b == UINT32_MAX && *carry) || b + *carry > a) ? 1 : 0; // overflow check
#endif
    return c;
}

int __softfp_add(uint32_t r[], const uint32_t a[], const uint32_t b[], size_t n) {
    uint8_t carry = 0;

    for (size_t i = 0; i < n; ++i)
        r[i] = addcarry(a[i], b[i], &carry);

    return carry;
}

int __softfp_sub(uint32_t r[], const uint32_t a[], const uint32_t b[], size_t n) {
    uint8_t carry = 1;

    for (size_t i = 0; i < n; ++i)
        r[i] = subborrow(a[i], b[i], &carry);

    if (!carry)
        return 0;

    for (size_t i = 0; i < n; ++i)
        r[i] = ~subborrow(r[i], 0, &carry);

    return 1;
}

int __softfp_mul(uint32_t r[], uint32_t a[], const uint32_t b[], size_t n) {
    uint8_t carry = 0;

    for (size_t i = 0, shift = 0; i < 32 * n; ++i, ++shift)
        if (b[BITS_TO_WORDS(i + 1) - 1] & (1 << (i % 32))) {
            __softfp_arr_shift(a, 2 * n, shift);

            carry = 0;
            shift = 0;

            for (size_t j = 0; j < 2 * n; ++j)
                r[j] = addcarry(r[j], a[j], &carry);
        }

    return carry;
}
