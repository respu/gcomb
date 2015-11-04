=====
gcomb
=====

gcomb is a minimal C++11/14 framework of orthogonal components for creating
composable, low overhead, and type safe generators in a combinator/continuation
passing style.

By default the primitives assume generators produce infinitely many terms, but
the components are available for constructing finite generators through use
of algebraic data types (see ``algebraic.hpp`` and ``algebraic_generators.hpp``)
and the ``bot`` generator (see ``generators.hpp``). Since there is no type-
generic way to specify a finite generator (at least in C++; languages with
dependent type systems can pull it off), these must, for the most
part, be constructed by hand. The exception to this is the ``bound`` combinator
which returns a generator producing the original values ``n`` times, then
reverts to being identically ``bot``.

----
Info
----

Listed below are several examples of generators created with ``gcomb``.

* Here is a generator for the powers of 2:

.. code-block:: c++

    // prod takes: (start, factor)
    //
    auto pows2 = gcomb::prod (1, 2);


* Here is a generator for the prime numbers:

.. code-block:: c++

    auto isprime  = [](uint64_t n) -> bool
    {
        // your favorite algorithm here
    };

    auto nthprime = [](uint64_t n) mutable -> uint64_t
    {
        // c++11 allows single instantiation of static variables
        static std::vector<uint64_t> naturals {};

        naturals.resize (n);

        for (uint64_t i = naturals.back() + 1;; ++i) {
            if (isprime (i)) {
                naturals.push_back (i);
                break;
            }
        }

        return naturals.back();
    };

    // here's our generator (count takes: (start, step=1)):
    //
    auto primes = gcomb::bind (nthprime, count (1Ull));


* This generator produces lines of input from ``stdin``:

.. code-block:: c++

    auto lines = gcomb::generator<std::string>
        ([](void) -> std::string
        {
            std::string l;
            std::getline (std::cin, l);
            return l;
        });


* Let's break the lines on whitespace:

.. code-block:: c++

    auto line_to_words =
        [](std::string & l)
        {
            std::vector<std::string> words;
           
            std::size_t pos;
            while (std::string::npos != (pos = l.find (" "))) {
                words.push_back (l.substr (0, pos));
                l.erase (0, pos + 1); // for simplicity assume a single space
            }

            return words;
        };

    auto linewords = gcomb::bind (line_to_words, lines);

    // how many words are in the first line?
    //      (notice the continuation passing style)
    auto length = [](auto && v) { return v.size(); };
    auto nwords = linewords (length);

    // ... do something

    // and the second? (same call as before, but next line!)
    //
    nwords = linewords (length);


-------
License
-------

gcomb is licensed under the OSI Approved MIT License Copyrighted (c) 2015 by 
Dalton Woodard. Please see the file LICENSE.md distributed with this package 
for details.
