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
#ifndef __SINKTASKMGR_H
#define __SINKTASKMGR_H

#include <pthread.h>
#ifdef __unix
#include <sys/time.h>
#endif
#include <time.h>
#include <errno.h>

#include <list>
#include <vector>


#include "processlib/PoolThreadMgr.h"


template<class Result>
class DLL_EXPORT SinkTaskMgr
{
  typedef std::vector<Result> FrameResultList;
public:
  enum ErrorCode {OK,NOT_MANAGED,NO_MORE_AVAILABLE,TIMEDOUT};
  enum RUN_MODE {Counter,Monitor};

  explicit SinkTaskMgr(int historySize = 4);
  
  void	setMode(RUN_MODE);

  Result getResult(double timeout = 0.,
		   int frameNumber = -1) const;
  void	getHistory(std::list<Result> &aHistory,int fromFrameNumber = 0) const;
  void	resizeHistory(int aSize);
  void  resetHistory(bool alockFlag = true);
  int	historySize() const;
  //@brief return the last available frame with no hole before
  int lastFrameNumber() const;
  //@brief methode called by the voidtask
  void setResult(const Result&);

  void ref();
  void unref();

protected:
  virtual ~SinkTaskMgr();

private:
  bool _isFrameAvailable(int frameNumber) const;

  volatile int			_currentFrameNumber;
  mutable pthread_mutex_t	_lock;
  mutable pthread_cond_t	_cond;
  FrameResultList		_historyResult;
  RUN_MODE			_mode;
  int				_refCounter;
};

#if (defined __unix || defined LIBPROCESSLIB_EXPORTS)
#include "SinkTaskMgr.i"
#endif

#endif
