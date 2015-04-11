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

bool CXEMySql::Initialize( const char* Hostname,const char* Username, const char* Password, const char* DatabaseName,unsigned int port /*= 3306*/,xe_uint32 ConnectionCount /*= 5*/,xe_uint32 JobCount /*= 1000*/ )
{
	for (xe_uint32 i = 0; i < ConnectionCount;++i)
	{
		CXEMySqlCon *pCon = new CXEMySqlCon();

		bool bRet = true;
		if (!pCon->Connect(Hostname,Username,Password,DatabaseName,port))
		{
			//连接test服务器
			if (!pCon->Connect(Hostname,Username,Password,"test"))
				return false;

			if (!ExecuteSQL(pCon->m_pMySql,"CREATE DATABASE %s character set utf8",DatabaseName));
				return false;

			pCon->Close();

			bRet =  pCon->Connect(Hostname,Username,Password,DatabaseName,port);		
		}
		if (bRet)
		{
			//		//初始化
			m_VectorCon.push_back(pCon);
		}
		else
		{
			SAFE_DELETE(pCon);
		}

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
	if (pCon)
	{
		return pCon->Reconnect();
	}

	return false;
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
		//CAutoLock cs(&pCon->cs);
		if(mysql_select_db(pCon->m_pMySql,db) == 0)
		{
			pCon->cs.UnLock();
			return true;
		}
	
	}
	if (pCon)
	{
		pCon->cs.UnLock();
	}
	return false;
}

bool CXEMySql::Query( char*cmd,MYSQL_RES *res,...)
{
	CXEMySqlCon *pCon = GetFreeCon();
	if (pCon &&pCon->m_pMySql)
	{

		char szQuery[1024] = {0};

		va_list ap;
		va_start(ap,res);
		vsnprintf(szQuery,1023,cmd,ap);
		va_end(ap);

		//执行
	//	CAutoLock cs(&pCon->cs);

		//执行成功返回0
		if (mysql_real_query(pCon->m_pMySql,szQuery,strlen(szQuery)) == 0)
		{
			res = mysql_store_result(pCon->m_pMySql);

			if (pCon)
			{
				pCon->cs.UnLock();
			}

			return true;
		}
		else
		{
			//输出错误
			printf("cmd:%s errorno:%d,error:%s\n",szQuery, mysql_errno(pCon->m_pMySql),mysql_error(pCon->m_pMySql));

			if (pCon)
			{
				pCon->cs.UnLock();
			}

			return HandleError(pCon,mysql_errno(pCon->m_pMySql));		//部分错误重新连接
		}
	}

	if (pCon)
	{
		pCon->cs.UnLock();
	}

	return false;
}

bool CXEMySql::Query(char*cmd,CXEMySqlResult *data,...)
{
	CXEMySqlCon *pCon = GetFreeCon();
	if (pCon &&pCon->m_pMySql)
	{
		//CAutoLock cs(&pCon->cs);

		char szQuery[1024] = {0};

		va_list ap;
		va_start(ap,data);
		vsnprintf(szQuery,1023,cmd,ap);
		va_end(ap);

		MYSQL_RES *res = NULL;
		if (mysql_real_query(pCon->m_pMySql,szQuery,strlen(szQuery))==0)
		{
			int nLength = mysql_field_count(pCon->m_pMySql);
			if( nLength > 0)			//查看当前查询的列数
			{
				res = mysql_store_result(pCon->m_pMySql);		//返回结果集,没有行数返回NULL;

				if (data)
				{
					data->SetRes(nLength,res);


					if (pCon)
					{
						pCon->cs.UnLock();
					}

					return true;
				}
			}


			if (pCon)
			{
				pCon->cs.UnLock();
			}

			return false;
		}
		else
		{
			//输出错误
			printf("cmd:%s errorno:%d,error:%s\n",szQuery, mysql_errno(pCon->m_pMySql),mysql_error(pCon->m_pMySql));


			if (pCon)
			{
				pCon->cs.UnLock();
			}

			return HandleError(pCon,mysql_errno(pCon->m_pMySql));		//部分错误重新连接
		}
	}


	if (pCon)
	{
		pCon->cs.UnLock();
	}

	return false;
}

CXEMySqlCon * CXEMySql::GetFreeCon()
{
	xe_uint32 nCount = m_VectorCon.size();
	if (nCount == 0)
		return NULL;

	xe_uint32 i = 0;
	for (;;++i)
	{
		CXEMySqlCon *pCon = m_VectorCon.at(i%nCount);
		if (pCon->cs.AttemptLock())		//占用临界区
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

bool CXEMySql::ExecuteSQL( MYSQL *pMySql,char * cmd,... )
{
	if (pMySql)
	{
		char szQuery[1024] = {0};

		va_list ap;
		va_start(ap,cmd);
		vsnprintf(szQuery,1023,cmd,ap);
		va_end(ap);

		MYSQL_RES *res = NULL;
		if (mysql_real_query(pMySql,szQuery,strlen(szQuery))==0)
		{
			return true;
		}
		else
		{
			//输出错误
			printf("cmd:%s errorno:%d,error:%s\n",szQuery, mysql_errno(pMySql),mysql_error(pMySql));
			return false;
		}
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




CXEMySqlCon::CXEMySqlCon()
{
	m_szHostName = "";
	m_szUserName = "";
	m_szPassword = "";
	m_szDatabaseName = "";
	m_nPort = 3306;

	m_pMySql = NULL;
}

CXEMySqlCon::~CXEMySqlCon()
{
	Close();
}

bool CXEMySqlCon::Connect( const char* Hostname,const char* Username, const char* Password, const char* DatabaseName,unsigned int port /*= 3306*/ )
{
	Close();


	m_szHostName = Hostname;
	m_nPort = port;
	m_szUserName = Username;
	m_szPassword = Password;
	m_szDatabaseName = DatabaseName;

	m_pMySql = mysql_init(NULL);		//初始化

	if (m_pMySql == NULL)
	{
		printf("mysql_init failed!\n");
		return false;
	}
	else
	{
		//连接
		if (!mysql_real_connect(m_pMySql,Hostname,Username,Password,DatabaseName,port,NULL,0))
		{
			printf("数据库连接失败,原因如下: %s\n",mysql_error(m_pMySql));
			return false;
		}


		//连接
		if (!m_pMySql)
		{
			mysql_close(m_pMySql);

			return false;
		}

		//设置
		if(mysql_options(m_pMySql, MYSQL_SET_CHARSET_NAME, "utf8"))
		{
			printf("MySQLDatabase Could not set utf8 character set.");
			return false;
		}

		my_bool my_true =true;
		if(mysql_options(m_pMySql, MYSQL_OPT_RECONNECT, &my_true))
		{
			printf("MySQLDatabase MYSQL_OPT_RECONNECT could not be set, connection drops may occur but will be counteracted.");
			return false;
		}
	}

	return true;
}

bool CXEMySqlCon::Close()
{
	if (m_pMySql)
	{
		mysql_close(m_pMySql);
	}

	return true;
}

bool CXEMySqlCon::Reconnect()
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

	if(m_pMySql != NULL)
		mysql_close(m_pMySql);

	m_pMySql= temp;

	return true;
}


