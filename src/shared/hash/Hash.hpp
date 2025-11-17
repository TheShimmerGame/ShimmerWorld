#pragma once

#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include <rfl.hpp>

namespace shm
{
    template< typename T >
    concept is_std_hashable = requires( const T & object ) {
        { std::hash< T >{}( object ) } -> std::convertible_to< std::size_t >;
    };

    template< typename T >
    concept is_iterable = requires( const T & object ) {
        object.begin();
        object.end();
    };

    // TODO: C++26 REFLECTION
    template< typename T >
    concept is_reflectable = requires {
        rfl::fields< T >();
    };

    template< class T >
    inline void hash_combine( std::size_t & seed, const T & v )
    {
        seed ^= ( std::size_t )std::hash< T >{}( v ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
    }

    template< typename T >
    std::size_t hash( const T & object )
    {
        if constexpr ( is_std_hashable< T > )
        {
            return std::hash< T >{}( object );
        }
        else if constexpr ( is_iterable< T > )
        {
            std::size_t result = 0;
            for ( const auto & elem : object )
            {
                if constexpr ( is_std_hashable< std::decay_t< decltype( elem ) > > )
                    hash_combine( result, elem );
                else
                    hash_combine( result, hash( elem ) );
            }
            return result;
        }
        // TODO: C++26 REFLECTION
        else if constexpr ( is_reflectable< T > )
        {
            std::size_t result = 0;
            const auto view    = rfl::to_view( object );
            view.apply( [ & ]( const auto & f )
                        {
                            using FieldT = std::remove_reference_t< decltype( *f.value() ) >;
                            if constexpr ( is_std_hashable< FieldT > )
                                hash_combine( result, *f.value() );
                            else
                                hash_combine( result, hash( *f.value() ) );
                        } );
            return result;
        }
        else
        {
            static_assert( std::is_void_v< T >, "Type is neither std-hashable, iterable, nor reflectable for shm::hash." );
            return 0;
        }
    }

    template< typename... Ts >
    std::size_t hash( const Ts &... objects )
    {
        std::size_t result = 0;
        ( hash_combine( result, objects ), ... );
        return result;
    }

    template< typename T1, typename T2 >
    std::size_t hash( const std::pair< T1, T2 > & p )
    {
        return hash( p.first, p.second );
    }

    template< typename... Ts >
    std::size_t hash( const std::tuple< Ts... > & t )
    {
        return std::apply( []( const auto &... elems )
                           {
                               return hash( elems... );
                           },
                           t );
    }
} // namespace shm

namespace std
{
    template< typename T1, typename T2 >
    struct hash< std::pair< T1, T2 > >
    {
        size_t operator()( const std::pair< T1, T2 > & p ) const noexcept
        {
            return ::shm::hash( p );
        }
    };

    template< typename... Ts >
    struct hash< std::tuple< Ts... > >
    {
        size_t operator()( const std::tuple< Ts... > & t ) const noexcept
        {
            return ::shm::hash( t );
        }
    };
} // namespace std

