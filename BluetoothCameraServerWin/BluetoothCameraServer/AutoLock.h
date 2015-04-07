#pragma once
#include <pthread.h>

/**
 * @class CAutoLock
 * @brief �������b�N�N���X
 */
class CAutoLock
{
public:
	CAutoLock(pthread_mutex_t* lock);
	~CAutoLock();
private:
	pthread_mutex_t* m_lock;
};

