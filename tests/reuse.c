/* Test file for in-place operations.

Copyright (C) 2000, 2001 Free Software Foundation.

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
the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111-1307, USA. */

#include <stdio.h>
#include <stdlib.h>
#include "gmp.h"
#include "mpfr.h"
#include "mpfr-impl.h"

void (*testfunc) () = NULL;
void test3 (char *, mp_prec_t, mp_rnd_t);
void test4 (char *, mp_prec_t, mp_rnd_t);
void test3a (char *, mp_prec_t, mp_rnd_t);
void test2ui (char *, mp_prec_t, mp_rnd_t);
void testui2 (char *, mp_prec_t, mp_rnd_t);
void test2 (char *, mp_prec_t, mp_rnd_t);
void test2a (char *, mp_prec_t);
int mpfr_compare (mpfr_t, mpfr_t);

/* same than mpfr_cmp, but returns 0 for both NaN's */
int mpfr_compare (mpfr_t a, mpfr_t b)
{
  return (MPFR_IS_NAN(a)) ? !MPFR_IS_NAN(b) : 
    (MPFR_IS_NAN(b) || mpfr_cmp(a, b));
}

void test3 (char *foo, mp_prec_t prec, mp_rnd_t rnd)
{
  mpfr_t ref1, ref2, ref3;
  mpfr_t res1;
  int i;

#ifdef DEBUG
  printf("checking %s\n", foo);
#endif
  mpfr_init2 (ref1, prec);
  mpfr_init2 (ref2, prec);
  mpfr_init2 (ref3, prec);
  mpfr_init2 (res1, prec);

  /* for each variable, consider each of the following 6 possibilities:
     NaN, +Infinity, -Infinity, +0, -0 or a random number */
  for (i=0; i<36; i++) {
    if (i%6==0) MPFR_SET_NAN(ref2);
    if (i%6==1) mpfr_set_d (ref2, 1.0/0.0, GMP_RNDN);
    if (i%6==2) mpfr_set_d (ref2, -1.0/0.0, GMP_RNDN);
    if (i%6==3) mpfr_set_d (ref2, 0.0, GMP_RNDN);
    if (i%6==4) mpfr_set_d (ref2, -0.0, GMP_RNDN);
    if (i%6==5) mpfr_random (ref2);

    if (i/6==0) MPFR_SET_NAN(ref3);
    if (i/6==1) mpfr_set_d (ref3, 1.0/0.0, GMP_RNDN);
    if (i/6==2) mpfr_set_d (ref3, -1.0/0.0, GMP_RNDN);
    if (i/6==3) mpfr_set_d (ref3, 0.0, GMP_RNDN);
    if (i/6==4) mpfr_set_d (ref3, -0.0, GMP_RNDN);
    if (i/6==5) mpfr_random (ref3);

    /* reference call: foo(a, b, c) */
    testfunc (ref1, ref2, ref3, rnd);
 
    /* foo(a, a, c) */
    mpfr_set (res1, ref2, rnd); /* exact operation */
    testfunc (res1, res1, ref3, rnd);
 
    if (mpfr_compare (res1, ref1)) {
      fprintf (stderr, "Error for %s(a, a, c) for a=%e, c=%e\n", foo,
	       mpfr_get_d (ref2), mpfr_get_d (ref3));
      fprintf (stderr, "expected %e, got %e\n", mpfr_get_d (ref1),
	       mpfr_get_d (res1));
      exit (1);
    }

    /* foo(a, b, a) */
    mpfr_set (res1, ref3, rnd);
    testfunc (res1, ref2, res1, rnd);
    if (mpfr_compare (res1, ref1)) {
      fprintf (stderr, "Error for %s(a, b, a) for b=%e, a=%e\n", foo,
	       mpfr_get_d (ref2), mpfr_get_d (ref3));
      fprintf (stderr, "expected %e, got %e\n", mpfr_get_d (ref1),
	       mpfr_get_d (res1));
      exit (1);
    }

    /* foo(a, a, a) */
    mpfr_set (ref3, ref2, rnd);
    testfunc (ref1, ref2, ref3, rnd);
    mpfr_set (res1, ref2, rnd);
    testfunc (res1, res1, res1, rnd);

   if (mpfr_compare (res1, ref1)) {
      fprintf (stderr, "Error for %s(a, a, a) for a=%e\n", foo,
	       mpfr_get_d (ref2));
      fprintf (stderr, "expected %e, got %e\n", mpfr_get_d (ref1),
	       mpfr_get_d (res1));
      exit (1);
    }
  }

  mpfr_clear (ref1);
  mpfr_clear (ref2);
  mpfr_clear (ref3);
  mpfr_clear (res1);
}

