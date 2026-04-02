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
#include "../ErrorCodes.h"

///STL
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

#include <latch>

namespace BongoJam {

    struct CompilationTask //using a raw ptr since this has to be trivially copyable so that bongomanager can pass a ref to the actual compilation unit and retrieve it to pass onto the linker
    {
        string FilePath;
        CompilationUnit* Output = nullptr;
    };

    template<size_t pm_ThreadCount>
    struct CompilerThreadPool
    {
        static_assert(pm_ThreadCount > 0, "Thread count must be greater than 0");

    public:
        CompilerThreadPool()
        {
            pm_Stop = false;

            for (size_t lv_ThreadNumber = 0; lv_ThreadNumber < pm_ThreadCount; ++lv_ThreadNumber)
            {
                pm_Workers[lv_ThreadNumber] = jthread(&CompilerThreadPool::Worker, this, lv_ThreadNumber);
            }

            threadpool_logger = Logger::CreateUnique("ThreadPoolLogger", PEACH_LOGGER_DEFAULT_FLAGS, PEACH_LOGGER_DEFAULT_OUTPUT_DIR);

            if (not threadpool_logger)
            {
                throw runtime_error("WTF MANG LOGGER FAILED TO INITIALIZE FROM THREADPOOL WTF MANG");
            }

            threadpool_logger->Debug("Properly Initialized CompilerThreadPool!", "ThreadPoolLogger");
        }

        ~CompilerThreadPool()
        {
            Shutdown();
        }

    public:
        atomic<bool> BONGO_COMPILE_SUCCESS = true;

    private:
        unique_ptr<Logger> threadpool_logger = nullptr;

        array<jthread, pm_ThreadCount> pm_Workers;

        queue<CompilationTask> pm_Tasks; // Main task queue categorized by priority, WARNING: this will forever grow but like there should be no use case where that memory leak matters
        //although just in case TODO: implement a memory tracking thing for this to clamp its max usage or garbage collect it o this language wll have one mebbe

        mutex pm_QueueMutex;
        condition_variable pm_Condition;

        atomic<bool> pm_Stop{ false };

        // latch to track a batch of tasks, this threadpool doesn't do continuous operation, it does everything inshort bursts since thats how compilation requests typically work where entire projects get done all at once
        unique_ptr<latch> pm_BatchLatch;
        atomic<bool> pm_BatchActive{ false };

    public:
        void 
            StartBatch(size_t fp_TaskCount)
        {
            if (fp_TaskCount == 0)
            {
                // No work; nothing to wait on
                BONGO_COMPILE_SUCCESS = true;
                pm_BatchActive = false;
                pm_BatchLatch.reset();
                return;
            }

            {
                lock_guard<mutex> lock(pm_QueueMutex);

                BONGO_COMPILE_SUCCESS = true;

                // Clear any leftover tasks just in case
                queue<CompilationTask>().swap(pm_Tasks);

                // create a fresh latch for this batch
                try
                {
                    pm_BatchLatch = make_unique<latch>(fp_TaskCount);
                    pm_BatchActive = true;
                }
                catch (const exception& fp_Exception)
                {
                    BONGO_COMPILE_SUCCESS = false;
                    pm_BatchActive = false;
                    pm_BatchLatch.reset();

                   PRINT_ERROR(fmt::format("Unhandled exception: {}", fp_Exception.what()));
                }
            }
        }

        void 
            WaitUntilAllTasksComplete()
        {
            if (pm_BatchLatch)
            {
                pm_BatchLatch->wait();
            }

            pm_BatchActive = false;
        }

        void 
            EnqueueTask(const CompilationTask& fp_Task)
        {
            {
                lock_guard<mutex> lock(pm_QueueMutex);

                if (not pm_BatchActive)
                {
                    threadpool_logger->Error("Tried to enqueue task without an active batch", "ThreadPool");
                    return;
                }

                threadpool_logger->Debug(fmt::format("Enqueueing Task with script path: {}", fp_Task.FilePath), "ThreadPool");

                pm_Tasks.push(fp_Task);
            }

            pm_Condition.notify_one();
        }

