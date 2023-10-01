#include "TaskSchedueler.hpp"

#include <iostream>

void EngineCore::Utility::TaskSchedueler::run(int worker_thread_cnt)
{
    worker_thread_pool_.resize(worker_thread_cnt);
    task_schedueler_active_.test_and_set();
    busy_threads_cnt_ = 0;
    tasks_cnt_ = 0;

    for (int i = 0; i < worker_thread_cnt; ++i)
    {
        worker_thread_pool_[i] = std::thread([this]() {
            while (task_schedueler_active_.test())
            {
                std::function<void()> task;
                bool success = false;

                {
                    // atomically try to pop task from queue and increment busy if successful
                    std::unique_lock<std::mutex> lock(mutex_);
                    cvar_.wait(lock, [this] { return (tasks_cnt_.load() > 0); });
                    task = std::move(queue_.front());
                    queue_.pop();
                    --tasks_cnt_;
                    ++busy_threads_cnt_;
                    success = true;
                }

                if (success)
                {
                    task();
                    --busy_threads_cnt_;
                }

                cvar_.notify_all();
            }
            });
    }
}

void EngineCore::Utility::TaskSchedueler::stop()
{
    task_schedueler_active_.clear();

    for (auto& thread : worker_thread_pool_)
        thread.join();
}

void EngineCore::Utility::TaskSchedueler::submitTask(Task new_task)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(new_task);
        ++tasks_cnt_;
    }
    cvar_.notify_all();
}

bool EngineCore::Utility::TaskSchedueler::empty() const {
    return (tasks_cnt_.load() == 0);
}

void EngineCore::Utility::TaskSchedueler::waitWhileBusy()
{
    // pessimistic wait while busy that checks every 0.01ms if acutally still busy,
    // because it sometimes somehow misses the notifications from the worker threads
    std::unique_lock<std::mutex> lock(mutex_);
    while (!cvar_.wait_for(lock, std::chrono::microseconds(10), [this] { return (tasks_cnt_.load() == 0) && (busy_threads_cnt_.load() == 0); }));
}
