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
Container<T>::Holder::Holder()
{
	pthread_mutexattr_t mutex_attr;
	pthread_mutexattr_init(&mutex_attr);
	pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&_mutex, &mutex_attr);
	pthread_mutexattr_destroy(&mutex_attr);
}

template <typename T>
Container<T>::Holder::~Holder()
{
	pthread_mutex_destroy(&_mutex);
}

template <typename T>
typename Container<T>::LockGuard Container<T>::Holder::lock()
{
	return LockGuard(&_mutex);
}

template <typename T>
typename Container<T>::Map& Container<T>::Holder::get(LockGuard& l)
{
	if (!l.isLocked())
		throw ProcessException("Getting ref. from unlocked Container");
	return _map;
}

template <typename T>
Container<T>::Container() : _ptr(std::make_shared<Holder>())
{
}

template <typename T>
Container<T>::Container(const Container& cnt) :	_ptr(cnt._ptr)
{
}

template <typename T>
Container<T>::LockedPtr::LockedPtr(const Container& cont)
	: _holder(cont._ptr), _lock(_holder->lock())
{
}

template <typename T>
bool Container<T>::LockedPtr::isLocked() const
{
	return bool(_holder);
}

template <typename T>
void Container<T>::LockedPtr::unLock()
{
	if (!isLocked())
		return;
	_lock.unLock();
	_holder.reset();
}

template <typename T>
typename Container<T>::Map *Container<T>::LockedPtr::operator->()
{
	if (!isLocked())
		throw ProcessException("De-referencing non-locked LockedPtr");
	return &_holder->get(_lock);
}

template <typename T>
const typename Container<T>::Map *Container<T>::LockedPtr::operator->() const
{
	if (!isLocked())
		throw ProcessException("De-referencing non-locked LockedPtr");
	return &_holder->get(_lock);
}

template <typename T>
typename Container<T>::Map& Container<T>::LockedPtr::operator*()
{
	if (!isLocked())
		throw ProcessException("De-referencing non-locked LockedPtr");
	return _holder->get(_lock);
}

template <typename T>
const typename Container<T>::Map& Container<T>::LockedPtr::operator*() const
{
	if (!isLocked())
		throw ProcessException("De-referencing non-locked LockedPtr");
	return _holder->get(_lock);
}

template <typename T>
Container<T>::SharedLockedPtr::SharedLockedPtr(const Container& cont)
	: _ptr(std::make_shared<LockedPtr>(cont))
{
}

template <typename T>
bool Container<T>::SharedLockedPtr::isLocked() const
{
	return _ptr && _ptr->isLocked();
}

template <typename T>
void Container<T>::SharedLockedPtr::unLock()
{
	if (_ptr)
		_ptr->unLock();
}

template <typename T>
typename Container<T>::Map *Container<T>::SharedLockedPtr::operator->()
{
	if (!_ptr)
		throw ProcessException("De-referencing moved-from "
		      		       "SharedLockedPtr");
	return &(**_ptr);
}

template <typename T>
const typename Container<T>::Map *Container<T>::SharedLockedPtr::operator->()
									const
{
	if (!_ptr)
		throw ProcessException("De-referencing moved-from "
		      		       "SharedLockedPtr");
	return &(**_ptr);
}

template <typename T>
typename Container<T>::Map& Container<T>::SharedLockedPtr::operator*()
{
	if (!_ptr)
		throw ProcessException("De-referencing moved-from "
		      		       "SharedLockedPtr");
	return **_ptr;
}

template <typename T>
const typename Container<T>::Map& Container<T>::SharedLockedPtr::operator*()
									const
{
	if (!_ptr)
		throw ProcessException("De-referencing moved-from "
		      		       "SharedLockedPtr");
	return **_ptr;
}

template <typename T>
Container<T>& Container<T>::operator=(const Container& cnt)
{
	_ptr = cnt._ptr;
	return *this;
}

template <typename T>
bool Container<T>::insert(const std::string& key, const T& value)
{
	LockedPtr l_ptr(*this);
	insert_type result = l_ptr->insert(value_type(key, value));
	if (!result.second)
		result.first->second = value;

	return result.second;
}

template <typename T>
void Container<T>::insertOrIncKey(const std::string& key, const T& value)
{
	LockedPtr l_ptr(*this);
	insert_type result = l_ptr->insert(value_type(key, value));
	int aNumber = 0;
	std::stringstream aTmpKey;
	while (!result.second)
	{
		aTmpKey << key << '_' << ++aNumber;
		result = l_ptr->insert(value_type(aTmpKey.str(), value));
		aTmpKey.seekp(0, aTmpKey.beg);
	}
}

template <typename T>
void Container<T>::erase(const std::string& key)
{
	LockedPtr l_ptr(*this);
	iterator i = l_ptr->find(key);
	if (i != l_ptr->end())
		l_ptr->erase(i);
}

template <typename T>
void Container<T>::clear()
{
	LockedPtr l_ptr(*this);
	l_ptr->clear();
}

template <typename T>
void Container<T>::reset()
{
	_ptr = std::make_shared<Holder>();
}

template <typename T>
typename Container<T>::Optional Container<T>::get(const std::string& key) const
{
	Optional aReturnValue;
	LockedPtr l_ptr(*this);
	const_iterator i = l_ptr->find(key);
	if (i != l_ptr->end())
		aReturnValue = i->second;
	return aReturnValue;
}

template <typename T>
T Container<T>::get(const std::string& key, const T& defaultValue) const
{
	T aReturnValue = defaultValue;
	LockedPtr l_ptr(*this);
	const_iterator i = l_ptr->find(key);
	if (i != l_ptr->end())
		aReturnValue = i->second.c_str();
	return aReturnValue;
}

template <typename T>
bool Container<T>::contains(const std::string& key) const
{
	LockedPtr l_ptr(*this);
	const_iterator i = l_ptr->find(key);
	return (i != l_ptr->end());
}

template <typename T>
int Container<T>::count(const std::string& key) const
{
	LockedPtr l_ptr(*this);
	int aReturnCount = int(l_ptr->count(key));
	return aReturnCount;
}

template <typename T>
int Container<T>::size() const
{
	LockedPtr l_ptr(*this);
	int aReturnSize = int(l_ptr->size());
	return aReturnSize;
}

template <typename T>
bool Container<T>::empty() const
{
	LockedPtr l_ptr(*this);
	bool res = l_ptr->empty();
	return res;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Container<T>& cont)
{
	os << "< ";
	typename Container<T>::LockedPtr l_ptr(cont);
	for (typename Container<T>::value_type const& v : *l_ptr)
		os << "(" << v.first << "," << v.second << ") ";
	os << ">";
	return os;
}
