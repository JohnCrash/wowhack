#include "stdafx.h"
#include "callhook.h"
#include "wowentry.h"
#include "dllentry.h"
#include "wowin.h"
#include "anticheck.h"
#include "scout.h"
#include "movement.h"
#include "collide.h"
#include "render.h"
#include "soundnote.h"
#include <tlhelp32.h>
#include <dbghelp.h>

extern const char* sWow;
const char* sKernel = "kernel32";
const char* sKernelDll = "kernel32.dll";
const char* sUser = "user32.dll";
const char* sUserDll = "user32.dll";
bool b_castFocus = false;
long long b_oldFocusID = 0;
int hookType;

HWND GetWowWnd();
WNDPROC GetWowWndProc();

typedef HMODULE (WINAPI *t_LoadLibraryA)(LPCTSTR);
typedef HMODULE	(WINAPI *t_LoadLibraryExA)(LPCTSTR,HANDLE,DWORD);
typedef PROC	(WINAPI	*t_GetProcAddressA)(HMODULE,LPCSTR);
typedef HANDLE	(WINAPI *t_CreateThread)(LPSECURITY_ATTRIBUTES lpThreadAttributes,
  SIZE_T dwStackSize,
  LPTHREAD_START_ROUTINE lpStartAddress,
  LPVOID lpParameter,
  DWORD dwCreationFlags,
  LPDWORD lpThreadId);
typedef BOOL	(WINAPI	*t_GetThreadContext)(HANDLE hThread,LPCONTEXT lpContext);
typedef BOOL	(WINAPI	*t_FreeLibrary)(HMODULE);
typedef BOOL	(WINAPI *t_Module32First)(HANDLE,LPMODULEENTRY32);
typedef BOOL	(WINAPI *t_Module32Next)(HANDLE,LPMODULEENTRY32);
typedef BOOL	(WINAPI *t_EnumWindows)(WNDENUMPROC,LPARAM);

PROC pfnLoadLibraryA;
PROC pfnLoadLibraryExA;
PROC pfnGetProcAddressA;
PROC pfnCreateThread;
PROC pfnGetThreadContext;
PROC pfnFreeLibrary;
PROC pfnModule32First;
PROC pfnModule32Next;
PROC pfnEnumWindows;

HMODULE hScanDLL = NULL;
//这里隐藏对硬件中断的修改
BOOL WINAPI mypfnGetThreadContext(HANDLE hThread,LPCONTEXT lpContext){
	printf("GetThreadContext call\n");
	BOOL retb=((t_GetThreadContext)pfnGetThreadContext)(hThread,lpContext);
	lpContext->Dr0 = 0;
	lpContext->Dr7 = 0;
	return retb;
}

//跟踪LoadLibrary对Scan.dll安装hook
HMODULE  WINAPI mypfnLoadLibraryA( LPCTSTR lpFileName ){
	HMODULE hModule = ((t_LoadLibraryA)pfnLoadLibraryA)( lpFileName );
	printf( "LoadLibraryA %s %X %X\n",lpFileName,mypfnLoadLibraryA,pfnLoadLibraryA);
	if( strcmp(lpFileName,".\\Scan.dll")==0 || strcmp(lpFileName,".\\Scan.dll.new")==0
		||strcmp(lpFileName,"Scan.dll")==0 || strcmp(lpFileName,"Scan.dll.new")==0){
		Log( "LoadLibrary Scan.dll" );
		hScanDLL = hModule;
		HookToolhelp32Snapshot( hModule );
		ClearAllHook();
	}
	return hModule;
}

//同上，另外以W结尾的没有加是一个漏洞
HMODULE  WINAPI mypfnLoadLibraryExA( LPCTSTR lpFileName,HANDLE hFile,DWORD dwFlags ){
	HMODULE hModule = ((t_LoadLibraryExA)pfnLoadLibraryExA)( lpFileName,hFile,dwFlags );
	printf( "LoadLibraryExA %s %X %X\n",lpFileName,mypfnLoadLibraryA,pfnLoadLibraryA);
	if( strcmp(lpFileName,".\\Scan.dll")==0 || strcmp(lpFileName,".\\Scan.dll.new")==0
		||strcmp(lpFileName,"Scan.dll")==0 || strcmp(lpFileName,"Scan.dll.new")==0){
		Log( "LoadLibraryEx Scan.dll" );
		hScanDLL = hModule;
		HookToolhelp32Snapshot( hModule );
		ClearAllHook();
	}
	return hModule;
}

BOOL WINAPI mypfnFreeLibrary( HMODULE hModule ){
	if( hModule == hScanDLL ){
		printf("FreeLibrary Scan.DLL\n");
		SetAllHook();
		Log( "FreeLibrary Scan.dll" );
	}
	return ((t_FreeLibrary)pfnFreeLibrary)( hModule );
}

typedef DWORD (WINAPI *t_ThreadProc)(LPVOID lpParameter);

struct ThreadWrap{
	t_ThreadProc proc;
	LPVOID	   param;
	ThreadWrap( t_ThreadProc p,LPVOID pp ):proc(p),param(pp){}
};

//隐藏对输入函数的地址,这里简单打印
FARPROC WINAPI mypfnGetProcAddressA(HMODULE hModule,LPCSTR lpProcName ){
	printf("GetProcAddressA %s\n",lpProcName );
	return ((t_GetProcAddressA)pfnGetProcAddressA)( hModule,lpProcName );
}

extern DWORD g_isHook;

DWORD WINAPI myThreadProc( LPVOID lpParameter ){
	ThreadWrap* pwrap = (ThreadWrap*)lpParameter;
//	printf("myThreadProc %d\n",GetCurrentThreadId() );
//	if( SetHardBreakPoint( GetCurrentThread(),g_isHook>0?TRUE:FALSE ) ){
//		printf("SetHardBreakpoint succues!\n");
//	}else{
//		printf("SetHardBreakpoint fail!\n");
//	}
	DWORD ret = ((t_ThreadProc)pwrap->proc)( pwrap->param );
//	delete pwrap;
//	printf("myThreadEnd\n");
	return ret;
}

//跟踪所有wow.exe和scan.dll中的CreateThread创建的进程,并且给他们设置硬件断点

HANDLE WINAPI mypfnCreateThread(  LPSECURITY_ATTRIBUTES lpThreadAttributes,
  SIZE_T dwStackSize,
  LPTHREAD_START_ROUTINE lpStartAddress,
  LPVOID lpParameter,
  DWORD dwCreationFlags,
  LPDWORD lpThreadId
  ){
	//  LPVOID lpParam = (LPVOID)new ThreadWrap( lpStartAddress,lpParameter );
	  HANDLE hThread = ((t_CreateThread)pfnCreateThread)(lpThreadAttributes,dwStackSize,lpStartAddress,lpParameter,dwCreationFlags,lpThreadId);
	//  printf("CreateThread %d CurrentThreadId %d\n",*lpThreadId,GetCurrentThreadId());
	 SetHardBreakPoint( *lpThreadId,(g_isHook==1?TRUE:FALSE) );
	 return hThread;
}

