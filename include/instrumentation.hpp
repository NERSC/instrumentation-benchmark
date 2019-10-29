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

#include <chrono>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ratio>
#include <sys/time.h>
#include <tuple>
#include <vector>

#if defined(__cplusplus)
//--------------------------------------------------------------------------------------//
// the system's real time (i.e. wall time) clock, expressed as the amount of time since
// the epoch.
template <typename _Tp = double, typename Precision = std::nano>
_Tp
wtime()
{
    using clock_type    = std::chrono::steady_clock;
    using duration_type = std::chrono::duration<clock_type::rep, Precision>;
    return std::chrono::duration_cast<duration_type>(clock_type::now().time_since_epoch())
               .count() /
           static_cast<_Tp>(std::nano::den);
}
#endif

//--------------------------------------------------------------------------------------//
/// data on the performance
struct cxx_runtime_data
{
    using dvec_t   = std::vector<double>;
    using ivec_t   = std::vector<int64_t>;
    using entry_t  = std::tuple<int64_t, int64_t, double>;
    using result_t = std::tuple<int64_t, int64_t, double, double>;

    int64_t entries = 0;
    ivec_t  inst_count;
    dvec_t  timing;
    dvec_t  inst_per_sec;

    cxx_runtime_data()                        = default;
    ~cxx_runtime_data()                       = default;
    cxx_runtime_data(const cxx_runtime_data&) = default;
    cxx_runtime_data(cxx_runtime_data&&)      = default;
    cxx_runtime_data& operator=(const cxx_runtime_data&) = default;
    cxx_runtime_data& operator=(cxx_runtime_data&&) = default;

    cxx_runtime_data(int64_t _entries)
    : entries(_entries)
    , inst_count(ivec_t(entries, 0))
    , timing(dvec_t(entries, 0.0))
    , inst_per_sec(dvec_t(entries, 0.0))
    {
    }

    cxx_runtime_data& operator/=(const std::tuple<int64_t, int64_t>& _div)
    {
        auto idx = std::get<0>(_div);
        if(std::get<1>(_div) > 0)
        {
            inst_count[idx] /= std::get<1>(_div);
            timing[idx] /= std::get<1>(_div);
            inst_per_sec[idx] /= std::get<1>(_div);
        }
        return *this;
    }

    cxx_runtime_data& operator+=(const entry_t& _entry)
    {
        auto idx = std::get<0>(_entry);
        inst_count[idx] += std::get<1>(_entry);
        timing[idx] += std::get<2>(_entry);
        inst_per_sec[idx] +=
            static_cast<double>(std::get<1>(_entry)) / std::get<2>(_entry);
        return *this;
    }

    cxx_runtime_data& operator+=(const result_t& _result)
    {
        auto idx          = std::get<0>(_result);
        inst_count[idx]   = std::get<1>(_result);
        timing[idx]       = std::get<2>(_result);
        inst_per_sec[idx] = std::get<3>(_result);
        return *this;
    }
};

//--------------------------------------------------------------------------------------//
/// execute a matrix multiply test
///
cxx_runtime_data
cxx_execute_matmul(int64_t s, int64_t max, int64_t nitr);

/// execute a fibonacci tets
///
cxx_runtime_data
cxx_execute_fibonacci(int64_t nfib, int64_t cutoff, int64_t nitr);

//--------------------------------------------------------------------------------------//
