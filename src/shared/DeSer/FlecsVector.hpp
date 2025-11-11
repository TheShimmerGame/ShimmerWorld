
#pragma once

#include <concepts>
#include <vector>

#include <flecs.h>
// Source: https://github.com/SanderMertens/flecs/blob/master/examples/cpp/reflection/ser_std_vector/src/main.cpp

namespace shm::deser::fl
{
    // clang-format off
    template< typename T >
    concept VectorType = requires( T )
    {
        typename T::value_type;
    } && std::convertible_to< T, std::vector< typename T::value_type > >;
    // clang-format on

    template< typename Elem, VectorType Vector = std::vector< Elem > >
    flecs::opaque< Vector, Elem > vector_deser( flecs::world & world )
    {
        return flecs::opaque< Vector, Elem >()
            .as_type( world.vector< Elem >() )

            // Forward elements of std::vector value to serializer
            .serialize( []( const flecs::serializer * s, const Vector * data )
                        {
                            for ( const auto & el : *data )
                            {
                                s->value( el );
                            }
                            return 0;
                        } )

            // Return vector count
            .count( []( const Vector * data )
                    {
                        return data->size();
                    } )

            // Resize contents of vector
            .resize( []( Vector * data, size_t size )
                     {
                         data->resize( size );
                     } )

            // Ensure element exists, return pointer
            .ensure_element( []( Vector * data, size_t elem )
                             {
                                 if ( data->size() <= elem )
                                 {
                                     data->resize( elem + 1 );
                                 }

                                 return &data->data()[ elem ];
                             } );
    }
} // namespace shm::deser::fl
