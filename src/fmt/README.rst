{fmt}
=====

.. image:: https://github.com/fmtlib/fmt/workflows/linux/badge.svg
   :target: https://github.com/fmtlib/fmt/actions?query=workflow%3Alinux

.. image:: https://github.com/fmtlib/fmt/workflows/macos/badge.svg
   :target: https://github.com/fmtlib/fmt/actions?query=workflow%3Amacos

.. image:: https://github.com/fmtlib/fmt/workflows/windows/badge.svg
   :target: https://github.com/fmtlib/fmt/actions?query=workflow%3Awindows

.. image:: https://ci.appveyor.com/api/projects/status/ehjkiefde6gucy1v?svg=true
   :target: https://ci.appveyor.com/project/vitaut/fmt

.. image:: https://oss-fuzz-build-logs.storage.googleapis.com/badges/fmt.svg
   :alt: fmt is continuously fuzzed at oss-fuzz
   :target: https://bugs.chromium.org/p/oss-fuzz/issues/list?\
            colspec=ID%20Type%20Component%20Status%20Proj%20Reported%20Owner%20\
            Summary&q=proj%3Dfmt&can=1

.. image:: https://img.shields.io/badge/stackoverflow-fmt-blue.svg
   :alt: Ask questions at StackOverflow with the tag fmt
   :target: https://stackoverflow.com/questions/tagged/fmt

**{fmt}** is an open-source formatting library providing a fast and safe
alternative to C stdio and C++ iostreams.

If you like this project, please consider donating to the BYSOL
Foundation that helps victims of political repressions in Belarus:
https://bysol.org/en/bs/general/.

`Documentation <https://fmt.dev>`__

Q&A: ask questions on `StackOverflow with the tag fmt
<https://stackoverflow.com/questions/tagged/fmt>`_.

Try {fmt} in `Compiler Explorer <https://godbolt.org/z/Eq5763>`_.

Features
--------

* Simple `format API <https://fmt.dev/latest/api.html>`_ with positional arguments
  for localization
* Implementation of `C++20 std::format
  <https://en.cppreference.com/w/cpp/utility/format>`__
* `Format string syntax <https://fmt.dev/latest/syntax.html>`_ similar to Python's
  `format <https://docs.python.org/3/library/stdtypes.html#str.format>`_
* Fast IEEE 754 floating-point formatter with correct rounding, shortness and
  round-trip guarantees
* Safe `printf implementation
  <https://fmt.dev/latest/api.html#printf-formatting>`_ including the POSIX
  extension for positional arguments
* Extensibility: `support for user-defined types
  <https://fmt.dev/latest/api.html#formatting-user-defined-types>`_
* High performance: faster than common standard library implementations of
  ``(s)printf``, iostreams, ``to_string`` and ``to_chars``, see `Speed tests`_
  and `Converting a hundred million integers to strings per second
  <http://www.zverovich.net/2020/06/13/fast-int-to-string-revisited.html>`_
* Small code size both in terms of source code with the minimum configuration
  consisting of just three files, ``core.h``, ``format.h`` and ``format-inl.h``,
  and compiled code; see `Compile time and code bloat`_
* Reliability: the library has an extensive set of `tests
  <https://github.com/fmtlib/fmt/tree/master/test>`_ and is `continuously fuzzed
  <https://bugs.chromium.org/p/oss-fuzz/issues/list?colspec=ID%20Type%20
  Component%20Status%20Proj%20Reported%20Owner%20Summary&q=proj%3Dfmt&can=1>`_
* Safety: the library is fully type safe, errors in format strings can be
  reported at compile time, automatic memory management prevents buffer overflow
  errors
* Ease of use: small self-contained code base, no external dependencies,
  permissive MIT `license
  <https://github.com/fmtlib/fmt/blob/master/LICENSE.rst>`_
* `Portability <https://fmt.dev/latest/index.html#portability>`_ with
  consistent output across platforms and support for older compilers
