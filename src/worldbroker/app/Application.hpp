
#pragma once

#include <chrono>

namespace wb
{
    class Application
    {
    public:

        Application();
        ~Application();

        Application( const Application & ) = delete;
        Application & operator=( const Application & ) = delete;
        Application( Application && )                  = delete;
        Application & operator=( Application && )      = delete;


        int Run( int argc, char * argv[] );

    protected:
        // chrono timer since start
        std::chrono::steady_clock::time_point m_startTime;
    };
}
