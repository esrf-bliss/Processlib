//###########################################################################
// This file is part of ProcessLib, a submodule of LImA project the
// Library for Image Acquisition
//
// Copyright (C) : 2009-2011
// European Synchrotron Radiation Facility
// BP 220, Grenoble 38043
// FRANCE
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//###########################################################################

#pragma once

#if !defined(PROCESSLIB_POOLTHREAD_H)
#define PROCESSLIB_POOLTHREAD_H

#ifndef __unix
#pragma warning(disable : 4251)
#endif

#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <processlib_export.h>

class TaskMgr;
struct Data;

class PROCESSLIB_EXPORT PoolThreadMgr
{
  public:
    typedef std::pair<int, int> TaskPriority;

    static PoolThreadMgr &get();

    void addProcess(TaskMgr *aProcess, bool lock = true);
    void removeProcess(TaskMgr *aProcess, bool lock = true);
    void setNumberOfThread(size_t nbThread);
    void setTaskMgr(const TaskMgr *);
    void abort();
    void suspend(bool);
    /// Wait until queue is empty or exit after timeout specified with rel_time_ms
    /// (ms)
    bool wait(int rel_time_ms);
    void quit();

  private:
    // Thread safe Singleton (guaranted in C++11)
    PoolThreadMgr() : _stopFlag(false), _suspendFlag(false), _runningThread(0), _taskMgr(NULL) {}
    ~PoolThreadMgr()= default;
    PoolThreadMgr(const PoolThreadMgr&)= delete;
    PoolThreadMgr& operator=(const PoolThreadMgr&)= delete;

    struct cmp
    {
        bool operator()(const TaskPriority &a, const TaskPriority &b)
        {
            if (a.first == b.first)
                return b.second < a.second;
            else
                return b.first < a.first;
        }
    };
    typedef std::multimap<TaskPriority, TaskMgr *, cmp> QueueType;

    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::vector<std::thread> m_threads;
    static std::once_flag init_flag;

    volatile bool _stopFlag;
    volatile bool _suspendFlag;
    volatile int _runningThread;
    QueueType _processQueue;
    TaskMgr *_taskMgr;

    static void *_run(void *);
    void _createProcessThread(size_t aNumber);
};

#endif //! defined(PROCESSLIB_POOLTHREAD_H)
