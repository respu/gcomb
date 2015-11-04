=========
algebraic
=========

This is a simple reference implementation for fixed type at instantiation
algebraic and recursive algebraic data types in C++11/14.

It differs from ``std::optional`` and ``boost::variant`` in that
an object of type ``algebraic<T_1,...,T_n>`` has a fixed instantiation
type ``T_i`` throughout it's lifetime; a consequence of this is that
every object type ``algebraic<T_1,...,T_n>`` has a never-empty guarantee;
i.e., they are non-nullable.

-------------
What is this?
-------------

Algebraic types are discriminated unions of types ``T_1, ..., T_n``,
initialized once with a single fixed type Ti, which is invariant through
the lifetime of the object. Moreover, such a type supports a never-empty
guarantee; that is, expressions of the form

    ``algebraic<T_1, ...,T_n> a;``

are not supported. Rather, they must be initialized at declaration with an
expression of type ``U``, implicitly convertible to one of ``T_1, ..., T_n``.

Recursive algebraic data types are like the above, except that the type
being declared can appear among the types ``T_1, ..., T_n``.

-------
LICENSE
-------
This implementation is licensed under the OSI Approved MIT License Copyrighted
(c) 2015 by Dalton Woodard. Please see the file LICENSE.md distributed with
this package for details.
