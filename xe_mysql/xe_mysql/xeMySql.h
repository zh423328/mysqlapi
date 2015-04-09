//////////////////////////////////////////////////////////////////////////
//简单mysql处理
//////////////////////////////////////////////////////////////////////////
#ifndef XEMYSQL_H_
#define XEMYSQL_H_

#include "ThreadPool.h"
#include "mysql.h"

class CXEMySqlCon
{
public:
	MYSQL * m_pMySql;
	CLock cs;
};

class CXEMySqlTask : public ITask
{
public:
	CXEMySqlTask();
	~CXEMySqlTask();

	char szQuery[256];		//查询语句
};

//结果
class CXEMySqlResult
{
public:
	char _name[32];
	char _data[128];
	BYTE _fields;

	CXEMySqlResult()
	{
		memset(this,0,sizeof(CXEMySqlResult));
	}
};

class CXEMySql;

//工作者
class CXEMySqlWorker : public IWorker
{
public:
	CXEMySqlWorker(CXEMySql *pMysql){ m_pMySql = m_pMySql;}
	~CXEMySqlWorker();

	void ProcessTask(ITask *pTask);

	CXEMySql * m_pMySql; 
};

class CXEMySql
{
public:
	CXEMySql();
	~CXEMySql();

	bool Initialize(const char* Hostname, unsigned int port,const char* Username, const char* Password, const char* DatabaseName,int ConnectionCount,int JobCount);

	void Start();

	void Stop();

	//重写连
	void Reconnect(CXEMySqlCon *pCon);

	// pool func
	void ExecuteQueryNoRet(char *query);

	bool SelectDB(CXEMySqlCon*con,char *db);

	bool Query(CXEMySqlCon*con,char*cmd,MYSQL_RES *res);

	bool Query(char*cmd,CXEMySqlResult *data);

	bool GetFirstRow(CXEMySqlCon*con,char*cmd,MYSQL_ROW &row);

	CXEMySqlCon * GetFreeCon();
	void ReleaseCon(CXEMySqlCon*con);

	CXEMySqlTask * GetFreeJob(char *query);
	void ReleaseJob(CXEMySqlTask*job);

	//操作错误
	bool _HandleError(CXEMySqlCon*, xe_uint32 ErrorNumber);
private:
	CThreadPool m_ThreadPool;

	std::string m_szHostName;
	std::string m_szUserName;
	std::string m_szPassword;
	std::string m_szDatabaseName;

	xe_uint32   m_nPort;
	xe_uint32   m_nConnection;
	xe_uint32   m_nJob
 };