//用来隐藏wowin.dll
BOOL WINAPI mypfnModule32First(HANDLE hSnapshot,LPMODULEENTRY32 lpme){
	BOOL ret = ((t_Module32First)pfnModule32First)(hSnapshot,lpme);
	if( strcmp(lpme->szModule,"wowin.dll")==0 ){
		printf("Module32First %s\n",lpme->szModule );
		ret = ((t_Module32First)pfnModule32Next)(hSnapshot,lpme);
	}
	return ret;
}
//用来隐藏wowin.dll
BOOL WINAPI mypfnModule32Next(HANDLE hSnapshot,LPMODULEENTRY32 lpme){
	BOOL ret = ((t_Module32Next)pfnModule32Next)(hSnapshot,lpme);
	if( strcmp(lpme->szModule,"wowin.dll")==0 ){
		printf("Module32Next %s\n",lpme->szModule );
		ret = ((t_Module32First)pfnModule32Next)(hSnapshot,lpme);
	}
	return ret;
}

//隐藏控制台窗口
typedef BOOL (WINAPI *t_EnumWindowsFunc)( HWND ,LPARAM );

struct EnumWnd{
	t_EnumWindowsFunc enumfunc;
	LPARAM	lparam;
	EnumWnd( t_EnumWindowsFunc func,LPARAM param ):enumfunc(func),lparam(param){}
};

BOOL WINAPI myEnumWindowsFunc( HWND hWnd,LPARAM lParam ){
	EnumWnd* pe = (EnumWnd*)lParam;
	char buf[128];
	GetWindowText( hWnd,buf,128 );
	if( strcmp( buf,"wow.exe" )==0 ){
		printf("Anti Check windows title : wow.exe\n" );
		return TRUE;
	}
	return pe->enumfunc( hWnd,lParam );
}

BOOL WINAPI mypfnEnumWindows(WNDENUMPROC lpEnumFunc,LPARAM lParam){
	EnumWnd ew( lpEnumFunc,lParam );
	return ((t_EnumWindows)pfnEnumWindows)( myEnumWindowsFunc,(LPARAM)(&ew) );
}

//屏蔽Module32Next
void HookToolhelp32Snapshot( HMODULE hmodCaller ){
	//替换LoadLibraryA
	pfnLoadLibraryA = GetProcAddress(GetModuleHandle(sKernel),"LoadLibraryA");
	if( pfnLoadLibraryA )
		RelpaceEntry(sKernelDll,pfnLoadLibraryA,(PROC)mypfnLoadLibraryA,hmodCaller);
	else
		printf("Can't find LoadLibraryA function in %s\n",sKernelDll );

	pfnLoadLibraryExA = GetProcAddress(GetModuleHandle(sKernel),"LoadLibraryExA");
	if( pfnLoadLibraryExA )
		RelpaceEntry(sKernelDll,pfnLoadLibraryExA,(PROC)mypfnLoadLibraryExA,hmodCaller);
	else
		printf("Can't find LoadLibraryExA function in %s\n",sKernelDll );

	pfnFreeLibrary = GetProcAddress(GetModuleHandle(sKernel),"FreeLibrary");
	if( pfnFreeLibrary )
		RelpaceEntry(sKernelDll,pfnFreeLibrary,(PROC)mypfnFreeLibrary,hmodCaller);
	else
		printf("Can't find pfnFreeLibrary function in %s\n",sKernelDll );

	pfnGetProcAddressA = GetProcAddress(GetModuleHandle(sKernel),"GetProcAddress");
	if( pfnGetProcAddressA )
		RelpaceEntry(sKernelDll,pfnGetProcAddressA,(PROC)mypfnGetProcAddressA,hmodCaller);
	else
		printf("Can't find GetProcAddress function in %s\n",sKernelDll );

	pfnCreateThread = GetProcAddress(GetModuleHandle(sKernel),"CreateThread");
	if( pfnCreateThread )
		RelpaceEntry(sKernelDll,pfnCreateThread,(PROC)mypfnCreateThread,hmodCaller);
	else
		printf("Can't find CreateThread function in %s\n",sKernelDll );

	pfnModule32First = GetProcAddress(GetModuleHandle(sKernel),"Module32First");
	if( pfnModule32First )
		RelpaceEntry(sKernelDll,pfnModule32First,(PROC)mypfnModule32First,hmodCaller);
	else
		printf("Can't find Module32First function in %s\n",sKernelDll );

	pfnModule32Next = GetProcAddress(GetModuleHandle(sKernel),"Module32Next");
	if( pfnModule32Next )
		RelpaceEntry(sKernelDll,pfnModule32Next,(PROC)mypfnModule32Next,hmodCaller);
	else
		printf("Can't find Module32Next function in %s\n",sKernelDll );

	pfnEnumWindows = GetProcAddress(GetModuleHandle(sUser),"EnumWindows");
	if( pfnEnumWindows )
		RelpaceEntry(sUserDll,pfnEnumWindows,(PROC)mypfnEnumWindows,hmodCaller);
	else
		printf("Can't find EnumWindows function in %s\n",sUserDll );
}

//Log EceptionRecord
//将异常码写入日志文件
void LogEceptionRecord( PEXCEPTION_RECORD pException ){
	char buf[128];
	sprintf(buf,"ExceptionCode:%X,ExceptionFlags:%X",pException->ExceptionCode,pException->ExceptionFlags );
	Log( buf );
}

//将关键寄存器保存起来
void LogEceptionContext(PCONTEXT pContext){
	char buf[256];
	sprintf(buf,"eip:%X,ebp:%X,esp:%X,dr0:%X,dr1:%X,dr2:%X,dr3:%X,dr6:%X,dr7:%X",
		pContext->Eip,pContext->Esp,pContext->Ebp,
		pContext->Dr0,pContext->Dr1,pContext->Dr2,pContext->Dr3,pContext->Dr6,pContext->Dr7);
	Log( buf );
}

void LogStack( LPDWORD lpstack ){
	char buf[512];
	char temp[128];
	memset( buf,0,sizeof(buf) );
	memset( temp,0,sizeof(temp) );
	for( int i = 0;i < 32;i++ ){
		sprintf( temp,"%08X ",*lpstack );
		strcat( buf,temp );
		lpstack++;
	}
	Log( buf );
}
//这里写一个简单的堆栈回溯函数
//ebp是函数的堆栈基地址,函数开始一般就是push ebp,move ebp,esp
//这样ebp
/*
void StackTrac(int nMaxFrames){
	STACKFRAME64 frame;
	int nCount = 0;
	CONTEXT Context;

	Context.ContextFlags = CONTEXT_ALL;
	GetThreadContext(GetCurrentThread(),&Context);
	memset( &frame,0x0,sizeof(frame));
	frame.AddrPC.Offset = Context.Eip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = Context.Ebp;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Offset = Context.Esp;
	frame.AddrStack.Mode = AddrModeFlat;
	while( nCount < nMaxFrames ){
		if( !StackWalk64( IMAGE_FILE_MACHINE_I386,GetCurrentProcess(),GetCurrentThread(),
			&frame,&Context,NULL,SymFunctionTableAccess64,SymGetModuleBase64,NULL ) ){
			printf("StackWalk64 fail\n");
			break;
		}
		printf("return %X\n",frame.AddrReturn.Offset );
		if( frame.AddrFrame.Offset == 0 || frame.AddrReturn.Offset == 0 ){
			break;
		}
	}
}
*/

//下面代码加入访问硬件中断，可以导致wow.exe crash

