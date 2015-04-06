#include "stdafx.h"
#include "AutoLock.h"


CAutoLock::CAutoLock(pthread_mutex_t* lock) :
	m_lock(lock)
{
	pthread_mutex_lock(m_lock);

}


CAutoLock::~CAutoLock()
{
	pthread_mutex_unlock(m_lock);
}
