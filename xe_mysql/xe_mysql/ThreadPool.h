//////////////////////////////////////////////////////////////////////////
//�̳߳�
//////////////////////////////////////////////////////////////////////////
#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include "GlobalDefine.h"
#include <list>

//Ҫ��ɵ�����
class ITask
{
public:
	ITask(){}

	virtual ~ITask(){}
};

//�����ߴ���
class IWorker
{
public:
	virtual void ProcessTask(ITask *pTask) = 0;
};

//�߳���Ϣ
struct ThreadInfo
{
	xe_uint32	m_dwThreadIdx;
	HANDLE		m_hThread;
	bool		m_bBusyWorking;
	ThreadInfo() { m_dwThreadIdx = 0; m_hThread=0; m_bBusyWorking=false; }
	ThreadInfo(xe_uint32 dwThreadIdx,HANDLE handle, bool bBusy) { m_dwThreadIdx = dwThreadIdx;  m_hThread=handle; m_bBusyWorking=bBusy; }
	ThreadInfo(const ThreadInfo& info) { m_dwThreadIdx =info.m_dwThreadIdx; m_hThread=info.m_hThread; m_bBusyWorking=info.m_bBusyWorking; }
};

//�̳߳�
class CThreadPool  
{
public:
	//interface to the outside
	//��ʼ
	void Start(xe_uint32 nmax);
	void Stop();

	void ProcessJob(ITask* pJob, IWorker* pWorker) const;

	//constructor and destructor
	CThreadPool();
	virtual ~CThreadPool();

	HANDLE GetWorkerIoPort() const { return m_hWorkerIoPort; }

	static DWORD WINAPI WorkerProc(void* p);

protected:
	 xe_uint32 m_nNumberOfTotalThreads;

	void ChangeStatus(DWORD threadId, bool status);

	void RemoveThread(xe_uint32 threadId);

protected:
	//all the work threads
	//CCriticalSection m_arrayCs;
	CLock m_arrayCs;
	std::list<ThreadInfo*> m_ThreadList;				//�߳�����
	HANDLE m_hWorkerIoPort;
};

#endif