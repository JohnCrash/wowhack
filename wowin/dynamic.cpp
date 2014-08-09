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
extern int hookType;
/*
	尝试使用动态修改的方法，不直接修改代码。
	而是通过子类化，CallChecker，等方法在运行状态的漏洞修改
*/
t_lua CastSpellByName;

int lua_MyCastSpellByName( void* p ){
	int ret;
	ret = ScriptEntry( p );
	if( ret != -1 ){
		return ret;
	}
	ret = CastSpellByName( p );
	CastSpellByNameReturn();
	return ret;
}

t_SwapBuff pSwapBuff;

void __stdcall MySwapBuff(){
	__asm{
		push ecx
	}
	myDraw();
	__asm{
		pop ecx
		call pSwapBuff
	}
}

WNDPROC pwowProc = NULL;

LRESULT CALLBACK myWindowProc(
  HWND hwnd, 
  UINT uMsg, 
  WPARAM wParam, 
  LPARAM lParam
  ){
	  if( !myProc( uMsg,wParam,lParam ) )
		return pwowProc(hwnd,uMsg,wParam,lParam);
	  else
	    return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

DWORD WINAPI subClassProc( LPVOID pvoid ){
	while(1){
		pwowProc = GetWowWndProc();
		if( pwowProc ){
			HWND hwnd = GetWowWnd();
			if( hwnd && pwowProc!=myWindowProc ){
#ifdef _OUT
				printf("call SetWindowLongPtr\n");
#endif
				SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG)myWindowProc);
			}
			break;
		}
		Sleep(50);
	}
	return 0;
}

void InitDynamicHook(){
	DWORD op,op2;
	//lua在注册一个函数的时候有个个表，
	//大概类似与funs={{"CastSpellByName",CastSpellByName},...}
	//这里我直接改动这个表中的地址,因为它位于.data段，和变量在一个数据段
	//wow并不校验这些变量的初值
	PDWORD pf;
	hookType = 3;

	pf = pdwCastSpellByName;
	VirtualProtect( GET_PAGE(pf),PAGE_SIZE,PAGE_EXECUTE_READWRITE,&op );
	CastSpellByName = (t_lua)(*pf);
	*pf = (DWORD)lua_MyCastSpellByName;
	VirtualProtect( GET_PAGE(pf),PAGE_SIZE,op,&op2 );
	//在调用一个lua函数的时候wow要检查他是不是出了wow的.text段范围
	//而wow的调用检查函数使用一个范围来标记.text的范围
	//[pdwCallCheckerLow,pdwCallCheckerLow+4]中存放这些范围
	//不能设置成0,0xffffffff,检查函数会比较这个值是否为0
	pf = pdwCallCheckerLow;
	VirtualProtect( GET_PAGE(pf),PAGE_SIZE,PAGE_EXECUTE_READWRITE,&op );
	*pf = 1;
	pf++;
	*pf = 0xffffffff;
	VirtualProtect( GET_PAGE(pf),PAGE_SIZE,op,&op2 );

	//设置绘制挂钩,这个数据处于.rdata段
	VirtualProtect( GET_PAGE(pdwSwapBuff),PAGE_SIZE,PAGE_EXECUTE_READWRITE,&op );
	pSwapBuff = (t_SwapBuff)(*pdwSwapBuff);
	*pdwSwapBuff = (DWORD)MySwapBuff;
	VirtualProtect( GET_PAGE(pdwSwapBuff),PAGE_SIZE,op,&op2 );

	//这里子类化wow窗口，重新定位winproc
	DWORD tid;
	CreateThread(NULL,0,subClassProc,0,0,&tid);
}