void test4 (char *foo, mp_prec_t prec, mp_rnd_t rnd)
{
  mpfr_t ref, op1,op2,op3,cop1,cop2,cop3, top1,top2,top3;
  mpfr_t res;
  int i,j,k;

#ifdef DEBUG
  printf("checking %s\n", foo);
#endif
  mpfr_init2 (ref, prec);
  mpfr_init2 (op1, prec);
  mpfr_init2 (op2, prec);
  mpfr_init2 (op3, prec);
   mpfr_init2 (cop1, prec);
  mpfr_init2 (cop2, prec);
  mpfr_init2 (cop3, prec);
  mpfr_init2 (top1, prec);
  mpfr_init2 (top2, prec);
  mpfr_init2 (top3, prec);
  mpfr_init2 (res, prec);

  /* for each variable, consider each of the following 6 possibilities:
     NaN, +Infinity, -Infinity, +0, -0 or a random number */

  for (i=0; i<6; i++) 
    {

      if (i==0) MPFR_SET_NAN(op1);
      if (i==1) mpfr_set_d (op1, 1.0/0.0, GMP_RNDN);
      if (i==2) mpfr_set_d (op1, -1.0/0.0, GMP_RNDN);
      if (i==3) mpfr_set_d (op1, 0.0, GMP_RNDN);
      if (i==4) mpfr_set_d (op1, -0.0, GMP_RNDN);
      if (i==5) mpfr_random (op1);
      
      for (j=0; j<6; j++) 
        {

          if (j==0) MPFR_SET_NAN(op2);
          if (j==1) mpfr_set_d (op2, 1.0/0.0, GMP_RNDN);
          if (j==2) mpfr_set_d (op2, -1.0/0.0, GMP_RNDN);
          if (j==3) mpfr_set_d (op2, 0.0, GMP_RNDN);
          if (j==4) mpfr_set_d (op2, -0.0, GMP_RNDN);
          if (j==5) mpfr_random (op2);

          for (k=0; k<6; k++) 
            {

              if (k==0) MPFR_SET_NAN(op3);
              if (k==1) mpfr_set_d (op3, 1.0/0.0, GMP_RNDN);
              if (k==2) mpfr_set_d (op3, -1.0/0.0, GMP_RNDN);
              if (k==3) mpfr_set_d (op3, 0.0, GMP_RNDN);
              if (k==4) mpfr_set_d (op3, -0.0, GMP_RNDN);
              if (k==5) mpfr_random (op3);

              mpfr_set (top1, op1, rnd);
              mpfr_set (top2, op2, rnd);
              mpfr_set (top3, op3, rnd);

              /* reference call: foo(s, a, b, c) */
              testfunc (ref, op1, op2, op3, rnd);
 
              /* foo(a, a, b, c) */
              mpfr_set (res, op1, rnd); /* exact operation */
              testfunc (res, res, op2, op3, rnd);
 
              if (mpfr_compare (res, ref)) 
              {
                fprintf (stderr, 
                         "Error for %s(a, a, b, c) for a=%e, b=%e, c=%e\n", 
                         foo,
                         mpfr_get_d (op1), mpfr_get_d (op2), mpfr_get_d (op3));
                fprintf (stderr, "expected %e, got %e\n", mpfr_get_d (ref),
                         mpfr_get_d (res));
                exit (1);
              }

              /* foo(b, a, b, c) */
              mpfr_set (res, op2, rnd);
              testfunc (res, op1, res, op3, rnd);

             if (mpfr_compare (res, ref)) 
              {
                fprintf (stderr, 
                         "Error for %s(b, a, b, c) for a=%e, b=%e, c=%e\n", 
                         foo,
                         mpfr_get_d (op1), mpfr_get_d (op2), mpfr_get_d (op3));
                fprintf (stderr, "expected %e, got %e\n", mpfr_get_d (ref),
                         mpfr_get_d (res));
                exit (1);
              }

              /* foo(c, a, b, c) */
              mpfr_set (res, op3, rnd);
              testfunc (res, op1, op2, res, rnd);

             if (mpfr_compare (res, ref)) 
              {
                fprintf (stderr, 
                         "Error for %s(c, a, b, c) for a=%e, b=%e, c=%e\n",
                         foo,
                         mpfr_get_d (op1), mpfr_get_d (op2), mpfr_get_d (op3));
                fprintf (stderr, "expected %e, got %e\n", mpfr_get_d (ref),
                         mpfr_get_d (res));
                exit (1);
              }

              /* foo(a, a, a,c) */
             mpfr_set (op2, op1, rnd);
             testfunc (ref, op1, op2, op3, rnd);
             mpfr_set (res, op1, rnd);
             testfunc (res, res, res, op3, rnd);
             if (mpfr_compare (res, ref)) 
              {
                fprintf (stderr, 
                         "Error for %s(a, a, a, c) for a=%e, a=%e, c=%e\n",
                         foo,
                         mpfr_get_d (op1), mpfr_get_d (op2), mpfr_get_d (op3));
                fprintf (stderr, "expected %e, got %e\n", mpfr_get_d (ref),
                         mpfr_get_d (res));
                exit (1);
              }

              mpfr_set (op1, top1, rnd);
              mpfr_set (op2, top2, rnd);
              mpfr_set (op3, top3, rnd);

              /* foo(a, a, b,a) */
              mpfr_set (op3, op1, rnd);
              testfunc (ref, op1, op2, op3, rnd);
              mpfr_set (res, op1, rnd);
              testfunc (res, res, op2, res, rnd);
              if (mpfr_compare (res, ref)) 
               {
                 fprintf (stderr, 
                          "Error for %s(a, a, b, a) for a=%e, a=%e, c=%e\n",
                          foo,
                       mpfr_get_d (op1), mpfr_get_d (op2), mpfr_get_d (op3));
                 fprintf (stderr, "expected %e, got %e\n", mpfr_get_d (ref),
                          mpfr_get_d (res));
                 exit (1);
               }

              mpfr_set (op1, top1, rnd);
              mpfr_set (op2, top2, rnd);
              mpfr_set (op3, top3, rnd);

              /* foo(b, a, b, b) */
              mpfr_set (op3, op2, rnd);
              testfunc (ref, op1, op2, op3, rnd);
              mpfr_set (res, op2, rnd);
              testfunc (res, op1, res, res, rnd);
              if (mpfr_compare (res, ref)) 
               {
                 fprintf (stderr, 
                          "Error for %s(b, a, b, b) for a=%e, a=%e, c=%e\n", 
                          foo,
                        mpfr_get_d (op1), mpfr_get_d (op2), mpfr_get_d (op3));
                 fprintf (stderr, "expected %e, got %e\n", mpfr_get_d (ref),
                          mpfr_get_d (res));
                 exit (1);
               }
                              
              MPFR_CLEAR_FLAGS(op1);
              MPFR_CLEAR_FLAGS(op2);
              MPFR_CLEAR_FLAGS(op3);
              
              mpfr_set (cop1, top1, rnd);
              mpfr_set (cop2, top2, rnd);
              mpfr_set (cop3, top3, rnd);
             
              mpfr_set (cop2, cop1, rnd);
              mpfr_set (cop3, cop1, rnd);
              testfunc (ref, cop1, cop2, cop3 ,rnd);
              mpfr_set (res, cop1, rnd);
              testfunc (res, res, res, res, rnd);
              if (mpfr_compare (res, ref)) 
               {
                 fprintf (stderr,
                          "Error for %s(a, a, a, a) for a=%e, a=%e, a=%e\n",
                          foo,
                       mpfr_get_d (cop1), mpfr_get_d (cop2), mpfr_get_d (cop3));
                 fprintf (stderr, "expected %e, got %e\n", mpfr_get_d (ref),
                          mpfr_get_d (res));
                 exit (1);
               }
               /*
              mpfr_set (op1, top1, rnd);
              mpfr_set (op2, top2, rnd);
              mpfr_set (op3, top3, rnd);
             
              mpfr_set (op2, op1, rnd);
              mpfr_set (op3, op1, rnd);
              testfunc (ref, op1, op2, op3 ,rnd);
              mpfr_set (res, op1, rnd);
              testfunc (res, res, res, res, rnd);
              if (mpfr_compare (res, ref)) 
               {
                 fprintf (stderr,
                          "Error for %s(a, a, a, a) for a=%e, a=%e, a=%e\n",
                          foo,
                       mpfr_get_d (op1), mpfr_get_d (op2), mpfr_get_d (op3));
                 fprintf (stderr, "expected %e, got %e\n", mpfr_get_d (ref),
                          mpfr_get_d (res));
                 exit (1);
               }
              */
            }
        }
    }

  mpfr_clear (ref);
  mpfr_clear (op1);
  mpfr_clear (op2);
  mpfr_clear (op3);
   mpfr_clear (cop1);
  mpfr_clear (cop2);
  mpfr_clear (cop3);
  mpfr_clear (top1);
  mpfr_clear (top2);
  mpfr_clear (top3);
  mpfr_clear (res);

}

