#pragma once
#include <pthread.h>

class CAutoLock
{
public:
	CAutoLock(pthread_mutex_t* lock);
	~CAutoLock();
private:
	pthread_mutex_t* m_lock;
};

