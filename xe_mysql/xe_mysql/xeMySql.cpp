#include "xeMySql.h"

void CXEMySqlWorker::ProcessTask( ITask *pTask )
{
	if (m_pMySql &&pTask)
	{
		CXEMySqlTask *pMySqlTask = dynamic_cast<CXEMySqlTask*>(pTask);

		MYSQL_RES res;
		m_pMySql->Query(pMySqlTask->szQuery,&res);
	}
}

CXEMySql::CXEMySql()
{
	m_pMySqlTaskPool = NULL;
	m_pWorker = NULL;
}

CXEMySql::~CXEMySql()
{
	SAFE_DELETE(m_pMySqlTaskPool);		//放到最后释放
	SAFE_DELETE(m_pWorker);
}

bool CXEMySql::Initialize( const char* Hostname, unsigned int port,const char* Username, const char* Password, const char* DatabaseName,xe_uint32 ConnectionCount,xe_uint32 JobCount )
{
	m_szHostName = Hostname;
	m_nPort = port;
	m_szUserName = Username;
	m_szPassword = Password;
	m_szDatabaseName = DatabaseName;
	

	for (xe_uint32 i = 0; i < ConnectionCount;++i)
	{
		CXEMySqlCon *pCon = new CXEMySqlCon();
		
		pCon->m_pMySql = mysql_init(NULL);		//初始化

		if (pCon->m_pMySql == NULL)
			continue;
		else
		{
			//连接
			if (!mysql_real_connect(pCon->m_pMySql,Hostname,Username,Password,DatabaseName,m_nPort,NULL,0))
			{
				printf("数据库连接失败,原因如下: %s\n",mysql_error(pCon->m_pMySql));
				return false;
			}

			if (!pCon->m_pMySql)
			{
				mysql_close(pCon->m_pMySql);
				SAFE_DELETE(pCon);

				return false;
			}

			//设置
			if(mysql_options(pCon->m_pMySql, MYSQL_SET_CHARSET_NAME, "utf8"))
			{
				printf("MySQLDatabase Could not set utf8 character set.");
				return false;
			}

			my_bool my_true =true;
			if(mysql_options(pCon->m_pMySql, MYSQL_OPT_RECONNECT, &my_true))
			{
				printf("MySQLDatabase MYSQL_OPT_RECONNECT could not be set, connection drops may occur but will be counteracted.");
				return false;
			}
		}

		//		//初始化
		m_VectorCon.push_back(pCon);
	}

	m_pMySqlTaskPool = new CPool<CXEMySqlTask>(JobCount);

	m_pWorker = new CXEMySqlWorker(this);

	return true;
}

void CXEMySql::Start()
{
	SYSTEM_INFO si;

	GetSystemInfo(&si);

	int processNum = si.dwNumberOfProcessors;

	//启动线程
	m_ThreadPool.Start(processNum);
}


void CXEMySql::Stop()
{
	m_ThreadPool.Stop();
}

//重新连接
bool CXEMySql::Reconnect( CXEMySqlCon *pCon )
{
	MYSQL * temp ,*temp2;

	temp = mysql_init(NULL);
	temp2 = mysql_real_connect(temp,m_szHostName.c_str(),m_szUserName.c_str(),m_szPassword.c_str(),m_szDatabaseName.c_str(),m_nPort,NULL,0);
	if(temp2 == NULL)
	{
		printf("MySQLDatabase Could not reconnect to database because of `%s`", mysql_error(temp));
		mysql_close(temp);
		return false;
	}

	if(pCon->m_pMySql != NULL)
		mysql_close(pCon->m_pMySql);

	pCon->m_pMySql= temp;

	return true;
}

void CXEMySql::ExecuteQueryNoRet( char *query )
{
	//执行查询
	CXEMySqlTask *pTask = GetFreeJob(query);		

	m_ThreadPool.ProcessJob(pTask,m_pWorker);
}

