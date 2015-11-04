// algebraic (variant) and recursive algebraic data types in C++
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#ifndef ALGEBRAIC_HPP
#define ALGEBRAIC_HPP

//
// Algebraic types are discriminated unions of types T1, T2, ..., TN,
// initialized once with a single fixed type Ti, which is invariant through
// the lifetime of the object. Moreover, such a type supports a never-empty
// guarantee; that is, expressions of the form
//      fnk::algebraic<T1,T2,...,TN> a;
// are not supported. Rather, they must be initialized at declaration with an
// expression of type U, implicitly convertible to one of T1, ..., TN.
//
// note:
//      This is NOT the same as a boost::variant or std::optional.
//

#include <algorithm>
#include <memory>
#include <type_traits>

namespace algebraic
{
namespace detail
{
    template <typename T, typename ... Ts>
    struct algebraic_internal_storage
    {
    public:
        ~algebraic_internal_storage (void) noexcept = default;

        template <typename U> 
        inline constexpr U&       value (void) &
            { return *addressof<U>(); }

        template <typename U>
        inline constexpr U&&      value (void) &&
            { return std::move (*addressof<U>()); }

        template <typename U>
        inline constexpr U const& value (void) const&
            { return *addressof<U>(); }
       
        template <typename U>
        inline constexpr U&       operator* (void) &
            { return value<U>(); }
        
        template <typename U>
        inline constexpr U&&      operator* (void) &&
            { return value<U>(); }
        
        template <typename U>
        inline constexpr U const& operator* (void) const&
            { return value<U>(); }
        
        template <typename U>
        inline constexpr U*       addressof (void) noexcept
            { return reinterpret_cast<U*> (data); }

        template <typename U>
        inline constexpr U const* addressof (void) const noexcept
            { return reinterpret_cast<U const*> (data); }

        template <typename U>
        inline constexpr U*       operator& (void) noexcept
            { return addressof<U>(); }

        template <typename U>
        inline constexpr U const* operator& (void) const noexcept
            { return addressof<U>(); }
    private: 
        static constexpr auto align = std::max ({alignof(T), alignof(Ts)...});
        static constexpr auto size  = std::max ({sizeof(T), sizeof(Ts)...});

        alignas (align) unsigned char data [size];
    };

    constexpr bool any_true (bool b) noexcept
    {
        return b;
    }

    template <typename Bool, typename ... Bools>
    constexpr bool any_true (bool b0, Bool b, Bools ... bs) noexcept
    {
        return b0 || any_true (b, bs...);
    }

    template <typename U, typename ... Ts>
    struct type_to_index;

    template <typename U, typename ... Ts>
    struct type_to_index <U, U, Ts...>
        : public std::integral_constant<std::size_t, 0> {};

    template <typename U, typename T, typename ... Ts>
    struct type_to_index <U, T, Ts...>
        : public std::integral_constant
            <std::size_t, 1 + type_to_index<U, Ts...>::value> {};
   
    template <typename U>
    struct type_to_index<U>
        { static_assert (sizeof(U) == 0, "type not found"); };
} // namespace detail

    template <typename T, class Alloc = std::allocator<T>>
    struct recursive
    {
    public:
        using type            = T;
        using reference       = T&;
        using const_reference = T const&;
        using pointer         = T*;
        using const_pointer   = T const*;

        recursive (void) noexcept = default;

        recursive (T && t) : data (alloc.allocate (1))
        {
            new (data.get()) T (t);
        }

        recursive (T const& t) : data (alloc.allocate (1))
        {
            new (data.get()) T (t);
        }

        recursive (recursive<T> && r) noexcept
            : data (std::move (r.ptr()))
        {}
 
        recursive (recursive<T> const& r)
            : data (alloc.allocate (1))
        {
            new (data.ptr()) T (r.get());
        }

        ~recursive (void) noexcept = default;

        void swap (recursive const& other) noexcept
        {
            std::swap (data, other.data);
        }

        inline constexpr T&       value (void) &
            { return *addressof(); }

        inline constexpr T&&      value (void) &&
            { return std::move (*addressof()); }

