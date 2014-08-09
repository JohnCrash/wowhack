#ifndef _CALLHOOK_H_
#define _CALLHOOK_H_

#define CALL( c,t ) ((t)c.pEntryFunc)
#define GET_PAGE(a) ((void*)(((unsigned long)a)&0xfffff000))
#define PAGE_SIZE 4096

struct FunEntry{
	const char* name;
	void* func;
	int	  off;
	const unsigned char code[16];
	const unsigned char mask[16];
};

struct DLLEXPORT callhook{
	void* pEntryFunc;
	char  pEntryCode[8];
	FunEntry* pFunInfo;
	
	callhook();
	callhook( void* pef );
	callhook( FunEntry* pfe );

	void set_entry( void* pfun );
	void jmp_setup( void* pNewFunc );
	void jmp_clear();
	bool check();
	bool research();
};

#endif