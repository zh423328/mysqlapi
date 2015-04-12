//////////////////////////////////////////////////////////////////////////
//��mysql����
//////////////////////////////////////////////////////////////////////////
#ifndef XEMYSQL_H_
#define XEMYSQL_H_

#include "CPool.h"
#include "ThreadPool.h"
#include "mysql.h"
#include <vector>


//mysql����
class CXEMySqlCon
{
public:
	CXEMySqlCon();
	~CXEMySqlCon();


	bool Connect(const char* Hostname,const char* Username, const char* Password, const char* DatabaseName,unsigned int port = 3306);
	bool Close();

	bool Reconnect();	//��������

	MYSQL * m_pMySql;
	CLock cs;

	std::string m_szHostName;
	std::string m_szUserName;
	std::string m_szPassword;
	std::string m_szDatabaseName;
	xe_uint32   m_nPort;
};

//blob����
class DB_BLOB
{
public:
	//
	DB_BLOB(xe_uint8 *pData[],xe_uint32 dwlen[],xe_uint32 dwParamCount);
	~DB_BLOB();

	void Reset();

	xe_uint8 **m_pData;
	xe_uint32 *m_dwLen;
	xe_uint32 m_wParamcount;
};

class CXEMySqlTask : public ITask
{
public:
	CXEMySqlTask()
	{
		m_pData = NULL;
		Init();
	}
	~CXEMySqlTask()
	{
		Init();
	}

	void Init()
	{
		memset(szQuery,0,256);

		SAFE_DELETE(m_pData);
	}

	void SetData(xe_uint8 *pData[],xe_uint32 dwlen[],xe_uint32 dwParamCount)
	{
		SAFE_DELETE(m_pData);
		m_pData = new DB_BLOB(pData,dwlen,dwParamCount);
	}

	char szQuery[256];		//��ѯ���

	DB_BLOB * m_pData;
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

//////////////////////////////////////////////////////////////////////////
//������mysqlԤ�������˲��Ƿǳ��Ƽ���ֻ�п�����blob����ʱ���ܿ�һ�㣬���Ǹ��Ƽ���blob��ָ���
//������2���ƴ洢�����,������Ҫ��ô���
//////////////////////////////////////////////////////////////////////////
class CXEMySqlStmt
{
public:
	CXEMySqlStmt(CXEMySqlCon *pCon);
	~CXEMySqlStmt();

	void SetSQL(char * szSQL);

	//index ����,pData���ݣ�bufflen dwLen����
	void SetParam(int index,xe_uint8*pData, xe_uint32 bufflen,xe_uint32 * dwLen);
	//�󶨲���
	bool BindParams();

	//ִ��
	bool Execute();	

	//����
	bool Reset();
private:
	CXEMySqlCon*	m_pCon;
	MYSQL_STMT*		m_pStmt;
	MYSQL_BIND*		m_pParams;
	xe_uint32		m_dwParamCount;		//��������
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

	bool Initialize(const char* Hostname,const char* Username, const char* Password, const char* DatabaseName,unsigned int port,xe_uint32 ConnectionCount = 5,xe_uint32 JobCount = 1000);

	void Start();

	void Stop();

	//��д��
	bool Reconnect(CXEMySqlCon *pCon);

	// pool func
	void ExecuteQueryNoRet(char *query);

	bool ExecuteSQL(MYSQL *pMySql,char * cmd,...);

	bool SelectDB(char *db);

	bool Query(char*cmd,MYSQL_RES *res,...);

	bool Query(char*cmd,CXEMySqlResult *data,...);
	
	bool Query(char *cmd,DB_BLOB * pData);

	CXEMySqlCon * GetFreeCon();

	CXEMySqlTask * GetFreeJob(char *query);
	void ReleaseJob(CXEMySqlTask*job);

	//��������
	bool HandleError(CXEMySqlCon*pCon, xe_uint32 ErrorNumber);
private:
	CThreadPool m_ThreadPool;

	CPool<CXEMySqlTask> * m_pMySqlTaskPool;
	std::vector<CXEMySqlCon*>	m_VectorCon;

	CXEMySqlWorker *m_pWorker;
 };

#endif