void SetHardBreakPoint( DWORD ThreadId,BOOL b ){
	CONTEXT cxt;
#ifndef _HARDBREAK_
	HANDLE hThread = OpenThread( THREAD_SET_CONTEXT|THREAD_GET_CONTEXT|THREAD_SUSPEND_RESUME,FALSE,ThreadId );
	if( hThread ){
		cxt.ContextFlags=CONTEXT_DEBUG_REGISTERS;
		if( GetThreadContext(hThread,&cxt) ){
			if( b ){
				//设置硬件断点
				cxt.Dr0=((DWORD)g_chCheckHash.pEntryFunc);
				cxt.Dr7=0xf0001;
			}else{
				//清除硬件断点
				cxt.Dr0=0;
				cxt.Dr7=0;
			}
			//在设置线程上下问的时候先挂起该线程,如果是就是当前线程则直接设置
			printf( "HardPoint:%X ",ThreadId );
			if( GetCurrentThreadId() == ThreadId ){
				SetThreadContext(hThread,&cxt);
			}else{
				if( SuspendThread( hThread ) != (DWORD)(-1) ){
					SetThreadContext(hThread,&cxt);
					ResumeThread( hThread );
				}
			}
			GetThreadContext(hThread,&cxt);
			printf("Dr0:%X Dr7:%X\n",cxt.Dr0,cxt.Dr7 );
		}
		CloseHandle( hThread );
	}
	#endif
}

void AllThreadHardBreakPoint(BOOL b){
	#ifndef _HARDBREAK_
	/*
	MODULEENTRY32 me32;

	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,GetCurrentProcessId());
	if( hModuleSnap == INVALID_HANDLE_VALUE ){
		printf( "hModuleSnap = INVALID_HANDLE_VALUE\n" );
		return;
	}
	me32.dwSize = sizeof(MODULEENTRY32);
	if( Module32First(hModuleSnap,&me32) ){
		do{
			printf("%s\n",me32.szExePath);
		}while( Module32Next(hModuleSnap,&me32) );
	}
	CloseHandle(hModuleSnap);
	*/
	//这里枚举线程
	THREADENTRY32 th32;
	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,GetCurrentProcessId());
	if( hThreadSnap == INVALID_HANDLE_VALUE ){
		printf( "hThreadSnap = INVALID_HANDLE_VALUE\n" );
		return;
	}
	th32.dwSize = sizeof(THREADENTRY32);
	if( Thread32First(hThreadSnap,&th32) ){
		do{
			if( GetCurrentProcessId() == th32.th32OwnerProcessID ){
				SetHardBreakPoint( th32.th32ThreadID,b );
			}
		}while( Thread32Next(hThreadSnap,&th32) );
	}
	CloseHandle(hThreadSnap);
	#endif
}

//简单测试代码
BOOL CALLBACK myFunc( HWND hWnd, LPARAM lParam ){
	char str[128];
	
	GetWindowText( hWnd,str,128 );
	if( strcmp( str,"wow.exe" ) == 0 ){
		printf("-----------------------------------------\n");
		printf("Find Windows: %s\n",str );
	}
	return TRUE;
}

void TestAntiCheck(){
	EnumWindows( myFunc,NULL );
}

//================================================
//下面代码加入访问硬件中断，可以导致wow.exe crash
BOOL SetBreakPoint( DWORD ThreadId,int dr,DWORD pAddress,BOOL b ){
	CONTEXT cxt;

	HANDLE hThread = OpenThread( THREAD_SET_CONTEXT|THREAD_GET_CONTEXT|THREAD_SUSPEND_RESUME,FALSE,ThreadId );
	if( hThread ){
		cxt.ContextFlags=CONTEXT_DEBUG_REGISTERS;
		if( GetThreadContext(hThread,&cxt) ){
			switch(dr){
				case 0:
					cxt.Dr0 = pAddress;
					if( b )
						cxt.Dr7 |= 1;
					else
						cxt.Dr7 &= 0xfffffffe;
					break;
				case 1:
					cxt.Dr1 = pAddress;
					if( b )
						cxt.Dr7 |= 4;
					else
						cxt.Dr7 &= 0xfffffffb;
					break;
				case 2:
					cxt.Dr2 = pAddress;
					if( b )
						cxt.Dr7 |= 16;
					else
						cxt.Dr7 &= 0xffffffef;
					break;
				case 3:
					cxt.Dr3 = pAddress;
					if( b )
						cxt.Dr7 |= 64;
					else
						cxt.Dr7 &= 0xffffffbf;
					break;
			}
			//在设置线程上下问的时候先挂起该线程,如果是就是当前线程则直接设置
			if( GetCurrentThreadId() == ThreadId ){
				//错误不能对正在运行的线程设置上下问
				//Do not try to set the context for a running thread; 
				//the results are unpredictable. 
				//Use the SuspendThread function to suspend the thread before calling SetThreadContext.
				//SetThreadContext(hThread,&cxt);
				CloseHandle( hThread );
				return FALSE;
			}else{
				if( SuspendThread( hThread ) != (DWORD)(-1) ){
					SetThreadContext(hThread,&cxt);
					ResumeThread( hThread );
				}
			}
			GetThreadContext(hThread,&cxt);
		}
		CloseHandle( hThread );
		return TRUE;
	}
	return FALSE;
}

extern WNDPROC ex_wowProc;
//开始对CreateGXInterface函数设置执行中断位置Dr0
void SetStartHardBreakPoint( DWORD th32ThreadID ){
	SetBreakPoint( th32ThreadID,0,(DWORD)g_chSwapBuff.pEntryFunc,TRUE );
	SetBreakPoint( th32ThreadID,1,(DWORD)g_chScriptEntry.pEntryFunc,TRUE );
}

PROC pGetThreadContext = NULL;
PROC pAddVectoredExceptionHandler = NULL;
BOOL SetHardBreakWowWnd();
//对GetThreadContext,AddVectoredExceptionHandler设置硬件断点
//来简单反检测，他可以防止wow通过GetThreadContext检测硬件断点
//防止wow通过AddVectoredExceptionHandler来检查向量中断
//事实上这是不完整的方案,还有一些函数需要跟踪
void SetStartHardBreakPointForAnitCheck( DWORD th32ThreadID ){
	pGetThreadContext = GetProcAddress(GetModuleHandle("kernel32.dll"),"GetThreadContext");
	if( !pGetThreadContext ){
#ifdef _OUT
		printf("GetProcAddress GetThreadContext fail\n");
#endif
	}
	pAddVectoredExceptionHandler = GetProcAddress(GetModuleHandle("kernel32.dll"),"AddVectoredExceptionHandler");
	if( !pAddVectoredExceptionHandler ){
#ifdef _OUT
		printf("GetProcAddress AddVectoredExceptionHandler fail\n");
#endif
	}
	if( pAddVectoredExceptionHandler )
		SetBreakPoint( th32ThreadID,3,(DWORD)pAddVectoredExceptionHandler,TRUE );
//	if( pGetThreadContext )
//		SetBreakPoint( th32ThreadID,2,(DWORD)pGetThreadContext,TRUE );
}

HANDLE g_restoreH = NULL;

DWORD WINAPI setupBreakProc( LPVOID pvoid ){
	#ifdef _OUT
		printf("setupBreakProc run\n");
	#endif
	Sleep(2000); //具体多长时间这个很难说
	SetMainThreadHardBreakPoint();
	#ifdef _OUT
		printf("Restore breakpoint\n");
	#endif
	g_restoreH = NULL;
	return 0;
}

