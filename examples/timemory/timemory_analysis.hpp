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

#include <iostream>
#include <string>

#include <timemory/timemory.hpp>

using namespace tim::component;

namespace general
{
using total_t  = papi_tuple<PAPI_TOT_CYC, PAPI_RES_STL, PAPI_PRF_DM>;
using inst_t   = papi_tuple<PAPI_TOT_INS, PAPI_LST_INS>;
using branch_t = papi_tuple<PAPI_BR_UCN, PAPI_BR_CN, PAPI_BR_TKN, PAPI_BR_NTK,
                            PAPI_BR_MSP, PAPI_BR_PRC, PAPI_BR_INS>;
}  // namespace general

namespace misses
{
using data_cache_t  = papi_tuple<PAPI_L1_DCM, PAPI_L2_DCM, PAPI_L3_DCM>;
using inst_cache_t  = papi_tuple<PAPI_L1_ICM, PAPI_L2_ICM>;
using total_cache_t = papi_tuple<PAPI_L1_TCM, PAPI_L2_TCM, PAPI_L3_TCM>;
}  // namespace misses

namespace reads
{
using data_cache_t  = papi_tuple<PAPI_L1_DCR, PAPI_L3_DCR>;
using inst_cache_t  = papi_tuple<PAPI_L2_ICR, PAPI_L3_ICR>;
using total_cache_t = papi_tuple<PAPI_L2_TCR, PAPI_L3_TCR>;
}  // namespace reads

namespace accesses
{
using data_cache_t  = papi_tuple<PAPI_L1_DCA, PAPI_L3_DCA>;
using inst_cache_t  = papi_tuple<PAPI_L2_ICA, PAPI_L3_ICA>;
using total_cache_t = papi_tuple<PAPI_L2_TCA, PAPI_L3_TCA>;
}  // namespace accesses

namespace writes
{
using data_cache_t  = papi_tuple<PAPI_L1_DCW, PAPI_L3_DCW>;
using total_cache_t = papi_tuple<PAPI_L2_TCW, PAPI_L3_TCW>;
}  // namespace writes

using hwcount_t = general::inst_t;
using analysis_t = tim::auto_tuple<papi_array_t>;
using toolset_t = tim::auto_tuple<wall_clock>;

#define INSTRUMENT_CONFIGURE()
#define INSTRUMENT_CREATE(...)
#define INSTRUMENT_START(...) TIMEMORY_BLANK_POINTER(toolset_t, __FUNCTION__)
#define INSTRUMENT_STOP(...)

#define ANALYSIS_CREATE(...)
#define ANALYSIS_START(name, ...) TIMEMORY_BLANK_CALIPER(t, analysis_t, name)
#define ANALYSIS_STOP(...) TIMEMORY_CALIPER_APPLY0(t, stop)
