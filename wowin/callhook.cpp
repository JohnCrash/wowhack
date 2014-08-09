#include "stdafx.h"
#include "callhook.h"
#include <stdio.h>

callhook::callhook():pEntryFunc(NULL),pFunInfo(NULL){
}

callhook::callhook( void* pef ):pEntryFunc(pef),pFunInfo(NULL){
}

callhook::callhook( FunEntry* pfe ):pFunInfo(pfe),pEntryFunc(pfe->func){
	//这里做检查看看入口地址对不对
}

void callhook::set_entry( void* pfun ){
	pEntryFunc = pfun;
}

void callhook::jmp_setup( void* pNewFunc ){
	int i;
	DWORD op;

	VirtualProtect( GET_PAGE(pEntryFunc),PAGE_SIZE,PAGE_EXECUTE_READWRITE,&op );
	//保存以前的代码
	for( i = 0;i < 8;i++ )
		((unsigned char*)pEntryCode)[i] = ((unsigned char*)pEntryFunc)[i];
	//加入jmp pNuwFunc的硬编码
	((unsigned char*)pEntryFunc)[0] = 0xe9; //jmp
	*(void**)((char*)pEntryFunc+1) = (void*)((char*)pNewFunc-(char*)pEntryFunc-5);
	VirtualProtect( GET_PAGE(pEntryFunc),PAGE_SIZE,PAGE_EXECUTE_READ,&op );
}

void callhook::jmp_clear(){
	int i;
	DWORD op;

	VirtualProtect( GET_PAGE(pEntryFunc),PAGE_SIZE,PAGE_EXECUTE_READWRITE,&op );
	for( i = 0;i < 8;i++ )
		((unsigned char*)pEntryFunc)[i] = ((unsigned char*)pEntryCode)[i];
	VirtualProtect( GET_PAGE(pEntryFunc),PAGE_SIZE,PAGE_EXECUTE_READ,&op );
}

bool callhook::check(){
	unsigned char int3[1] = {0xcc};

	if( !pFunInfo )return false;
	__try{
		if( memcmp_mask( (char*)pEntryFunc+pFunInfo->off,pFunInfo->code,pFunInfo->mask,sizeof(pFunInfo->code) ) == 0 ){
#ifdef _OUT
			printf("callhook::check(\"%s\") PASS\n",pFunInfo->name);
#endif
			return true;
		}
	}__except(1){
	}
#ifdef _OUT
	printf("callhook::check(\"%s\") FALSE\n",pFunInfo->name);
#endif
	return false;
}

#define SEARCH_RANG 32*1024

bool callhook::research(){
	void* pEntry;
	void* pEnd;

	pEntry = GET_PAGE((unsigned long)pEntryFunc-SEARCH_RANG);
	pEnd = GET_PAGE((unsigned long)pEntryFunc+SEARCH_RANG);

	if( !pFunInfo )return false;

	void* pf = ScreechFunctionEntry( pEntry,pEnd,pFunInfo->code,pFunInfo->mask );
	if( pf ){
		void* pold = pEntryFunc;
		pEntryFunc = (char*)pf-pFunInfo->off;
		if( check() ){
#ifdef _OUT
			printf("callhook::research(\"%s\") PASS %X\n",pFunInfo->name,pEntryFunc);
#endif
			return true;
		}
		pEntryFunc = pold;	
	}
#ifdef _OUT
	printf("callhook::research(\"%s\") FALSE\n",pFunInfo->name);
#endif
	return false;
}