void CreateDelayThread(){
	if( g_restoreH==NULL ){
		DWORD tid;
		#ifdef _OUT
			printf("CreateDelayThread\n");
		#endif
		g_restoreH = CreateThread(NULL,0,setupBreakProc,0,0,&tid);
	}
}
//对线程设置硬件断点
void SetMainThreadHardBreakPoint(){
	THREADENTRY32 th32;
	int nCount = 0;
	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,GetCurrentProcessId());
	if( hThreadSnap == INVALID_HANDLE_VALUE ){
#ifdef _OUT
		printf( "hThreadSnap = INVALID_HANDLE_VALUE\n" );
#endif
		return;
	}
	th32.dwSize = sizeof(THREADENTRY32);
	if( Thread32First(hThreadSnap,&th32) ){
		do{
			if( GetCurrentProcessId() == th32.th32OwnerProcessID ){
				if( nCount==0 ){
					SetStartHardBreakPoint( th32.th32ThreadID );
					SetStartHardBreakPointForAnitCheck( th32.th32ThreadID );
				}
				nCount++;
			}
		}while( Thread32Next(hThreadSnap,&th32) );
	}
	CloseHandle(hThreadSnap);
}

WNDPROC ex_wowProc = NULL;
UINT  vkey[5];
BOOL wsad[5];
BOOL bStop = FALSE;

void PostKeyUpForStop(){
	WPARAM wparam;
	LPARAM lparam,lp;
	HWND hwnd = GetWowWnd();
	if( hwnd && IsWindow(hwnd) ){
		for( int i =0;i<5;i++ ){
			if( wsad[i] ){
				wparam = vkey[i];
				lp = wparam;
				lp <<= 16;
				lparam = 1;
				lparam |= lp;
				lparam |= 0xc0000000;
				PostMessage(hwnd,WM_KEYUP,wparam,lparam);
			}
		}
	}
}

/* 下面函数监视wsad键，用来停止移动
*/
BOOL myProc(UINT uMsg,WPARAM wParam,LPARAM lParam){
	if( uMsg==WM_KEYDOWN || uMsg==WM_KEYUP ){
		int i;
		BOOL b = FALSE;
		BOOL bContinue = FALSE;
		for( i = 0;i < 5;i++ ){
			if( (UINT)wParam == vkey[i] ){
				bContinue = TRUE;
			}
		}
		if( !bContinue )return FALSE;

		if( uMsg==WM_KEYDOWN ){
			for( i = 0;i < 5;i++ ){
				if( (UINT)wParam == vkey[i] ){
					//按键抬起&&第一次按下
					if( !wsad[i] && !(lParam&0x40000000) && !bStop ){
						wsad[i] = TRUE;
					//按键抬起&&中间按下
					}else if( !wsad[i] && (lParam&0x40000000) ){
						b = TRUE;
					}
				}
			}
			if( b||bStop )return TRUE;
		}else if( uMsg==WM_KEYUP ){
			for( i = 0;i < 5;i++ ){
				if( (UINT)wParam==vkey[i] ){
					if( wsad[i] )
						wsad[i] = FALSE;
					else
						b = TRUE;
				}
			}
			//避免出现两次抬起的情况
			if( b )return TRUE;
		}
	}
	return FALSE;
}

DWORD WINAPI setupProc( LPVOID pvoid ){
	SetBreakPoint( (DWORD)pvoid,3,(DWORD)ex_wowProc,TRUE );
	return 0;
}

extern DWORD WINAPI subClassProc( LPVOID pvoid );

//设置断点,对窗口过程函数
BOOL SetHardBreakWowWnd(){
	if( hookType==1 || hookType==3 ){
		ex_wowProc = GetWowWndProc();

		wsad[0] = FALSE;
		wsad[1] = FALSE;
		wsad[2] = FALSE;
		wsad[3] = FALSE;
		wsad[4] = FALSE;

		vkey[0] = 'W';
		vkey[1] = 'S';
		vkey[2] = 'A';
		vkey[3] = 'D';
		vkey[4] = VK_SPACE;

		//只有硬件断点类型的挂钩才可以使用这个功能
		if( !ex_wowProc )
			return FALSE;

		bStop = FALSE;
		DWORD tid;
		HANDLE h;
		if( hookType==1 )
			h = CreateThread(NULL,0,setupProc,(LPVOID)GetCurrentThreadId(),0,&tid);
		else
			h = CreateThread(NULL,0,subClassProc,(LPVOID)GetCurrentThreadId(),0,&tid);
		if( h ){
			return TRUE;
		}else{
			ex_wowProc = NULL;
			return FALSE;
		}
	}
	return FALSE;
}

HWND g_hWowWnd = NULL;

BOOL CALLBACK EnumProc( HWND hWnd,LPARAM lp ){
	char TiTle[64];
	if( GetWindowText(hWnd,TiTle,64) ){
		DWORD id = 0;
		GetWindowThreadProcessId(hWnd,&id);
		if(strcmp(TiTle,sWowTitle)==0&&GetCurrentProcessId()==id){
			g_hWowWnd = hWnd;
			return FALSE;
		}
	}
	return TRUE;
}

//取魔兽世界主窗口
HWND GetWowWnd(){
	if( g_hWowWnd && IsWindow(g_hWowWnd) )
		return g_hWowWnd;
	EnumWindows( EnumProc,NULL );
	return g_hWowWnd;
}

//取窗口过程函数入口
WNDPROC GetWowWndProc(){
	HWND hwnd = GetWowWnd();
	if( hwnd ){
//		return (WNDPROC)GetWindowLongPtr(hwnd,GWLP_WNDPROC);
		char className[64];
		WNDCLASS wndClass;
		GetClassName( hwnd,className,64 );
		GetClassInfo(GetModuleHandle(NULL),className,&wndClass );
		
#ifdef _OUT
		printf("Wnd:%x\n",hwnd);
		printf("ClassName:%s\n",wndClass.lpszClassName);
		printf("WndProc:%x\n",wndClass.lpfnWndProc);
		printf("GetWindowLongPtr:%x\n",GetWindowLongPtr(hwnd,GWLP_WNDPROC));
#endif
		return wndClass.lpfnWndProc;
	}
	return NULL;
}

//0 停止移动 1 继续移动
int lua_Stop( void* p ){
	int t;
	if( Lua_isnumber(p,1) ){
		t = (int)Lua_tonumber(p,1);
		if( t==0 ){
			if( !bStop ){
				bStop = TRUE;
				PostKeyUpForStop();
			}
			Lua_pushnumber(p,0);
			return 1;
		}else if( t==1 ){
			bStop = FALSE;
			Lua_pushnumber(p,1);
			return 1;
		}else if( t==2 ){
			//初始化
			if( SetHardBreakWowWnd() ){
				Lua_pushnumber(p,2);
				return 1;
			}
		}
	}
	Lua_pushnil(p);
	return 1;
}

