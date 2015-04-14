// xe_mysql.cpp : Defines the entry point for the console application.
//���Լ���װ��mysql�ӿ�,���Խӿ�
///////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "xeMySql.h"
#include "log.h"
#include "crashhandle.h"

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

//����̨�¼�
BOOL CtrlHandler( DWORD fdwCtrlType ) 
{ 
	switch( fdwCtrlType ) 
	{ 
		// Handle the CTRL-C signal. 
	case CTRL_C_EVENT: 
		return TRUE;

		// CTRL-CLOSE: confirm that the user wants to exit. 
	case CTRL_CLOSE_EVENT: 
		{
			sLog.Close();		//����ֱ�ӹرյĻ�������������
		}
		return TRUE;

		// Pass other signals to the next handler. 
	case CTRL_BREAK_EVENT: 
		return FALSE; 

	case CTRL_LOGOFF_EVENT: 
		return FALSE; 

	case CTRL_SHUTDOWN_EVENT: 
		return FALSE; 
	default: 
		return FALSE; 
	} 
} 


int _tmain(int argc, _TCHAR* argv[])
{
	//����
	DWORD address1, address2;
	__asm mov address1, offset begindecrypt;
	__asm mov address2, offset enddecrypt;

begindecrypt:
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler,true);		//ע��һ������

	//InstallMyExceptHandle();

	CXEMySql *pMysql = new CXEMySql();
	bool bRet = pMysql->Initialize("127.0.0.1","root","123456","ioslz_chardata",3306,5,1000);

	if (bRet)
	{
		pMysql->Start();

		CXEMySqlResult res;
		DWORD dwTick = GetTickCount();
		for (int i = 0; i < 2;  ++i)
		{
			pMysql->Query("select * from chardata",&res);
		}
		DWORD dwEnd = GetTickCount();

		printf("Use Time:%d\n",dwEnd-dwTick);
	}


	int *a = NULL;
	*a = 1;
	///*pMysql->Stop();*/
	
	while (true)
	{
		char ch = (char)getchar();

		if (ch =='q')
		{
			pMysql->Stop();
			break;
		}
	}

	system("pause");
enddecrypt:
	return 0;
}