bool CXEMySql::SelectDB(char *db )
{
	CXEMySqlCon *pCon = GetFreeCon();
	if (pCon &&pCon->m_pMySql)
	{
		CAutoLock cs(&pCon->cs);
		if(mysql_select_db(pCon->m_pMySql,db) == 0)
		{
			return true;
		}
	}
	return false;
}

bool CXEMySql::Query( char*cmd,MYSQL_RES *res )
{
	CXEMySqlCon *pCon = GetFreeCon();
	if (pCon &&pCon->m_pMySql)
	{
		//执行
		CAutoLock cs(&pCon->cs);

		//执行成功返回0
		if (mysql_real_query(pCon->m_pMySql,cmd,strlen(cmd)) == 0)
		{
			res = mysql_store_result(pCon->m_pMySql);

			if(res)
				return true;
		}
		else
		{
			//输出错误
			printf("cmd:%s errorno:%d,error:%s\n",cmd, mysql_errno(pCon->m_pMySql),mysql_error(pCon->m_pMySql));

			return HandleError(pCon,mysql_errno(pCon->m_pMySql));		//部分错误重新连接
		}
	}

	return false;
}

bool CXEMySql::Query(char*cmd,CXEMySqlResult *data )
{
	CXEMySqlCon *pCon = GetFreeCon();
	if (pCon &&pCon->m_pMySql)
	{
		CAutoLock cs(&pCon->cs);

		MYSQL_RES *res = NULL;
		if (mysql_real_query(pCon->m_pMySql,cmd,strlen(cmd))==0)
		{
			int nLength = mysql_field_count(pCon->m_pMySql);
			if( nLength > 0)			//查看当前查询的列数
			{
				res = mysql_store_result(pCon->m_pMySql);		//返回结果集,没有行数返回NULL;

				if (data)
				{
					data->SetRes(nLength,res);

					return true;
				}
			}

			return false;
		}
		else
		{
			//输出错误
			printf("cmd:%s errorno:%d,error:%s\n",cmd, mysql_errno(pCon->m_pMySql),mysql_error(pCon->m_pMySql));

			return HandleError(pCon,mysql_errno(pCon->m_pMySql));		//部分错误重新连接
		}
	}

	return false;
}

CXEMySqlCon * CXEMySql::GetFreeCon()
{
	xe_uint32 nCount = m_VectorCon.size();
	xe_uint32 i = 0;
	for (;;++i)
	{
		CXEMySqlCon *pCon = m_VectorCon.at(i%nCount);
		if (pCon->cs.AttemptLock())
			return pCon;
	}

	return NULL;;
}

CXEMySqlTask * CXEMySql::GetFreeJob( char *query )
{
	if (query == NULL)
		return NULL;

	CXEMySqlTask *pTask = m_pMySqlTaskPool->Alloc();

	strncpy(pTask->szQuery,query,strlen(query));


	return pTask;
}

void CXEMySql::ReleaseJob( CXEMySqlTask*job )
{
	if (job)
	{
		memset(job->szQuery,0,256);

		m_pMySqlTaskPool->Free(job);
	}
}

bool CXEMySql::HandleError( CXEMySqlCon*pCon, xe_uint32 ErrorNumber )
{
	switch(ErrorNumber)
	{
	case 2006:  // Mysql server has gone away
	case 2008:  // Client ran out of memory
	case 2013:  // Lost connection to sql server during query
	case 2055:  // Lost connection to sql server - system error
		{
			// Let's instruct a reconnect to the db when we encounter these errors.
			return Reconnect(pCon);
		}
		break;
	}

	return false;
}



CXEMySqlResult::CXEMySqlResult()
{
	m_pRes = NULL;
	m_pRecords = NULL;
	m_pRow = NULL;
	m_pLengths = NULL;
	m_pFields = NULL;
	m_dwFieldCnt = 0;
}

CXEMySqlResult::~CXEMySqlResult()
{
	if (m_pRecords)
	{
		mysql_free_result(m_pRecords);
	}
}


void CXEMySqlResult::SetRes( xe_int32 nCount,MYSQL_RES *pRes )
{
	m_dwFieldCnt = nCount;
	m_pRes = pRes;
}

