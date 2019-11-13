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

#include "@SUBMODULE_HEADER_FILE@"

// assume this is bare minimum...
#if !defined(INSTRUMENT_CREATE) && !defined(INSTRUMENT_START)
#    error "Submodule header did not define INSTRUMENT_CREATE or INSTRUMENT_START"
#endif

// provides instrumentation definitions if not
#include "fallback_inst.h"
// provides structures for returning data to python
#include "instrumentation.hpp"

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
    return 0;
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
            INSTRUMENT_CREATE(j);
            INSTRUMENT_START(j);
            for(int64_t k = 0; k < s; k++)
                a[i * s + j] += b[i * s + k] * c[k * s + j];
            INSTRUMENT_STOP(j);
        }
    }
    return s * s;
}

//--------------------------------------------------------------------------------------//

// returns sum of a
double
mm_sum(int64_t s, double* a)
{
    double sum = 0.0;
    for(int64_t i = 0; i < s; i++)
        for(int64_t j = 0; j < s; j++)
            sum += a[i * s + j];
    return sum;
}

//--------------------------------------------------------------------------------------//

void
mm_reset(int64_t s, double* a, double* b, double* c)
{
    for(int64_t i = 0; i < s; i++)
        for(int64_t j = 0; j < s; j++)
        {
            a[i * s + j] = 0.0;
            b[i * s + j] = 1.0;
            c[i * s + j] = 2.0;
        }
}

//--------------------------------------------------------------------------------------//

cxx_runtime_data
cxx_execute_matmul(int64_t s, int64_t imax, int64_t nitr)
{
    using dvec_t  = std::vector<double>;
    using ivec_t  = std::vector<int64_t>;
    using entry_t = std::tuple<int64_t, int64_t, double>;

    printf("\nRunning %" PRId64 " MM on %" PRId64 " x %" PRId64 "\n", imax, s, s);
    auto _a = dvec_t(s * s, 0.0);
    auto _b = dvec_t(s * s, 0.0);
    auto _c = dvec_t(s * s, 0.0);

    auto a = _a.data();
    auto b = _b.data();
    auto c = _c.data();

    cxx_runtime_data data(nitr);

    double base_sum = 0.0;
    // base-line and warm-up
    for(int64_t i = 0; i < nitr; ++i)
    {
        mm_reset(s, a, b, c);
        int64_t inst_count = 0;
        ANALYSIS_CREATE("matmul", i, nitr, s, s);
        ANALYSIS_START("matmul", i, nitr, s, s);
        for(int64_t iter = 0; iter < imax; iter++)
            inst_count += mm(s, a, b, c);
        ANALYSIS_STOP("matmul", i, nitr, s, s);
        base_sum += mm_sum(s, a);
    }

    double inst_sum = 0.0;
    // with instrumentation
    for(int64_t i = 0; i < nitr; ++i)
    {
        mm_reset(s, a, b, c);
        int64_t inst_count = 0;
        ANALYSIS_CREATE("matmul_inst", i, nitr, s, s);
        ANALYSIS_START("matmul_inst", i, nitr, s, s);
        double t_beg = wtime();
        for(int64_t iter = 0; iter < imax; iter++)
            inst_count += mm_inst(s, a, b, c);
        double t_end = wtime();
        ANALYSIS_STOP("matmul_inst", i, nitr, s, s);
        double t_diff = t_end - t_beg;
        inst_sum += mm_sum(s, a);
        data += entry_t(i, inst_count, t_diff);
    }

    if(abs(base_sum - inst_sum) > 1.0e-9)
        fprintf(stderr, "Error! Baseline result != instrumentation result: %f vs. %f",
                base_sum, inst_sum);

    return data;
}

//--------------------------------------------------------------------------------------//
