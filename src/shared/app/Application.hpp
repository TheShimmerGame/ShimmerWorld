#pragma once

#include <memory>
#include <thread>

namespace flecs
{
    struct world;
}

namespace wb
{
    class Application
    {
    public:
        Application();
        ~Application();

        Application( const Application & )            = delete;
        Application &operator=( const Application & ) = delete;
        Application( Application && )                 = delete;
        Application &operator=( Application && )      = delete;

        int Run( int argc, char *argv[] );

    protected:
        std::unique_ptr< flecs::world > m_broker_world;
    };
} // namespace wb
