#pragma once
#include "windows.h"

class CLock
{
public:
	CLock(void);

	~CLock(void);

public:
	void Lock();
	void UnLock();

private:
	CRITICAL_SECTION m_cs;
};


class CAutoLock
{
public:
	CAutoLock(CLock *t);
	~CAutoLock();
private:
	CLock *m_pLock;
};