#include "stdafx.h"
#include "callhook.h"
#include "dllentry.h"
#include <stdio.h>

//下面代码引用"windows核心编程"
void RelpaceEntry(PCSTR pszCalleeModName,
				  PROC pfnCurrent,PROC pfnNew,
				  HMODULE hmodCaller){
	ULONG ulSize;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)
		ImageDirectoryEntryToData(hmodCaller,TRUE,IMAGE_DIRECTORY_ENTRY_IMPORT,&ulSize);

	if( pImportDesc == NULL ){
		printf("pImportDesc==NULL\n");
		return;
	}
	
	for(;pImportDesc->Name;pImportDesc++){
		PSTR pszModName = (PSTR)((PBYTE)hmodCaller+pImportDesc->Name);
		if( lstrcmpiA( pszModName,pszCalleeModName)==0 )
			break;
	}

	if( pImportDesc->Name == 0 ){
		printf("pImportDesc->Name == 0\n");
		return;
	}
	
	PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)
		((PBYTE)hmodCaller+pImportDesc->FirstThunk);

	for(;pThunk->u1.Function;pThunk++){
		PROC* ppfn = (PROC*)&pThunk->u1.Function;
		BOOL fFound = (*ppfn==pfnCurrent);
		if( fFound ){
			DWORD op;
			VirtualProtect( GET_PAGE(ppfn),PAGE_SIZE,PAGE_EXECUTE_READWRITE,&op );
			*ppfn = pfnNew;
			VirtualProtect( GET_PAGE(ppfn),PAGE_SIZE,PAGE_EXECUTE_READ,&op );
//			WriteProcessMemory(GetCurrentProcess(),ppfn,&pfnNew,sizeof(pfnNew),NULL);
			return;
		}
	}
	printf("RelpaceEntry can't found %s\n",pszCalleeModName);
}