int lua_Click( void* p ){
	int x,y;
	bool left = true;
	bool isok = true;
	if( Lua_isnumber(p,1) ){
		x =  (int)Lua_tonumber(p,1);
	}else isok = false;
	if( Lua_isnumber(p,2) ){
		y =  (int)Lua_tonumber(p,2);
	}else isok = false;
	if( Lua_isnumber(p,3) ){
		left = false;
	}
	if( isok ){
		HWND hWnd = GetWowWnd();
		if( hWnd&&IsWindow(hWnd) ){
			WPARAM wParam;
			LPARAM lParam;
			POINT spt;
			BOOL b;
			lParam = (LPARAM)y;
			lParam <<= 16;
			lParam |=  (LPARAM)x;
			spt.x = x;
			spt.y =y;
			ClientToScreen(hWnd,&spt);
			SetCursorPos( spt.x,spt.y );
			SetCursorPos( spt.x,spt.y );
			wParam = 0;
			PostMessage( hWnd,WM_MOUSEMOVE,wParam,lParam );
			PostMessage( hWnd,WM_MOUSEMOVE,wParam,lParam );
			
			if( left ){
				wParam = MK_LBUTTON;
				PostMessage( hWnd,WM_LBUTTONDOWN,wParam,lParam );
			}else{
				wParam = MK_RBUTTON;
				PostMessage( hWnd,WM_RBUTTONDOWN,wParam,lParam );
			}
			wParam = 0;
			PostMessage( hWnd,WM_MOUSEMOVE,wParam,lParam );
			if( left )
				PostMessage( hWnd,WM_LBUTTONUP,wParam,lParam );
			else
				PostMessage( hWnd,WM_RBUTTONUP,wParam,lParam );
			Lua_pushnumber(p,1);
			return 1;
		}
	}
	Lua_pushnil(p);
	return 1;
}

int lua_PushKey( void* p ){
	unsigned char ScanCode;

	if( Lua_isstring(p,1) ){
		size_t len;
		WPARAM wparam;
		LPARAM lparam,lp;
		const char* s = Lua_tolstring(p,1,&len);
		wparam = 0;
		if( len == 1 ){
			wparam = s[0];
		}else if( len > 1 ){
			if( s[0] == 'F' ){
				if( strcmp(s,"F1")==0 ){
					wparam = VK_F1;
					ScanCode = 0x3b;
				}else if( strcmp(s,"F2")==0 ){
					wparam = VK_F2;
					ScanCode = 0x3c;
				}else if( strcmp(s,"F3")==0 ){
					wparam = VK_F3;
					ScanCode = 0x3d;
				}else if( strcmp(s,"F4")==0 ){
					wparam = VK_F4;
					ScanCode = 0x3e;
				}else if( strcmp(s,"F5")==0 ){
					wparam = VK_F5;
					ScanCode = 0x3f;
				}else if( strcmp(s,"F6")==0 ){
					wparam = VK_F6;
					ScanCode = 0x40;
				}else if( strcmp(s,"F7")==0 ){
					wparam = VK_F7;
					ScanCode = 0x41;
				}else if( strcmp(s,"F8")==0 ){
					wparam = VK_F8;
					ScanCode = 0x42;
				}else if( strcmp(s,"F9")==0 ){
					wparam = VK_F9;
					ScanCode = 0x43;
				}else if( strcmp(s,"F10")==0 ){
					wparam = VK_F10;
					ScanCode = 0x44;
				}else if( strcmp(s,"F11")==0 ){
					wparam = VK_F11;
					ScanCode = 0x57;
				}else if( strcmp(s,"F12")==0 ){
					wparam = VK_F12;
					ScanCode = 0x58;
				}
			}else if( s[0] == 'N' ){
				if( strcmp(s,"NEXT")==0 ){
					wparam = VK_NEXT;
				}else if( strcmp(s,"NUMPAD0")==0 ){
					wparam = VK_NUMPAD0;
				}else if( strcmp(s,"NUMPAD1")==0 ){
					wparam = VK_NUMPAD1;
				}else if( strcmp(s,"NUMPAD2")==0 ){
					wparam = VK_NUMPAD2;
				}else if( strcmp(s,"NUMPAD3")==0 ){
					wparam = VK_NUMPAD3;
				}else if( strcmp(s,"NUMPAD4")==0 ){
					wparam = VK_NUMPAD4;
				}else if( strcmp(s,"NUMPAD5")==0 ){
					wparam = VK_NUMPAD5;
				}else if( strcmp(s,"NUMPAD6")==0 ){
					wparam = VK_NUMPAD6;
				}else if( strcmp(s,"NUMPAD7")==0 ){
					wparam = VK_NUMPAD7;
				}else if( strcmp(s,"NUMPAD8")==0 ){
					wparam = VK_NUMPAD8;
				}else if( strcmp(s,"NUMPAD9")==0 ){
					wparam = VK_NUMPAD9;
				}
			}else{
				if( strcmp(s,"Capslock")==0 ){
					wparam = VK_CAPITAL;
				}else if( strcmp(s,"Tab")==0 ){
					wparam = VK_TAB;
				}else if( strcmp(s,"BACK")==0 ){
					wparam = VK_BACK;
				}else if( strcmp(s,"HOME")==0 ){
					wparam = VK_HOME;
				}else if( strcmp(s,"INSERT")==0 ){
					wparam = VK_INSERT;
				}else if( strcmp(s,"DELETE")==0 ){
					wparam = VK_DELETE;
				}else if( strcmp(s,"END")==0 ){
					wparam = VK_END;
				}else if( strcmp(s,"PRIOR")==0 ){
					wparam = VK_PRIOR;
				}else if( strcmp(s,"ADD")==0 ){
					wparam = VK_ADD;
				}else if( strcmp(s,"SUBTRACT")==0 ){
					wparam = VK_SUBTRACT;
				}else if( strcmp(s,"DIVIDE")==0 ){
					wparam = VK_DIVIDE;
				}else if( strcmp(s,"MULTIPLY")==0 ){
					wparam = VK_MULTIPLY;
				}
			}
		}

		HWND hWnd = GetWowWnd();
		if( wparam>0 && wparam<0xff &&hWnd!=NULL&&IsWindow(hWnd) ){ 
			lparam = 1;
			lp = wparam;
			lp <<= 16;
			lparam |= lp;
			PostMessage( hWnd,WM_KEYDOWN,wparam,lparam );
			lparam |= 0xc0000000;
			PostMessage( hWnd,WM_KEYUP,wparam,lparam );
		}
	}
	return 0;
}

int g_castspell = 0;
static int olddwProhibitSpell = 0;
static bool bProhibitSpell = false;
static const char* s_unknowCaseSpellName = "UnknowCastSpellName";
static const char* s_unknowCastSpellTarget = "mouseover";
static bool s_changName = false;
static bool s_changTarget = false;
static char s_castSpellName[64];
static char s_castSpellTarget[32]; 
static char* s_name;
static char* s_spell;

int lua_CastOn( void* p ){
	if( Lua_isnumber(p,1) ){
		double d = Lua_tonumber(p,1);
		if( d == 0 ){
			g_castspell = 0;
		}else{
			g_castspell = 1;
		}
	}
	return 0;
}

int lua_GetMouseState(void* p){
	if( GetCapture() ){
		Lua_pushnumber(p,1);
		return 1;
	}
	Lua_pushnil(p);
	return 1;
}

