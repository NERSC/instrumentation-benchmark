#!/usr/bin/env python

import os
import argparse
import instrument_benchmark as bench
from statistics import mean, stdev


def print_info(results, label, chunk_size=5):

    nchunk = (len(results.timing()) - 1) / chunk_size
    nmodulo = (len(results.timing()) - 1) % chunk_size
    nitr = int(nchunk + 1 if nmodulo > 0 else nchunk)
    _time = ""
    _over = ""
    _inst = ""
    for i in range(nitr+1):
        if i == 0:
            beg = 0
            end = 1
        else:
            beg = (i - 1) * chunk_size + 1
            end = i * chunk_size + 1
        if end > len(results.timing()):
            end = len(results.timing())
        _rtime = results.timing()[beg:end]
        _rover = results.overhead()[beg:end]
        _rinst = results.inst_count()[beg:end]
        _t = ", ".join(["{:10.3e}".format(i) for i in _rtime])
        _o = ", ".join(["{:10.3e}".format(i) for i in _rover])
        _i = ", ".join(["{:10}".format(i) for i in _rinst])
        if i > 0:
            _time = "{}\n\t{:23}{}".format(_time, "", _t)
            _over = "{}\n\t{:23}{}".format(_over, "", _o)
            _inst = "{}\n\t{:23}{}".format(_inst, "", _i)
        else:
            _time = "{}  {:>10}".format(_t, "baseline")
            _over = "{}  {:>10}".format(_o, "baseline")
            _inst = "{}  {:>10}".format(_i, "baseline")
    _time = _time.strip("\n")
    _over = _over.strip("\n")
    _inst = _inst.strip("\n")
    _ftime = results.timing()[1:]
    _fover = results.overhead()[1:]

    print("\n{}".format(label))
    print("\t{:20} : {:10}".format("entries", results.entries()))
    print("")
    print("\t{:20} : {}".format("instrument (count)", _inst))
    print("")
    print("\t{:20} : {}".format("runtime (sec)", _time))
    print("")
    print("\t{:20} : {}".format("overhead (sec)", _over))
    print("")
    print("\t{:20} : {:10.3e}".format("runtime (mean)", mean(_ftime)))
    print("\t{:20} : {:10.3e}".format("runtime (stdev)", stdev(_ftime)))
    print("\t{:20} : {:10.3e}".format("overhead (mean)", mean(_fover)))
    print("\t{:20} : {:10.3e}".format("overhead (stdev)", stdev(_fover)))


if __name__ == "__main__":

    parser = argparse.ArgumentParser()

    parser.add_argument("-n", "--size", type=int,
                        default=50, help="Matrix size (N x N)")
    parser.add_argument("-i", "--iterations", type=int, default=100,
                        help="Number of iterations per timing entry")
    parser.add_argument("-e", "--entries", type=int,
                        default=50, help="Number of timing entries")
    parser.add_argument("-f", "--fibonacci", type=int,
                        default=43, help="Fibonacci value")
    parser.add_argument("-c", "--cutoff", type=int,
                        default=23, help="Fibonacci cutoff")
    parser.add_argument("-m", "--modes", type=str, nargs='*',
                        default=["fibonacci", "matrix"], choices=["fibonacci", "matrix"])

    args = parser.parse_args()

    m_N = args.size         # matrix size is N x N
    m_I = args.iterations   # number of iterations per timing entry
    m_E = args.entries      # number of timing entries
    m_F = args.fibonacci    # fibonacci value
    m_C = args.cutoff       # cutoff value

    if "matrix" in args.modes:
        for lang in ["c", "cxx"]:
            for submodule in sorted(bench.submodules):
                ret = getattr(bench, submodule).matmul(m_N, m_I, m_E, lang)
                if ret is not None:
                    print_info(ret, "[{:^3}]> {}".format(
                        lang, submodule.upper()))
                    print("")  # spacing

    if "fibonacci" in args.modes:
        for lang in ["cxx"]:
            for submodule in sorted(bench.submodules):
                ret = getattr(bench, submodule).fibonacci(m_F, m_C, m_I, lang)
                if ret is not None:
                    print_info(ret, "[{:^3}]> {}".format(
                        lang, submodule.upper()))
                    print("")  # spacing
