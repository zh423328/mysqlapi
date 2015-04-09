#include "StdAfx.h"
#include ".\dbclass.h"


CDBClass::CDBClass(void)
{
}

CDBClass::~CDBClass(void)
{
}

BOOL CDBClass::Connect(char *ip,int port,char*db,char*user,char*pwd)
{

	m_sqlCon = new MYSQL;//(MYSQL *)malloc(sizeof(MYSQL));  

	//在某些版本中，不需要该初始化工作，可观看mysql.H以及readme
	mysql_init(m_sqlCon);

	if(m_sqlCon == NULL)
	{
		return FALSE;
	}

	//连接到指定的数据库
	m_sqlCon = mysql_real_connect(m_sqlCon, ip, user, pwd, db, port, NULL, 0);

	if(!m_sqlCon)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CDBClass::SelectDB(char *db)
{
	if (m_sqlCon)
	{
		if(mysql_select_db(m_sqlCon,db)==0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CDBClass::Query(char*cmd,MYSQL_RES *res)
{
	CAutoLock lock(&m_lock);

	if (m_sqlCon)
	{
		if (mysql_real_query(m_sqlCon,cmd,strlen(cmd))==0)
		{
			if(mysql_field_count(m_sqlCon) > 0)
			{
				res = mysql_store_result(m_sqlCon);

				if (res)
				{
					return TRUE;
				}
			}

		}
	}

	return FALSE;
}

void CDBClass::GC(MYSQL_RES *res)
{
	if (res)
		mysql_free_result(res);
}

BOOL CDBClass::GetFirstRow(char*cmd,MYSQL_ROW &row)
{
	CAutoLock lock(&m_lock);

	MYSQL_RES * res = NULL;

	if (Query(cmd,res)==TRUE)
	{
		row = mysql_fetch_row(res);

		if (row)
		{
			return TRUE;
		}
	}
	return FALSE;
}