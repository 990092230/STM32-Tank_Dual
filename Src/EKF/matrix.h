/*
The MIT License (MIT)

Copyright (c) 2015 - 2021

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _MATRIX_H_
#define _MATRIX_H_

#include "stm32f10x.h"
#include <math.h>


  /**
   * @brief 32-bit floating-point type definition.
   */
  typedef float float32_t;

  /**
   * @brief Instance structure for the floating-point matrix structure.
   */

  typedef struct
  {
    uint16_t numRows;     /**< number of rows of the matrix.     */
    uint16_t numCols;     /**< number of columns of the matrix.  */
    float32_t *pData;     /**< points to the data of the matrix. */
  } arm_matrix_instance_f32;
	
	
/**
 * @brief Error status returned by some functions in the library.
 */

typedef enum
{
	ARM_MATH_SUCCESS = 0,                /**< No error */
	ARM_MATH_ARGUMENT_ERROR = -1,        /**< One or more arguments are incorrect */
	ARM_MATH_LENGTH_ERROR = -2,          /**< Length of data buffer is incorrect */
	ARM_MATH_SIZE_MISMATCH = -3,         /**< Size of matrices is not compatible with the operation. */
	ARM_MATH_NANINF = -4,                /**< Not-a-number (NaN) or infinity is generated */
	ARM_MATH_SINGULAR = -5,              /**< Generated by matrix inversion if the input matrix is singular and cannot be inverted. */
	ARM_MATH_TEST_FAILURE = -6           /**< Test Failed  */
} arm_status;



/**
 * @brief  Floating-point square root function.
 * @param[in]  in     input value.
 * @param[out] *pOut  square root of input value.
 * @return The function returns ARM_MATH_SUCCESS if input value is positive value or ARM_MATH_ARGUMENT_ERROR if
 * <code>in</code> is negative value and returns zero output for negative values.
 */

static __INLINE arm_status arm_sqrt_f32(
float32_t in,
float32_t * pOut)
{
	if(in > 0)
	{

//      #if __FPU_USED
#if (__FPU_USED == 1) && defined ( __CC_ARM   )
		*pOut = __sqrtf(in);
#else
		*pOut = sqrtf(in);
#endif

		return (ARM_MATH_SUCCESS);
	}
	else
	{
		*pOut = 0.0f;
		return (ARM_MATH_ARGUMENT_ERROR);
	}

}
	


void arm_mat_zero_f32(arm_matrix_instance_f32* s);
arm_status mat_identity(float32_t *pData, uint16_t numRows, uint16_t numCols, float32_t value);
arm_status arm_mat_identity_f32(arm_matrix_instance_f32* s, float32_t value);
arm_status arm_mat_fill_f32(arm_matrix_instance_f32* s, float32_t *pData, uint32_t blockSize);
arm_status arm_mat_chol_f32(arm_matrix_instance_f32* s);
arm_status arm_mat_remainlower_f32(arm_matrix_instance_f32* s);

void arm_mat_getsubmatrix_f32(arm_matrix_instance_f32* s, arm_matrix_instance_f32 *a, int row, int col);
void arm_mat_setsubmatrix_f32(arm_matrix_instance_f32* a, arm_matrix_instance_f32 *s, int row, int col);

void arm_mat_getcolumn_f32(arm_matrix_instance_f32* s, float32_t *x, uint32_t col);
void arm_mat_setcolumn_f32(arm_matrix_instance_f32* s, float32_t *x, uint32_t col);
void arm_mat_cumsum_f32(arm_matrix_instance_f32* s, float32_t *tmp, float32_t *x);
int arm_mat_qr_decompositionT_f32(arm_matrix_instance_f32 *A, arm_matrix_instance_f32 *R);

#endif
