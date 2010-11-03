#include <pthread.h>
#include <map>
#include <string>


/** @brief this class manage error message in thread safe maner
 */
class DLL_EXPORT GslErrorMgr
{
  typedef std::map<pthread_t,std::string> ErrorMessageType;
  typedef std::map<pthread_t,int>	  ErrnoType;
 public:
  static inline GslErrorMgr& get() throw() {return GslErrorMgr::_errorMgr;}
  const char* lastErrorMsg() const;
  int   lastErrno() const;
  void	resetErrorMsg();
 private:
  ErrorMessageType	_errorMessage;
  ErrnoType		_lastGslErrno;
  static GslErrorMgr	_errorMgr;

  GslErrorMgr(); 
  static void _error_handler(const char*,const char *,int,int);
};
