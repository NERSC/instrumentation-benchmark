# Instrumentation Benchmark

Performance analysis tests for performance analysis tools

## Implementing Custom Instrumentation

1. Create a `cmake/UserSettings.cmake` file
2. Create a `include/user/instrumentation.h` file
3. Create a `include/user/instrumentation.hpp` file

### CMake User Configuration

The `cmake/UserSettings.cmake` file configures two CMake `INTERFACE` libraries to provide :

- `user-c-config`
  - This will be linked to the C benchmarks
- `user-cxx-config`
  - This will be linked to the C++ benchmarks
  
`cmake/UserSettings.cmake` is the default user config file. In order to maintain multiple configs, override the `USER_CONFIG` CMake CACHE variable to point to the desired configuration file.

#### Example `UserSettings.cmake`

```cmake
add_library(user-c-config INTERFACE)
add_library(user-cxx-config INTERFACE)

find_package(timemory REQUIRED
    COMPONENTS headers papi vector compile-options)

target_link_libraries(user-c-config   INTERFACE timemory timemory-c-shared)
target_link_libraries(user-cxx-config INTERFACE timemory)
```

### Custom C Instrumentation Header

The `include/user/instrumentation.h` file will provide the user's C instrumentation when `USE_INST` is defined.

#### Example `instrumentation.h`

```c
#include <stdio.h>
#include <string.h>

#include <timemory/ctimemory.h>

#if !defined(USE_INST)

//
// will be invoked with something like:
//      INSTRUMENT_<MODE>("custom-tag")
//  and one can use __FILE__, __func__, and __LINE__ as needed
//
#    define INSTRUMENT_CREATE(...)
#    define INSTRUMENT_START(...)
#    define INSTRUMENT_STOP(...)

#else

//
// will be invoked with something like:
//      INSTRUMENT_<MODE>("custom-tag")
//  and one can use __FILE__, __func__, and __LINE__ as needed
//
#    define INSTRUMENT_CREATE(...)
#    define INSTRUMENT_START(name) void* timer = TIMEMORY_AUTO_TIMER(name);
#    define INSTRUMENT_STOP(name) FREE_TIMEMORY_AUTO_TIMER(timer);

#endif
```

### Custom C++ Instrumentation Header

The `include/user/instrumentation.hpp` file will provide the user's C++ instrumentation when `USE_INST` is defined.

#### Example `instrumentation.hpp`

```cpp
#include <iostream>
#include <string>

#include <timemory/timemory.hpp>

#if !defined(USE_INST)

//
// will be invoked with something like:
//      INSTRUMENT_<MODE>("custom-tag")
//  and one can use __FILE__, __func__, and __LINE__ as needed
//
#    define INSTRUMENT_CREATE(...)
#    define INSTRUMENT_START(...)
#    define INSTRUMENT_STOP(...)

#else

using namespace tim::component;
using auto_timer_t = tim::auto_tuple<real_clock, cpu_clock, peak_rss>;
//
// will be invoked with something like:
//      INSTRUMENT_<MODE>("custom-tag")
//  and one can use __FILE__, __func__, and __LINE__ as needed
//
#    define INSTRUMENT_CREATE(...)
#    define INSTRUMENT_START(name) TIMEMORY_POINTER(auto_timer_t, "_", name);
#    define INSTRUMENT_STOP(...)

#endif
```

## Building Project

Once `UserSettings.cmake`, `instrumentation.h`, and `instrumentation.hpp` have been created:

