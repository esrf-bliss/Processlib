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

template <typename T>
Container<T>::Holder::Holder() : refcount(1)
{
	pthread_mutexattr_init(&_lock_attr);
	pthread_mutexattr_settype(&_lock_attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&_lock, &_lock_attr);
}

template <typename T>
Container<T>::Holder::~Holder()
{
	pthread_mutex_destroy(&_lock);
	pthread_mutexattr_destroy(&_lock_attr);
}

template <typename T>
inline void Container<T>::Holder::ref()
{
	while (pthread_mutex_lock(&_lock));
	++refcount;
	pthread_mutex_unlock(&_lock);
}

template <typename T>
inline void Container<T>::Holder::unref()
{
	while (pthread_mutex_lock(&_lock));
	if (!(--refcount))
	{
		pthread_mutex_unlock(&_lock);
		delete this;
	}
	else
		pthread_mutex_unlock(&_lock);
}

template <typename T>
inline void Container<T>::Holder::lock()
{
	while (pthread_mutex_lock(&_lock));
}

template <typename T>
inline void Container<T>::Holder::unlock()
{
	pthread_mutex_unlock(&_lock);
}

template <typename T>
Container<T>::Container() :
	_ptr(new Holder())
{
}

template <typename T>
Container<T>::Container(const Container& cnt) :
	_ptr(NULL)
{
	*this = cnt;
}

template <typename T>
Container<T>::~Container()
{
	if (_ptr)
		_ptr->unref();
}

template <typename T>
Container<T>& Container<T>::operator=(const Container& cnt)
{
	cnt._ptr->ref();
	if (_ptr) _ptr->unref();
	_ptr = cnt._ptr;
	return *this;
}

template <typename T>
bool Container<T>::insert(const std::string& key, const T& value)
{
	_ptr->lock();
	Map& map = _ptr->map;
	insert_type result = map.insert(value_type(key, value));
	if (!result.second)
		result.first->second = value;
	_ptr->unlock();

	return result.second;
}

template <typename T>
void Container<T>::insertOrIncKey(const std::string& key, const T& value)
{
	_ptr->lock();
	Map& map = _ptr->map;
	insert_type result = map.insert(value_type(key, value));
	int aNumber = 0;
	std::stringstream aTmpKey;
	while (!result.second)
	{
		aTmpKey << key << '_' << ++aNumber;
		result = map.insert(value_type(aTmpKey.str(), value));
		aTmpKey.seekp(0, aTmpKey.beg);
	}
	_ptr->unlock();
}

template <typename T>
void Container<T>::erase(const std::string& key)
{
	_ptr->lock();
	Map& map = _ptr->map;
	iterator i = map.find(key);
	if (i != map.end())
		map.erase(i);
	_ptr->unlock();
}

template <typename T>
void Container<T>::clear()
{
	_ptr->lock();
	_ptr->map.clear();
	_ptr->unlock();
}

template <typename T>
void Container<T>::reset()
{
	if (_ptr)
		_ptr->unref();
	_ptr = new Holder();
}

template <typename T>
typename Container<T>::Optional Container<T>::get(const std::string& key) const
{
	Optional aReturnValue;
	_ptr->lock();
	Map& map = _ptr->map;
	const_iterator i = map.find(key);
	if (i != map.end())
		aReturnValue = i->second;
	_ptr->unlock();
	return aReturnValue;
}

template <typename T>
T Container<T>::get(const std::string& key, const T& defaultValue) const
{
	T aReturnValue = defaultValue;
	_ptr->lock();
	Map& map = _ptr->map;
	const_iterator i = map.find(key);
	if (i != map.end())
		aReturnValue = i->second.c_str();
	_ptr->unlock();
	return aReturnValue;
}

template <typename T>
bool Container<T>::contains(const std::string& key) const
{
	_ptr->lock();
	Map& map = _ptr->map;
	const_iterator i = _ptr->map.find(key);
	_ptr->unlock();
	return (i != map.end());
}

template <typename T>
int Container<T>::count(const std::string& key) const
{
	_ptr->lock();
	int aReturnCount = int(_ptr->map.count(key));
	_ptr->unlock();
	return aReturnCount;
}

template <typename T>
int Container<T>::size() const
{
	_ptr->lock();
	int aReturnSize = int(_ptr->map.size());
	_ptr->unlock();
	return aReturnSize;
}

template <typename T>
bool Container<T>::empty() const
{
	_ptr->lock();
	bool res = _ptr->map.empty();
	_ptr->unlock();
	return res;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Container<T>& cont)
{
	os << "< ";
	PoolThreadMgr::Lock aLock(cont.mutex());
	//const Container<T>::Map& m = aHeader.map();
	for (typename Container<T>::value_type const& v : cont.map())
		os << "(" << v.first << "," << v.second << ") ";
	os << ">";
	return os;
}
