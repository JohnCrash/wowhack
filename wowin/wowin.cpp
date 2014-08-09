#include "stdafx.h"
#include "callhook.h"
#include "wowentry.h"
#include "wowin.h"
#include "waypoint.h"
#include "collide.h"
#include "render.h"
#include "movement.h"
#include "scout.h"
#include "camera.h"
#include "ai.h"
#include "key.h"
#include "nethook.h"
#include "anticheck.h"
#include "hidemodule.h"
#include <io.h>
#include <fcntl.h>

extern const char* sWow;
bool  g_bAccess = false;
const char* sPlayer = "player";

void* g_pGfx = NULL;  //wow的图形引擎
int	  g_iGfxType = -1;//引擎类型，如果0=GL,1D3D 
void* g_pWndProc = NULL; //wow的窗口过程函数
void* CheckHash(void* a,void* b,void* c);
void* CreateGXInterface(void* a,void* b,void* c);
void  ClearAllHook();

void DeniedOpenProcess(){
	return;
}

void GetUnitDirection( void* p,float* v ){
	float a;
	a = GetUnitAngle( p );
	v[0] = cos(a);
	v[1] = sin(a);
	v[2] = 0;
}

int lua_IsHack( void* lua ){
	MessageBox(NULL,"Hack work!","wowhack",MB_OK );
	return 0;
}

//Lua扩展函数，打开被禁用的函数
int lua_AccessOn( void* l ){
	if( Lua_isnumber(l,1) ){
		double d = Lua_tonumber(l,1);
		if( d == 0 )
			g_bAccess = false;
		else
			g_bAccess = true;
		EnableProhibitFunc();
	}
	return 0;
}

int lua_AccessOff( void* l ){
	g_bAccess = false;
	return 0;
}

//Lua扩展函数，取得对象位置x,y,z = GetUnitPosition("player")
int lua_GetUnitPosition( void* l ){
	float v[6];

	if( Lua_isstring(l,1) ){
		size_t len;
		const char * name = Lua_tolstring(l,1,&len);
		void* punit = GetUnitByName( name );
		if( punit ){
			GetUnitPosition( punit,v );
			Lua_pushnumber(l,v[0]);
			Lua_pushnumber(l,v[1]);
			Lua_pushnumber(l,v[2]);
			return 3;
		}else if(g_pFocusID&&strcmp(name,"focus")==0){
			punit = GetUnit(*g_pFocusID,1);
			if( punit ){
				GetUnitPosition( punit,v );
				Lua_pushnumber(l,v[0]);
				Lua_pushnumber(l,v[1]);
				Lua_pushnumber(l,v[2]);
				return 3;
			}
		}
	}
	Lua_pushnil(l);
	return 1;
}

//Lua扩展函数，取得对象的面向x,y,z = GetUnitDirection("player")
int lua_GetUnitDirection( void* l ){
	float v[6];

	if( Lua_isstring(l,1) ){
		size_t len;
		const char * name = Lua_tolstring(l,1,&len);
		void* punit = GetUnitByName( name );
		if( punit ){
			GetUnitDirection( punit,v );
			Lua_pushnumber(l,v[0]);
			Lua_pushnumber(l,v[1]);
			Lua_pushnumber(l,v[2]);
			return 3;
		}
	}
	Lua_pushnil(l);
	return 1;
}

//计算x,y,z和你的距离d=GetDistance(x,y,z)
int lua_GetDistance( void* l ){
	float uv[3],pv[3],v[3];
	bool isok = true;
	if( Lua_isnumber(l,1) ){
		uv[0] = (float)Lua_tonumber(l,1);
	}else isok = false;
	if( Lua_isnumber(l,2) ){
		uv[1] = (float)Lua_tonumber(l,2);
	}else isok = false;
	if( Lua_isnumber(l,3) ){
		uv[2] = (float)Lua_tonumber(l,3);
	}else isok = false;

	if( isok ){
		size_t len;
		const char * name = Lua_tolstring(l,1,&len);
		void* pplayer = GetUnitByName(sPlayer);
		if( pplayer ){
			GetUnitPosition( pplayer,pv );
	//		pv[2] = 0;
	//		uv[2] = 0;
			vector_sub(v,pv,uv);

			Lua_pushnumber( l,vector_mod( v ) );
			return 1;
		}
	}
	Lua_pushnil(l);
	return 1;
}

