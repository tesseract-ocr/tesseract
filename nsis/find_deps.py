#!/usr/bin/env python3
#
# Copyright (C) 2024 Stefan Weil
#
# SPDX-License-Identifier: MIT
#
# Find the DLL files which are required for a given set of
# Windows executables and libraries.

import argparse
import os
import pefile

VERBOSE = False

def find_dependencies(binary, search_path, analyzed_deps):
    pe = pefile.PE(binary)
    pe.parse_data_directories()
    if VERBOSE:
        print(f'{binary}:')
    # print(pe.dump_info())

    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        name = entry.dll.decode('utf-8')
        if name in analyzed_deps:
            if VERBOSE:
                print(f'skip {name} (already analyzed)')
            continue
        analyzed_deps.add(name)
        fullpath = os.path.join(search_path, name)
        if not os.path.exists(fullpath):
            # Not found, maybe system DLL. Skip it.
            if VERBOSE:
                print(f'skip {name} (not found, maybe system DLL)')
            continue
        print(fullpath)
        analyzed_deps = find_dependencies(fullpath, search_path, analyzed_deps)

    return analyzed_deps

def main():
    """
    Command-line interface for universal dependency scanner.
    """

    parser = argparse.ArgumentParser(description='Find and copy DLL dependencies')
    parser.add_argument('files', nargs='+', help='Paths to executable or library files')
    parser.add_argument('--dlldir', dest='dlldir', default='/mingw64/bin/',
                        help='path to dll files')

    args = parser.parse_args()

    # try:
    # Find dependencies
    analyzed_deps = set()
    for binary in args.files:
        if True:
            analyzed_deps = find_dependencies(binary, args.dlldir, analyzed_deps)
        # except:
        #    print(f'error: failed to find dependencies for {binary}')


if __name__ == '__main__':
    main()