void test2ui (char *foo, mp_prec_t prec, mp_rnd_t rnd)
{
  mpfr_t ref1, ref2;
  unsigned int ref3;
  mp_limb_t c[1];
  mpfr_t res1;
  int i;

#ifdef DEBUG  
  printf("checking %s\n", foo);
#endif
  mpfr_init2 (ref1, prec);
  mpfr_init2 (ref2, prec);
  mpfr_init2 (res1, prec);



  /* ref2 can be NaN, +Inf, -Inf, +0, -0 or any number
     ref3 can be 0 or any number */
  for (i=0; i<12; i++)
    {
      if (i%6==0) MPFR_SET_NAN(ref2);
      if (i%6==1) mpfr_set_d (ref2, 1.0/0.0, GMP_RNDN);
      if (i%6==2) mpfr_set_d (ref2, -1.0/0.0, GMP_RNDN);
      if (i%6==3) mpfr_set_d (ref2, 0.0, GMP_RNDN);
      if (i%6==4) mpfr_set_d (ref2, -0.0, GMP_RNDN);
      if (i%6==5) mpfr_random (ref2);

      if (i/6==0) 
	ref3=0;
      else 
	{
	  mpn_random (c, 1);
	  ref3 = (unsigned int) c[0];
	}

      /* reference call: foo(a, b, c) */
      testfunc (ref1, ref2, ref3, rnd);

      /* foo(a, a, c) */
      mpfr_set (res1, ref2, rnd); /* exact operation */
      testfunc (res1, res1, ref3, rnd);

      if (mpfr_compare (res1, ref1))
	{
	  fprintf (stderr, "Error for %s(a, a, c) for a=%e c=%u\n", foo,
	       mpfr_get_d (ref2), ref3);
	  fprintf (stderr, "expected %e, got %e\n", mpfr_get_d (ref1),
	       mpfr_get_d (res1));
	  exit (1);
	}
    }

  mpfr_clear (ref1);
  mpfr_clear (ref2);
  mpfr_clear (res1);
}

