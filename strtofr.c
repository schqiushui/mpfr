/* mpfr_strtofr -- set a floating-point number from a string

Copyright 2004, 2005 Free Software Foundation, Inc.

This file is part of the MPFR Library.

The MPFR Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version.

The MPFR Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the MPFR Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Place, Fifth Floor, Boston,
MA 02110-1301, USA. */

#include <string.h> /* For strlen */
#include <stdlib.h> /* For strtol */
#include <ctype.h>  /* For isdigit */

#define MPFR_NEED_LONGLONG_H
#include "mpfr-impl.h"

#define MPFR_MAX_BASE 62

struct parsed_string {
  int            negative;
  int            base;
  unsigned char *mantissa, *mant;
  size_t         prec, alloc;
  mp_exp_t       exp_base, exp_bin;
};

/* This table has been generated by the following program */
static const unsigned long RedInvLog2Table[MPFR_MAX_BASE-1][2] = {
  {1UL, 1UL},
  {53UL, 84UL},
  {1UL, 2UL},
  {4004UL, 9297UL},
  {53UL, 137UL},
  {2393UL, 6718UL},
  {1UL, 3UL},
  {665UL, 2108UL},
  {4004UL, 13301UL},
  {949UL, 3283UL},
  {53UL, 190UL},
  {5231UL, 19357UL},
  {2393UL, 9111UL},
  {247UL, 965UL},
  {1UL, 4UL},
  {4036UL, 16497UL},
  {665UL, 2773UL},
  {5187UL, 22034UL},
  {4004UL, 17305UL},
  {51UL, 224UL},
  {949UL, 4232UL},
  {3077UL, 13919UL},
  {53UL, 243UL},
  {73UL, 339UL},
  {5231UL, 24588UL},
  {665UL, 3162UL},
  {2393UL, 11504UL},
  {4943UL, 24013UL},
  {247UL, 1212UL},
  {3515UL, 17414UL},
  {1UL, 5UL},
  {4415UL, 22271UL},
  {4036UL, 20533UL},
  {263UL, 1349UL},
  {665UL, 3438UL},
  {1079UL, 5621UL},
  {5187UL, 27221UL},
  {2288UL, 12093UL},
  {4004UL, 21309UL},
  {179UL, 959UL},
  {51UL, 275UL},
  {495UL, 2686UL},
  {949UL, 5181UL},
  {3621UL, 19886UL},
  {3077UL, 16996UL},
  {229UL, 1272UL},
  {53UL, 296UL},
  {109UL, 612UL},
  {73UL, 412UL},
  {1505UL, 8537UL},
  {5231UL, 29819UL},
  {283UL, 1621UL},
  {665UL, 3827UL},
  {32UL, 185UL},
  {2393UL, 13897UL},
  {1879UL, 10960UL},
  {4943UL, 28956UL},
  {409UL, 2406UL},
  {247UL, 1459UL},
  {231UL, 1370UL},
  {3515UL, 20929UL} };
#if 0
#include <stdio.h>
#include <limits.h>
#include "gmp.h"
#include "mpfr.h"
#define N 8
int main ()
{
  unsigned long tab[N];
  int i, n, base;
  mpfr_t x, y;
  mpq_t q1, q2;
  int overflow = 0, base_overflow;

  mpfr_init2 (x, 200);
  mpfr_init2 (y, 200);
  mpq_init (q1);
  mpq_init (q2);

  for (base = 2 ; base < 63 ; base ++)
    {
      mpfr_set_ui (x, base, GMP_RNDN);
      mpfr_log2 (x, x, GMP_RNDN);
      mpfr_ui_div (x, 1, x, GMP_RNDN);
      printf ("Base: %d x=%e ", base, mpfr_get_d1 (x));
      for (i = 0 ; i < N ; i++)
        {
          mpfr_floor (y, x);
          tab[i] = mpfr_get_ui (y, GMP_RNDN);
          mpfr_sub (x, x, y, GMP_RNDN);
          mpfr_ui_div (x, 1, x, GMP_RNDN);
        }
      for (i = N-1 ; i >= 0 ; i--)
        if (tab[i] != 0)
          break;
      mpq_set_ui (q1, tab[i], 1);
      for (i = i-1 ; i >= 0 ; i--)
          {
            mpq_inv (q1, q1);
            mpq_set_ui (q2, tab[i], 1);
            mpq_add (q1, q1, q2);
          }
      printf("Approx: ", base);
      mpq_out_str (stdout, 10, q1);
      printf (" = %e\n", mpq_get_d (q1) );
      fprintf (stderr, "{");
      mpz_out_str (stderr, 10, mpq_numref (q1));
      fprintf (stderr, "UL, ");
      mpz_out_str (stderr, 10, mpq_denref (q1));
      fprintf (stderr, "UL},\n");
      if (mpz_cmp_ui (mpq_numref (q1), 1<<16-1) >= 0
          || mpz_cmp_ui (mpq_denref (q1), 1<<16-1) >= 0)
        overflow = 1, base_overflow = base;
    }

  mpq_clear (q2);
  mpq_clear (q1);
  mpfr_clear (y);
  mpfr_clear (x);
  if (overflow )
    printf ("OVERFLOW for base =%d!\n", base_overflow);
}
#endif


