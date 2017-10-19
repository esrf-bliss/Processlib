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
#ifndef __POOLTHREAD_H__
#define __POOLTHREAD_H__

#ifndef __unix
#pragma warning(disable:4251)
#endif

#include <map>
#include <vector>
#include <pthread.h>

#include "processlib/Compatibility.h"

class TaskMgr;
struct Data;

class DLL_EXPORT PoolThreadMgr
{
public:
  typedef std::pair<int,int> TaskPriority;

  PoolThreadMgr();
  ~PoolThreadMgr();
  static PoolThreadMgr& get() throw();

  void addProcess(TaskMgr *aProcess,bool lock = true);
  void removeProcess(TaskMgr *aProcess,bool lock = true);
  void setNumberOfThread(int);
  void setTaskMgr(const TaskMgr *);
  void abort();
  void suspend(bool);
  bool wait(double timeout = -1.);
  void quit();

  class Lock
  {
  public:
    Lock(pthread_mutex_t *aLock,bool aLockFlag = true) :
      _lock(aLock),_lockFlag(false)
    {if(aLockFlag) lock();}

    ~Lock() {unLock();}
    inline void lock() 
    {
      if(!_lockFlag)
	while(pthread_mutex_lock(_lock)) ;
      _lockFlag = true;
    }
    inline void unLock()
    {
      if(_lockFlag)
	{
	  _lockFlag = false;
	  pthread_mutex_unlock(_lock);
	}
    }
  private:
    pthread_mutex_t *_lock;
    bool   _lockFlag;
  };

private:
  struct cmp
  {
    bool operator()(const TaskPriority& a,
		    const TaskPriority& b)
    {
      if(a.first == b.first)
	return b.second < a.second;
      else
	return b.first < a.first;
    }
  };
  typedef std::multimap<TaskPriority,TaskMgr*,cmp> QueueType;

  pthread_mutex_t                    _lock;
  pthread_cond_t                     _cond;
  volatile bool                      _stopFlag;
  volatile bool			     _suspendFlag;
  volatile int			     _runningThread;
  QueueType                          _processQueue;
  std::vector<pthread_t>             _threadID;
  TaskMgr		             *_taskMgr;

  static void* _run(void*);
  void _createProcessThread(int aNumber);

#ifdef __unix
  static void atfork_prepare();
  static void atfork_parent();
  static void atfork_child();
#endif
};

#endif