void testui2 (char *foo, mp_prec_t prec, mp_rnd_t rnd)
{
  mpfr_t ref1, ref3;
  unsigned int ref2;
  mp_limb_t c[1];
  mpfr_t res1;
  int i;

#ifdef DEBUG  
  printf("checking %s\n", foo);
#endif
  mpfr_init2 (ref1, prec);
  mpfr_init2 (ref3, prec);
  mpfr_init2 (res1, prec);
  mpfr_random (ref3);
  mpn_random (c, 1);
  ref2 = (unsigned int) c[0];

  for (i=0; i<12; i++) {
    if (i%6==0) MPFR_SET_NAN(ref3);
    if (i%6==1) mpfr_set_d (ref3, 1.0/0.0, GMP_RNDN);
    if (i%6==2) mpfr_set_d (ref3, -1.0/0.0, GMP_RNDN);
    if (i%6==3) mpfr_set_d (ref3, 0.0, GMP_RNDN);
    if (i%6==4) mpfr_set_d (ref3, -0.0, GMP_RNDN);
    if (i%6==5) mpfr_random (ref3);

    if (i/6==0) ref2=0;
    else {
      mpn_random (c, 1);
      ref2 = (unsigned int) c[0];
    }

    /* reference call: foo(a, b, c) */
    testfunc (ref1, ref2, ref3, rnd);

    /* foo(a, b, a) */
    mpfr_set (res1, ref3, rnd); /* exact operation */
    testfunc (res1, ref2, res1, rnd);
    if (mpfr_compare (res1, ref1)) {
      fprintf (stderr, "Error for %s(a, b, a) for b=%u a=%e\n", foo,
	       ref2, mpfr_get_d (ref3));
      fprintf (stderr, "expected %e, got %e\n", mpfr_get_d (ref1),
	       mpfr_get_d (res1));
      exit (1);
    }
  }

  mpfr_clear (ref1);
  mpfr_clear (ref3);
  mpfr_clear (res1);
}

