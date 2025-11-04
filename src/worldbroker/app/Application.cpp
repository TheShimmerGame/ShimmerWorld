
#include "Application.hpp"

#include <ranges>
#include <vector>
#include <print>

wb::Application::Application()
{
}

wb::Application::~Application() = default;

int wb::Application::Run( int /*argc*/, char ** /*argv[] */ )
{
    std::vector< int32_t > tests{ 0, 1, 2, 3, 4 };
    auto new_tests = tests
        | std::views::filter(
            []( int32_t v )
            {
                return v % 2 == 0;
            } )
        | std::views::transform(
            []( int32_t v )
            {
                return v * v;
            } );

    for ( int32_t test_id : new_tests )
        std::print( "ID: {}", test_id );

    return 0;
}