```console
$ mkdir -p build-release
$ cd build-release
$ cmake -DCMAKE_BUILD_TYPE=Release ..

-- The C compiler identification is GNU 9.2.0
-- The CXX compiler identification is GNU 9.2.0
-- Checking whether C compiler has -isysroot
-- Checking whether C compiler has -isysroot - yes
-- Checking whether C compiler supports OSX deployment target flag
-- Checking whether C compiler supports OSX deployment target flag - yes
-- Check for working C compiler: /opt/local/bin/gcc
-- Check for working C compiler: /opt/local/bin/gcc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting C compile features
-- Detecting C compile features - done
-- Checking whether CXX compiler has -isysroot
-- Checking whether CXX compiler has -isysroot - yes
-- Checking whether CXX compiler supports OSX deployment target flag
-- Checking whether CXX compiler supports OSX deployment target flag - yes
-- Check for working CXX compiler: /opt/local/bin/g++
-- Check for working CXX compiler: /opt/local/bin/g++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Performing Test c_instrument_compile_options_W
-- Performing Test c_instrument_compile_options_W - Success
-- Performing Test cxx_instrument_compile_options_W
-- Performing Test cxx_instrument_compile_options_W - Success
-- Performing Test c_instrument_compile_options_Wall
-- Performing Test c_instrument_compile_options_Wall - Success
-- Performing Test cxx_instrument_compile_options_Wall
-- Performing Test cxx_instrument_compile_options_Wall - Success
-- Performing Test c_instrument_compile_options_Wno_unknown_pragmas
-- Performing Test c_instrument_compile_options_Wno_unknown_pragmas - Success
-- Performing Test cxx_instrument_compile_options_Wno_unknown_pragmas
-- Performing Test cxx_instrument_compile_options_Wno_unknown_pragmas - Success
-- Performing Test c_instrument_compile_options_Wno_ignored_attributes
-- Performing Test c_instrument_compile_options_Wno_ignored_attributes - Success
-- Performing Test cxx_instrument_compile_options_Wno_ignored_attributes
-- Performing Test cxx_instrument_compile_options_Wno_ignored_attributes - Success
-- Performing Test c_instrument_compile_options_Wno_attributes
-- Performing Test c_instrument_compile_options_Wno_attributes - Success
-- Performing Test cxx_instrument_compile_options_Wno_attributes
-- Performing Test cxx_instrument_compile_options_Wno_attributes - Success
-- Performing Test c_instrument_compile_options_Wno_unused_local_typedefs
-- Performing Test c_instrument_compile_options_Wno_unused_local_typedefs - Success
-- Performing Test cxx_instrument_compile_options_Wno_unused_local_typedefs
-- Performing Test cxx_instrument_compile_options_Wno_unused_local_typedefs - Success
-- Performing Test c_instrument_compile_options_faligned_new
-- Performing Test c_instrument_compile_options_faligned_new - Failed
-- Performing Test cxx_instrument_compile_options_faligned_new
-- Performing Test cxx_instrument_compile_options_faligned_new - Success
--
-- timemory was not compiled with support for the following INTERFACE libraries (empty INTERFACE targets):
--     timemory-mpi
--     timemory-papi
--     timemory-papi-static
--     timemory-cuda
--     timemory-cudart
--     timemory-cudart-device
--     timemory-cudart-static
--     timemory-cupti
--     timemory-cuda-nvtx
-- [Instrumentation-Benchmark] timemory : "timemory" components: compile-options, gperftools-cpu, headers, vector
-- Found timemory: /opt/timemory
-- Found PythonInterp: /opt/local/bin/python3.7 (found version "3.7.4")
-- Found PythonLibs: /opt/local/Library/Frameworks/Python.framework/Versions/3.7/lib/libpython3.7m.dylib
-- pybind11 v2.3.dev1
-- Performing Test HAS_FLTO
-- Performing Test HAS_FLTO - Failed
-- LTO disabled (not supported by the compiler and/or linker)
-- Configuring done
-- Generating done
-- Build files have been written to: /Users/jrmadsen/devel/c++/instrumentation-benchmark/build-release

$ make

[  6%] Building CXX object CMakeFiles/cxx-disable-instrument.dir/source/fibonacci.cpp.o
[ 12%] Building CXX object CMakeFiles/cxx-disable-instrument.dir/source/matmul.cpp.o
[ 18%] Linking CXX shared library instrument_benchmark/libcxx-disable-instrument.dylib
[ 18%] Built target cxx-disable-instrument
[ 25%] Building C object CMakeFiles/c-disable-instrument.dir/source/fibonacci.c.o
[ 31%] Building C object CMakeFiles/c-disable-instrument.dir/source/matmul.c.o
[ 37%] Linking C shared library instrument_benchmark/libc-disable-instrument.dylib
[ 37%] Built target c-disable-instrument
[ 43%] Building CXX object CMakeFiles/libinstrument_benchmark_disabled.dir/source/python/libinstrument_benchmark.cpp.o
[ 50%] Linking CXX shared module instrument_benchmark/libinstrument_benchmark_disabled.cpython-37m-darwin.so
[ 50%] Built target libinstrument_benchmark_disabled
[ 56%] Building C object CMakeFiles/c-enable-instrument.dir/source/fibonacci.c.o
[ 62%] Building C object CMakeFiles/c-enable-instrument.dir/source/matmul.c.o
[ 68%] Linking C shared library instrument_benchmark/libc-enable-instrument.dylib
[ 68%] Built target c-enable-instrument
[ 75%] Building CXX object CMakeFiles/cxx-enable-instrument.dir/source/fibonacci.cpp.o
[ 81%] Building CXX object CMakeFiles/cxx-enable-instrument.dir/source/matmul.cpp.o
[ 87%] Linking CXX shared library instrument_benchmark/libcxx-enable-instrument.dylib
[ 87%] Built target cxx-enable-instrument
Scanning dependencies of target libinstrument_benchmark_enabled
[ 93%] Building CXX object CMakeFiles/libinstrument_benchmark_enabled.dir/source/python/libinstrument_benchmark.cpp.o
[100%] Linking CXX shared module instrument_benchmark/libinstrument_benchmark_enabled.cpython-37m-darwin.so
[100%] Built target libinstrument_benchmark_enabled
```