/* foo(mpfr_ptr, mpfr_srcptr, mp_rndt) */
void test2 (char *foo, mp_prec_t prec, mp_rnd_t rnd)
{
  mpfr_t ref1, ref2;
  mpfr_t res1;
  int i;

#ifdef DEBUG  
  printf("checking %s\n", foo);
#endif
  mpfr_init2 (ref1, prec);
  mpfr_init2 (ref2, prec);
  mpfr_init2 (res1, prec);
  mpfr_random (ref2);

  for (i=0; i<6; i++) {
    if (i==0) MPFR_SET_NAN(ref2);
    if (i==1) mpfr_set_d (ref2, 1.0/0.0, GMP_RNDN);
    if (i==2) mpfr_set_d (ref2, -1.0/0.0, GMP_RNDN);
    if (i==3) mpfr_set_d (ref2, 0.0, GMP_RNDN);
    if (i==4) mpfr_set_d (ref2, -0.0, GMP_RNDN);
    if (i==5) mpfr_random (ref2);

    /* reference call: foo(a, b) */
    testfunc (ref1, ref2, rnd);

    /* foo(a, a) */
    mpfr_set (res1, ref2, rnd); /* exact operation */
    testfunc (res1, res1, rnd);
    if (mpfr_compare (res1, ref1)) {
      fprintf (stderr, "Error for %s(a, a) for a=%e\n", foo, mpfr_get_d (ref2));
      fprintf (stderr, "expected %e, got %e\n", mpfr_get_d (ref1),
	       mpfr_get_d (res1));
      exit (1);
    }
  }

  mpfr_clear (ref1);
  mpfr_clear (ref2);
  mpfr_clear (res1);
}

