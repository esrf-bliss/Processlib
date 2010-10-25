#include <pthread.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>

void (**_pthread_key_dest)(void *);

#define PTHREAD_KEYS_MAX (1<<20)

pthread_rwlock_t _pthread_key_lock;
long _pthread_key_max;
long _pthread_key_sch;

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
	
	if (!_pthread_key_max) _pthread_key_max = 1;
	if (_pthread_key_max == PTHREAD_KEYS_MAX)
	{
		pthread_rwlock_unlock(&_pthread_key_lock);
		
		return ENOMEM;
	}
	
	nmax = _pthread_key_max * 2;
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
