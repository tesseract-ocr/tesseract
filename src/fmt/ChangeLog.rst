8.0.1 - 2021-07-02
------------------

* Fixed the version number in the inline namespace
  (`#2374 <https://github.com/fmtlib/fmt/issues/2374>`_).

* Added a missing presentation type check for ``std::string``
  (`#2402 <https://github.com/fmtlib/fmt/issues/2402>`_).

* Fixed a linkage error when mixing code built with clang and gcc
  (`#2377 <https://github.com/fmtlib/fmt/issues/2377>`_).

* Fixed documentation issues
  (`#2396 <https://github.com/fmtlib/fmt/pull/2396>`_,
  `#2403 <https://github.com/fmtlib/fmt/issues/2403>`_,
  `#2406 <https://github.com/fmtlib/fmt/issues/2406>`_).
  Thanks `@mkurdej (Marek Kurdej) <https://github.com/mkurdej>`_.

* Removed dead code in FP formatter (
  `#2398 <https://github.com/fmtlib/fmt/pull/2398>`_).
  Thanks `@javierhonduco (Javier Honduvilla Coto)
  <https://github.com/javierhonduco>`_.

* Fixed various warnings and compilation issues
  (`#2351 <https://github.com/fmtlib/fmt/issues/2351>`_,
  `#2359 <https://github.com/fmtlib/fmt/issues/2359>`_,
  `#2365 <https://github.com/fmtlib/fmt/pull/2365>`_,
  `#2368 <https://github.com/fmtlib/fmt/issues/2368>`_,
  `#2370 <https://github.com/fmtlib/fmt/pull/2370>`_,
  `#2376 <https://github.com/fmtlib/fmt/pull/2376>`_,
  `#2381 <https://github.com/fmtlib/fmt/pull/2381>`_,
  `#2382 <https://github.com/fmtlib/fmt/pull/2382>`_,
  `#2386 <https://github.com/fmtlib/fmt/issues/2386>`_,
  `#2389 <https://github.com/fmtlib/fmt/pull/2389>`_,
  `#2395 <https://github.com/fmtlib/fmt/pull/2395>`_,
  `#2397 <https://github.com/fmtlib/fmt/pull/2397>`_,
  `#2400 <https://github.com/fmtlib/fmt/issues/2400>`_
  `#2401 <https://github.com/fmtlib/fmt/issues/2401>`_,
  `#2407 <https://github.com/fmtlib/fmt/pull/2407>`_).
  Thanks `@zx2c4 (Jason A. Donenfeld) <https://github.com/zx2c4>`_,
  `@AidanSun05 (Aidan Sun) <https://github.com/AidanSun05>`_,
  `@mattiasljungstrom (Mattias Ljungström)
  <https://github.com/mattiasljungstrom>`_,
  `@joemmett (Jonathan Emmett) <https://github.com/joemmett>`_,
  `@erengy (Eren Okka) <https://github.com/erengy>`_,
  `@patlkli (Patrick Geltinger) <https://github.com/patlkli>`_,
  `@gsjaardema (Greg Sjaardema) <https://github.com/gsjaardema>`_,
  `@phprus (Vladislav Shchapov) <https://github.com/phprus>`_.

8.0.0 - 2021-06-21
------------------

* Enabled compile-time format string check by default.
  For example (`godbolt <https://godbolt.org/z/sMxcohGjz>`__):

  .. code:: c++

     #include <fmt/core.h>

     int main() {
       fmt::print("{:d}", "I am not a number");
     }

  gives a compile-time error on compilers with C++20 ``consteval`` support
  (gcc 10+, clang 11+) because ``d`` is not a valid format specifier for a
  string.

  To pass a runtime string wrap it in ``fmt::runtime``:

  .. code:: c++

     fmt::print(fmt::runtime("{:d}"), "I am not a number");

* Added compile-time formatting
  (`#2019 <https://github.com/fmtlib/fmt/pull/2019>`_,
  `#2044 <https://github.com/fmtlib/fmt/pull/2044>`_,
  `#2056 <https://github.com/fmtlib/fmt/pull/2056>`_,
  `#2072 <https://github.com/fmtlib/fmt/pull/2072>`_,
  `#2075 <https://github.com/fmtlib/fmt/pull/2075>`_,
  `#2078 <https://github.com/fmtlib/fmt/issues/2078>`_,
  `#2129 <https://github.com/fmtlib/fmt/pull/2129>`_,
  `#2326 <https://github.com/fmtlib/fmt/pull/2326>`_).
  For example (`godbolt <https://godbolt.org/z/Mxx9d89jM>`__):

  .. code:: c++

     #include <fmt/compile.h>

     consteval auto compile_time_itoa(int value) -> std::array<char, 10> {
       auto result = std::array<char, 10>();
       fmt::format_to(result.data(), FMT_COMPILE("{}"), value);
       return result;
     }

     constexpr auto answer = compile_time_itoa(42);

  Most of the formatting functionality is available at compile time with a
  notable exception of floating-point numbers and pointers.
  Thanks `@alexezeder (Alexey Ochapov) <https://github.com/alexezeder>`_.

* Optimized handling of format specifiers during format string compilation.
  For example, hexadecimal formatting (``"{:x}"``) is now 3-7x faster than
  before when using ``format_to`` with format string compilation and a
  stack-allocated buffer (`#1944 <https://github.com/fmtlib/fmt/issues/1944>`_).

  Before (7.1.3)::

    ----------------------------------------------------------------------------
    Benchmark                                  Time             CPU   Iterations
    ----------------------------------------------------------------------------
    FMTCompileOld/0                         15.5 ns         15.5 ns     43302898
    FMTCompileOld/42                        16.6 ns         16.6 ns     43278267
    FMTCompileOld/273123                    18.7 ns         18.6 ns     37035861
    FMTCompileOld/9223372036854775807       19.4 ns         19.4 ns     35243000
    ----------------------------------------------------------------------------

  After (8.x)::

    ----------------------------------------------------------------------------
    Benchmark                                  Time             CPU   Iterations
    ----------------------------------------------------------------------------
    FMTCompileNew/0                         1.99 ns         1.99 ns    360523686
    FMTCompileNew/42                        2.33 ns         2.33 ns    279865664
    FMTCompileNew/273123                    3.72 ns         3.71 ns    190230315
    FMTCompileNew/9223372036854775807       5.28 ns         5.26 ns    130711631
    ----------------------------------------------------------------------------

  It is even faster than ``std::to_chars`` from libc++ compiled with clang on
  macOS::

    ----------------------------------------------------------------------------
    Benchmark                                  Time             CPU   Iterations
    ----------------------------------------------------------------------------
    ToChars/0                               4.42 ns         4.41 ns    160196630
    ToChars/42                              5.00 ns         4.98 ns    140735201
    ToChars/273123                          7.26 ns         7.24 ns     95784130
    ToChars/9223372036854775807             8.77 ns         8.75 ns     75872534
    ----------------------------------------------------------------------------

  In other cases, especially involving ``std::string`` construction, the
  speed up is usually lower because handling format specifiers takes a smaller
  fraction of the total time.

* Added the ``_cf`` user-defined literal to represent a compiled format string.
  It can be used instead of the ``FMT_COMPILE`` macro
  (`#2043 <https://github.com/fmtlib/fmt/pull/2043>`_,
  `#2242 <https://github.com/fmtlib/fmt/pull/2242>`_):

  .. code:: c++

     #include <fmt/compile.h>

     using namespace fmt::literals;
     auto s = fmt::format(FMT_COMPILE("{}"), 42); // 🙁 not modern
     auto s = fmt::format("{}"_cf, 42);           // 🙂 modern as hell

  It requires compiler support for class types in non-type template parameters
  (a C++20 feature) which is available in GCC 9.3+.
  Thanks `@alexezeder (Alexey Ochapov) <https://github.com/alexezeder>`_.

* Format string compilation now requires ``format`` functions of ``formatter``
  specializations for user-defined types to be ``const``:

  .. code:: c++

     template <> struct fmt::formatter<my_type>: formatter<string_view> {
       template <typename FormatContext>
       auto format(my_type obj, FormatContext& ctx) const {  // Note const here.
         // ...
       }
     };

* Added UDL-based named argument support to format string compilation
  (`#2243 <https://github.com/fmtlib/fmt/pull/2243>`_,
  `#2281 <https://github.com/fmtlib/fmt/pull/2281>`_). For example:

  .. code:: c++

     #include <fmt/compile.h>

     using namespace fmt::literals;
     auto s = fmt::format(FMT_COMPILE("{answer}"), "answer"_a = 42);

  Here the argument named "answer" is resolved at compile time with no
  runtime overhead.
  Thanks `@alexezeder (Alexey Ochapov) <https://github.com/alexezeder>`_.

* Added format string compilation support to ``fmt::print``
  (`#2280 <https://github.com/fmtlib/fmt/issues/2280>`_,
  `#2304 <https://github.com/fmtlib/fmt/pull/2304>`_).
  Thanks `@alexezeder (Alexey Ochapov) <https://github.com/alexezeder>`_.

* Added initial support for compiling {fmt} as a C++20 module
  (`#2235 <https://github.com/fmtlib/fmt/pull/2235>`_,
  `#2240 <https://github.com/fmtlib/fmt/pull/2240>`_,
  `#2260 <https://github.com/fmtlib/fmt/pull/2260>`_,
  `#2282 <https://github.com/fmtlib/fmt/pull/2282>`_,
  `#2283 <https://github.com/fmtlib/fmt/pull/2283>`_,
  `#2288 <https://github.com/fmtlib/fmt/pull/2288>`_,
  `#2298 <https://github.com/fmtlib/fmt/pull/2298>`_,
  `#2306 <https://github.com/fmtlib/fmt/pull/2306>`_,
  `#2307 <https://github.com/fmtlib/fmt/pull/2307>`_,
  `#2309 <https://github.com/fmtlib/fmt/pull/2309>`_,
  `#2318 <https://github.com/fmtlib/fmt/pull/2318>`_,
  `#2324 <https://github.com/fmtlib/fmt/pull/2324>`_,
  `#2332 <https://github.com/fmtlib/fmt/pull/2332>`_,
  `#2340 <https://github.com/fmtlib/fmt/pull/2340>`_).
  Thanks `@DanielaE (Daniela Engert) <https://github.com/DanielaE>`_.

* Made symbols private by default reducing shared library size
  (`#2301 <https://github.com/fmtlib/fmt/pull/2301>`_). For example there was
  a ~15% reported reduction on one platform.
  Thanks `@sergiud (Sergiu Deitsch) <https://github.com/sergiud>`_.

* Optimized includes making the result of preprocessing ``fmt/format.h``
  ~20% smaller with libstdc++/C++20 and slightly improving build times
  (`#1998 <https://github.com/fmtlib/fmt/issues/1998>`_).

* Added support of ranges with non-const ``begin`` / ``end``
  (`#1953 <https://github.com/fmtlib/fmt/pull/1953>`_).
  Thanks `@kitegi (sarah) <https://github.com/kitegi>`_.

* Added support of ``std::byte`` and other formattable types to ``fmt::join``
  (`#1981 <https://github.com/fmtlib/fmt/issues/1981>`_,
  `#2040 <https://github.com/fmtlib/fmt/issues/2040>`_,
  `#2050 <https://github.com/fmtlib/fmt/pull/2050>`_,
  `#2262 <https://github.com/fmtlib/fmt/issues/2262>`_). For example:

  .. code:: c++

     #include <fmt/format.h>
     #include <cstddef>
     #include <vector>

     int main() {
       auto bytes = std::vector{std::byte(4), std::byte(2)};
       fmt::print("{}", fmt::join(bytes, ""));
     }

  prints "42".

  Thanks `@kamibo (Camille Bordignon) <https://github.com/kamibo>`_.

* Implemented the default format for ``std::chrono::system_clock``
  (`#2319 <https://github.com/fmtlib/fmt/issues/2319>`_,
  `#2345 <https://github.com/fmtlib/fmt/pull/2345>`_). For example:

  .. code:: c++

     #include <fmt/chrono.h>

     int main() {
       fmt::print("{}", std::chrono::system_clock::now());
     }

  prints "2021-06-18 15:22:00" (the output depends on the current date and
  time). Thanks `@sunmy2019 <https://github.com/sunmy2019>`_.

* Made more chrono specifiers locale independent by default. Use the ``'L'``
  specifier to get localized formatting. For example:

  .. code:: c++

     #include <fmt/chrono.h>

     int main() {
       std::locale::global(std::locale("ru_RU.UTF-8"));
       auto monday = std::chrono::weekday(1);
       fmt::print("{}\n", monday);   // prints "Mon"
       fmt::print("{:L}\n", monday); // prints "пн"
     }

* Improved locale handling in chrono formatting
  (`#2337 <https://github.com/fmtlib/fmt/issues/2337>`_,
  `#2349 <https://github.com/fmtlib/fmt/pull/2349>`_,
  `#2350 <https://github.com/fmtlib/fmt/pull/2350>`_).
  Thanks `@phprus (Vladislav Shchapov) <https://github.com/phprus>`_.

* Deprecated ``fmt/locale.h`` moving the formatting functions that take a
  locale to ``fmt/format.h`` (``char``) and ``fmt/xchar`` (other overloads).
  This doesn't introduce a dependency on ``<locale>`` so there is virtually no
  compile time effect.

* Deprecated an undocumented ``format_to`` overload that takes
  ``basic_memory_buffer``.

* Made parameter order in ``vformat_to`` consistent with ``format_to``
  (`#2327 <https://github.com/fmtlib/fmt/issues/2327>`_).

* Added support for time points with arbitrary durations
  (`#2208 <https://github.com/fmtlib/fmt/issues/2208>`_). For example:

  .. code:: c++

     #include <fmt/chrono.h>

     int main() {
       using tp = std::chrono::time_point<
         std::chrono::system_clock, std::chrono::seconds>;
       fmt::print("{:%S}", tp(std::chrono::seconds(42)));
     }

  prints "42".

* Formatting floating-point numbers no longer produces trailing zeros by default
  for consistency with ``std::format``. For example:

  .. code:: c++

     #include <fmt/core.h>

     int main() {
       fmt::print("{0:.3}", 1.1);
     }

  prints "1.1". Use the ``'#'`` specifier to keep trailing zeros.

* Dropped a limit on the number of elements in a range and replaced ``{}`` with
  ``[]`` as range delimiters for consistency with Python's ``str.format``.

* The ``'L'`` specifier for locale-specific numeric formatting can now be
  combined with presentation specifiers as in ``std::format``. For example:

  .. code:: c++

     #include <fmt/core.h>
     #include <locale>

     int main() {
       std::locale::global(std::locale("fr_FR.UTF-8"));
       fmt::print("{0:.2Lf}", 0.42);
     }

  prints "0,42". The deprecated ``'n'`` specifier has been removed.

* Made the ``0`` specifier ignored for infinity and NaN
  (`#2305 <https://github.com/fmtlib/fmt/issues/2305>`_,
  `#2310 <https://github.com/fmtlib/fmt/pull/2310>`_).
  Thanks `@Liedtke (Matthias Liedtke) <https://github.com/Liedtke>`_.

* Made the hexfloat formatting use the right alignment by default
  (`#2308 <https://github.com/fmtlib/fmt/issues/2308>`_,
  `#2317 <https://github.com/fmtlib/fmt/pull/2317>`_).
  Thanks `@Liedtke (Matthias Liedtke) <https://github.com/Liedtke>`_.

* Removed the deprecated numeric alignment (``'='``). Use the ``'0'`` specifier
  instead.

* Removed the deprecated ``fmt/posix.h`` header that has been replaced with
  ``fmt/os.h``.

* Removed the deprecated ``format_to_n_context``, ``format_to_n_args`` and
  ``make_format_to_n_args``. They have been replaced with ``format_context``,
  ``format_args` and ``make_format_args`` respectively.

* Moved ``wchar_t``-specific functions and types to ``fmt/xchar.h``.
  You can define ``FMT_DEPRECATED_INCLUDE_XCHAR`` to automatically include
  ``fmt/xchar.h`` from ``fmt/format.h`` but this will be disabled in the next
  major release.

* Fixed handling of the ``'+'`` specifier in localized formatting
  (`#2133 <https://github.com/fmtlib/fmt/issues/2133>`_).

* Added support for the ``'s'`` format specifier that gives textual
  representation of ``bool``
  (`#2094 <https://github.com/fmtlib/fmt/issues/2094>`_,
  `#2109 <https://github.com/fmtlib/fmt/pull/2109>`_). For example:

  .. code:: c++

     #include <fmt/core.h>

     int main() {
       fmt::print("{:s}", true);
     }

  prints "true".
  Thanks `@powercoderlol (Ivan Polyakov) <https://github.com/powercoderlol>`_.

* Made ``fmt::ptr`` work with function pointers
  (`#2131 <https://github.com/fmtlib/fmt/pull/2131>`_). For example:

  .. code:: c++

     #include <fmt/format.h>

     int main() {
       fmt::print("My main: {}\n", fmt::ptr(main));
     }

  Thanks `@mikecrowe (Mike Crowe) <https://github.com/mikecrowe>`_.

* The undocumented support for specializing ``formatter`` for pointer types
  has been removed.

* Fixed ``fmt::formatted_size`` with format string compilation
  (`#2141 <https://github.com/fmtlib/fmt/pull/2141>`_,
  `#2161 <https://github.com/fmtlib/fmt/pull/2161>`_).
  Thanks `@alexezeder (Alexey Ochapov) <https://github.com/alexezeder>`_.

* Fixed handling of empty format strings during format string compilation
  (`#2042 <https://github.com/fmtlib/fmt/issues/2042>`_):

  .. code:: c++

     auto s = fmt::format(FMT_COMPILE(""));

  Thanks `@alexezeder (Alexey Ochapov) <https://github.com/alexezeder>`_.

* Fixed handling of enums in ``fmt::to_string``
  (`#2036 <https://github.com/fmtlib/fmt/issues/2036>`_).

* Improved width computation
  (`#2033 <https://github.com/fmtlib/fmt/issues/2033>`_,
  `#2091 <https://github.com/fmtlib/fmt/issues/2091>`_). For example:

  .. code:: c++

     #include <fmt/core.h>

     int main() {
       fmt::print("{:-<10}{}\n", "你好", "世界");
       fmt::print("{:-<10}{}\n", "hello", "world");
     }

  prints

  .. image:: https://user-images.githubusercontent.com/576385/
             119840373-cea3ca80-beb9-11eb-91e0-54266c48e181.png

  on a modern terminal.

* The experimental fast output stream (``fmt::ostream``) is now truncated by
  default for consistency with ``fopen``
  (`#2018 <https://github.com/fmtlib/fmt/issues/2018>`_). For example:

  .. code:: c++

     #include <fmt/os.h>

     int main() {
       fmt::ostream out1 = fmt::output_file("guide");
       out1.print("Zaphod");
       out1.close();
       fmt::ostream out2 = fmt::output_file("guide");
       out2.print("Ford");
     }

  writes "Ford" to the file "guide". To preserve the old file content if any
  pass ``fmt::file::WRONLY | fmt::file::CREATE`` flags to ``fmt::output_file``.

* Fixed moving of ``fmt::ostream`` that holds buffered data
  (`#2197 <https://github.com/fmtlib/fmt/issues/2197>`_,
  `#2198 <https://github.com/fmtlib/fmt/pull/2198>`_).
  Thanks `@vtta <https://github.com/vtta>`_.

* Replaced the ``fmt::system_error`` exception with a function of the same
  name that constructs ``std::system_error``
  (`#2266 <https://github.com/fmtlib/fmt/issues/2266>`_).

* Replaced the ``fmt::windows_error`` exception with a function of the same
  name that constructs ``std::system_error`` with the category returned by
  ``fmt::system_category()``
  (`#2274 <https://github.com/fmtlib/fmt/issues/2274>`_,
  `#2275 <https://github.com/fmtlib/fmt/pull/2275>`_).
  The latter is similar to ``std::sytem_category`` but correctly handles UTF-8.
  Thanks `@phprus (Vladislav Shchapov) <https://github.com/phprus>`_.

* Replaced ``fmt::error_code`` with ``std::error_code`` and made it formattable
  (`#2269 <https://github.com/fmtlib/fmt/issues/2269>`_,
  `#2270 <https://github.com/fmtlib/fmt/pull/2270>`_,
  `#2273 <https://github.com/fmtlib/fmt/pull/2273>`_).
  Thanks `@phprus (Vladislav Shchapov) <https://github.com/phprus>`_.
 
* Added speech synthesis support
  (`#2206 <https://github.com/fmtlib/fmt/pull/2206>`_).

* Made ``format_to`` work with a memory buffer that has a custom allocator
  (`#2300 <https://github.com/fmtlib/fmt/pull/2300>`_).
  Thanks `@voxmea <https://github.com/voxmea>`_.

* Added ``Allocator::max_size`` support to ``basic_memory_buffer``.
  (`#1960 <https://github.com/fmtlib/fmt/pull/1960>`_).
  Thanks `@phprus (Vladislav Shchapov) <https://github.com/phprus>`_.

* Added wide string support to ``fmt::join``
  (`#2236 <https://github.com/fmtlib/fmt/pull/2236>`_).
  Thanks `@crbrz <https://github.com/crbrz>`_.

* Made iterators passed to ``formatter`` specializations via a format context
  satisfy C++20 ``std::output_iterator`` requirements
  (`#2156 <https://github.com/fmtlib/fmt/issues/2156>`_,
  `#2158 <https://github.com/fmtlib/fmt/pull/2158>`_,
  `#2195 <https://github.com/fmtlib/fmt/issues/2195>`_,
  `#2204 <https://github.com/fmtlib/fmt/pull/2204>`_).
  Thanks `@randomnetcat (Jason Cobb) <https://github.com/randomnetcat>`_.

* Optimized the ``printf`` implementation
  (`#1982 <https://github.com/fmtlib/fmt/pull/1982>`_,
  `#1984 <https://github.com/fmtlib/fmt/pull/1984>`_,
  `#2016 <https://github.com/fmtlib/fmt/pull/2016>`_,
  `#2164 <https://github.com/fmtlib/fmt/pull/2164>`_).
  Thanks `@rimathia <https://github.com/rimathia>`_ and
  `@moiwi <https://github.com/moiwi>`_.

* Improved detection of ``constexpr`` ``char_traits``
  (`#2246 <https://github.com/fmtlib/fmt/pull/2246>`_,
  `#2257 <https://github.com/fmtlib/fmt/pull/2257>`_).
  Thanks `@phprus (Vladislav Shchapov) <https://github.com/phprus>`_.

* Fixed writing to ``stdout`` when it is redirected to ``NUL`` on Windows
  (`#2080 <https://github.com/fmtlib/fmt/issues/2080>`_).

* Fixed exception propagation from iterators
  (`#2097 <https://github.com/fmtlib/fmt/issues/2097>`_).
  
* Improved ``strftime`` error handling 
  (`#2238 <https://github.com/fmtlib/fmt/issues/2238>`_,
  `#2244 <https://github.com/fmtlib/fmt/pull/2244>`_).
  Thanks `@yumeyao <https://github.com/yumeyao>`_.

* Stopped using deprecated GCC UDL template extension.

* Added ``fmt/args.h`` to the install target
  (`#2096 <https://github.com/fmtlib/fmt/issues/2096>`_).

* Error messages are now passed to assert when exceptions are disabled
  (`#2145 <https://github.com/fmtlib/fmt/pull/2145>`_).
  Thanks `@NobodyXu (Jiahao XU) <https://github.com/NobodyXu>`_.

* Added the ``FMT_MASTER_PROJECT`` CMake option to control build and install
  targets when {fmt} is included via ``add_subdirectory``
  (`#2098 <https://github.com/fmtlib/fmt/issues/2098>`_,
  `#2100 <https://github.com/fmtlib/fmt/pull/2100>`_).
  Thanks `@randomizedthinking <https://github.com/randomizedthinking>`_.

* Improved build configuration
  (`#2026 <https://github.com/fmtlib/fmt/pull/2026>`_,
  `#2122 <https://github.com/fmtlib/fmt/pull/2122>`_).
  Thanks `@luncliff (Park DongHa) <https://github.com/luncliff>`_ and
  `@ibaned (Dan Ibanez) <https://github.com/ibaned>`_.