/* foo(mpfr_ptr, mpfr_srcptr) */
void test2a (char *foo, mp_prec_t prec)
{
  mpfr_t ref1, ref2;
  mpfr_t res1;
  int i;

#ifdef DEBUG  
  printf("checking %s\n", foo);
#endif
  mpfr_init2 (ref1, prec);
  mpfr_init2 (ref2, prec);
  mpfr_init2 (res1, prec);
  mpfr_random (ref2);

  for (i=0; i<6; i++) {
    if (i==0) MPFR_SET_NAN(ref2);
    if (i==1) mpfr_set_d (ref2, 1.0/0.0, GMP_RNDN);
    if (i==2) mpfr_set_d (ref2, -1.0/0.0, GMP_RNDN);
    if (i==3) mpfr_set_d (ref2, 0.0, GMP_RNDN);
    if (i==4) mpfr_set_d (ref2, -0.0, GMP_RNDN);
    if (i==5) mpfr_random (ref2);

    /* reference call: foo(a, b) */
    testfunc (ref1, ref2);

    /* foo(a, a) */
    mpfr_set (res1, ref2, GMP_RNDN); /* exact operation */
    testfunc (res1, res1);
    if (mpfr_compare (res1, ref1)) {
      fprintf (stderr, "Error for %s(a, a) for a=%e\n", foo, mpfr_get_d (ref2));
      fprintf (stderr, "expected %e, got %e\n", mpfr_get_d (ref1),
	       mpfr_get_d (res1));
      exit (1);
    }
  }

  mpfr_clear (ref1);
  mpfr_clear (ref2);
  mpfr_clear (res1);
}

/* one operand, two results */
void test3a (char *foo, mp_prec_t prec, mp_rnd_t rnd)
{
  mpfr_t ref1, ref2, ref3;
  mpfr_t res1, res2;
  int i;

#ifdef DEBUG  
  printf("checking %s\n", foo);
#endif
  mpfr_init2 (ref1, prec);
  mpfr_init2 (ref2, prec);
  mpfr_init2 (ref3, prec);
  mpfr_init2 (res1, prec);
  mpfr_init2 (res2, prec);
  mpfr_random (ref3);

  for (i=0; i<6; i++) {
    if (i==0) MPFR_SET_NAN(ref3);
    if (i==1) mpfr_set_d (ref3, 1.0/0.0, GMP_RNDN);
    if (i==2) mpfr_set_d (ref3, -1.0/0.0, GMP_RNDN);
    if (i==3) mpfr_set_d (ref3, 0.0, GMP_RNDN);
    if (i==4) mpfr_set_d (ref3, -0.0, GMP_RNDN);
    if (i==5) mpfr_random (ref3);

     /* reference call: foo(a, b, c) */
     testfunc (ref1, ref2, ref3, rnd);

     /* foo(a, b, a) */
     mpfr_set (res1, ref3, rnd); /* exact operation */
     testfunc (res1, res2, res1, rnd);
     if (mpfr_compare (res1, ref1) || mpfr_compare (res2, ref2)) {
       fprintf (stderr, "Error for %s(a, b, a) for a=%e\n", foo, mpfr_get_d (ref3));
       fprintf (stderr, "expected (%e,%e), got (%e,%e)\n", mpfr_get_d (ref1),
		mpfr_get_d (ref2), mpfr_get_d (res1), mpfr_get_d (res2));
       exit (1);
     }

     /* foo(a, b, b) */
     mpfr_set (res2, ref3, rnd); /* exact operation */
     testfunc (res1, res2, res2, rnd);
     if (mpfr_compare (res1, ref1) || mpfr_compare (res2, ref2)) {
       fprintf (stderr, "Error for %s(a, b, b) for b=%e\n", foo, mpfr_get_d (ref3));
       fprintf (stderr, "expected (%e,%e), got (%e,%e)\n", mpfr_get_d (ref1),
		mpfr_get_d (ref2), mpfr_get_d (res1), mpfr_get_d (res2));
       exit (1);
     }
  }

  mpfr_clear (ref1);
  mpfr_clear (ref2);
  mpfr_clear (ref3);
  mpfr_clear (res1);
  mpfr_clear (res2);
}

