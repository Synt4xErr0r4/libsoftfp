/** @brief calculates `a + b` */
%T %Padd%I3(%T a, %T b);

/** @brief calculates `a - b` */
%T %Psub%I3(%T a, %T b);

/** @brief calculates `a * b` */
%T %Pmul%I3(%T a, %T b);

/** @brief calculates `a / b` */
%T %Pdiv%I3(%T a, %T b);

/** @brief calculates `-a` */
%T %Pneg%I2(%T a);

/** @brief converts `a` into a signed 32-bit integer
 * Converts `a` into an unsigned 64-bit integer,
 * rounding towards zero.
 */
int32_t %Pfix%Isi(%T a);

/** @brief converts `a` into a signed 64-bit integer
 * Converts `a` into an unsigned 64-bit integer,
 * rounding towards zero.
 */
int64_t %Pfix%Idi(%T a);

/** @brief converts `a` into an unsigned 32-bit integer 
 * Converts `a` into an unsigned 64-bit integer, rounding
 * towards zero and converting negative values into zero.
 */
uint32_t %Pfixuns%Isi(%T a);

/** @brief converts `a` into an unsigned 64-bit integer 
 * Converts `a` into an unsigned 64-bit integer, rounding
 * towards zero and converting negative values into zero.
 */
uint64_t %Pfixuns%Idi(%T a);

/** @brief converts `a` into a floating-point number */
%T %Pfloat%Isi(int32_t a);

/** @brief converts `a` into a floating-point number */
%T %Pfloat%Idi(int64_t a);

/** @brief converts `a` into a floating-point number */
%T %Pfloatun%Isi(uint32_t a);

/** @brief converts `a` into a floating-point number */
%T %Pfloatun%Idi(uint64_t a);

/** @brief converts `a` into a bit-precise integer
 * converts `a` into a bit-precise integer, pointed
 * to by `r`, with `rprec` bits of precision.
 * If `rprec` is positive, the bit-precise integer
 * is assumed to be unsigned and all negative values
 * will become zero.
 * If `rprec` is negative, the bit-precise integer
 * is assumed to be signed.
 */
void %Pfix%Ibitint(void *r, int32_t rprec, %T a);

/** @brief converts `a` into a floating-point number
 * converts `a`, a bit-precise integer with `rprec`
 * bits of precision, into a floating-point number.
 * If `rprec` is positive, the bit-precise integer
 * is assumed to be unsigned.
 * If `rprec` is negative, the bit-precise integer
 * is assumed to be signed.
 */
%T %Pfloatbitint%I(const void *r, int32_t rprec);

/** @brief calculates `a <=> b`
 * Calculates `a <=> b` (-1 if `a < b`,
 * 0 if `a == b` or 1 if `a > b`).
 * Returns 1 if either argument is `NaN`.
 */
int %Pcmp%I2(%T a, %T b);

/** @brief returns non-zero if either argument is NaN */
int %Punord%I2(%T a, %T b);

/** @brief calculates `a == b`
 * Returns 0 if `a` and `b` are equal or if either argument is `NaN`.
 */
int %Peq%I2(%T a, %T b);

/** @brief calculates `a != b`
 * Returns non-zero if `a` and `b` are not equal or if either argument is `NaN`.
 */
int %Pne%I2(%T a, %T b);

/** @brief calculates `a >= b`
 * Returns a non-negative value if `a` is greater than
 * or equal to `b` and neither argument is `NaN`.
 */
int %Pge%I2(%T a, %T b);

/** @brief calculates `a < b`
 * Returns a negative value if `a` is less than `b`
 * and neither argument is `NaN`.
 */
int %Plt%I2(%T a, %T b);

/** @brief calculates `a <= b`
 * Returns a non-positive value if `a` is less than
 * or equal to `b` and neither argument is `NaN`.
 */
int %Ple%I2(%T a, %T b);

/** @brief calculates `a < b`
 * Returns a positive value if `a` is greater than `b`
 * and neither argument is `NaN`.
 */
int %Pgt%I2(%T a, %T b);