int ScriptEntry( void* p ){
	if( Lua_isstring(p,0) ){
		size_t len;
		const char* s = Lua_tolstring(p,0,&len);
		//使用首字母加速命中
		if( s[0] == 'G'){
			if(strcmp(s,"GetUnitPosition")==0){
				return lua_GetUnitPosition(p);
			}else if(strcmp(s,"GetUnitDirection")==0){
				return lua_GetUnitDirection(p);
			}else if(strcmp(s,"GetDistance")==0){
				return lua_GetDistance(p);
			}else if(strcmp(s,"GetAngle")==0){
				return lua_GetAngle(p);
			}else if(strcmp(s,"GetKeyState")==0){
				return lua_GetKeyState(p);
			}else if(strcmp(s,"GetMouseState")==0){
				return lua_GetMouseState(p);
			}
		}else if( s[0] == 'R' ){
			//从4.1开始不支持Render系列函数
			if(strcmp(s,"ResumeFocus")==0){
				return lua_ResumeFocus(p);
			}
			/*else if(strcmp(s,"RenderBegin")==0){
				return lua_RenderBegin(p);
			}else if(strcmp(s,"RenderEnd")==0){
				return lua_RenderEnd(p);
			}else if(strcmp(s,"RenderMoveTo")==0){
				return lua_RenderMoveTo(p);
			}else if(strcmp(s,"RenderLineTo")==0){
				return lua_RenderLineTo(p);
			}else if(strcmp(s,"RenderColor")==0){
				return lua_RenderColor(p);
			}else if(strcmp(s,"RenderLineWidth")==0){
				return lua_RenderLineWidth(p);
			}else if(strcmp(s,"RenderCircle")==0){
				return lua_RenderCircle(p);
			}else if(strcmp(s,"RenderRect")==0){
				return lua_RenderRect(p);
			}else if(strcmp(s,"RenderTriangle")==0){
				return lua_RenderTriangle(p);
			}else if(strcmp(s,"RenderBox")==0){
				return lua_RenderBox(p);
			}else if(strcmp(s,"RenderArrow")==0){
				return lua_RenderArrow(p);
			}else if(strcmp(s,"RenderNormal")==0){
				return lua_RenderNormal(p);
			}else if(strcmp(s,"RenderSize")==0){
				return lua_RenderSize(p);
			}else if(strcmp(s,"RenderStipple")==0){
				return lua_RenderStipple(p);
			}else if(strcmp(s,"RenderClear")==0){
				return lua_RenderClear(p);
			}else if(strcmp(s,"RenderTranslate")==0){
				return lua_RenderTranslate(p);
			}else if(strcmp(s,"RenderScale")==0){
				return lua_RenderScale(p);
			}else if(strcmp(s,"RenderShow")==0){
				return lua_RenderShow(p);
			}else if(strcmp(s,"RenderOverlay")==0){
				return lua_RenderOverlay(p);
			}else if(strcmp(s,"RenderString")==0){
				return lua_RenderString(p);
			}*/
		}else if( s[0] == 'U' ){
			if(strcmp(s,"UnitFirst")==0){
				return lua_FirstUnit(p);
			}else if(strcmp(s,"UnitNext")==0){
				return lua_NextUnit(p);
			}else if(strcmp(s,"UnitPrev")==0){
				return lua_PrevUnit(p);
			}else if(strcmp(s,"UnitFocus")==0){
				return lua_FocusUnit(p);
			}else if(strcmp(s,"UnitNextAndFocus")==0){
				return lua_NextUnitAndFocus(p);
			}
		}else if( s[0] == 'L' ){
			if(strcmp(s,"LogOpen")==0){
				return lua_LogOpen(p);
			}else if(strcmp(s,"LogLine")==0){
				return lua_LogLine(p);
			}else if(strcmp(s,"LogClose")==0){
				return lua_LogClose(p);
			}
		}else{
			if(strcmp(s,"FocusLock")==0){
				return lua_FocusLock(p);
			}else if(strcmp(s,"IsCollide")==0){
				return lua_IsCollide(p);
			}else if(strcmp(s,"CalcAngle")==0){
				return lua_CalcAngle(p);
			}else if(strcmp(s,"PushKey")==0){
				return lua_PushKey(p);
			}else if(strcmp(s,"IsMoving")==0){
				return lua_IsMoving(p);
//没有发现eye_position
//			}else if(strcmp(s,"ScreenPt")==0){
//				return lua_ScreenPt(p);
			}else if(strcmp(s,"Click")==0){
				return lua_Click(p);
			}else if(strcmp(s,"CastOn")==0){
				return lua_CastOn(p);
			}else if(strcmp(s,"SetCastSpellFocus")==0){
				return lua_SetCastSpellFocus(p);
			}else if(strcmp(s,"SetCastSpellName")==0){
				return lua_SetCastSpellName(p);
//			}else if(strcmp(s,"PlaySound")==0){
//				return lua_PlaySound(p);
//			}else if(strcmp(s,"SoundNote")==0){
//				return lua_SoundNote(p);
//			}else if(strcmp(s,"StopMove")==0 ){
//				return lua_Stop(p);
			}
		}
		if( strcmp(s,s_unknowCaseSpellName)==0 ){
			strcpy( s_castSpellName,s ); //保存以前的参数
			s_name = (char*)s;
			memset( (char*)s,0,sizeof(s_unknowCaseSpellName) );
			strcpy((char*)s,g_CastSpellName);
			s_changName = true;
			if( Lua_isstring(p,1) ){
				size_t len1;
				const char* s1 = Lua_tolstring(p,1,&len1);
				if( strcmp(s1,s_unknowCastSpellTarget)==0 ){
					strcpy( s_castSpellTarget,s1 );
					s_spell = (char*)s1;
					memset((char*)s1,0,sizeof(s_unknowCastSpellTarget));
					strcpy((char*)s1,g_CastSpellTarget);
					s_changTarget = true;
				}
			}
#ifdef _OUT
	//		printf("%s : ",ansi(std::string(s)).c_str());
	//		for( int i=0;i<strlen(s);i++ )
	//			printf("%X,",s[i]);
	//		printf("\n");
#endif
		}
	}
	if( g_castspell==1 ){
		if( g_pdwProhibitSpell ){
			olddwProhibitSpell = *g_pdwProhibitSpell;
			*g_pdwProhibitSpell = 0;
			bProhibitSpell = true;
		}
	}
	if( castFocusID != 0 ){//临时设置当前施法的焦点目标
		b_oldFocusID = *g_pFocusID;
		*g_pFocusID = castFocusID;
		b_castFocus = true;
	}
	return -1;
}

void CastSpellByNameReturn(){
	if( s_changName ){
		strcpy( s_name,s_castSpellName );
		s_changName = false;
	}
	if( s_changTarget ){
		strcpy( s_spell,s_castSpellTarget );
		s_changTarget = false;
	}
	if( b_castFocus && castFocusID != 0 ){//回复焦点目标
		castFocusID = 0;
		*g_pFocusID = b_oldFocusID;
		b_castFocus = false;
	}
	if( g_castspell==1&&bProhibitSpell ){
		bProhibitSpell = false;
		if( g_pdwProhibitSpell ){
			*g_pdwProhibitSpell = olddwProhibitSpell;
		}
	}
}

/*发现battel.net.dll利用硬件中断完成一些事情
	他们设置的中断在3.35中是这些
	dr0=5B81E830,dr1=5B83B410,dr2=5B856610,dr3=5B821E80
	AddVectoredExceptionHandler(1,5B883460)
	这里回复我的硬件中断
*/
/*
void RestoreBreak(PEXCEPTION_POINTERS pExceptionInfo){
	PCONTEXT pCt = pExceptionInfo->ContextRecord;
	pCt->Dr0 = (DWORD)g_chSwapBuff.pEntryFunc;
	pCt->Dr1 = (DWORD)g_chScriptEntry.pEntryFunc;
	pCt->Dr2 = (DWORD)pGetThreadContext;
	pCt->Dr3 = (DWORD)pAddVectoredExceptionHandler;
	pCt->Dr7 = 0x455;
}

LONG CALLBACK AfterVectorHandler( PEXCEPTION_POINTERS pExceptionInfo ){
}
*/