int
main (void)
{
  testfunc = (void*) mpfr_add; test3 ("mpfr_add", 53, GMP_RNDN);
  testfunc = (void*) mpfr_add_ui; test2ui ("mpfr_add_ui", 53, GMP_RNDN);
  testfunc = mpfr_agm; test3 ("mpfr_agm", 53, GMP_RNDN);
  testfunc = mpfr_ceil; test2 ("mpfr_ceil", 53, GMP_RNDN);
  testfunc = (void*) mpfr_div; test3 ("mpfr_div", 53, GMP_RNDN);
  testfunc = (void*) mpfr_div_2exp; test2ui ("mpfr_div_2exp", 53, GMP_RNDN);
  testfunc = (void*) mpfr_div_ui; test2ui ("mpfr_div_ui", 53, GMP_RNDN);
  testfunc = (void*) mpfr_exp; test2 ("mpfr_exp", 53, GMP_RNDN);
  testfunc = mpfr_floor; test2 ("mpfr_floor", 53, GMP_RNDN);
  testfunc = (void*) mpfr_log; test2 ("mpfr_log", 53, GMP_RNDN);
  testfunc = (void*) mpfr_mul; test3 ("mpfr_mul", 53, GMP_RNDN);
  testfunc = (void*) mpfr_mul_2exp; test2ui ("mpfr_mul_2exp", 53, GMP_RNDN);
  testfunc = (void*) mpfr_mul_ui; test2ui ("mpfr_mul_ui", 53, GMP_RNDN);
  testfunc = (void*) mpfr_neg; test2 ("mpfr_neg", 53, GMP_RNDN);
  testfunc = (void*) mpfr_pow_ui; test2ui ("mpfr_pow_ui", 53, GMP_RNDN);
  testfunc = mpfr_reldiff; test3 ("mpfr_reldiff", 53, GMP_RNDN);
  testfunc = (void*) mpfr_sub; test3 ("mpfr_sub", 53, GMP_RNDN);
  testfunc = (void*) mpfr_sub_ui; test2ui ("mpfr_sub_ui", 53, GMP_RNDN);
  testfunc = (void*) mpfr_sqrt; test2 ("mpfr_sqrt", 53, GMP_RNDN);
  testfunc = (void*) mpfr_ui_div; testui2 ("mpfr_ui_div", 53, GMP_RNDN);
  testfunc = (void*) mpfr_ui_sub; testui2 ("mpfr_ui_sub", 53, GMP_RNDN);
  testfunc = mpfr_trunc; test2 ("mpfr_trunc", 53, GMP_RNDN);
  testfunc = (void*) mpfr_asin; test2 ("mpfr_asin", 53, GMP_RNDN);
  testfunc = (void*) mpfr_atan; test2 ("mpfr_atan", 53, GMP_RNDN);
  testfunc = (void*) mpfr_sinh; test2 ("mpfr_sinh", 53, GMP_RNDN);
  testfunc = (void*) mpfr_cosh; test2 ("mpfr_cosh", 53, GMP_RNDN);
  testfunc = (void*) mpfr_tanh; test2 ("mpfr_tanh", 53, GMP_RNDN);
  testfunc = (void*) mpfr_asinh; test2 ("mpfr_asinh", 53, GMP_RNDN);
  testfunc = (void*) mpfr_acosh; test2 ("mpfr_acosh", 53, GMP_RNDN);
  testfunc = (void*) mpfr_atanh; test2 ("mpfr_atanh", 53, GMP_RNDN);
  testfunc = (void*) mpfr_exp2; test2 ("mpfr_exp2", 53, GMP_RNDN);
  testfunc = (void*) mpfr_cos; test2 ("mpfr_cos", 53, GMP_RNDN);
  testfunc = (void*) mpfr_sin; test2 ("mpfr_sin", 53, GMP_RNDN);
  testfunc = (void*) mpfr_tan; test2 ("mpfr_tan", 53, GMP_RNDN);
  testfunc = (void*) mpfr_log10; test2 ("mpfr_log10", 53, GMP_RNDN);
  testfunc = (void*) mpfr_log2; test2 ("mpfr_log2", 53, GMP_RNDN);
  testfunc = (void*) mpfr_pow; test3 ("mpfr_pow", 53, GMP_RNDN);
  testfunc = (void*) mpfr_fma; test4 ("mpfr_fma", 53, GMP_RNDN);
  return 0;
}

