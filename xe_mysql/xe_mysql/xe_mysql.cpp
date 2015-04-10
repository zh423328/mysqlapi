// xe_mysql.cpp : Defines the entry point for the console application.
//简单自己封装的mysql接口,测试接口
///////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "xeMySql.h"

DWORD WINAPI ThreadProc(void *pParam)
{
	while (true)
	{
		char ch = (char)getchar();

		if (ch =='q')
			break;
	}

	return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
	
	//HANDLE hThread = CreateThread(NULL,0,ThreadProc,NULL,0,NULL);

	CXEMySql *pMysql = new CXEMySql();
	pMysql->Initialize("10.10.10.12",3306,"root","123456","yxmr2_chardata",5,1000);


	pMysql->Start();
	
	CXEMySqlResult res;
	DWORD dwTick = GetTickCount();
	for (int i = 0; i < 100;  ++i)
	{
		pMysql->ExecuteQueryNoRet("select * from chardata");
		//if (res.FetchRecords())
		//{
		//	while(res.FetchRow())		//取一行数据
		//	{

		//	}
		//}
	}
	DWORD dwEnd = GetTickCount();

	printf("Use Time:%d\n",dwEnd-dwTick);

	///*pMysql->Stop();*/

	//WaitForSingleObject(hThread,INFINITE);
	
	while (true)
	{
		char ch = (char)getchar();

		if (ch =='q')
			break;
	}

	system("pause");
	return 0;
}

