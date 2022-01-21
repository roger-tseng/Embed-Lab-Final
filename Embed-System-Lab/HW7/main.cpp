#include "mbed.h"

// main() runs in its own thread in the OS
/* ----------------------------------------------------------------------
 * Copyright (C) 2010-2012 ARM Limited. All rights reserved.
 *
* $Date:         17. January 2013
* $Revision:     V1.4.0
*
* Project:       CMSIS DSP Library
 * Title:        arm_fir_example_f32.c
 *
 * Description:  Example code demonstrating how an FIR filter can be used
 *               as a low pass filter.
 *
 * Target Processor: Cortex-M4/Cortex-M3
 *
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*   - Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   - Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*   - Neither the name of ARM LIMITED nor the names of its contributors
*     may be used to endorse or promote products derived from this
*     software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
 * -------------------------------------------------------------------- */
/* ----------------------------------------------------------------------
** Include Files
** ------------------------------------------------------------------- */
#include "arm_math.h"
#include "math_helper.h"

#include "stm32l475e_iot01_gyro.h"

#define SEMIHOSTING
#if defined(SEMIHOSTING)
#include <stdio.h>
#endif
/* ----------------------------------------------------------------------
** Macro Defines
** ------------------------------------------------------------------- */
#define GYRO_SAMPLES  160
/*
This SNR is a bit small. Need to understand why
this example is not giving better SNR ...
*/
#define SNR_THRESHOLD_F32    75.0f
#define BLOCK_SIZE            32
#if defined(ARM_MATH_MVEF) && !defined(ARM_MATH_AUTOVECTORIZE)
/* Must be a multiple of 16 */
#define NUM_TAPS_ARRAY_SIZE              32
#else
#define NUM_TAPS_ARRAY_SIZE              29
#endif
#define NUM_TAPS              29
/* -------------------------------------------------------------------
 * Declare Ref and Test output buffer
 * ------------------------------------------------------------------- */
static float32_t refOutput[GYRO_SAMPLES];
static float32_t testOutput[GYRO_SAMPLES];
/* -------------------------------------------------------------------
 * Declare State buffer of size (numTaps + blockSize - 1)
 * ------------------------------------------------------------------- */
#if defined(ARM_MATH_MVEF) && !defined(ARM_MATH_AUTOVECTORIZE)
static float32_t firStateF32[2 * BLOCK_SIZE + NUM_TAPS - 1];
#else
static float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1];
#endif 
/* ----------------------------------------------------------------------
** FIR Coefficients buffer generated using fir1() MATLAB function.
** fir1(28, 6/24)
** ------------------------------------------------------------------- */
#if defined(ARM_MATH_MVEF) && !defined(ARM_MATH_AUTOVECTORIZE)
const float32_t firCoeffs32[NUM_TAPS_ARRAY_SIZE] = {
  -0.0018225230f, -0.0015879294f, +0.0000000000f, +0.0036977508f, +0.0080754303f, +0.0085302217f, -0.0000000000f, -0.0173976984f,
  -0.0341458607f, -0.0333591565f, +0.0000000000f, +0.0676308395f, +0.1522061835f, +0.2229246956f, +0.2504960933f, +0.2229246956f,
  +0.1522061835f, +0.0676308395f, +0.0000000000f, -0.0333591565f, -0.0341458607f, -0.0173976984f, -0.0000000000f, +0.0085302217f,
  +0.0080754303f, +0.0036977508f, +0.0000000000f, -0.0015879294f, -0.0018225230f, 0.0f,0.0f,0.0f
};

#else
const float32_t firCoeffs32[NUM_TAPS_ARRAY_SIZE] = {
  -0.0018225230f, -0.0015879294f, +0.0000000000f, +0.0036977508f, +0.0080754303f, +0.0085302217f, -0.0000000000f, -0.0173976984f,
  -0.0341458607f, -0.0333591565f, +0.0000000000f, +0.0676308395f, +0.1522061835f, +0.2229246956f, +0.2504960933f, +0.2229246956f,
  +0.1522061835f, +0.0676308395f, +0.0000000000f, -0.0333591565f, -0.0341458607f, -0.0173976984f, -0.0000000000f, +0.0085302217f,
  +0.0080754303f, +0.0036977508f, +0.0000000000f, -0.0015879294f, -0.0018225230f
};
#endif
/* ------------------------------------------------------------------
 * Global variables for FIR LPF Example
 * ------------------------------------------------------------------- */