/* Compatible with any locale, but one still assumes that 'a', 'b', 'c',
   ..., 'z', and 'A', 'B', 'C', ..., 'Z' are consecutive values (like
   in any ASCII-based character set). */
static int
digit_value_in_base (int c, int base)
{
  int digit;

  MPFR_ASSERTD (base > 0 && base <= MPFR_MAX_BASE);

  if (isdigit (c))
    digit = c - '0';
  else if (c >= 'a' && c <= 'z')
    digit = (base >= 37) ? c - 'a' + 36 : c - 'a' + 10;
  else if (c >= 'A' && c <= 'Z')
    digit = c - 'A' + 10;
  else
    return -1;

  return MPFR_LIKELY (digit < base) ? digit : -1;
}

/* Parse a string and fill pstr.
   Return the advanced ptr too.
   It returns:
      -1 if invalid string,
      0 if special string (like nan),
      1 if the string is ok.
      2 if overflows
   So it doesn't return the ternary value
   BUT if it returns 0 (NAN or INF), the ternary value is also '0'
   (ie NAN and INF are exact) */
static int
parse_string (mpfr_t x, struct parsed_string *pstr,
              const char **string, int base)
{
  const char *str = *string;
  unsigned char *mant;
  int point;
  int res = -1;  /* Invalid input return value */
  const char *prefix_str;

  /* Init variable */
  pstr->mantissa = NULL;

  /* Optional leading whitespace */
  while (isspace((unsigned char) *str)) str++;

  /* Can be case-insensitive NAN */
  if (strncasecmp (str, "@nan@", 5) == 0)
    {
      str += 5;
      goto set_nan;
    }
  if (base <= 16 && strncasecmp (str, "nan", 3) == 0)
    {
      str += 3;
    set_nan:
      /* Check for "(dummychars)" */
      if (*str == '(')
        {
          const char *s;
          for (s = str+1 ; *s != ')' ; s++)
            if (!(*s >= 'A' && *s <= 'Z')
                && !(*s >= 'a' && *s <= 'z')
                && !(*s >= '0' && *s <= '9')
                && *s != '_')
              break;
          if (*s == ')')
            str = s+1;
        }
      *string = str;
      MPFR_SET_NAN(x);
      /* MPFR_RET_NAN not used as the return value isn't a ternary value */
      __gmpfr_flags |= MPFR_FLAGS_NAN;
      return 0;
    }

  /* An optional `+' or `-' */
  pstr->negative = (*str == '-');
  if ((*str == '-') || (*str == '+'))
    str++;

  /* Can be case-insensitive INF */
  if (strncasecmp (str, "@inf@", 5) == 0)
    {
      str += 5;
      goto set_inf;
    }
  if (base <= 16 && strncasecmp (str, "infinity", 8) == 0)
    {
      str += 8;
      goto set_inf;
    }
  if (base <= 16 && strncasecmp (str, "inf", 3) == 0)
    {
      str += 3;
    set_inf:
      *string = str;
      MPFR_SET_INF (x);
      (pstr->negative) ? MPFR_SET_NEG (x) : MPFR_SET_POS (x);
      return 0;
    }

  /* If base=0 or 16, it may include '0x' prefix */
  prefix_str = NULL;
  if ((base == 0 || base == 16) && str[0]=='0'
      && (str[1]=='x' || str[1] == 'X'))
    {
      prefix_str = str;
      base = 16;
      str += 2;
    }
  /* If base=0 or 2, it may include '0b' prefix */
  if ((base == 0 || base == 2) && str[0]=='0'
      && (str[1]=='b' || str[1] == 'B'))
    {
      prefix_str = str;
      base = 2;
      str += 2;
    }
  /* Else if base=0, we assume decimal base */
  if (base == 0)
    base = 10;
  pstr->base = base;

  /* Alloc mantissa */
  pstr->alloc = (size_t) strlen (str) * sizeof(char) + 1;
  pstr->mantissa = (unsigned char*) (*__gmp_allocate_func) (pstr->alloc);

  /* Read mantissa digits */
 parse_begin:
  mant = pstr->mantissa;
  point = 0;
  pstr->exp_base = 0;
  pstr->exp_bin  = 0;

  for (;;) /* Loop until an invalid character is read */
    {
      int c = *str++;
      if (c == '.')
        {
          if (MPFR_UNLIKELY(point)) /* Second '.': stop parsing */
            break;
          point = 1;
          continue;
        }
      c = digit_value_in_base (c, base);
      if (c == -1)
        break;
      *mant++ = (char) c;
      if (!point)
        pstr->exp_base ++;
    }
  str--; /* The last read character was invalid */

  /* Update the # of char in the mantissa */
  pstr->prec = mant - pstr->mantissa;
  /* Check if there are no characters in the mantissa (Invalid argument) */
  if (pstr->prec == 0)
    {
      /* Check if there was a prefix (in such a case, we have to read
         again the mantissa without skipping the prefix)
         The allocated mantissa is still enought big since we will
         read only 0, and we alloc one more char than needed.
         FIXME: Not really friendly. Maybe cleaner code? */
      if (prefix_str != NULL)
        {
          str = prefix_str;
          prefix_str = NULL;
          goto parse_begin;
        }
      goto end;
    }

  /* Valid entry */
  res = 1;
  MPFR_ASSERTD (pstr->exp_base >= 0);

  /* an optional exponent (e or E, p or P, @) */
  if ( (*str == '@' || (base <= 10 && (*str == 'e' || *str == 'E')))
       && (!isspace((unsigned char) str[1])) )
    {
      char *endptr[1];
      /* the exponent digits are kept in ASCII */
      mp_exp_t read_exp = strtol (str + 1, endptr, 10);
      mp_exp_t sum = 0;
      if (endptr[0] != str+1)
        str = endptr[0];
      MPFR_ASSERTN (read_exp == (long) read_exp);
      MPFR_SADD_OVERFLOW (sum, read_exp, pstr->exp_base,
                          mp_exp_t, mp_exp_unsigned_t,
                          MPFR_EXP_MIN, MPFR_EXP_MAX,
                          res = 2, res = 3);
      /* Since exp_base was positive, read_exp + exp_base can't
         do a negative overflow. */
      MPFR_ASSERTD (res != 3);
      pstr->exp_base = sum;
    }
  else if ((base == 2 || base == 16)
           && (*str == 'p' || *str == 'P')
           && (!isspace((unsigned char) str[1])))
    {
      char *endptr[1];
      pstr->exp_bin = (mp_exp_t) strtol (str + 1, endptr, 10);
      if (endptr[0] != str+1)
        str = endptr[0];
    }

  /* Remove 0's at the beginning and end of mant_s[0..prec_s-1] */
  mant = pstr->mantissa;
  for ( ; (pstr->prec > 0) && (*mant == 0) ; mant++, pstr->prec--)
    pstr->exp_base--;
  for ( ; (pstr->prec > 0) && (mant[pstr->prec - 1] == 0); pstr->prec--);
  pstr->mant = mant;

  /* Check if x = 0 */
  if (pstr->prec == 0)
    {
      MPFR_SET_ZERO (x);
      if (pstr->negative)
        MPFR_SET_NEG(x);
      else
        MPFR_SET_POS(x);
      res = 0;
    }

  *string = str;
 end:
  if (pstr->mantissa != NULL && res != 1)
    (*__gmp_free_func) (pstr->mantissa, pstr->alloc);
  return res;
}

