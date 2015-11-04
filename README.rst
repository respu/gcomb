=========
algebraic
=========

This is a simple reference implementation for fixed at instantiation
algebraic data types for C++11/14.

It differs from ``std::optional`` and ``boost::variant`` in that
an object of type ``algebraic<T_1,...,T_n>`` has a fixed instantiation
type ``T_i`` throughout it's lifetime; a consequence of this is that
every object type ``algebraic<T_1,...,T_n>`` has a never-empty guarantee;
i.e., they are non-nullable.

-------
LICENSE
-------
This implementation is licensed under the OSI Approved MIT License Copyrighted
(c) 2015 by Dalton Woodard. Please see the file LICENSE.md distributed with
this package for details.
