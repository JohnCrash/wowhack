#include "stdafx.h"
#include <winnt.h>
#include <winternl.h>
#include <tlhelp32.h>
#include <dbghelp.h>

typedef struct _LDR_DATA_TABLE_ENTRY
{
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    union
    {
        LIST_ENTRY HashLinks;
        PVOID SectionPointer;
    };
    ULONG CheckSum;
    union
    {
        ULONG TimeDateStamp;
        PVOID LoadedImports;
    };
    PVOID EntryPointActivationContext;
    PVOID PatchInformation;
}LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef struct _PEB_LDR_DATA {
  BYTE       Reserved1[8];
  PVOID      Reserved2;
  PLIST_ENTRY InLoadOrderModuleList;
  PLIST_ENTRY InMemoryOrderModuleList;
  PLIST_ENTRY InInitializationOrderModuleList;
}PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef NTSTATUS
(WINAPI *t_NtQueryInformationProcess)(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

BOOL HideDLL(HMODULE hModMyself){
	HMODULE hMod = GetModuleHandle("ntdll.dll");
	PROCESS_BASIC_INFORMATION stInfo = {0};
	DWORD dwRetnLen = 0;
	t_NtQueryInformationProcess QueryInformationProcess;
	QueryInformationProcess = (t_NtQueryInformationProcess)GetProcAddress(hMod,"NtQueryInformationProcess");
	DWORD dw = QueryInformationProcess( GetCurrentProcess(),(PROCESSINFOCLASS)NULL, &stInfo, sizeof(stInfo), &dwRetnLen);

	PPEB pPeb = stInfo.PebBaseAddress;
	PLIST_ENTRY ListHead, Current;
	PLDR_DATA_TABLE_ENTRY pstEntry = NULL;

//	Sleep(20000);
//	__asm{
//		int 3
//	}
	PPEB_LDR_DATA pLdr = (PPEB_LDR_DATA)(*(PULONG)((ULONG)stInfo.PebBaseAddress + 0x00C));
	ListHead = pLdr->InLoadOrderModuleList;
	Current = ListHead->Flink;
	while ( Current != ListHead)
	{
		pstEntry = CONTAINING_RECORD( Current, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
#ifdef _OUT
		wprintf( L"Module:%s, base:0x%X\r\n", pstEntry->FullDllName.Buffer, pstEntry->EntryPoint);
#endif
		if ( pstEntry->DllBase == hModMyself)
		{
#ifdef _OUT
			printf("Find Module InLoadOrderLinks hModMyself=%X\n",hModMyself);
#endif
			pstEntry->InLoadOrderLinks.Flink->Blink = pstEntry->InLoadOrderLinks.Blink;
			pstEntry->InLoadOrderLinks.Blink->Flink = pstEntry->InLoadOrderLinks.Flink;
			break;
		}
		Current = pstEntry->InLoadOrderLinks.Flink;
	}
	/*
	ListHead = pLdr->InMemoryOrderModuleList;
//	ListHead = &( stInfo.PebBaseAddress->Ldr->InMemoryOrderModuleList);
	Current = ListHead->Flink;
	while ( Current != ListHead)
	{
		pstEntry = CONTAINING_RECORD( Current, LDR_DATA_TABLE_ENTRY, InMemoryOrderModuleList);
#ifdef _OUT
		printf( "Module:%s, base:0x%X\r\n", pstEntry->FullDllName.Buffer, pstEntry->EntryPoint);
#endif
		if ( pstEntry->DllBase == hModMyself)
		{
#ifdef _OUT
			printf("Find Module InLoadOrderLinks InMemoryOrderModuleList\n");
#endif
			pstEntry->InMemoryOrderModuleList.Flink->Blink = pstEntry->InMemoryOrderModuleList.Blink;
			pstEntry->InMemoryOrderModuleList.Blink->Flink = pstEntry->InMemoryOrderModuleList.Flink;
			break;
		}
		Current = pstEntry->InMemoryOrderModuleList.Flink;
	}
	ListHead = pLdr->InInitializationOrderModuleList;
//	ListHead = &( stInfo.PebBaseAddress->Ldr->InInitializationOrderModuleList);
	Current = ListHead->Flink;
	while ( Current != ListHead)
	{
		pstEntry = CONTAINING_RECORD( Current, LDR_DATA_TABLE_ENTRY, InInitializationOrderModuleList);
#ifdef _OUT
		printf( "Module:%s, base:0x%X\r\n", pstEntry->FullDllName.Buffer, pstEntry->EntryPoint);
#endif
		if ( pstEntry->DllBase == hModMyself)
		{
#ifdef _OUT
			printf("Find Module InLoadOrderLinks InInitializationOrderModuleList\n");
#endif
			pstEntry->InInitializationOrderModuleList.Flink->Blink = pstEntry->InInitializationOrderModuleList.Blink;
			pstEntry->InInitializationOrderModuleList.Blink->Flink = pstEntry->InInitializationOrderModuleList.Flink;
			break;
		}
		Current = pstEntry->InInitializationOrderModuleList.Flink;
	}*/
#ifdef _OUT
	MODULEENTRY32 me32; 
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,GetCurrentProcessId());
	me32.dwSize = sizeof(MODULEENTRY32); 
	printf("Module32First Module32Next Enum Module\n");
	if (Module32First(hModuleSnap, &me32)){
		do{
			printf("Found Module %s\n",me32.szExePath );
		}while(Module32Next(hModuleSnap, &me32)); 
	}
	CloseHandle(hModuleSnap);
	printf("Enum Module End\n");
#endif	
	return TRUE;
}

/*
NTSTATUS HideModule(IN HMODULE hModule)
{
    NTSTATUS                  status = -1;
    DWORD                     dwReturnLen = 0;
    PLIST_ENTRY               ListHead = NULL;
    PLIST_ENTRY               Current = NULL;
    PPEB_LDR_DATA             pLdr = NULL;
    PLDR_DATA_TABLE_ENTRY     pstEntry = NULL;
    PROCESS_BASIC_INFORMATION processInfo;

    ZeroMemory(&processInfo, sizeof(PROCESS_BASIC_INFORMATION));
    status = ZwQueryInformationProcess(
        GetCurrentProcess(), 
        ProcessBasicInformation, 
        &processInfo, 
        sizeof(PROCESS_BASIC_INFORMATION), 
        &dwReturnLen);
    if (!NT_SUCCESS(status))
        return status;

    pLdr = (PPEB_LDR_DATA)(*(PULONG)((ULONG)processInfo.PebBaseAddress + 0x00C));

    ListHead = &pLdr->InLoadOrderModuleList;
    Current = ListHead->Blink;
    while (Current != ListHead)
    {
        pstEntry = CONTAINING_RECORD(Current, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        if (pstEntry->DllBase == hModule)
        {
            if (pstEntry->FullDllName.Length != 0)
            {
                ZeroMemory(pstEntry->FullDllName.Buffer, pstEntry->FullDllName.Length);
                pstEntry->FullDllName.Length = 0;
                pstEntry->FullDllName.MaximumLength = sizeof(WCHAR);
                pstEntry->BaseDllName.Length = 0;
                pstEntry->BaseDllName.MaximumLength = sizeof(WCHAR);
            }

            status = STATUS_SUCCESS;
            break;
        }

        Current = pstEntry->InLoadOrderLinks.Blink;
    }

    return status;
}
*/
extern "C" HANDLE _crtheap;
LPVOID dword_base = (LPVOID)0x80000;
DWORD  dword_size = 0x1000;
/*
	把crtheap写入到启动器的地址0x8000
*/
void CopyHeapToLauncher(){
	PROCESSENTRY32 pe32; 
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	pe32.dwSize = sizeof(PROCESSENTRY32);
	BOOL bFound = FALSE;
	if (Process32First(hProcessSnap, &pe32)){
		do{
			if( strcmp(pe32.szExeFile,"Inject.exe") == 0 ){
				bFound = TRUE;
				break;
			}
		}while(Process32Next(hProcessSnap, &pe32)); 
	}
	if( bFound ){
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery( _crtheap,&mbi,sizeof(mbi) );
		HANDLE hInject = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION|PROCESS_VM_WRITE, FALSE, pe32.th32ProcessID);
		if( hInject ){
			LPVOID pheap = VirtualAllocEx(hInject,dword_base,dword_size,MEM_RESERVE,PAGE_READWRITE);
			pheap = VirtualAllocEx(hInject,dword_base,dword_size,MEM_COMMIT,PAGE_READWRITE);
			if( pheap ){
				SIZE_T size;
				DWORD data[2];
				data[0] = (DWORD)_crtheap;
				data[1] = mbi.RegionSize;
				WriteProcessMemory(hInject,dword_base,data,sizeof(data),&size);
#ifdef _OUT
		printf("Found Inject.exe processer and dll _crtheap=0x%x,size=0x%x\n",_crtheap,mbi.RegionSize);
#endif
			}else{
#ifdef _OUT
				printf("VirtualAllocEx fail in Inject.exe\n");
#endif
			}
		}
	}else{
#ifdef _OUT
		printf("Can't found Inject.exe processer\n");
#endif
	}
	CloseHandle(hProcessSnap);
}
