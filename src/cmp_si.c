/* mpfr_cmp_si_2exp -- compare a floating-point number with a signed
machine integer multiplied by a power of 2

Copyright 1999, 2001-2013 Free Software Foundation, Inc.
Contributed by the AriC and Caramel projects, INRIA.

This file is part of the GNU MPFR Library.

The GNU MPFR Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

The GNU MPFR Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the GNU MPFR Library; see the file COPYING.LESSER.  If not, see
http://www.gnu.org/licenses/ or write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA. */

#define MPFR_NEED_LONGLONG_H
#include "mpfr-impl.h"

/* returns a positive value if b > i*2^f,
           a negative value if b < i*2^f,
           zero if b = i*2^f.
   b must not be NaN.
*/

int
mpfr_cmp_si_2exp (mpfr_srcptr b, long int i, mpfr_exp_t f)
{
  int si;

  si = i < 0 ? -1 : 1; /* sign of i */
  if (MPFR_UNLIKELY (MPFR_IS_SINGULAR (b)))
    {
      if (MPFR_IS_INF(b))
        return MPFR_INT_SIGN(b);
      else if (MPFR_IS_ZERO(b))
        return i != 0 ? -si : 0;
      /* NAN */
      MPFR_SET_ERANGEFLAG ();
      return 0;
    }
  else if (MPFR_SIGN(b) != si || i == 0)
    return MPFR_INT_SIGN (b);
  else /* b and i are of same sign si */
    {
      mpfr_exp_t e;
      unsigned long ai;
      int k;
      mp_size_t bn;
      mp_limb_t c, *bp;

      ai = SAFE_ABS(unsigned long, i);

      /* ai must be representable in a mp_limb_t */
      MPFR_ASSERTN(ai == (mp_limb_t) ai);

      e = MPFR_GET_EXP (b); /* 2^(e-1) <= b < 2^e */
      if (e <= f)
        return -si;
      if (f < MPFR_EMAX_MAX - GMP_NUMB_BITS &&
          e > f + GMP_NUMB_BITS)
        return si;

      /* now f < e <= f + GMP_NUMB_BITS */
      c = (mp_limb_t) ai;
      count_leading_zeros(k, c);
      if ((int) (e - f) > GMP_NUMB_BITS - k)
        return si;
      if ((int) (e - f) < GMP_NUMB_BITS - k)
        return -si;

      /* now b and i*2^f have the same exponent */
      c <<= k;
      bn = (MPFR_PREC(b) - 1) / GMP_NUMB_BITS;
      bp = MPFR_MANT(b);
      if (bp[bn] > c)
        return si;
      if (bp[bn] < c)
        return -si;

      /* most significant limbs agree, check remaining limbs from b */
      while (bn > 0)
        if (bp[--bn])
          return si;
      return 0;
    }
}

#undef mpfr_cmp_si
int
mpfr_cmp_si (mpfr_srcptr b, long int i)
{
  return mpfr_cmp_si_2exp (b, i, 0);
}
