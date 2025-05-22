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
    // Forward definition
    class Holder;

public:
    typedef std::map<std::string, T> Map;

    typedef typename Map::value_type value_type;
    typedef typename Map::iterator iterator;
    typedef typename Map::const_iterator const_iterator;
    typedef std::pair<iterator, bool> insert_type;

    typedef PoolThreadMgr::LockGuard LockGuard;

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
    friend class LockedPtr;

    class LockedPtr
    {
    public:
        typedef Container::Map Map;
        // For backward compatibility with HeaderContainer
        typedef Map Header;

	LockedPtr(const Container& cont);

	LockedPtr() = delete;
	LockedPtr(const LockedPtr& o) = delete;
	LockedPtr(LockedPtr&& o) = default;

	LockedPtr& operator=(const LockedPtr& o) = delete;
	LockedPtr& operator=(LockedPtr&& o) = default;

        bool isLocked() const;
	void unLock();

	Map *operator->();
	Map const *operator->() const;
	Map& operator*();
	Map const& operator*() const;

    private:
	std::shared_ptr<typename Container::Holder> _holder;
	LockGuard _lock;
    };

    class SharedLockedPtr
    {
    public:
        SharedLockedPtr(const Container& cont);

	SharedLockedPtr(const SharedLockedPtr& o) = default;
	SharedLockedPtr(SharedLockedPtr&& o) = default;

	SharedLockedPtr& operator=(const SharedLockedPtr& o) = default;
	SharedLockedPtr& operator=(SharedLockedPtr&& o) = default;

	bool isLocked() const;
	void unLock();

	Map *operator->();
	Map const *operator->() const;
	Map& operator*();
	Map const& operator*() const;

    private:
	std::shared_ptr<LockedPtr> _ptr;
    };

private:
    class Holder
    {
    public:
        Holder();
        ~Holder();

        LockGuard lock();
	Map& get(LockGuard& l);

    private:
	pthread_mutex_t _mutex;
        Map _map;
    };

    std::shared_ptr<Holder> _ptr;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Container<T>& cont);

#include "Container.inl"

#endif //!defined(PROCESSLIB_CONTAINER_H)
