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
#include <iostream>
#ifdef __unix
#include <sys/time.h>
#endif
#include <errno.h>
#include "processlib/PoolThreadMgr.h"
#include "processlib/TaskMgr.h"
#include "processlib/ProcessExceptions.h"

//Static variable
static const int NB_DEFAULT_THREADS = 2;

static PoolThreadMgr *_processMgrPt = NULL;
static pthread_once_t _init = PTHREAD_ONCE_INIT;
static void _processMgrInit()
{
  _processMgrPt = new PoolThreadMgr();
}

PoolThreadMgr::PoolThreadMgr()
{
  pthread_mutex_init(&_lock,NULL);
  pthread_cond_init(&_cond,NULL);

  _stopFlag = false;
  _suspendFlag = false;
  _runningThread = 0;
  _taskMgr = NULL;
  _threadWaitOnQuit = true;
  _createProcessThread(NB_DEFAULT_THREADS);
}

PoolThreadMgr::~PoolThreadMgr()
{
#ifdef __unix
  if (_threadWaitOnQuit)
    quit();
#endif
  pthread_mutex_destroy(&_lock);
  pthread_cond_destroy(&_cond);
}

/** @brief enable/disable waiting for threads to finish on quit (Unix).
 *  This is necessary when the process is forked, and default threads do not
 *  exist on the child process
 *  @param wait_on_quit if false do not wait for threads on quit (default true)
*/
void PoolThreadMgr::setThreadWaitOnQuit(bool wait_on_quit)
{
  _threadWaitOnQuit = wait_on_quit;
}

/** @brief add a process in the process queue.
 *  Notice that after calling this methode aProcess will be own by PoolThreadMgr,
 *  do not modify or even delete it.
 *  @param aProcess a background process
 *  @param aFlag if true lock the queue which is the default
*/
void PoolThreadMgr::addProcess(TaskMgr *aProcess,bool aFlag) 
{
  Lock aLock(&_lock,aFlag);
  _processQueue.insert(QueueType::value_type(aProcess->priority(),aProcess));
  aProcess->_pool = this;
  pthread_cond_broadcast(&_cond);
}

void PoolThreadMgr::removeProcess(TaskMgr *aProcess,bool aFlag)
{
  Lock aLock(&_lock,aFlag);
  for(QueueType::iterator i = _processQueue.begin();
      i != _processQueue.end();++i)
    {
      if(i->second == aProcess)
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
void PoolThreadMgr::setNumberOfThread(int nbThread) 
{
  if(int(_threadID.size()) <= nbThread)
    _createProcessThread(nbThread - _threadID.size());
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
  if(aMgr)
    refBackgrounMgr = new TaskMgr(*aMgr);
  Lock aLock(&_lock);
  delete _taskMgr;
  _taskMgr = refBackgrounMgr;
}

/** @brief clean quit
 * @warning this methode is not MT-Safe
 * @see setNumberOfThread
 */

void PoolThreadMgr::quit()
{
  Lock aLock(&_lock);
  _stopFlag = true;
  pthread_cond_broadcast(&_cond);
  aLock.unLock();

  for(std::vector<pthread_t>::iterator i = _threadID.begin();
      i != _threadID.end();++i)
    {
      void *returnThread;
      while(pthread_join(*i,&returnThread));
    }
  _stopFlag = false;
  _threadID.clear();
}
/** @brief abort all process
 */
void PoolThreadMgr::abort()
{
  Lock aLock(&_lock);
  _suspendFlag = true;
  while(_runningThread) pthread_cond_wait(&_cond,&_lock);
  for(QueueType::iterator i = _processQueue.begin();
      i != _processQueue.end();++i)
    delete i->second;
  _processQueue.clear();
  _suspendFlag = false;
}

/** @brief suspend all process
 */
void PoolThreadMgr::suspend(bool aFlag)
{
  Lock aLock(&_lock);
  _suspendFlag = aFlag;
  if(!aFlag)
    pthread_cond_broadcast(&_cond);
}
/** @brief wait until queue is empty
 */
bool PoolThreadMgr::wait(double askedTimeout)
{
  if(askedTimeout >= 0.)
    {
      struct timeval now;
      struct timespec timeout;
      int retcode = 0;
      gettimeofday(&now,NULL);
      timeout.tv_sec = now.tv_sec + long(askedTimeout);
      timeout.tv_nsec = (now.tv_usec * 1000) + 
	long((askedTimeout - long(askedTimeout)) * 1e9);
      if(timeout.tv_nsec >= 1000000000L) // Carry
	++timeout.tv_sec,timeout.tv_nsec -= 1000000000L;
      Lock aLock(&_lock);
      while(_runningThread && !retcode)
	retcode = pthread_cond_timedwait(&_cond,&_lock,&timeout);
      return retcode != ETIMEDOUT;
    }
  else
    {
      Lock aLock(&_lock);
      while(_runningThread)
	pthread_cond_wait(&_cond,&_lock);
      return true;
    }
}
void* PoolThreadMgr::_run(void *arg) 
{
  PoolThreadMgr* processMgrPt = (PoolThreadMgr*)arg;
  Lock aLock(&processMgrPt->_lock);
  processMgrPt->_runningThread++;
  while(1)
    {
      bool aBroadcastFlag = true;
      processMgrPt->_runningThread--;

      while(processMgrPt->_suspendFlag ||
	    (!processMgrPt->_stopFlag && processMgrPt->_processQueue.empty()))
	{
	  if(aBroadcastFlag)
	    {
	      pthread_cond_broadcast(&processMgrPt->_cond);	// synchro if abort
	      aBroadcastFlag = false;
	    }
	  pthread_cond_wait(&processMgrPt->_cond,&processMgrPt->_lock);
	}      
      processMgrPt->_runningThread++;

      if(!processMgrPt->_processQueue.empty())
	{
	  QueueType::iterator i = processMgrPt->_processQueue.begin();
	  TaskMgr *processPt = i->second;
	  TaskMgr::TaskWrap *aNextTask = processPt->next();
	  aLock.unLock();
	  try
	    {
	      aNextTask->process();
	    }
	  catch(ProcessException &exp)
	    {
	      aNextTask->error(exp.getErrMsg());
	    }
	  catch(...)
	    {
	      aNextTask->error("Unknowed exception!");
	    }
	  aLock.lock();
	  delete aNextTask;
	}
      else break;		// stop
    }
  processMgrPt->_runningThread--;
  return NULL;
}

void PoolThreadMgr::_createProcessThread(int aNumber)
{
  for(int i = aNumber;i;--i)
    {
      pthread_t  threadID;
      if(!pthread_create(&threadID,NULL,_run,this))
	_threadID.push_back(threadID);
    }
}

PoolThreadMgr& PoolThreadMgr::get() throw()
{
#ifdef WIN32
  _pthread_once_raw(&_init,_processMgrInit);
#else
  pthread_once(&_init,_processMgrInit);
#endif
  return *_processMgrPt;
}

