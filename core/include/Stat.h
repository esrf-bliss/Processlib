#include <stdio.h>      /* for printf() and fprintf() */
#ifdef __unix
#include <sys/time.h>
#endif
#include <stdlib.h>
#include <sstream>
#include <string>

class Stat
{
public:
  Stat(const Data &data,const std::string &info) : _data(data),_info(info) 
  {
    if(!info.empty())
      gettimeofday(&_start,NULL);
  }
  ~Stat()
  {
    if(!_info.empty())
      {
	gettimeofday(&_end,NULL);
	double diff = (_end.tv_sec - _start.tv_sec) + 
	  (_end.tv_usec - _start.tv_usec) / 1e6;
	std::stringstream str;
	str << "take : " << diff << "s";
   
	_data.header.insertOrIncKey(_info,str.str());
      }
  }
private:
  Data		_data;
  std::string	_info;
  struct timeval _start,_end;
};
