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
#ifndef __SINKTASKMGR_I
#define __SINKTASKMGR_I

template<class Result>
inline bool _history_sort(const Result &A,const Result &B)
{return A.frameNumber < B.frameNumber;}

//template code
template <class Result>
SinkTaskMgr<Result>::SinkTaskMgr(int historySize) :
  _currentFrameNumber(0),
  _mode(SinkTaskMgr::Counter),
  _refCounter(1)
{
  pthread_mutex_init(&_lock,NULL);
  pthread_cond_init(&_cond,NULL);
  _historyResult.resize(historySize);
  resetHistory();
}
template<class Result>
SinkTaskMgr<Result>::~SinkTaskMgr()
{
  pthread_mutex_destroy(&_lock);
  pthread_cond_destroy(&_cond);
}

template<class Result>
void SinkTaskMgr<Result>::setMode(typename SinkTaskMgr<Result>::RUN_MODE aMode)
{
  PoolThreadMgr::Lock aLock(&_lock);
  _mode = aMode;
}

/** @brief get the Result class of the  calculation.
    @param timeout the maximum wait to get the result, timeout is in second
    @param frameNumber the frame id you want or last frame if < 0
*/
template<class Result>
Result SinkTaskMgr<Result>::getResult(double askedTimeout,
				      int frameNumber) const
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

      PoolThreadMgr::Lock aLock(&_lock);
      while(!_isFrameAvailable(frameNumber))
	{
	  retcode = pthread_cond_timedwait(&_cond,&_lock,&timeout);
          if(retcode == ETIMEDOUT)
	    return Result(SinkTaskMgr<Result>::TIMEDOUT);
	}
    }
  else
    {
      PoolThreadMgr::Lock aLock(&_lock);
      while(!_isFrameAvailable(frameNumber))
	pthread_cond_wait(&_cond,&_lock);
    }

  if(frameNumber < 0)
    frameNumber = _currentFrameNumber;
  else if(frameNumber < (_currentFrameNumber - int(_historyResult.size())))
    return Result(SinkTaskMgr<Result>::NO_MORE_AVAILABLE);

  // still in history
  int aResultPos = frameNumber % _historyResult.size();
  return _historyResult[aResultPos];
}
template<class Result>
void SinkTaskMgr<Result>::setResult(const Result &aResult)
{
  PoolThreadMgr::Lock aLock(&_lock);
  if(aResult.frameNumber > _currentFrameNumber)
    _currentFrameNumber = aResult.frameNumber;
  int aResultPos = aResult.frameNumber % _historyResult.size();
  if(aResultPos < 0)
    {
      _historyResult.front() = aResult;
      _historyResult.front().frameNumber = 0;
    }
  else
    {
      _historyResult[aResultPos] = aResult;
    }
  pthread_cond_broadcast(&_cond);
}

template<class Result>
void SinkTaskMgr<Result>::resetHistory(bool lockFlag)
{
  PoolThreadMgr::Lock aLock(&_lock,lockFlag);
  typename std::vector<Result>::iterator i;
  for(i = _historyResult.begin();i != _historyResult.end();++i)
    i->frameNumber = -1;
  _currentFrameNumber = 0;
}

/** @brief return the Result history
 */
template<class Result>
void SinkTaskMgr<Result>::getHistory(std::list<Result> & anHistory,int fromFrameNumber) const
{
  bool sort_needed = false;
  PoolThreadMgr::Lock aLock(&_lock);
  if(fromFrameNumber > _currentFrameNumber) return; // not yet available
  else if (fromFrameNumber < 0) fromFrameNumber = 0;

  int current_index = _currentFrameNumber % _historyResult.size();
  int from_index = fromFrameNumber % _historyResult.size();
  if(_currentFrameNumber - fromFrameNumber >= _historyResult.size()) // return all history
    {
      from_index = 0;
      current_index = _historyResult.size() - 1;
      sort_needed = true;
    }
  typename FrameResultList::const_iterator i;
  typename FrameResultList::const_iterator end;
  if(from_index <= current_index)
    {
      i = _historyResult.begin() + from_index;
      end = _historyResult.begin() + current_index + 1;
      for(;i != end;++i)
	{
	  if(i->frameNumber >= fromFrameNumber)
	    anHistory.push_back(*i);
	  else if(!sort_needed)
	    break;
	}
    }
  else
    {
      bool continue_flag = true;
      i = _historyResult.begin() + from_index;
      end = _historyResult.end();
      for(;i != end;++i)
	{
	  if(i->frameNumber >= fromFrameNumber)
	    anHistory.push_back(*i);
	  else if(!sort_needed)
	    {
	      continue_flag = false;
	      break;
	    }
	}
      if(continue_flag)
	{
	  i = _historyResult.begin();
	  end = _historyResult.begin() + current_index + 1;
	  for(;i != end;++i)
	    {
	      if(i->frameNumber >= fromFrameNumber)
		anHistory.push_back(*i);
	      else if(!sort_needed)
		break;
	    }
	}
    }
  aLock.unLock();
  if(sort_needed)
    anHistory.sort(_history_sort<Result>);
}

/** @brief set the number of result keeped in history.
 *  It resize the result history and clear it
 */
template<class Result>
void SinkTaskMgr<Result>::resizeHistory(int aSize)
{
  if(aSize < 1) aSize = 1;
  PoolThreadMgr::Lock aLock(&_lock);
  _historyResult.resize(aSize);
  resetHistory(false);
}

template<class Result>
int SinkTaskMgr<Result>::historySize() const
{
  PoolThreadMgr::Lock aLock(&_lock);
  return _historyResult.size();
}

template<class Result>
int SinkTaskMgr<Result>::lastFrameNumber() const
{
  int aLastFrameNumber = -1;
  std::list<Result> anHistory;
  this->getHistory(anHistory);
  if(!anHistory.empty())
    {
      aLastFrameNumber = anHistory.front().frameNumber;
      anHistory.pop_front();
      typename std::list<Result>::iterator i;
      for(i = anHistory.begin();i != anHistory.end();++i)
	{
	  if(i->frameNumber > aLastFrameNumber + 1)
	    aLastFrameNumber = i->frameNumber;
	  else
	    break;
	}
    }
  return aLastFrameNumber;
}

/** @brief check if the frame asked is available.
 *  check if at the position in the history the result 
 *  have a frame number >= 0 and if so return true
 *  @warning All this call must be under mutex lock
 *  @param frameNumber the frameNumber or < 0 if last
 */
template<class Result>
bool SinkTaskMgr<Result>::_isFrameAvailable(int aFrameNumber) const
{
  if(aFrameNumber < 0) aFrameNumber = _currentFrameNumber;

  if(aFrameNumber < (_currentFrameNumber - int(_historyResult.size()))) 
    return true;		// the frame asked is no more available
  else
    {
      int aResultPos = aFrameNumber % _historyResult.size();
      return _historyResult[aResultPos].frameNumber == aFrameNumber;
    }
}

template<class Result>
void SinkTaskMgr<Result>::ref()
{
  PoolThreadMgr::Lock aLock(&_lock);
  ++_refCounter;
}

template<class Result>
void SinkTaskMgr<Result>::unref()
{
  PoolThreadMgr::Lock aLock(&_lock);
  if(!(--_refCounter))
    {
      aLock.unLock();
      delete this;
    }
}

#endif