* Fixed various warnings and compilation issues
  (`#1947 <https://github.com/fmtlib/fmt/issues/1947>`_,
  `#1959 <https://github.com/fmtlib/fmt/pull/1959>`_,
  `#1963 <https://github.com/fmtlib/fmt/pull/1963>`_,
  `#1965 <https://github.com/fmtlib/fmt/pull/1965>`_,
  `#1966 <https://github.com/fmtlib/fmt/issues/1966>`_,
  `#1974 <https://github.com/fmtlib/fmt/pull/1974>`_,
  `#1975 <https://github.com/fmtlib/fmt/pull/1975>`_,
  `#1990 <https://github.com/fmtlib/fmt/pull/1990>`_,
  `#2000 <https://github.com/fmtlib/fmt/issues/2000>`_,
  `#2001 <https://github.com/fmtlib/fmt/pull/2001>`_,
  `#2002 <https://github.com/fmtlib/fmt/issues/2002>`_,
  `#2004 <https://github.com/fmtlib/fmt/issues/2004>`_,
  `#2006 <https://github.com/fmtlib/fmt/pull/2006>`_,
  `#2009 <https://github.com/fmtlib/fmt/pull/2009>`_,
  `#2010 <https://github.com/fmtlib/fmt/pull/2010>`_,
  `#2038 <https://github.com/fmtlib/fmt/issues/2038>`_,
  `#2039 <https://github.com/fmtlib/fmt/issues/2039>`_,
  `#2047 <https://github.com/fmtlib/fmt/issues/2047>`_,
  `#2053 <https://github.com/fmtlib/fmt/pull/2053>`_,
  `#2059 <https://github.com/fmtlib/fmt/issues/2059>`_,
  `#2065 <https://github.com/fmtlib/fmt/pull/2065>`_,
  `#2067 <https://github.com/fmtlib/fmt/pull/2067>`_,
  `#2068 <https://github.com/fmtlib/fmt/pull/2068>`_,
  `#2073 <https://github.com/fmtlib/fmt/pull/2073>`_,
  `#2103 <https://github.com/fmtlib/fmt/issues/2103>`_
  `#2105 <https://github.com/fmtlib/fmt/issues/2105>`_
  `#2106 <https://github.com/fmtlib/fmt/pull/2106>`_,
  `#2107 <https://github.com/fmtlib/fmt/pull/2107>`_,
  `#2116 <https://github.com/fmtlib/fmt/issues/2116>`_
  `#2117 <https://github.com/fmtlib/fmt/pull/2117>`_,
  `#2118 <https://github.com/fmtlib/fmt/issues/2118>`_
  `#2119 <https://github.com/fmtlib/fmt/pull/2119>`_,
  `#2127 <https://github.com/fmtlib/fmt/issues/2127>`_,
  `#2128 <https://github.com/fmtlib/fmt/pull/2128>`_,
  `#2140 <https://github.com/fmtlib/fmt/issues/2140>`_,
  `#2142 <https://github.com/fmtlib/fmt/issues/2142>`_,
  `#2143 <https://github.com/fmtlib/fmt/pull/2143>`_,
  `#2144 <https://github.com/fmtlib/fmt/pull/2144>`_,
  `#2147 <https://github.com/fmtlib/fmt/issues/2147>`_,
  `#2148 <https://github.com/fmtlib/fmt/issues/2148>`_,
  `#2149 <https://github.com/fmtlib/fmt/issues/2149>`_,
  `#2152 <https://github.com/fmtlib/fmt/pull/2152>`_,
  `#2160 <https://github.com/fmtlib/fmt/pull/2160>`_,
  `#2170 <https://github.com/fmtlib/fmt/issues/2170>`_,
  `#2175 <https://github.com/fmtlib/fmt/issues/2175>`_,
  `#2176 <https://github.com/fmtlib/fmt/issues/2176>`_,
  `#2177 <https://github.com/fmtlib/fmt/pull/2177>`_,
  `#2178 <https://github.com/fmtlib/fmt/issues/2178>`_,
  `#2179 <https://github.com/fmtlib/fmt/pull/2179>`_,
  `#2180 <https://github.com/fmtlib/fmt/issues/2180>`_,
  `#2181 <https://github.com/fmtlib/fmt/issues/2181>`_,
  `#2183 <https://github.com/fmtlib/fmt/pull/2183>`_,
  `#2184 <https://github.com/fmtlib/fmt/issues/2184>`_,
  `#2185 <https://github.com/fmtlib/fmt/issues/2185>`_,
  `#2186 <https://github.com/fmtlib/fmt/pull/2186>`_,
  `#2187 <https://github.com/fmtlib/fmt/pull/2187>`_,
  `#2190 <https://github.com/fmtlib/fmt/pull/2190>`_,
  `#2192 <https://github.com/fmtlib/fmt/pull/2192>`_,
  `#2194 <https://github.com/fmtlib/fmt/pull/2194>`_,
  `#2205 <https://github.com/fmtlib/fmt/pull/2205>`_,
  `#2210 <https://github.com/fmtlib/fmt/issues/2210>`_,
  `#2211 <https://github.com/fmtlib/fmt/pull/2211>`_,
  `#2215 <https://github.com/fmtlib/fmt/pull/2215>`_,
  `#2216 <https://github.com/fmtlib/fmt/pull/2216>`_,
  `#2218 <https://github.com/fmtlib/fmt/pull/2218>`_,
  `#2220 <https://github.com/fmtlib/fmt/pull/2220>`_,
  `#2228 <https://github.com/fmtlib/fmt/issues/2228>`_,
  `#2229 <https://github.com/fmtlib/fmt/pull/2229>`_,
  `#2230 <https://github.com/fmtlib/fmt/pull/2230>`_,
  `#2233 <https://github.com/fmtlib/fmt/issues/2233>`_,
  `#2239 <https://github.com/fmtlib/fmt/pull/2239>`_,
  `#2248 <https://github.com/fmtlib/fmt/issues/2248>`_,
  `#2252 <https://github.com/fmtlib/fmt/issues/2252>`_,
  `#2253 <https://github.com/fmtlib/fmt/pull/2253>`_,
  `#2255 <https://github.com/fmtlib/fmt/pull/2255>`_,
  `#2261 <https://github.com/fmtlib/fmt/issues/2261>`_,
  `#2278 <https://github.com/fmtlib/fmt/issues/2278>`_,
  `#2284 <https://github.com/fmtlib/fmt/issues/2284>`_,
  `#2287 <https://github.com/fmtlib/fmt/pull/2287>`_,
  `#2289 <https://github.com/fmtlib/fmt/pull/2289>`_,
  `#2290 <https://github.com/fmtlib/fmt/pull/2290>`_,
  `#2293 <https://github.com/fmtlib/fmt/pull/2293>`_,
  `#2295 <https://github.com/fmtlib/fmt/issues/2295>`_,
  `#2296 <https://github.com/fmtlib/fmt/pull/2296>`_,
  `#2297 <https://github.com/fmtlib/fmt/pull/2297>`_,
  `#2311 <https://github.com/fmtlib/fmt/issues/2311>`_,
  `#2313 <https://github.com/fmtlib/fmt/pull/2313>`_,
  `#2315 <https://github.com/fmtlib/fmt/pull/2315>`_,
  `#2320 <https://github.com/fmtlib/fmt/issues/2320>`_,
  `#2321 <https://github.com/fmtlib/fmt/pull/2321>`_,
  `#2323 <https://github.com/fmtlib/fmt/pull/2323>`_,
  `#2328 <https://github.com/fmtlib/fmt/issues/2328>`_,
  `#2329 <https://github.com/fmtlib/fmt/pull/2329>`_,
  `#2333 <https://github.com/fmtlib/fmt/pull/2333>`_,
  `#2338 <https://github.com/fmtlib/fmt/pull/2338>`_,
  `#2341 <https://github.com/fmtlib/fmt/pull/2341>`_).
  Thanks `@darklukee <https://github.com/darklukee>`_,
  `@fagg (Ashton Fagg) <https://github.com/fagg>`_,
  `@killerbot242 (Lieven de Cock) <https://github.com/killerbot242>`_,
  `@jgopel (Jonathan Gopel) <https://github.com/jgopel>`_,
  `@yeswalrus (Walter Gray) <https://github.com/yeswalrus>`_,
  `@Finkman <https://github.com/Finkman>`_,
  `@HazardyKnusperkeks (Björn Schäpers) <https://github.com/HazardyKnusperkeks>`_,
  `@dkavolis (Daumantas Kavolis) <https://github.com/dkavolis>`_
  `@concatime (Issam Maghni) <https://github.com/concatime>`_,
  `@chronoxor (Ivan Shynkarenka) <https://github.com/chronoxor>`_,
  `@summivox (Yin Zhong) <https://github.com/summivox>`_,
  `@yNeo <https://github.com/yNeo>`_,
  `@Apache-HB (Elliot) <https://github.com/Apache-HB>`_,
  `@alexezeder (Alexey Ochapov) <https://github.com/alexezeder>`_,
  `@toojays (John Steele Scott) <https://github.com/toojays>`_,
  `@Brainy0207 <https://github.com/Brainy0207>`_,
  `@vadz (VZ) <https://github.com/vadz>`_,
  `@imsherlock (Ryan Sherlock) <https://github.com/imsherlock>`_,
  `@phprus (Vladislav Shchapov) <https://github.com/phprus>`_,
  `@white238 (Chris White) <https://github.com/white238>`_,
  `@yafshar (Yaser Afshar) <https://github.com/yafshar>`_,
  `@BillyDonahue (Billy Donahue) <https://github.com/BillyDonahue>`_,
  `@jstaahl <https://github.com/jstaahl>`_,
  `@denchat <https://github.com/denchat>`_,
  `@DanielaE (Daniela Engert) <https://github.com/DanielaE>`_,
  `@ilyakurdyukov (Ilya Kurdyukov) <https://github.com/ilyakurdyukov>`_,
  `@ilmai <https://github.com/ilmai>`_,
  `@JessyDL (Jessy De Lannoit) <https://github.com/JessyDL>`_,
  `@sergiud (Sergiu Deitsch) <https://github.com/sergiud>`_,
  `@mwinterb <https://github.com/mwinterb>`_,
  `@sven-herrmann <https://github.com/sven-herrmann>`_,
  `@jmelas (John Melas) <https://github.com/jmelas>`_,
  `@twoixter (Jose Miguel Pérez) <https://github.com/twoixter>`_,
  `@crbrz <https://github.com/crbrz>`_,
  `@upsj (Tobias Ribizel) <https://github.com/upsj>`_.

* Improved documentation
  (`#1986 <https://github.com/fmtlib/fmt/issues/1986>`_,
  `#2051 <https://github.com/fmtlib/fmt/pull/2051>`_,
  `#2057 <https://github.com/fmtlib/fmt/issues/2057>`_,
  `#2081 <https://github.com/fmtlib/fmt/pull/2081>`_,
  `#2084 <https://github.com/fmtlib/fmt/issues/2084>`_,
  `#2312 <https://github.com/fmtlib/fmt/pull/2312>`_).
  Thanks `@imba-tjd (谭九鼎) <https://github.com/imba-tjd>`_,
  `@0x416c69 (AlιAѕѕaѕѕιN) <https://github.com/0x416c69>`_,
  `@mordante <https://github.com/mordante>`_.

* Continuous integration and test improvements
  (`#1969 <https://github.com/fmtlib/fmt/issues/1969>`_,
  `#1991 <https://github.com/fmtlib/fmt/pull/1991>`_,
  `#2020 <https://github.com/fmtlib/fmt/pull/2020>`_,
  `#2110 <https://github.com/fmtlib/fmt/pull/2110>`_,
  `#2114 <https://github.com/fmtlib/fmt/pull/2114>`_,
  `#2196 <https://github.com/fmtlib/fmt/issues/2196>`_,
  `#2217 <https://github.com/fmtlib/fmt/pull/2217>`_,
  `#2247 <https://github.com/fmtlib/fmt/pull/2247>`_,
  `#2256 <https://github.com/fmtlib/fmt/pull/2256>`_,
  `#2336 <https://github.com/fmtlib/fmt/pull/2336>`_,
  `#2346 <https://github.com/fmtlib/fmt/pull/2346>`_).
  Thanks `@jgopel (Jonathan Gopel) <https://github.com/jgopel>`_,
  `@alexezeder (Alexey Ochapov) <https://github.com/alexezeder>`_ and
  `@DanielaE (Daniela Engert) <https://github.com/DanielaE>`_.
  
7.1.3 - 2020-11-24
------------------

* Fixed handling of buffer boundaries in ``format_to_n``
  (`#1996 <https://github.com/fmtlib/fmt/issues/1996>`_,
  `#2029 <https://github.com/fmtlib/fmt/issues/2029>`_).

* Fixed linkage errors when linking with a shared library
  (`#2011 <https://github.com/fmtlib/fmt/issues/2011>`_).

* Reintroduced ostream support to range formatters
  (`#2014 <https://github.com/fmtlib/fmt/issues/2014>`_).

* Worked around an issue with mixing std versions in gcc
  (`#2017 <https://github.com/fmtlib/fmt/issues/2017>`_).

7.1.2 - 2020-11-04
------------------

* Fixed floating point formatting with large precision
  (`#1976 <https://github.com/fmtlib/fmt/issues/1976>`_).

7.1.1 - 2020-11-01
------------------

* Fixed ABI compatibility with 7.0.x
  (`#1961 <https://github.com/fmtlib/fmt/issues/1961>`_).

* Added the ``FMT_ARM_ABI_COMPATIBILITY`` macro to work around ABI
  incompatibility between GCC and Clang on ARM
  (`#1919 <https://github.com/fmtlib/fmt/issues/1919>`_).

* Worked around a SFINAE bug in GCC 8
  (`#1957 <https://github.com/fmtlib/fmt/issues/1957>`_).

* Fixed linkage errors when building with GCC's LTO
  (`#1955 <https://github.com/fmtlib/fmt/issues/1955>`_).

* Fixed a compilation error when building without ``__builtin_clz`` or equivalent
  (`#1968 <https://github.com/fmtlib/fmt/pull/1968>`_).
  Thanks `@tohammer (Tobias Hammer) <https://github.com/tohammer>`_.

* Fixed a sign conversion warning
  (`#1964 <https://github.com/fmtlib/fmt/pull/1964>`_).
  Thanks `@OptoCloud <https://github.com/OptoCloud>`_.

7.1.0 - 2020-10-25
------------------

* Switched from `Grisu3
  <https://www.cs.tufts.edu/~nr/cs257/archive/florian-loitsch/printf.pdf>`_
  to `Dragonbox <https://github.com/jk-jeon/dragonbox>`_ for the default
  floating-point formatting which gives the shortest decimal representation
  with round-trip guarantee and correct rounding
  (`#1882 <https://github.com/fmtlib/fmt/pull/1882>`_,
  `#1887 <https://github.com/fmtlib/fmt/pull/1887>`_,
  `#1894 <https://github.com/fmtlib/fmt/pull/1894>`_). This makes {fmt} up to
  20-30x faster than common implementations of ``std::ostringstream`` and
  ``sprintf`` on `dtoa-benchmark <https://github.com/fmtlib/dtoa-benchmark>`_
  and faster than double-conversion and Ryū:

  .. image:: https://user-images.githubusercontent.com/576385/
             95684665-11719600-0ba8-11eb-8e5b-972ff4e49428.png

  It is possible to get even better performance at the cost of larger binary
  size by compiling with the ``FMT_USE_FULL_CACHE_DRAGONBOX`` macro set to 1.

  Thanks `@jk-jeon (Junekey Jeon) <https://github.com/jk-jeon>`_.

* Added an experimental unsynchronized file output API which, together with
  `format string compilation <https://fmt.dev/latest/api.html#compile-api>`_,
  can give `5-9 times speed up compared to fprintf
  <https://www.zverovich.net/2020/08/04/optimal-file-buffer-size.html>`_
  on common platforms (`godbolt <https://godbolt.org/z/nsTcG8>`__):

  .. code:: c++

     #include <fmt/os.h>

     int main() {
       auto f = fmt::output_file("guide");
       f.print("The answer is {}.", 42);
     }

* Added a formatter for ``std::chrono::time_point<system_clock>``
  (`#1819 <https://github.com/fmtlib/fmt/issues/1819>`_,
  `#1837 <https://github.com/fmtlib/fmt/pull/1837>`_). For example
  (`godbolt <https://godbolt.org/z/c4M6fh>`__):

  .. code:: c++

     #include <fmt/chrono.h>

     int main() {
       auto now = std::chrono::system_clock::now();
       fmt::print("The time is {:%H:%M:%S}.\n", now);
     }

  Thanks `@adamburgess (Adam Burgess) <https://github.com/adamburgess>`_.

* Added support for ranges with non-const ``begin``/``end`` to ``fmt::join``
  (`#1784 <https://github.com/fmtlib/fmt/issues/1784>`_,
  `#1786 <https://github.com/fmtlib/fmt/pull/1786>`_). For example
  (`godbolt <https://godbolt.org/z/jP63Tv>`__):

  .. code:: c++

     #include <fmt/ranges.h>
     #include <range/v3/view/filter.hpp>

     int main() {
       using std::literals::string_literals::operator""s;
       auto strs = std::array{"a"s, "bb"s, "ccc"s};
       auto range = strs | ranges::views::filter(
         [] (const std::string &x) { return x.size() != 2; }
       );
       fmt::print("{}\n", fmt::join(range, ""));
     }

  prints "accc".

  Thanks `@tonyelewis (Tony E Lewis) <https://github.com/tonyelewis>`_.

* Added a ``memory_buffer::append`` overload that takes a range
  (`#1806 <https://github.com/fmtlib/fmt/pull/1806>`_).
  Thanks `@BRevzin (Barry Revzin) <https://github.com/BRevzin>`_.

* Improved handling of single code units in ``FMT_COMPILE``. For example:

  .. code:: c++

     #include <fmt/compile.h>

     char* f(char* buf) {
       return fmt::format_to(buf, FMT_COMPILE("x{}"), 42);
     }

  compiles to just (`godbolt <https://godbolt.org/z/5vncz3>`__):

  .. code:: asm

     _Z1fPc:
       movb $120, (%rdi)
       xorl %edx, %edx
       cmpl $42, _ZN3fmt2v76detail10basic_dataIvE23zero_or_powers_of_10_32E+8(%rip)
       movl $3, %eax
       seta %dl
       subl %edx, %eax
       movzwl _ZN3fmt2v76detail10basic_dataIvE6digitsE+84(%rip), %edx
       cltq
       addq %rdi, %rax
       movw %dx, -2(%rax)
       ret

  Here a single ``mov`` instruction writes ``'x'`` (``$120``) to the output
  buffer.

* Added dynamic width support to format string compilation
  (`#1809 <https://github.com/fmtlib/fmt/issues/1809>`_).

* Improved error reporting for unformattable types: now you'll get the type name
  directly in the error message instead of the note:

  .. code:: c++

     #include <fmt/core.h>

     struct how_about_no {};

     int main() {
       fmt::print("{}", how_about_no());
     }

  Error (`godbolt <https://godbolt.org/z/GoxM4e>`__):

  ``fmt/core.h:1438:3: error: static_assert failed due to requirement
  'fmt::v7::formattable<how_about_no>()' "Cannot format an argument.
  To make type T formattable provide a formatter<T> specialization:
  https://fmt.dev/latest/api.html#udt"
  ...``

* Added the `make_args_checked <https://fmt.dev/7.1.0/api.html#argument-lists>`_
  function template that allows you to write formatting functions with
  compile-time format string checks and avoid binary code bloat
  (`godbolt <https://godbolt.org/z/PEf9qr>`__):

  .. code:: c++

     void vlog(const char* file, int line, fmt::string_view format,
               fmt::format_args args) {
       fmt::print("{}: {}: ", file, line);
       fmt::vprint(format, args);
     }

     template <typename S, typename... Args>
     void log(const char* file, int line, const S& format, Args&&... args) {
       vlog(file, line, format,
           fmt::make_args_checked<Args...>(format, args...));
     }

     #define MY_LOG(format, ...) \
       log(__FILE__, __LINE__, FMT_STRING(format), __VA_ARGS__)

     MY_LOG("invalid squishiness: {}", 42);

* Replaced ``snprintf`` fallback with a faster internal IEEE 754 ``float`` and
  ``double`` formatter for arbitrary precision. For example
  (`godbolt <https://godbolt.org/z/dPhWvj>`__):

  .. code:: c++

     #include <fmt/core.h>

     int main() {
       fmt::print("{:.500}\n", 4.9406564584124654E-324);
     }

  prints

  ``4.9406564584124654417656879286822137236505980261432476442558568250067550727020875186529983636163599237979656469544571773092665671035593979639877479601078187812630071319031140452784581716784898210368871863605699873072305000638740915356498438731247339727316961514003171538539807412623856559117102665855668676818703956031062493194527159149245532930545654440112748012970999954193198940908041656332452475714786901472678015935523861155013480352649347201937902681071074917033322268447533357208324319360923829e-324``.

* Made ``format_to_n`` and ``formatted_size`` part of the `core API
  <https://fmt.dev/latest/api.html#core-api>`__
  (`godbolt <https://godbolt.org/z/sPjY1K>`__):

  .. code:: c++

     #include <fmt/core.h>

     int main() {
       char buffer[10];
       auto result = fmt::format_to_n(buffer, sizeof(buffer), "{}", 42);
     }

* Added ``fmt::format_to_n`` overload with format string compilation
  (`#1764 <https://github.com/fmtlib/fmt/issues/1764>`_,
  `#1767 <https://github.com/fmtlib/fmt/pull/1767>`_,
  `#1869 <https://github.com/fmtlib/fmt/pull/1869>`_). For example
  (`godbolt <https://godbolt.org/z/93h86q>`__):

  .. code:: c++

     #include <fmt/compile.h>

     int main() {
       char buffer[8];
       fmt::format_to_n(buffer, sizeof(buffer), FMT_COMPILE("{}"), 42);
     }

  Thanks `@Kurkin (Dmitry Kurkin) <https://github.com/Kurkin>`_,
  `@alexezeder (Alexey Ochapov) <https://github.com/alexezeder>`_.

* Added ``fmt::format_to`` overload that take ``text_style``
  (`#1593 <https://github.com/fmtlib/fmt/issues/1593>`_,
  `#1842 <https://github.com/fmtlib/fmt/issues/1842>`_,
  `#1843 <https://github.com/fmtlib/fmt/pull/1843>`_). For example
  (`godbolt <https://godbolt.org/z/91153r>`__):

  .. code:: c++

     #include <fmt/color.h>

     int main() {
       std::string out;
       fmt::format_to(std::back_inserter(out),
                      fmt::emphasis::bold | fg(fmt::color::red),
                      "The answer is {}.", 42);
     }

  Thanks `@Naios (Denis Blank) <https://github.com/Naios>`_.

* Made the ``'#'`` specifier emit trailing zeros in addition to the decimal
  point (`#1797 <https://github.com/fmtlib/fmt/issues/1797>`_). For example
  (`godbolt <https://godbolt.org/z/bhdcW9>`__):

  .. code:: c++

     #include <fmt/core.h>

     int main() {
       fmt::print("{:#.2g}", 0.5);
     }

  prints ``0.50``.

* Changed the default floating point format to not include ``.0`` for
  consistency with ``std::format`` and ``std::to_chars``
  (`#1893 <https://github.com/fmtlib/fmt/issues/1893>`_,
  `#1943 <https://github.com/fmtlib/fmt/issues/1943>`_). It is possible to get
  the decimal point and trailing zero with the ``#`` specifier.

* Fixed an issue with floating-point formatting that could result in addition of
  a non-significant trailing zero in rare cases e.g. ``1.00e-34`` instead of
  ``1.0e-34`` (`#1873 <https://github.com/fmtlib/fmt/issues/1873>`_,
  `#1917 <https://github.com/fmtlib/fmt/issues/1917>`_).

* Made ``fmt::to_string`` fallback on ``ostream`` insertion operator if
  the ``formatter`` specialization is not provided
  (`#1815 <https://github.com/fmtlib/fmt/issues/1815>`_,
  `#1829 <https://github.com/fmtlib/fmt/pull/1829>`_).
  Thanks `@alexezeder (Alexey Ochapov) <https://github.com/alexezeder>`_.

* Added support for the append mode to the experimental file API and
  improved ``fcntl.h`` detection.
  (`#1847 <https://github.com/fmtlib/fmt/pull/1847>`_,
  `#1848 <https://github.com/fmtlib/fmt/pull/1848>`_).
  Thanks `@t-wiser <https://github.com/t-wiser>`_.

* Fixed handling of types that have both an implicit conversion operator and
  an overloaded ``ostream`` insertion operator
  (`#1766 <https://github.com/fmtlib/fmt/issues/1766>`_).

* Fixed a slicing issue in an internal iterator type
  (`#1822 <https://github.com/fmtlib/fmt/pull/1822>`_).
  Thanks `@BRevzin (Barry Revzin) <https://github.com/BRevzin>`_.

* Fixed an issue in locale-specific integer formatting
  (`#1927 <https://github.com/fmtlib/fmt/issues/1927>`_).

* Fixed handling of exotic code unit types
  (`#1870 <https://github.com/fmtlib/fmt/issues/1870>`_,
  `#1932 <https://github.com/fmtlib/fmt/issues/1932>`_).

* Improved ``FMT_ALWAYS_INLINE``
  (`#1878 <https://github.com/fmtlib/fmt/pull/1878>`_).
  Thanks `@jk-jeon (Junekey Jeon) <https://github.com/jk-jeon>`_.

* Removed dependency on ``windows.h``
  (`#1900 <https://github.com/fmtlib/fmt/pull/1900>`_).
  Thanks `@bernd5 (Bernd Baumanns) <https://github.com/bernd5>`_.

* Optimized counting of decimal digits on MSVC
  (`#1890 <https://github.com/fmtlib/fmt/pull/1890>`_).
  Thanks `@mwinterb <https://github.com/mwinterb>`_.

* Improved documentation
  (`#1772 <https://github.com/fmtlib/fmt/issues/1772>`_,
  `#1775 <https://github.com/fmtlib/fmt/pull/1775>`_,
  `#1792 <https://github.com/fmtlib/fmt/pull/1792>`_,
  `#1838 <https://github.com/fmtlib/fmt/pull/1838>`_,
  `#1888 <https://github.com/fmtlib/fmt/pull/1888>`_,
  `#1918 <https://github.com/fmtlib/fmt/pull/1918>`_,
  `#1939 <https://github.com/fmtlib/fmt/pull/1939>`_).
  Thanks `@leolchat (Léonard Gérard) <https://github.com/leolchat>`_,
  `@pepsiman (Malcolm Parsons) <https://github.com/pepsiman>`_,
  `@Klaim (Joël Lamotte) <https://github.com/Klaim>`_,
  `@ravijanjam (Ravi J) <https://github.com/ravijanjam>`_,
  `@francesco-st <https://github.com/francesco-st>`_,
  `@udnaan (Adnan) <https://github.com/udnaan>`_.

* Added the ``FMT_REDUCE_INT_INSTANTIATIONS`` CMake option that reduces the
  binary code size at the cost of some integer formatting performance. This can
  be useful for extremely memory-constrained embedded systems
  (`#1778 <https://github.com/fmtlib/fmt/issues/1778>`_,
  `#1781 <https://github.com/fmtlib/fmt/pull/1781>`_).
  Thanks `@kammce (Khalil Estell) <https://github.com/kammce>`_.

* Added the ``FMT_USE_INLINE_NAMESPACES`` macro to control usage of inline
  namespaces (`#1945 <https://github.com/fmtlib/fmt/pull/1945>`_).
  Thanks `@darklukee <https://github.com/darklukee>`_.

* Improved build configuration
  (`#1760 <https://github.com/fmtlib/fmt/pull/1760>`_,
  `#1770 <https://github.com/fmtlib/fmt/pull/1770>`_,
  `#1779 <https://github.com/fmtlib/fmt/issues/1779>`_,
  `#1783 <https://github.com/fmtlib/fmt/pull/1783>`_,
  `#1823 <https://github.com/fmtlib/fmt/pull/1823>`_).
  Thanks `@dvetutnev (Dmitriy Vetutnev) <https://github.com/dvetutnev>`_,
  `@xvitaly (Vitaly Zaitsev) <https://github.com/xvitaly>`_,
  `@tambry (Raul Tambre) <https://github.com/tambry>`_,
  `@medithe <https://github.com/medithe>`_,
  `@martinwuehrer (Martin Wührer) <https://github.com/martinwuehrer>`_.