//取所有结果
xe_bool CXEMySqlResult::FetchRecords()
{
	if (m_pRes == NULL)
		return false;

	if (m_dwFieldCnt <= 0)
		return 0;

	m_pFields = mysql_fetch_fields(m_pRes);		//结果集

	return true;			
}

xe_bool CXEMySqlResult::FetchRow()
{
	//获取每一行
	if (m_pRes == NULL || m_dwFieldCnt <= 0)
		return false;

	m_pRow = mysql_fetch_row(m_pRes);		//获取一行数据

	if (m_pRow == NULL)
		return false;

	m_pLengths = (xe_uint32*)mysql_fetch_lengths(m_pRes);	//当前行数的大小

	return true;
}

xe_int32 CXEMySqlResult::_GetFieldIndex( const xe_int8* szField )
{
	//获取区域
	if (m_pRes == NULL || m_pRow == NULL || szField == NULL)
		return -1;

	for (xe_int32  i = 0; i < m_dwFieldCnt; ++i)
	{
		if (stricmp(m_pFields[i].name,szField) == 0)
			return i;
	}

	return -1;
}

xe_int8* CXEMySqlResult::GetFieldName( xe_int32 nIndex )
{
	//获取区域
	if (m_pRes == NULL || m_pRow == NULL || nIndex < 0 || nIndex >= m_dwFieldCnt)
		return "";

	if (m_pFields[nIndex].name)
		return m_pFields[nIndex].name;

	return "";
}


xe_uint32 CXEMySqlResult::GetRecordsCnt()
{
	return m_dwFieldCnt;
}

xe_int8* CXEMySqlResult::GetStr( const xe_int8* szField )
{
	int index = _GetFieldIndex(szField);
	if (index < 0)
		return "";

	if (m_pRow[index])
		return m_pRow[index];

	return "";
}

//获取word数据
xe_uint16 CXEMySqlResult::GetWORD( const xe_int8* szField )
{
	int index = _GetFieldIndex(szField);
	if (index < 0)
		return 0;

	if (m_pRow[index])
		return (xe_uint16)atoi(m_pRow[index]);

	return 0;
}

xe_int32 CXEMySqlResult::GetInt( const xe_int8* szField )
{
	int index = _GetFieldIndex(szField);
	if (index < 0)
		return 0;

	if (m_pRow[index])
		return atoi(m_pRow[index]);

	return 0;
}

xe_uint32 CXEMySqlResult::GetDWORD( const xe_int8* szField )
{
	int index = _GetFieldIndex(szField);
	if (index < 0)
		return 0;

	if (m_pRow[index])
		return (xe_uint32)atoi(m_pRow[index]);

	return 0;
}

xe_double CXEMySqlResult::GetDouble( const xe_int8* szField )
{
	int index = _GetFieldIndex(szField);
	if (index < 0)
		return 0.0;

	if (m_pRow[index])
		return atof(m_pRow[index]);

	return 0.0;
}

xe_int8* CXEMySqlResult::GetFieldValue( xe_int32 nIndex,xe_int32* pnLen /*= NULL*/ )
{
	if (nIndex < 0 || nIndex >= m_dwFieldCnt)
		return "";

	if (pnLen!= NULL)
		*pnLen = m_pLengths[nIndex];

	if (m_pRow[nIndex])
		return m_pRow[nIndex];

	return "";
}

xe_bool CXEMySqlResult::IsBLOBField( xe_int32 nIndex )
{
	if (nIndex < 0 || nIndex >= m_dwFieldCnt)
		return false;

	if (m_pFields[nIndex].flags && BLOB_FLAG)
		return true;

	return false;
}

xe_uint8* CXEMySqlResult::GetBLOB( const xe_int8* szField,xe_int32* pnLen )
{
	xe_int32 nIndex = _GetFieldIndex(szField);

	if (nIndex < 0)
		return (xe_uint8*)"";

	if (pnLen!= NULL)
		*pnLen = m_pLengths[nIndex];

	if (m_pRow[nIndex])
		return (xe_uint8*)m_pRow[nIndex];

	return (xe_uint8*)"";
}