        inline constexpr T const& value (void) const&
            { return *addressof(); }


        inline constexpr T&       operator* (void) &
            { return value(); }

        inline constexpr T&&      operator* (void) &&
            { return value(); }

        inline constexpr T const& operator* (void) const&
            { return value(); }


        inline constexpr T*       addressof (void) noexcept
            { return reinterpret_cast<T*> (data.get()); }

        inline constexpr T const* addressof (void) const noexcept
            { return reinterpret_cast<T const*> (data.get()); }


        inline constexpr T*       operator& (void) noexcept
            { return addressof(); }

        inline constexpr T const* operator& (void) const noexcept
            { return addressof(); }


        inline constexpr T* ptr (void) noexcept
            { return data.get(); }

        inline constexpr T const* ptr (void) const noexcept
            { return data.get(); }

    private:
        Alloc alloc;

        // recursive<T> is owning, as it merely replaces
        // the existence of an object of type T in a variant.
        std::unique_ptr<T> data; 
    };


    template <typename T, typename ... Ts>
    struct algebraic
    {
    public:
        algebraic (void) : tindex (0)
        {
            static_assert
                (sizeof(T) == 0, "no default construction of algebraic type");
        }

        template <typename U>
        algebraic (U && u)
            : tindex (detail::type_to_index<U, T, Ts...>::value)
        {
            static_assert
            (detail::any_true
                (std::is_same<typename std::remove_cv<U>::type, T>::value,
                 std::is_same<typename std::remove_cv<U>::type, Ts>::value...),
            "no possible conversion");

            auto const address =
                reinterpret_cast<void*> (storage.template addressof<U>());

            new (address) U (std::forward<U>(u));
        }

        template <typename U, bool /*no template redeclaration*/ = bool{}>
        algebraic (U && u)
            : tindex
                (detail::type_to_index
                <recursive<typename std::remove_cv<U>::type>, T, Ts...>::value)
        {
            using W = typename std::remove_cv<U>::type;

            auto const address =
                reinterpret_cast<void*> (storage.template addressof<W>());

            new (address) recursive<W> (std::forward<U> (u));
        }

        algebraic (algebraic<T, Ts...> &&) noexcept = default;
        algebraic (algebraic<T, Ts...> const&)      = default;

        algebraic & operator= (algebraic<T, Ts...> &&) noexcept = default;
        algebraic & operator= (algebraic<T, Ts...> const&)      = default;

        ~algebraic (void) noexcept = default;

        template <typename U>
        static algebraic<T, Ts...> emplace (U && u)
            noexcept (std::is_nothrow_move_constructible<U>::value)
        {
            static_assert
                (detail::any_true
                    (std::is_same<U,T>::value, std::is_same<U,Ts>::value...),
            "no possible conversion");

            return algebraic (U (std::forward<U>(u))); 
        }


        template <typename U, typename ... Args>
        static algebraic<T, Ts...> emplace (Args && ... args)
            noexcept (std::is_nothrow_constructible<U, Args...>::value)
        {
            static_assert
                (detail::any_true
                    (std::is_same<U,T>::value, std::is_same<U,Ts>::value...),
            "no possible conversion");

            return algebraic (U (std::forward<Args>(args)...));
        }


        template <typename U, typename A, typename ... Args>
        static algebraic<T, Ts...> emplace
            (std::initializer_list<A> const& il, Args && ... args)
                noexcept (std::is_nothrow_constructible
                            <U, decltype(il), Args...>::value) 
        {
            static_assert
                (detail::any_true
                    (std::is_same<U,T>::value, std::is_same<U,Ts>::value...),
            "no possible conversion");

            return algebraic (U (il, std::forward<Args>(args)...));
        }