* Clean warning-free codebase even on high warning levels such as
  ``-Wall -Wextra -pedantic``
* Locale-independence by default
* Optional header-only configuration enabled with the ``FMT_HEADER_ONLY`` macro

See the `documentation <https://fmt.dev>`_ for more details.

Examples
--------

**Print to stdout** (`run <https://godbolt.org/z/Tevcjh>`_)

.. code:: c++

    #include <fmt/core.h>
    
    int main() {
      fmt::print("Hello, world!\n");
    }

**Format a string** (`run <https://godbolt.org/z/oK8h33>`_)

.. code:: c++

    std::string s = fmt::format("The answer is {}.", 42);
    // s == "The answer is 42."

**Format a string using positional arguments** (`run <https://godbolt.org/z/Yn7Txe>`_)

.. code:: c++

    std::string s = fmt::format("I'd rather be {1} than {0}.", "right", "happy");
    // s == "I'd rather be happy than right."

**Print chrono durations** (`run <https://godbolt.org/z/K8s4Mc>`_)

.. code:: c++

    #include <fmt/chrono.h>

    int main() {
      using namespace std::literals::chrono_literals;
      fmt::print("Default format: {} {}\n", 42s, 100ms);
      fmt::print("strftime-like format: {:%H:%M:%S}\n", 3h + 15min + 30s);
    }

Output::

    Default format: 42s 100ms
    strftime-like format: 03:15:30

**Print a container** (`run <https://godbolt.org/z/MjsY7c>`_)

.. code:: c++

    #include <vector>
    #include <fmt/ranges.h>

    int main() {
      std::vector<int> v = {1, 2, 3};
      fmt::print("{}\n", v);
    }

Output::

    [1, 2, 3]

**Check a format string at compile time**

.. code:: c++

    std::string s = fmt::format("{:d}", "I am not a number");

This gives a compile-time error in C++20 because ``d`` is an invalid format
specifier for a string.

**Write a file from a single thread**

.. code:: c++

    #include <fmt/os.h>

    int main() {
      auto out = fmt::output_file("guide.txt");
      out.print("Don't {}", "Panic");
    }

This can be `5 to 9 times faster than fprintf
<http://www.zverovich.net/2020/08/04/optimal-file-buffer-size.html>`_.

**Print with colors and text styles**

.. code:: c++

    #include <fmt/color.h>

    int main() {
      fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
                 "Hello, {}!\n", "world");
      fmt::print(fg(fmt::color::floral_white) | bg(fmt::color::slate_gray) |
                 fmt::emphasis::underline, "Hello, {}!\n", "мир");
      fmt::print(fg(fmt::color::steel_blue) | fmt::emphasis::italic,
                 "Hello, {}!\n", "世界");
    }

Output on a modern terminal:

.. image:: https://user-images.githubusercontent.com/
           576385/88485597-d312f600-cf2b-11ea-9cbe-61f535a86e28.png

Benchmarks
----------

Speed tests
~~~~~~~~~~~

================= ============= ===========
Library           Method        Run Time, s
================= ============= ===========
libc              printf          1.04
libc++            std::ostream    3.05
{fmt} 6.1.1       fmt::print      0.75
Boost Format 1.67 boost::format   7.24
Folly Format      folly::format   2.23
================= ============= ===========

{fmt} is the fastest of the benchmarked methods, ~35% faster than ``printf``.

The above results were generated by building ``tinyformat_test.cpp`` on macOS
10.14.6 with ``clang++ -O3 -DNDEBUG -DSPEED_TEST -DHAVE_FORMAT``, and taking the
best of three runs. In the test, the format string ``"%0.10f:%04d:%+g:%s:%p:%c:%%\n"``
or equivalent is filled 2,000,000 times with output sent to ``/dev/null``; for
further details refer to the `source
<https://github.com/fmtlib/format-benchmark/blob/master/src/tinyformat-test.cc>`_.

