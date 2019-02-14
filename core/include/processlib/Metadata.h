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

#if !defined(PROCESSLIB_METADATA_H)
#define PROCESSLIB_METADATA_H

#include <memory>
#include <string>
#include <unordered_map>

#include <processlib_export.h>

class PROCESSLIB_EXPORT HeaderContainer
{
public:
  typedef std::unordered_map<std::string, std::string> Header;

  HeaderContainer();
  ~HeaderContainer();

  void insert(const char *key,const char *value);
  void insertOrIncKey(const std::string &key,const std::string &value);
  void erase(const char *key);
  void clear();

  const char* get(const char *key,const char *defaultValue = NULL) const;
  int size() const;

  const char* operator[](const char *aKey) const {return get(aKey);}

  HeaderContainer& operator=(const HeaderContainer&);

  // ExpertMethodes for macro insertion a loop
  void lock();
  void unlock();
  pthread_mutex_t* mutex() const;
  Header& header();
  const Header& header() const;

private:
  struct HeaderHolder;
  
  std::unique_ptr<HeaderHolder> m_impl;
};


PROCESSLIB_EXPORT std::ostream& operator<<(std::ostream &os,
				    const HeaderContainer &aHeader);


#endif //!defined(PROCESSLIB_METADATA_H)
