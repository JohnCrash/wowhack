// stdafx.cpp : 只包括标准包含文件的源文件
// wowhack.pch 将成为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

// TODO: 在 STDAFX.H 中
//引用任何所需的附加头文件，而不是在此文件中引用

//	HWND hWnd = FindWindow( NULL,"魔獸世界" );
//	if( hWnd ){
//		GetWindowThreadProcessId( hWnd,&pid );
//		InjectLib( pid,"F:\\Source\\wowhack\\Debug\\vcsys.dll" );
//	}

/*
int WINAPI EjectLib(DWORD process_id, char *lib_name)   
{   
    PTHREAD_START_ROUTINE pfnRemote =(PTHREAD_START_ROUTINE)   
            GetProcAddress(GetModuleHandle("Kernel32"), "FreeLibrary");   
       
    if(pfnRemote ==NULL)   
    {   
        WriteLog("EjectLib:GetProcAddress FreeLibrary failed!");   
        return -1;   
    }   
    HANDLE hProcess =OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);   
    if(hProcess ==NULL)   
    {   
        WriteLog("EjectLib:OpenProcess %d failed!", process_id);   
        return -1;   
    }   
   
    CProcessModule pm;   
    HANDLE hmod =pm.GetProcessModuleHandle(process_id, lib_name);   
    if(hmod ==NULL)   
    {   
        CloseHandle(hProcess);   
        //WriteLog("EjectLib(), not found lib:%s", lib_name);   
        return 0;   
    }   
    //WriteLog("EjectLib(), found lib:%s, mod handle:0x%x", lib_name, hmod);   
   
    HANDLE hThread =CreateRemoteThread(hProcess, NULL, 0,    
            pfnRemote, hmod, 0, NULL);   
   
    if(hThread ==NULL)   
    {   
        WriteLog("EjectLib():CreateRemoteThread failed! process_id:%d", process_id);   
        CloseHandle(hProcess);   
        return -1;   
    }   
    WaitForSingleObject(hThread, INFINITE);   
   
    CloseHandle(hProcess);   
    CloseHandle(hThread);   
   
    return 0;   
}   */
