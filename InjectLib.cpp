#include "stdafx.h"
#include "Tlhelp32.h"

int WINAPI InjectLib( DWORD process_id, HANDLE hproc, char *lib_name )   
{   
    PTHREAD_START_ROUTINE pfnRemote =(PTHREAD_START_ROUTINE)   
            GetProcAddress(GetModuleHandle("Kernel32"), "LoadLibraryA");   
    if(pfnRemote ==NULL)   
        return -1;   

    HANDLE hProcess =OpenProcess(PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION|PROCESS_VM_WRITE, FALSE, process_id);   
	DWORD err = GetLastError();
    if(hProcess ==NULL)   
    {   
		if( hproc )
			hProcess = hproc;
		else
			return -1;
    }   
   
    size_t mem_size =strlen(lib_name)+1;   
    void *premote_mem =VirtualAllocEx(hProcess, NULL, mem_size, MEM_COMMIT, PAGE_READWRITE);   
    if(premote_mem ==NULL)   
    {   
        CloseHandle(hProcess);   
        return -1;   
    }   
      
    int ret =WriteProcessMemory(hProcess, premote_mem, lib_name, mem_size,NULL);   
    if(ret ==STATUS_ACCESS_VIOLATION || ret ==false)   
    {   
        VirtualFreeEx(hProcess, premote_mem, 0, MEM_RELEASE);   
        CloseHandle(hProcess);    
        return -1;   
    }
	/*
	//”√¿¥µ˜ ‘
	{
    HANDLE hThread =CreateRemoteThread(hProcess, NULL, 0,    
            pfnRemote, 0, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);   
	}*/
    HANDLE hThread =CreateRemoteThread(hProcess, NULL, 0,    
            pfnRemote, premote_mem, 0, NULL);   
   
    if(hThread ==NULL)   
    {   
        VirtualFreeEx(hProcess, premote_mem, 0, MEM_RELEASE);   
        CloseHandle(hProcess);   
        return -1;   
    }   
          
    WaitForSingleObject(hThread, INFINITE);   
   
    VirtualFreeEx(hProcess, premote_mem, 0, MEM_RELEASE);   
    CloseHandle(hProcess);   
    CloseHandle(hThread);   
   
    return 0;   
}

int WINAPI EjectLib(DWORD process_id, char *lib_name)   
{   
    PTHREAD_START_ROUTINE pfnRemote =(PTHREAD_START_ROUTINE)   
            GetProcAddress(GetModuleHandle("Kernel32"), "FreeLibrary");   
       
    if(pfnRemote ==NULL)   
    {   
        return -1;   
    }   

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,process_id );
	if( hSnapshot ==NULL )
		return -1;
	MODULEENTRY32 me ={sizeof(me)};
	BOOL fFound = FALSE;
	BOOL fMoreMods = Module32First( hSnapshot,&me );
	for(;fMoreMods;fMoreMods = Module32Next(hSnapshot,&me)){
		fFound = (lstrcmpi(me.szModule,lib_name)==0)||(lstrcmpi(me.szExePath,lib_name)==0);
		if( fFound )break;
	}
	if( !fFound )return -2;

    HANDLE hProcess =OpenProcess(PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION|PROCESS_VM_WRITE, FALSE, process_id);   
    if(hProcess ==NULL)   
    {   
		DWORD err = GetLastError();
        return -1;   
    }   

    HANDLE hThread =CreateRemoteThread(hProcess, NULL, 0,    
            pfnRemote, me.modBaseAddr, 0, NULL);   
   
    if(hThread ==NULL)   
    {   
        CloseHandle(hProcess);   
        return -1;   
    }   
    WaitForSingleObject(hThread, INFINITE);   
   
    CloseHandle(hProcess);   
    CloseHandle(hThread);   
   
    return 0;   
}