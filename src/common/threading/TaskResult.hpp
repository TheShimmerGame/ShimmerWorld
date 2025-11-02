#pragma once

#include "CancellationSignal.hpp"

#include <boost/assert.hpp>
#include <boost/asio/cancellation_signal.hpp>

#include <future>
#include <memory>

namespace Threading
{
    namespace Detail::ReturnType
    {
        struct Void
        {};
        struct Future
        {};
        struct Cancellable
        {};
    }
    template<typename T>
    struct deduct_false
    {
        constexpr static bool value = false;
    };

    constexpr Detail::ReturnType::Void        DetachedTask;
    constexpr Detail::ReturnType::Future      AwaitableTask;
    constexpr Detail::ReturnType::Cancellable CancellableTask;

    template< typename T >
    class TaskResult
    {
    public:
        using FutureType = std::future< T >;

        TaskResult() = default;

        TaskResult( FutureType && future )
            : m_future( std::move( future ) )
        {}

        TaskResult( FutureType && future, std::weak_ptr< ed::thread::CancellationSignal > && cancelSignal )
            : m_future( std::move( future ) )
            , m_cancelSignal( std::move( cancelSignal ) )
        {}

        TaskResult( const TaskResult & other ) = delete;
        TaskResult & operator=( const TaskResult & other ) = delete;

        TaskResult( TaskResult && other ) noexcept
            : m_future( std::exchange( other.m_future, {} ) )
            , m_cancelSignal( std::move( other.m_cancelSignal ) )
        {}

        TaskResult & operator=( TaskResult && other ) noexcept
        {
            m_future = std::exchange( other.m_future, {} );
            m_cancelSignal = std::move( other.m_cancelSignal );
            return *this;
        }

        virtual ~TaskResult()
        {
            // Tasks are usually held in some vector, which upon resize will call the destructor of TaskResult.
            //StopTask();
        }

        bool IsValid() const
        {
            return m_future.valid();
        }

        bool IsReady() const
        {
            return m_future.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready;
        }

        void Wait() const
        {
            m_future.wait();
        }

        T GetResult()
        {
            BOOST_ASSERT( IsValid() );
            return m_future.get();
        }

        /// @brief Broadcast cancellation signal to the task. ITask will be destroyed and the task coroutine will exit.
        /// @return Returns true if signal exists and cancellation was emitted
        inline bool StopTask()
        {
            auto signal = m_cancelSignal.lock();
            if ( !signal )
                return false;

            BOOST_ASSERT_MSG( !signal->SignalCalled(), "Cancellation signal was already emitted, cannot emit again" );
            signal->Emit();
            return true;
        }

    private:
        std::future< T > m_future;
        std::weak_ptr < ed::thread::CancellationSignal > m_cancelSignal;
    };
};