DWORD pGetThreadContextReturn = 0;
DWORD pCastSpellByName = 0;
/*有4个硬件断点，这里被用来做不同的事情
	1.断点SwapBuff用来绘制
	2.断点CastSpellByName用来使用脚本
	3.断点GetThreadContext用来阻止wow检查线程上下文
	4.断点AddVectoredExceptionHandler这样可以阻止wow消除VEH,目前看wow并不这样做,可以不加
	   或者断点WOW窗口过程函数，用于强制停止移动
*/
//这里加入VEH处理器来处理硬件断点异常
LONG CALLBACK VectorHandler( PEXCEPTION_POINTERS pExceptionInfo ){
	//0X406D1388 wow的一个特定异常，功能未知。暂时不管它
	PEXCEPTION_RECORD pException = pExceptionInfo->ExceptionRecord;
	//if( pException->ExceptionCode!=0X80000004 )
//	printf("Code=%X,EIP=%X,Dr0=%X,Dr1=%X,Dr2=%X,Dr3=%X\nDr6=%X,Dr7=%X\n",
//			pException->ExceptionCode,pExceptionInfo->ContextRecord->Eip,
//			pExceptionInfo->ContextRecord->Dr0,pExceptionInfo->ContextRecord->Dr1,
//			pExceptionInfo->ContextRecord->Dr2,pExceptionInfo->ContextRecord->Dr3,
//			pExceptionInfo->ContextRecord->Dr6,pExceptionInfo->ContextRecord->Dr7);

	if( pException->ExceptionCode == 0X80000004 ){
		BOOL birt = FALSE;
		PCONTEXT pCt = pExceptionInfo->ContextRecord;
		if( pCt->Dr6&1 ){ //硬件断点0中断
			if( pCt->Dr0 == (DWORD)g_chSwapBuff.pEntryFunc ){//wow调用了GL版本的SwapBuff 
				pCt->Dr0 = (DWORD)g_chSwapBuff.pEntryFunc+1;
				myDraw(); //先调用我的myDraw
				birt = TRUE;
			}else if(pCt->Dr0 == (DWORD)g_chSwapBuff.pEntryFunc+1){
				pCt->Dr0 = (DWORD)g_chSwapBuff.pEntryFunc;
				birt = TRUE;
			}
		}
		if( pCt->Dr6&2 ){//硬件断点1中断
			if( pCt->Dr1 == (DWORD)g_chScriptEntry.pEntryFunc ){//wow调用了脚本
				int ret = ScriptEntry( (void*)*(LPDWORD)(pCt->Esp+4));
				if( ret != -1 ){
					pCt->Eax = ret;
					//这里然后函数直接返回
					pCt->Eip = *((LPDWORD)pCt->Esp);
					pCt->Esp += 4; //这个是eip的出栈
					birt = TRUE;
				}else{ //没有找到自己的函数，这里调用默认函数
					//这里在调用CastSpellByName的返回的下一条指令设置断点
					pCt->Dr1 = *((LPDWORD)pCt->Esp);
					pCastSpellByName = pCt->Dr1;
					birt = TRUE;
				}
			}else if( pCt->Dr1 == pCastSpellByName ){
				CastSpellByNameReturn();
				pCt->Dr1 = (DWORD)g_chScriptEntry.pEntryFunc;
				birt = TRUE;
			}
		}
		if( pCt->Dr6&4 ){
			if( pCt->Dr2 == (DWORD)pGetThreadContext ){ //硬件断点GetThreadContext
				pCt->Dr2 = *((LPDWORD)pCt->Esp);
				pGetThreadContextReturn = pCt->Dr2;
				#ifdef _OUT
					printf("GetThreadContrext\n");
				#endif
				birt = TRUE;
			}else if( pCt->Dr2 == (DWORD)pGetThreadContextReturn ){ 
				//返回时对LPCONTEXT进行修改用来隐藏Dr0-Dr7寄存器
				pCt->Dr2 = (DWORD)pGetThreadContext;
				LPCONTEXT pCon = (LPCONTEXT)(pCt->Esp+4);
				pCon->Dr0 = 0;
				pCon->Dr1 = 0;
				pCon->Dr2 = 0;
				pCon->Dr3 = 0;
				pCon->Dr6 = 0;
				pCon->Dr7 = 0;
				birt = TRUE;
			}
		}
		if( pCt->Dr6&8 ){
			if( ex_wowProc==NULL ){
				if( pCt->Dr3 == (DWORD)pAddVectoredExceptionHandler ){
					pCt->Dr3 = (DWORD)pAddVectoredExceptionHandler+2;
					#ifdef _OUT
						printf("AddVectoredExceptionHandler(%X,%X)\n",*((ULONG*)(pCt->Esp+4)),*((ULONG*)(pCt->Esp+8)) );
					#endif
					*((ULONG*)(pCt->Esp+4)) = 0; //把后来的AddVectoredExceptionHandler都放到向量处理的后面
					birt = TRUE;
				}else if( pCt->Dr3 == (DWORD)pAddVectoredExceptionHandler+2){
					pCt->Dr3 = (DWORD)pAddVectoredExceptionHandler;
					birt = TRUE;
				}
			}else{
				//中断窗口过程函数
				if( pCt->Dr3 == (DWORD)ex_wowProc ){
					UINT uMsg = *(UINT*)(pCt->Esp+8);
					WPARAM wParam = *(WPARAM*)(pCt->Esp+12);
					LPARAM lParam = *(LPARAM*)(pCt->Esp+16);
					BOOL b = myProc(uMsg,wParam,lParam);
					if( b ){
						//直接返回，不把这个消息发给wow的过程函数
						//wndProc是stdcall
						pCt->Eip = *((LPDWORD)pCt->Esp);
						pCt->Esp += 20; //4个参数加eip的出栈
						pCt->Eax = 0;
						birt = TRUE;
					}else{
						//让程序继续
						pCt->Dr3 = (DWORD)ex_wowProc+1;
						birt = TRUE;
					}
				}else if(pCt->Dr3 == (DWORD)ex_wowProc+1){
					pCt->Dr3 = (DWORD)ex_wowProc;
					birt = TRUE;
				}
			}
		}
		if( birt )return EXCEPTION_CONTINUE_EXECUTION;
		/*wow从3.3.5开始在登录时候使用硬件中断，我猜测他是为了防止跟踪和调试
		*/
		CreateDelayThread();
	}
#ifdef _OUT
	printf("Code=%X,EIP=%X,Dr0=%X,Dr1=%X,Dr2=%X,Dr3=%X\nDr6=%X,Dr7=%X\n",
			pException->ExceptionCode,pExceptionInfo->ContextRecord->Eip,
			pExceptionInfo->ContextRecord->Dr0,pExceptionInfo->ContextRecord->Dr1,
			pExceptionInfo->ContextRecord->Dr2,pExceptionInfo->ContextRecord->Dr3,
			pExceptionInfo->ContextRecord->Dr6,pExceptionInfo->ContextRecord->Dr7);
#endif
	//LogStack( (LPDWORD)pExceptionInfo->ContextRecord->Esp );
	return EXCEPTION_CONTINUE_SEARCH;//EXCEPTION_CONTINUE_EXECUTION;//EXCEPTION_CONTINUE_SEARCH;
}

