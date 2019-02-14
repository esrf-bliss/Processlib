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

#include <sstream>
#include <utility>

#include "processlib/Metadata.h"

struct HeaderContainer::HeaderHolder
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
  HeaderContainer::Header header;
};

HeaderContainer::HeaderContainer() :
  m_impl(std::make_unique<HeaderHolder>())
{
}

HeaderContainer& HeaderContainer::operator=(const HeaderContainer &cnt)
{
  cnt.m_impl->ref();
  if(m_impl) m_impl->unref();
  m_impl = cnt.m_impl;
  return *this;
}

HeaderContainer::~HeaderContainer()
{
  if(m_impl)
    m_impl->unref();
}

void HeaderContainer::insert(const char *key,const char *value)
{
  m_impl->lock();
  auto result = 
    m_impl->header.insert(std::make_pair(key, value));
  if(!result.second)
    result.first->second = value;
  m_impl->unlock();
}

void HeaderContainer::insertOrIncKey(
    const std::string &key,
    const std::string &value)
{
  m_impl->lock();
  auto result = 
    m_impl->header.insert(std::make_pair(key, value));
  int aNumber = 0;
  std::stringstream aTmpKey;
  while(!result.second)
    {
      aTmpKey << key << '_' << ++aNumber;
      result = m_impl->header.insert(std::make_pair(aTmpKey.str(), value));
      aTmpKey.seekp(0,aTmpKey.beg);
    }
  m_impl->unlock();
}

void HeaderContainer::erase(const char *key)
{
  m_impl->lock();
  auto i = m_impl->header.find(key);
  if(i != m_impl->header.end())
    m_impl->header.erase(i);
  m_impl->unlock();
}

void HeaderContainer::clear()
{
  m_impl->lock();
  m_impl->header.clear();
  m_impl->unlock();
}

const char * HeaderContainer::get(const char *key,
					const char *defaultValue) const
{
  const char *aReturnValue = defaultValue;
  m_impl->lock();
  auto i = m_impl->header.find(key);
  if(i != m_impl->header.end())
    aReturnValue = i->second.c_str();
  m_impl->unlock();
  return aReturnValue;
}

int HeaderContainer::size() const
{
  m_impl->lock();
  int aReturnSize = int(m_impl->header.size());
  m_impl->unlock();
  return aReturnSize;
}

void HeaderContainer::lock()
{
  m_impl->lock();
}

void HeaderContainer::unlock()
{
  m_impl->unlock();
}

pthread_mutex_t* HeaderContainer::mutex() const
{
  return &m_impl->_lock;
}

HeaderContainer::Header& HeaderContainer::header()
{
  return m_impl->header;
}

const HeaderContainer::Header& HeaderContainer::header() const
{
  return m_impl->header;
}

std::ostream& operator<<(std::ostream &os,
			 const HeaderContainer &aHeader)
{
  os << "< ";
  PoolThreadMgr::Lock aLock(aHeader.mutex());
  const HeaderContainer::Header &header = aHeader.header();
  for(HeaderContainer::Header::const_iterator i = header.begin();
      i != header.end();++i)
    os << "(" << i->first << "," << i->second << ") ";
  os << ">";
  return os;
}
