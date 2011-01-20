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
#ifndef __PTHREAD_KEY_H__
#define __PTHREAD_KEY_H__

#include "Compatibility.h"

typedef long pthread_key_t;

#ifdef __cplusplus
extern "C"{
#endif

  DLL_EXPORT int pthread_key_create(pthread_key_t *key, void (* dest)(void *));

  DLL_EXPORT int pthread_key_delete(pthread_key_t key);

  DLL_EXPORT void* pthread_getspecific(pthread_key_t key);
  DLL_EXPORT int pthread_setspecific(pthread_key_t key,
				     const void *pointer);
#ifdef __cplusplus
} //  Assume C declarations for C++
#endif
#endif
