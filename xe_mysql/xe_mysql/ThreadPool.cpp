#include "ThreadPool.h"

CThreadPool::CThreadPool()
{

}

CThreadPool::~CThreadPool()
{

}

void CThreadPool::Start( xe_uint32 nMax )
{
	CAutoLock lock(&m_arrayCs);
	//工作者线程
	m_hWorkerIoPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);
	for (xe_int32 i = 0; i < nMax; ++i)
	{
		//创建工作者线程
		ThreadInfo *pThread = new ThreadInfo();
		
		pThread->m_hThread = CreateThread(NULL,0,WorkerProc,this,0,(LPDWORD)&pThread->m_dwThreadIdx);

		m_ThreadList.push_back(pThread);
	}
}

void CThreadPool::Stop()
{
	xe_uint32 nCount = 0;
	HANDLE* pThread;

	CAutoLock lock(&m_arrayCs);

	//shut down all the worker threads
	nCount = m_ThreadList.size();
	pThread = new HANDLE[nCount];

	for (std::list<ThreadInfo*>::iterator iter = m_ThreadList.begin(); iter != m_ThreadList.end();++iter)
	{
		PostQueuedCompletionStatus(m_hWorkerIoPort, 0, 0, (OVERLAPPED*)EXIT_CODE);
	}

	WaitForMultipleObjects(nCount, pThread, TRUE, INFINITE);//wait for 2 minutes, then start to kill threads
	CloseHandle(m_hWorkerIoPort);

	SAFE_DELETE_ARRAY(pThread);
}

void CThreadPool::ProcessJob( ITask* pJob, IWorker* pWorker ) const
{
	PostQueuedCompletionStatus(m_hWorkerIoPort,reinterpret_cast<DWORD>(pWorker), reinterpret_cast<DWORD>(pJob),NULL);
}

void CThreadPool::ChangeStatus( DWORD threadId, bool status )
{
	CAutoLock lock(&m_arrayCs);

	for (std::list<ThreadInfo*>::iterator iter = m_ThreadList.begin(); iter != m_ThreadList.end();++iter)
	{
		ThreadInfo *pInfo = *iter;
		if (pInfo && pInfo->m_dwThreadIdx == threadId)
		{
			pInfo->m_bBusyWorking = status;
		}
	}
}

DWORD WINAPI CThreadPool::WorkerProc( void* p )
{
	CThreadPool *pPool = (CThreadPool*)p;
	if (pPool == NULL)
		return 0;

	xe_uint32 dwBytes = 0;

	HANDLE		 IoPort	= pPool->GetWorkerIoPort();
	DWORD		 pN1, pN2; 
	OVERLAPPED*	 pOverLapped;

	//获取当前线程
	xe_uint32 dwThreadIdx = GetCurrentThreadId();

	while(true)
	{
		BOOL dwRet = GetQueuedCompletionStatus(pPool->GetWorkerIoPort(),&pN1,&pN2,&pOverLapped,INFINITE);
		if (dwRet == FALSE)
		{
			pPool->RemoveThread(dwThreadIdx);
			break;
		}
		else
		{
			//修改
			if (pOverLapped == (OVERLAPPED*)EXIT_CODE)
			{
				pPool->RemoveThread(dwThreadIdx);
				break;
			}
			else
			{
				IWorker *pWorker = reinterpret_cast<IWorker*>(pN1);
				ITask* pTask = reinterpret_cast<ITask*>(pN2);

				//pPool->ChangeStatus(dwThreadIdx,true);
				//完成任务
				if (pWorker)
					pWorker->ProcessTask(pTask);

				//释放task
				

				//pPool->ChangeStatus(dwThreadIdx,false);
			}
		}
	}

	return 1;
}

void CThreadPool::RemoveThread( xe_uint32 threadId )
{
	CAutoLock lock(&m_arrayCs);

	for (std::list<ThreadInfo*>::iterator iter = m_ThreadList.begin(); iter != m_ThreadList.end();++iter)
	{
		ThreadInfo *pInfo = *iter;
		if (pInfo && pInfo->m_dwThreadIdx == threadId)
		{
			SAFE_DELETE(pInfo);
			m_ThreadList.erase(iter);
			break;
		}
	}
}