PVOID pVeh = NULL;

//注册VEH处理器
void RegVEH(){
	pVeh = AddVectoredExceptionHandler(1,VectorHandler);
	if( !pVeh ){
#ifdef _OUT
		printf("AddVectoredExceptionHandler fail!\n");
#endif
	}else{
#ifdef _OUT
		printf("AddVectoredExceptionHandler ok!\n");
#endif
	}
	hookType = 1;
}

void UnregVEH(){
	RemoveVectoredExceptionHandler( pVeh );
}

//使用内存跟踪的方法
DWORD retCastSpellByName;
BOOL		bCastSpellByName;

void CastSpellByNameReturn2(){
	DWORD op;
	//这里需要做堆栈调整，因为没有人调用这个函数
	//必须在堆栈前面加入一个调用返回值
	_asm{
		push eax
		push eax
		mov eax,[ebp]
		mov [ebp-4],eax
		sub ebp,4
		mov eax,retCastSpellByName
		mov [ebp+4],eax
	}
	if( bCastSpellByName )
		CastSpellByNameReturn();
	VirtualProtect( GET_PAGE(g_chScriptEntry.pEntryFunc),PAGE_SIZE,PAGE_READONLY,&op );
	_asm{
		pop eax
	}
}

DWORD retSwapBuff;

void SwapBuffReturn(){
	DWORD op;
	_asm{
		push eax
		push eax
		mov eax,[ebp]
		mov [ebp-4],eax
		sub ebp,4
		mov eax,retSwapBuff
		mov [ebp+4],eax
	}
	VirtualProtect( GET_PAGE(g_chSwapBuff.pEntryFunc),PAGE_SIZE,PAGE_READONLY,&op );
	_asm{
		pop eax
	}
}

LONG CALLBACK VectorHandler2( PEXCEPTION_POINTERS pExceptionInfo ){
	DWORD code = pExceptionInfo->ExceptionRecord->ExceptionCode;
	PCONTEXT pCt = pExceptionInfo->ContextRecord;

	if( code==STATUS_ACCESS_VIOLATION ){
		//如果是一次指定范围的调用
		if( pCt->Eip>=(DWORD)GET_PAGE(g_chScriptEntry.pEntryFunc) &&
					pCt->Eip<((DWORD)GET_PAGE(g_chScriptEntry.pEntryFunc)+PAGE_SIZE) ){
			bCastSpellByName = FALSE;
			//如果是对CastSpellByName的调用
			if( pCt->Eip==(DWORD)g_chScriptEntry.pEntryFunc ){
				int ret = ScriptEntry( (void*)*(LPDWORD)(pCt->Esp+4));
				if( ret!=-1 ){
					pCt->Eax = ret;
					//是一个自定义功能，直接返回
					//printf("ret=%X,eip=pCt->Eip\n",*((LPDWORD)pCt->Esp),pCt->Eip);
					pCt->Eip = *((LPDWORD)pCt->Esp);
					pCt->Esp += 4; //这个是eip的出栈
					return EXCEPTION_CONTINUE_EXECUTION;
				}else{
					bCastSpellByName = TRUE;
				}
			}
			//调用原来的CastSpellByName
			DWORD op;
			//使目标区域可以执行
			VirtualProtect( GET_PAGE(g_chScriptEntry.pEntryFunc),PAGE_SIZE,PAGE_EXECUTE_READ,&op );
			//保持正常的返回位置
			retCastSpellByName = *((LPDWORD)pCt->Esp);
			//设置一个堆栈挂钩，当程序执行完成返回到retCastSpellByName
			*((LPDWORD)pCt->Esp) = (DWORD)CastSpellByNameReturn2;
			return EXCEPTION_CONTINUE_EXECUTION;
		}else if(pCt->Eip>=(DWORD)GET_PAGE(g_chSwapBuff.pEntryFunc) &&
					pCt->Eip<((DWORD)GET_PAGE(g_chSwapBuff.pEntryFunc)+PAGE_SIZE) ){
			if( pCt->Eip==(DWORD)g_chSwapBuff.pEntryFunc ){
				myDraw();
			}
			DWORD op;
			VirtualProtect( GET_PAGE(g_chSwapBuff.pEntryFunc),PAGE_SIZE,PAGE_EXECUTE_READ,&op );
			retSwapBuff = *((LPDWORD)pCt->Esp);
			*((LPDWORD)pCt->Esp) = (DWORD)SwapBuffReturn;
			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}else if( code==STATUS_SINGLE_STEP ){
		if( pCt->Dr6&8 ){
			if( pCt->Dr3 == (DWORD)pAddVectoredExceptionHandler ){
				pCt->Dr3 = (DWORD)pAddVectoredExceptionHandler+2;
				#ifdef _OUT
					printf("AddVectoredExceptionHandler(%X,%X)\n",*((ULONG*)(pCt->Esp+4)),*((ULONG*)(pCt->Esp+8)) );
				#endif
				*((ULONG*)(pCt->Esp+4)) = 0; //把后来的AddVectoredExceptionHandler都放到向量处理的后面
				return EXCEPTION_CONTINUE_EXECUTION;
			}else if( pCt->Dr3 == (DWORD)pAddVectoredExceptionHandler+2){
				pCt->Dr3 = (DWORD)pAddVectoredExceptionHandler;
				return EXCEPTION_CONTINUE_EXECUTION;
			}
		}
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

void SetMainThreadHardBreakPoint2(){
	THREADENTRY32 th32;
	int nCount = 0;
	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,GetCurrentProcessId());
	if( hThreadSnap == INVALID_HANDLE_VALUE ){
#ifdef _OUT
		printf( "hThreadSnap = INVALID_HANDLE_VALUE\n" );
#endif
		return;
	}
	th32.dwSize = sizeof(THREADENTRY32);
	if( Thread32First(hThreadSnap,&th32) ){
		do{
			if( GetCurrentProcessId() == th32.th32OwnerProcessID ){
				if( nCount==0 ){
					SetStartHardBreakPointForAnitCheck( th32.th32ThreadID );
				}
				nCount++;
			}
		}while( Thread32Next(hThreadSnap,&th32) );
	}
	CloseHandle(hThreadSnap);
}

void RegVEH2(){
	pVeh = AddVectoredExceptionHandler(1,VectorHandler2);
	if( !pVeh ){
#ifdef _OUT
		printf("AddVectoredExceptionHandler fail!\n");
#endif
	}else{
#ifdef _OUT
		printf("AddVectoredExceptionHandler ok!\n");
#endif
	}
	hookType = 2;
	//对CastSpellByName设置只读属性
	DWORD op;
	BOOL b=VirtualProtect( GET_PAGE(g_chScriptEntry.pEntryFunc),PAGE_SIZE,PAGE_READONLY,&op );
	#ifdef _OUT
	if( !b ){
		printf("VirtualProtect fail!\n");
	}else{
		printf("VirtualProtect ok!\n");
	}
	#endif
//	VirtualProtect( GET_PAGE(g_chSwapBuff.pEntryFunc),PAGE_SIZE,PAGE_READONLY,&op );
	SetMainThreadHardBreakPoint2();
}