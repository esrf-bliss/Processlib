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

#if !defined(PROCESSLIB_BUFFER_H)
#define PROCESSLIB_BUFFER_H

#ifndef __unix
#pragma warning(disable : 4251)
#pragma warning(disable : 4244)
#endif

#include <atomic>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <cstring>

#include <functional>
#include <mutex>

#include <processlib_export.h>
#include "processlib/ProcessExceptions.h"

// template <typename T>
struct PROCESSLIB_EXPORT Buffer {

  std::function<void (void*)> Callback;

  enum Ownership { MAPPED, SHARED };

  ~Buffer()
  {
    if (m_callback)
        m_callback->destroy(data);
    pthread_mutex_destroy(&_lock);
  }

  Buffer() : owner(SHARED), refcount(1), data(NULL), callback(NULL) { pthread_mutex_init(&_lock, NULL); }

  explicit Buffer(int aSize) : owner(SHARED), refcount(1), callback(NULL)
  {
#ifdef __unix
    if (posix_memalign(&data, 16, aSize))
#else /* window */
    data = _aligned_malloc(aSize, 16);
    if (!data)
#endif
      std::cerr << "Can't allocate memory" << std::endl;
    pthread_mutex_init(&_lock, NULL);
  }

//   void unref()
//   {
//     while (pthread_mutex_lock(&_lock))
//       ;
//     if (!(--refcount)) {
//       if (owner == SHARED && data)
// #ifdef __unix
//         free(data);
// #else
//         _aligned_free(data);
// #endif
//       pthread_mutex_unlock(&_lock);
//       delete this;
//     } else
//       pthread_mutex_unlock(&_lock);
//   }
// 
//   void ref()
//   {
//     while (pthread_mutex_lock(&_lock))
//       ;
//     ++refcount;
//     pthread_mutex_unlock(&_lock);
//   }

  Ownership owner;
  std::atomic<int> refcount;
  // T* data;
  void *data;
  //std::mutex lock;
  Callback m_callback;
};

inline std::ostream &operator<<(std::ostream &os, const Buffer &aBuffer)
{
  const char *anOwnerShip = (aBuffer.owner == Buffer::MAPPED) ? "Mapped" : "Shared";
  os << "<"
     << "owner=" << anOwnerShip << ", "
     << "refcount=" << aBuffer.refcount << ", "
     << "data=" << aBuffer.data << ">";
  return os;
}

#endif //! defined(PROCESSLIB_BUFFER_H)
