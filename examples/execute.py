#!/usr/bin/env python

import os
import argparse
import instrument_benchmark as bench
from statistics import mean, stdev

import numpy as np
import matplotlib.pyplot as plt

lout = None


def lprint(message):
    print("{}".format(message))
    if lout is not None:
        lout.write("{}\n".format(message))


def plot(x, y, yerr, label, fname):

    font = {'family': 'serif',
            'color':  'darkblue',
            'weight': 'bold',
            }

    dpi = 100
    fig = plt.figure(figsize=(1600 / dpi, 800 / dpi), dpi=dpi)
    ax = fig.add_subplot(111)
    plt.title(label, **font, size=22)
    axes = plt.gca()
    _max = 1.25 * max([abs(y[i]) + abs(yerr[i]) for i in range(len(y))])
    axes.set_ylim([-_max, _max])
    axes.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    plt.errorbar(x, y, yerr=yerr, fmt='bD--', label=label)
    plt.xticks(rotation=-45, rotation_mode='anchor', **font, size=18)
    # zip joins x and y coordinates in pairs
    for _x, _y in zip(x, y):
        _label = "{:8.3e}".format(_y)
        plt.annotate(_label, (_x, _y), textcoords="offset points",
                     xytext=(0, 10), ha='center', **font, size=18)
    box = ax.get_position()
    ax.set_position([box.x0, box.y0 + 0.15 * box.height,
                     box.width, 0.85 * box.height])
    plt.savefig(fname, dpi=75)
    # plt.show()


def print_info(results, label, chunk_size=5, baseline=None):

    ntimes = len(results.timing())
    nchunk = ntimes / chunk_size
    modulo = ntimes % chunk_size
    nitr = int(nchunk + 1 if modulo > 0 else nchunk)
    _time = ""
    _over = ""
    _inst = ""
    _ftime = results.timing()
    _finst = results.inst_count()
    _fover = results.overhead(baseline)
    for i in range(nitr):
        beg = i * chunk_size
        end = (i + 1) * chunk_size
        if end > ntimes:
            end = ntimes
        _rtime = _ftime[beg:end]
        _rover = _fover[beg:end]
        _rinst = _finst[beg:end]
        _t = ", ".join(["{:10.3e}".format(i) for i in _rtime])
        _o = ", ".join(["{:10.3e}".format(i) for i in _rover])
        _i = ", ".join(["{:10}".format(i) for i in _rinst])
        if i == 0:
            _time = "{}".format(_t)
            _over = "{}".format(_o)
            _inst = "{}".format(_i)
        else:
            _time = "{}\n\t{:23}{}".format(_time, "", _t)
            _over = "{}\n\t{:23}{}".format(_over, "", _o)
            _inst = "{}\n\t{:23}{}".format(_inst, "", _i)

    _time = _time.strip("\n")
    _over = _over.strip("\n")
    _inst = _inst.strip("\n")

    lprint("\n{}:\n".format(label))
    lprint("\t{:20} : {:10}".format("entries", results.entries()))
    lprint("")
    lprint("\t{:20} : {}".format("instrument (count)", _inst))
    lprint("")
    lprint("\t{:20} : {}".format("runtime (sec)", _time))
    lprint("")
    lprint("\t{:20} : {}".format("overhead (sec)", _over))
    lprint("")
    lprint("\t{:20} : {:10.3e}".format("runtime (mean)", mean(_ftime)))
    lprint("\t{:20} : {:10.3e}".format("runtime (stdev)", stdev(_ftime)))
    lprint("\t{:20} : {:10.3e}".format("overhead (mean)", mean(_fover)))
    lprint("\t{:20} : {:10.3e}".format("overhead (stdev)", stdev(_fover)))
    return {"runtime": [mean(_ftime), stdev(_ftime)],
            "overhead": [mean(_fover), stdev(_fover)]}


