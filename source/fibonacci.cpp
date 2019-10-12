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

#include <cstdint>
#include <iostream>
#include <sstream>
#include <tuple>
#include <type_traits>

static int64_t nmeasure = 0;
using result_type       = std::tuple<int64_t, double>;

template <bool _Ret, typename _Tp = int>
using enable_if = typename std::enable_if<_Ret, _Tp>::type;

namespace mode
{
// clang-format off
struct none  {};
struct inst  {};
struct count {};
// clang-format on
}  // namespace mode

//======================================================================================//

int64_t
fib(int64_t n)
{
    return (n < 2) ? n : (fib(n - 1) + fib(n - 2));
}

//======================================================================================//

template <typename _Tp, enable_if<std::is_same<_Tp, mode::none>::value> = 0>
int64_t
fib(int64_t n, int64_t)
{
    return fib(n);
}

//======================================================================================//

template <typename _Tp, enable_if<std::is_same<_Tp, mode::count>::value> = 0>
int64_t
fib(int64_t n, int64_t cutoff)
{
    if(n > cutoff)
    {
        nmeasure += 1;
        return (n < 2) ? n : (fib<_Tp>(n - 1, cutoff) + fib<_Tp>(n - 2, cutoff));
    }
    return fib(n);
}

//======================================================================================//

template <typename _Tp, enable_if<std::is_same<_Tp, mode::inst>::value> = 0>
int64_t
fib(int64_t n, int64_t cutoff)
{
    if(n > cutoff)
    {
        INSTRUMENT_CREATE(n);
        INSTRUMENT_START(n);
        int64_t ret = (n < 2) ? n : (fib<_Tp>(n - 1, cutoff) + fib<_Tp>(n - 2, cutoff));
        INSTRUMENT_STOP(n);
        return ret;
    }
    return fib(n);
}

//======================================================================================//

template <typename _Tp>
result_type
run(int64_t n, int64_t cutoff)
{
    auto    t_beg  = wtime();
    int64_t result = fib<_Tp>(n, cutoff);
    auto    t_end  = wtime();
    return result_type(result, t_end - t_beg);
}

//======================================================================================//

template <typename _Tp>
int64_t
launch(const int64_t& nitr, const int64_t& nfib, const int64_t& cutoff,
       cxx_runtime_data& data, int64_t idx_mult)
{
    using entry_t = std::tuple<int64_t, int64_t, double>;

    // count the number of measurements and warm-up
    nmeasure           = 0;
    auto    ans_count  = fib<mode::count>(nfib, cutoff) * nitr;
    int64_t inst_count = (nmeasure * nitr);
    int64_t ans_run    = 0;
    for(int i = 0; i < nitr; ++i)
    {
        auto&& ret = run<_Tp>(nfib, cutoff);
        ans_run += std::get<0>(ret);
        auto idx = (i + 1) * idx_mult;
        data += entry_t(idx, inst_count, std::get<1>(ret));
    }

    if(std::is_same<_Tp, mode::none>::value && idx_mult == 0)
        data /= std::tuple<int64_t, int64_t>(0, nitr);

    // we need to use these values so they don't get optimized away
    if(ans_count != ans_run)
    {
        std::stringstream ss;
        ss << "Answer w/ counting != answer during run : " << ans_count << " vs. "
           << ans_run;
        throw std::runtime_error(ss.str());
    }
    return ans_run;
}

//======================================================================================//

cxx_runtime_data
cxx_execute_fibonacci(int64_t nfib, int64_t cutoff, int64_t nitr)
{
    cxx_runtime_data data(nitr + 1);

    std::cout << "\nRunning " << nitr << " iterations of fib(n = " << nfib
              << ", cutoff = " << cutoff << ")...\n"
              << std::endl;

    auto warmup = run<mode::none>(nfib, nfib);
    std::cout << "[warmup] fibonacci(" << nfib << ") = " << std::get<0>(warmup)
              << std::endl;

    //----------------------------------------------------------------------------------//
    //      run baseline and instruction mode
    //----------------------------------------------------------------------------------//
    auto ans_none = launch<mode::none>(nitr, nfib, nfib, data, 0);
    auto ans_inst = launch<mode::inst>(nitr, nfib, cutoff, data, 1);

    // we need to use these values so they don't get optimized away
    if(ans_none != ans_inst)
    {
        std::stringstream ss;
        ss << "Answer w/o instrumentation != answer w/ instrumentation : " << ans_none
           << " vs. " << ans_inst;
        throw std::runtime_error(ss.str());
    }

    return data;
}
