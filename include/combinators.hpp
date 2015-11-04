// gcomb : composable generator combinators for elegant
//         manipulation of infinite data streams.
//
// combinators : utilities for composing generators
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE
//

#ifndef GCOMB_COMBINATORS_HPP
#define GCOMB_COMBINATORS_HPP

#include <type_traits>
#include <utility>

#include "algebraic_generator.hpp"
#include "generator.hpp"

namespace gcomb
{
namespace detail
{
    // positive integer sequences
    //   (std::integer_sequence is C++14 only)
    //
    template <std::size_t ...>
    struct seq {};

    template <std::size_t N, std::size_t ... S>
    struct seq_gen : seq_gen <N-1, N-1, S...> {};

    template <std::size_t ... S>
    struct seq_gen <0, S...>
    {
        using type = seq<S...>;
    };

    template <typename F, typename ... Args>
    struct is_good_call
    {
        struct success {};
        struct failure {};

        template<typename F_, typename... Args_,
            typename = typename std::result_of
                <F_(Args_...)>::type>
        static constexpr success test (int) { return {}; }

        template<typename F_, typename... Args_>
        static constexpr failure test (...) { return {}; }

        static constexpr bool value = 
            std::is_same<decltype(test<F, Args...>(0)), success>::value;
    };

    template <typename F, typename Tup, std::size_t ... S>
    auto call_impl (F&& f, Tup&& args, seq<S...>)
        -> typename std::result_of
                <F(decltype(std::get<S>(std::forward<Tup>(args)))...)>::type
    {
        return std::forward<F>(f) (std::get<S>(std::forward<Tup>(args))...);
    }

    // call a function on a tuple of it's arguments
    //
    template <typename F, typename Tup>
    auto call (F&& f, Tup&& args)
        -> decltype(call_impl
            (std::forward<F>(f),
             std::forward<Tup>(args),
             seq_gen<std::tuple_size<Tup>::value>::type))
    {
        return call_impl
            (std::forward<F>(f),
             std::forward<Tup>(args),
             seq_gen<std::tuple_size<Tup>::value>::type);
    }
} // namespace detail

    template <typename T, typename ... Ts>
    generator<std::tuple<T, Ts...>> braid
        (generator<T> const& g, generator<Ts> const& ... gs) noexcept
    {
        return generator<std::tuple<T, Ts...>>
            ([g,gs...] (void) { return std::make_tuple (g(), gs()...); } );
    }


    // greedy instantiate template, but if it fails let us
    // assume T is a tuple type and defer to the next
    // bind function that unpacks the tuple.
    //
    template <typename F, typename T,
        typename U = typename std::result_of<F(T)>::type,
        typename = typename std::enable_if
            <detail::is_good_call<F, T>::value>::type>
    generator<U> bind (F&& f, generator<T> const& g) noexcept
    {
        return generator<U> ([g,f] (void) { return g (f); });
    }


    template <typename F, typename T, typename ... Ts,
        typename U = typename std::result_of<F(T, Ts...)>::type>
    generator<U> bind (F&& f, generator<std::tuple<T,Ts...>> const& g) noexcept
    {
        auto call = [f](std::tuple<T,Ts...> const& tup)
        {
            return detail::call (f, tup);
        };

        return generator<U> ([g,f] (void) { return g (call); } );
    }


    template <typename F, typename T, typename ... Ts,
        typename U = typename std::result_of<F(T, Ts...)>::type,
        typename = typename std::enable_if<sizeof...(Ts) >= 1>::type>
    generator<U> bind (F&& f, generator<T> const& g, generator<Ts> const&... gs)
        noexcept
    {
        return bind (f, braid (g, gs...));
    }
} // namspace gcomb

#endif // ifndef GCOMB_COMBINATORS
