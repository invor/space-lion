#ifndef TaskSchedueler_hpp
#define TaskSchedueler_hpp

#include <atomic>
#include <condition_variable>
#include <functional>

#include "MTQueue.hpp"

namespace EngineCore
{
    namespace Utility
    {

        typedef std::function<void()> Task;

        class TaskSchedueler
        {
        private:
            std::atomic_flag               m_taskScheduelerActive = ATOMIC_FLAG_INIT; //TODO replace
            MTQueue<std::function<void()>> m_task_queue;
            std::vector<std::thread>       m_worker_thread_pool;
            
            std::mutex                     m_busy_mutex;
            std::condition_variable        m_busy_cv;
            std::atomic_int                m_busy_threads_cnt;

        public:
            void run(int worker_thread_cnt);

            void stop();

            void submitTask(Task new_task);

            void waitWhileBusy();
        };
    }
}

#endif // !TaskSchedueler_hpp

