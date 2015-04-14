#ifndef _DEBUG_DEFINE_H_
#define _DEBUG_DEFINE_H_

#define _WIN32_WINDOWS 0x0500

#include <crtdbg.h>
#include <stdio.h>
#include "Windows.h"
#include <malloc.h>
#define  _NO_CVCONST_H

/*
 *	VC6.0需要安装最新的Platform SDK － Debugging Tools for Windows
 */

//公用函数
void*	memdup(const void* ptr, int size); 
int		DumpCallStack(LPEXCEPTION_POINTERS pException, DWORD code);//GetExceptionInformation(), GetExceptionCode()
void    strcpy(char* des, const char* src, int size);//add by dxx    strcpy函数重载

//安装自己的异常处理器
void	InstallMyExceptHandle();
LONG WINAPI MyExceptHandle(struct _EXCEPTION_POINTERS *ExceptionInfo);

//公用宏
#define CHECK_PTR(p) 			(IsBadReadPtr(p, sizeof(void*)) == 0 ? TRUE : FALSE)
#define CHECK_WRITE_PTR(p, n)	(IsBadWritePtr(p, n) == 0 ? TRUE : FALSE)
#define DUMP_CALLSTACK()        DumpCallStack(GetExceptionInformation(), GetExceptionCode())
#define CLOSE_PROCESS()			{DUMP_STRING_TIME("CLOSE_PROCESS\n");ExitProcess(0);}
#define MEMZERO(p)				memset(p, 0, sizeof(p))
#define STRNCPY(d, s, n)        {strncpy(d, s, n); d[n-1] = 0;}

#define DUMP_MEMORY(p,n)		::DumpMemory(p, n)
#define DUMP_MEMORY_TIME(p,n)	::DumpMemory(p, n, TRUE)
#define DUMP_STRING(str)		DUMP_MEMORY(str, strlen(str))
#define DUMP_STRING_TIME(str)	DUMP_MEMORY_TIME(str, strlen(str))
#define CHECK_MEMORY(p, n)		::CheckHeap(p, n)

BOOL DumpMemory(LPVOID lpVoid, int nSize, BOOL timeStamp=FALSE);
BOOL CheckHeap(LPVOID lpVoid, int nSize);

#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

//DEBUG
#ifdef _DEBUG
	#define ASSERT(f)	\
						do		\
						{		\
							if (!(f) && _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, 0, 0)) \
							__asm { int 3 }	\
						} while (0)	

	void Trace(char* lpszFormat, ...);
	
	#define OUT_STRING(s)			::OutputDebugString(s)
	#define TRACE					::Trace
#else//release
	#define ASSERT(f)			((void)0)
	#define OUT_STRING(s)		((void)0)
	#define TRACE				((void)0)
#endif	//_DEBUG

#ifdef _DEBUG
#define RELEASE_TRY_BEGIN(a) {a}
#define RELEASE_TRY_END(a) {}
#else
#define RELEASE_TRY_BEGIN(a) TRY_BEGIN(a)
#define RELEASE_TRY_END(a) TRY_END(a)
#endif

#define TRY_BEGIN(a) __try {a} 
#define TRY_END(a) __except(DUMP_CALLSTACK()) {a}

#endif	//_DEBUG_DEFINE_H_