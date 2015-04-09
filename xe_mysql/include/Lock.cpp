#include "StdAfx.h"
#include ".\lock.h"

CLock::CLock(void)
{
	InitializeCriticalSection(&m_cs);
}

CLock::~CLock(void)
{
	DeleteCriticalSection(&m_cs);
}

void CLock::Lock()
{
	EnterCriticalSection(&m_cs);
}

void CLock::UnLock()
{
	LeaveCriticalSection(&m_cs);
}

CAutoLock::CAutoLock(CLock *t)
{
	m_pLock = t;

	if (m_pLock)
	{
		m_pLock->Lock();
	}
}

CAutoLock::~CAutoLock()
{
	if (m_pLock)
	{
		m_pLock->UnLock();
	}
}