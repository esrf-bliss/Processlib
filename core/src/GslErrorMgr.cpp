#include <gsl/gsl_errno.h>
#include "GslErrorMgr.h"
#include "PoolThreadMgr.h"

//Static variable
GslErrorMgr GslErrorMgr::_errorMgr;
static pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;
static const int MAX_ERROR_SIZE = 1024;

/** @brief just set the gsl handler
 */
GslErrorMgr::GslErrorMgr()
{
  gsl_set_error_handler(&GslErrorMgr::_error_handler);
}
/** @brief return the last thread error message or empty string ("")
 */
const char* GslErrorMgr::lastErrorMsg() const
{
  PoolThreadMgr::Lock aLock(&_lock);
  std::map<pthread_t,std::string>::const_iterator i = _errorMessage.find(pthread_self());
  return (i != _errorMessage.end()) ? i->second.c_str() : "";
}
/** @brief return the last errno
 */
int GslErrorMgr::lastErrno() const
{
  PoolThreadMgr::Lock aLock(&_lock);
  std::map<pthread_t,int>::const_iterator i = _lastGslErrno.find(pthread_self());
  return (i != _lastGslErrno.end()) ? i->second : 0;
}
/** @brief reset the error gsl string for this thread
 */
void GslErrorMgr::resetErrorMsg()
{
  PoolThreadMgr::Lock aLock(&_lock);
  _errorMessage.insert(ErrorMessageType::value_type(pthread_self(),""));
  _lastGslErrno.insert(ErrnoType::value_type(pthread_self(),0));
}

/** @brief store gsl error into the thread's error message
 */
void GslErrorMgr::_error_handler(const char *reason,
				 const char *file,
				 int line,int gsl_errno)
{
  char aTmpBuffer[MAX_ERROR_SIZE];
  snprintf(aTmpBuffer,MAX_ERROR_SIZE,"GSL call failed ! : %s %s %d %d",
	   reason,file,line,gsl_errno);
  PoolThreadMgr::Lock aLock(&_lock);
  std::map<pthread_t,std::string>::iterator i = get()._errorMessage.find(pthread_self());
  if(i != get()._errorMessage.end())
    {
      i->second += '\n';
      i->second += aTmpBuffer;
    }
  else
    get()._errorMessage.insert(ErrorMessageType::value_type(pthread_self(),
							    aTmpBuffer));
  get()._lastGslErrno.insert(ErrnoType::value_type(pthread_self(),gsl_errno));
}
