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
