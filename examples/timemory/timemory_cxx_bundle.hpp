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
// using toolset_t = tim::auto_tuple<wall_clock, cpu_clock, peak_rss>;
// using toolset_t = tim::auto_timer;
using toolset_t = tim::auto_tuple<user_list_bundle>;

#define INSTRUMENT_CONFIGURE()                                                           \
    user_list_bundle::reset();                                                           \
    user_list_bundle::configure<wall_clock>();
#define INSTRUMENT_CREATE(...)
#define INSTRUMENT_START(...)                                                            \
    static auto bundle_label = TIMEMORY_JOIN("_", __FUNCTION__, "bundle")                \
        TIMEMORY_BLANK_POINTER(toolset_t, bundle_label.c_str());
#define INSTRUMENT_STOP(...)
