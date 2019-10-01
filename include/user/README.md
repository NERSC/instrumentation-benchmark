# Implementation Header

## Example 1

```c
#include <stdio.h>
#include <string.h>
#include <timemory/ctimemory.h>

#define INSTRUMENT_CREATE(...)
#define INSTRUMENT_START(name) void* timer = TIMEMORY_AUTO_TIMER(name);
#define INSTRUMENT_STOP(name) FREE_TIMEMORY_AUTO_TIMER(timer);
```

## Example 2

```cxx
#include <iostream>
#include <string>
#include <timemory/timemory.hpp>

using namespace tim::component;
using auto_timer_t = tim::auto_tuple<real_clock, cpu_clock, peak_rss>;

#define INSTRUMENT_CREATE(...)
#define INSTRUMENT_START(name) TIMEMORY_POINTER(auto_timer_t, "_", name);
#define INSTRUMENT_STOP(...)
```

## Example 3

```cxx
#include <iostream>
#include <string>

#include <timemory/library.h>
#include <timemory/timemory.hpp>

#define INSTRUMENT_CREATE(...)
#define INSTRUMENT_START(name)                                                           \
    auto inst_id = timemory_get_begin_record(TIMEMORY_BASIC_LABEL("_", name).c_str());
#define INSTRUMENT_STOP(...) timemory_end_record(inst_id);
```
