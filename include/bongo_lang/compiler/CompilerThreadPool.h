/**************************************************************************
 *                         BongoJam Script v0.0.1
 *                  Created by Ranyodh Mandur - 🗻2024
 *
 *                 Licensed under the MIT License (MIT).
 *            For more details, see the LICENSE file or visit:
 *                 https://opensource.org/licenses/MIT
 *
 * BongoJam is an open-source scripting language compiler and interpreter
 *        primarily intended for embedding within game engines.
**************************************************************************/
#pragma once

///BongoJam
#include "Compiler.h"

///STL
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace BongoJam {

    struct CompilationTask
    {
        string FilePath;
        CompilationUnit* Output;
    };

    template<size_t pm_ThreadCount>
    struct CompilerThreadPool
    {
        static_assert(pm_ThreadCount > 0, "Thread count must be greater than 0");

    public:
        CompilerThreadPool()
        {
            m_Stop = false;

            for (size_t i = 0; i < pm_ThreadCount; ++i)
            {
                m_Workers[i] = thread(&CompilerThreadPool::Worker, this);
            }

            threadpool_logger = make_unique<Logger>();
            threadpool_logger->Initialize(DEFAULT_LOG_OUTPUT_DIRECTORY, "ThreadPoolLogger", DEFAULT_LOG_LEVEL_FILTER);
            threadpool_logger->Debug("Properly Initialized CompilerThreadPool!", "ThreadPoolLogger");
        }

        ~CompilerThreadPool()
        {
            Shutdown();
        }
    public:

    private:
        unique_ptr<Logger> threadpool_logger = nullptr;

        array<thread, pm_ThreadCount> m_Workers;

        queue<CompilationTask> m_Tasks; // Main task queue categorized by priority, WARNING: this will forever grow but like there should be no use case where that memory leak matters
        //although just in case TODO: implement a memory tracking thing for this to clamp its max usage or garbage collect it o this language wll have one mebbe

        mutex m_QueueMutex;

        condition_variable m_Condition;
        condition_variable m_CompletionCondition;

        atomic<bool> m_Stop{ false };

        atomic<size_t> m_IdleThreadCount{ 0 };

    public:

        void
            WaitUntilAllTasksComplete()
        {
            unique_lock<mutex> lock(m_QueueMutex);

            m_CompletionCondition.wait
            (
                lock,
                [this]()
                {
                    return m_Tasks.empty();
                }
            );
        }


        void
            EnqueueTask(const CompilationTask& fp_Task)
        {
            {
                lock_guard<mutex> lock(m_QueueMutex);
                threadpool_logger->Debug(format("Enqueueing Task with script path: {}", fp_Task.FilePath), "ThreadPoolLogger");
                m_Tasks.push(fp_Task);
            }

            m_Condition.notify_one();
        }

        void
            Shutdown()
        {
            {
                lock_guard<mutex> lock(m_QueueMutex);
                m_Stop = true;
                threadpool_logger->Debug("Shutting down compiler thread pool", "ThreadPoolLogger");
                m_Condition.notify_all();
            }

            for (thread& worker : m_Workers)
            {
                if (worker.joinable())
                {
                    worker.join();
                }
            }
        }

    private:
        void
            Worker() //maybe have a worker ID idk for tracking might as well w the logger name right
        {
            thread_local BongoCompiler f_Compiler;
            Logger* f_CompilerLogger = f_Compiler.compiler_logger.get();

            while (true)
            {
                CompilationTask f_Task;

                {
                    unique_lock<mutex> lock(m_QueueMutex);
                    m_Condition.wait
                    (
                        lock,
                        [this]
                        {
                            return m_Stop or not AreTasksEmpty();
                        }
                    );

                    if (m_Stop and AreTasksEmpty())
                    {
                        return;
                    }

                    f_CompilerLogger->Debug("Worker taking compilation task using this compiler", "Worker");
                    f_Task = move(m_Tasks.front());
                    m_Tasks.pop();
                }

                f_CompilerLogger->Debug("Worker compiling unit with this compiler you scoundrel!", "Worker");
                
                try
                {
                    int result = f_Compiler.CompileUnit(f_Task.FilePath, f_Task.Output);

                    if (result != BONGO_OK)
                    {
                        f_CompilerLogger->Error(format("Failed to compile : '{}', with compiler exit code : '{}' ", f_Task.FilePath, result), "Worker");
                    }
                    else
                    {
                        f_CompilerLogger->Info(format("Worker successfully compiled: '{}'!", f_Task.FilePath), "Worker");
                    }
                }
                catch (const exception& Exception) ///Try to ensure all destructors are called especially close() on LogManager
                {
                    f_CompilerLogger->Error(format("Unhandled exception: {}, while compiling : '{}' " , Exception.what(), f_Task.FilePath), "Worker");
                }

                {
                    lock_guard<mutex> lock(m_QueueMutex);

                    if (m_Tasks.empty())
                    {
                        m_CompletionCondition.notify_all();
                    }
                }
            }
        }

        bool
            AreTasksEmpty()
            const
        {
            return m_Tasks.empty();
        }
    };

}//namespace BongoJam