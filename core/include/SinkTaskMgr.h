#ifndef __SINKTASKMGR_H
#define __SINKTASKMGR_H

#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#include <list>
#include <vector>


#include "PoolThreadMgr.h"


template<class Result>
class SinkTaskMgr
{
  typedef std::vector<Result> FrameResultList;
public:
  enum ErrorCode {OK,NO_MORE_AVAILABLE,TIMEDOUT};
  enum RUN_MODE {Counter,Monitor};

  explicit SinkTaskMgr(int historySize = 4);
  virtual ~SinkTaskMgr();
  
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

private:
  bool _isFrameAvailable(int frameNumber) const;

  int				_currentFrameNumber;
  mutable pthread_mutex_t	_lock;
  mutable pthread_cond_t	_cond;
  FrameResultList		_historyResult;
  RUN_MODE			_mode;
};

template<class Result>
inline bool _history_sort(const Result &A,const Result &B)
{return A.frameNumber < B.frameNumber;}

//template code
template <class Result>
SinkTaskMgr<Result>::SinkTaskMgr(int historySize) :
  _currentFrameNumber(0),
  _mode(SinkTaskMgr::Counter)
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
void SinkTaskMgr<Result>::setMode(SinkTaskMgr<Result>::RUN_MODE aMode)
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
      PoolThreadMgr::Lock aLock(&_lock);
      while(!_isFrameAvailable(frameNumber) && retcode != ETIMEDOUT)
	retcode = pthread_cond_timedwait(&_cond,&_lock,&timeout);
      if(retcode == ETIMEDOUT)
	return Result(SinkTaskMgr<Result>::TIMEDOUT);
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
  PoolThreadMgr::Lock aLock(&_lock);
  typename FrameResultList::const_iterator i;
  for(i = _historyResult.begin();
      i != _historyResult.end();++i)
    {
      if(i->frameNumber >= fromFrameNumber)
	anHistory.push_back(*i);
    }
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
      return _historyResult[aResultPos].frameNumber >= 0;
    }
}
#endif
