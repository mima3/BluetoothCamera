#pragma once
#include <pthread.h>

/**
 * @class CAutoLock
 * @brief 自動ロッククラス
 */
class CAutoLock
{
public:
	CAutoLock(pthread_mutex_t* lock);
	~CAutoLock();
private:
	pthread_mutex_t* m_lock;
};

