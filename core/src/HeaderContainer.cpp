#include "Data.h"
#include <sstream>

struct Data::HeaderContainer::HeaderHolder
{
  HeaderHolder() : refcount(1)
  {
    pthread_mutexattr_init(&_lock_attr);
    pthread_mutexattr_settype(&_lock_attr,PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&_lock,&_lock_attr);
  }
  ~HeaderHolder()
  {
    pthread_mutex_destroy(&_lock);
    pthread_mutexattr_destroy(&_lock_attr);
  }

  inline void ref()
  {
    while(pthread_mutex_lock(&_lock));
    ++refcount;
    pthread_mutex_unlock(&_lock);
  }
  
  inline void unref()
  {
    while(pthread_mutex_lock(&_lock));
    if(!(--refcount))
      {
	pthread_mutex_unlock(&_lock);
	delete this;
      }
    else
      pthread_mutex_unlock(&_lock);
  }

  inline void lock()
  {
    while(pthread_mutex_lock(&_lock));
  }
  inline void unlock()
  {
    pthread_mutex_unlock(&_lock);
  }
  int				refcount;
  pthread_mutex_t		_lock;
  pthread_mutexattr_t           _lock_attr;
  Data::HeaderContainer::Header header;
};


Data::HeaderContainer::HeaderContainer() :
  _header(new HeaderHolder())
{
}

Data::HeaderContainer::HeaderContainer(const HeaderContainer &cnt) :
  _header(NULL)
{
  *this = cnt;
}

Data::HeaderContainer& Data::HeaderContainer::operator=(const HeaderContainer &cnt)
{
  cnt._header->ref();
  if(_header) _header->unref();
  _header = cnt._header;
  return *this;
}

Data::HeaderContainer::~HeaderContainer()
{
  if(_header)
    _header->unref();
}

void Data::HeaderContainer::insert(const char *key,const char *value)
{
  _header->lock();
  std::pair<std::map<std::string,std::string>::iterator, bool> result = 
    _header->header.insert(std::pair<std::string,std::string>(key,value));
  if(!result.second)
    result.first->second = value;
  _header->unlock();
}
void Data::HeaderContainer::insertOrIncKey(const std::string &key,
					   const std::string &value)
{
  _header->lock();
std::pair<std::map<std::string,std::string>::iterator, bool> result = 
  _header->header.insert(std::pair<std::string,std::string>(key,value));
  int aNumber = 0;
  std::stringstream aTmpKey;
  while(!result.second)
    {
      aTmpKey << key << '_' << ++aNumber;
      result = _header->header.insert(std::pair<std::string,std::string>(aTmpKey.str(),value));
      aTmpKey.seekp(0,aTmpKey.beg);
    }
  _header->unlock();
}
void Data::HeaderContainer::erase(const char *key)
{
  _header->lock();
  std::map<std::string,std::string>::iterator i = _header->header.find(key);
  if(i != _header->header.end())
    _header->header.erase(i);
  _header->unlock();
}

void Data::HeaderContainer::clear()
{
  _header->lock();
  _header->header.clear();
  _header->unlock();
}

const char * Data::HeaderContainer::get(const char *key,
					const char *defaultValue) const
{
  const char *aReturnValue = defaultValue;
  _header->lock();
  std::map<std::string,std::string>::const_iterator i = _header->header.find(key);
  if(i != _header->header.end())
    aReturnValue = i->second.c_str();
  _header->unlock();
  return aReturnValue;
}

int Data::HeaderContainer::size() const
{
  _header->lock();
  int aReturnSize = _header->header.size();
  _header->unlock();
  return aReturnSize;
}

void Data::HeaderContainer::lock()
{
  _header->lock();
}

void Data::HeaderContainer::unlock()
{
  _header->unlock();
}
pthread_mutex_t* Data::HeaderContainer::mutex()
{
  return &_header->_lock;
}

Data::HeaderContainer::Header& Data::HeaderContainer::header()
{
  return _header->header;
}