{fmt} is up to 20-30x faster than ``std::ostringstream`` and ``sprintf`` on
floating-point formatting (`dtoa-benchmark <https://github.com/fmtlib/dtoa-benchmark>`_)
and faster than `double-conversion <https://github.com/google/double-conversion>`_ and
`ryu <https://github.com/ulfjack/ryu>`_:

.. image:: https://user-images.githubusercontent.com/576385/
           95684665-11719600-0ba8-11eb-8e5b-972ff4e49428.png
   :target: https://fmt.dev/unknown_mac64_clang12.0.html

Compile time and code bloat
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The script `bloat-test.py
<https://github.com/fmtlib/format-benchmark/blob/master/bloat-test.py>`_
from `format-benchmark <https://github.com/fmtlib/format-benchmark>`_
tests compile time and code bloat for nontrivial projects.
It generates 100 translation units and uses ``printf()`` or its alternative
five times in each to simulate a medium sized project.  The resulting
executable size and compile time (Apple LLVM version 8.1.0 (clang-802.0.42),
macOS Sierra, best of three) is shown in the following tables.

**Optimized build (-O3)**

============= =============== ==================== ==================
Method        Compile Time, s Executable size, KiB Stripped size, KiB
============= =============== ==================== ==================
printf                    2.6                   29                 26
printf+string            16.4                   29                 26
iostreams                31.1                   59                 55
{fmt}                    19.0                   37                 34
Boost Format             91.9                  226                203
Folly Format            115.7                  101                 88
============= =============== ==================== ==================

As you can see, {fmt} has 60% less overhead in terms of resulting binary code
size compared to iostreams and comes pretty close to ``printf``. Boost Format
and Folly Format have the largest overheads.

``printf+string`` is the same as ``printf`` but with extra ``<string>``
include to measure the overhead of the latter.

**Non-optimized build**

============= =============== ==================== ==================
Method        Compile Time, s Executable size, KiB Stripped size, KiB
============= =============== ==================== ==================
printf                    2.2                   33                 30
printf+string            16.0                   33                 30
iostreams                28.3                   56                 52
{fmt}                    18.2                   59                 50
Boost Format             54.1                  365                303
Folly Format             79.9                  445                430
============= =============== ==================== ==================

``libc``, ``lib(std)c++`` and ``libfmt`` are all linked as shared libraries to
compare formatting function overhead only. Boost Format is a
header-only library so it doesn't provide any linkage options.

Running the tests
~~~~~~~~~~~~~~~~~

Please refer to `Building the library`__ for the instructions on how to build
the library and run the unit tests.

__ https://fmt.dev/latest/usage.html#building-the-library

Benchmarks reside in a separate repository,
`format-benchmarks <https://github.com/fmtlib/format-benchmark>`_,
so to run the benchmarks you first need to clone this repository and
generate Makefiles with CMake::

    $ git clone --recursive https://github.com/fmtlib/format-benchmark.git
    $ cd format-benchmark
    $ cmake .

Then you can run the speed test::

    $ make speed-test

or the bloat test::

    $ make bloat-test
    
Migrating code
--------------

`clang-tidy-fmt <https://github.com/mikecrowe/clang-tidy-fmt>`_ provides clang
tidy checks for converting occurrences of ``printf`` and ``fprintf`` to
``fmt::print``.

Projects using this library
---------------------------

* `0 A.D. <https://play0ad.com/>`_: a free, open-source, cross-platform
  real-time strategy game

* `2GIS <https://2gis.ru/>`_: free business listings with a city map

* `AMPL/MP <https://github.com/ampl/mp>`_:
  an open-source library for mathematical programming

* `Aseprite <https://github.com/aseprite/aseprite>`_:
  animated sprite editor & pixel art tool 

* `AvioBook <https://www.aviobook.aero/en>`_: a comprehensive aircraft
  operations suite
  
