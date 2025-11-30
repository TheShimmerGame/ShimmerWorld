#pragma once

#include <memory>
#include <thread>

namespace shm
{
    struct Logger;
}

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

        Application( const Application & )             = delete;
        Application & operator=( const Application & ) = delete;
        Application( Application && )                  = delete;
        Application & operator=( Application && )      = delete;

        int Run( int argc, char * argv[] );

    protected:
        bool InitializeLoggingSystem();

    private:
        std::unique_ptr< flecs::world > m_broker_world;
        std::unique_ptr< shm::Logger > m_logger;
    };
} // namespace wb
