#pragma once
#include<list>
#include<thread>
#include<functional>
#include<memory>
#include <atomic>
#include "SyncQueue.hpp"
#include "config_mgr.h"
#include <assistx2/database.h>
#include <assistx2/time_tracer.h>

class DBThreadPool
{
    const int MaxTaskCount = 10000000;
public:
    using Task = std::function<void(const std::shared_ptr<Database> db)>;
    DBThreadPool(int numThreads = std::thread::hardware_concurrency()) : m_queue(MaxTaskCount)
    {
        Start(numThreads);
    }

    ~DBThreadPool(void)
    {
        //如果没有停止时则主动停止线程池
        Stop();
    }

    void Stop()
    {
        std::call_once(m_flag, [this]{StopThreadGroup(); }); //保证多线程情况下只调用一次StopThreadGroup
    }

    void AddTask(Task&&task)
    {
       // LOG_IF(INFO, m_queue.Count() >= 1000) << "AddTask m_queue size:=" << m_queue.Count();

        assistx2::TimeTracer tracerline(new assistx2::SimpleDumpHelper(("AddTask"), assistx2::TimeTracer::MILLISECOND));
        m_queue.Put(std::forward<Task>(task));
    }

    void AddTask(const Task& task)
    {
        //LOG_IF(INFO, m_queue.Count() >= 1000) << "AddTask m_queue size:=" << m_queue.Count();

        assistx2::TimeTracer tracerline(new assistx2::SimpleDumpHelper(("AddTask"), assistx2::TimeTracer::MILLISECOND));
        m_queue.Put(task);
    }

private:
    void Start(int numThreads)
    {
        m_running = true;
        //创建线程组
        for (int i = 0; i < numThreads; ++i)
        {
            auto database_client = std::make_shared<Database>();
            auto dbcfg = ConfigMgr::getInstance()->getDBConfig();
            CHECK(database_client->Connect(dbcfg) == true);
            m_threadgroup.push_back(std::make_shared<std::thread>(&DBThreadPool::RunInThread, this, database_client));
        }
    }    

    void RunInThread(const std::shared_ptr<Database> db)
    {
        while (m_running)
        {
            std::deque<Task> list;
            m_queue.Take(list);

            while (!list.empty())
            {
                if (!m_running)
                    return;

                auto task = list.front();
                list.pop_front();

                task(db);
            }
        }
    }

    void StopThreadGroup()
    {
        m_queue.Stop(); //让同步队列中的线程停止
        m_running = false; //置为false，让内部线程跳出循环并退出

        for (auto thread : m_threadgroup) //等待线程结束
        {
            if (thread)
                thread->join();
        }
        m_threadgroup.clear();
    }

    std::list<std::shared_ptr<std::thread>> m_threadgroup; //处理任务的线程组
    SyncQueue<Task> m_queue; //同步队列     
    atomic_bool m_running; //是否停止的标志
    std::once_flag m_flag;
};
