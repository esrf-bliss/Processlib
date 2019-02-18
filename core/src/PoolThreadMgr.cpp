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
#include "processlib/PoolThreadMgr.h"
#include "processlib/ProcessExceptions.h"
#include "processlib/TaskMgr.h"
#include <errno.h>
#include <iostream>

// Static variable
static const int NB_DEFAULT_THREADS = 2;

static PoolThreadMgr *_processMgrPt = NULL;
std::once_flag PoolThreadMgr::init_flag;

/** @brief add a process in the process queue.
 *  Notice that after calling this methode aProcess will be own by
 * PoolThreadMgr, do not modify or even delete it.
 *  @param aProcess a background process
 *  @param aFlag if true lock the queue which is the default
 */
void PoolThreadMgr::addProcess(TaskMgr *aProcess, bool aFlag)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    _processQueue.insert(QueueType::value_type(aProcess->priority(), aProcess));
    aProcess->_pool = this;
    m_cond.notify_all();
}

void PoolThreadMgr::removeProcess(TaskMgr *aProcess, bool aFlag)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (QueueType::iterator i = _processQueue.begin(); i != _processQueue.end(); ++i)
    {
        if (i->second == aProcess)
        {
            _processQueue.erase(i);
            break;
        }
    }
}

/** @brief change the number of thread in the pool
 * @warning this methode is not MT-Safe
 * @see quit
 */
void PoolThreadMgr::setNumberOfThread(size_t nbThread)
{
    if (m_threads.size() <= nbThread)
        _createProcessThread(nbThread - m_threads.size());
    else
    {
        quit();
        _createProcessThread(nbThread);
    }
}
/** @brief set the image processing mgr
 *
 *  this is the way to defined the chained process of all images.
 *  each time an image is received, this TaskMgr is clone
 * @param aMgr set a TaskMgr or NULL to remove the previous one
 */
void PoolThreadMgr::setTaskMgr(const TaskMgr *aMgr)
{
    TaskMgr *refBackgrounMgr = NULL;
    if (aMgr)
        refBackgrounMgr = new TaskMgr(*aMgr);
    std::lock_guard<std::mutex> lock(m_mutex);
    delete _taskMgr;
    _taskMgr = refBackgrounMgr;
}

/** @brief clean quit
 * @warning this methode is not MT-Safe
 * @see setNumberOfThread
 */

void PoolThreadMgr::quit()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    _stopFlag = true;
    m_cond.notify_all();

    for (auto &&thread : m_threads)
        thread.join();

    _stopFlag = false;
    m_threads.clear();
}
/** @brief abort all process
 */
void PoolThreadMgr::abort()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    _suspendFlag = true;

    m_cond.wait(lock, [&] { return _runningThread; });

    for (QueueType::iterator i = _processQueue.begin(); i != _processQueue.end(); ++i)
        delete i->second;
    _processQueue.clear();
    _suspendFlag = false;
}

/** @brief suspend all process
 */
void PoolThreadMgr::suspend(bool aFlag)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    _suspendFlag = aFlag;
    if (!aFlag)
        m_cond.notify_all();
}
/** @brief wait until queue is empty
 */
bool PoolThreadMgr::wait(int rel_time_ms)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_cond.wait_for(lock, std::chrono::milliseconds(rel_time_ms), [&] { return _runningThread; });
}
void *PoolThreadMgr::_run(void *arg)
{
    // that is the equivalent of this (but this is a reserved keyword)
    PoolThreadMgr *that = reinterpret_cast<PoolThreadMgr *>(arg);

    std::lock_guard<std::mutex> lock(that->m_mutex);
    that->_runningThread++;
    while (1)
    {
        bool aBroadcastFlag = true;
        that->_runningThread--;

        while (that->_suspendFlag || (!that->_stopFlag && that->_processQueue.empty()))
        {
            if (aBroadcastFlag)
            {
                that->m_cond.notify_all();
                aBroadcastFlag = false;
            }

            std::unique_lock<std::mutex> lock(that->m_mutex);
            that->m_cond.wait(lock);
        }
        that->_runningThread++;

        if (!that->_processQueue.empty())
        {
            QueueType::iterator i        = that->_processQueue.begin();
            TaskMgr *processPt           = i->second;
            TaskMgr::TaskWrap *aNextTask = processPt->next();
            // aLock.unLock();

            try
            {
                aNextTask->process();
            } catch (ProcessException &exp)
            {
                aNextTask->error(exp.getErrMsg());
            } catch (...)
            {
                aNextTask->error("Unknown exception!");
            }

            // aLock.lock();
        } else
            break; // stop
    }
    that->_runningThread--;
    return NULL;
}

void PoolThreadMgr::_createProcessThread(size_t aNumber)
{
    for (size_t i = aNumber; i; --i)
        m_threads.emplace_back(_run, this);
}

PoolThreadMgr &PoolThreadMgr::get() throw()
{
    // Defere the creation of the threads to the first call of get
    std::call_once(PoolThreadMgr::init_flag, []() { _processMgrPt->_createProcessThread(NB_DEFAULT_THREADS); });
    return *_processMgrPt;
}
