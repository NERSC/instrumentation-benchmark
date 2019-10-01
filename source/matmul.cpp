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

void
mm_reset(int64_t s, double* a, double* b, double* c)
{
    for(int64_t i = 0; i < s; i++)
        for(int64_t j = 0; j < s; j++)
            a[i * s + j] = b[i * s + j] = c[i * s + j] = 1.0;
}

//--------------------------------------------------------------------------------------//

cxx_runtime_data
cxx_execute_matmul(int64_t s, int64_t imax, int64_t nitr)
{
    using dvec_t  = std::vector<double>;
    using ivec_t  = std::vector<int64_t>;
    using entry_t = std::tuple<int64_t, int64_t, double>;

    printf("Running %" PRId64 " a MM on %" PRId64 " x %" PRId64 "\n", imax, s, s);
    auto _a = dvec_t(s * s, 0.0);
    auto _b = dvec_t(s * s, 0.0);
    auto _c = dvec_t(s * s, 0.0);

    auto a = _a.data();
    auto b = _b.data();
    auto c = _c.data();

    cxx_runtime_data data(nitr + 1);

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

        data += entry_t(0, inst_count, tdiff);

        if(i + 1 == nitr)
            data /= std::tuple<int64_t, int64_t>(0, nitr);
    }

    // with instrumentation
    for(int64_t i = 0; i < nitr; ++i)
    {
        double  t1         = wtime();
        int64_t inst_count = 0;
        for(int64_t iter = 0; iter < imax; iter++)
            inst_count += mm_inst(s, a, b, c);
        double tdiff = wtime() - t1;

        data += entry_t(i + 1, inst_count, tdiff);
    }

    return data;
}

//--------------------------------------------------------------------------------------//