* `Blizzard Battle.net <https://battle.net/>`_: an online gaming platform
  
* `Celestia <https://celestia.space/>`_: real-time 3D visualization of space

* `Ceph <https://ceph.com/>`_: a scalable distributed storage system

* `ccache <https://ccache.dev/>`_: a compiler cache

* `ClickHouse <https://github.com/ClickHouse/ClickHouse>`_: analytical database
  management system

* `CUAUV <https://cuauv.org/>`_: Cornell University's autonomous underwater
  vehicle

* `Drake <https://drake.mit.edu/>`_: a planning, control, and analysis toolbox
  for nonlinear dynamical systems (MIT)

* `Envoy <https://lyft.github.io/envoy/>`_: C++ L7 proxy and communication bus
  (Lyft)

* `FiveM <https://fivem.net/>`_: a modification framework for GTA V

* `fmtlog <https://github.com/MengRao/fmtlog>`_: a performant fmtlib-style
  logging library with latency in nanoseconds

* `Folly <https://github.com/facebook/folly>`_: Facebook open-source library

* `Grand Mountain Adventure
  <https://store.steampowered.com/app/1247360/Grand_Mountain_Adventure/>`_:
  A beautiful open-world ski & snowboarding game

* `HarpyWar/pvpgn <https://github.com/pvpgn/pvpgn-server>`_:
  Player vs Player Gaming Network with tweaks

* `KBEngine <https://github.com/kbengine/kbengine>`_: an open-source MMOG server
  engine

* `Keypirinha <https://keypirinha.com/>`_: a semantic launcher for Windows

* `Kodi <https://kodi.tv/>`_ (formerly xbmc): home theater software

* `Knuth <https://kth.cash/>`_: high-performance Bitcoin full-node

* `Microsoft Verona <https://github.com/microsoft/verona>`_:
  research programming language for concurrent ownership

* `MongoDB <https://mongodb.com/>`_: distributed document database

* `MongoDB Smasher <https://github.com/duckie/mongo_smasher>`_: a small tool to
  generate randomized datasets

* `OpenSpace <https://openspaceproject.com/>`_: an open-source
  astrovisualization framework

* `PenUltima Online (POL) <https://www.polserver.com/>`_:
  an MMO server, compatible with most Ultima Online clients

* `PyTorch <https://github.com/pytorch/pytorch>`_: an open-source machine
  learning library

* `quasardb <https://www.quasardb.net/>`_: a distributed, high-performance,
  associative database
  
* `Quill <https://github.com/odygrd/quill>`_: asynchronous low-latency logging library

* `QKW <https://github.com/ravijanjam/qkw>`_: generalizing aliasing to simplify
  navigation, and executing complex multi-line terminal command sequences

* `redis-cerberus <https://github.com/HunanTV/redis-cerberus>`_: a Redis cluster
  proxy

* `redpanda <https://vectorized.io/redpanda>`_: a 10x faster Kafka® replacement
  for mission critical systems written in C++

* `rpclib <http://rpclib.net/>`_: a modern C++ msgpack-RPC server and client
  library

* `Salesforce Analytics Cloud
  <https://www.salesforce.com/analytics-cloud/overview/>`_:
  business intelligence software

* `Scylla <https://www.scylladb.com/>`_: a Cassandra-compatible NoSQL data store
  that can handle 1 million transactions per second on a single server

* `Seastar <http://www.seastar-project.org/>`_: an advanced, open-source C++
  framework for high-performance server applications on modern hardware

* `spdlog <https://github.com/gabime/spdlog>`_: super fast C++ logging library

* `Stellar <https://www.stellar.org/>`_: financial platform

* `Touch Surgery <https://www.touchsurgery.com/>`_: surgery simulator

* `TrinityCore <https://github.com/TrinityCore/TrinityCore>`_: open-source
  MMORPG framework

* `Windows Terminal <https://github.com/microsoft/terminal>`_: the new Windows
  terminal