/* Transform a parsed string to a mpfr_t according to the rounding mode
   and the precision of x.
   Returns the ternary value. */
static int
parsed_string_to_mpfr (mpfr_t x, struct parsed_string *pstr, mp_rnd_t rnd)
{
  mp_prec_t prec;
  mp_exp_t  exp;
  mp_exp_t  ysize_bits;
  mp_limb_t *y, *result;
  int count, exact;
  size_t pstr_size;
  mp_size_t ysize, real_ysize;
  int res, err;
  MPFR_ZIV_DECL (loop);
  MPFR_TMP_DECL (marker);

  /* determine the minimal precision for the computation */
  prec = MPFR_PREC (x) + MPFR_INT_CEIL_LOG2 (MPFR_PREC (x));

  /* compute y as long as rounding is not possible */
  MPFR_TMP_MARK(marker);
  MPFR_ZIV_INIT (loop, prec);
  for (;;)
    {
      /* initialize y to the value of 0.mant_s[0]...mant_s[pr-1] */
      ysize = (prec - 1) / BITS_PER_MP_LIMB + 1;
      ysize_bits = ysize * BITS_PER_MP_LIMB;
      y = (mp_limb_t*) MPFR_TMP_ALLOC ((2 * ysize + 1) * sizeof (mp_limb_t));
      y += ysize;

      /* required precision for str to have ~ysize limbs
         We must have (2^(BITS_PER_MP_LIMB))^ysize ~= base^pstr_size
         aka pstr_size = ceil (ysize*BITS_PER_MP_LIMB/log2(base))
          ysize ~ prec/BITS_PER_MP_LIMB and prec < Umax/2 =>
          ysize*BITS_PER_MP_LIMB can not overflow.
         Compute pstr_size = ysize_bits * Num / Den
          Where Num/Den ~ 1/log2(base)
         It is not exactly ceil(1/log2(base)) but could be one more (base 2)
         Quite ugly since it tries to avoid overflow */
      pstr_size = ( ysize_bits / RedInvLog2Table[pstr->base-2][1]
                    * RedInvLog2Table[pstr->base-2][0] )
        + (( ysize_bits % RedInvLog2Table[pstr->base-2][1])
           * RedInvLog2Table[pstr->base-2][0]
           / RedInvLog2Table[pstr->base-2][1] )
        + 1;

      if (pstr_size >= pstr->prec)
        pstr_size = pstr->prec;
      MPFR_ASSERTD ((mp_exp_t) pstr_size == (mp_exp_t) pstr_size);

      /* convert str into binary */
      real_ysize = mpn_set_str (y, pstr->mant, pstr_size, pstr->base);
      MPFR_ASSERTD ( real_ysize <= ysize+1);

      /* normalize y: warning we can get even get ysize+1 limbs! */
      MPFR_ASSERTD (y[real_ysize - 1] != 0);
      count_leading_zeros (count, y[real_ysize - 1]);
      exact = (real_ysize <= ysize);
      if (exact != 0) /* shift y to the left in that case y shoud be exact */
        {
          /* shift {y, num_limb} for count bits to the left */
          if (count != 0)
            mpn_lshift (y, y, real_ysize, count);
          /* shift {y, num_limb} for (ysize-num_limb) limbs to the left */
          if (real_ysize != ysize)
            {
              MPN_COPY_DECR (y + ysize - real_ysize, y, real_ysize);
              MPN_ZERO (y, ysize - real_ysize);
            }
          /* for each bit shift decrease exponent of y */
          /* (This should not overflow) */
          exp = - ((ysize - real_ysize) * BITS_PER_MP_LIMB + count);
        }
      else  /* shift y for the right */
        {
          /* shift {y, num_limb} for (BITS_PER_MP_LIMB - count) bits
             to the right */
          mpn_rshift (y, y, real_ysize, BITS_PER_MP_LIMB - count);
          /* for each bit shift increase exponent of y */
          exp = BITS_PER_MP_LIMB - count;
        }

      /* compute base^(exp_s-pr) on n limbs */
      if (IS_POW2 (pstr->base))
        {
          /* Base: 2, 4, 8, 16, 32 */
          int pow2;
          mp_exp_t tmp;

          count_leading_zeros (pow2, (mp_limb_t) pstr->base);
          pow2 = BITS_PER_MP_LIMB - pow2 - 1; /* base = 2^pow2 */
          MPFR_ASSERTD (0 < pow2 && pow2 <= 5);
          /* exp += pow2 * (pstr->exp_base - pstr_size) + pstr->exp_bin
             with overflow checking
             and check that we can add/substract 2 to exp without overflow */
          MPFR_SADD_OVERFLOW (tmp, pstr->exp_base, -(mp_exp_t) pstr_size,
                              mp_exp_t, mp_exp_unsigned_t,
                              MPFR_EXP_MIN, MPFR_EXP_MAX,
                              goto overflow, goto underflow);
          /* On some FreeBsd/Alpha, LONG_MIN/1 produces an exception
             so we check for this before doing the division */
          if (tmp > 0 && pow2 != 1 && MPFR_EXP_MAX/pow2 <= tmp)
            goto overflow;
          else if (tmp < 0 && pow2 != 1 && MPFR_EXP_MIN/pow2 >= tmp)
            goto underflow;
          tmp *= pow2;
          MPFR_SADD_OVERFLOW (tmp, tmp, pstr->exp_bin,
                              mp_exp_t, mp_exp_unsigned_t,
                              MPFR_EXP_MIN, MPFR_EXP_MAX,
                              goto overflow, goto underflow);
          MPFR_SADD_OVERFLOW (exp, exp, tmp,
                              mp_exp_t, mp_exp_unsigned_t,
                              MPFR_EXP_MIN+2, MPFR_EXP_MAX-2,
                              goto overflow, goto underflow);
          result = y;
          err = 0;
        }
      /* case pstr->exp_base > pstr_size */
      else if (pstr->exp_base > (mp_exp_t) pstr_size)
        {
          mp_limb_t *z;
          mp_exp_t   exp_z;

          result = (mp_limb_t*) MPFR_TMP_ALLOC ((2*ysize+1)*BYTES_PER_MP_LIMB);

          /* z = base^(exp_base-sptr_size) using space allocated at y-ysize */
          z = y - ysize;
          /* NOTE: exp_base-pstr_size can't overflow since pstr_size > 0 */
          err = mpfr_mpn_exp (z, &exp_z, pstr->base,
                              pstr->exp_base - pstr_size, ysize);
          if (err == -2)
            goto overflow;
          exact = exact && (err == -1);

          /* multiply(y = 0.mant_s[0]...mant_s[pr-1])_base by base^(exp_s-g) */
          mpn_mul_n (result, y, z, ysize);

          /* compute the error on the product */
          if (err == -1)
            err = 0;
          err ++;

          /* compute the exponent of y */
          /* exp += exp_z + ysize_bits with overflow checking
             and check that we can add/substract 2 to exp without overflow */
          MPFR_SADD_OVERFLOW (exp_z, exp_z, ysize_bits,
                              mp_exp_t, mp_exp_unsigned_t,
                              MPFR_EXP_MIN, MPFR_EXP_MAX,
                              goto overflow, goto underflow);
          MPFR_SADD_OVERFLOW (exp, exp, exp_z,
                              mp_exp_t, mp_exp_unsigned_t,
                              MPFR_EXP_MIN+2, MPFR_EXP_MAX-2,
                              goto overflow, goto underflow);

          /* normalize result */
          if (MPFR_LIMB_MSB (result[2 * ysize - 1]) == 0)
            {
              mp_limb_t *r = result + ysize - 1;
              mpn_lshift (r, r, ysize + 1, 1);
              /* Overflow checking not needed */
              exp --;
            }

          exact = exact && (mpn_scan1 (result, 0)
                            >= (unsigned long) ysize_bits);
          result += ysize;
        }
      /* case exp_base < pstr_size */
      else if (pstr->exp_base < (mp_exp_t) pstr_size)
        {
          mp_limb_t *z;
          mp_exp_t exp_z;

          result = (mp_limb_t*) MPFR_TMP_ALLOC ( (3*ysize+1) * BYTES_PER_MP_LIMB);

          /* set y to y * K^ysize */
          y = y - ysize;  /* we have allocated ysize limbs at y - ysize */
          MPN_ZERO (y, ysize);

          /* pstr_size - pstr->exp_base can overflow */
          MPFR_SADD_OVERFLOW (exp_z, (mp_exp_t) pstr_size, -pstr->exp_base,
                              mp_exp_t, mp_exp_unsigned_t,
                              MPFR_EXP_MIN, MPFR_EXP_MAX,
                              goto underflow, goto overflow);

          /* (z, exp_z) = base^(exp_base-pstr_size) */
          z = result + 2*ysize + 1;
          err = mpfr_mpn_exp (z, &exp_z, pstr->base, exp_z, ysize);
          exact = exact && (err == -1);
          if (err == -2)
            goto underflow; /* FIXME: Sure? */
          if (err == -1)
            err = 0;

          /* compute y / z */
          /* result will be put into result + n, and remainder into result */
          mpn_tdiv_qr (result + ysize, result, (mp_size_t) 0, y,
                       2*ysize, z, ysize);

          /* exp -= exp_z + ysize_bits with overflow checking
             and check that we can add/substract 2 to exp without overflow */
          MPFR_SADD_OVERFLOW (exp_z, exp_z, ysize_bits,
                              mp_exp_t, mp_exp_unsigned_t,
                              MPFR_EXP_MIN, MPFR_EXP_MAX,
                              goto underflow, goto overflow);
          MPFR_SADD_OVERFLOW (exp, exp, -exp_z,
                              mp_exp_t, mp_exp_unsigned_t,
                              MPFR_EXP_MIN+2, MPFR_EXP_MAX-2,
                              goto overflow, goto underflow);
          err += 2;
          exact = exact && (mpn_popcount (result, ysize) == 0);

          /* normalize result */
          if (result[2 * ysize] == MPFR_LIMB_ONE)
            {
              mp_limb_t *r = result + ysize;
              exact = exact && ((*r & MPFR_LIMB_ONE) == 0);
              mpn_rshift (r, r, ysize + 1, 1);
              /* Overflow Checking not needed */
              exp ++;
            }
          result += ysize;
        }
      /* case exp_base = pstr_size */
      else
        {
          /* base^(exp_s-pr) = 1             nothing to compute */
          result = y;
          err = 0;
        }

      /* test if rounding is possible, and if so exit the loop */
      if (exact || mpfr_can_round_raw (result, ysize,
                                       (pstr->negative) ? -1 : 1,
                                       ysize_bits - err - 1,
                                       GMP_RNDN, rnd, MPFR_PREC(x)))
        break;

      /* update the prec for next loop */
      MPFR_ZIV_NEXT (loop, prec);
    } /* loop */
  MPFR_ZIV_FREE (loop);

  /* round y */
  if (mpfr_round_raw (MPFR_MANT (x), result,
                      ysize_bits,
                      pstr->negative, MPFR_PREC(x), rnd, &res ))
    {
      /* overflow when rounding y */
      MPFR_MANT (x)[MPFR_LIMB_SIZE (x) - 1] = MPFR_LIMB_HIGHBIT;
      /* Overflow Checking not needed */
      exp ++;
    }

  /* Set sign of x before exp since check_range needs a valid sign */
  (pstr->negative) ? MPFR_SET_NEG (x) : MPFR_SET_POS (x);

  /* DO NOT USE MPFR_SET_EXP. The exp may be out of range! */
  MPFR_SADD_OVERFLOW (exp, exp, ysize_bits,
                      mp_exp_t, mp_exp_unsigned_t,
                      MPFR_EXP_MIN, MPFR_EXP_MAX,
                      goto overflow, goto underflow);
  MPFR_EXP (x) = exp;
  res = mpfr_check_range (x, res, rnd);
  goto end;

 underflow:
  /* This is called when there is a huge overflow
     (Real expo < MPFR_EXP_MIN << __gmpfr_emin */
  if (rnd == GMP_RNDN)
    rnd = GMP_RNDZ;
  res = mpfr_underflow (x, rnd, (pstr->negative) ? -1 : 1);
  goto end;

 overflow:
  res = mpfr_overflow (x, rnd, (pstr->negative) ? -1 : 1);

 end:
  MPFR_TMP_FREE (marker);
  return res;
}