//计算x,y,z和你的角度a=GetAngle(x,y,z)
int lua_GetAngle( void* l ){
	float uv[3],pv[3],av[3],sv[3];

	bool isok = true;
	if( Lua_isnumber(l,1) ){
		uv[0] = (float)Lua_tonumber(l,1);
	}else isok = false;
	if( Lua_isnumber(l,2) ){
		uv[1] = (float)Lua_tonumber(l,2);
	}else isok = false;
	if( Lua_isnumber(l,3) ){
		uv[2] = (float)Lua_tonumber(l,3);
	}else isok = false;

	if( isok ){
		size_t len;
		const char * name = Lua_tolstring(l,1,&len);
		void* pplayer = GetUnitByName(sPlayer);
		if( pplayer ){
			GetUnitPosition( pplayer,pv );
			GetUnitDirection( pplayer,av );
			vector_sub( sv,uv,pv );
			sv[2] = 0;
			double a = vector_angle(av,sv);

			a *= 180/M_PI;
			Lua_pushnumber( l,a );
			return 1;
		}
	}
	Lua_pushnil(l);
	return 1;
}

int lua_CalcAngle( void* l ){
	float uv[3],pv[3];
	bool isok = true;
	if( Lua_isnumber(l,1) ){
		uv[0] = (float)Lua_tonumber(l,1);
	}else isok = false;
	if( Lua_isnumber(l,2) ){
		uv[1] = (float)Lua_tonumber(l,2);
	}else isok = false;
	if( Lua_isnumber(l,3) ){
		uv[2] = (float)Lua_tonumber(l,3);
	}else isok = false;
	if( Lua_isnumber(l,4) ){
		pv[0] = (float)Lua_tonumber(l,4);
	}else isok = false;
	if( Lua_isnumber(l,5) ){
		pv[1] = (float)Lua_tonumber(l,5);
	}else isok = false;
	if( Lua_isnumber(l,6) ){
		pv[2] = (float)Lua_tonumber(l,6);
	}else isok = false;

	if( isok ){
		double a = vector_angle(uv,pv);

		a *= 180/M_PI;
		Lua_pushnumber( l,a );
		return 1;
	}
	Lua_pushnil(l);
	return 1;
}

