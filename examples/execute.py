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
