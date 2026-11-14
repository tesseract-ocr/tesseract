#!/usr/bin/env python3

# Summarize a call graph which was written by the macOS tool "sample".
#
# Usage:
#   sample PID SECONDS 1 -file sample.txt
#   sample-summary.py sample.txt [NUMBER_OF_FUNCTIONS]
#
# Prints the functions of the main thread with the largest inclusive sample
# counts (a function and everything which it calls), and the callers of the
# dot product functions. "sample" splits the counts of a function by call
# site, so its own summary does not show the totals.

import collections
import re
import sys

# Functions for which the direct callers are listed.
LEAF_FUNCTIONS = ('DotProductNEON', 'RowDotProductsNEON',
                  'CrossDotProductsNEON')


def main():
    if len(sys.argv) < 2:
        sys.exit(f'Usage: {sys.argv[0]} sample.txt [NUMBER_OF_FUNCTIONS]')
    limit = int(sys.argv[2]) if len(sys.argv) > 2 else 40
    with open(sys.argv[1]) as f:
        lines = f.read().split('\n')
    start = next(i for i, l in enumerate(lines) if l.startswith('Call graph:'))
    end = next(i for i, l in enumerate(lines)
               if l.startswith('Total number in stack'))
    # Each line is the indentation (which gives the depth), the count and
    # the name of the function.
    pattern = re.compile(r'^([ +!:|]*?)(\d+) (.*)$')
    inclusive = collections.Counter()
    callers = collections.Counter()
    stack = []  # (depth, name) of the callers of the current line
    total = 0
    in_main_thread = False
    for line in lines[start + 1:end]:
        match = pattern.match(line)
        if not match:
            continue
        depth = len(match.group(1))
        count = int(match.group(2))
        name = re.sub(r'  \(in .*$', '', match.group(3))
        name = re.sub(r'\(.*$', '', name).replace('tesseract::', '')
        if name.startswith('Thread_'):
            in_main_thread = 'main-thread' in line
            if in_main_thread:
                total = count
            stack = []
            continue
        if not in_main_thread:
            continue
        while stack and stack[-1][0] >= depth:
            stack.pop()
        ancestors = [n for _, n in stack]
        # Count recursive calls only once.
        if name not in ancestors:
            inclusive[name] += count
        if name in LEAF_FUNCTIONS and ancestors:
            caller = ancestors[-1]
            caller2 = ancestors[-2] if len(ancestors) > 1 else ''
            callers[(name, caller, caller2)] += count
        stack.append((depth, name))
    if total == 0:
        sys.exit('No samples for the main thread found')
    print(f'Samples of the main thread: {total}')
    for name, count in inclusive.most_common(limit):
        print(f'{count:7d} {100.0 * count / total:5.1f}%  {name}')
    if callers:
        print('Callers of the dot product functions:')
        for (name, caller, caller2), count in callers.most_common(12):
            print(f'{count:7d} {100.0 * count / total:5.1f}%  '
                  f'{name} <- {caller} <- {caller2}')


if __name__ == '__main__':
    main()