int lua_GetKeyState( void* l ){
	if( Lua_isnumber(l,1) ){
		int k = (int)Lua_tonumber(l,1);
		if( GetKeyState(k)&0x8000 ){
			Lua_pushnumber( l,1.0 );
			return 1;
		}
	}
	Lua_pushnil(l);
	return 1;
}
/*
struct vkmap_t{
	UINT key;
	const char* name;
};

vkmap_t vkmap[] = {
	{VK_LSHIFT,"Shift"},{VK_LCONTROL,"Ctrl"},{VK_LMENU ,"Alt"},

	{9,"Tab"},{20,"Capslock"},

	{48,"0"},{49,"1"},{50,"2"},{51,"3"},{52,"4"},
	{53,"5"},{54,"6"},{55,"7"},{56,"8"},{57,"9"},

	{65,"A"},{66,"B"},{67,"C"},{68,"D"},{69,"E"},
	{70,"F"},{71,"G"},{72,"H"},{73,"I"},{74,"J"},
	{75,"K"},{76,"L"},{77,"M"},{78,"N"},{79,"O"},
	{80,"P"},{81,"Q"},{82,"R"},{83,"S"},{84,"T"},
	{85,"U"},{86,"V"},{87,"W"},{88,"X"},{89,"Y"},{90,"Z"},

	{112,"F1"},{113,"F2"},{114,"F3"},{115,"F4"},{116,"F5"},
	{117,"F6"},{118,"F7"},{119,"F8"},{120,"F9"},{121,"F10"},
	{122,"F11"},{123,"F12"}
};

int lua_GetKeyStates( void* l ){
	BYTE keys[256];
	if( GetKeyboardState(keys) ){
		for( int i = 0;i < sizeof(vkmap)/sizeof(vkmap_t);++i ){
			if( keys[vkmap[i].key] ){

			}
		}
	}
	Lua_pushnil(l);
	return 1;
}
*/
void RegisterMyFuncs(){
#ifndef _HARDBREAK_
	RegistFunction("IsHack",lua_IsHack );
	RegistFunction("AccessOn",lua_AccessOn );
	RegistFunction("AccessOff",lua_AccessOff );
	RegistFunction("GetUnitPosition",lua_GetUnitPosition );
	RegistFunction("GetUnitDirection",lua_GetUnitDirection );
	RegistFunction("GetDistance",lua_GetDistance );
	RegistFunction("GetAngle",lua_GetAngle );
	RegistFunction("CalcAngle",lua_CalcAngle );

	RegistFunction("CreateWay",lua_CreateWay);
	RegistFunction("SaveWay",lua_SaveWay);
	RegistFunction("LoadWay",lua_LoadWay);
	RegistFunction("AddWayPoint",lua_AddWayPoint);
	RegistFunction("CurrentWayPoint",lua_CurrentWayPoint);
	RegistFunction("PrevWayPoint",lua_PrevWayPoint);
	RegistFunction("NextWayPoint",lua_NextWayPoint);
	RegistFunction("SearchWayPoint",lua_SearchWayPoint);
	RegistFunction("ClearWayPoint",lua_ClearWayPoint);
	RegistFunction("SetWayPointArea",lua_SetWayPointArea);
	RegistFunction("LoadCross",lua_LoadCross);
	RegistFunction("SaveCross",lua_SaveCross);
	RegistFunction("RecalcCross",lua_RecalcCross);
	RegistFunction("CalcGoto",lua_CalcGotoCross);

	RegistFunction("IsCalcing",lua_IsCalcingCross);
	RegistFunction("IsCollide",lua_IsCollide);

	RegistFunction("RenderBegin",lua_RenderBegin);
	RegistFunction("RenderEnd",lua_RenderEnd);
	RegistFunction("RenderMoveTo",lua_RenderMoveTo);
	RegistFunction("RenderLineTo",lua_RenderLineTo);
	RegistFunction("RenderColor",lua_RenderColor);
	RegistFunction("RenderLineWidth",lua_RenderLineWidth);
	RegistFunction("RenderCircle",lua_RenderCircle);
	RegistFunction("RenderRect",lua_RenderRect);
	RegistFunction("RenderTriangle",lua_RenderTriangle);
	RegistFunction("RenderBox",lua_RenderBox);
	RegistFunction("RenderArrow",lua_RenderArrow);
	RegistFunction("RenderNormal",lua_RenderNormal);
	RegistFunction("RenderSize",lua_RenderSize);
	RegistFunction("RenderClear",lua_RenderClear);
	RegistFunction("RenderTranslate",lua_RenderTranslate);
	RegistFunction("RenderRotate",lua_RenderRotate);
	RegistFunction("RenderScale",lua_RenderScale);
	RegistFunction("RenderShow",lua_RenderShow);
	RegistFunction("RenderOverlay",lua_RenderOverlay);
	RegistFunction("RenderStipple",lua_RenderStipple);
	RegistFunction("RenderString",lua_RenderString);

	RegistFunction("IsMoving",lua_IsMoving);

	RegistFunction("UnitFirst",lua_FirstUnit);
	RegistFunction("UnitNext",lua_NextUnit);
	RegistFunction("UnitPrev",lua_PrevUnit);
	RegistFunction("ResumeFocus",lua_ResumeFocus);
	RegistFunction("UnitFocus",lua_FocusUnit);

	//RegistFunction("TurnCamera",lua_TurnCamera);

	RegistFunction("AIUnitSearch",lua_UnitSearch);
	RegistFunction("AIIsUpdate",lua_IsUpdate);
	RegistFunction("AIUpdateStatic",lua_UpdateStatic);
	RegistFunction("AIUpdateHealth",lua_UpdateHealth);
	RegistFunction("AIUpdateState",lua_UpdateState);
	RegistFunction("AIUpdateCasting",lua_UpdateCasting);
	RegistFunction("AIUpdateTarget",lua_UpdateTarget);
	RegistFunction("AIUpdateBuff",lua_UpdateBuff);
	RegistFunction("AIUpdateDebuff",lua_UpdateDebuff);
	RegistFunction("AIResumeFocus",lua_AIResumeFocus);
	RegistFunction("AIClearSkill",lua_AIClearSkill);
	RegistFunction("AIAddSkill",lua_AIAddSkill);
	RegistFunction("AILoadBot",lua_AILoadBot);
	RegistFunction("AIFreeBot",lua_AIFreeBot);
	RegistFunction("AIThink",lua_AIThink);
	RegistFunction("AIGetCooldownListSize",lua_AIGetCooldownListSize);
	RegistFunction("AIGetCooldownSpell",lua_AIGetCooldownSpell);
	RegistFunction("AIUpdateCooldown",lua_AIUpdateCooldown);
	RegistFunction("AIMove",lua_AIMove);
	RegistFunction("AITurn",lua_AITurn);
	RegistFunction("AIUpdateRegion",lua_AIUpdateRegion);
	RegistFunction("AIAction",lua_AIAction);
	RegistFunction("AIBotIsReady",lua_AIBotIsReady);
	RegistFunction("AIChatEvent",lua_AIChatEvent);

	RegistFunction("GetKeyState",lua_GetKeyState);

	RegistFunction("LoadDiffText",lua_LoadDiffText);
	RegistFunction("AddDiffText",lua_AddDiffText);
	RegistFunction("SaveDiffText",lua_SaveDiffText);
#endif
}

