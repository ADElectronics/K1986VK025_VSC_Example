/* ----------------------------------------------------------------------
 * Project:      NMSIS DSP Library
 * Title:        riscv_conv_partial_fast_opt_q15.c
 * Description:  Fast Q15 Partial convolution
 *
 * $Date:        18. March 2019
 * $Revision:    V1.6.0
 *
 * Target Processor: RISC-V Cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2019 ARM Limited or its affiliates. All rights reserved.
 * Copyright (c) 2019 Nuclei Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "riscv_math.h"

/**
  @ingroup groupFilters
 */

/**
  @addtogroup PartialConv
  @{
 */

/**
  @brief         Partial convolution of Q15 sequences (fast version).
  @param[in]     pSrcA      points to the first input sequence
  @param[in]     srcALen    length of the first input sequence
  @param[in]     pSrcB      points to the second input sequence
  @param[in]     srcBLen    length of the second input sequence
  @param[out]    pDst       points to the location where the output result is written
  @param[in]     firstIndex is the first output sample to start with
  @param[in]     numPoints  is the number of output points to be computed
  @param[in]     pScratch1  points to scratch buffer of size max(srcALen, srcBLen) + 2*min(srcALen, srcBLen) - 2
  @param[in]     pScratch2  points to scratch buffer of size min(srcALen, srcBLen)
  @return        execution status
                   - \ref RISCV_MATH_SUCCESS        : Operation successful
                   - \ref RISCV_MATH_ARGUMENT_ERROR : requested subset is not in the range [0 srcALen+srcBLen-2]

  @remark
                   Refer to \ref riscv_conv_partial_q15() for a slower implementation of this function which uses a 64-bit accumulator to avoid wrap around distortion.
 */

