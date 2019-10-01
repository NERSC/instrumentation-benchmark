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

//--------------------------------------------------------------------------------------//
// AUTO-CONFIGURED BY CMAKE!
//
// clang-format off
#define INST_MODULE_NAME @SUBMODULE_LIBRARY_NAME@
// clang-format on
//--------------------------------------------------------------------------------------//

#if !defined(INST_MODULE_NAME)
#    error "CMake configuration error! SUBMODULE_LIBRARY_NAME not configured."
#endif

#if !defined(USE_C) && !defined(USE_CXX) && !defined(USE_CUDA) && !defined(USE_Fortran)
#    error "No supported languages! Specify C, CXX, CUDA, or Fortran"
#endif

#if defined(USE_CXX)
#    define DEFAULT_LANGUAGE "cxx"
#elif defined(USE_C)
#    define DEFAULT_LANGUAGE "c"
#else
#    define DEFAULT_LANGUAGE "cxx"
#endif

#include "pybind11/cast.h"
#include "pybind11/chrono.h"
#include "pybind11/embed.h"
#include "pybind11/eval.h"
#include "pybind11/functional.h"
#include "pybind11/iostream.h"
#include "pybind11/numpy.h"
#include "pybind11/pybind11.h"
#include "pybind11/pytypes.h"
#include "pybind11/stl.h"

#include <string>

#include "instrumentation.h"
#include "instrumentation.hpp"

namespace py   = pybind11;
using string_t = std::string;

PYBIND11_MODULE(INST_MODULE_NAME, inst)
{
    py::add_ostream_redirect(inst, "ostream_redirect");

    // define calls to C tests here
#if defined(USE_C)
    auto execute_c_matmul = [](int64_t s, int64_t max, int64_t nitr) {
        c_runtime_data ret = c_execute_matmul(s, max, nitr);
        // convert to C++ type
        cxx_runtime_data _data(ret.entries);
        using result_t = std::tuple<int64_t, int64_t, double, double, double>;
        for(int64_t i = 0; i < ret.entries; ++i)
            _data += result_t(i, ret.inst_count[i], ret.timing[i], ret.inst_per_sec[i],
                              ret.overhead[i]);
        return _data;
    };
#endif

    // define calls to C++ tests here
#if defined(USE_CXX)
    auto execute_cxx_matmul = [](int64_t s, int64_t max, int64_t nitr) {
        return cxx_execute_matmul(s, max, nitr);
    };
#endif

    auto execute_matmul = [&](int64_t s, int64_t max, int64_t nitr, std::string lang) {
        for(auto& itr : lang)
            itr = tolower(itr);

        cxx_runtime_data* _data = nullptr;

        if(lang == "c")
        {
#    if defined(USE_C)
            _data = new cxx_runtime_data(execute_c_matmul(s, max, nitr));
#    endif
        }

        if(lang == "cxx")
        {
#if defined(USE_CXX)
            _data = new cxx_runtime_data(execute_cxx_matmul(s, max, nitr));
#endif
        }

        // potentially return None to Python
        return _data;
    };

    inst.def("matmul", execute_matmul, "Execute matrix multiply test",
             py::arg("size") = 100, py::arg("ientry") = 10000, py::arg("nitr") = 1,
             py::arg("language") = DEFAULT_LANGUAGE);

#if defined(BUILD_RUNTIME_DATA_BINDINGS)
    py::class_<cxx_runtime_data> runtime_data(inst, "runtime_data");
    runtime_data.def(py::init<>(), "construct runtime_data");
    runtime_data.def("entries", [](cxx_runtime_data* d) { return d->entries; });
    runtime_data.def("inst_count", [](cxx_runtime_data* d) { return d->inst_count; });
    runtime_data.def("timing", [](cxx_runtime_data* d) { return d->timing; });
    runtime_data.def("inst_per_sec", [](cxx_runtime_data* d) { return d->inst_per_sec; });
    runtime_data.def("overhead", [](cxx_runtime_data* d) { return d->overhead; });
#endif
}