#ifndef _HARDBREAK_
void RegistALLFuncs(){
	g_chRegistALLFuncs.jmp_clear();
	CALL(g_chRegistALLFuncs,t_RegistALLFuncs)();

	RegisterMyFuncs();

	g_chRegistALLFuncs.jmp_setup(RegistALLFuncs);
}
#endif

void CallChecker( void* pCall ){
	if( g_bAccess )
		EnableProhibitFunc();
	return;
}

void EnableProhibitFunc(){
	if( g_pdwProhibitSpell )
		*g_pdwProhibitSpell = 0;
}

#ifndef _HARDBREAK_
int CheckProhibitFunc( int a ){
	if( g_bAccess ){
		return 1;
	}
	g_chCheckProhibitFunc.jmp_clear();
	int ret = CALL(g_chCheckProhibitFunc,t_CheckProhibitFunc)( a );
	g_chCheckProhibitFunc.jmp_setup( CheckProhibitFunc );
	return ret;
}
#endif

#ifndef _HARDBREAK_
int ScanDLLStart( void* p ){
	Log( "ScanDLLStart in" );
	printf( "ScanDLLStart begin....\n");
	ClearAllHook();
	int ret = CALL(g_chScanDLLStart,t_ScanDLLStart)( p );
	SetAllHook();
	printf( "ScanDLLStart end\n");
	Log( "ScanDLLStart out" );
	return ret;
}
#endif
/*
//说明HashEncode用来计算一小段代码的hash值，wow使用它来检测自己
int HashEncode( void* p0,void* p1 ){
	//这里一旦发现有不被监控的HashEncode直接ClearAllHook
	//然后日志里面写入堆栈，以便于跟踪看看是谁调用了HashEncode
	Log( "HashEncode in!!!!!!" );
	ClearAllHook();
	int ret = CALL(g_chHashEncode,t_HashEncode)( p0,p1 );
	//这种函数一单开始会被平凡调用，这里不设置hook了
	Log( "HashEncode out!!!!!!" );
	return ret;
}

//HashEncode2和HashEncode的调用方式一致

int HashEncode2( void* p0,void* p1 ){
	Log( "HashEncode2 in!!!!!!" );
	ClearAllHook();
	int ret = CALL(g_chHashEncode2,t_HashEncode)( p0,p1 );
	SetAllHook();
	Log( "HashEncode2 out!!!!!!" );
	return ret;
}

//HashEncode1是个成员函数
void HashEncode1(){
	//成员函数先保存this指针
	__asm{
		push ecx
	}
	Log( "HashEncode1 in!!!!!!" );
	ClearAllHook();
	__asm{
		pop ecx
		call g_chHashEncode1.pEntryFunc
	}
}
*/
//BotDetected
//这是一个成员函数
/*
void* BotDetected(){
	void* retv;
	Log( "BotDetected in" );
	ClearAllHook();
	retv = CALL(g_chBotDetected,t_BotDetected)();
	SetAllHook();
	Log( "BotDetected out" );
	return retv;
}
*/

DWORD g_isHook = 0;

void SetAllHook(){
#ifndef _HARDBREAK_
	g_isHook = 1;
	g_chCheckHash.jmp_setup( CheckHash );
	g_chRegistALLFuncs.jmp_setup( RegistALLFuncs );
	g_chCallChecker.jmp_setup( CallChecker );
	g_chCheckProhibitFunc.jmp_setup( CheckProhibitFunc );
	g_chCreateGXInterface.jmp_setup( CreateGXInterface );
	g_chScanDLLStart.jmp_setup( ScanDLLStart );
//	g_chHashEncode.jmp_setup( HashEncode );
//	g_chHashEncode1.jmp_setup( HashEncode1 );
//	g_chHashEncode2.jmp_setup( HashEncode2 );
//	g_chBotDetected.jmp_setup( BotDetected );
	printf("Set\n");
	SetRenderHook();
#endif
//	AllThreadHardBreakPoint(TRUE);
}

