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
#include <pthread.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>

void (**_pthread_key_dest)(void *) = NULL;

#define PTHREAD_KEYS_MAX (1<<20)

pthread_rwlock_t _pthread_key_lock;
long _pthread_key_max = 0;
long _pthread_key_sch = 0;

int pthread_key_create(pthread_key_t *key, void (* dest)(void *))
{
	int i;
	long nmax;
	void (**d)(void *);

	if (!key) return EINVAL;
	
	pthread_rwlock_wrlock(&_pthread_key_lock);
	
	for (i = _pthread_key_sch; i < _pthread_key_max; i++)
	{
		if (!_pthread_key_dest[i])
		{
			*key = i;
			if (dest)
			{
				_pthread_key_dest[i] = dest;
			}
			else
			{
				_pthread_key_dest[i] = (void(*)(void *))1;
			}
			pthread_rwlock_unlock(&_pthread_key_lock);
			
			return 0;
		}
	}
	
	for (i = 0; i < _pthread_key_sch; i++)
	{
		if (!_pthread_key_dest[i])
		{
			*key = i;
			if (dest)
			{
				_pthread_key_dest[i] = dest;
			}
			else
			{
				_pthread_key_dest[i] = (void(*)(void *))1;
			}
			pthread_rwlock_unlock(&_pthread_key_lock);
			
			return 0;
		}
	}
	
	if (_pthread_key_max == PTHREAD_KEYS_MAX)
	{
		pthread_rwlock_unlock(&_pthread_key_lock);
		
		return ENOMEM;
	}
	if(_pthread_key_max)
	  nmax = _pthread_key_max * 2;
	else			// Init
	  nmax = 2;

	if (nmax > PTHREAD_KEYS_MAX) nmax = PTHREAD_KEYS_MAX;
	
	/* No spare room anywhere */
	d = (void (**)(void *))realloc(_pthread_key_dest, nmax * sizeof(*d));
	if (!d)
	{
		pthread_rwlock_unlock(&_pthread_key_lock);
		
		return ENOMEM;
	}
	
	/* Clear new region */
	memset((void *) &d[_pthread_key_max], 0, (nmax-_pthread_key_max)*sizeof(void *));
	
	/* Use new region */
	_pthread_key_dest = d;
	_pthread_key_sch = _pthread_key_max + 1;
	*key = _pthread_key_max;
	_pthread_key_max = nmax;
	
	if (dest)
	{
		_pthread_key_dest[*key] = dest;
	}
	else
	{
		_pthread_key_dest[*key] = (void(*)(void *))1;
	}

	pthread_rwlock_unlock(&_pthread_key_lock);
	
	return 0;
}

int pthread_key_delete(pthread_key_t key)
{
	if (key > _pthread_key_max) return EINVAL;
	if (!_pthread_key_dest) return EINVAL;
	
	pthread_rwlock_wrlock(&_pthread_key_lock);
	_pthread_key_dest[key] = NULL;
	
	/* Start next search from our location */
	if (_pthread_key_sch > key) _pthread_key_sch = key;
	
	pthread_rwlock_unlock(&_pthread_key_lock);
	
	return 0;
}

void* pthread_getspecific(pthread_key_t key)
{
  void *aReturnValuePt = NULL;
  pthread_t t = pthread_self();
  if(t->keymax > key)
    aReturnValuePt = t->keyval[key];
  return aReturnValuePt;
}

int pthread_setspecific(pthread_key_t key,
			const void *pointer)
{
  pthread_t t = pthread_self();
  if(t->keymax <= key)
    {
      int newSize = key + 1;
      void **d = (void**)realloc(t->keyval,newSize * sizeof(void*));
      memset(&d[t->keymax], 0, (newSize - t->keymax)*sizeof(void *));
      t->keymax = newSize;
      t->keyval = d;
    }
  t->keyval[key] = (void*)pointer;
  return 0;
}
