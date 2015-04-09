#pragma once
#include <winsock2.h>
#include "mysql.h"

#ifdef _DLL_EXPORT
#define _MY_DLL _declspec(dllexport)
#else
#define _MY_DLL _declspec(dllimport)
#endif

#include "Lock.h"

class _MY_DLL CDBClass
{
public:
	CDBClass(void);
	~CDBClass(void);
public:
	BOOL Connect(char *ip,int port,char*db,char*user,char*pwd);

	BOOL SelectDB(char *db);

	BOOL Query(char*cmd,MYSQL_RES *res);

	void GC(MYSQL_RES *res);

	BOOL GetFirstRow(char*cmd,MYSQL_ROW&row);

private:
	MYSQL *m_sqlCon;

	CLock m_lock;
};