## Benchmarking Script

An example benchmarking script is located in `examples/execute.py`:
```python
#!/usr/bin/env python

import os


def print_info(results, label):

    _timing = ", ".join(["{:10.3e}".format(i) for i in results.timing()])
    _overhead = ", ".join(["{:10.3e}".format(i) for i in results.overhead()])
    print("\n{}".format(label))
    print("\t{:20} : {}".format("entries", results.entries()))
    print("\t{:20} : {}".format("runtime (sec)", _timing))
    print("\t{:20} : {}".format("overhead (sec)", _overhead))


if __name__ == "__main__":

    os.environ["TIMEMORY_ENABLED"] = "OFF"
    import instrument_benchmark as bench

    m_N = 50      # matrix size is 50 x 50
    m_I = 100     # 100 iterations per timing entry
    m_E = 5       # 5 timing entries

    # run C benchmarks
    disabled_c = bench.disabled.matmul("c", m_N, m_I, m_E)
    enabled_c = bench.enabled.matmul("c", m_N, m_I, m_E)
    # run C++ benchmarks
    disabled_cxx = bench.disabled.matmul("cxx", m_N, m_I, m_E)
    enabled_cxx = bench.enabled.matmul("cxx", m_N, m_I, m_E)

    # print results
    print_info(disabled_c, "[_c_]> Disabled (no instrumentation)")
    print_info(enabled_c,  "[_c_]> Enabled but dormant")
    print_info(disabled_cxx, "[cxx]> Disabled (no instrumentation)")
    print_info(enabled_cxx, "[cxx]> Enabled but dormant")
```

Copy this file to the build directory, edit as necessary, or create your own, and execute:

```console
$ cp ../examples/execute.py ./
$ python3.7 ./execute.py

Running 100 a MM on 50 x 50
Running 100 a MM on 50 x 50
Running 100 a MM on 50 x 50
Running 100 a MM on 50 x 50

[_c_]> Disabled (no instrumentation)
	entries              : 6
	runtime (sec)        :  1.092e-02,  3.518e-02,  3.003e-02,  3.011e-02,  3.201e-02,  3.483e-02
	overhead (sec)       :  0.000e+00,  9.707e-08,  7.646e-08,  7.678e-08,  8.439e-08,  9.563e-08

[_c_]> Enabled but dormant
	entries              : 6
	runtime (sec)        :  9.467e-03,  2.925e+00,  2.461e+00,  2.181e+00,  2.457e+00,  2.338e+00
	overhead (sec)       :  0.000e+00,  1.166e-05,  9.805e-06,  8.687e-06,  9.789e-06,  9.315e-06

[cxx]> Disabled (no instrumentation)
	entries              : 6
	runtime (sec)        :  8.812e-03,  8.883e-03,  8.985e-03,  9.431e-03,  9.846e-03,  8.896e-03
	overhead (sec)       :  0.000e+00,  2.840e-10,  6.922e-10,  2.476e-09,  4.136e-09,  3.365e-10

[cxx]> Enabled but dormant
	entries              : 6
	runtime (sec)        :  7.163e-03,  9.836e-03,  9.563e-03,  9.805e-03,  9.844e-03,  9.950e-03
	overhead (sec)       :  0.000e+00,  1.069e-08,  9.600e-09,  1.057e-08,  1.072e-08,  1.115e-08
```

## TODO

- Provide functions that compute the statistics
- Write fibonacci benchmarks