uint32_t blockSize = BLOCK_SIZE;
uint32_t numBlocks = GYRO_SAMPLES/BLOCK_SIZE;
float32_t  snr;
/* ----------------------------------------------------------------------
 * FIR LPF Example
 * ------------------------------------------------------------------- */
float from_sensor(float GyroDataX[], float GyroDataY[], float GyroDataZ[])
    {
        float pGyroDataXYZ[3] = {0};
        int sample_num = 0;
        printf("Start sensor init\n");
        BSP_GYRO_Init();

        while(sample_num < GYRO_SAMPLES) {
            printf("\nLoop %d\n", sample_num);
            ++sample_num;

            BSP_GYRO_GetXYZ(pGyroDataXYZ);
            //printf("\nGYRO_X = %.2f\n", pGyroDataXYZ[0]);
            //printf("GYRO_Y = %.2f\n", pGyroDataXYZ[1]);
            //printf("GYRO_Z = %.2f\n", pGyroDataXYZ[2]);

            GyroDataX[sample_num] = (float)((int)(pGyroDataXYZ[0]*10000))/10000;
            GyroDataY[sample_num] = (float)((int)(pGyroDataXYZ[1]*10000))/10000;
            GyroDataZ[sample_num] = (float)((int)(pGyroDataXYZ[2]*10000))/10000;

        }
        return *GyroDataX, *GyroDataY, *GyroDataZ;
    }

int32_t main(void)
{
  uint32_t i;
  arm_fir_instance_f32 S;
  arm_status status;
  float32_t  *inputF32, *outputF32;

  /* Initialize input and output buffer pointers */
  float GyroDataX[GYRO_SAMPLES];
  float GyroDataY[GYRO_SAMPLES];
  float GyroDataZ[GYRO_SAMPLES];
  float GyroOutputX[GYRO_SAMPLES];
  float sum = 0;

  /*Read gyroscope data from sensor */
  from_sensor(GyroDataX, GyroDataY, GyroDataZ);
  
  /* Compute average as reference signal*/
  for(i = 0; i < GYRO_SAMPLES; i++){
      sum += GyroDataX[i];
  }
  for(i = 0; i < GYRO_SAMPLES; i++){
      refOutput[i] = sum/GYRO_SAMPLES;
  }

  printf("%f %f %f\n", GyroDataX[19], GyroDataY[19], GyroDataZ[19]);
  inputF32 = &GyroDataX[0];
  outputF32 = &GyroOutputX[0];

  /* Call FIR init function to initialize the instance structure. */
  arm_fir_init_f32(&S, NUM_TAPS, (float32_t *)&firCoeffs32[0], &firStateF32[0], blockSize);

  /* ----------------------------------------------------------------------
  ** Call the FIR process function for every blockSize samples
  ** ------------------------------------------------------------------- */
  for(i=0; i < numBlocks; i++)
  {
    arm_fir_f32(&S, inputF32 + (i * blockSize), outputF32 + (i * blockSize), blockSize);
  }

  /* ----------------------------------------------------------------------
  ** We initially tried comparing the generated output against the reference output computed
  ** with arithmetic average, but later realized that gyroscope data is very noisy,
  ** Print generated output, then copy it to MATLAB to compare the reference output computed
  ** in MATLAB.
  ** ------------------------------------------------------------------- */
  snr = arm_snr_f32(&refOutput[0], &testOutput[0], TEST_LENGTH_SAMPLES);
  status = (snr < SNR_THRESHOLD_F32) ? ARM_MATH_TEST_FAILURE : ARM_MATH_SUCCESS;

  for (i=0; i< GYRO_SAMPLES; i++){
      printf("%f, ", GyroDataX[i]);
  }
  printf("\n\n\n\n\n\n\n\n\n");
  for (i=0; i< GYRO_SAMPLES; i++){
      printf("%f, ", GyroOutputX[i]);
  }

  if (status != ARM_MATH_SUCCESS)
  {
    #if defined (SEMIHOSTING)
        printf("FAILURE\n");
    #else
        while (1);                             /* main function does not return */
    #endif
  }
  else
  {
    #if defined (SEMIHOSTING)
        printf("SUCCESS\n");
    #else
        while (1);                             /* main function does not return */
    #endif
  }
}