static void
free_parsed_string (struct parsed_string *pstr)
{
  (*__gmp_free_func) (pstr->mantissa, pstr->alloc);
}

int
mpfr_strtofr (mpfr_t x, const char *string, char **end, int base,
              mp_rnd_t rnd)
{
  int res = -1;
  struct parsed_string pstr;

  /* If an error occured, it must return 0 */
  MPFR_SET_ZERO (x);
  MPFR_SET_POS (x);

  if (base == 0 || (base >= 2 && base <= /*MPFR_MAX_BASE*/36))
    {
      res = parse_string (x, &pstr, &string, base);
      /* If res == 0, then it was exact (NAN or INF),
         so it is also the ternary value */
      if (res == 1)
        {
          res = parsed_string_to_mpfr (x, &pstr, rnd);
          free_parsed_string (&pstr);
        }
      else if (res == 2)
        res = mpfr_overflow (x, rnd, (pstr.negative) ? -1 : 1);
      MPFR_ASSERTD (res != 3);
#if 0
      else if (res == 3)
        {
          /* This is called when there is a huge overflow
             (Real expo < MPFR_EXP_MIN << __gmpfr_emin */
          if (rnd == GMP_RNDN)
            rnd = GMP_RNDZ;
          res = mpfr_underflow (x, rnd, (pstr.negative) ? -1 : 1);
        }
#endif
    }
  if (end != NULL)
    *end = (char *) string;
  return res;
}
