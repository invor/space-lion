#include "TaskSchedueler.hpp"

#include <iostream>

void EngineCore::Utility::TaskSchedueler::run(int worker_thread_cnt)
{
    m_worker_thread_pool.resize(worker_thread_cnt);
    m_taskScheduelerActive.test_and_set();
    m_busy_threads_cnt = 0;

    for (int i = 0; i < worker_thread_cnt; ++i)
    {
        m_worker_thread_pool[i] = std::thread([this]() {
            while (m_taskScheduelerActive.test())
            {
                std::function<void()> f;
                if (m_task_queue.tryPop(f, std::chrono::microseconds(10))) {
                    ++m_busy_threads_cnt;
                    f();
                    --m_busy_threads_cnt;
                    m_busy_cv.notify_all();
                }
            }
        });
    }
}

void EngineCore::Utility::TaskSchedueler::stop()
{
    m_taskScheduelerActive.clear();

    for (auto& thread : m_worker_thread_pool)
        thread.join();
}

void EngineCore::Utility::TaskSchedueler::submitTask(Task new_task)
{
    m_task_queue.push(new_task);
}

void EngineCore::Utility::TaskSchedueler::waitWhileBusy()
{
    std::unique_lock<std::mutex> lock(m_busy_mutex);
    m_busy_cv.wait(lock, [this] { return m_task_queue.empty() && (m_busy_threads_cnt.load() == 0); });
}
