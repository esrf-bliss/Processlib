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
#include "processlib/Data.h"
#include "processlib/PoolThreadMgr.h"
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
  int aReturnSize = int(_header->header.size());
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
pthread_mutex_t* Data::HeaderContainer::mutex() const
{
  return &_header->_lock;
}

Data::HeaderContainer::Header& Data::HeaderContainer::header()
{
  return _header->header;
}

const Data::HeaderContainer::Header& Data::HeaderContainer::header() const
{
  return _header->header;
}


std::ostream& operator<<(std::ostream &os,
			 const Data::HeaderContainer &aHeader)
{
  os << "< ";
  PoolThreadMgr::Lock aLock(aHeader.mutex());
  const Data::HeaderContainer::Header &header = aHeader.header();
  for(Data::HeaderContainer::Header::const_iterator i = header.begin();
      i != header.end();++i)
    os << "(" << i->first << "," << i->second << ") ";
  os << ">";
  return os;
}