riscv_status riscv_conv_partial_fast_opt_q15(
  const q15_t * pSrcA,
        uint32_t srcALen,
  const q15_t * pSrcB,
        uint32_t srcBLen,
        q15_t * pDst,
        uint32_t firstIndex,
        uint32_t numPoints,
        q15_t * pScratch1,
        q15_t * pScratch2)
{
        q15_t *pOut = pDst;                            /* Output pointer */
        q15_t *pScr1 = pScratch1;                      /* Temporary pointer for scratch1 */
        q15_t *pScr2 = pScratch2;                      /* Temporary pointer for scratch1 */
        q31_t acc0;                                    /* Accumulator */
  const q15_t *pIn1;                                   /* InputA pointer */
  const q15_t *pIn2;                                   /* InputB pointer */
  const q15_t *px;                                     /* Intermediate inputA pointer */
        q15_t *py;                                     /* Intermediate inputB pointer */
        uint32_t j, k, blkCnt;                         /* Loop counter */
        uint32_t tapCnt;                               /* Loop count */
        riscv_status status;                             /* Status variable */
        q31_t x1;                                      /* Temporary variables to hold state and coefficient values */
        q31_t y1;                                      /* State variables */

#if defined (RISCV_MATH_LOOPUNROLL)
        q31_t acc1, acc2, acc3;                        /* Accumulator */
        q31_t x2, x3;                                  /* Temporary variables to hold state and coefficient values */
        q31_t y2;                                      /* State variables */
#endif

  /* Check for range of output samples to be calculated */
  if ((firstIndex + numPoints) > ((srcALen + (srcBLen - 1U))))
  {
    /* Set status as RISCV_MATH_ARGUMENT_ERROR */
    status = RISCV_MATH_ARGUMENT_ERROR;
  }
  else
  {
    /* The algorithm implementation is based on the lengths of the inputs. */
    /* srcB is always made to slide across srcA. */
    /* So srcBLen is always considered as shorter or equal to srcALen */
    if (srcALen >= srcBLen)
    {
      /* Initialization of inputA pointer */
      pIn1 = pSrcA;

      /* Initialization of inputB pointer */
      pIn2 = pSrcB;
    }
    else
    {
      /* Initialization of inputA pointer */
      pIn1 = pSrcB;

      /* Initialization of inputB pointer */
      pIn2 = pSrcA;

      /* srcBLen is always considered as shorter or equal to srcALen */
      j = srcBLen;
      srcBLen = srcALen;
      srcALen = j;
    }

    /* Temporary pointer for scratch2 */
    py = pScratch2;

#if __RISCV_XLEN == 64
  /* Pointer to take end of scratch2 buffer */
  pScr2 = pScratch2 + srcBLen - 4;
#else
   /* Pointer to take end of scratch2 buffer */
    pScr2 = pScratch2 + srcBLen - 1;
#endif /* __RISCV_XLEN == 64 */

    /* points to smaller length sequence */
    px = pIn2;

#if defined (RISCV_MATH_LOOPUNROLL)

    /* Loop unrolling: Compute 4 outputs at a time */
    k = srcBLen >> 2U;

    /* Copy smaller length input sequence in reverse order into second scratch buffer */
    while (k > 0U)
    {
#if __RISCV_XLEN == 64
    write_q15x4_da(&pScr2,read_q15x4_ia((q15_t **)&px));
#else
      /* copy second buffer in reversal manner */
      *pScr2-- = *px++;
      *pScr2-- = *px++;
      *pScr2-- = *px++;
      *pScr2-- = *px++;
#endif /* __RISCV_XLEN == 64 */

      /* Decrement loop counter */
      k--;
    }
#if __RISCV_XLEN == 64
    pScr2 += 3;
#endif /* __RISCV_XLEN == 64 */
    /* Loop unrolling: Compute remaining outputs */
    k = srcBLen % 0x4U;

#else

    /* Initialize k with number of samples */
    k = srcBLen;

#endif /* #if defined (RISCV_MATH_LOOPUNROLL) */

    while (k > 0U)
    {
      /* copy second buffer in reversal manner for remaining samples */
      *pScr2-- = *px++;

      /* Decrement loop counter */
      k--;
    }

    /* Initialze temporary scratch pointer */
    pScr1 = pScratch1;

    /* Assuming scratch1 buffer is aligned by 32-bit */
    /* Fill (srcBLen - 1U) zeros in scratch buffer */
    riscv_fill_q15(0, pScr1, (srcBLen - 1U));

    /* Update temporary scratch pointer */
    pScr1 += (srcBLen - 1U);

    /* Copy bigger length sequence(srcALen) samples in scratch1 buffer */

    /* Copy (srcALen) samples in scratch buffer */
    riscv_copy_q15(pIn1, pScr1, srcALen);

    /* Update pointers */
    pScr1 += srcALen;

    /* Fill (srcBLen - 1U) zeros at end of scratch buffer */
    riscv_fill_q15(0, pScr1, (srcBLen - 1U));

    /* Update pointer */
    pScr1 += (srcBLen - 1U);

    /* Initialization of pIn2 pointer */
    pIn2 = py;

    pScratch1 += firstIndex;

    pOut = pDst + firstIndex;

    /* Actual convolution process starts here */

#if defined (RISCV_MATH_LOOPUNROLL)

    /* Loop unrolling: Compute 4 outputs at a time */
    blkCnt = (numPoints) >> 2;

    while (blkCnt > 0)
    {
      /* Initialze temporary scratch pointer as scratch1 */
      pScr1 = pScratch1;

      /* Clear Accumlators */
      acc0 = 0;
      acc1 = 0;
      acc2 = 0;
      acc3 = 0;

      /* Read two samples from scratch1 buffer */
      x1 = read_q15x2_ia (&pScr1);

      /* Read next two samples from scratch1 buffer */
      x2 = read_q15x2_ia (&pScr1);

      tapCnt = (srcBLen) >> 2U;

      while (tapCnt > 0U)
      {

        /* Read four samples from smaller buffer */
        y1 = read_q15x2_ia ((q15_t **) &pIn2);
        y2 = read_q15x2_ia ((q15_t **) &pIn2);

        /* multiply and accumlate */
        acc0 = __SMLAD(x1, y1, acc0);
        acc2 = __SMLAD(x2, y1, acc2);

        /* pack input data */
#ifndef RISCV_MATH_BIG_ENDIAN
        x3 = __PKHBT(x2, x1, 0);
#else
        x3 = __PKHBT(x1, x2, 0);
#endif

        /* multiply and accumlate */
        acc1 = __SMLADX(x3, y1, acc1);

        /* Read next two samples from scratch1 buffer */
        x1 = read_q15x2_ia (&pScr1);

        /* multiply and accumlate */
        acc0 = __SMLAD(x2, y2, acc0);
        acc2 = __SMLAD(x1, y2, acc2);

        /* pack input data */
#ifndef RISCV_MATH_BIG_ENDIAN
        x3 = __PKHBT(x1, x2, 0);
#else
        x3 = __PKHBT(x2, x1, 0);
#endif

        acc3 = __SMLADX(x3, y1, acc3);
        acc1 = __SMLADX(x3, y2, acc1);

        x2 = read_q15x2_ia (&pScr1);

#ifndef RISCV_MATH_BIG_ENDIAN
        x3 = __PKHBT(x2, x1, 0);
#else
        x3 = __PKHBT(x1, x2, 0);
#endif

        /* multiply and accumlate */
        acc3 = __SMLADX(x3, y2, acc3);

        /* Decrement loop counter */
        tapCnt--;
      }

      /* Update scratch pointer for remaining samples of smaller length sequence */
      pScr1 -= 4U;

      /* apply same above for remaining samples of smaller length sequence */
      tapCnt = (srcBLen) & 3U;

      while (tapCnt > 0U)
      {
        /* accumlate the results */
        acc0 += (*pScr1++ * *pIn2);
        acc1 += (*pScr1++ * *pIn2);
        acc2 += (*pScr1++ * *pIn2);
        acc3 += (*pScr1++ * *pIn2++);

        pScr1 -= 3U;

        /* Decrement loop counter */
        tapCnt--;
      }

      blkCnt--;

      /* Store the results in the accumulators in the destination buffer. */
#ifndef  RISCV_MATH_BIG_ENDIAN
      write_q15x2_ia (&pOut, __PKHBT(__SSAT((acc0 >> 15), 16), __SSAT((acc1 >> 15), 16), 16));
      write_q15x2_ia (&pOut, __PKHBT(__SSAT((acc2 >> 15), 16), __SSAT((acc3 >> 15), 16), 16));
#else
      write_q15x2_ia (&pOut, __PKHBT(__SSAT((acc1 >> 15), 16), __SSAT((acc0 >> 15), 16), 16));
      write_q15x2_ia (&pOut, __PKHBT(__SSAT((acc3 >> 15), 16), __SSAT((acc2 >> 15), 16), 16));
#endif /* #ifndef  RISCV_MATH_BIG_ENDIAN */

      /* Initialization of inputB pointer */
      pIn2 = py;

      pScratch1 += 4U;
    }

    /* Loop unrolling: Compute remaining outputs */
    blkCnt = numPoints & 0x3;

#else

    /* Initialize blkCnt with number of samples */
    blkCnt = numPoints;

#endif /* #if defined (RISCV_MATH_LOOPUNROLL) */

    /* Calculate convolution for remaining samples of Bigger length sequence */
    while (blkCnt > 0)
    {
      /* Initialze temporary scratch pointer as scratch1 */
      pScr1 = pScratch1;

      /* Clear Accumlators */
      acc0 = 0;

      tapCnt = (srcBLen) >> 1U;

      while (tapCnt > 0U)
      {
        /* Read next two samples from scratch1 buffer */
        x1 = read_q15x2_ia (&pScr1);

        /* Read two samples from smaller buffer */
        y1 = read_q15x2_ia ((q15_t **) &pIn2);

        /* multiply and accumlate */
        acc0 = __SMLAD(x1, y1, acc0);

        /* Decrement loop counter */
        tapCnt--;
      }

      tapCnt = (srcBLen) & 1U;

      /* apply same above for remaining samples of smaller length sequence */
      while (tapCnt > 0U)
      {
        /* accumlate the results */
        acc0 += (*pScr1++ * *pIn2++);

        /* Decrement loop counter */
        tapCnt--;
      }

      blkCnt--;

      /* The result is in 2.30 format.  Convert to 1.15 with saturation.
       ** Then store the output in the destination buffer. */
      *pOut++ = (q15_t) (__SSAT((acc0 >> 15), 16));

      /* Initialization of inputB pointer */
      pIn2 = py;

      pScratch1 += 1U;

    }

    /* Set status as RISCV_MATH_SUCCESS */
    status = RISCV_MATH_SUCCESS;
  }

  /* Return to application */
  return (status);
}

/**
  @} end of PartialConv group
 */
