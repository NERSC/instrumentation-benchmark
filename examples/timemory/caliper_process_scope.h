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

#if defined(__cplusplus)
#    include <iostream>
#    include <string>
#else
#    include <stdint.h>
#endif

#include <caliper/cali.h>

#define INSTRUMENT_CONFIGURE() cali_init();

#if defined(__cplusplus)
#    define INSTRUMENT_CREATE(...)                                                       \
        static cali_id_t _id = cali_create_attribute(                                    \
            "inst", CALI_TYPE_STRING, CALI_ATTR_NESTED | CALI_ATTR_SCOPE_PROCESS);
#    define INSTRUMENT_START(...) cali_begin_string(_id, __FUNCTION__);
#    define INSTRUMENT_STOP(...) cali_end(_id);
#else
#    define INSTRUMENT_CREATE(...)                                                       \
        static cali_id_t* _id = NULL;                                                    \
        if(_id == NULL)                                                                  \
        {                                                                                \
            _id  = malloc(sizeof(cali_id_t));                                            \
            *_id = cali_create_attribute("inst", CALI_TYPE_STRING,                       \
                                         CALI_ATTR_NESTED | CALI_ATTR_SCOPE_PROCESS);    \
        }
#    define INSTRUMENT_START(...) cali_begin_string(*_id, __FUNCTION__);
#    define INSTRUMENT_STOP(...) cali_end(*_id);
#endif
