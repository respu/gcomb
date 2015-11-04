=====
gcomb
=====

gcomb is a minimal C++11/14 framework of orthogonal components for creating
omposable, low overhead, and type safe generators in a combinator/continuation
passing style.

By default the primitives assume generators produce infinitely many terms, but
the components are available for constructing finite generators through use
of algebraic data types (see ``algebraic.hpp`` and ``algebraic_generators.hpp``)
and the ``bot`` generator (see ``generators.hpp``). Since there is no type-
generic way to specify a finite generator (at least in C++; languages with
dependent type systems can pull it off), these must, for the most
part, be constructed by hand.

----
Info
----

For a detailed explanation of how gcomb works and how to use gcomb for data
generation, please see the `docs </docs>`_.

-------
License
-------

gcomb is licensed under the OSI Approved MIT License Copyrighted (c) 2015 by 
Dalton Woodard. Please see the file LICENSE.md distributed with this package 
for details.
