#!/usr/bin/env python

import os
import argparse
import instrument_benchmark as bench
from statistics import mean, stdev

import numpy as np
import matplotlib.pyplot as plt


def plot(x, y, yerr, label, fname):
    fig = plt.figure(figsize=(1600 / 75, 800 / 75), dpi=75)
    ax = fig.add_subplot(111)
    plt.title(label)
    plt.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    axes = plt.gca()
    _max = 1.25 * max([abs(y[i]) + abs(yerr[i]) for i in range(len(y))])
    axes.set_ylim([-_max, _max])
    axes.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    plt.errorbar(x, y, yerr=yerr, fmt='bo-', label=label)
    plt.xticks(rotation=90, rotation_mode='anchor')
    # zip joins x and y coordinates in pairs
    for _x, _y in zip(x, y):
        _label = "{:8.3e}".format(_y)
        plt.annotate(_label, (_x, _y), textcoords="offset points",
                     xytext=(0, 10), ha='center')
    box = ax.get_position()
    ax.set_position([box.x0, box.y0 + 0.15 * box.height,
                     box.width, 0.85 * box.height])
    plt.savefig(fname, dpi=75)
    # plt.show()


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
    return {"runtime": [mean(_ftime), stdev(_ftime)],
            "overhead": [mean(_fover), stdev(_fover)]}


if __name__ == "__main__":

    parser = argparse.ArgumentParser()

    parser.add_argument("-n", "--size", type=int,
                        default=100, help="Matrix size (N x N)")
    parser.add_argument("-i", "--iterations", type=int, default=50,
                        help="Number of iterations per timing entry")
    parser.add_argument("-e", "--entries", type=int,
                        default=250, help="Number of timing entries")
    parser.add_argument("-f", "--fibonacci", type=int,
                        default=43, help="Fibonacci value")
    parser.add_argument("-c", "--cutoff", type=int,
                        default=23, help="Fibonacci cutoff")
    parser.add_argument("-m", "--modes", type=str, nargs='*',
                        default=["fibonacci", "matrix"], choices=["fibonacci", "matrix"])
    parser.add_argument("-p", "--prefix", type=str, default="DISABLED")

    args = parser.parse_args()

    m_N = args.size         # matrix size is N x N
    m_I = args.iterations   # number of iterations per timing entry
    m_E = args.entries      # number of timing entries
    m_F = args.fibonacci    # fibonacci value
    m_C = args.cutoff       # cutoff value

    mtx_keys = []
    fib_keys = []
    mtx_time_data = {"y": [], "yerr": []}
    fib_time_data = {"y": [], "yerr": []}
    mtx_over_data = {"y": [], "yerr": []}
    fib_over_data = {"y": [], "yerr": []}

    if "matrix" in args.modes:
        for lang in ["c", "cxx"]:
            for submodule in sorted(bench.submodules):
                ret = getattr(bench, submodule).matmul(m_N, m_E, m_I, lang)
                if ret is not None:
                    data = print_info(ret, "[{}]> {}_{}".format(
                        lang.upper(), "MATMUL", submodule.upper()))
                    print("")  # spacing
                    module = submodule.upper()
                    mtx_keys += ["{}_MATMUL_{}".format(lang.upper(), module)]
                    mtx_time_data["y"] += [data["runtime"][0]]
                    mtx_over_data["y"] += [data["overhead"][0]]
                    mtx_time_data["yerr"] += [data["runtime"][1]]
                    mtx_over_data["yerr"] += [data["overhead"][1]]

    if len(mtx_keys) > 0:
        plot(mtx_keys, mtx_time_data["y"],
             mtx_time_data["yerr"], "Matrix Multiply ({} x {}) Runtime".format(
                 m_N, m_N),
             "{}_MATMUL_RUNTIME.png".format(args.prefix.upper()))
        plot(mtx_keys, mtx_over_data["y"],
             mtx_over_data["yerr"], "Matrix Multiply  ({} x {}) Overhead".format(
                 m_N, m_N),
             "{}_MATMUL_OVERHEAD.png".format(args.prefix.upper()))

    if "fibonacci" in args.modes:
        for lang in ["c", "cxx"]:
            for submodule in sorted(bench.submodules):
                ret = getattr(bench, submodule).fibonacci(m_F, m_C, m_I, lang)
                if ret is not None:
                    data = print_info(ret, "[{}]> {}_{}".format(
                        lang.upper(), "FIBONACCI", submodule.upper()))
                    print("")  # spacing
                    module = submodule.upper()
                    fib_keys += ["{}_FIB_{}".format(lang.upper(), module)]
                    fib_time_data["y"] += [data["runtime"][0]]
                    fib_over_data["y"] += [data["overhead"][0]]
                    fib_time_data["yerr"] += [data["runtime"][1]]
                    fib_over_data["yerr"] += [data["overhead"][1]]

    if len(fib_keys) > 0:
        plot(fib_keys, fib_time_data["y"],
             fib_time_data["yerr"], "Fibonacci({}, {}) Runtime".format(
                 m_F, m_C),
             "{}_FIBONACCI_RUNTIME.png".format(args.prefix.upper()))
        plot(fib_keys, fib_over_data["y"],
             fib_over_data["yerr"], "Fibonacci({}, {}) Overhead".format(
                 m_F, m_C),
             "{}_FIBONACCI_OVERHEAD.png".format(args.prefix.upper()))
