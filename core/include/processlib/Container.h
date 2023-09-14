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

#pragma once

#if !defined(PROCESSLIB_CONTAINER_H)
#define PROCESSLIB_CONTAINER_H

#include <map>
#include <memory>
#include <utility>

#include <iostream> // TO BE REMOVED

#include "processlib/Compatibility.h"
#include "processlib/ProcessExceptions.h"
#include "processlib/PoolThreadMgr.h"

template <typename T>
class DLL_EXPORT Container
{
public:
    typedef std::map<std::string, T> Map;

    typedef typename Map::value_type value_type;
    typedef typename Map::iterator iterator;
    typedef typename Map::const_iterator const_iterator;
    typedef std::pair<iterator, bool> insert_type;

    struct Optional
    {
        bool empty = true;
        T data;

        Optional& operator=(Optional const&) = default;
        Optional& operator=(T const& other) {
            empty = false;
            data = other;
            return *this;
        }

        operator bool() {
            return !empty;
        }

        T& operator*() {
            if (empty)
                throw ProcessException("Deferencing empty Optional");
            else
                return data;
        }
    };

    Container();
    Container(const Container& cnt);
    ~Container();

    bool insert(const std::string& key, const T& value);
    void insertOrIncKey(const std::string& key, const T& value);
    void erase(const std::string& key);
    void clear();  // empty all shared Container instances
    void reset();  // empty only this Container instance

    bool contains(const std::string& key) const;
    int count(const std::string& key) const;
    Optional get(const std::string& key) const;
    T get(const std::string& key, const T& defaultValue) const;
    int size() const;
    bool empty() const;

    const char* operator[](const char* aKey) const { return get(aKey); }

    Container& operator=(const Container&);

    // ensure that Container is locked while a reference to the map is exposed
    friend class LockedRef;

    class LockedRef
    {
    public:
        typedef Container::Map Map;
        // For backward compatibility with HeaderContainer
        typedef Map Header;

	LockedRef() = default;
	LockedRef(const LockedRef& o) = default;
	LockedRef(LockedRef&& o) = default;

	LockedRef(const Container& cont)
	    : m_data(std::make_shared<Data>(cont))
        {
	}

	LockedRef& operator=(const LockedRef& o) = default;
	LockedRef& operator=(LockedRef&& o) = default;

	LockedRef& operator=(const Container& cont)
	{
	    return *this = LockedRef(cont);
	}

        bool isLocked() const { return m_data && m_data->isLocked(); }

	void unLock()
	{
	    if (m_data)
		m_data->unLock();
	}

	Map& get()
	{
	    if (!isLocked())
                throw ProcessException("Non-locked LockedRef");
	    return *m_data->m_map;
	}

    private:
	struct Data {
	    PoolThreadMgr::Lock m_lock;
	    Map *m_map;

	    Data(const Container& cont)
		: m_lock(&cont._ptr->_lock), m_map(&cont._ptr->map)
	    {}

	    bool isLocked() { return m_map; }

	    void unLock()
	    {
	        if (!isLocked())
		    return;
		m_lock.unLock();
		m_map = NULL;
	    }
	};

	std::shared_ptr<Data> m_data;
    };

private:
    struct Holder
    {
        Holder();
        ~Holder();

        void ref();
        void unref();

        void lock();
        void unlock();

        int	refcount;
        pthread_mutex_t		_lock;
        pthread_mutexattr_t _lock_attr;
        Map map;
    };
    Holder* _ptr;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Container<T>& cont);

#include "Container.inl"

#endif //!defined(PROCESSLIB_CONTAINER_H)
