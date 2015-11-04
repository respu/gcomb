// gcomb : composable generator combinators for elegant
//         manipulation of infinite data streams.
//
// algebraic_generator :
//
//      utilities for creating generators of the form:
//
//          generator<T_1 | T_2 | ... | T_N>
//
//      where T_1 | T_2 | ... | T_N represents a discriminated
//      union of these types.
//
// note:
//      This makes use of the algebraic::algebraic<T_1,...,T_N> type.
//      See algebraic.hpp for details.
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE
//

#ifndef GCOMB_ALGEBRAIC_GENERATOR_HPP
#define GCOMB_ALGEBRAIC_GENERATOR_HPP

#include "generator.hpp"
#include "algebraic/include/algebraic.hpp"

namespace gcomb
{
    template <typename T, typename ... Ts>
    using algebraic_generator = generator<algebraic::algebraic<T, Ts...>>;
} // namespace gcomb

#endif // ifndef GCOMB_ALGEBRAIC_GENERATOR_HPP
