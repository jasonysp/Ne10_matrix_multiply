/*
 *  Copyright 2011-16 ARM Limited and Contributors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of ARM Limited nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY ARM LIMITED AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL ARM LIMITED AND CONTRIBUTORS BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "NE10.h"

#define MATRICES 3

static void initialise_matrix_column(ne10_mat_row3f *col);
static void initialise_matrix(ne10_mat3x3f_t *mat);
static void matrix_multiply_compute();
/**
 * @example NE10_sample_matrix_multiply.c
 * An example of using the matrix multiply functions.
 */
int matrix_multiply_sample_main(void)
{
    // matrix multiply compute 
    matrix_multiply_compute();

    return 0;
}

void initialise_matrix(ne10_mat3x3f_t *mat)
{
    initialise_matrix_column(&mat->c1);
    initialise_matrix_column(&mat->c2);
    initialise_matrix_column(&mat->c3);
}

void initialise_matrix_column(ne10_mat_row3f *col)
{
	col->r1 = (ne10_float32_t)rand() / RAND_MAX * 5.0f;
	col->r2 = (ne10_float32_t)rand() / RAND_MAX * 5.0f;
	col->r3 = (ne10_float32_t)rand() / RAND_MAX * 5.0f;
}

void matrix_multiply_compute()
{
	struct timeval time_start, time_stop, time_diff;
	float time_cost = 0;
	float **srcmat;
	float **mulmat;
	float **dstmat;
	float **neon_dstmat;
	int num = 1000;

	srcmat = (float **)malloc(num * sizeof(float*));
	mulmat = (float **)malloc(num * sizeof(float*));
	dstmat = (float **)malloc(num * sizeof(float*));
	neon_dstmat = (float **)malloc(num * sizeof(float*));
	for (int i = 0; i < num; i++)
	{
		srcmat[i] = (float *)malloc(num * sizeof(float));
		mulmat[i] = (float *)malloc(num * sizeof(float));
		dstmat[i] = (float *)malloc(num * sizeof(float));
		neon_dstmat[i] = (float *)malloc(num * sizeof(float));
	}

	for (int i = 0; i < num; i++)
	{
		for (int j = 0; j < num; j++)
		{
			srcmat[i][j] = 2.1;//(ne10_float32_t)rand() / RAND_MAX * 5.0f;;
			mulmat[i][j] = 2.1;//(ne10_float32_t)rand() / RAND_MAX * 5.0f;;
			dstmat[i][j] = 0;
			neon_dstmat[i][j] = 0;
		}
	}

	printf("begin simple matrix multiply\n"); 
	gettimeofday(&time_start, NULL);
	for (int i = 0; i < num; i++)
	{
		for (int j = 0; j < num; j++)
		{
			for (int k = 0; k < num; k++)
			{
				dstmat[i][j] += srcmat[i][k] * mulmat[k][j];
			}

			//printf("dstmat[%d][%d]: %f\n", i, j, dstmat[i][j]);
		}
	}
	gettimeofday(&time_stop, NULL);
	printf("end simple matrix multiply\n");
	timersub(&time_stop, &time_start, &time_diff);
	time_cost = time_diff.tv_sec * 1000000.0 + time_diff.tv_usec;
	printf("simple matrix multiply time cost %1.3f ms\n", time_cost / 1000.0);

	//////////////////////////////////////////////////////////////////////////////

	ne10_mat4x4f_t src[250]; 
	ne10_mat4x4f_t mul[250]; 
	ne10_mat4x4f_t dst[250];
	time_cost = 0; 

	if (ne10_init() != NE10_OK)
	{
		fprintf(stderr, "Failed to initialise Ne10.\n");
		return;
	}

	printf("begin neon matrix multiply\n");
	//gettimeofday(&time_start, NULL);
	for (int i = 0; i < num; i+=4)
	{
		for (int j = 0; j < num; j+=4)
		{
			for (int k = 0; k < num/4; k++)
			{
				src[k].c1.r1 = srcmat[i][k * 4];
				src[k].c1.r2 = srcmat[i + 1][k * 4];
				src[k].c1.r3 = srcmat[i + 2][k * 4];
				src[k].c1.r4 = srcmat[i + 3][k * 4];

				src[k].c2.r1 = srcmat[i][k * 4 + 1];
				src[k].c2.r2 = srcmat[i + 1][k * 4 + 1];
				src[k].c2.r3 = srcmat[i + 2][k * 4 + 1];
				src[k].c2.r4 = srcmat[i + 3][k * 4 + 1];

				src[k].c3.r1 = srcmat[i][k * 4 + 2];
				src[k].c3.r2 = srcmat[i + 1][k * 4 + 2];
				src[k].c3.r3 = srcmat[i + 2][k * 4 + 2];
				src[k].c3.r4 = srcmat[i + 3][k * 4 + 2];

				src[k].c4.r1 = srcmat[i][k * 4 + 3];
				src[k].c4.r2 = srcmat[i + 1][k * 4 + 3];
				src[k].c4.r3 = srcmat[i + 2][k * 4 + 3];
				src[k].c4.r4 = srcmat[i + 3][k * 4 + 3];

				mul[k].c1.r1 = mulmat[k * 4][j];
				mul[k].c1.r2 = mulmat[k * 4 + 1][j];
				mul[k].c1.r3 = mulmat[k * 4 + 2][j];
				mul[k].c1.r4 = mulmat[k * 4 + 3][j];

				mul[k].c2.r1 = mulmat[k * 4][j + 1]; 
				mul[k].c2.r2 = mulmat[k * 4 + 1][j + 1];
				mul[k].c2.r3 = mulmat[k * 4 + 2][j + 1];
				mul[k].c2.r4 = mulmat[k * 4 + 3][j + 1];

				mul[k].c3.r1 = mulmat[k * 4][j + 2];
				mul[k].c3.r2 = mulmat[k * 4 + 1][j + 2];
				mul[k].c3.r3 = mulmat[k * 4 + 2][j + 2];
				mul[k].c3.r4 = mulmat[k * 4 + 3][j + 2];

				mul[k].c4.r1 = mulmat[k * 4][j + 3];
				mul[k].c4.r2 = mulmat[k * 4 + 1][j + 3];
				mul[k].c4.r3 = mulmat[k * 4 + 2][j + 3];
				mul[k].c4.r4 = mulmat[k * 4 + 3][j + 3];
			}
			gettimeofday(&time_start, NULL); 
			ne10_mulmat_4x4f(dst, src, mul, num/4);

			for (int n = 0; n < num/4; n++)
			{
				neon_dstmat[i][j] += dst[n].c1.r1;  
				neon_dstmat[i+1][j] += dst[n].c1.r2;
				neon_dstmat[i+2][j] += dst[n].c1.r3;
				neon_dstmat[i+3][j] += dst[n].c1.r4;

				neon_dstmat[i][j+1] += dst[n].c2.r1;
				neon_dstmat[i+1][j+1] += dst[n].c2.r2;
				neon_dstmat[i+2][j+1] += dst[n].c2.r3;
				neon_dstmat[i+3][j+1] += dst[n].c2.r4;

				neon_dstmat[i][j+2] += dst[n].c3.r1;
				neon_dstmat[i+1][j+2] += dst[n].c3.r2;
				neon_dstmat[i+2][j+2] += dst[n].c3.r3;
				neon_dstmat[i+3][j+2] += dst[n].c3.r4;

				neon_dstmat[i][j+3] += dst[n].c4.r1;
				neon_dstmat[i+1][j+3] += dst[n].c4.r2;
				neon_dstmat[i+2][j+3] += dst[n].c4.r3;
				neon_dstmat[i+3][j+3] += dst[n].c4.r4;

				/*                printf("[ %5.2f %5.2f %5.2f %5.2f     [ %5.2f %5.2f %5.2f %5.2f     [ %5.2f %5.2f %5.2f %5.2f\n",
								  src[n].c1.r1, src[n].c2.r1, src[n].c3.r1, src[n].c4.r1,
								  mul[n].c1.r1, mul[n].c2.r1, mul[n].c3.r1, mul[n].c4.r1,
								  dst[n].c1.r1, dst[n].c2.r1, dst[n].c3.r1, dst[n].c4.r1);
								  printf("  %5.2f %5.2f %5.2f %5.2f  *    %5.2f %5.2f %5.2f %5.2f  =    %5.2f %5.2f %5.2f %5.2f\n",
								  src[n].c1.r2, src[n].c2.r2, src[n].c3.r2, src[n].c4.r2,
								  mul[n].c1.r2, mul[n].c2.r2, mul[n].c3.r2, mul[n].c4.r2,
								  dst[n].c1.r2, dst[n].c2.r2, dst[n].c3.r2, dst[n].c4.r2);
								  printf("[ %5.2f %5.2f %5.2f %5.2f     [ %5.2f %5.2f %5.2f %5.2f     [ %5.2f %5.2f %5.2f %5.2f\n",
								  src[n].c1.r3, src[n].c2.r3, src[n].c3.r3, src[n].c4.r3,
								  mul[n].c1.r3, mul[n].c2.r3, mul[n].c3.r3, mul[n].c4.r3,
								  dst[n].c1.r3, dst[n].c2.r3, dst[n].c3.r3, dst[n].c4.r3);
								  printf("[ %5.2f %5.2f %5.2f %5.2f     [ %5.2f %5.2f %5.2f %5.2f     [ %5.2f %5.2f %5.2f %5.2f\n",
								  src[n].c1.r4, src[n].c2.r4, src[n].c3.r4, src[n].c4.r4,
								  mul[n].c1.r4, mul[n].c2.r4, mul[n].c3.r4, mul[n].c4.r4,
								  dst[n].c1.r4, dst[n].c2.r4, dst[n].c3.r4, dst[n].c4.r4);

								  printf("\n");*/
			}

			//printf("neon_dstmat[%d][%d]: %f\n", i, j, neon_dstmat[i][j]);
			gettimeofday(&time_stop, NULL);
			timersub(&time_stop, &time_start, &time_diff);
			time_cost += time_diff.tv_sec * 1000000.0 + time_diff.tv_usec;

		}
	}
	//gettimeofday(&time_stop, NULL);
	printf("end neon matrix multiply\n");
	//timersub(&time_stop, &time_start, &time_diff);
	//time_cost = time_diff.tv_sec * 1000000.0 + time_diff.tv_usec;
	printf("neon matrix multiply time cost %1.3f ms\n", time_cost / 1000.0);
	
	for (int i = 0; i < num; i++)
	{
		free(srcmat[i]);
		free(mulmat[i]);
		free(dstmat[i]);
		free(neon_dstmat[i]);
	}

	free(srcmat);
	free(mulmat);
	free(dstmat);
	free(neon_dstmat);
}


