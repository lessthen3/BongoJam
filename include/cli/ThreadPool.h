#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <atomic>
#include <stdexcept>

#include "Logger.h"

namespace BongoJam {

    class ThreadPool
    {
    public:
        ThreadPool() = default;

        ~ThreadPool()
        {
            Shutdown();
        }

        void Initialize(size_t threadCount = thread::hardware_concurrency())
        {
            if (m_Initialized) return;

            m_Stop = false;
            m_Initialized = true;

            for (size_t i = 0; i < threadCount; ++i)
            {
                m_Workers.emplace_back([this]() {
                    WorkerLoop();
                    });
            }
        }

        template<typename Func, typename... Args>
        auto Enqueue(Func&& f, Args&&... args)
            -> future<invoke_result_t<Func, Args...>>
        {
            using ReturnType = invoke_result_t<Func, Args...>;

            auto task = make_shared<packaged_task<ReturnType()>>(
                bind(forward<Func>(f), forward<Args>(args)...)
            );

            future<ReturnType> result = task->get_future();

            {
                unique_lock<mutex> lock(m_QueueMutex);
                if (m_Stop) throw runtime_error("ThreadPool is stopped");

                m_Tasks.emplace([task]() { (*task)(); });
            }

            m_Condition.notify_one();
            return result;
        }

        void Shutdown()
        {
            {
                unique_lock<mutex> lock(m_QueueMutex);
                m_Stop = true;
            }

            m_Condition.notify_all();

            for (thread& worker : m_Workers)
            {
                if (worker.joinable())
                    worker.join();
            }

            m_Workers.clear();
            m_Initialized = false;
        }

        bool Busy() const
        {
            unique_lock<mutex> lock(m_QueueMutex);
            return !m_Tasks.empty();
        }

    private:
        void WorkerLoop()
        {
            while (true)
            {
                function<void()> task;

                {
                    unique_lock<mutex> lock(m_QueueMutex);
                    m_Condition.wait(lock, [this] { return m_Stop || !m_Tasks.empty(); });

                    if (m_Stop && m_Tasks.empty())
                        return;

                    task = move(m_Tasks.front());
                    m_Tasks.pop();
                }

                task();
            }
        }

    private:
        vector<thread> m_Workers;
        queue<function<void()>> m_Tasks;
        mutable mutex m_QueueMutex;
        condition_variable m_Condition;
        bool m_Stop = false;
        bool m_Initialized = false;
    };

} // namespace BongoJam