        template <typename U>
        void call_dtor (void) noexcept (std::is_nothrow_destructible<U>::value)
        {
            static_assert
                (detail::any_true
                    (std::is_same<U,T>::value, std::is_same<U,Ts>::value...),
            "no possible conversion");

            reinterpret_cast<U*> (storage.template addressof<U>())->U::~U (); 
        }


        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<U_, T>::value,
                     std::is_same<U_, Ts>::value...)>::type>
        inline constexpr U_ & value (void) &
            { return *addressof<U_>(); }


        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<recursive<U_>, T>::value,
                     std::is_same<recursive<U_>, Ts>::value...)>::type,
            bool /*no template redeclaration*/ = bool{}>
        inline constexpr U& value (void) &
            { return **addressof<recursive<U>>(); }


        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<U_, T>::value,
                     std::is_same<U_, Ts>::value...)>::type>
        inline constexpr U&& value (void) &&
            { return std::move (*addressof<U>()); }
 

        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<recursive<U_>, T>::value,
                     std::is_same<recursive<U_>, Ts>::value...)>::type,
            bool /*no template redeclaration*/ = bool{}>
        inline constexpr U&& value (void) &&
            { return std::move (**addressof<recursive<U>>()); } 


        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<U_, T>::value,
                     std::is_same<U_, Ts>::value...)>::type>
        inline constexpr U const& value (void) const&
            { return *addressof<U>(); }
  

        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<recursive<U_>, T>::value,
                     std::is_same<recursive<U_>, Ts>::value...)>::type,
            bool /*no template redeclaration*/ = bool{}>
        inline constexpr U const& value (void) const&
            { return **addressof<recursive<U>>(); }


        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<U_, T>::value,
                     std::is_same<U_, Ts>::value...)>::type>
        inline constexpr U&  operator* (void) &
            { return value<U>(); }
   

        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<recursive<U_>, T>::value,
                     std::is_same<recursive<U_>, Ts>::value...)>::type,
            bool /*no template redeclaration*/ = bool{}>
        inline constexpr U&  operator* (void) &
            { return value<recursive<U>>(); }


        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<U_, T>::value,
                     std::is_same<U_, Ts>::value...)>::type>
        inline constexpr U&& operator* (void) &&
            { return value<U>(); }
    

        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<recursive<U_>, T>::value,
                     std::is_same<recursive<U_>, Ts>::value...)>::type,
            bool /*no template redeclaration*/ = bool{}>
        inline constexpr U&& operator* (void) &&
            { return value<recursive<U>>(); }


        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<U_, T>::value,
                     std::is_same<U_, Ts>::value...)>::type>
        inline constexpr U const& operator* (void) const&
            { return value<U>(); }
     

        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<recursive<U_>, T>::value,
                     std::is_same<recursive<U_>, Ts>::value...)>::type,
            bool /*no template redeclaration*/ = bool{}>
        inline constexpr U const& operator* (void) const&
            { return value<recursive<U>>(); }


        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<U_, T>::value,
                     std::is_same<U_, Ts>::value...)>::type>
        inline constexpr U* addressof (void) noexcept
        {
            return reinterpret_cast<U*> (storage.template addressof<U>());
        }


        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<recursive<U_>, T>::value,
                     std::is_same<recursive<U_>, Ts>::value...)>::type,
            bool /*no template redeclaration*/ = bool{}>
        inline constexpr U* addressof (void) noexcept
        {
            return reinterpret_cast<U*>
                (storage.template addressof<recursive<U>>()->addressof());
        }


        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<U_, T>::value,
                     std::is_same<U_, Ts>::value...)>::type>
        inline constexpr U const* addressof (void) const noexcept
        {
            return reinterpret_cast<U const*>
                (storage.template addressof<U>());
        }


        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<recursive<U_>, T>::value,
                     std::is_same<recursive<U_>, Ts>::value...)>::type,
            bool /*no template redeclaration*/ = bool{}>
        inline constexpr U const* addressof (void) const noexcept
        {
            return reinterpret_cast<U const*>
                (storage.template addressof<recursive<U>>()->addressof());
        }


        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<U_, T>::value,
                     std::is_same<U_, Ts>::value...)>::type>
        inline constexpr U* operator& (void) noexcept
            { return addressof<U>(); }
 

        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<recursive<U_>, T>::value,
                     std::is_same<recursive<U_>, Ts>::value...)>::type,
            bool /*no template redeclaration*/ = bool{}>
        inline constexpr U* operator& (void) noexcept
            { return addressof<recursive<U>>(); }


        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<U_, T>::value,
                     std::is_same<U_, Ts>::value...)>::type>
        inline constexpr U const* operator& (void) const noexcept
            { return addressof<U>(); }
  

        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<recursive<U_>, T>::value,
                     std::is_same<recursive<U_>, Ts>::value...)>::type,
            bool /*no template redeclaration*/ = bool{}>
        inline constexpr U const* operator& (void) const noexcept
            { return addressof<recursive<U>>(); }


        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<U_, T>::value,
                     std::is_same<U_, Ts>::value...)>::type>
        constexpr algebraic<T, Ts...> & operator= (U && u)
            noexcept (std::is_nothrow_move_assignable<U>::value)
        {
            auto const address =
                reinterpret_cast<void*> (storage.template addressof<U>());

            new (address) U (std::forward<U>(u));
            return *this;
        }
   

        template <typename U, typename U_ = typename std::remove_cv<U>::type,
            typename = typename
                std::enable_if<detail::any_true
                    (std::is_same<recursive<U_>, T>::value,
                     std::is_same<recursive<U_>, Ts>::value...)>::type,
            bool /*no template redeclaration*/ = bool{}>
        constexpr algebraic<T, Ts...> & operator= (U && u)
            noexcept (std::is_nothrow_move_assignable<U>::value)
        {
            auto const address =
                reinterpret_cast<void*>
                    (storage.template addressof<recursive<U>>());

            new (address) recursive<U> (std::forward<U>(u));
            return *this;
        }


        inline std::size_t type_index (void) const noexcept
        {
            return tindex;
        }


        template <typename U>
        using is_algebraic_type =
            typename std::conditional
                <detail::any_true
                    (std::is_same<U,T>::value, std::is_same<U,Ts>::value...), 
                std::true_type, 
                std::false_type>::type;

        using ntypes = std::integral_constant<std::size_t, 1 + sizeof...(Ts)>;

        template <std::size_t N,
            typename = typename std::enable_if<N < ntypes::value>::type>
        using type = std::tuple_element_t<N, std::tuple<T, Ts...>>;

        template <typename U,
            typename = typename std::enable_if
                <is_algebraic_type<U>::value>::type>
        using index = detail::type_to_index<U, T, Ts...>;

    private:
        std::size_t const tindex;
        detail::algebraic_internal_storage<T, Ts...> storage;
    };


    template <typename T>
    struct is_algebraic : public std::false_type {};

    template <typename T, typename ... Ts>
    struct is_algebraic<algebraic<T, Ts...>> : public std::true_type {};
    
    template <typename T, typename ... Ts>
    struct is_algebraic<algebraic<T, Ts...> const> : public std::true_type {};

    template <typename T, typename ... Ts>
    struct is_algebraic<algebraic<T, Ts...> volatile>
        : public std::true_type {};

    template <typename T, typename ... Ts>
    struct is_algebraic<algebraic<T, Ts...> const volatile>
        : public std::true_type {};

    template <typename T, typename ... Ts>
    struct is_algebraic<algebraic<T, Ts...> &> : public std::true_type {};
    
    template <typename T, typename ... Ts>
    struct is_algebraic<algebraic<T, Ts...> const&> : public std::true_type {};

    template <typename T, typename ... Ts>
    struct is_algebraic<algebraic<T, Ts...> volatile&>
        : public std::true_type {};

    template <typename T, typename ... Ts>
    struct is_algebraic<algebraic<T, Ts...> const volatile&>
        : public std::true_type {};

    template <typename T, typename ... Ts>
    struct is_algebraic<algebraic<T, Ts...> &&> : public std::true_type {};

    template <typename T, typename ... Ts>
    struct is_algebraic<algebraic<T, Ts...> const&&>
        : public std::true_type {};

    template <typename T, typename ... Ts>
    struct is_algebraic<algebraic<T, Ts...> volatile&&>
        : public std::true_type {};

    template <typename T, typename ... Ts>
    struct is_algebraic<algebraic<T, Ts...> const volatile&&>
        : public std::true_type {};
} // namespace fnk

#endif // ifndef ALGEBRAIC_HPP
