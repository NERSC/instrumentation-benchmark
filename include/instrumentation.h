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

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#if !defined(__cplusplus)
//--------------------------------------------------------------------------------------//
/// get the time
inline double
wtime()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return (double) now.tv_sec + 0.000001 * now.tv_usec;
}

#endif

#if defined(__cplusplus)
extern "C"
{
#endif

    //--------------------------------------------------------------------------------------//
    /// data on the performance
    typedef struct _runtime_data
    {
        int64_t  entries;
        int64_t* inst_count;
        double*  timing;
        double*  inst_per_sec;
        double*  overhead;
    } c_runtime_data;

    //--------------------------------------------------------------------------------------//
    /// execute a test
    c_runtime_data c_execute_matmul(int64_t s, int64_t max, int64_t nitr);

    //--------------------------------------------------------------------------------------//

#if defined(__cplusplus)
}
#endif
