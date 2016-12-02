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
#ifndef __TASKMGR_H
#define __TASKMGR_H

#include <deque>
#include "processlib/Data.h"
#include "processlib/Compatibility.h"

class LinkTask;
class SinkTaskBase;
class PoolThreadMgr;

class DLL_EXPORT TaskMgr
{
  friend class PoolThreadMgr;
  struct Task
  {
    Task() : _linkTask(NULL) {}
    ~Task();
    Task* copy() const;
    LinkTask		       *_linkTask;
    std::deque<SinkTaskBase*>	_sinkTaskQueue;
  };
  typedef std::deque<Task*> StageTask;
  
public:
  class EventCallback
  {
  public:
    virtual ~EventCallback() {}
    virtual void error(Data&,const char*) {}
  };

  class TaskWrap
  {
    friend class TaskMgr;
  public:
    virtual ~TaskWrap(){};
    virtual void process() = 0;
    virtual void error(const std::string &errMsg) = 0;
  protected:
  TaskWrap(TaskMgr &aMgr) : _Mgr(aMgr) {};

    inline void _endLinkTask(LinkTask *aFinnishedTask) 
      {_Mgr._endLinkTask(aFinnishedTask);}
    inline void _endSinkTask(SinkTaskBase *aFinnishedTask)
      {_Mgr._endSinkTask(aFinnishedTask);}
    inline void _setNextData(Data &aNextData)
      {_Mgr._nextData = aNextData;}
    inline void _callError(Data &aData,const char *msg)
      {_Mgr._callError(aData,msg);}
    TaskMgr &_Mgr;
  };
  friend class TaskWrap;
  
  TaskMgr(int priority = 0);
  TaskMgr(const TaskMgr&);
  ~TaskMgr();

  void setInputData(Data &aData) {_currentData = aData;}
  bool setLinkTask(int aStage,LinkTask *);
  void addSinkTask(int aStage,SinkTaskBase *);
  void getLastTask(std::pair<int,LinkTask*>&,
		   std::pair<int,SinkTaskBase*>&);
  void setEventCallback(EventCallback *);
  TaskWrap* next();
  std::pair<int,int> priority() const
  {return std::pair<int,int>(_priority,_sub_priority);}
  //@brief do all the task synchronously
  Data syncProcess();
private:
  StageTask			_Tasks;
  LinkTask		       *_PendingLinkTask;
  bool				_initStageFlag;
  int				_nbPendingSinkTask;
  Data       			_currentData;
  Data       			_nextData;
  EventCallback*		_eventCBK;
  int				_priority;
  int				_sub_priority;
  PoolThreadMgr*		_pool;

  void _endLinkTask(LinkTask *aFinnishedTask);
  void _endSinkTask(SinkTaskBase *aFinnishedTask);
  void _goToNextStage();
  void _callError(Data&,const char*);
};

#endif // __TASKMGR_H