`More... <https://github.com/search?q=fmtlib&type=Code>`_

If you are aware of other projects using this library, please let me know
by `email <mailto:victor.zverovich@gmail.com>`_ or by submitting an
`issue <https://github.com/fmtlib/fmt/issues>`_.

Motivation
----------

So why yet another formatting library?

There are plenty of methods for doing this task, from standard ones like
the printf family of function and iostreams to Boost Format and FastFormat
libraries. The reason for creating a new library is that every existing
solution that I found either had serious issues or didn't provide
all the features I needed.

printf
~~~~~~

The good thing about ``printf`` is that it is pretty fast and readily available
being a part of the C standard library. The main drawback is that it
doesn't support user-defined types. ``printf`` also has safety issues although
they are somewhat mitigated with `__attribute__ ((format (printf, ...))
<https://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html>`_ in GCC.
There is a POSIX extension that adds positional arguments required for
`i18n <https://en.wikipedia.org/wiki/Internationalization_and_localization>`_
to ``printf`` but it is not a part of C99 and may not be available on some
platforms.

iostreams
~~~~~~~~~

The main issue with iostreams is best illustrated with an example:

.. code:: c++

    std::cout << std::setprecision(2) << std::fixed << 1.23456 << "\n";

which is a lot of typing compared to printf:

.. code:: c++

    printf("%.2f\n", 1.23456);

Matthew Wilson, the author of FastFormat, called this "chevron hell". iostreams
don't support positional arguments by design.

The good part is that iostreams support user-defined types and are safe although
error handling is awkward.

Boost Format
~~~~~~~~~~~~

This is a very powerful library which supports both ``printf``-like format
strings and positional arguments. Its main drawback is performance. According to
various benchmarks, it is much slower than other methods considered here. Boost
Format also has excessive build times and severe code bloat issues (see
`Benchmarks`_).

FastFormat
~~~~~~~~~~

This is an interesting library which is fast, safe and has positional arguments.
However, it has significant limitations, citing its author:

    Three features that have no hope of being accommodated within the
    current design are:

    * Leading zeros (or any other non-space padding)
    * Octal/hexadecimal encoding
    * Runtime width/alignment specification

It is also quite big and has a heavy dependency, STLSoft, which might be too
restrictive for using it in some projects.

Boost Spirit.Karma
~~~~~~~~~~~~~~~~~~

This is not really a formatting library but I decided to include it here for
completeness. As iostreams, it suffers from the problem of mixing verbatim text
with arguments. The library is pretty fast, but slower on integer formatting
than ``fmt::format_to`` with format string compilation on Karma's own benchmark,
see `Converting a hundred million integers to strings per second
<http://www.zverovich.net/2020/06/13/fast-int-to-string-revisited.html>`_.

License
-------

{fmt} is distributed under the MIT `license
<https://github.com/fmtlib/fmt/blob/master/LICENSE.rst>`_.

Documentation License
---------------------

The `Format String Syntax <https://fmt.dev/latest/syntax.html>`_
section in the documentation is based on the one from Python `string module
documentation <https://docs.python.org/3/library/string.html#module-string>`_.
For this reason the documentation is distributed under the Python Software
Foundation license available in `doc/python-license.txt
<https://raw.github.com/fmtlib/fmt/master/doc/python-license.txt>`_.
It only applies if you distribute the documentation of {fmt}.

Maintainers
-----------

The {fmt} library is maintained by Victor Zverovich (`vitaut
<https://github.com/vitaut>`_) and Jonathan Müller (`foonathan
<https://github.com/foonathan>`_) with contributions from many other people.
See `Contributors <https://github.com/fmtlib/fmt/graphs/contributors>`_ and
`Releases <https://github.com/fmtlib/fmt/releases>`_ for some of the names.
Let us know if your contribution is not listed or mentioned incorrectly and
we'll make it right.