* Fixed various warnings and compilation issues
  (`#1790 <https://github.com/fmtlib/fmt/pull/1790>`_,
  `#1802 <https://github.com/fmtlib/fmt/pull/1802>`_,
  `#1808 <https://github.com/fmtlib/fmt/pull/1808>`_,
  `#1810 <https://github.com/fmtlib/fmt/issues/1810>`_,
  `#1811 <https://github.com/fmtlib/fmt/issues/1811>`_,
  `#1812 <https://github.com/fmtlib/fmt/pull/1812>`_,
  `#1814 <https://github.com/fmtlib/fmt/pull/1814>`_,
  `#1816 <https://github.com/fmtlib/fmt/pull/1816>`_,
  `#1817 <https://github.com/fmtlib/fmt/pull/1817>`_,
  `#1818 <https://github.com/fmtlib/fmt/pull/1818>`_,
  `#1825 <https://github.com/fmtlib/fmt/issues/1825>`_,
  `#1836 <https://github.com/fmtlib/fmt/pull/1836>`_,
  `#1855 <https://github.com/fmtlib/fmt/pull/1855>`_,
  `#1856 <https://github.com/fmtlib/fmt/pull/1856>`_,
  `#1860 <https://github.com/fmtlib/fmt/pull/1860>`_,
  `#1877 <https://github.com/fmtlib/fmt/pull/1877>`_,
  `#1879 <https://github.com/fmtlib/fmt/pull/1879>`_,
  `#1880 <https://github.com/fmtlib/fmt/pull/1880>`_,
  `#1896 <https://github.com/fmtlib/fmt/issues/1896>`_,
  `#1897 <https://github.com/fmtlib/fmt/pull/1897>`_,
  `#1898 <https://github.com/fmtlib/fmt/pull/1898>`_,
  `#1904 <https://github.com/fmtlib/fmt/issues/1904>`_,
  `#1908 <https://github.com/fmtlib/fmt/pull/1908>`_,
  `#1911 <https://github.com/fmtlib/fmt/issues/1911>`_,
  `#1912 <https://github.com/fmtlib/fmt/issues/1912>`_,
  `#1928 <https://github.com/fmtlib/fmt/issues/1928>`_,
  `#1929 <https://github.com/fmtlib/fmt/pull/1929>`_,
  `#1935 <https://github.com/fmtlib/fmt/issues/1935>`_
  `#1937 <https://github.com/fmtlib/fmt/pull/1937>`_,
  `#1942 <https://github.com/fmtlib/fmt/pull/1942>`_,
  `#1949 <https://github.com/fmtlib/fmt/issues/1949>`_).
  Thanks `@TheQwertiest <https://github.com/TheQwertiest>`_,
  `@medithe <https://github.com/medithe>`_,
  `@martinwuehrer (Martin Wührer) <https://github.com/martinwuehrer>`_,
  `@n16h7hunt3r <https://github.com/n16h7hunt3r>`_,
  `@Othereum (Seokjin Lee) <https://github.com/Othereum>`_,
  `@gsjaardema (Greg Sjaardema) <https://github.com/gsjaardema>`_,
  `@AlexanderLanin (Alexander Lanin) <https://github.com/AlexanderLanin>`_,
  `@gcerretani (Giovanni Cerretani) <https://github.com/gcerretani>`_,
  `@chronoxor (Ivan Shynkarenka) <https://github.com/chronoxor>`_,
  `@noizefloor (Jan Schwers) <https://github.com/noizefloor>`_,
  `@akohlmey (Axel Kohlmeyer) <https://github.com/akohlmey>`_,
  `@jk-jeon (Junekey Jeon) <https://github.com/jk-jeon>`_,
  `@rimathia <https://github.com/rimathia>`_,
  `@rglarix (Riccardo Ghetta (larix)) <https://github.com/rglarix>`_,
  `@moiwi <https://github.com/moiwi>`_,
  `@heckad (Kazantcev Andrey) <https://github.com/heckad>`_,
  `@MarcDirven <https://github.com/MarcDirven>`_.
  `@BartSiwek (Bart Siwek) <https://github.com/BartSiwek>`_,
  `@darklukee <https://github.com/darklukee>`_.

7.0.3 - 2020-08-06
------------------

* Worked around broken ``numeric_limits`` for 128-bit integers
  (`#1787 <https://github.com/fmtlib/fmt/issues/1787>`_).

* Added error reporting on missing named arguments
  (`#1796 <https://github.com/fmtlib/fmt/issues/1796>`_).

* Stopped using 128-bit integers with clang-cl
  (`#1800 <https://github.com/fmtlib/fmt/pull/1800>`_).
  Thanks `@Kingcom <https://github.com/Kingcom>`_.

* Fixed issues in locale-specific integer formatting
  (`#1782 <https://github.com/fmtlib/fmt/issues/1782>`_,
  `#1801 <https://github.com/fmtlib/fmt/issues/1801>`_).

7.0.2 - 2020-07-29
------------------

* Worked around broken ``numeric_limits`` for 128-bit integers
  (`#1725 <https://github.com/fmtlib/fmt/issues/1725>`_).

* Fixed compatibility with CMake 3.4
  (`#1779 <https://github.com/fmtlib/fmt/issues/1779>`_).

* Fixed handling of digit separators in locale-specific formatting
  (`#1782 <https://github.com/fmtlib/fmt/issues/1782>`_).

7.0.1 - 2020-07-07
------------------

* Updated the inline version namespace name.

* Worked around a gcc bug in mangling of alias templates
  (`#1753 <https://github.com/fmtlib/fmt/issues/1753>`_).

* Fixed a linkage error on Windows
  (`#1757 <https://github.com/fmtlib/fmt/issues/1757>`_).
  Thanks `@Kurkin (Dmitry Kurkin) <https://github.com/Kurkin>`_.

* Fixed minor issues with the documentation.

7.0.0 - 2020-07-05
------------------

* Reduced the library size. For example, on macOS a stripped test binary
  statically linked with {fmt} `shrank from ~368k to less than 100k
  <http://www.zverovich.net/2020/05/21/reducing-library-size.html>`_.

* Added a simpler and more efficient `format string compilation API
  <https://fmt.dev/7.0.0/api.html#compile-api>`_:

  .. code:: c++

     #include <fmt/compile.h>

     // Converts 42 into std::string using the most efficient method and no
     // runtime format string processing.
     std::string s = fmt::format(FMT_COMPILE("{}"), 42);

  The old ``fmt::compile`` API is now deprecated.

* Optimized integer formatting: ``format_to`` with format string compilation
  and a stack-allocated buffer is now `faster than to_chars on both
  libc++ and libstdc++
  <http://www.zverovich.net/2020/06/13/fast-int-to-string-revisited.html>`_.

* Optimized handling of small format strings. For example,

  .. code:: c++

      fmt::format("Result: {}: ({},{},{},{})", str1, str2, str3, str4, str5)

  is now ~40% faster (`#1685 <https://github.com/fmtlib/fmt/issues/1685>`_).

* Applied extern templates to improve compile times when using the core API
  and ``fmt/format.h`` (`#1452 <https://github.com/fmtlib/fmt/issues/1452>`_).
  For example, on macOS with clang the compile time of a test translation unit
  dropped from 2.3s to 0.3s with ``-O2`` and from 0.6s to 0.3s with the default
  settings (``-O0``).

  Before (``-O2``)::

    % time c++ -c test.cc -I include -std=c++17 -O2
    c++ -c test.cc -I include -std=c++17 -O2  2.22s user 0.08s system 99% cpu 2.311 total

  After (``-O2``)::

    % time c++ -c test.cc -I include -std=c++17 -O2
    c++ -c test.cc -I include -std=c++17 -O2  0.26s user 0.04s system 98% cpu 0.303 total

  Before (default)::

    % time c++ -c test.cc -I include -std=c++17
    c++ -c test.cc -I include -std=c++17  0.53s user 0.06s system 98% cpu 0.601 total

  After (default)::

    % time c++ -c test.cc -I include -std=c++17
    c++ -c test.cc -I include -std=c++17  0.24s user 0.06s system 98% cpu 0.301 total

  It is still recommended to use ``fmt/core.h`` instead of ``fmt/format.h`` but
  the compile time difference is now smaller. Thanks
  `@alex3d <https://github.com/alex3d>`_ for the suggestion.

* Named arguments are now stored on stack (no dynamic memory allocations) and
  the compiled code is more compact and efficient. For example

  .. code:: c++

     #include <fmt/core.h>

     int main() {
       fmt::print("The answer is {answer}\n", fmt::arg("answer", 42));
     }

  compiles to just (`godbolt <https://godbolt.org/z/NcfEp_>`__)

  .. code:: asm

      .LC0:
              .string "answer"
      .LC1:
              .string "The answer is {answer}\n"
      main:
              sub     rsp, 56
              mov     edi, OFFSET FLAT:.LC1
              mov     esi, 23
              movabs  rdx, 4611686018427387905
              lea     rax, [rsp+32]
              lea     rcx, [rsp+16]
              mov     QWORD PTR [rsp+8], 1
              mov     QWORD PTR [rsp], rax
              mov     DWORD PTR [rsp+16], 42
              mov     QWORD PTR [rsp+32], OFFSET FLAT:.LC0
              mov     DWORD PTR [rsp+40], 0
              call    fmt::v6::vprint(fmt::v6::basic_string_view<char>,
                                      fmt::v6::format_args)
              xor     eax, eax
              add     rsp, 56
              ret

          .L.str.1:
                  .asciz  "answer"

* Implemented compile-time checks for dynamic width and precision
  (`#1614 <https://github.com/fmtlib/fmt/issues/1614>`_):

  .. code:: c++

     #include <fmt/format.h>

     int main() {
       fmt::print(FMT_STRING("{0:{1}}"), 42);
     }

  now gives a compilation error because argument 1 doesn't exist::

    In file included from test.cc:1:
    include/fmt/format.h:2726:27: error: constexpr variable 'invalid_format' must be
    initialized by a constant expression
      FMT_CONSTEXPR_DECL bool invalid_format =
                              ^
    ...
    include/fmt/core.h:569:26: note: in call to
    '&checker(s, {}).context_->on_error(&"argument not found"[0])'
        if (id >= num_args_) on_error("argument not found");
                            ^

* Added sentinel support to ``fmt::join``
  (`#1689 <https://github.com/fmtlib/fmt/pull/1689>`_)

  .. code:: c++

    struct zstring_sentinel {};
    bool operator==(const char* p, zstring_sentinel) { return *p == '\0'; }
    bool operator!=(const char* p, zstring_sentinel) { return *p != '\0'; }

    struct zstring {
      const char* p;
      const char* begin() const { return p; }
      zstring_sentinel end() const { return {}; }
    };

    auto s = fmt::format("{}", fmt::join(zstring{"hello"}, "_"));
    // s == "h_e_l_l_o"

  Thanks `@BRevzin (Barry Revzin) <https://github.com/BRevzin>`_.

* Added support for named arguments, ``clear`` and ``reserve`` to
  ``dynamic_format_arg_store``
  (`#1655 <https://github.com/fmtlib/fmt/issues/1655>`_,
  `#1663 <https://github.com/fmtlib/fmt/pull/1663>`_,
  `#1674 <https://github.com/fmtlib/fmt/pull/1674>`_,
  `#1677 <https://github.com/fmtlib/fmt/pull/1677>`_).
  Thanks `@vsolontsov-ll (Vladimir Solontsov)
  <https://github.com/vsolontsov-ll>`_.

* Added support for the ``'c'`` format specifier to integral types for
  compatibility with ``std::format``
  (`#1652 <https://github.com/fmtlib/fmt/issues/1652>`_).

* Replaced the ``'n'`` format specifier with ``'L'`` for compatibility with
  ``std::format`` (`#1624 <https://github.com/fmtlib/fmt/issues/1624>`_).
  The ``'n'`` specifier can be enabled via the ``FMT_DEPRECATED_N_SPECIFIER``
  macro.

* The ``'='`` format specifier is now disabled by default for compatibility with
  ``std::format``. It can be enabled via the ``FMT_DEPRECATED_NUMERIC_ALIGN``
  macro.

* Removed the following deprecated APIs:

  * ``FMT_STRING_ALIAS`` and ``fmt`` macros - replaced by ``FMT_STRING``
  * ``fmt::basic_string_view::char_type`` - replaced by
    ``fmt::basic_string_view::value_type``
  * ``convert_to_int``
  * ``format_arg_store::types``
  * ``*parse_context`` - replaced by ``*format_parse_context``
  * ``FMT_DEPRECATED_INCLUDE_OS``
  * ``FMT_DEPRECATED_PERCENT`` - incompatible with ``std::format``
  * ``*writer`` - replaced by compiled format API

* Renamed the ``internal`` namespace to ``detail``
  (`#1538 <https://github.com/fmtlib/fmt/issues/1538>`_). The former is still
  provided as an alias if the ``FMT_USE_INTERNAL`` macro is defined.

* Improved compatibility between ``fmt::printf`` with the standard specs
  (`#1595 <https://github.com/fmtlib/fmt/issues/1595>`_,
  `#1682 <https://github.com/fmtlib/fmt/pull/1682>`_,
  `#1683 <https://github.com/fmtlib/fmt/pull/1683>`_,
  `#1687 <https://github.com/fmtlib/fmt/pull/1687>`_,
  `#1699 <https://github.com/fmtlib/fmt/pull/1699>`_).
  Thanks `@rimathia <https://github.com/rimathia>`_.

* Fixed handling of ``operator<<`` overloads that use ``copyfmt``
  (`#1666 <https://github.com/fmtlib/fmt/issues/1666>`_).

* Added the ``FMT_OS`` CMake option to control inclusion of OS-specific APIs
  in the fmt target. This can be useful for embedded platforms
  (`#1654 <https://github.com/fmtlib/fmt/issues/1654>`_,
  `#1656 <https://github.com/fmtlib/fmt/pull/1656>`_).
  Thanks `@kwesolowski (Krzysztof Wesolowski)
  <https://github.com/kwesolowski>`_.

* Replaced ``FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION`` with the ``FMT_FUZZ``
  macro to prevent interferring with fuzzing of projects using {fmt}
  (`#1650 <https://github.com/fmtlib/fmt/pull/1650>`_).
  Thanks `@asraa (Asra Ali) <https://github.com/asraa>`_.

* Fixed compatibility with emscripten
  (`#1636 <https://github.com/fmtlib/fmt/issues/1636>`_,
  `#1637 <https://github.com/fmtlib/fmt/pull/1637>`_).
  Thanks `@ArthurSonzogni (Arthur Sonzogni)
  <https://github.com/ArthurSonzogni>`_.

* Improved documentation
  (`#704 <https://github.com/fmtlib/fmt/issues/704>`_,
  `#1643 <https://github.com/fmtlib/fmt/pull/1643>`_,
  `#1660 <https://github.com/fmtlib/fmt/pull/1660>`_,
  `#1681 <https://github.com/fmtlib/fmt/pull/1681>`_,
  `#1691 <https://github.com/fmtlib/fmt/pull/1691>`_,
  `#1706 <https://github.com/fmtlib/fmt/pull/1706>`_,
  `#1714 <https://github.com/fmtlib/fmt/pull/1714>`_,
  `#1721 <https://github.com/fmtlib/fmt/pull/1721>`_,
  `#1739 <https://github.com/fmtlib/fmt/pull/1739>`_,
  `#1740 <https://github.com/fmtlib/fmt/pull/1740>`_,
  `#1741 <https://github.com/fmtlib/fmt/pull/1741>`_,
  `#1751 <https://github.com/fmtlib/fmt/pull/1751>`_).
  Thanks `@senior7515 (Alexander Gallego) <https://github.com/senior7515>`_,
  `@lsr0 (Lindsay Roberts) <https://github.com/lsr0>`_,
  `@puetzk (Kevin Puetz) <https://github.com/puetzk>`_,
  `@fpelliccioni (Fernando Pelliccioni) <https://github.com/fpelliccioni>`_,
  Alexey Kuzmenko, `@jelly (jelle van der Waa) <https://github.com/jelly>`_,
  `@claremacrae (Clare Macrae) <https://github.com/claremacrae>`_,
  `@jiapengwen (文佳鹏) <https://github.com/jiapengwen>`_,
  `@gsjaardema (Greg Sjaardema) <https://github.com/gsjaardema>`_,
  `@alexey-milovidov <https://github.com/alexey-milovidov>`_.

* Implemented various build configuration fixes and improvements
  (`#1603 <https://github.com/fmtlib/fmt/pull/1603>`_,
  `#1657 <https://github.com/fmtlib/fmt/pull/1657>`_,
  `#1702 <https://github.com/fmtlib/fmt/pull/1702>`_,
  `#1728 <https://github.com/fmtlib/fmt/pull/1728>`_).
  Thanks `@scramsby (Scott Ramsby) <https://github.com/scramsby>`_,
  `@jtojnar (Jan Tojnar) <https://github.com/jtojnar>`_,
  `@orivej (Orivej Desh) <https://github.com/orivej>`_,
  `@flagarde <https://github.com/flagarde>`_.

* Fixed various warnings and compilation issues
  (`#1616 <https://github.com/fmtlib/fmt/pull/1616>`_,
  `#1620 <https://github.com/fmtlib/fmt/issues/1620>`_,
  `#1622 <https://github.com/fmtlib/fmt/issues/1622>`_,
  `#1625 <https://github.com/fmtlib/fmt/issues/1625>`_,
  `#1627 <https://github.com/fmtlib/fmt/pull/1627>`_,
  `#1628 <https://github.com/fmtlib/fmt/issues/1628>`_,
  `#1629 <https://github.com/fmtlib/fmt/pull/1629>`_,
  `#1631 <https://github.com/fmtlib/fmt/issues/1631>`_,
  `#1633 <https://github.com/fmtlib/fmt/pull/1633>`_,
  `#1649 <https://github.com/fmtlib/fmt/pull/1649>`_,
  `#1658 <https://github.com/fmtlib/fmt/issues/1658>`_,
  `#1661 <https://github.com/fmtlib/fmt/pull/1661>`_,
  `#1667 <https://github.com/fmtlib/fmt/pull/1667>`_,
  `#1668 <https://github.com/fmtlib/fmt/issues/1668>`_,
  `#1669 <https://github.com/fmtlib/fmt/pull/1669>`_,
  `#1692 <https://github.com/fmtlib/fmt/issues/1692>`_,
  `#1696 <https://github.com/fmtlib/fmt/pull/1696>`_,
  `#1697 <https://github.com/fmtlib/fmt/pull/1697>`_,
  `#1707 <https://github.com/fmtlib/fmt/issues/1707>`_,
  `#1712 <https://github.com/fmtlib/fmt/pull/1712>`_,
  `#1716 <https://github.com/fmtlib/fmt/pull/1716>`_,
  `#1722 <https://github.com/fmtlib/fmt/pull/1722>`_,
  `#1724 <https://github.com/fmtlib/fmt/issues/1724>`_,
  `#1729 <https://github.com/fmtlib/fmt/pull/1729>`_,
  `#1738 <https://github.com/fmtlib/fmt/pull/1738>`_,
  `#1742 <https://github.com/fmtlib/fmt/issues/1742>`_,
  `#1743 <https://github.com/fmtlib/fmt/issues/1743>`_,
  `#1744 <https://github.com/fmtlib/fmt/pull/1744>`_,
  `#1747 <https://github.com/fmtlib/fmt/issues/1747>`_,
  `#1750 <https://github.com/fmtlib/fmt/pull/1750>`_).
  Thanks `@gsjaardema (Greg Sjaardema) <https://github.com/gsjaardema>`_,
  `@gabime (Gabi Melman) <https://github.com/gabime>`_,
  `@johnor (Johan) <https://github.com/johnor>`_,
  `@Kurkin (Dmitry Kurkin) <https://github.com/Kurkin>`_,
  `@invexed (James Beach) <https://github.com/invexed>`_,
  `@peterbell10 <https://github.com/peterbell10>`_,
  `@daixtrose (Markus Werle) <https://github.com/daixtrose>`_,
  `@petrutlucian94 (Lucian Petrut) <https://github.com/petrutlucian94>`_,
  `@Neargye (Daniil Goncharov) <https://github.com/Neargye>`_,
  `@ambitslix (Attila M. Szilagyi) <https://github.com/ambitslix>`_,
  `@gabime (Gabi Melman) <https://github.com/gabime>`_,
  `@erthink (Leonid Yuriev) <https://github.com/erthink>`_,
  `@tohammer (Tobias Hammer) <https://github.com/tohammer>`_,
  `@0x8000-0000 (Florin Iucha) <https://github.com/0x8000-0000>`_.

6.2.1 - 2020-05-09
------------------

* Fixed ostream support in ``sprintf``
  (`#1631 <https://github.com/fmtlib/fmt/issues/1631>`_).

* Fixed type detection when using implicit conversion to ``string_view`` and
  ostream ``operator<<`` inconsistently
  (`#1662 <https://github.com/fmtlib/fmt/issues/1662>`_).

6.2.0 - 2020-04-05
------------------