if __name__ == "__main__":

    submodules = sorted(bench.submodules)
    if len(submodules) == 0:
        raise RuntimeError("No submodules!")

    default_baseline = "baseline" if "baseline" in submodules else submodules[0]

    parser = argparse.ArgumentParser()

    parser.add_argument("-p", "--prefix", type=str, default="DISABLED")
    parser.add_argument("-m", "--modes", type=str, nargs='*',
                        default=["fibonacci", "matrix"], choices=["fibonacci", "matrix"])
    parser.add_argument("-l", "--languages", type=str, choices=["c", "cxx"],
                        default=["c", "cxx"], nargs='*')
    parser.add_argument("-b", "--baseline", type=str, choices=submodules,
                        default=default_baseline, required=False,
                        help="Compute overhead w.r.t. to this submodule measurement")
    parser.add_argument("-i", "--iterations", type=int, default=50,
                        help="Number of iterations per timing entry")
    parser.add_argument("--skip-plot", action='store_true', help="Skip plotting")
    # specific to MATMUL
    parser.add_argument("-n", "--size", type=int,
                        default=100, help="Matrix size (N x N)")
    parser.add_argument("-e", "--entries", type=int,
                        default=50, help="Number of timing entries")
    # specific to FIBONACCI
    parser.add_argument("-f", "--fibonacci", type=int,
                        default=43, help="Fibonacci value")
    parser.add_argument("-c", "--cutoff", type=int,
                        default=23, help="Fibonacci cutoff")

    args = parser.parse_args()

    # log file
    lout = open("{}.txt".format(args.prefix.strip('_')), 'w')

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

    # calculate the baseline first
    submodules.remove(args.baseline)
    submodules = [args.baseline] + submodules

    if "matrix" in args.modes:
        for lang in args.languages:
            baseline = None
            for submodule in submodules:
                key = "[{}]> {}_{}".format(
                    lang.upper(), "MATMUL", submodule.upper())
                lprint("Executing {}...".format(key))
                ret = getattr(bench, submodule).matmul(m_N, m_E, m_I, lang)
                if ret is not None:
                    if baseline is None:
                        baseline = ret
                    data = print_info(ret, key, baseline=baseline)
                    lprint("")  # spacing
                    module = submodule.upper()
                    mtx_keys += ["{}_MATMUL_{}".format(lang.upper(), module)]
                    mtx_time_data["y"] += [data["runtime"][0]]
                    mtx_over_data["y"] += [data["overhead"][0]]
                    mtx_time_data["yerr"] += [data["runtime"][1]]
                    mtx_over_data["yerr"] += [data["overhead"][1]]

    if len(mtx_keys) > 0 and not args.skip_plot:
        plot(mtx_keys, mtx_time_data["y"],
             mtx_time_data["yerr"], "Matrix Multiply ({} x {}) Runtime".format(
                 m_N, m_N),
             "{}_MATMUL_RUNTIME.png".format(args.prefix.upper()))
        plot(mtx_keys, mtx_over_data["y"],
             mtx_over_data["yerr"], "Matrix Multiply  ({} x {}) Overhead".format(
                 m_N, m_N),
             "{}_MATMUL_OVERHEAD.png".format(args.prefix.upper()))

    if "fibonacci" in args.modes:
        for lang in args.languages:
            baseline = None
            for submodule in submodules:
                key = "[{}]> {}_{}".format(
                    lang.upper(), "FIBONACCI", submodule.upper())
                lprint("Executing {}...".format(key))
                ret = getattr(bench, submodule).fibonacci(m_F, m_C, m_I, lang)
                if ret is not None:
                    if baseline is None:
                        baseline = ret
                    data = print_info(ret, key, baseline=baseline)
                    lprint("")  # spacing
                    module = submodule.upper()
                    fib_keys += ["{}_FIB_{}".format(lang.upper(), module)]
                    fib_time_data["y"] += [data["runtime"][0]]
                    fib_over_data["y"] += [data["overhead"][0]]
                    fib_time_data["yerr"] += [data["runtime"][1]]
                    fib_over_data["yerr"] += [data["overhead"][1]]

    if len(fib_keys) > 0 and not args.skip_plot:
        plot(fib_keys, fib_time_data["y"],
             fib_time_data["yerr"], "Fibonacci({}, {}) Runtime".format(
                 m_F, m_C),
             "{}_FIBONACCI_RUNTIME.png".format(args.prefix.upper().strip("_")))
        plot(fib_keys, fib_over_data["y"],
             fib_over_data["yerr"], "Fibonacci({}, {}) Overhead".format(
                 m_F, m_C),
             "{}_FIBONACCI_OVERHEAD.png".format(args.prefix.upper().strip("_")))

    lout.close()
