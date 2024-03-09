#!/bin/sh
# Licensed under the Apache License, Version 2.0

# This script is designed to assist developers with GNU autotools,
# specifically:
#   - aclocal
#   - libtoolize
#   - autoconf
#   - autoheader
#   - automake
# It aims to simplify the process of generating configuration files
# for autotools.

# The actual source files:
#   - acinclude.m4 (used by aclocal)
#   - configure.ac (main autoconf file)
#   - Makefile.am, */Makefile.am (automake config files)
# All other files are auto-generated.

# Clean the generated files if "clean" argument is passed
if [ "$1" = "clean" ]; then
    echo "Cleaning..."
    rm configure aclocal.m4
    rm m4/l*
    rm config/*
    rmdir config
    find . -iname "Makefile.in" -type f -exec rm '{}' +
fi

bail_out()
{
    echo
    echo "  Something went wrong, bailing out!"
    echo
    exit 1
}

# Determine the libtoolize command and ensure its existence
if command -v libtoolize >/dev/null 2>&1; then
  LIBTOOLIZE="$(command -v libtoolize)"
elif command -v glibtoolize >/dev/null 2>&1; then
  LIBTOOLIZE="$(command -v glibtoolize)"
else
  echo "Unable to find a valid copy of libtoolize or glibtoolize in your PATH!"
  bail_out
fi

# Step 1: Generate aclocal.m4
mkdir -p config
echo "Running aclocal"
aclocal -I config || bail_out

# Step 2: Run libtoolize
echo "Running $LIBTOOLIZE"
$LIBTOOLIZE -f -c || bail_out
$LIBTOOLIZE --automake || bail_out
echo "Running aclocal"
aclocal -I config || bail_out

# Step 3: Generate configure and include/miaconfig.h
echo "Running autoconf"
autoconf || bail_out

# Check for pkg-config in configure
if grep -q PKG_CHECK_MODULES configure; then
  rm configure
  echo "Missing pkg-config. Check the build requirements."
  bail_out
fi

# Step 4: Generate config.h.in
echo "Running autoheader"
autoheader -f || bail_out

# Step 5: Generate Makefile.in, src/Makefile.in, and other necessary files
echo "Running automake --add-missing --copy"
automake --add-missing --copy --warnings=all || bail_out

echo ""
echo "All done."
echo "To build the software now, do something like:"
echo ""
echo "$ ./configure [--enable-debug] [...other options]"
