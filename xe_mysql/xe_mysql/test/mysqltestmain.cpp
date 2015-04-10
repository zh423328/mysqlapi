#include <windows.h>
#include "MySqlDB.h"
#include <STDIO.H>
#include <tchar.h>
void ErrorDump(const char* szError,void* p)
{
	printf("%s\n",szError);
}

DWORD WINAPI ThreadProc(LPVOID pParam)
{
	printf("ThreadProc\n");
	MySqlDB* pDB = (MySqlDB*)pParam;
	MySqlQuery query(pDB);
	DWORD dwTick = GetTickCount();
	for(int i = 0; i < 100; i ++)
	{
		query.ExecuteSQL("select SELECT name, level, exp, sex, hp, maxHp, mp,maxMp,country,job,WXjmsht from chardata where uin = 16");
		bool rlt = query.FetchRecords(true);
		if(rlt)
		{
			bool rlt = query.FetchRow();
			while(rlt)
			{
				char* szName = query.GetStr("name");
				if(szName != NULL)
					printf("Time:%d Name:%s\n",i,szName);
				rlt = query.FetchRow();
			}
		}
	}
	printf("Use time:%d\n",GetTickCount() - dwTick);
	return 0;
}

#define Thread_Cnt	3
void main()
{
	bool rlt = IsThreadSafe();
	MySqlDB db[Thread_Cnt];
	HANDLE hThread[Thread_Cnt];
	DWORD dwThreadID = 0;
	for(int i = 0; i < Thread_Cnt; i++)
	{
		db[i].SetCallBackFunc(ErrorDump,NULL);
		if(!db[i].Connect("192.168.1.254","chardb","root","123456",3306))
			printf("%d:connect to dbase failed!\n",i+1);
		else
			printf("%d:connect to dbase ok1\n",i+1);
		
	}
	for(i = 0; i < Thread_Cnt; i++)
		hThread[i] = CreateThread(NULL,0,ThreadProc,&db[i],0,&dwThreadID);
	getchar();
}