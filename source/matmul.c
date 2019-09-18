// MIT License
//
// Copyright (c) 2019 NERSC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "instrumentation.h"

#if defined(USE_INST)
#    include "user/instrumentation.h"
#else
#    include "fallback/instrumentation.h"
#endif

//--------------------------------------------------------------------------------------//

// returns number of instrumentations triggered
int64_t
mm(int64_t s, double* a, double* b, double* c)
{
    for(int64_t i = 0; i < s; i++)
    {
        for(int64_t j = 0; j < s; j++)
        {
            for(int64_t k = 0; k < s; k++)
                a[i * s + j] += b[i * s + k] * c[k * s + j];
        }
    }
    return s * s;
}

//--------------------------------------------------------------------------------------//

// returns number of instrumentations triggered
int64_t
mm_inst(int64_t s, double* a, double* b, double* c)
{
    for(int64_t i = 0; i < s; i++)
    {
        for(int64_t j = 0; j < s; j++)
        {
            char buf[32];
            sprintf(buf, "%" PRId64, j);

            INSTRUMENT_CREATE(buf);
            INSTRUMENT_START(buf);
            for(int64_t k = 0; k < s; k++)
                a[i * s + j] += b[i * s + k] * c[k * s + j];
            INSTRUMENT_STOP(buf);
        }
    }
    return s * s;
}

//--------------------------------------------------------------------------------------//

void
mm_reset(int64_t s, double* a, double* b, double* c)
{
    for(int64_t i = 0; i < s; i++)
        for(int64_t j = 0; j < s; j++)
            a[i * s + j] = b[i * s + j] = c[i * s + j] = 1.0;
}

//--------------------------------------------------------------------------------------//

c_runtime_data
c_execute_matmul(int64_t s, int64_t imax, int64_t nitr)
{
    printf("Running %" PRId64 " a MM on %" PRId64 " x %" PRId64 "\n", imax, s, s);
    double* a = (double*) malloc(s * s * sizeof(double));
    double* b = (double*) malloc(s * s * sizeof(double));
    double* c = (double*) malloc(s * s * sizeof(double));

    int64_t        nentries = nitr + 1;
    c_runtime_data data     = { nentries, NULL, NULL, NULL, NULL };
    data.inst_count         = (int64_t*) malloc(nentries * sizeof(int64_t));
    data.timing             = (double*) malloc(nentries * sizeof(double));
    data.inst_per_sec       = (double*) malloc(nentries * sizeof(double));
    data.overhead           = (double*) malloc(nentries * sizeof(double));

    mm_reset(s, a, b, c);

    // warm-up
    {
        int64_t inst_count = 0;
        for(int64_t iter = 0; iter < imax; iter++)
            inst_count += mm(s, a, b, c);
        mm_reset(s, a, b, c);
    }

    // base-line
    for(int64_t i = 0; i < nitr; ++i)
    {
        double  t1         = wtime();
        int64_t inst_count = 0;
        for(int64_t iter = 0; iter < imax; iter++)
            inst_count += mm(s, a, b, c);
        double tdiff = wtime() - t1;

        int idx = 0;
        data.inst_count[idx] += inst_count;
        data.timing[idx] += tdiff;
        data.inst_per_sec[idx] += ((double) inst_count) / tdiff;
        data.overhead[idx] = 0.0;

        if(i + 1 == nitr)
        {
            data.inst_count[idx] /= nitr;
            data.timing[idx] /= nitr;
            data.inst_per_sec[idx] /= nitr;
        }
    }

    // with instrumentation
    for(int64_t i = 0; i < nitr; ++i)
    {
        double  t1         = wtime();
        int64_t inst_count = 0;
        for(int64_t iter = 0; iter < imax; iter++)
            inst_count += mm_inst(s, a, b, c);
        double tdiff = wtime() - t1;

        int idx                = i + 1;
        data.inst_count[idx]   = inst_count;
        data.timing[idx]       = tdiff;
        data.inst_per_sec[idx] = ((double) inst_count) / tdiff;
        data.overhead[idx]     = ((tdiff - data.timing[0]) / ((double) inst_count));
    }

    free(a);
    free(b);
    free(c);

    return data;
}

//--------------------------------------------------------------------------------------//
