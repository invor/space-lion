#ifndef TaskScheduler_hpp
#define TaskScheduler_hpp

#include <atomic>
#include <condition_variable>
#include <functional>

#include "MTQueue.hpp"

namespace EngineCore
{
    namespace Utility
    {

        typedef std::function<void()> Task;

        class TaskScheduler
        {
        private:
            std::atomic_flag         task_schedueler_active_ = ATOMIC_FLAG_INIT; //TODO replace
            std::vector<std::thread> worker_thread_pool_;

            /** Underlying standard queue. */
            std::queue<Task>         queue_;
            /** Mutex to protect task queue operations. */
            mutable std::mutex       mutex_;
            /** Condition variable to wait while busy or for new task*/
            std::condition_variable  cvar_;

            /** Atomically keep track of threads currently busy processing a task */
            std::atomic_int          busy_threads_cnt_;
            /** Atomically keep track of tasks currently still in queue */
            std::atomic_int          tasks_cnt_;

        public:
            void run(int worker_thread_cnt);

            void stop();

            void submitTask(Task new_task);

            bool empty() const;

            void waitWhileBusy();
        };
    }
}

#endif // !TaskScheduler_hpp

