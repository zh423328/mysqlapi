//////////////////////////////////////////////////////////////////////////
//��mysql����
//////////////////////////////////////////////////////////////////////////
#ifndef XEMYSQL_H_
#define XEMYSQL_H_

#include "CPool.h"
#include "ThreadPool.h"
#include "mysql.h"
#include <vector>

class CXEMySqlCon
{
public:
	MYSQL * m_pMySql;
	CLock cs;
};

class CXEMySqlTask : public ITask
{
public:
	CXEMySqlTask()
	{
		memset(szQuery,0,256);
	}
	~CXEMySqlTask()
	{

	}

	char szQuery[256];		//��ѯ���
};

//���
class CXEMySqlResult
{
public:
	CXEMySqlResult();

	~CXEMySqlResult();

	void			SetRes(xe_int32 nCount,MYSQL_RES *pRes);
	xe_int32		_GetFieldIndex(const xe_int8* szField);
	xe_int8*		GetFieldName(xe_int32 nIndex);

	xe_bool			FetchRecords();
	xe_uint32		GetRecordsCnt();
	xe_uint32		GetFieldCnt();
	xe_bool			FetchRow();

	xe_int8*		GetStr(const xe_int8* szField);

	xe_uint16		GetWORD(const xe_int8* szField);
	xe_int32		GetInt(const xe_int8* szField);
	xe_uint32		GetDWORD(const xe_int8* szField);
	xe_double		GetDouble(const xe_int8* szField);	
	xe_int8*		GetFieldValue(xe_int32 nIndex,xe_int32* pnLen = NULL);

	xe_bool			IsBLOBField(xe_int32 nIndex);
	xe_uint8*		GetBLOB(const xe_int8* szField,xe_int32* pnLen);
private:
	MYSQL_RES*		m_pRes;
	MYSQL_RES*		m_pRecords;			//�����
	MYSQL_ROW		m_pRow;				//��ǰ����
	MYSQL_FIELD*	m_pFields;			//��ǰ��

	xe_uint32*		m_pLengths;			//ÿһ�е�����

	xe_int32		m_dwFieldCnt;		//������������

};

class CXEMySql;

//������
class CXEMySqlWorker : public IWorker
{
public:
	CXEMySqlWorker(CXEMySql *pMysql){ m_pMySql = pMysql;}
	~CXEMySqlWorker(){}

	void ProcessTask(ITask *pTask);

	CXEMySql * m_pMySql; 
};

//typedef std::list<CXEMySqlResult> MySqlResultList;

class CXEMySql
{
public:
	CXEMySql();
	~CXEMySql();

	bool Initialize(const char* Hostname, unsigned int port,const char* Username, const char* Password, const char* DatabaseName,xe_uint32 ConnectionCount,xe_uint32 JobCount);

	void Start();

	void Stop();

	//��д��
	bool Reconnect(CXEMySqlCon *pCon);

	// pool func
	void ExecuteQueryNoRet(char *query);

	bool SelectDB(char *db);

	bool Query(char*cmd,MYSQL_RES *res);

	bool Query(char*cmd,CXEMySqlResult *data);

	CXEMySqlCon * GetFreeCon();

	CXEMySqlTask * GetFreeJob(char *query);
	void ReleaseJob(CXEMySqlTask*job);

	//��������
	bool HandleError(CXEMySqlCon*pCon, xe_uint32 ErrorNumber);
private:
	CThreadPool m_ThreadPool;

	std::string m_szHostName;
	std::string m_szUserName;
	std::string m_szPassword;
	std::string m_szDatabaseName;
	xe_uint32   m_nPort;

	CPool<CXEMySqlTask> * m_pMySqlTaskPool;
	std::vector<CXEMySqlCon*>	m_VectorCon;

	CXEMySqlWorker *m_pWorker;
 };

#endif