        [[nodiscard]] inline bool
            IsBatchActive()
            const noexcept
        {
            return pm_BatchActive;
        }

    private:
        void
            Shutdown()
        {
            {
                lock_guard<mutex> lock(pm_QueueMutex);
                pm_Stop = true;
                threadpool_logger->Debug("Shutting down compiler thread pool", "ThreadPoolLogger");
                pm_Condition.notify_all();
            }

            for (jthread& worker : pm_Workers)
            {
                if (worker.joinable())
                {
                    worker.join();
                }
            }
        }

        void 
            Worker(uint64_t fp_ThreadNumber)
        {
            thread_local BongoCompiler f_Compiler("CompilerThreadPool__ThreadID( " + to_string(fp_ThreadNumber) + " )");
            Logger* f_CompilerLogger = f_Compiler.compiler_logger.get();

            while(1)
            {
                CompilationTask f_Task;

                // --------- Take a task or exit ---------
                {
                    unique_lock<mutex> lock(pm_QueueMutex);
                    pm_Condition.wait
                    (
                        lock,
                        [this]
                        {
                            return pm_Stop or not pm_Tasks.empty();
                        }
                    );

                    // true shutdown path: destructor called Shutdown()
                    if (pm_Stop and pm_Tasks.empty())
                    {
                        f_CompilerLogger->Debug("Worker exiting due to stop flag", "Worker");
                        return;
                    }
                    
                    if (pm_Tasks.empty()) // Nothing to do, go back to waiting
                    {
                        continue; 
                    }

                    // if we woke up and there IS work, grab it
                    f_CompilerLogger->Debug("Worker taking compilation task using this compiler", "Worker");

                    f_Task = move(pm_Tasks.front());
                    pm_Tasks.pop();
                }

                // --------- Compile the unit ---------
                bool f_IsSuccessful = true;

                try
                {
                    int f_Result = f_Compiler.CompileUnit(f_Task.FilePath, f_Task.Output);

                    if (f_Result != BONGO_OK)
                    {
                        f_IsSuccessful = false;
                        f_CompilerLogger->Error(fmt::format("Failed to compile : '{}', with compiler exit code : {} ", f_Task.FilePath, f_Result), "Worker");
                    }
                    else
                    {
                        f_CompilerLogger->Info(fmt::format("Worker successfully compiled: '{}'!", f_Task.FilePath), "Worker");
                    }
                }
                catch (const exception& Exception)
                {
                    f_IsSuccessful = false;
                    f_CompilerLogger->Error(fmt::format("Unhandled exception: {}, while compiling : '{}' ", Exception.what(), f_Task.FilePath), "Worker");
                }

                // --------- Update global state + latch ---------
                if (not f_IsSuccessful)
                {
                    // On first failure, flip flags and drain the queue, and count down the latch for all remaining tasks.
                    {
                        lock_guard<mutex> lock(pm_QueueMutex);

                        BONGO_COMPILE_SUCCESS = false;

                        if (pm_BatchLatch)
                        {
                            // one count for *this* failed task
                            pm_BatchLatch->count_down();

                            // drain remaining tasks and count them as "done"
                            while (not pm_Tasks.empty())
                            {
                                pm_Tasks.pop();
                                pm_BatchLatch->count_down();
                            }
                        }
                    }

                    // wake other workers: they’ll see an empty queue and just wait
                    pm_Condition.notify_all();

                    // IMPORTANT: do NOT return here, We want this worker to stay alive for future batches.
                    continue;
                }
                else if (pm_BatchLatch) // Successful compile: mark this task as done
                {
                    pm_BatchLatch->count_down();
                }
                else
                {
                    f_CompilerLogger->Error(fmt::format("Invalid nullptr ref to latch threadpool cannot operate uwu, while compiling : '{}' ", f_Task.FilePath), "Worker");
                    return;
                }
            }
        }
    };
}//namespace BongoJam