void ClearAllHook(){
#ifndef _HARDBREAK_
	g_isHook = 0;
//	AllThreadHardBreakPoint(FALSE);
	g_chCheckHash.jmp_clear();
	g_chRegistALLFuncs.jmp_clear();
	g_chCallChecker.jmp_clear();
	g_chCheckProhibitFunc.jmp_clear();
	g_chCreateGXInterface.jmp_clear();
	g_chScanDLLStart.jmp_clear();
//	g_chHashEncode.jmp_clear();
//	g_chHashEncode1.jmp_clear();
//	g_chHashEncode2.jmp_clear();
//	g_chBotDetected.jmp_clear();
	printf("Clear\n");
	ClearRenderHook();
#endif
}

#ifndef _HARDBREAK_
void* CheckHash(void* a,void* b,void* c){
	void* pret;

	Log( "Login wow check hash in" );
	printf("wow start check itself hash code...\n");
	printf("clear all hook...\n");
	ClearAllHook();
	printf("clear complete...\n");
	pret = CALL(g_chCheckHash,t_CheckHash)( a,b,c );
	printf("wow check complete...\n");
	printf("set all hook...\n");
	SetAllHook();
	Log( "Login wow check hash out" );
	return pret;
}
#endif

//type = 0,OpenGL =1 D3D,其他初始化引擎返回
//WndProc窗口过程函数
//c是初始的窗口信息，包括宽高等信息
#ifndef _HARDBREAK_
void* CreateGXInterface(void* type,void* WndProc,void* c){
	void* pret;
	g_chCreateGXInterface.jmp_clear();
	pret = CALL(g_chCreateGXInterface,t_CreateGXInterface)( type,WndProc,c );
	g_chCreateGXInterface.jmp_setup( CreateGXInterface );
	g_iGfxType = (int)type;
	g_pGfx = pret; //保存魔兽的图形引擎
	g_pWndProc = WndProc;
	printf("CreateGXInterface,type=%s,WndProc=%X,Gfx=%X\n",type==0?"GL":"D3D",g_pWndProc,g_pGfx);
	SetRenderHook();
	return pret;
}
#endif

FILE* g_stdOut = NULL;

void OpenConsole(){//下面代码替wow创建一个控制台
#ifdef _OUT
	int hCrt;
	FILE *hf;

	AllocConsole();
	hCrt = _open_osfhandle(
				(long) GetStdHandle(STD_OUTPUT_HANDLE),
				_O_TEXT
			);
	hf = _fdopen( hCrt, "a+" );

	*stdout = *hf;

//	g_stdOut = fopen("wowstdout.txt","w");
//	if( g_stdOut )
//		*stdout = *g_stdOut;

	setvbuf( stdout, NULL, _IONBF, 0 );
#endif
}

void CloseConsole(){//关闭控制台
#ifdef _OUT
//	fclose(g_stdOut);
	FreeConsole();
#endif
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		//这里分成两种实行
			OpenConsole();
			CheckAllHookEntry();
#ifndef _HARDBREAK_
			//修改指令法
			SetAllHook();
#else
			//内存异常法
//#ifndef _OUT
			RegVEH2();
//#else
//			InitDynamicHook();
//#endif
			//硬件断点法
		//	RegVEH();
		//	SetMainThreadHardBreakPoint();
			//动态内存法
		//	InitDynamicHook();
#endif
//这里使用卸除然后复制的办法隐藏dll,因此不调用HideDLL
//具体见wowhack工程
//			HideDLL( (HMODULE)hModule );
//			printf("hModule=%x\n",hModule);
//	不是使用HideDLL,这里要先保存_crtheap内容，因为在FreeLibrary后他被释放了
// 这里提前保存它，然后在回来开始调用后重新分配
			CopyHeapToLauncher();
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
#ifndef _HARDBREAK_
			ClearAllHook();		
#else
		//不卸载，因为隐藏dll个时要卸载这个dll,随后代码会被复制回来
		//因此这里不卸载VEH
		//	UnregVEH();
#endif
//			CloseConsole();
			break;
    }
    return TRUE;
}