* Improved error reporting when trying to format an object of a non-formattable
  type:

  .. code:: c++

     fmt::format("{}", S());

  now gives::

    include/fmt/core.h:1015:5: error: static_assert failed due to requirement
    'formattable' "Cannot format argument. To make type T formattable provide a
    formatter<T> specialization:
    https://fmt.dev/latest/api.html#formatting-user-defined-types"
        static_assert(
        ^
    ...
    note: in instantiation of function template specialization
    'fmt::v6::format<char [3], S, char>' requested here
      fmt::format("{}", S());
           ^

  if ``S`` is not formattable.

* Reduced the library size by ~10%.

* Always print decimal point if ``#`` is specified
  (`#1476 <https://github.com/fmtlib/fmt/issues/1476>`_,
  `#1498 <https://github.com/fmtlib/fmt/issues/1498>`_):

  .. code:: c++

     fmt::print("{:#.0f}", 42.0);

  now prints ``42.``

* Implemented the ``'L'`` specifier for locale-specific numeric formatting to
  improve compatibility with ``std::format``. The ``'n'`` specifier is now
  deprecated and will be removed in the next major release.

* Moved OS-specific APIs such as ``windows_error`` from ``fmt/format.h`` to
  ``fmt/os.h``. You can define ``FMT_DEPRECATED_INCLUDE_OS`` to automatically
  include ``fmt/os.h`` from ``fmt/format.h`` for compatibility but this will be
  disabled in the next major release.

* Added precision overflow detection in floating-point formatting.

* Implemented detection of invalid use of ``fmt::arg``.

* Used ``type_identity`` to block unnecessary template argument deduction.
  Thanks Tim Song.

* Improved UTF-8 handling
  (`#1109 <https://github.com/fmtlib/fmt/issues/1109>`_):

  .. code:: c++

     fmt::print("┌{0:─^{2}}┐\n"
                "│{1: ^{2}}│\n"
                "└{0:─^{2}}┘\n", "", "Привет, мир!", 20);

  now prints::

     ┌────────────────────┐
     │    Привет, мир!    │
     └────────────────────┘

  on systems that support Unicode.

* Added experimental dynamic argument storage
  (`#1170 <https://github.com/fmtlib/fmt/issues/1170>`_,
  `#1584 <https://github.com/fmtlib/fmt/pull/1584>`_):

  .. code:: c++

     fmt::dynamic_format_arg_store<fmt::format_context> store;
     store.push_back("answer");
     store.push_back(42);
     fmt::vprint("The {} is {}.\n", store);
  
  prints::

     The answer is 42.

  Thanks `@vsolontsov-ll (Vladimir Solontsov)
  <https://github.com/vsolontsov-ll>`_.

* Made ``fmt::join`` accept ``initializer_list``
  (`#1591 <https://github.com/fmtlib/fmt/pull/1591>`_).
  Thanks `@Rapotkinnik (Nikolay Rapotkin) <https://github.com/Rapotkinnik>`_.

* Fixed handling of empty tuples
  (`#1588 <https://github.com/fmtlib/fmt/issues/1588>`_).

* Fixed handling of output iterators in ``format_to_n``
  (`#1506 <https://github.com/fmtlib/fmt/issues/1506>`_).

* Fixed formatting of ``std::chrono::duration`` types to wide output
  (`#1533 <https://github.com/fmtlib/fmt/pull/1533>`_).
  Thanks `@zeffy (pilao) <https://github.com/zeffy>`_.

* Added const ``begin`` and ``end`` overload to buffers
  (`#1553 <https://github.com/fmtlib/fmt/pull/1553>`_).
  Thanks `@dominicpoeschko <https://github.com/dominicpoeschko>`_.

* Added the ability to disable floating-point formatting via ``FMT_USE_FLOAT``,
  ``FMT_USE_DOUBLE`` and ``FMT_USE_LONG_DOUBLE`` macros for extremely
  memory-constrained embedded system
  (`#1590 <https://github.com/fmtlib/fmt/pull/1590>`_).
  Thanks `@albaguirre (Alberto Aguirre) <https://github.com/albaguirre>`_.

* Made ``FMT_STRING`` work with ``constexpr`` ``string_view``
  (`#1589 <https://github.com/fmtlib/fmt/pull/1589>`_).
  Thanks `@scramsby (Scott Ramsby) <https://github.com/scramsby>`_.

* Implemented a minor optimization in the format string parser
  (`#1560 <https://github.com/fmtlib/fmt/pull/1560>`_).
  Thanks `@IkarusDeveloper <https://github.com/IkarusDeveloper>`_.

* Improved attribute detection
  (`#1469 <https://github.com/fmtlib/fmt/pull/1469>`_,
  `#1475 <https://github.com/fmtlib/fmt/pull/1475>`_,
  `#1576 <https://github.com/fmtlib/fmt/pull/1576>`_).
  Thanks `@federico-busato (Federico) <https://github.com/federico-busato>`_,
  `@chronoxor (Ivan Shynkarenka) <https://github.com/chronoxor>`_,
  `@refnum <https://github.com/refnum>`_.

* Improved documentation
  (`#1481 <https://github.com/fmtlib/fmt/pull/1481>`_,
  `#1523 <https://github.com/fmtlib/fmt/pull/1523>`_).
  Thanks `@JackBoosY (Jack·Boos·Yu) <https://github.com/JackBoosY>`_,
  `@imba-tjd (谭九鼎) <https://github.com/imba-tjd>`_.

* Fixed symbol visibility on Linux when compiling with ``-fvisibility=hidden``
  (`#1535 <https://github.com/fmtlib/fmt/pull/1535>`_).
  Thanks `@milianw (Milian Wolff) <https://github.com/milianw>`_.

* Implemented various build configuration fixes and improvements
  (`#1264 <https://github.com/fmtlib/fmt/issues/1264>`_,
  `#1460 <https://github.com/fmtlib/fmt/issues/1460>`_,
  `#1534 <https://github.com/fmtlib/fmt/pull/1534>`_,
  `#1536 <https://github.com/fmtlib/fmt/issues/1536>`_,
  `#1545 <https://github.com/fmtlib/fmt/issues/1545>`_,
  `#1546 <https://github.com/fmtlib/fmt/pull/1546>`_,
  `#1566 <https://github.com/fmtlib/fmt/issues/1566>`_,
  `#1582 <https://github.com/fmtlib/fmt/pull/1582>`_,
  `#1597 <https://github.com/fmtlib/fmt/issues/1597>`_,
  `#1598 <https://github.com/fmtlib/fmt/pull/1598>`_).
  Thanks `@ambitslix (Attila M. Szilagyi) <https://github.com/ambitslix>`_,
  `@jwillikers (Jordan Williams) <https://github.com/jwillikers>`_,
  `@stac47 (Laurent Stacul) <https://github.com/stac47>`_.

* Fixed various warnings and compilation issues
  (`#1433 <https://github.com/fmtlib/fmt/pull/1433>`_,
  `#1461 <https://github.com/fmtlib/fmt/issues/1461>`_,
  `#1470 <https://github.com/fmtlib/fmt/pull/1470>`_,
  `#1480 <https://github.com/fmtlib/fmt/pull/1480>`_,
  `#1485 <https://github.com/fmtlib/fmt/pull/1485>`_,
  `#1492 <https://github.com/fmtlib/fmt/pull/1492>`_,
  `#1493 <https://github.com/fmtlib/fmt/issues/1493>`_,
  `#1504 <https://github.com/fmtlib/fmt/issues/1504>`_,
  `#1505 <https://github.com/fmtlib/fmt/pull/1505>`_,
  `#1512 <https://github.com/fmtlib/fmt/pull/1512>`_,
  `#1515 <https://github.com/fmtlib/fmt/issues/1515>`_,
  `#1516 <https://github.com/fmtlib/fmt/pull/1516>`_,
  `#1518 <https://github.com/fmtlib/fmt/pull/1518>`_,
  `#1519 <https://github.com/fmtlib/fmt/pull/1519>`_,
  `#1520 <https://github.com/fmtlib/fmt/pull/1520>`_,
  `#1521 <https://github.com/fmtlib/fmt/pull/1521>`_,
  `#1522 <https://github.com/fmtlib/fmt/pull/1522>`_,
  `#1524 <https://github.com/fmtlib/fmt/issues/1524>`_,
  `#1530 <https://github.com/fmtlib/fmt/pull/1530>`_,
  `#1531 <https://github.com/fmtlib/fmt/issues/1531>`_,
  `#1532 <https://github.com/fmtlib/fmt/pull/1532>`_,
  `#1539 <https://github.com/fmtlib/fmt/issues/1539>`_,
  `#1547 <https://github.com/fmtlib/fmt/issues/1547>`_,
  `#1548 <https://github.com/fmtlib/fmt/issues/1548>`_,
  `#1554 <https://github.com/fmtlib/fmt/pull/1554>`_,
  `#1567 <https://github.com/fmtlib/fmt/issues/1567>`_,
  `#1568 <https://github.com/fmtlib/fmt/pull/1568>`_,
  `#1569 <https://github.com/fmtlib/fmt/pull/1569>`_,
  `#1571 <https://github.com/fmtlib/fmt/pull/1571>`_,
  `#1573 <https://github.com/fmtlib/fmt/pull/1573>`_,
  `#1575 <https://github.com/fmtlib/fmt/pull/1575>`_,
  `#1581 <https://github.com/fmtlib/fmt/pull/1581>`_,
  `#1583 <https://github.com/fmtlib/fmt/issues/1583>`_,
  `#1586 <https://github.com/fmtlib/fmt/issues/1586>`_,
  `#1587 <https://github.com/fmtlib/fmt/issues/1587>`_,
  `#1594 <https://github.com/fmtlib/fmt/issues/1594>`_,
  `#1596 <https://github.com/fmtlib/fmt/pull/1596>`_,
  `#1604 <https://github.com/fmtlib/fmt/issues/1604>`_,
  `#1606 <https://github.com/fmtlib/fmt/pull/1606>`_,
  `#1607 <https://github.com/fmtlib/fmt/issues/1607>`_,
  `#1609 <https://github.com/fmtlib/fmt/issues/1609>`_).
  Thanks `@marti4d (Chris Martin) <https://github.com/marti4d>`_,
  `@iPherian <https://github.com/iPherian>`_,
  `@parkertomatoes <https://github.com/parkertomatoes>`_,
  `@gsjaardema (Greg Sjaardema) <https://github.com/gsjaardema>`_,
  `@chronoxor (Ivan Shynkarenka) <https://github.com/chronoxor>`_,
  `@DanielaE (Daniela Engert) <https://github.com/DanielaE>`_,
  `@torsten48 <https://github.com/torsten48>`_,
  `@tohammer (Tobias Hammer) <https://github.com/tohammer>`_,
  `@lefticus (Jason Turner) <https://github.com/lefticus>`_,
  `@ryusakki (Haise) <https://github.com/ryusakki>`_,
  `@adnsv (Alex Denisov) <https://github.com/adnsv>`_,
  `@fghzxm <https://github.com/fghzxm>`_,
  `@refnum <https://github.com/refnum>`_,
  `@pramodk (Pramod Kumbhar) <https://github.com/pramodk>`_,
  `@Spirrwell <https://github.com/Spirrwell>`_,
  `@scramsby (Scott Ramsby) <https://github.com/scramsby>`_.

6.1.2 - 2019-12-11
------------------

* Fixed ABI compatibility with ``libfmt.so.6.0.0``
  (`#1471 <https://github.com/fmtlib/fmt/issues/1471>`_).

* Fixed handling types convertible to ``std::string_view``
  (`#1451 <https://github.com/fmtlib/fmt/pull/1451>`_).
  Thanks `@denizevrenci (Deniz Evrenci) <https://github.com/denizevrenci>`_.

* Made CUDA test an opt-in enabled via the ``FMT_CUDA_TEST`` CMake option.

* Fixed sign conversion warnings
  (`#1440 <https://github.com/fmtlib/fmt/pull/1440>`_).
  Thanks `@0x8000-0000 (Florin Iucha) <https://github.com/0x8000-0000>`_.

6.1.1 - 2019-12-04
------------------

* Fixed shared library build on Windows
  (`#1443 <https://github.com/fmtlib/fmt/pull/1443>`_,
  `#1445 <https://github.com/fmtlib/fmt/issues/1445>`_,
  `#1446 <https://github.com/fmtlib/fmt/pull/1446>`_,
  `#1450 <https://github.com/fmtlib/fmt/issues/1450>`_).
  Thanks `@egorpugin (Egor Pugin) <https://github.com/egorpugin>`_,
  `@bbolli (Beat Bolli) <https://github.com/bbolli>`_.

* Added a missing decimal point in exponent notation with trailing zeros.

* Removed deprecated ``format_arg_store::TYPES``.

6.1.0 - 2019-12-01
------------------

* {fmt} now formats IEEE 754 ``float`` and ``double`` using the shortest decimal
  representation with correct rounding by default:

  .. code:: c++

     #include <cmath>
     #include <fmt/core.h>

     int main() {
       fmt::print("{}", M_PI);
     }

  prints ``3.141592653589793``.

* Made the fast binary to decimal floating-point formatter the default,
  simplified it and improved performance. {fmt} is now 15 times faster than
  libc++'s ``std::ostringstream``, 11 times faster than ``printf`` and 10%
  faster than double-conversion on `dtoa-benchmark
  <https://github.com/fmtlib/dtoa-benchmark>`_:

  ==================  =========  =======
  Function            Time (ns)  Speedup
  ==================  =========  =======
  ostringstream        1,346.30    1.00x
  ostrstream           1,195.74    1.13x
  sprintf                995.08    1.35x
  doubleconv              99.10   13.59x
  fmt                     88.34   15.24x
  ==================  =========  =======

  .. image:: https://user-images.githubusercontent.com/576385/
             69767160-cdaca400-112f-11ea-9fc5-347c9f83caad.png

* {fmt} no longer converts ``float`` arguments to ``double``. In particular this
  improves the default (shortest) representation of floats and makes
  ``fmt::format`` consistent with ``std::format`` specs
  (`#1336 <https://github.com/fmtlib/fmt/issues/1336>`_,
  `#1353 <https://github.com/fmtlib/fmt/issues/1353>`_,
  `#1360 <https://github.com/fmtlib/fmt/pull/1360>`_,
  `#1361 <https://github.com/fmtlib/fmt/pull/1361>`_):

  .. code:: c++

     fmt::print("{}", 0.1f);

  prints ``0.1`` instead of ``0.10000000149011612``.

  Thanks `@orivej (Orivej Desh) <https://github.com/orivej>`_.

* Made floating-point formatting output consistent with ``printf``/iostreams
  (`#1376 <https://github.com/fmtlib/fmt/issues/1376>`_,
  `#1417 <https://github.com/fmtlib/fmt/issues/1417>`_).

* Added support for 128-bit integers
  (`#1287 <https://github.com/fmtlib/fmt/pull/1287>`_):

  .. code:: c++

     fmt::print("{}", std::numeric_limits<__int128_t>::max());

  prints ``170141183460469231731687303715884105727``.

  Thanks `@denizevrenci (Deniz Evrenci) <https://github.com/denizevrenci>`_.

* The overload of ``print`` that takes ``text_style`` is now atomic, i.e. the
  output from different threads doesn't interleave
  (`#1351 <https://github.com/fmtlib/fmt/pull/1351>`_).
  Thanks `@tankiJong (Tanki Zhang) <https://github.com/tankiJong>`_.

* Made compile time in the header-only mode ~20% faster by reducing the number
  of template instantiations. ``wchar_t`` overload of ``vprint`` was moved from
  ``fmt/core.h`` to ``fmt/format.h``.

* Added an overload of ``fmt::join`` that works with tuples
  (`#1322 <https://github.com/fmtlib/fmt/issues/1322>`_,
  `#1330 <https://github.com/fmtlib/fmt/pull/1330>`_):

  .. code:: c++

     #include <tuple>
     #include <fmt/ranges.h>

     int main() {
       std::tuple<char, int, float> t{'a', 1, 2.0f};
       fmt::print("{}", t);
     }

  prints ``('a', 1, 2.0)``.

  Thanks `@jeremyong (Jeremy Ong) <https://github.com/jeremyong>`_.

* Changed formatting of octal zero with prefix from "00" to "0":

  .. code:: c++

     fmt::print("{:#o}", 0);

  prints ``0``.

* The locale is now passed to ostream insertion (``<<``) operators
  (`#1406 <https://github.com/fmtlib/fmt/pull/1406>`_):

  .. code:: c++

     #include <fmt/locale.h>
     #include <fmt/ostream.h>

     struct S {
       double value;
     };

     std::ostream& operator<<(std::ostream& os, S s) {
       return os << s.value;
     }

     int main() {
       auto s = fmt::format(std::locale("fr_FR.UTF-8"), "{}", S{0.42});
       // s == "0,42"
     }

  Thanks `@dlaugt (Daniel Laügt) <https://github.com/dlaugt>`_.

* Locale-specific number formatting now uses grouping
  (`#1393 <https://github.com/fmtlib/fmt/issues/1393>`_
  `#1394 <https://github.com/fmtlib/fmt/pull/1394>`_).
  Thanks `@skrdaniel <https://github.com/skrdaniel>`_.

* Fixed handling of types with deleted implicit rvalue conversion to
  ``const char**`` (`#1421 <https://github.com/fmtlib/fmt/issues/1421>`_):

  .. code:: c++

     struct mystring {
       operator const char*() const&;
       operator const char*() &;
       operator const char*() const&& = delete;
       operator const char*() && = delete;
     };
     mystring str;
     fmt::print("{}", str); // now compiles

* Enums are now mapped to correct underlying types instead of ``int``
  (`#1286 <https://github.com/fmtlib/fmt/pull/1286>`_).
  Thanks `@agmt (Egor Seredin) <https://github.com/agmt>`_.

* Enum classes are no longer implicitly converted to ``int``
  (`#1424 <https://github.com/fmtlib/fmt/issues/1424>`_).

* Added ``basic_format_parse_context`` for consistency with C++20
  ``std::format`` and deprecated ``basic_parse_context``.

* Fixed handling of UTF-8 in precision
  (`#1389 <https://github.com/fmtlib/fmt/issues/1389>`_,
  `#1390 <https://github.com/fmtlib/fmt/pull/1390>`_).
  Thanks `@tajtiattila (Attila Tajti) <https://github.com/tajtiattila>`_.

* {fmt} can now be installed on Linux, macOS and Windows with
  `Conda <https://docs.conda.io/en/latest/>`__ using its
  `conda-forge <https://conda-forge.org>`__
  `package <https://github.com/conda-forge/fmt-feedstock>`__
  (`#1410 <https://github.com/fmtlib/fmt/pull/1410>`_)::

    conda install -c conda-forge fmt

  Thanks `@tdegeus (Tom de Geus) <https://github.com/tdegeus>`_.

* Added a CUDA test (`#1285 <https://github.com/fmtlib/fmt/pull/1285>`_,
  `#1317 <https://github.com/fmtlib/fmt/pull/1317>`_).
  Thanks `@luncliff (Park DongHa) <https://github.com/luncliff>`_ and
  `@risa2000 <https://github.com/risa2000>`_.

* Improved documentation (`#1276 <https://github.com/fmtlib/fmt/pull/1276>`_,
  `#1291 <https://github.com/fmtlib/fmt/issues/1291>`_,
  `#1296 <https://github.com/fmtlib/fmt/issues/1296>`_,
  `#1315 <https://github.com/fmtlib/fmt/pull/1315>`_,
  `#1332 <https://github.com/fmtlib/fmt/pull/1332>`_,
  `#1337 <https://github.com/fmtlib/fmt/pull/1337>`_,
  `#1395 <https://github.com/fmtlib/fmt/issues/1395>`_
  `#1418 <https://github.com/fmtlib/fmt/pull/1418>`_).
  Thanks
  `@waywardmonkeys (Bruce Mitchener) <https://github.com/waywardmonkeys>`_,
  `@pauldreik (Paul Dreik) <https://github.com/pauldreik>`_,
  `@jackoalan (Jack Andersen) <https://github.com/jackoalan>`_.

* Various code improvements
  (`#1358 <https://github.com/fmtlib/fmt/pull/1358>`_,
  `#1407 <https://github.com/fmtlib/fmt/pull/1407>`_).
  Thanks `@orivej (Orivej Desh) <https://github.com/orivej>`_,
  `@dpacbach (David P. Sicilia) <https://github.com/dpacbach>`_,

* Fixed compile-time format string checks for user-defined types
  (`#1292 <https://github.com/fmtlib/fmt/issues/1292>`_).

* Worked around a false positive in ``unsigned-integer-overflow`` sanitizer
  (`#1377 <https://github.com/fmtlib/fmt/issues/1377>`_).

* Fixed various warnings and compilation issues
  (`#1273 <https://github.com/fmtlib/fmt/issues/1273>`_,
  `#1278 <https://github.com/fmtlib/fmt/pull/1278>`_,
  `#1280 <https://github.com/fmtlib/fmt/pull/1280>`_,
  `#1281 <https://github.com/fmtlib/fmt/issues/1281>`_,
  `#1288 <https://github.com/fmtlib/fmt/issues/1288>`_,
  `#1290 <https://github.com/fmtlib/fmt/pull/1290>`_,
  `#1301 <https://github.com/fmtlib/fmt/pull/1301>`_,
  `#1305 <https://github.com/fmtlib/fmt/issues/1305>`_,
  `#1306 <https://github.com/fmtlib/fmt/issues/1306>`_,
  `#1309 <https://github.com/fmtlib/fmt/issues/1309>`_,
  `#1312 <https://github.com/fmtlib/fmt/pull/1312>`_,
  `#1313 <https://github.com/fmtlib/fmt/issues/1313>`_,
  `#1316 <https://github.com/fmtlib/fmt/issues/1316>`_,
  `#1319 <https://github.com/fmtlib/fmt/issues/1319>`_,
  `#1320 <https://github.com/fmtlib/fmt/pull/1320>`_,
  `#1326 <https://github.com/fmtlib/fmt/pull/1326>`_,
  `#1328 <https://github.com/fmtlib/fmt/pull/1328>`_,
  `#1344 <https://github.com/fmtlib/fmt/issues/1344>`_,
  `#1345 <https://github.com/fmtlib/fmt/pull/1345>`_,
  `#1347 <https://github.com/fmtlib/fmt/pull/1347>`_,
  `#1349 <https://github.com/fmtlib/fmt/pull/1349>`_,
  `#1354 <https://github.com/fmtlib/fmt/issues/1354>`_,
  `#1362 <https://github.com/fmtlib/fmt/issues/1362>`_,
  `#1366 <https://github.com/fmtlib/fmt/issues/1366>`_,
  `#1364 <https://github.com/fmtlib/fmt/pull/1364>`_,
  `#1370 <https://github.com/fmtlib/fmt/pull/1370>`_,
  `#1371 <https://github.com/fmtlib/fmt/pull/1371>`_,
  `#1385 <https://github.com/fmtlib/fmt/issues/1385>`_,
  `#1388 <https://github.com/fmtlib/fmt/issues/1388>`_,
  `#1397 <https://github.com/fmtlib/fmt/pull/1397>`_,
  `#1414 <https://github.com/fmtlib/fmt/pull/1414>`_,
  `#1416 <https://github.com/fmtlib/fmt/pull/1416>`_,
  `#1422 <https://github.com/fmtlib/fmt/issues/1422>`_
  `#1427 <https://github.com/fmtlib/fmt/pull/1427>`_,
  `#1431 <https://github.com/fmtlib/fmt/issues/1431>`_,
  `#1433 <https://github.com/fmtlib/fmt/pull/1433>`_).
  Thanks `@hhb <https://github.com/hhb>`_,
  `@gsjaardema (Greg Sjaardema) <https://github.com/gsjaardema>`_,
  `@gabime (Gabi Melman) <https://github.com/gabime>`_,
  `@neheb (Rosen Penev) <https://github.com/neheb>`_,
  `@vedranmiletic (Vedran Miletić) <https://github.com/vedranmiletic>`_,
  `@dkavolis (Daumantas Kavolis) <https://github.com/dkavolis>`_,
  `@mwinterb <https://github.com/mwinterb>`_,
  `@orivej (Orivej Desh) <https://github.com/orivej>`_,
  `@denizevrenci (Deniz Evrenci) <https://github.com/denizevrenci>`_
  `@leonklingele <https://github.com/leonklingele>`_,
  `@chronoxor (Ivan Shynkarenka) <https://github.com/chronoxor>`_,
  `@kent-tri <https://github.com/kent-tri>`_,
  `@0x8000-0000 (Florin Iucha) <https://github.com/0x8000-0000>`_,
  `@marti4d (Chris Martin) <https://github.com/marti4d>`_.

6.0.0 - 2019-08-26
------------------

* Switched to the `MIT license
  <https://github.com/fmtlib/fmt/blob/5a4b24613ba16cc689977c3b5bd8274a3ba1dd1f/LICENSE.rst>`_
  with an optional exception that allows distributing binary code without
  attribution.

* Floating-point formatting is now locale-independent by default:

  .. code:: c++

     #include <locale>
     #include <fmt/core.h>

     int main() {
       std::locale::global(std::locale("ru_RU.UTF-8"));
       fmt::print("value = {}", 4.2);
     }

  prints "value = 4.2" regardless of the locale.

  For locale-specific formatting use the ``n`` specifier:

  .. code:: c++

     std::locale::global(std::locale("ru_RU.UTF-8"));
     fmt::print("value = {:n}", 4.2);

  prints "value = 4,2".

* Added an experimental Grisu floating-point formatting algorithm
  implementation (disabled by default). To enable it compile with the
  ``FMT_USE_GRISU`` macro defined to 1:

  .. code:: c++

     #define FMT_USE_GRISU 1
     #include <fmt/format.h>

     auto s = fmt::format("{}", 4.2); // formats 4.2 using Grisu

  With Grisu enabled, {fmt} is 13x faster than ``std::ostringstream`` (libc++)
  and 10x faster than ``sprintf`` on `dtoa-benchmark
  <https://github.com/fmtlib/dtoa-benchmark>`_ (`full results
  <https://fmt.dev/unknown_mac64_clang10.0.html>`_):

  .. image:: https://user-images.githubusercontent.com/576385/
             54883977-9fe8c000-4e28-11e9-8bde-272d122e7c52.jpg

* Separated formatting and parsing contexts for consistency with
  `C++20 std::format <http://eel.is/c++draft/format>`_, removing the
  undocumented ``basic_format_context::parse_context()`` function.

* Added `oss-fuzz <https://github.com/google/oss-fuzz>`_ support
  (`#1199 <https://github.com/fmtlib/fmt/pull/1199>`_).
  Thanks `@pauldreik (Paul Dreik) <https://github.com/pauldreik>`_.

* ``formatter`` specializations now always take precedence over ``operator<<``
  (`#952 <https://github.com/fmtlib/fmt/issues/952>`_):

  .. code:: c++

     #include <iostream>
     #include <fmt/ostream.h>

     struct S {};

     std::ostream& operator<<(std::ostream& os, S) {
       return os << 1;
     }

     template <>
     struct fmt::formatter<S> : fmt::formatter<int> {
       auto format(S, format_context& ctx) {
         return formatter<int>::format(2, ctx);
       }
     };

     int main() {
       std::cout << S() << "\n"; // prints 1 using operator<<
       fmt::print("{}\n", S());  // prints 2 using formatter
     }

* Introduced the experimental ``fmt::compile`` function that does format string
  compilation (`#618 <https://github.com/fmtlib/fmt/issues/618>`_,
  `#1169 <https://github.com/fmtlib/fmt/issues/1169>`_,
  `#1171 <https://github.com/fmtlib/fmt/pull/1171>`_):

  .. code:: c++

     #include <fmt/compile.h>

     auto f = fmt::compile<int>("{}");
     std::string s = fmt::format(f, 42); // can be called multiple times to
                                         // format different values
     // s == "42"

  It moves the cost of parsing a format string outside of the format function
  which can be beneficial when identically formatting many objects of the same
  types. Thanks `@stryku (Mateusz Janek) <https://github.com/stryku>`_.

* Added experimental ``%`` format specifier that formats floating-point values
  as percentages (`#1060 <https://github.com/fmtlib/fmt/pull/1060>`_,
  `#1069 <https://github.com/fmtlib/fmt/pull/1069>`_,
  `#1071 <https://github.com/fmtlib/fmt/pull/1071>`_):

  .. code:: c++

     auto s = fmt::format("{:.1%}", 0.42); // s == "42.0%"

  Thanks `@gawain-bolton (Gawain Bolton) <https://github.com/gawain-bolton>`_.

* Implemented precision for floating-point durations
  (`#1004 <https://github.com/fmtlib/fmt/issues/1004>`_,
  `#1012 <https://github.com/fmtlib/fmt/pull/1012>`_):

  .. code:: c++

     auto s = fmt::format("{:.1}", std::chrono::duration<double>(1.234));
     // s == 1.2s

  Thanks `@DanielaE (Daniela Engert) <https://github.com/DanielaE>`_.

* Implemented ``chrono`` format specifiers ``%Q`` and ``%q`` that give the value
  and the unit respectively (`#1019 <https://github.com/fmtlib/fmt/pull/1019>`_):

  .. code:: c++

     auto value = fmt::format("{:%Q}", 42s); // value == "42"
     auto unit  = fmt::format("{:%q}", 42s); // unit == "s"

  Thanks `@DanielaE (Daniela Engert) <https://github.com/DanielaE>`_.

* Fixed handling of dynamic width in chrono formatter:

  .. code:: c++

     auto s = fmt::format("{0:{1}%H:%M:%S}", std::chrono::seconds(12345), 12);
     //                        ^ width argument index                     ^ width
     // s == "03:25:45    "

  Thanks Howard Hinnant.

* Removed deprecated ``fmt/time.h``. Use ``fmt/chrono.h`` instead.

* Added ``fmt::format`` and ``fmt::vformat`` overloads that take ``text_style``
  (`#993 <https://github.com/fmtlib/fmt/issues/993>`_,
  `#994 <https://github.com/fmtlib/fmt/pull/994>`_):

  .. code:: c++

     #include <fmt/color.h>

     std::string message = fmt::format(fmt::emphasis::bold | fg(fmt::color::red),
                                       "The answer is {}.", 42);

  Thanks `@Naios (Denis Blank) <https://github.com/Naios>`_.

* Removed the deprecated color API (``print_colored``). Use the new API, namely
  ``print`` overloads that take ``text_style`` instead.

* Made ``std::unique_ptr`` and ``std::shared_ptr`` formattable as pointers via
  ``fmt::ptr`` (`#1121 <https://github.com/fmtlib/fmt/pull/1121>`_):

  .. code:: c++

     std::unique_ptr<int> p = ...;
     fmt::print("{}", fmt::ptr(p)); // prints p as a pointer

  Thanks `@sighingnow (Tao He) <https://github.com/sighingnow>`_.

* Made ``print`` and ``vprint`` report I/O errors
  (`#1098 <https://github.com/fmtlib/fmt/issues/1098>`_,
  `#1099 <https://github.com/fmtlib/fmt/pull/1099>`_).
  Thanks `@BillyDonahue (Billy Donahue) <https://github.com/BillyDonahue>`_.

* Marked deprecated APIs with the ``[[deprecated]]`` attribute and removed
  internal uses of deprecated APIs
  (`#1022 <https://github.com/fmtlib/fmt/pull/1022>`_).
  Thanks `@eliaskosunen (Elias Kosunen) <https://github.com/eliaskosunen>`_.

* Modernized the codebase using more C++11 features and removing workarounds.
  Most importantly, ``buffer_context`` is now an alias template, so
  use ``buffer_context<T>`` instead of ``buffer_context<T>::type``.
  These features require GCC 4.8 or later.

* ``formatter`` specializations now always take precedence over implicit
  conversions to ``int`` and the undocumented ``convert_to_int`` trait
  is now deprecated.

* Moved the undocumented ``basic_writer``, ``writer``, and ``wwriter`` types
  to the ``internal`` namespace.

* Removed deprecated ``basic_format_context::begin()``. Use ``out()`` instead.

* Disallowed passing the result of ``join`` as an lvalue to prevent misuse.

* Refactored the undocumented structs that represent parsed format specifiers
  to simplify the API and allow multibyte fill.

* Moved SFINAE to template parameters to reduce symbol sizes.

* Switched to ``fputws`` for writing wide strings so that it's no longer
  required to call ``_setmode`` on Windows
  (`#1229 <https://github.com/fmtlib/fmt/issues/1229>`_,
  `#1243 <https://github.com/fmtlib/fmt/pull/1243>`_).
  Thanks `@jackoalan (Jack Andersen) <https://github.com/jackoalan>`_.

* Improved literal-based API
  (`#1254 <https://github.com/fmtlib/fmt/pull/1254>`_).
  Thanks `@sylveon (Charles Milette) <https://github.com/sylveon>`_.

* Added support for exotic platforms without ``uintptr_t`` such as IBM i
  (AS/400) which has 128-bit pointers and only 64-bit integers
  (`#1059 <https://github.com/fmtlib/fmt/issues/1059>`_).

* Added `Sublime Text syntax highlighting config
  <https://github.com/fmtlib/fmt/blob/master/support/C%2B%2B.sublime-syntax>`_
  (`#1037 <https://github.com/fmtlib/fmt/issues/1037>`_).
  Thanks `@Kronuz (Germán Méndez Bravo) <https://github.com/Kronuz>`_.

* Added the ``FMT_ENFORCE_COMPILE_STRING`` macro to enforce the use of
  compile-time format strings
  (`#1231 <https://github.com/fmtlib/fmt/pull/1231>`_).
  Thanks `@jackoalan (Jack Andersen) <https://github.com/jackoalan>`_.

* Stopped setting ``CMAKE_BUILD_TYPE`` if {fmt} is a subproject
  (`#1081 <https://github.com/fmtlib/fmt/issues/1081>`_).

* Various build improvements
  (`#1039 <https://github.com/fmtlib/fmt/pull/1039>`_,
  `#1078 <https://github.com/fmtlib/fmt/pull/1078>`_,
  `#1091 <https://github.com/fmtlib/fmt/pull/1091>`_,
  `#1103 <https://github.com/fmtlib/fmt/pull/1103>`_,
  `#1177 <https://github.com/fmtlib/fmt/pull/1177>`_).
  Thanks `@luncliff (Park DongHa) <https://github.com/luncliff>`_,
  `@jasonszang (Jason Shuo Zang) <https://github.com/jasonszang>`_,
  `@olafhering (Olaf Hering) <https://github.com/olafhering>`_,
  `@Lecetem <https://github.com/Lectem>`_,
  `@pauldreik (Paul Dreik) <https://github.com/pauldreik>`_.

* Improved documentation
  (`#1049 <https://github.com/fmtlib/fmt/issues/1049>`_,
  `#1051 <https://github.com/fmtlib/fmt/pull/1051>`_,
  `#1083 <https://github.com/fmtlib/fmt/pull/1083>`_,
  `#1113 <https://github.com/fmtlib/fmt/pull/1113>`_,
  `#1114 <https://github.com/fmtlib/fmt/pull/1114>`_,
  `#1146 <https://github.com/fmtlib/fmt/issues/1146>`_,
  `#1180 <https://github.com/fmtlib/fmt/issues/1180>`_,
  `#1250 <https://github.com/fmtlib/fmt/pull/1250>`_,
  `#1252 <https://github.com/fmtlib/fmt/pull/1252>`_,
  `#1265 <https://github.com/fmtlib/fmt/pull/1265>`_).
  Thanks `@mikelui (Michael Lui) <https://github.com/mikelui>`_,
  `@foonathan (Jonathan Müller) <https://github.com/foonathan>`_,
  `@BillyDonahue (Billy Donahue) <https://github.com/BillyDonahue>`_,
  `@jwakely (Jonathan Wakely) <https://github.com/jwakely>`_,
  `@kaisbe (Kais Ben Salah) <https://github.com/kaisbe>`_,
  `@sdebionne (Samuel Debionne) <https://github.com/sdebionne>`_.

* Fixed ambiguous formatter specialization in ``fmt/ranges.h``
  (`#1123 <https://github.com/fmtlib/fmt/issues/1123>`_).

* Fixed formatting of a non-empty ``std::filesystem::path`` which is an
  infinitely deep range of its components
  (`#1268 <https://github.com/fmtlib/fmt/issues/1268>`_).

* Fixed handling of general output iterators when formatting characters
  (`#1056 <https://github.com/fmtlib/fmt/issues/1056>`_,
  `#1058 <https://github.com/fmtlib/fmt/pull/1058>`_).
  Thanks `@abolz (Alexander Bolz) <https://github.com/abolz>`_.

* Fixed handling of output iterators in ``formatter`` specialization for
  ranges (`#1064 <https://github.com/fmtlib/fmt/issues/1064>`_).

* Fixed handling of exotic character types
  (`#1188 <https://github.com/fmtlib/fmt/issues/1188>`_).

* Made chrono formatting work with exceptions disabled
  (`#1062 <https://github.com/fmtlib/fmt/issues/1062>`_).

* Fixed DLL visibility issues
  (`#1134 <https://github.com/fmtlib/fmt/pull/1134>`_,
  `#1147 <https://github.com/fmtlib/fmt/pull/1147>`_).
  Thanks `@denchat <https://github.com/denchat>`_.

* Disabled the use of UDL template extension on GCC 9
  (`#1148 <https://github.com/fmtlib/fmt/issues/1148>`_).

* Removed misplaced ``format`` compile-time checks from ``printf``
  (`#1173 <https://github.com/fmtlib/fmt/issues/1173>`_).

* Fixed issues in the experimental floating-point formatter
  (`#1072 <https://github.com/fmtlib/fmt/issues/1072>`_,
  `#1129 <https://github.com/fmtlib/fmt/issues/1129>`_,
  `#1153 <https://github.com/fmtlib/fmt/issues/1153>`_,
  `#1155 <https://github.com/fmtlib/fmt/pull/1155>`_,
  `#1210 <https://github.com/fmtlib/fmt/issues/1210>`_,
  `#1222 <https://github.com/fmtlib/fmt/issues/1222>`_).
  Thanks `@alabuzhev (Alex Alabuzhev) <https://github.com/alabuzhev>`_.

* Fixed bugs discovered by fuzzing or during fuzzing integration
  (`#1124 <https://github.com/fmtlib/fmt/issues/1124>`_,
  `#1127 <https://github.com/fmtlib/fmt/issues/1127>`_,
  `#1132 <https://github.com/fmtlib/fmt/issues/1132>`_,
  `#1135 <https://github.com/fmtlib/fmt/pull/1135>`_,
  `#1136 <https://github.com/fmtlib/fmt/issues/1136>`_,
  `#1141 <https://github.com/fmtlib/fmt/issues/1141>`_,
  `#1142 <https://github.com/fmtlib/fmt/issues/1142>`_,
  `#1178 <https://github.com/fmtlib/fmt/issues/1178>`_,
  `#1179 <https://github.com/fmtlib/fmt/issues/1179>`_,
  `#1194 <https://github.com/fmtlib/fmt/issues/1194>`_).
  Thanks `@pauldreik (Paul Dreik) <https://github.com/pauldreik>`_.

* Fixed building tests on FreeBSD and Hurd
  (`#1043 <https://github.com/fmtlib/fmt/issues/1043>`_).
  Thanks `@jackyf (Eugene V. Lyubimkin) <https://github.com/jackyf>`_.

* Fixed various warnings and compilation issues
  (`#998 <https://github.com/fmtlib/fmt/pull/998>`_,
  `#1006 <https://github.com/fmtlib/fmt/pull/1006>`_,
  `#1008 <https://github.com/fmtlib/fmt/issues/1008>`_,
  `#1011 <https://github.com/fmtlib/fmt/issues/1011>`_,
  `#1025 <https://github.com/fmtlib/fmt/issues/1025>`_,
  `#1027 <https://github.com/fmtlib/fmt/pull/1027>`_,
  `#1028 <https://github.com/fmtlib/fmt/pull/1028>`_,
  `#1029 <https://github.com/fmtlib/fmt/pull/1029>`_,
  `#1030 <https://github.com/fmtlib/fmt/pull/1030>`_,
  `#1031 <https://github.com/fmtlib/fmt/pull/1031>`_,
  `#1054 <https://github.com/fmtlib/fmt/pull/1054>`_,
  `#1063 <https://github.com/fmtlib/fmt/issues/1063>`_,
  `#1068 <https://github.com/fmtlib/fmt/pull/1068>`_,
  `#1074 <https://github.com/fmtlib/fmt/pull/1074>`_,
  `#1075 <https://github.com/fmtlib/fmt/pull/1075>`_,
  `#1079 <https://github.com/fmtlib/fmt/pull/1079>`_,
  `#1086 <https://github.com/fmtlib/fmt/pull/1086>`_,
  `#1088 <https://github.com/fmtlib/fmt/issues/1088>`_,
  `#1089 <https://github.com/fmtlib/fmt/pull/1089>`_,
  `#1094 <https://github.com/fmtlib/fmt/pull/1094>`_,
  `#1101 <https://github.com/fmtlib/fmt/issues/1101>`_,
  `#1102 <https://github.com/fmtlib/fmt/pull/1102>`_,
  `#1105 <https://github.com/fmtlib/fmt/issues/1105>`_,
  `#1107 <https://github.com/fmtlib/fmt/pull/1107>`_,
  `#1115 <https://github.com/fmtlib/fmt/issues/1115>`_,
  `#1117 <https://github.com/fmtlib/fmt/issues/1117>`_,
  `#1118 <https://github.com/fmtlib/fmt/issues/1118>`_,
  `#1120 <https://github.com/fmtlib/fmt/issues/1120>`_,
  `#1123 <https://github.com/fmtlib/fmt/issues/1123>`_,
  `#1139 <https://github.com/fmtlib/fmt/pull/1139>`_,
  `#1140 <https://github.com/fmtlib/fmt/issues/1140>`_,
  `#1143 <https://github.com/fmtlib/fmt/issues/1143>`_,
  `#1144 <https://github.com/fmtlib/fmt/pull/1144>`_,
  `#1150 <https://github.com/fmtlib/fmt/pull/1150>`_,
  `#1151 <https://github.com/fmtlib/fmt/pull/1151>`_,
  `#1152 <https://github.com/fmtlib/fmt/issues/1152>`_,
  `#1154 <https://github.com/fmtlib/fmt/issues/1154>`_,
  `#1156 <https://github.com/fmtlib/fmt/issues/1156>`_,
  `#1159 <https://github.com/fmtlib/fmt/pull/1159>`_,
  `#1175 <https://github.com/fmtlib/fmt/issues/1175>`_,
  `#1181 <https://github.com/fmtlib/fmt/issues/1181>`_,
  `#1186 <https://github.com/fmtlib/fmt/issues/1186>`_,
  `#1187 <https://github.com/fmtlib/fmt/pull/1187>`_,
  `#1191 <https://github.com/fmtlib/fmt/pull/1191>`_,
  `#1197 <https://github.com/fmtlib/fmt/issues/1197>`_,
  `#1200 <https://github.com/fmtlib/fmt/issues/1200>`_,
  `#1203 <https://github.com/fmtlib/fmt/issues/1203>`_,
  `#1205 <https://github.com/fmtlib/fmt/issues/1205>`_,
  `#1206 <https://github.com/fmtlib/fmt/pull/1206>`_,
  `#1213 <https://github.com/fmtlib/fmt/issues/1213>`_,
  `#1214 <https://github.com/fmtlib/fmt/issues/1214>`_,
  `#1217 <https://github.com/fmtlib/fmt/pull/1217>`_,
  `#1228 <https://github.com/fmtlib/fmt/issues/1228>`_,
  `#1230 <https://github.com/fmtlib/fmt/pull/1230>`_,
  `#1232 <https://github.com/fmtlib/fmt/issues/1232>`_,
  `#1235 <https://github.com/fmtlib/fmt/pull/1235>`_,
  `#1236 <https://github.com/fmtlib/fmt/pull/1236>`_,
  `#1240 <https://github.com/fmtlib/fmt/issues/1240>`_).
  Thanks `@DanielaE (Daniela Engert) <https://github.com/DanielaE>`_,
  `@mwinterb <https://github.com/mwinterb>`_,
  `@eliaskosunen (Elias Kosunen) <https://github.com/eliaskosunen>`_,
  `@morinmorin <https://github.com/morinmorin>`_,
  `@ricco19 (Brian Ricciardelli) <https://github.com/ricco19>`_,
  `@waywardmonkeys (Bruce Mitchener) <https://github.com/waywardmonkeys>`_,
  `@chronoxor (Ivan Shynkarenka) <https://github.com/chronoxor>`_,
  `@remyabel <https://github.com/remyabel>`_,
  `@pauldreik (Paul Dreik) <https://github.com/pauldreik>`_,
  `@gsjaardema (Greg Sjaardema) <https://github.com/gsjaardema>`_,
  `@rcane (Ronny Krüger) <https://github.com/rcane>`_,
  `@mocabe <https://github.com/mocabe>`_,
  `@denchat <https://github.com/denchat>`_,
  `@cjdb (Christopher Di Bella) <https://github.com/cjdb>`_,
  `@HazardyKnusperkeks (Björn Schäpers) <https://github.com/HazardyKnusperkeks>`_,
  `@vedranmiletic (Vedran Miletić) <https://github.com/vedranmiletic>`_,
  `@jackoalan (Jack Andersen) <https://github.com/jackoalan>`_,
  `@DaanDeMeyer (Daan De Meyer) <https://github.com/DaanDeMeyer>`_,
  `@starkmapper (Mark Stapper) <https://github.com/starkmapper>`_.

5.3.0 - 2018-12-28
------------------

* Introduced experimental chrono formatting support:

  .. code:: c++

     #include <fmt/chrono.h>

     int main() {
       using namespace std::literals::chrono_literals;
       fmt::print("Default format: {} {}\n", 42s, 100ms);
       fmt::print("strftime-like format: {:%H:%M:%S}\n", 3h + 15min + 30s);
     }

  prints::

     Default format: 42s 100ms
     strftime-like format: 03:15:30

* Added experimental support for emphasis (bold, italic, underline,
  strikethrough), colored output to a file stream, and improved colored
  formatting API
  (`#961 <https://github.com/fmtlib/fmt/pull/961>`_,
  `#967 <https://github.com/fmtlib/fmt/pull/967>`_,
  `#973 <https://github.com/fmtlib/fmt/pull/973>`_):

  .. code:: c++

     #include <fmt/color.h>

     int main() {
       print(fg(fmt::color::crimson) | fmt::emphasis::bold,
             "Hello, {}!\n", "world");
       print(fg(fmt::color::floral_white) | bg(fmt::color::slate_gray) |
             fmt::emphasis::underline, "Hello, {}!\n", "мир");
       print(fg(fmt::color::steel_blue) | fmt::emphasis::italic,
             "Hello, {}!\n", "世界");
     }

  prints the following on modern terminals with RGB color support:

  .. image:: https://user-images.githubusercontent.com/576385/
             50405788-b66e7500-076e-11e9-9592-7324d1f951d8.png

  Thanks `@Rakete1111 (Nicolas) <https://github.com/Rakete1111>`_.

* Added support for 4-bit terminal colors
  (`#968 <https://github.com/fmtlib/fmt/issues/968>`_,
  `#974 <https://github.com/fmtlib/fmt/pull/974>`_)

  .. code:: c++

     #include <fmt/color.h>

     int main() {
       print(fg(fmt::terminal_color::red), "stop\n");
     }

  Note that these colors vary by terminal:

  .. image:: https://user-images.githubusercontent.com/576385/
             50405925-dbfc7e00-0770-11e9-9b85-333fab0af9ac.png

  Thanks `@Rakete1111 (Nicolas) <https://github.com/Rakete1111>`_.

* Parameterized formatting functions on the type of the format string
  (`#880 <https://github.com/fmtlib/fmt/issues/880>`_,
  `#881 <https://github.com/fmtlib/fmt/pull/881>`_,
  `#883 <https://github.com/fmtlib/fmt/pull/883>`_,
  `#885 <https://github.com/fmtlib/fmt/pull/885>`_,
  `#897 <https://github.com/fmtlib/fmt/pull/897>`_,
  `#920 <https://github.com/fmtlib/fmt/issues/920>`_).
  Any object of type ``S`` that has an overloaded ``to_string_view(const S&)``
  returning ``fmt::string_view`` can be used as a format string:

  .. code:: c++

     namespace my_ns {
     inline string_view to_string_view(const my_string& s) {
       return {s.data(), s.length()};
     }
     }

     std::string message = fmt::format(my_string("The answer is {}."), 42);

  Thanks `@DanielaE (Daniela Engert) <https://github.com/DanielaE>`_.

* Made ``std::string_view`` work as a format string
  (`#898 <https://github.com/fmtlib/fmt/pull/898>`_):

  .. code:: c++

     auto message = fmt::format(std::string_view("The answer is {}."), 42);

  Thanks `@DanielaE (Daniela Engert) <https://github.com/DanielaE>`_.

* Added wide string support to compile-time format string checks
  (`#924 <https://github.com/fmtlib/fmt/pull/924>`_):

  .. code:: c++

     print(fmt(L"{:f}"), 42); // compile-time error: invalid type specifier

  Thanks `@XZiar <https://github.com/XZiar>`_.

* Made colored print functions work with wide strings
  (`#867 <https://github.com/fmtlib/fmt/pull/867>`_):

  .. code:: c++

     #include <fmt/color.h>

     int main() {
       print(fg(fmt::color::red), L"{}\n", 42);
     }

  Thanks `@DanielaE (Daniela Engert) <https://github.com/DanielaE>`_.

* Introduced experimental Unicode support
  (`#628 <https://github.com/fmtlib/fmt/issues/628>`_,
  `#891 <https://github.com/fmtlib/fmt/pull/891>`_):

  .. code:: c++

     using namespace fmt::literals;
     auto s = fmt::format("{:*^5}"_u, "🤡"_u); // s == "**🤡**"_u

* Improved locale support:

  .. code:: c++

     #include <fmt/locale.h>

     struct numpunct : std::numpunct<char> {
      protected:
       char do_thousands_sep() const override { return '~'; }
     };

     std::locale loc;
     auto s = fmt::format(std::locale(loc, new numpunct()), "{:n}", 1234567);
     // s == "1~234~567"

* Constrained formatting functions on proper iterator types
  (`#921 <https://github.com/fmtlib/fmt/pull/921>`_).
  Thanks `@DanielaE (Daniela Engert) <https://github.com/DanielaE>`_.

* Added ``make_printf_args`` and ``make_wprintf_args`` functions
  (`#934 <https://github.com/fmtlib/fmt/pull/934>`_).
  Thanks `@tnovotny <https://github.com/tnovotny>`_.

* Deprecated ``fmt::visit``, ``parse_context``, and ``wparse_context``.
  Use ``fmt::visit_format_arg``, ``format_parse_context``, and
  ``wformat_parse_context`` instead.

* Removed undocumented ``basic_fixed_buffer`` which has been superseded by the
  iterator-based API
  (`#873 <https://github.com/fmtlib/fmt/issues/873>`_,
  `#902 <https://github.com/fmtlib/fmt/pull/902>`_).
  Thanks `@superfunc (hollywood programmer) <https://github.com/superfunc>`_.

* Disallowed repeated leading zeros in an argument ID:

  .. code:: c++

     fmt::print("{000}", 42); // error

* Reintroduced support for gcc 4.4.

* Fixed compilation on platforms with exotic ``double``
  (`#878 <https://github.com/fmtlib/fmt/issues/878>`_).

* Improved documentation
  (`#164 <https://github.com/fmtlib/fmt/issues/164>`_,
  `#877 <https://github.com/fmtlib/fmt/issues/877>`_,
  `#901 <https://github.com/fmtlib/fmt/pull/901>`_,
  `#906 <https://github.com/fmtlib/fmt/pull/906>`_,
  `#979 <https://github.com/fmtlib/fmt/pull/979>`_).
  Thanks `@kookjr (Mathew Cucuzella) <https://github.com/kookjr>`_,
  `@DarkDimius (Dmitry Petrashko) <https://github.com/DarkDimius>`_,
  `@HecticSerenity <https://github.com/HecticSerenity>`_.

* Added pkgconfig support which makes it easier to consume the library from
  meson and other build systems
  (`#916 <https://github.com/fmtlib/fmt/pull/916>`_).
  Thanks `@colemickens (Cole Mickens) <https://github.com/colemickens>`_.

* Various build improvements
  (`#909 <https://github.com/fmtlib/fmt/pull/909>`_,
  `#926 <https://github.com/fmtlib/fmt/pull/926>`_,
  `#937 <https://github.com/fmtlib/fmt/pull/937>`_,
  `#953 <https://github.com/fmtlib/fmt/pull/953>`_,
  `#959 <https://github.com/fmtlib/fmt/pull/959>`_).
  Thanks `@tchaikov (Kefu Chai) <https://github.com/tchaikov>`_,
  `@luncliff (Park DongHa) <https://github.com/luncliff>`_,
  `@AndreasSchoenle (Andreas Schönle) <https://github.com/AndreasSchoenle>`_,
  `@hotwatermorning <https://github.com/hotwatermorning>`_,
  `@Zefz (JohanJansen) <https://github.com/Zefz>`_.

* Improved ``string_view`` construction performance
  (`#914 <https://github.com/fmtlib/fmt/pull/914>`_).
  Thanks `@gabime (Gabi Melman) <https://github.com/gabime>`_.

* Fixed non-matching char types
  (`#895 <https://github.com/fmtlib/fmt/pull/895>`_).
  Thanks `@DanielaE (Daniela Engert) <https://github.com/DanielaE>`_.

* Fixed ``format_to_n`` with ``std::back_insert_iterator``
  (`#913 <https://github.com/fmtlib/fmt/pull/913>`_).
  Thanks `@DanielaE (Daniela Engert) <https://github.com/DanielaE>`_.

* Fixed locale-dependent formatting
  (`#905 <https://github.com/fmtlib/fmt/issues/905>`_).

* Fixed various compiler warnings and errors
  (`#882 <https://github.com/fmtlib/fmt/pull/882>`_,
  `#886 <https://github.com/fmtlib/fmt/pull/886>`_,
  `#933 <https://github.com/fmtlib/fmt/pull/933>`_,
  `#941 <https://github.com/fmtlib/fmt/pull/941>`_,
  `#931 <https://github.com/fmtlib/fmt/issues/931>`_,
  `#943 <https://github.com/fmtlib/fmt/pull/943>`_,
  `#954 <https://github.com/fmtlib/fmt/pull/954>`_,
  `#956 <https://github.com/fmtlib/fmt/pull/956>`_,
  `#962 <https://github.com/fmtlib/fmt/pull/962>`_,
  `#965 <https://github.com/fmtlib/fmt/issues/965>`_,
  `#977 <https://github.com/fmtlib/fmt/issues/977>`_,
  `#983 <https://github.com/fmtlib/fmt/pull/983>`_,
  `#989 <https://github.com/fmtlib/fmt/pull/989>`_).
  Thanks `@Luthaf (Guillaume Fraux) <https://github.com/Luthaf>`_,
  `@stevenhoving (Steven Hoving) <https://github.com/stevenhoving>`_,
  `@christinaa (Kristina Brooks) <https://github.com/christinaa>`_,
  `@lgritz (Larry Gritz) <https://github.com/lgritz>`_,
  `@DanielaE (Daniela Engert) <https://github.com/DanielaE>`_,
  `@0x8000-0000 (Sign Bit) <https://github.com/0x8000-0000>`_,
  `@liuping1997 <https://github.com/liuping1997>`_.

5.2.1 - 2018-09-21
------------------

* Fixed ``visit`` lookup issues on gcc 7 & 8
  (`#870 <https://github.com/fmtlib/fmt/pull/870>`_).
  Thanks `@medithe <https://github.com/medithe>`_.

* Fixed linkage errors on older gcc.

* Prevented ``fmt/range.h`` from specializing ``fmt::basic_string_view``
  (`#865 <https://github.com/fmtlib/fmt/issues/865>`_,
  `#868 <https://github.com/fmtlib/fmt/pull/868>`_).
  Thanks `@hhggit (dual) <https://github.com/hhggit>`_.

* Improved error message when formatting unknown types
  (`#872 <https://github.com/fmtlib/fmt/pull/872>`_).
  Thanks `@foonathan (Jonathan Müller) <https://github.com/foonathan>`_,

* Disabled templated user-defined literals when compiled under nvcc
  (`#875 <https://github.com/fmtlib/fmt/pull/875>`_).
  Thanks `@CandyGumdrop (Candy Gumdrop) <https://github.com/CandyGumdrop>`_,

* Fixed ``format_to`` formatting to ``wmemory_buffer``
  (`#874 <https://github.com/fmtlib/fmt/issues/874>`_).

5.2.0 - 2018-09-13
------------------

* Optimized format string parsing and argument processing which resulted in up
  to 5x speed up on long format strings and significant performance boost on
  various benchmarks. For example, version 5.2 is 2.22x faster than 5.1 on
  decimal integer formatting with ``format_to`` (macOS, clang-902.0.39.2):

  ==================  =======  =======
  Method              Time, s  Speedup
  ==================  =======  =======
  fmt::format 5.1      0.58
  fmt::format 5.2      0.35     1.66x
  fmt::format_to 5.1   0.51
  fmt::format_to 5.2   0.23     2.22x
  sprintf              0.71
  std::to_string       1.01
  std::stringstream    1.73
  ==================  =======  =======

* Changed the ``fmt`` macro from opt-out to opt-in to prevent name collisions.
  To enable it define the ``FMT_STRING_ALIAS`` macro to 1 before including
  ``fmt/format.h``:

  .. code:: c++

     #define FMT_STRING_ALIAS 1
     #include <fmt/format.h>
     std::string answer = format(fmt("{}"), 42);

* Added compile-time format string checks to ``format_to`` overload that takes
  ``fmt::memory_buffer`` (`#783 <https://github.com/fmtlib/fmt/issues/783>`_):

  .. code:: c++

     fmt::memory_buffer buf;
     // Compile-time error: invalid type specifier.
     fmt::format_to(buf, fmt("{:d}"), "foo");

* Moved experimental color support to ``fmt/color.h`` and enabled the
  new API by default. The old API can be enabled by defining the
  ``FMT_DEPRECATED_COLORS`` macro.

* Added formatting support for types explicitly convertible to
  ``fmt::string_view``:

  .. code:: c++

     struct foo {
       explicit operator fmt::string_view() const { return "foo"; }
     };
     auto s = format("{}", foo());

  In particular, this makes formatting function work with
  ``folly::StringPiece``.

* Implemented preliminary support for ``char*_t`` by replacing the ``format``
  function overloads with a single function template parameterized on the string
  type.

* Added support for dynamic argument lists
  (`#814 <https://github.com/fmtlib/fmt/issues/814>`_,
  `#819 <https://github.com/fmtlib/fmt/pull/819>`_).
  Thanks `@MikePopoloski (Michael Popoloski)
  <https://github.com/MikePopoloski>`_.

* Reduced executable size overhead for embedded targets using newlib nano by
  making locale dependency optional
  (`#839 <https://github.com/fmtlib/fmt/pull/839>`_).
  Thanks `@teajay-fr (Thomas Benard) <https://github.com/teajay-fr>`_.

* Keep ``noexcept`` specifier when exceptions are disabled
  (`#801 <https://github.com/fmtlib/fmt/issues/801>`_,
  `#810 <https://github.com/fmtlib/fmt/pull/810>`_).
  Thanks `@qis (Alexej Harm) <https://github.com/qis>`_.

* Fixed formatting of user-defined types providing ``operator<<`` with
  ``format_to_n``
  (`#806 <https://github.com/fmtlib/fmt/pull/806>`_).
  Thanks `@mkurdej (Marek Kurdej) <https://github.com/mkurdej>`_.

* Fixed dynamic linkage of new symbols
  (`#808 <https://github.com/fmtlib/fmt/issues/808>`_).

* Fixed global initialization issue
  (`#807 <https://github.com/fmtlib/fmt/issues/807>`_):

  .. code:: c++

     // This works on compilers with constexpr support.
     static const std::string answer = fmt::format("{}", 42);

* Fixed various compiler warnings and errors
  (`#804 <https://github.com/fmtlib/fmt/pull/804>`_,
  `#809 <https://github.com/fmtlib/fmt/issues/809>`_,
  `#811 <https://github.com/fmtlib/fmt/pull/811>`_,
  `#822 <https://github.com/fmtlib/fmt/issues/822>`_,
  `#827 <https://github.com/fmtlib/fmt/pull/827>`_,
  `#830 <https://github.com/fmtlib/fmt/issues/830>`_,
  `#838 <https://github.com/fmtlib/fmt/pull/838>`_,
  `#843 <https://github.com/fmtlib/fmt/issues/843>`_,
  `#844 <https://github.com/fmtlib/fmt/pull/844>`_,
  `#851 <https://github.com/fmtlib/fmt/issues/851>`_,
  `#852 <https://github.com/fmtlib/fmt/pull/852>`_,
  `#854 <https://github.com/fmtlib/fmt/pull/854>`_).
  Thanks `@henryiii (Henry Schreiner) <https://github.com/henryiii>`_,
  `@medithe <https://github.com/medithe>`_, and
  `@eliasdaler (Elias Daler) <https://github.com/eliasdaler>`_.

5.1.0 - 2018-07-05
------------------

* Added experimental support for RGB color output enabled with
  the ``FMT_EXTENDED_COLORS`` macro:

  .. code:: c++

     #define FMT_EXTENDED_COLORS
     #define FMT_HEADER_ONLY // or compile fmt with FMT_EXTENDED_COLORS defined
     #include <fmt/format.h>

     fmt::print(fmt::color::steel_blue, "Some beautiful text");

  The old API (the ``print_colored`` and ``vprint_colored`` functions and the
  ``color`` enum) is now deprecated.
  (`#762 <https://github.com/fmtlib/fmt/issues/762>`_
  `#767 <https://github.com/fmtlib/fmt/pull/767>`_).
  thanks `@Remotion (Remo) <https://github.com/Remotion>`_.

* Added quotes to strings in ranges and tuples
  (`#766 <https://github.com/fmtlib/fmt/pull/766>`_).
  Thanks `@Remotion (Remo) <https://github.com/Remotion>`_.

* Made ``format_to`` work with ``basic_memory_buffer``
  (`#776 <https://github.com/fmtlib/fmt/issues/776>`_).

* Added ``vformat_to_n`` and ``wchar_t`` overload of ``format_to_n``
  (`#764 <https://github.com/fmtlib/fmt/issues/764>`_,
  `#769 <https://github.com/fmtlib/fmt/issues/769>`_).

* Made ``is_range`` and ``is_tuple_like`` part of public (experimental) API
  to allow specialization for user-defined types
  (`#751 <https://github.com/fmtlib/fmt/issues/751>`_,
  `#759 <https://github.com/fmtlib/fmt/pull/759>`_).
  Thanks `@drrlvn (Dror Levin) <https://github.com/drrlvn>`_.

* Added more compilers to continuous integration and increased ``FMT_PEDANTIC``
  warning levels
  (`#736 <https://github.com/fmtlib/fmt/pull/736>`_).
  Thanks `@eliaskosunen (Elias Kosunen) <https://github.com/eliaskosunen>`_.

* Fixed compilation with MSVC 2013.

* Fixed handling of user-defined types in ``format_to``
  (`#793 <https://github.com/fmtlib/fmt/issues/793>`_).

* Forced linking of inline ``vformat`` functions into the library
  (`#795 <https://github.com/fmtlib/fmt/issues/795>`_).

* Fixed incorrect call to on_align in ``'{:}='``
  (`#750 <https://github.com/fmtlib/fmt/issues/750>`_).

* Fixed floating-point formatting to a non-back_insert_iterator with sign &
  numeric alignment specified
  (`#756 <https://github.com/fmtlib/fmt/issues/756>`_).

* Fixed formatting to an array with ``format_to_n``
  (`#778 <https://github.com/fmtlib/fmt/issues/778>`_).

* Fixed formatting of more than 15 named arguments
  (`#754 <https://github.com/fmtlib/fmt/issues/754>`_).

* Fixed handling of compile-time strings when including ``fmt/ostream.h``.
  (`#768 <https://github.com/fmtlib/fmt/issues/768>`_).

* Fixed various compiler warnings and errors
  (`#742 <https://github.com/fmtlib/fmt/issues/742>`_,
  `#748 <https://github.com/fmtlib/fmt/issues/748>`_,
  `#752 <https://github.com/fmtlib/fmt/issues/752>`_,
  `#770 <https://github.com/fmtlib/fmt/issues/770>`_,
  `#775 <https://github.com/fmtlib/fmt/pull/775>`_,
  `#779 <https://github.com/fmtlib/fmt/issues/779>`_,
  `#780 <https://github.com/fmtlib/fmt/pull/780>`_,
  `#790 <https://github.com/fmtlib/fmt/pull/790>`_,
  `#792 <https://github.com/fmtlib/fmt/pull/792>`_,
  `#800 <https://github.com/fmtlib/fmt/pull/800>`_).
  Thanks `@Remotion (Remo) <https://github.com/Remotion>`_,
  `@gabime (Gabi Melman) <https://github.com/gabime>`_,
  `@foonathan (Jonathan Müller) <https://github.com/foonathan>`_,
  `@Dark-Passenger (Dhruv Paranjape) <https://github.com/Dark-Passenger>`_, and
  `@0x8000-0000 (Sign Bit) <https://github.com/0x8000-0000>`_.

5.0.0 - 2018-05-21
------------------

* Added a requirement for partial C++11 support, most importantly variadic
  templates and type traits, and dropped ``FMT_VARIADIC_*`` emulation macros.
  Variadic templates are available since GCC 4.4, Clang 2.9 and MSVC 18.0 (2013).
  For older compilers use {fmt} `version 4.x
  <https://github.com/fmtlib/fmt/releases/tag/4.1.0>`_ which continues to be
  maintained and works with C++98 compilers.

* Renamed symbols to follow standard C++ naming conventions and proposed a subset
  of the library for standardization in `P0645R2 Text Formatting
  <https://wg21.link/P0645>`_.

* Implemented ``constexpr`` parsing of format strings and `compile-time format
  string checks
  <https://fmt.dev/latest/api.html#compile-time-format-string-checks>`_. For
  example

  .. code:: c++

     #include <fmt/format.h>

     std::string s = format(fmt("{:d}"), "foo");

  gives a compile-time error because ``d`` is an invalid specifier for strings
  (`godbolt <https://godbolt.org/g/rnCy9Q>`__)::

     ...
     <source>:4:19: note: in instantiation of function template specialization 'fmt::v5::format<S, char [4]>' requested here
       std::string s = format(fmt("{:d}"), "foo");
                       ^
     format.h:1337:13: note: non-constexpr function 'on_error' cannot be used in a constant expression
         handler.on_error("invalid type specifier");

  Compile-time checks require relaxed ``constexpr`` (C++14 feature) support. If
  the latter is not available, checks will be performed at runtime.

* Separated format string parsing and formatting in the extension API to enable
  compile-time format string processing. For example

  .. code:: c++

     struct Answer {};

     namespace fmt {
     template <>
     struct formatter<Answer> {
       constexpr auto parse(parse_context& ctx) {
         auto it = ctx.begin();
         spec = *it;
         if (spec != 'd' && spec != 's')
           throw format_error("invalid specifier");
         return ++it;
       }

       template <typename FormatContext>
       auto format(Answer, FormatContext& ctx) {
         return spec == 's' ?
           format_to(ctx.begin(), "{}", "fourty-two") :
           format_to(ctx.begin(), "{}", 42);
       }

       char spec = 0;
     };
     }

     std::string s = format(fmt("{:x}"), Answer());

  gives a compile-time error due to invalid format specifier (`godbolt
  <https://godbolt.org/g/2jQ1Dv>`__)::

     ...
     <source>:12:45: error: expression '<throw-expression>' is not a constant expression
            throw format_error("invalid specifier");

* Added `iterator support
  <https://fmt.dev/latest/api.html#output-iterator-support>`_:

  .. code:: c++

     #include <vector>
     #include <fmt/format.h>

     std::vector<char> out;
     fmt::format_to(std::back_inserter(out), "{}", 42);

* Added the `format_to_n
  <https://fmt.dev/latest/api.html#_CPPv2N3fmt11format_to_nE8OutputItNSt6size_tE11string_viewDpRK4Args>`_
  function that restricts the output to the specified number of characters
  (`#298 <https://github.com/fmtlib/fmt/issues/298>`_):

  .. code:: c++

     char out[4];
     fmt::format_to_n(out, sizeof(out), "{}", 12345);
     // out == "1234" (without terminating '\0')

* Added the `formatted_size
  <https://fmt.dev/latest/api.html#_CPPv2N3fmt14formatted_sizeE11string_viewDpRK4Args>`_
  function for computing the output size:

  .. code:: c++

     #include <fmt/format.h>

     auto size = fmt::formatted_size("{}", 12345); // size == 5

* Improved compile times by reducing dependencies on standard headers and
  providing a lightweight `core API <https://fmt.dev/latest/api.html#core-api>`_:

  .. code:: c++

     #include <fmt/core.h>

     fmt::print("The answer is {}.", 42);

  See `Compile time and code bloat
  <https://github.com/fmtlib/fmt#compile-time-and-code-bloat>`_.

* Added the `make_format_args
  <https://fmt.dev/latest/api.html#_CPPv2N3fmt16make_format_argsEDpRK4Args>`_
  function for capturing formatting arguments:

  .. code:: c++
  
     // Prints formatted error message.
     void vreport_error(const char *format, fmt::format_args args) {
       fmt::print("Error: ");
       fmt::vprint(format, args);
     }
     template <typename... Args>
     void report_error(const char *format, const Args & ... args) {
       vreport_error(format, fmt::make_format_args(args...));
     }

* Added the ``make_printf_args`` function for capturing ``printf`` arguments
  (`#687 <https://github.com/fmtlib/fmt/issues/687>`_,
  `#694 <https://github.com/fmtlib/fmt/pull/694>`_).
  Thanks `@Kronuz (Germán Méndez Bravo) <https://github.com/Kronuz>`_.

* Added prefix ``v`` to non-variadic functions taking ``format_args`` to
  distinguish them from variadic ones:

  .. code:: c++

     std::string vformat(string_view format_str, format_args args);
     
     template <typename... Args>
     std::string format(string_view format_str, const Args & ... args);

* Added experimental support for formatting ranges, containers and tuple-like
  types in ``fmt/ranges.h`` (`#735 <https://github.com/fmtlib/fmt/pull/735>`_):

  .. code:: c++

     #include <fmt/ranges.h>

     std::vector<int> v = {1, 2, 3};
     fmt::print("{}", v); // prints {1, 2, 3}

  Thanks `@Remotion (Remo) <https://github.com/Remotion>`_.

* Implemented ``wchar_t`` date and time formatting
  (`#712 <https://github.com/fmtlib/fmt/pull/712>`_):

  .. code:: c++

     #include <fmt/time.h>

     std::time_t t = std::time(nullptr);
     auto s = fmt::format(L"The date is {:%Y-%m-%d}.", *std::localtime(&t));

  Thanks `@DanielaE (Daniela Engert) <https://github.com/DanielaE>`_.

* Provided more wide string overloads
  (`#724 <https://github.com/fmtlib/fmt/pull/724>`_).
  Thanks `@DanielaE (Daniela Engert) <https://github.com/DanielaE>`_.

* Switched from a custom null-terminated string view class to ``string_view``
  in the format API and provided ``fmt::string_view`` which implements a subset
  of ``std::string_view`` API for pre-C++17 systems.

* Added support for ``std::experimental::string_view``
  (`#607 <https://github.com/fmtlib/fmt/pull/607>`_):

  .. code:: c++

     #include <fmt/core.h>
     #include <experimental/string_view>

     fmt::print("{}", std::experimental::string_view("foo"));

  Thanks `@virgiliofornazin (Virgilio Alexandre Fornazin)
  <https://github.com/virgiliofornazin>`__.

* Allowed mixing named and automatic arguments:

  .. code:: c++

     fmt::format("{} {two}", 1, fmt::arg("two", 2));

* Removed the write API in favor of the `format API
  <https://fmt.dev/latest/api.html#format-api>`_ with compile-time handling of
  format strings.

* Disallowed formatting of multibyte strings into a wide character target
  (`#606 <https://github.com/fmtlib/fmt/pull/606>`_).

* Improved documentation
  (`#515 <https://github.com/fmtlib/fmt/pull/515>`_,
  `#614 <https://github.com/fmtlib/fmt/issues/614>`_,
  `#617 <https://github.com/fmtlib/fmt/pull/617>`_,
  `#661 <https://github.com/fmtlib/fmt/pull/661>`_,
  `#680 <https://github.com/fmtlib/fmt/pull/680>`_).
  Thanks `@ibell (Ian Bell) <https://github.com/ibell>`_,
  `@mihaitodor (Mihai Todor) <https://github.com/mihaitodor>`_, and
  `@johnthagen <https://github.com/johnthagen>`_.

* Implemented more efficient handling of large number of format arguments.

* Introduced an inline namespace for symbol versioning.

* Added debug postfix ``d`` to the ``fmt`` library name
  (`#636 <https://github.com/fmtlib/fmt/issues/636>`_).

* Removed unnecessary ``fmt/`` prefix in includes
  (`#397 <https://github.com/fmtlib/fmt/pull/397>`_).
  Thanks `@chronoxor (Ivan Shynkarenka) <https://github.com/chronoxor>`_.

* Moved ``fmt/*.h`` to ``include/fmt/*.h`` to prevent irrelevant files and
  directories appearing on the include search paths when fmt is used as a
  subproject and moved source files to the ``src`` directory.

* Added qmake project file ``support/fmt.pro``
  (`#641 <https://github.com/fmtlib/fmt/pull/641>`_).
  Thanks `@cowo78 (Giuseppe Corbelli) <https://github.com/cowo78>`_.

* Added Gradle build file ``support/build.gradle``
  (`#649 <https://github.com/fmtlib/fmt/pull/649>`_).
  Thanks `@luncliff (Park DongHa) <https://github.com/luncliff>`_.

* Removed ``FMT_CPPFORMAT`` CMake option.

* Fixed a name conflict with the macro ``CHAR_WIDTH`` in glibc
  (`#616 <https://github.com/fmtlib/fmt/pull/616>`_).
  Thanks `@aroig (Abdó Roig-Maranges) <https://github.com/aroig>`_.

* Fixed handling of nested braces in ``fmt::join``
  (`#638 <https://github.com/fmtlib/fmt/issues/638>`_).

* Added ``SOURCELINK_SUFFIX`` for compatibility with Sphinx 1.5
  (`#497 <https://github.com/fmtlib/fmt/pull/497>`_).
  Thanks `@ginggs (Graham Inggs) <https://github.com/ginggs>`_.

* Added a missing ``inline`` in the header-only mode
  (`#626 <https://github.com/fmtlib/fmt/pull/626>`_).
  Thanks `@aroig (Abdó Roig-Maranges) <https://github.com/aroig>`_.

* Fixed various compiler warnings
  (`#640 <https://github.com/fmtlib/fmt/pull/640>`_,
  `#656 <https://github.com/fmtlib/fmt/pull/656>`_,
  `#679 <https://github.com/fmtlib/fmt/pull/679>`_,
  `#681 <https://github.com/fmtlib/fmt/pull/681>`_,
  `#705 <https://github.com/fmtlib/fmt/pull/705>`__,
  `#715 <https://github.com/fmtlib/fmt/issues/715>`_,
  `#717 <https://github.com/fmtlib/fmt/pull/717>`_,
  `#720 <https://github.com/fmtlib/fmt/pull/720>`_,
  `#723 <https://github.com/fmtlib/fmt/pull/723>`_,
  `#726 <https://github.com/fmtlib/fmt/pull/726>`_,
  `#730 <https://github.com/fmtlib/fmt/pull/730>`_,
  `#739 <https://github.com/fmtlib/fmt/pull/739>`_).
  Thanks `@peterbell10 <https://github.com/peterbell10>`_,
  `@LarsGullik <https://github.com/LarsGullik>`_,
  `@foonathan (Jonathan Müller) <https://github.com/foonathan>`_,
  `@eliaskosunen (Elias Kosunen) <https://github.com/eliaskosunen>`_,
  `@christianparpart (Christian Parpart) <https://github.com/christianparpart>`_,
  `@DanielaE (Daniela Engert) <https://github.com/DanielaE>`_,
  and `@mwinterb <https://github.com/mwinterb>`_.

* Worked around an MSVC bug and fixed several warnings
  (`#653 <https://github.com/fmtlib/fmt/pull/653>`_).
  Thanks `@alabuzhev (Alex Alabuzhev) <https://github.com/alabuzhev>`_.

* Worked around GCC bug 67371
  (`#682 <https://github.com/fmtlib/fmt/issues/682>`_).

* Fixed compilation with ``-fno-exceptions``
  (`#655 <https://github.com/fmtlib/fmt/pull/655>`_).
  Thanks `@chenxiaolong (Andrew Gunnerson) <https://github.com/chenxiaolong>`_.

* Made ``constexpr remove_prefix`` gcc version check tighter
  (`#648 <https://github.com/fmtlib/fmt/issues/648>`_).

* Renamed internal type enum constants to prevent collision with poorly written
  C libraries (`#644 <https://github.com/fmtlib/fmt/issues/644>`_).

* Added detection of ``wostream operator<<``
  (`#650 <https://github.com/fmtlib/fmt/issues/650>`_).

* Fixed compilation on OpenBSD
  (`#660 <https://github.com/fmtlib/fmt/pull/660>`_).
  Thanks `@hubslave <https://github.com/hubslave>`_.

* Fixed compilation on FreeBSD 12
  (`#732 <https://github.com/fmtlib/fmt/pull/732>`_).
  Thanks `@dankm <https://github.com/dankm>`_.

* Fixed compilation when there is a mismatch between ``-std`` options between
  the library and user code
  (`#664 <https://github.com/fmtlib/fmt/issues/664>`_).

* Fixed compilation with GCC 7 and ``-std=c++11``
  (`#734 <https://github.com/fmtlib/fmt/issues/734>`_).

* Improved generated binary code on GCC 7 and older
  (`#668 <https://github.com/fmtlib/fmt/issues/668>`_).

* Fixed handling of numeric alignment with no width 
  (`#675 <https://github.com/fmtlib/fmt/issues/675>`_).

* Fixed handling of empty strings in UTF8/16 converters
  (`#676 <https://github.com/fmtlib/fmt/pull/676>`_).
  Thanks `@vgalka-sl (Vasili Galka) <https://github.com/vgalka-sl>`_.

* Fixed formatting of an empty ``string_view``
  (`#689 <https://github.com/fmtlib/fmt/issues/689>`_).

* Fixed detection of ``string_view`` on libc++ 
  (`#686 <https://github.com/fmtlib/fmt/issues/686>`_).

* Fixed DLL issues (`#696 <https://github.com/fmtlib/fmt/pull/696>`_).
  Thanks `@sebkoenig <https://github.com/sebkoenig>`_.

* Fixed compile checks for mixing narrow and wide strings
  (`#690 <https://github.com/fmtlib/fmt/issues/690>`_).

* Disabled unsafe implicit conversion to ``std::string``
  (`#729 <https://github.com/fmtlib/fmt/issues/729>`_).

* Fixed handling of reused format specs (as in ``fmt::join``) for pointers
  (`#725 <https://github.com/fmtlib/fmt/pull/725>`_).
  Thanks `@mwinterb <https://github.com/mwinterb>`_.

* Fixed installation of ``fmt/ranges.h``
  (`#738 <https://github.com/fmtlib/fmt/pull/738>`_).
  Thanks `@sv1990 <https://github.com/sv1990>`_.

4.1.0 - 2017-12-20
------------------

* Added ``fmt::to_wstring()`` in addition to ``fmt::to_string()``
  (`#559 <https://github.com/fmtlib/fmt/pull/559>`_).
  Thanks `@alabuzhev (Alex Alabuzhev) <https://github.com/alabuzhev>`_.

* Added support for C++17 ``std::string_view``
  (`#571 <https://github.com/fmtlib/fmt/pull/571>`_ and
  `#578 <https://github.com/fmtlib/fmt/pull/578>`_).
  Thanks `@thelostt (Mário Feroldi) <https://github.com/thelostt>`_ and
  `@mwinterb <https://github.com/mwinterb>`_.

* Enabled stream exceptions to catch errors
  (`#581 <https://github.com/fmtlib/fmt/issues/581>`_).
  Thanks `@crusader-mike <https://github.com/crusader-mike>`_.

* Allowed formatting of class hierarchies with ``fmt::format_arg()``
  (`#547 <https://github.com/fmtlib/fmt/pull/547>`_).
  Thanks `@rollbear (Björn Fahller) <https://github.com/rollbear>`_.

* Removed limitations on character types
  (`#563 <https://github.com/fmtlib/fmt/pull/563>`_).
  Thanks `@Yelnats321 (Elnar Dakeshov) <https://github.com/Yelnats321>`_.

* Conditionally enabled use of ``std::allocator_traits``
  (`#583 <https://github.com/fmtlib/fmt/pull/583>`_).
  Thanks `@mwinterb <https://github.com/mwinterb>`_.

* Added support for ``const`` variadic member function emulation with
  ``FMT_VARIADIC_CONST`` (`#591 <https://github.com/fmtlib/fmt/pull/591>`_).
  Thanks `@ludekvodicka (Ludek Vodicka) <https://github.com/ludekvodicka>`_.

* Various bugfixes: bad overflow check, unsupported implicit type conversion
  when determining formatting function, test segfaults
  (`#551 <https://github.com/fmtlib/fmt/issues/551>`_), ill-formed macros
  (`#542 <https://github.com/fmtlib/fmt/pull/542>`_) and ambiguous overloads
  (`#580 <https://github.com/fmtlib/fmt/issues/580>`_).
  Thanks `@xylosper (Byoung-young Lee) <https://github.com/xylosper>`_.

* Prevented warnings on MSVC (`#605 <https://github.com/fmtlib/fmt/pull/605>`_,
  `#602 <https://github.com/fmtlib/fmt/pull/602>`_, and
  `#545 <https://github.com/fmtlib/fmt/pull/545>`_),
  clang (`#582 <https://github.com/fmtlib/fmt/pull/582>`_),
  GCC (`#573 <https://github.com/fmtlib/fmt/issues/573>`_),
  various conversion warnings (`#609 <https://github.com/fmtlib/fmt/pull/609>`_,
  `#567 <https://github.com/fmtlib/fmt/pull/567>`_,
  `#553 <https://github.com/fmtlib/fmt/pull/553>`_ and
  `#553 <https://github.com/fmtlib/fmt/pull/553>`_), and added ``override`` and
  ``[[noreturn]]`` (`#549 <https://github.com/fmtlib/fmt/pull/549>`_ and
  `#555 <https://github.com/fmtlib/fmt/issues/555>`_).
  Thanks `@alabuzhev (Alex Alabuzhev) <https://github.com/alabuzhev>`_,
  `@virgiliofornazin (Virgilio Alexandre Fornazin)
  <https://gihtub.com/virgiliofornazin>`_,
  `@alexanderbock (Alexander Bock) <https://github.com/alexanderbock>`_,
  `@yumetodo <https://github.com/yumetodo>`_,
  `@VaderY (Császár Mátyás) <https://github.com/VaderY>`_,
  `@jpcima (JP Cimalando) <https://github.com/jpcima>`_,
  `@thelostt (Mário Feroldi) <https://github.com/thelostt>`_, and
  `@Manu343726 (Manu Sánchez) <https://github.com/Manu343726>`_.

* Improved CMake: Used ``GNUInstallDirs`` to set installation location
  (`#610 <https://github.com/fmtlib/fmt/pull/610>`_) and fixed warnings
  (`#536 <https://github.com/fmtlib/fmt/pull/536>`_ and
  `#556 <https://github.com/fmtlib/fmt/pull/556>`_).
  Thanks `@mikecrowe (Mike Crowe) <https://github.com/mikecrowe>`_,
  `@evgen231 <https://github.com/evgen231>`_ and
  `@henryiii (Henry Schreiner) <https://github.com/henryiii>`_.

4.0.0 - 2017-06-27
------------------

* Removed old compatibility headers ``cppformat/*.h`` and CMake options
  (`#527 <https://github.com/fmtlib/fmt/pull/527>`_).
  Thanks `@maddinat0r (Alex Martin) <https://github.com/maddinat0r>`_.

* Added ``string.h`` containing ``fmt::to_string()`` as alternative to
  ``std::to_string()`` as well as other string writer functionality
  (`#326 <https://github.com/fmtlib/fmt/issues/326>`_ and
  `#441 <https://github.com/fmtlib/fmt/pull/441>`_):

  .. code:: c++

    #include "fmt/string.h"
  
    std::string answer = fmt::to_string(42);

  Thanks to `@glebov-andrey (Andrey Glebov)
  <https://github.com/glebov-andrey>`_.

* Moved ``fmt::printf()`` to new ``printf.h`` header and allowed ``%s`` as
  generic specifier (`#453 <https://github.com/fmtlib/fmt/pull/453>`_),
  made ``%.f`` more conformant to regular ``printf()``
  (`#490 <https://github.com/fmtlib/fmt/pull/490>`_), added custom writer
  support (`#476 <https://github.com/fmtlib/fmt/issues/476>`_) and implemented
  missing custom argument formatting
  (`#339 <https://github.com/fmtlib/fmt/pull/339>`_ and
  `#340 <https://github.com/fmtlib/fmt/pull/340>`_):

  .. code:: c++

    #include "fmt/printf.h"
 
    // %s format specifier can be used with any argument type.
    fmt::printf("%s", 42);

  Thanks `@mojoBrendan <https://github.com/mojoBrendan>`_,
  `@manylegged (Arthur Danskin) <https://github.com/manylegged>`_ and
  `@spacemoose (Glen Stark) <https://github.com/spacemoose>`_.
  See also `#360 <https://github.com/fmtlib/fmt/issues/360>`_,
  `#335 <https://github.com/fmtlib/fmt/issues/335>`_ and
  `#331 <https://github.com/fmtlib/fmt/issues/331>`_.

* Added ``container.h`` containing a ``BasicContainerWriter``
  to write to containers like ``std::vector``
  (`#450 <https://github.com/fmtlib/fmt/pull/450>`_).
  Thanks `@polyvertex (Jean-Charles Lefebvre) <https://github.com/polyvertex>`_.

* Added ``fmt::join()`` function that takes a range and formats
  its elements separated by a given string
  (`#466 <https://github.com/fmtlib/fmt/pull/466>`_):

  .. code:: c++

    #include "fmt/format.h"
 
    std::vector<double> v = {1.2, 3.4, 5.6};
    // Prints "(+01.20, +03.40, +05.60)".
    fmt::print("({:+06.2f})", fmt::join(v.begin(), v.end(), ", "));

  Thanks `@olivier80 <https://github.com/olivier80>`_.

* Added support for custom formatting specifications to simplify customization
  of built-in formatting (`#444 <https://github.com/fmtlib/fmt/pull/444>`_).
  Thanks `@polyvertex (Jean-Charles Lefebvre) <https://github.com/polyvertex>`_.
  See also `#439 <https://github.com/fmtlib/fmt/issues/439>`_.

* Added ``fmt::format_system_error()`` for error code formatting
  (`#323 <https://github.com/fmtlib/fmt/issues/323>`_ and
  `#526 <https://github.com/fmtlib/fmt/pull/526>`_).
  Thanks `@maddinat0r (Alex Martin) <https://github.com/maddinat0r>`_.

* Added thread-safe ``fmt::localtime()`` and ``fmt::gmtime()``
  as replacement   for the standard version to ``time.h``
  (`#396 <https://github.com/fmtlib/fmt/pull/396>`_).
  Thanks `@codicodi <https://github.com/codicodi>`_.

* Internal improvements to ``NamedArg`` and ``ArgLists``
  (`#389 <https://github.com/fmtlib/fmt/pull/389>`_ and
  `#390 <https://github.com/fmtlib/fmt/pull/390>`_).
  Thanks `@chronoxor <https://github.com/chronoxor>`_.

* Fixed crash due to bug in ``FormatBuf``
  (`#493 <https://github.com/fmtlib/fmt/pull/493>`_).
  Thanks `@effzeh <https://github.com/effzeh>`_. See also
  `#480 <https://github.com/fmtlib/fmt/issues/480>`_ and
  `#491 <https://github.com/fmtlib/fmt/issues/491>`_.

* Fixed handling of wide strings in ``fmt::StringWriter``.

* Improved compiler error messages
  (`#357 <https://github.com/fmtlib/fmt/issues/357>`_).

* Fixed various warnings and issues with various compilers
  (`#494 <https://github.com/fmtlib/fmt/pull/494>`_,
  `#499 <https://github.com/fmtlib/fmt/pull/499>`_,
  `#483 <https://github.com/fmtlib/fmt/pull/483>`_,
  `#485 <https://github.com/fmtlib/fmt/pull/485>`_,
  `#482 <https://github.com/fmtlib/fmt/pull/482>`_,
  `#475 <https://github.com/fmtlib/fmt/pull/475>`_,
  `#473 <https://github.com/fmtlib/fmt/pull/473>`_ and
  `#414 <https://github.com/fmtlib/fmt/pull/414>`_).
  Thanks `@chronoxor <https://github.com/chronoxor>`_,
  `@zhaohuaxishi <https://github.com/zhaohuaxishi>`_,
  `@pkestene (Pierre Kestener) <https://github.com/pkestene>`_,
  `@dschmidt (Dominik Schmidt) <https://github.com/dschmidt>`_ and
  `@0x414c (Alexey Gorishny) <https://github.com/0x414c>`_ .

* Improved CMake: targets are now namespaced
  (`#511 <https://github.com/fmtlib/fmt/pull/511>`_ and
  `#513 <https://github.com/fmtlib/fmt/pull/513>`_), supported header-only
  ``printf.h`` (`#354 <https://github.com/fmtlib/fmt/pull/354>`_), fixed issue
  with minimal supported library subset
  (`#418 <https://github.com/fmtlib/fmt/issues/418>`_,
  `#419 <https://github.com/fmtlib/fmt/pull/419>`_ and
  `#420 <https://github.com/fmtlib/fmt/pull/420>`_).
  Thanks `@bjoernthiel (Bjoern Thiel) <https://github.com/bjoernthiel>`_,
  `@niosHD (Mario Werner) <https://github.com/niosHD>`_,
  `@LogicalKnight (Sean LK) <https://github.com/LogicalKnight>`_ and
  `@alabuzhev (Alex Alabuzhev) <https://github.com/alabuzhev>`_.

* Improved documentation. Thanks to
  `@pwm1234 (Phil) <https://github.com/pwm1234>`_ for
  `#393 <https://github.com/fmtlib/fmt/pull/393>`_.

3.0.2 - 2017-06-14
------------------

* Added ``FMT_VERSION`` macro
  (`#411 <https://github.com/fmtlib/fmt/issues/411>`_).

* Used ``FMT_NULL`` instead of literal ``0``
  (`#409 <https://github.com/fmtlib/fmt/pull/409>`_).
  Thanks `@alabuzhev (Alex Alabuzhev) <https://github.com/alabuzhev>`_.

* Added extern templates for ``format_float``
  (`#413 <https://github.com/fmtlib/fmt/issues/413>`_).

* Fixed implicit conversion issue
  (`#507 <https://github.com/fmtlib/fmt/issues/507>`_).

* Fixed signbit detection (`#423 <https://github.com/fmtlib/fmt/issues/423>`_).

* Fixed naming collision (`#425 <https://github.com/fmtlib/fmt/issues/425>`_).

* Fixed missing intrinsic for C++/CLI
  (`#457 <https://github.com/fmtlib/fmt/pull/457>`_).
  Thanks `@calumr (Calum Robinson) <https://github.com/calumr>`_

* Fixed Android detection (`#458 <https://github.com/fmtlib/fmt/pull/458>`_).
  Thanks `@Gachapen (Magnus Bjerke Vik) <https://github.com/Gachapen>`_.

* Use lean ``windows.h`` if not in header-only mode
  (`#503 <https://github.com/fmtlib/fmt/pull/503>`_).
  Thanks `@Quentin01 (Quentin Buathier) <https://github.com/Quentin01>`_.

* Fixed issue with CMake exporting C++11 flag
  (`#445 <https://github.com/fmtlib/fmt/pull/455>`_).
  Thanks `@EricWF (Eric) <https://github.com/EricWF>`_.

* Fixed issue with nvcc and MSVC compiler bug and MinGW
  (`#505 <https://github.com/fmtlib/fmt/issues/505>`_).

* Fixed DLL issues (`#469 <https://github.com/fmtlib/fmt/pull/469>`_ and
  `#502 <https://github.com/fmtlib/fmt/pull/502>`_).
  Thanks `@richardeakin (Richard Eakin) <https://github.com/richardeakin>`_ and
  `@AndreasSchoenle (Andreas Schönle) <https://github.com/AndreasSchoenle>`_.

* Fixed test compilation under FreeBSD
  (`#433 <https://github.com/fmtlib/fmt/issues/433>`_).

* Fixed various warnings (`#403 <https://github.com/fmtlib/fmt/pull/403>`_,
  `#410 <https://github.com/fmtlib/fmt/pull/410>`_ and
  `#510 <https://github.com/fmtlib/fmt/pull/510>`_).
  Thanks `@Lecetem <https://github.com/Lectem>`_,
  `@chenhayat (Chen Hayat) <https://github.com/chenhayat>`_ and
  `@trozen <https://github.com/trozen>`_.

* Worked around a broken ``__builtin_clz`` in clang with MS codegen
  (`#519 <https://github.com/fmtlib/fmt/issues/519>`_).

* Removed redundant include
  (`#479 <https://github.com/fmtlib/fmt/issues/479>`_).

* Fixed documentation issues.

3.0.1 - 2016-11-01
------------------
* Fixed handling of thousands separator
  (`#353 <https://github.com/fmtlib/fmt/issues/353>`_).

* Fixed handling of ``unsigned char`` strings
  (`#373 <https://github.com/fmtlib/fmt/issues/373>`_).

* Corrected buffer growth when formatting time
  (`#367 <https://github.com/fmtlib/fmt/issues/367>`_).

* Removed warnings under MSVC and clang
  (`#318 <https://github.com/fmtlib/fmt/issues/318>`_,
  `#250 <https://github.com/fmtlib/fmt/issues/250>`_, also merged
  `#385 <https://github.com/fmtlib/fmt/pull/385>`_ and
  `#361 <https://github.com/fmtlib/fmt/pull/361>`_).
  Thanks `@jcelerier (Jean-Michaël Celerier) <https://github.com/jcelerier>`_
  and `@nmoehrle (Nils Moehrle) <https://github.com/nmoehrle>`_.

* Fixed compilation issues under Android
  (`#327 <https://github.com/fmtlib/fmt/pull/327>`_,
  `#345 <https://github.com/fmtlib/fmt/issues/345>`_ and
  `#381 <https://github.com/fmtlib/fmt/pull/381>`_),
  FreeBSD (`#358 <https://github.com/fmtlib/fmt/pull/358>`_),
  Cygwin (`#388 <https://github.com/fmtlib/fmt/issues/388>`_),
  MinGW (`#355 <https://github.com/fmtlib/fmt/issues/355>`_) as well as other
  issues (`#350 <https://github.com/fmtlib/fmt/issues/350>`_,
  `#366 <https://github.com/fmtlib/fmt/issues/355>`_,
  `#348 <https://github.com/fmtlib/fmt/pull/348>`_,
  `#402 <https://github.com/fmtlib/fmt/pull/402>`_,
  `#405 <https://github.com/fmtlib/fmt/pull/405>`_).
  Thanks to `@dpantele (Dmitry) <https://github.com/dpantele>`_,
  `@hghwng (Hugh Wang) <https://github.com/hghwng>`_,
  `@arvedarved (Tilman Keskinöz) <https://github.com/arvedarved>`_,
  `@LogicalKnight (Sean) <https://github.com/LogicalKnight>`_ and
  `@JanHellwig (Jan Hellwig) <https://github.com/janhellwig>`_.

* Fixed some documentation issues and extended specification
  (`#320 <https://github.com/fmtlib/fmt/issues/320>`_,
  `#333 <https://github.com/fmtlib/fmt/pull/333>`_,
  `#347 <https://github.com/fmtlib/fmt/issues/347>`_,
  `#362 <https://github.com/fmtlib/fmt/pull/362>`_).
  Thanks to `@smellman (Taro Matsuzawa aka. btm)
  <https://github.com/smellman>`_.

3.0.0 - 2016-05-07
------------------

* The project has been renamed from C++ Format (cppformat) to fmt for
  consistency with the used namespace and macro prefix
  (`#307 <https://github.com/fmtlib/fmt/issues/307>`_).
  Library headers are now located in the ``fmt`` directory:

  .. code:: c++

    #include "fmt/format.h"

  Including ``format.h`` from the ``cppformat`` directory is deprecated
  but works via a proxy header which will be removed in the next major version.
  
  The documentation is now available at https://fmt.dev.

* Added support for `strftime <http://en.cppreference.com/w/cpp/chrono/c/strftime>`_-like
  `date and time formatting <https://fmt.dev/3.0.0/api.html#date-and-time-formatting>`_
  (`#283 <https://github.com/fmtlib/fmt/issues/283>`_):

  .. code:: c++

    #include "fmt/time.h"

    std::time_t t = std::time(nullptr);
    // Prints "The date is 2016-04-29." (with the current date)
    fmt::print("The date is {:%Y-%m-%d}.", *std::localtime(&t));

* ``std::ostream`` support including formatting of user-defined types that provide
  overloaded ``operator<<`` has been moved to ``fmt/ostream.h``:

  .. code:: c++

    #include "fmt/ostream.h"

    class Date {
      int year_, month_, day_;
    public:
      Date(int year, int month, int day) : year_(year), month_(month), day_(day) {}

      friend std::ostream &operator<<(std::ostream &os, const Date &d) {
        return os << d.year_ << '-' << d.month_ << '-' << d.day_;
      }
    };

    std::string s = fmt::format("The date is {}", Date(2012, 12, 9));
    // s == "The date is 2012-12-9"

* Added support for `custom argument formatters
  <https://fmt.dev/3.0.0/api.html#argument-formatters>`_
  (`#235 <https://github.com/fmtlib/fmt/issues/235>`_).

* Added support for locale-specific integer formatting with the ``n`` specifier
  (`#305 <https://github.com/fmtlib/fmt/issues/305>`_):

  .. code:: c++

    std::setlocale(LC_ALL, "en_US.utf8");
    fmt::print("cppformat: {:n}\n", 1234567); // prints 1,234,567

* Sign is now preserved when formatting an integer with an incorrect ``printf``
  format specifier (`#265 <https://github.com/fmtlib/fmt/issues/265>`_):

  .. code:: c++

    fmt::printf("%lld", -42); // prints -42

  Note that it would be an undefined behavior in ``std::printf``.

* Length modifiers such as ``ll`` are now optional in printf formatting
  functions and the correct type is determined automatically
  (`#255 <https://github.com/fmtlib/fmt/issues/255>`_):

  .. code:: c++

    fmt::printf("%d", std::numeric_limits<long long>::max());

  Note that it would be an undefined behavior in ``std::printf``.

* Added initial support for custom formatters
  (`#231 <https://github.com/fmtlib/fmt/issues/231>`_).

* Fixed detection of user-defined literal support on Intel C++ compiler
  (`#311 <https://github.com/fmtlib/fmt/issues/311>`_,
  `#312 <https://github.com/fmtlib/fmt/pull/312>`_).
  Thanks to `@dean0x7d (Dean Moldovan) <https://github.com/dean0x7d>`_ and
  `@speth (Ray Speth) <https://github.com/speth>`_.

* Reduced compile time
  (`#243 <https://github.com/fmtlib/fmt/pull/243>`_,
  `#249 <https://github.com/fmtlib/fmt/pull/249>`_,
  `#317 <https://github.com/fmtlib/fmt/issues/317>`_):

  .. image:: https://cloud.githubusercontent.com/assets/4831417/11614060/
             b9e826d2-9c36-11e5-8666-d4131bf503ef.png

  .. image:: https://cloud.githubusercontent.com/assets/4831417/11614080/
             6ac903cc-9c37-11e5-8165-26df6efae364.png

  Thanks to `@dean0x7d (Dean Moldovan) <https://github.com/dean0x7d>`_.

* Compile test fixes (`#313 <https://github.com/fmtlib/fmt/pull/313>`_).
  Thanks to `@dean0x7d (Dean Moldovan) <https://github.com/dean0x7d>`_.

* Documentation fixes (`#239 <https://github.com/fmtlib/fmt/pull/239>`_,
  `#248 <https://github.com/fmtlib/fmt/issues/248>`_,
  `#252 <https://github.com/fmtlib/fmt/issues/252>`_,
  `#258 <https://github.com/fmtlib/fmt/pull/258>`_,
  `#260 <https://github.com/fmtlib/fmt/issues/260>`_,
  `#301 <https://github.com/fmtlib/fmt/issues/301>`_,
  `#309 <https://github.com/fmtlib/fmt/pull/309>`_).
  Thanks to `@ReadmeCritic <https://github.com/ReadmeCritic>`_
  `@Gachapen (Magnus Bjerke Vik) <https://github.com/Gachapen>`_ and
  `@jwilk (Jakub Wilk) <https://github.com/jwilk>`_.

* Fixed compiler and sanitizer warnings
  (`#244 <https://github.com/fmtlib/fmt/issues/244>`_,
  `#256 <https://github.com/fmtlib/fmt/pull/256>`_,
  `#259 <https://github.com/fmtlib/fmt/pull/259>`_,
  `#263 <https://github.com/fmtlib/fmt/issues/263>`_,
  `#274 <https://github.com/fmtlib/fmt/issues/274>`_,
  `#277 <https://github.com/fmtlib/fmt/pull/277>`_,
  `#286 <https://github.com/fmtlib/fmt/pull/286>`_,
  `#291 <https://github.com/fmtlib/fmt/issues/291>`_,
  `#296 <https://github.com/fmtlib/fmt/issues/296>`_,
  `#308 <https://github.com/fmtlib/fmt/issues/308>`_)
  Thanks to `@mwinterb <https://github.com/mwinterb>`_,
  `@pweiskircher (Patrik Weiskircher) <https://github.com/pweiskircher>`_,
  `@Naios <https://github.com/Naios>`_.

* Improved compatibility with Windows Store apps
  (`#280 <https://github.com/fmtlib/fmt/issues/280>`_,
  `#285 <https://github.com/fmtlib/fmt/pull/285>`_)
  Thanks to `@mwinterb <https://github.com/mwinterb>`_.

* Added tests of compatibility with older C++ standards
  (`#273 <https://github.com/fmtlib/fmt/pull/273>`_).
  Thanks to `@niosHD <https://github.com/niosHD>`_.

* Fixed Android build (`#271 <https://github.com/fmtlib/fmt/pull/271>`_).
  Thanks to `@newnon <https://github.com/newnon>`_.

* Changed ``ArgMap`` to be backed by a vector instead of a map.
  (`#261 <https://github.com/fmtlib/fmt/issues/261>`_,
  `#262 <https://github.com/fmtlib/fmt/pull/262>`_).
  Thanks to `@mwinterb <https://github.com/mwinterb>`_.

* Added ``fprintf`` overload that writes to a ``std::ostream``
  (`#251 <https://github.com/fmtlib/fmt/pull/251>`_).
  Thanks to `nickhutchinson (Nicholas Hutchinson) <https://github.com/nickhutchinson>`_.

* Export symbols when building a Windows DLL
  (`#245 <https://github.com/fmtlib/fmt/pull/245>`_).
  Thanks to `macdems (Maciek Dems) <https://github.com/macdems>`_.

* Fixed compilation on Cygwin (`#304 <https://github.com/fmtlib/fmt/issues/304>`_).

* Implemented a workaround for a bug in Apple LLVM version 4.2 of clang
  (`#276 <https://github.com/fmtlib/fmt/issues/276>`_).

* Implemented a workaround for Google Test bug
  `#705 <https://github.com/google/googletest/issues/705>`_ on gcc 6
  (`#268 <https://github.com/fmtlib/fmt/issues/268>`_).
  Thanks to `octoploid <https://github.com/octoploid>`_.

* Removed Biicode support because the latter has been discontinued.

2.1.1 - 2016-04-11
------------------

* The install location for generated CMake files is now configurable via
  the ``FMT_CMAKE_DIR`` CMake variable
  (`#299 <https://github.com/fmtlib/fmt/pull/299>`_).
  Thanks to `@niosHD <https://github.com/niosHD>`_.

* Documentation fixes (`#252 <https://github.com/fmtlib/fmt/issues/252>`_).

2.1.0 - 2016-03-21
------------------

* Project layout and build system improvements
  (`#267 <https://github.com/fmtlib/fmt/pull/267>`_):

  * The code have been moved to the ``cppformat`` directory.
    Including ``format.h`` from the top-level directory is deprecated
    but works via a proxy header which will be removed in the next
    major version.

  * C++ Format CMake targets now have proper interface definitions.

  * Installed version of the library now supports the header-only
    configuration.

  * Targets ``doc``, ``install``, and ``test`` are now disabled if C++ Format
    is included as a CMake subproject. They can be enabled by setting
    ``FMT_DOC``, ``FMT_INSTALL``, and ``FMT_TEST`` in the parent project.

  Thanks to `@niosHD <https://github.com/niosHD>`_.

2.0.1 - 2016-03-13
------------------

* Improved CMake find and package support
  (`#264 <https://github.com/fmtlib/fmt/issues/264>`_).
  Thanks to `@niosHD <https://github.com/niosHD>`_.

* Fix compile error with Android NDK and mingw32
  (`#241 <https://github.com/fmtlib/fmt/issues/241>`_).
  Thanks to `@Gachapen (Magnus Bjerke Vik) <https://github.com/Gachapen>`_.

* Documentation fixes
  (`#248 <https://github.com/fmtlib/fmt/issues/248>`_,
  `#260 <https://github.com/fmtlib/fmt/issues/260>`_).

2.0.0 - 2015-12-01
------------------

General
~~~~~~~

* [Breaking] Named arguments
  (`#169 <https://github.com/fmtlib/fmt/pull/169>`_,
  `#173 <https://github.com/fmtlib/fmt/pull/173>`_,
  `#174 <https://github.com/fmtlib/fmt/pull/174>`_):

  .. code:: c++

    fmt::print("The answer is {answer}.", fmt::arg("answer", 42));

  Thanks to `@jamboree <https://github.com/jamboree>`_.

* [Experimental] User-defined literals for format and named arguments
  (`#204 <https://github.com/fmtlib/fmt/pull/204>`_,
  `#206 <https://github.com/fmtlib/fmt/pull/206>`_,
  `#207 <https://github.com/fmtlib/fmt/pull/207>`_):

  .. code:: c++

    using namespace fmt::literals;
    fmt::print("The answer is {answer}.", "answer"_a=42);

  Thanks to `@dean0x7d (Dean Moldovan) <https://github.com/dean0x7d>`_.

* [Breaking] Formatting of more than 16 arguments is now supported when using
  variadic templates
  (`#141 <https://github.com/fmtlib/fmt/issues/141>`_).
  Thanks to `@Shauren <https://github.com/Shauren>`_.

* Runtime width specification
  (`#168 <https://github.com/fmtlib/fmt/pull/168>`_):

  .. code:: c++

    fmt::format("{0:{1}}", 42, 5); // gives "   42"

  Thanks to `@jamboree <https://github.com/jamboree>`_.

* [Breaking] Enums are now formatted with an overloaded ``std::ostream`` insertion
  operator (``operator<<``) if available
  (`#232 <https://github.com/fmtlib/fmt/issues/232>`_).

* [Breaking] Changed default ``bool`` format to textual, "true" or "false"
  (`#170 <https://github.com/fmtlib/fmt/issues/170>`_):

  .. code:: c++
  
    fmt::print("{}", true); // prints "true"

  To print ``bool`` as a number use numeric format specifier such as ``d``:

  .. code:: c++

    fmt::print("{:d}", true); // prints "1"

* ``fmt::printf`` and ``fmt::sprintf`` now support formatting of ``bool`` with the
  ``%s`` specifier giving textual output, "true" or "false"
  (`#223 <https://github.com/fmtlib/fmt/pull/223>`_):

  .. code:: c++

    fmt::printf("%s", true); // prints "true"

  Thanks to `@LarsGullik <https://github.com/LarsGullik>`_.

* [Breaking] ``signed char`` and ``unsigned char`` are now formatted as integers by default
  (`#217 <https://github.com/fmtlib/fmt/pull/217>`_).

* [Breaking] Pointers to C strings can now be formatted with the ``p`` specifier
  (`#223 <https://github.com/fmtlib/fmt/pull/223>`_):

  .. code:: c++

    fmt::print("{:p}", "test"); // prints pointer value

  Thanks to `@LarsGullik <https://github.com/LarsGullik>`_.

* [Breaking] ``fmt::printf`` and ``fmt::sprintf`` now print null pointers as ``(nil)``
  and null strings as ``(null)`` for consistency with glibc
  (`#226 <https://github.com/fmtlib/fmt/pull/226>`_).
  Thanks to `@LarsGullik <https://github.com/LarsGullik>`_.

* [Breaking] ``fmt::(s)printf`` now supports formatting of objects of user-defined types
  that provide an overloaded ``std::ostream`` insertion operator (``operator<<``)
  (`#201 <https://github.com/fmtlib/fmt/issues/201>`_):

  .. code:: c++

    fmt::printf("The date is %s", Date(2012, 12, 9));

* [Breaking] The ``Buffer`` template is now part of the public API and can be used
  to implement custom memory buffers
  (`#140 <https://github.com/fmtlib/fmt/issues/140>`_).
  Thanks to `@polyvertex (Jean-Charles Lefebvre) <https://github.com/polyvertex>`_.

* [Breaking] Improved compatibility between ``BasicStringRef`` and
  `std::experimental::basic_string_view
  <http://en.cppreference.com/w/cpp/experimental/basic_string_view>`_
  (`#100 <https://github.com/fmtlib/fmt/issues/100>`_,
  `#159 <https://github.com/fmtlib/fmt/issues/159>`_,
  `#183 <https://github.com/fmtlib/fmt/issues/183>`_):

  - Comparison operators now compare string content, not pointers
  - ``BasicStringRef::c_str`` replaced by ``BasicStringRef::data``
  - ``BasicStringRef`` is no longer assumed to be null-terminated

  References to null-terminated strings are now represented by a new class,
  ``BasicCStringRef``.

* Dependency on pthreads introduced by Google Test is now optional
  (`#185 <https://github.com/fmtlib/fmt/issues/185>`_).

* New CMake options ``FMT_DOC``, ``FMT_INSTALL`` and ``FMT_TEST`` to control
  generation of ``doc``, ``install`` and ``test`` targets respectively, on by default
  (`#197 <https://github.com/fmtlib/fmt/issues/197>`_,
  `#198 <https://github.com/fmtlib/fmt/issues/198>`_,
  `#200 <https://github.com/fmtlib/fmt/issues/200>`_).
  Thanks to `@maddinat0r (Alex Martin) <https://github.com/maddinat0r>`_.

* ``noexcept`` is now used when compiling with MSVC2015
  (`#215 <https://github.com/fmtlib/fmt/pull/215>`_).
  Thanks to `@dmkrepo (Dmitriy) <https://github.com/dmkrepo>`_.

* Added an option to disable use of ``windows.h`` when ``FMT_USE_WINDOWS_H``
  is defined as 0 before including ``format.h``
  (`#171 <https://github.com/fmtlib/fmt/issues/171>`_).
  Thanks to `@alfps (Alf P. Steinbach) <https://github.com/alfps>`_.

* [Breaking] ``windows.h`` is now included with ``NOMINMAX`` unless
  ``FMT_WIN_MINMAX`` is defined. This is done to prevent breaking code using
  ``std::min`` and ``std::max`` and only affects the header-only configuration
  (`#152 <https://github.com/fmtlib/fmt/issues/152>`_,
  `#153 <https://github.com/fmtlib/fmt/pull/153>`_,
  `#154 <https://github.com/fmtlib/fmt/pull/154>`_).
  Thanks to `@DevO2012 <https://github.com/DevO2012>`_.

* Improved support for custom character types
  (`#171 <https://github.com/fmtlib/fmt/issues/171>`_).
  Thanks to `@alfps (Alf P. Steinbach) <https://github.com/alfps>`_.

* Added an option to disable use of IOStreams when ``FMT_USE_IOSTREAMS``
  is defined as 0 before including ``format.h``
  (`#205 <https://github.com/fmtlib/fmt/issues/205>`_,
  `#208 <https://github.com/fmtlib/fmt/pull/208>`_).
  Thanks to `@JodiTheTigger <https://github.com/JodiTheTigger>`_.

* Improved detection of ``isnan``, ``isinf`` and ``signbit``.

Optimization
~~~~~~~~~~~~

* Made formatting of user-defined types more efficient with a custom stream buffer
  (`#92 <https://github.com/fmtlib/fmt/issues/92>`_,
  `#230 <https://github.com/fmtlib/fmt/pull/230>`_).
  Thanks to `@NotImplemented <https://github.com/NotImplemented>`_.

* Further improved performance of ``fmt::Writer`` on integer formatting
  and fixed a minor regression. Now it is ~7% faster than ``karma::generate``
  on Karma's benchmark
  (`#186 <https://github.com/fmtlib/fmt/issues/186>`_).

* [Breaking] Reduced `compiled code size
  <https://github.com/fmtlib/fmt#compile-time-and-code-bloat>`_
  (`#143 <https://github.com/fmtlib/fmt/issues/143>`_,
  `#149 <https://github.com/fmtlib/fmt/pull/149>`_).

Distribution
~~~~~~~~~~~~

* [Breaking] Headers are now installed in
  ``${CMAKE_INSTALL_PREFIX}/include/cppformat``
  (`#178 <https://github.com/fmtlib/fmt/issues/178>`_).
  Thanks to `@jackyf (Eugene V. Lyubimkin) <https://github.com/jackyf>`_.

* [Breaking] Changed the library name from ``format`` to ``cppformat``
  for consistency with the project name and to avoid potential conflicts
  (`#178 <https://github.com/fmtlib/fmt/issues/178>`_).
  Thanks to `@jackyf (Eugene V. Lyubimkin) <https://github.com/jackyf>`_.

* C++ Format is now available in `Debian <https://www.debian.org/>`_ GNU/Linux
  (`stretch <https://packages.debian.org/source/stretch/cppformat>`_,
  `sid <https://packages.debian.org/source/sid/cppformat>`_) and 
  derived distributions such as
  `Ubuntu <https://launchpad.net/ubuntu/+source/cppformat>`_ 15.10 and later
  (`#155 <https://github.com/fmtlib/fmt/issues/155>`_)::

    $ sudo apt-get install libcppformat1-dev

  Thanks to `@jackyf (Eugene V. Lyubimkin) <https://github.com/jackyf>`_.

* `Packages for Fedora and RHEL <https://admin.fedoraproject.org/pkgdb/package/cppformat/>`_
  are now available. Thanks to Dave Johansen.
  
* C++ Format can now be installed via `Homebrew <http://brew.sh/>`_ on OS X
  (`#157 <https://github.com/fmtlib/fmt/issues/157>`_)::

    $ brew install cppformat

  Thanks to `@ortho <https://github.com/ortho>`_, Anatoliy Bulukin.

Documentation
~~~~~~~~~~~~~

* Migrated from ReadTheDocs to GitHub Pages for better responsiveness
  and reliability
  (`#128 <https://github.com/fmtlib/fmt/issues/128>`_).
  New documentation address is http://cppformat.github.io/.


* Added `Building the documentation
  <https://fmt.dev/2.0.0/usage.html#building-the-documentation>`_
  section to the documentation.

* Documentation build script is now compatible with Python 3 and newer pip versions.
  (`#189 <https://github.com/fmtlib/fmt/pull/189>`_,
  `#209 <https://github.com/fmtlib/fmt/issues/209>`_).
  Thanks to `@JodiTheTigger <https://github.com/JodiTheTigger>`_ and
  `@xentec <https://github.com/xentec>`_.
  
* Documentation fixes and improvements
  (`#36 <https://github.com/fmtlib/fmt/issues/36>`_,
  `#75 <https://github.com/fmtlib/fmt/issues/75>`_,
  `#125 <https://github.com/fmtlib/fmt/issues/125>`_,
  `#160 <https://github.com/fmtlib/fmt/pull/160>`_,
  `#161 <https://github.com/fmtlib/fmt/pull/161>`_,
  `#162 <https://github.com/fmtlib/fmt/issues/162>`_,
  `#165 <https://github.com/fmtlib/fmt/issues/165>`_,
  `#210 <https://github.com/fmtlib/fmt/issues/210>`_).
  Thanks to `@syohex (Syohei YOSHIDA) <https://github.com/syohex>`_ and
  bug reporters.

* Fixed out-of-tree documentation build
  (`#177 <https://github.com/fmtlib/fmt/issues/177>`_).
  Thanks to `@jackyf (Eugene V. Lyubimkin) <https://github.com/jackyf>`_.

Fixes
~~~~~

* Fixed ``initializer_list`` detection
  (`#136 <https://github.com/fmtlib/fmt/issues/136>`_).
  Thanks to `@Gachapen (Magnus Bjerke Vik) <https://github.com/Gachapen>`_.

* [Breaking] Fixed formatting of enums with numeric format specifiers in
  ``fmt::(s)printf`` 
  (`#131 <https://github.com/fmtlib/fmt/issues/131>`_,
  `#139 <https://github.com/fmtlib/fmt/issues/139>`_):

  .. code:: c++

    enum { ANSWER = 42 };
    fmt::printf("%d", ANSWER);

  Thanks to `@Naios <https://github.com/Naios>`_.

* Improved compatibility with old versions of MinGW
  (`#129 <https://github.com/fmtlib/fmt/issues/129>`_,
  `#130 <https://github.com/fmtlib/fmt/pull/130>`_,
  `#132 <https://github.com/fmtlib/fmt/issues/132>`_).
  Thanks to `@cstamford (Christopher Stamford) <https://github.com/cstamford>`_.

* Fixed a compile error on MSVC with disabled exceptions
  (`#144 <https://github.com/fmtlib/fmt/issues/144>`_).

* Added a workaround for broken implementation of variadic templates in MSVC2012
  (`#148 <https://github.com/fmtlib/fmt/issues/148>`_).

* Placed the anonymous namespace within ``fmt`` namespace for the header-only
  configuration
  (`#171 <https://github.com/fmtlib/fmt/issues/171>`_).
  Thanks to `@alfps (Alf P. Steinbach) <https://github.com/alfps>`_.

* Fixed issues reported by Coverity Scan
  (`#187 <https://github.com/fmtlib/fmt/issues/187>`_,
  `#192 <https://github.com/fmtlib/fmt/issues/192>`_).

* Implemented a workaround for a name lookup bug in MSVC2010
  (`#188 <https://github.com/fmtlib/fmt/issues/188>`_).

* Fixed compiler warnings
  (`#95 <https://github.com/fmtlib/fmt/issues/95>`_,
  `#96 <https://github.com/fmtlib/fmt/issues/96>`_,
  `#114 <https://github.com/fmtlib/fmt/pull/114>`_,
  `#135 <https://github.com/fmtlib/fmt/issues/135>`_,
  `#142 <https://github.com/fmtlib/fmt/issues/142>`_,
  `#145 <https://github.com/fmtlib/fmt/issues/145>`_,
  `#146 <https://github.com/fmtlib/fmt/issues/146>`_,
  `#158 <https://github.com/fmtlib/fmt/issues/158>`_,
  `#163 <https://github.com/fmtlib/fmt/issues/163>`_,
  `#175 <https://github.com/fmtlib/fmt/issues/175>`_,
  `#190 <https://github.com/fmtlib/fmt/issues/190>`_,
  `#191 <https://github.com/fmtlib/fmt/pull/191>`_,
  `#194 <https://github.com/fmtlib/fmt/issues/194>`_,
  `#196 <https://github.com/fmtlib/fmt/pull/196>`_,
  `#216 <https://github.com/fmtlib/fmt/issues/216>`_,
  `#218 <https://github.com/fmtlib/fmt/pull/218>`_,
  `#220 <https://github.com/fmtlib/fmt/pull/220>`_,
  `#229 <https://github.com/fmtlib/fmt/pull/229>`_,
  `#233 <https://github.com/fmtlib/fmt/issues/233>`_,
  `#234 <https://github.com/fmtlib/fmt/issues/234>`_,
  `#236 <https://github.com/fmtlib/fmt/pull/236>`_,
  `#281 <https://github.com/fmtlib/fmt/issues/281>`_,
  `#289 <https://github.com/fmtlib/fmt/issues/289>`_).
  Thanks to `@seanmiddleditch (Sean Middleditch) <https://github.com/seanmiddleditch>`_,
  `@dixlorenz (Dix Lorenz) <https://github.com/dixlorenz>`_,
  `@CarterLi (李通洲) <https://github.com/CarterLi>`_,
  `@Naios <https://github.com/Naios>`_,
  `@fmatthew5876 (Matthew Fioravante) <https://github.com/fmatthew5876>`_,
  `@LevskiWeng (Levski Weng) <https://github.com/LevskiWeng>`_,
  `@rpopescu <https://github.com/rpopescu>`_,
  `@gabime (Gabi Melman) <https://github.com/gabime>`_,
  `@cubicool (Jeremy Moles) <https://github.com/cubicool>`_,
  `@jkflying (Julian Kent) <https://github.com/jkflying>`_,
  `@LogicalKnight (Sean L) <https://github.com/LogicalKnight>`_,
  `@inguin (Ingo van Lil) <https://github.com/inguin>`_ and
  `@Jopie64 (Johan) <https://github.com/Jopie64>`_.

* Fixed portability issues (mostly causing test failures) on ARM, ppc64, ppc64le,
  s390x and SunOS 5.11 i386
  (`#138 <https://github.com/fmtlib/fmt/issues/138>`_,
  `#179 <https://github.com/fmtlib/fmt/issues/179>`_,
  `#180 <https://github.com/fmtlib/fmt/issues/180>`_,
  `#202 <https://github.com/fmtlib/fmt/issues/202>`_,
  `#225 <https://github.com/fmtlib/fmt/issues/225>`_,
  `Red Hat Bugzilla Bug 1260297 <https://bugzilla.redhat.com/show_bug.cgi?id=1260297>`_).
  Thanks to `@Naios <https://github.com/Naios>`_,
  `@jackyf (Eugene V. Lyubimkin) <https://github.com/jackyf>`_ and Dave Johansen.

* Fixed a name conflict with macro ``free`` defined in
  ``crtdbg.h`` when ``_CRTDBG_MAP_ALLOC`` is set
  (`#211 <https://github.com/fmtlib/fmt/issues/211>`_).

* Fixed shared library build on OS X
  (`#212 <https://github.com/fmtlib/fmt/pull/212>`_).
  Thanks to `@dean0x7d (Dean Moldovan) <https://github.com/dean0x7d>`_.

* Fixed an overload conflict on MSVC when ``/Zc:wchar_t-`` option is specified
  (`#214 <https://github.com/fmtlib/fmt/pull/214>`_).
  Thanks to `@slavanap (Vyacheslav Napadovsky) <https://github.com/slavanap>`_.

* Improved compatibility with MSVC 2008
  (`#236 <https://github.com/fmtlib/fmt/pull/236>`_).
  Thanks to `@Jopie64 (Johan) <https://github.com/Jopie64>`_.

* Improved compatibility with bcc32
  (`#227 <https://github.com/fmtlib/fmt/issues/227>`_).

* Fixed ``static_assert`` detection on Clang
  (`#228 <https://github.com/fmtlib/fmt/pull/228>`_).
  Thanks to `@dean0x7d (Dean Moldovan) <https://github.com/dean0x7d>`_.

1.1.0 - 2015-03-06
------------------

* Added ``BasicArrayWriter``, a class template that provides operations for
  formatting and writing data into a fixed-size array
  (`#105 <https://github.com/fmtlib/fmt/issues/105>`_ and
  `#122 <https://github.com/fmtlib/fmt/issues/122>`_):

  .. code:: c++
  
    char buffer[100];
    fmt::ArrayWriter w(buffer);
    w.write("The answer is {}", 42);

* Added `0 A.D. <http://play0ad.com/>`_ and `PenUltima Online (POL)
  <http://www.polserver.com/>`_ to the list of notable projects using C++ Format.

* C++ Format now uses MSVC intrinsics for better formatting performance
  (`#115 <https://github.com/fmtlib/fmt/pull/115>`_,
  `#116 <https://github.com/fmtlib/fmt/pull/116>`_,
  `#118 <https://github.com/fmtlib/fmt/pull/118>`_ and
  `#121 <https://github.com/fmtlib/fmt/pull/121>`_).
  Previously these optimizations where only used on GCC and Clang.
  Thanks to `@CarterLi <https://github.com/CarterLi>`_ and
  `@objectx <https://github.com/objectx>`_.

* CMake install target (`#119 <https://github.com/fmtlib/fmt/pull/119>`_).
  Thanks to `@TrentHouliston <https://github.com/TrentHouliston>`_.

  You can now install C++ Format with ``make install`` command.

* Improved `Biicode <http://www.biicode.com/>`_ support
  (`#98 <https://github.com/fmtlib/fmt/pull/98>`_ and
  `#104 <https://github.com/fmtlib/fmt/pull/104>`_). Thanks to
  `@MariadeAnton <https://github.com/MariadeAnton>`_ and
  `@franramirez688 <https://github.com/franramirez688>`_.

* Improved support for building with `Android NDK
  <https://developer.android.com/tools/sdk/ndk/index.html>`_
  (`#107 <https://github.com/fmtlib/fmt/pull/107>`_).
  Thanks to `@newnon <https://github.com/newnon>`_.
  
  The `android-ndk-example <https://github.com/fmtlib/android-ndk-example>`_
  repository provides and example of using C++ Format with Android NDK:

  .. image:: https://raw.githubusercontent.com/fmtlib/android-ndk-example/
            master/screenshot.png

* Improved documentation of ``SystemError`` and ``WindowsError``
  (`#54 <https://github.com/fmtlib/fmt/issues/54>`_).

* Various code improvements
  (`#110 <https://github.com/fmtlib/fmt/pull/110>`_,
  `#111 <https://github.com/fmtlib/fmt/pull/111>`_
  `#112 <https://github.com/fmtlib/fmt/pull/112>`_).
  Thanks to `@CarterLi <https://github.com/CarterLi>`_.

* Improved compile-time errors when formatting wide into narrow strings
  (`#117 <https://github.com/fmtlib/fmt/issues/117>`_).

* Fixed ``BasicWriter::write`` without formatting arguments when C++11 support
  is disabled (`#109 <https://github.com/fmtlib/fmt/issues/109>`_).

* Fixed header-only build on OS X with GCC 4.9
  (`#124 <https://github.com/fmtlib/fmt/issues/124>`_).

* Fixed packaging issues (`#94 <https://github.com/fmtlib/fmt/issues/94>`_).

* Added `changelog <https://github.com/fmtlib/fmt/blob/master/ChangeLog.rst>`_
  (`#103 <https://github.com/fmtlib/fmt/issues/103>`_).

1.0.0 - 2015-02-05
------------------

* Add support for a header-only configuration when ``FMT_HEADER_ONLY`` is
  defined before including ``format.h``:

  .. code:: c++

    #define FMT_HEADER_ONLY
    #include "format.h"

* Compute string length in the constructor of ``BasicStringRef``
  instead of the ``size`` method
  (`#79 <https://github.com/fmtlib/fmt/issues/79>`_).
  This eliminates size computation for string literals on reasonable optimizing
  compilers.

* Fix formatting of types with overloaded ``operator <<`` for ``std::wostream``
  (`#86 <https://github.com/fmtlib/fmt/issues/86>`_):

  .. code:: c++

    fmt::format(L"The date is {0}", Date(2012, 12, 9));

* Fix linkage of tests on Arch Linux
  (`#89 <https://github.com/fmtlib/fmt/issues/89>`_).

* Allow precision specifier for non-float arguments
  (`#90 <https://github.com/fmtlib/fmt/issues/90>`_):

  .. code:: c++

    fmt::print("{:.3}\n", "Carpet"); // prints "Car"

* Fix build on Android NDK
  (`#93 <https://github.com/fmtlib/fmt/issues/93>`_)

* Improvements to documentation build procedure.

* Remove ``FMT_SHARED`` CMake variable in favor of standard `BUILD_SHARED_LIBS
  <http://www.cmake.org/cmake/help/v3.0/variable/BUILD_SHARED_LIBS.html>`_.

* Fix error handling in ``fmt::fprintf``.

* Fix a number of warnings.

0.12.0 - 2014-10-25
-------------------

* [Breaking] Improved separation between formatting and buffer management.
  ``Writer`` is now a base class that cannot be instantiated directly.
  The new ``MemoryWriter`` class implements the default buffer management
  with small allocations done on stack. So ``fmt::Writer`` should be replaced
  with ``fmt::MemoryWriter`` in variable declarations.

  Old code:

  .. code:: c++

    fmt::Writer w;

  New code: 

  .. code:: c++

    fmt::MemoryWriter w;

  If you pass ``fmt::Writer`` by reference, you can continue to do so:

  .. code:: c++

      void f(fmt::Writer &w);

  This doesn't affect the formatting API.

* Support for custom memory allocators
  (`#69 <https://github.com/fmtlib/fmt/issues/69>`_)

* Formatting functions now accept `signed char` and `unsigned char` strings as
  arguments (`#73 <https://github.com/fmtlib/fmt/issues/73>`_):

  .. code:: c++

    auto s = format("GLSL version: {}", glGetString(GL_VERSION));

* Reduced code bloat. According to the new `benchmark results
  <https://github.com/fmtlib/fmt#compile-time-and-code-bloat>`_,
  cppformat is close to ``printf`` and by the order of magnitude better than
  Boost Format in terms of compiled code size.

* Improved appearance of the documentation on mobile by using the `Sphinx
  Bootstrap theme <http://ryan-roemer.github.io/sphinx-bootstrap-theme/>`_:

  .. |old| image:: https://cloud.githubusercontent.com/assets/576385/4792130/
                   cd256436-5de3-11e4-9a62-c077d0c2b003.png

  .. |new| image:: https://cloud.githubusercontent.com/assets/576385/4792131/
                   cd29896c-5de3-11e4-8f59-cac952942bf0.png
  
  +-------+-------+
  |  Old  |  New  |
  +-------+-------+
  | |old| | |new| |
  +-------+-------+

0.11.0 - 2014-08-21
-------------------

* Safe printf implementation with a POSIX extension for positional arguments:

  .. code:: c++

    fmt::printf("Elapsed time: %.2f seconds", 1.23);
    fmt::printf("%1$s, %3$d %2$s", weekday, month, day);

* Arguments of ``char`` type can now be formatted as integers
  (Issue `#55 <https://github.com/fmtlib/fmt/issues/55>`_):

  .. code:: c++

    fmt::format("0x{0:02X}", 'a');

* Deprecated parts of the API removed.

* The library is now built and tested on MinGW with Appveyor in addition to
  existing test platforms Linux/GCC, OS X/Clang, Windows/MSVC.

0.10.0 - 2014-07-01
-------------------

**Improved API**

* All formatting methods are now implemented as variadic functions instead
  of using ``operator<<`` for feeding arbitrary arguments into a temporary
  formatter object. This works both with C++11 where variadic templates are
  used and with older standards where variadic functions are emulated by
  providing lightweight wrapper functions defined with the ``FMT_VARIADIC``
  macro. You can use this macro for defining your own portable variadic
  functions:

  .. code:: c++

    void report_error(const char *format, const fmt::ArgList &args) {
      fmt::print("Error: {}");
      fmt::print(format, args);
    }
    FMT_VARIADIC(void, report_error, const char *)

    report_error("file not found: {}", path);

  Apart from a more natural syntax, this also improves performance as there
  is no need to construct temporary formatter objects and control arguments'
  lifetimes. Because the wrapper functions are very lightweight, this doesn't
  cause code bloat even in pre-C++11 mode.

* Simplified common case of formatting an ``std::string``. Now it requires a
  single function call:

  .. code:: c++

    std::string s = format("The answer is {}.", 42);

  Previously it required 2 function calls:

  .. code:: c++

    std::string s = str(Format("The answer is {}.") << 42);

  Instead of unsafe ``c_str`` function, ``fmt::Writer`` should be used directly
  to bypass creation of ``std::string``:

  .. code:: c++

    fmt::Writer w;
    w.write("The answer is {}.", 42);
    w.c_str();  // returns a C string

  This doesn't do dynamic memory allocation for small strings and is less error
  prone as the lifetime of the string is the same as for ``std::string::c_str``
  which is well understood (hopefully).

* Improved consistency in naming functions that are a part of the public API.
  Now all public functions are lowercase following the standard library
  conventions. Previously it was a combination of lowercase and
  CapitalizedWords.
  Issue `#50 <https://github.com/fmtlib/fmt/issues/50>`_.

* Old functions are marked as deprecated and will be removed in the next
  release.

**Other Changes**

* Experimental support for printf format specifications (work in progress):

  .. code:: c++

    fmt::printf("The answer is %d.", 42);
    std::string s = fmt::sprintf("Look, a %s!", "string");

* Support for hexadecimal floating point format specifiers ``a`` and ``A``:

  .. code:: c++

    print("{:a}", -42.0); // Prints -0x1.5p+5
    print("{:A}", -42.0); // Prints -0X1.5P+5

* CMake option ``FMT_SHARED`` that specifies whether to build format as a
  shared library (off by default).

0.9.0 - 2014-05-13
------------------

* More efficient implementation of variadic formatting functions.

* ``Writer::Format`` now has a variadic overload:

  .. code:: c++

    Writer out;
    out.Format("Look, I'm {}!", "variadic");

* For efficiency and consistency with other overloads, variadic overload of
  the ``Format`` function now returns ``Writer`` instead of ``std::string``.
  Use the ``str`` function to convert it to ``std::string``:

  .. code:: c++

    std::string s = str(Format("Look, I'm {}!", "variadic"));

* Replaced formatter actions with output sinks: ``NoAction`` -> ``NullSink``,
  ``Write`` -> ``FileSink``, ``ColorWriter`` -> ``ANSITerminalSink``.
  This improves naming consistency and shouldn't affect client code unless
  these classes are used directly which should be rarely needed.

* Added ``ThrowSystemError`` function that formats a message and throws
  ``SystemError`` containing the formatted message and system-specific error
  description. For example, the following code

  .. code:: c++

    FILE *f = fopen(filename, "r");
    if (!f)
      ThrowSystemError(errno, "Failed to open file '{}'") << filename;

  will throw ``SystemError`` exception with description
  "Failed to open file '<filename>': No such file or directory" if file
  doesn't exist.

* Support for AppVeyor continuous integration platform.

* ``Format`` now throws ``SystemError`` in case of I/O errors.

* Improve test infrastructure. Print functions are now tested by redirecting
  the output to a pipe.

0.8.0 - 2014-04-14
------------------

* Initial release
