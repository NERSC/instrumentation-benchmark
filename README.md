# Instrumentation Benchmark

Performance analysis tests for performance analysis tools

## Implementing Custom Instrumentation

1. Create a `cmake/UserSettings.cmake` file
2. Create one or more header files in `include/user` that define the macros for the instrumentation

### CMake User Configuration

See [cmake/UserSettings.cmake](/cmake/README.md)

### Custom Instrumentation Header

#### Example for C

```c
#include <stdio.h>
#include <string.h>

#include <timemory/ctimemory.h>

#    define INSTRUMENT_CREATE(...)
#    define INSTRUMENT_START(name) void* timer = TIMEMORY_AUTO_TIMER(name);
#    define INSTRUMENT_STOP(name) FREE_TIMEMORY_AUTO_TIMER(timer);
```

### Example for C++

```cpp
#include <iostream>
#include <string>

#include <timemory/timemory.hpp>

using namespace tim::component;
using auto_timer_t = tim::auto_tuple<real_clock, cpu_clock, peak_rss>;

#    define INSTRUMENT_CREATE(...)
#    define INSTRUMENT_START(name) TIMEMORY_POINTER(auto_timer_t, "_", name);
#    define INSTRUMENT_STOP(...)
```

## Building Project

```console
mkdir build && cd build
cmake ..
```

## Benchmarking Script

An example benchmarking script is located in `examples/execute.py`:

```python
#!/usr/bin/env python

import os
import statistics


def print_info(results, label):

    _timing = ", ".join(["{:10.3e}".format(i) for i in results.timing()])
    _overhead = ", ".join(["{:10.3e}".format(i) for i in results.overhead()])
    _foverhead = results.overhead()[1:]

    print("\n{}".format(label))
    print("\t{:20} : {}".format("entries", results.entries()))
    print("\t{:20} : {}".format("runtime (sec)", _timing))
    print("\t{:20} : {}".format("overhead (sec)", _overhead))
    print("\t{:20} : {:10.3e}".format("avg overhead", statistics.mean(_foverhead)))
    print("\t{:20} : {:10.3e}".format("std-dev overhead", statistics.stdev(_foverhead)))


if __name__ == "__main__":

    os.environ["TIMEMORY_ENABLED"] = "OFF"
    import instrument_benchmark as bench

    m_N = 50      # matrix size is N x N
    m_I = 200     # number of iterations per timing entry
    m_E = 5       # number of timing entries

    for lang in ["c", "cxx"]:
        for submodule in bench.submodules:
            ret = getattr(bench, submodule).matmul(m_N, m_I, m_E, lang)
            if ret is not None:
                print_info(ret, "[{:^3}]> {}".format(lang, submodule))
            else:
                print("[{:^3}]> {} returned None".format(lang, submodule))
            print("")  # spacing
```

This file is copied over during the build process. Copy this file to another location and edit as necessary:

```console
$ python3.7 ./execute.py

[ c ]> timemory_cxx_templates returned None

[ c ]> timemory_cxx_library returned None

Running 200 a MM on 50 x 50

[ c ]> timemory_c_library
	entries              : 6
	runtime (sec)        :  1.588e-02,  3.254e-01,  3.241e-01,  3.261e-01,  3.261e-01,  3.304e-01
	overhead (sec)       :  0.000e+00,  6.190e-07,  6.165e-07,  6.204e-07,  6.205e-07,  6.291e-07
	avg overhead         :  6.211e-07
	std-dev overhead     :  4.760e-09

Running 200 a MM on 50 x 50

[ c ]> baseline
	entries              : 6
	runtime (sec)        :  1.570e-02,  5.110e-02,  5.154e-02,  5.192e-02,  5.282e-02,  5.191e-02
	overhead (sec)       :  0.000e+00,  7.081e-08,  7.168e-08,  7.245e-08,  7.424e-08,  7.243e-08
	avg overhead         :  7.232e-08
	std-dev overhead     :  1.267e-09

Running 200 a MM on 50 x 50

[ c ]> dormant
	entries              : 6
	runtime (sec)        :  1.587e-02,  5.108e-02,  5.112e-02,  5.044e-02,  5.111e-02,  5.163e-02
	overhead (sec)       :  0.000e+00,  7.041e-08,  7.049e-08,  6.914e-08,  7.047e-08,  7.151e-08
	avg overhead         :  7.041e-08
	std-dev overhead     :  8.403e-10

Running 200 a MM on 50 x 50

[cxx]> timemory_cxx_templates
	entries              : 6
	runtime (sec)        :  1.571e-02,  1.780e-02,  1.658e-02,  1.695e-02,  1.713e-02,  1.719e-02
	overhead (sec)       :  0.000e+00,  4.184e-09,  1.743e-09,  2.466e-09,  2.829e-09,  2.964e-09
	avg overhead         :  2.837e-09
	std-dev overhead     :  8.897e-10

Running 200 a MM on 50 x 50

[cxx]> timemory_cxx_library
	entries              : 6
	runtime (sec)        :  1.611e-02,  2.308e-01,  2.310e-01,  2.304e-01,  2.405e-01,  2.262e-01
	overhead (sec)       :  0.000e+00,  4.294e-07,  4.298e-07,  4.285e-07,  4.487e-07,  4.203e-07
	avg overhead         :  4.313e-07
	std-dev overhead     :  1.047e-08

[cxx]> timemory_c_library returned None

Running 200 a MM on 50 x 50

[cxx]> baseline
	entries              : 6
	runtime (sec)        :  1.574e-02,  1.539e-02,  1.555e-02,  1.541e-02,  1.542e-02,  1.575e-02
	overhead (sec)       :  0.000e+00, -6.949e-10, -3.726e-10, -6.530e-10, -6.306e-10,  3.510e-11
	avg overhead         : -4.632e-10
	std-dev overhead     :  3.059e-10

Running 200 a MM on 50 x 50

[cxx]> dormant
	entries              : 6
	runtime (sec)        :  1.622e-02,  1.591e-02,  1.550e-02,  1.548e-02,  1.562e-02,  1.537e-02
	overhead (sec)       :  0.000e+00, -6.140e-10, -1.434e-09, -1.484e-09, -1.206e-09, -1.704e-09
	avg overhead         : -1.289e-09
	std-dev overhead     :  4.165e-10
```

## TODO

- Write fibonacci benchmarks
