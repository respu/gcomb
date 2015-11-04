// gcomb : composable generator combinators for elegant
//         manipulation of infinite data streams.
//
// generator : type definition of abstract
//             generator and some primitive
//             generators.
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE
//

#ifndef GCOMB_GENERATOR_HPP
#define GCOMB_GENERATOR_HPP

#include <functional>
#include <type_traits>
#include <ostream>

namespace gcomb
{
namespace detail
{
    // This function is used to "bottom out" a generator
    // continuation; thus creating a "pure" generator.
    //
    template <typename T>
    inline constexpr auto bot (T&& t) noexcept -> decltype(std::forward<T>(t))
    {
        return std::forward<T>(t);
    }
} // namespace detail

    template <typename T>
    class generator
    {
    private:
        mutable std::function<T (void)> gen;
    public:
        using value_type      = T;
        using reference       = T &;
        using const_reference = T const&;

        // no sensible default
        generator (void) = delete;

        generator (std::function<T (void)> const& g)
            : gen (g)
        {}

        generator (generator &&) noexcept = default;
        generator (generator const&)      = default;

        generator & operator= (generator &&) noexcept = default;
        generator & operator= (generator const&)      = default;

        ~generator (void) noexcept = default;

        void swap (generator & other) noexcept
        {
            std::swap (gen, other.gen);
        }

        // The single entry point for a generator is operator()
        //
        // note:
        //      Please forgive me for the mess of a noexcept(...) and
        //      decltype(...) specificaiton this has become. Unfortunately
        //      g++ still has issues with decltype(auto) as of ver. 5.2.0;
        //      clang has no such issues ... such is life when writing
        //      portable code.
        //
        //      In any case, if both gen() and k(...) calls are noexcept,
        //      then so too will this be noexcept.
        //
        template <typename K>
        auto operator() (K && k = detail::bot<T>) const
            noexcept
                (noexcept(gen()) &&
                noexcept(std::forward<K>(k) (gen())))
            -> decltype (std::forward<K>(k) (gen()))
        {
            return std::forward<K>(k) (gen());
        }

        T && operator () (void) const
            noexcept (noexcept(std::move(std::declval<T>())))
        {
            return this->operator() (std::move<T>);
        }
    };

    template <typename T>
    class generator<T const> : public generator<T> {};

    template <typename T>
    class generator<T volatile> : public generator<T> {};

    template <typename T>
    class generator<T const volatile> : public generator<T> {};

    template <typename T>
    class generator<T &> : public generator<T> {};

    template <typename T>
    class generator<T const&> : public generator<T> {};

    template <typename T>
    class generator<T volatile&> : public generator<T> {};

    template <typename T>
    class generator<T const volatile&> : public generator<T> {};

    template <typename T>
    class generator<T &&> : public generator<T> {};

    // one special overload for generator<T> w.r.t.
    // std::ostream::operator<<
    //
    // note:
    //      This requires that std::ostream::operator<< is
    //      correctly specialized for T.
    //
    template <typename T>
    std::ostream & operator<< (std::ostream & st, generator<T> const& gen)
    {
        return (st << gen ());
    }

//
// some useful default generators
//

    // a constant value generator
    template <typename T>
    generator<T> pure (T && t)
    {
        return generator<T> ([t] (void) { return t; });
    }
} // namespace gcomb

#endif // ifndef GCOMB_GENERATOR_HPP
