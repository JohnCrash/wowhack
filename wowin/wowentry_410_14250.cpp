#include "stdafx.h"
#include "callhook.h"
#include "wowentry.h"
#include <stdio.h>
#include <tlhelp32.h>
#include <dbghelp.h>

#ifdef _WOW_410_14250_CN_
extern const char* sPlayer;

#define NOMASK {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define ENTRY( a,b ) {a,(void*)0X##b,0,NOMASK,NOMASK}
//适用版本WOW3.3.5 zh(13507)
//=====================================
//一个临时函数

const char* sWowTitle="魔兽世界";

//4.1
FunEntry feScriptEntry = ENTRY( "CastSpellByName",00880B10 );
//4.1
FunEntry feMainCheckHack = ENTRY( "MainCheckHack",00A772C0 );
//WOW使用CheckProhibitFunc来判断Lua c函数权限
//4.1 使用SendChatMessage来查询
FunEntry feCheckProhibitFunc= ENTRY( "CheckProhibitFunc",00837370 );

//下面是一组lua函数
//被CastSpellByName所引用
//4.1
FunEntry felua_isstring= ENTRY( "lua_isstring",004429C0 );
//被CastSpellByName所引用
//4.1
FunEntry felua_tolstring= ENTRY( "lua_tolstring",00442B40 );
//被UnitState引用
//4.1
FunEntry felua_isnumber= ENTRY( "lua_isnumber",00442980 );
//被UnitState引用
//4.1
FunEntry felua_tonumber= ENTRY( "lua_tonumber",00442A90 );
//被UnitState引用
//4.1
FunEntry felua_pushnumber= ENTRY( "lua_pushnumber",00442D00 );
//lua_pushnumber的上一个函数
//4.1
FunEntry felua_pushnil= ENTRY( "lua_pushnil",00442CE0 );
//被Lua_CheckInteractDistance引用
//4.1
FunEntry feGetUnitByName= ENTRY( "GetUnitByName",004678C0 );
//碰撞测试，返回一个碰撞的三角形列表
//HitWrap该函数调用一个较为容易定位的函数，在其尾部
//4.1
FunEntry feHitWrap= ENTRY( "HitWrap",0069C040 );

//暂时没有定位
FunEntry feSetEyeMatrix={"SetEyeMatrix",(void*)0X007959E0,0,
								{0X55,0X8B,0XEC,0X8B,0X0D,0X58,0XEF,0X27,0X01,0X81,0XEC,0XB4,0X00,0X00,0X00,0X53},
								{0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0}};
//4.1
FunEntry feEnumUint= ENTRY( "EnumUnit",004A42A0 );
//4.1
FunEntry feGetUnit= ENTRY( "GetUnit",004A45A0 );
//4.1
FunEntry feUnitFocus= ENTRY( "UnitFocus",0083E5E0 );
//4.1
FunEntry feSwapBuff= ENTRY("SwapBuff",007D0930 );

unsigned int *g_pdwProhibitSpell = NULL;


callhook g_chCheckHash(&feMainCheckHack);

callhook g_chCheckProhibitFunc(&feCheckProhibitFunc);
callhook g_ch_lua_isnumber(&felua_isnumber);
callhook g_ch_lua_isstring(&felua_isstring);
callhook g_ch_lua_tonumber(&felua_tonumber);
callhook g_ch_lua_tolstring(&felua_tolstring);
callhook g_ch_lua_pushnumber(&felua_pushnumber);
callhook g_ch_lua_pushnil(&felua_pushnil);
callhook g_chGetUnitByName(&feGetUnitByName);
callhook g_chHitWrap(&feHitWrap);
callhook g_chSetEyeMatrix(&feSetEyeMatrix);
callhook g_chEnumUnit(&feEnumUint);
callhook g_chGetUnit(&feGetUnit);
callhook g_chUnitFocus(&feUnitFocus);
callhook g_chSwapBuff(&feSwapBuff);

callhook g_chScriptEntry(&feScriptEntry);


void CheckHookEntry( callhook& ch ){
#ifdef _OUT
	printf("=================================================\n");
#endif
	if( !ch.check() && !ch.research() ){
		if( ch.pFunInfo ){
			char s[255];
			char ss[16];
			sprintf(s,"Can't found function:%s\naddrass:%08X\noffset:%d\ncode:{",ch.pFunInfo->name,ch.pFunInfo->func,ch.pFunInfo->off );
			for( int i = 0;i < sizeof(ch.pFunInfo->code);i++ ){
				if( i == sizeof(ch.pFunInfo->code)-1 )
					sprintf( ss,"%02X}",ch.pFunInfo->code[i] );
				else
					sprintf( ss,"%02X,",ch.pFunInfo->code[i] );
				strcat( s,ss );
			}
			MessageBox(NULL,s,"wowhack",MB_OK|MB_ICONERROR );
		}
	}
}

//定位一些wow使用的全局变量
static void HookGlobalToLocal(){
	//这里使用函数SetEyeMatrix定位全局变量g_eye_position和g_eye_lookat
	//004F8003 mov     eye_position_x, eax
	//硬编码是A3 2C 6B 0B 01
	//wow3.11参考位置004F8003
	//wow3.13参考位置004F9460
	//wow3.20参考位置0074E1DB
	//wow3.22(12484)参考位置0074EF0B
	//wow3.35参考位置007959EB
	//==============================================
	//4.1 暂时不支持
	/*
	if( *((unsigned char*)g_chSetEyeMatrix.pEntryFunc+0xab)==0xa3 ){
		g_eye_position = *((float**)((char*)g_chSetEyeMatrix.pEntryFunc+0xab+1));
#ifdef _OUT
		printf("eye_position:%X PASS\n",g_eye_position);
#endif
		g_eye_lookat = g_eye_position+3;
#ifdef _OUT
		printf("eye_lookat:%X PASS\n",g_eye_lookat );
#endif
	}else{
#ifdef _OUT
		printf("Local eye_position FALSE\n");
#endif
		MessageBox(NULL,"Local eye_position FALSE","wowhack",MB_OK|MB_ICONERROR );
	}*/
	//这里使用函数SetEyeMatrix定位全局变量matrix_eye
	//这里是mov ecx, offset matrix_eye
	//硬编码是B9 38 BC FD 00
	//0x1e8是从SetEyeMatrix到mov ecx ...的偏移
	//wow3.11参考位置004F81B8
	//wow3.13参考位置004F9618
	//wow3.20参考位置0074E390 mov     ecx, offset look_matrix
	//wow3.22(12484)参考位置0074F0C0
	//wow3.35参考位置00795BA0                 mov     ecx, offset look_matrix
	//==============================================
	//4.1 暂时不支持
	/*
	if( *((unsigned char*)g_chSetEyeMatrix.pEntryFunc+0x260)==0xb9 ){
		g_matrix_eye = *((float**)((char*)g_chSetEyeMatrix.pEntryFunc+0x260+1));
#ifdef _OUT
		printf("matrix_eye:%X PASS\n",g_matrix_eye);
#endif
		g_matrix_projection = g_matrix_eye+16;
#ifdef _OUT
		printf("matrix_projection:%X PASS\n",g_matrix_projection);
#endif
	}else{
#ifdef _OUT
		printf("Local matrix_eye FALSE\n");
#endif
		MessageBox(NULL,"Local matrix_eye FALSE","wowhack",MB_OK|MB_ICONERROR );
	}*/
	//这里通过g_chCheckProhibitFunc来定位dwProhibitSpell
	//cmp dwProhibitSpell,0
	//硬编码是83 3D ,40 B4 2E  01 00,其中dwProhibitSpell是01 2E B4 40
	//可以看出83 3D等价于cmp,中间是dwProhibitSpell
	//3.20 004B8D73 cmp     dwProhibitSpell, 0
	//wow3.22(12484) 004B8E93
	//wow3.35 00519703                 cmp     dwProhibitSpell, 0
	if( *((unsigned char*)g_chCheckProhibitFunc.pEntryFunc+3)==0x83 &&
		*((unsigned char*)g_chCheckProhibitFunc.pEntryFunc+4)==0x3d ){
		g_pdwProhibitSpell = *((unsigned int**)((unsigned char*)g_chCheckProhibitFunc.pEntryFunc+5));
#ifdef _OUT
		printf("g_pdwProhibitSpell=%X PASS\n",g_pdwProhibitSpell);
#endif
	}else{
#ifdef _OUT
		printf("Local dwProhibitSpell FALSE\n");
		//*((unsigned char*)0X00837370+0) = 123;
		for(  unsigned char* i = (unsigned char*)g_chCheckProhibitFunc.pEntryFunc;
			i<(unsigned char*)g_chCheckProhibitFunc.pEntryFunc+64;++i )
		{
			printf( "%X ",*i );
		}
			/*
		printf("%X [0]=%X,[1]=%X,[2]=%X,[3]=%X,[4]=%X",g_chCheckProhibitFunc.pEntryFunc,
			*((unsigned char*)g_chCheckProhibitFunc.pEntryFunc+0),
			*((unsigned char*)g_chCheckProhibitFunc.pEntryFunc+1),
			*((unsigned char*)g_chCheckProhibitFunc.pEntryFunc+2),
			*((unsigned char*)g_chCheckProhibitFunc.pEntryFunc+3),
			*((unsigned char*)g_chCheckProhibitFunc.pEntryFunc+4));*/
#endif

		MessageBox(NULL,"Local dwProhibitSpell FALSE","wowhack",MB_OK|MB_ICONERROR );
	}
	//这里通过g_chUnitFocus来定位g_pFocusID
	//mov     eax, dword_10A6900
	//硬编码是A1 00 69 0A 01
	//3.20 004BF6A3 mov     eax, g_pFocusID
	//wow3.22(12484) 004BF7E3
	//wow3.35 00520463                 mov     eax, g_pFocusID
	if( *((unsigned char*)g_chUnitFocus.pEntryFunc+3)==0xa1 ){
		g_pFocusID = *((long long**)((unsigned char*)g_chUnitFocus.pEntryFunc+4));
#ifdef _OUT
		printf("g_pFocusID=%X PASS\n",g_pFocusID);
#endif
	}else{
#ifdef _OUT
		printf("Local g_pFocusID FALSE\n");
#endif
		MessageBox(NULL,"Local g_pFocusID FALSE","wowhack",MB_OK|MB_ICONERROR );
	}
}

#define OFFSET( a ) a.pEntryFunc = (char*)a.pEntryFunc + offset

void CheckAllHookEntry(){
	//4.1使用动态基地址，这里重新定位下
	////////////////////////////////////////////
	MODULEENTRY32 me ={sizeof(me)};
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,GetCurrentProcessId());
	BOOL fMoreMods = Module32First( hSnapshot,&me );
	int offset;
	for(;fMoreMods;fMoreMods = Module32Next(hSnapshot,&me)){
		strlwr( me.szModule );
		if( lstrcmpi(me.szModule,"wow.exe")==0 ){
			offset = (DWORD)me.modBaseAddr-0X400000;

			OFFSET(g_chCheckHash);
			OFFSET(g_chCheckProhibitFunc);
			OFFSET(g_ch_lua_isnumber);
			OFFSET(g_ch_lua_isstring);
			OFFSET(g_ch_lua_tonumber);
			OFFSET(g_ch_lua_tolstring);
			OFFSET(g_ch_lua_pushnumber);

			OFFSET(g_ch_lua_pushnil);
			OFFSET(g_chGetUnitByName);
			OFFSET(g_chHitWrap);
			OFFSET(g_chSetEyeMatrix);
			OFFSET(g_chEnumUnit);
			OFFSET(g_chGetUnit);
			OFFSET(g_chUnitFocus);
			OFFSET(g_chSwapBuff);
			OFFSET(g_chScriptEntry);
			break;
		}
	}
	HookGlobalToLocal();
}

//参考函数:Lua_CheckInteractDistance(004A5BF0) wow3.09
//参考位置:004A5CCA
//取得对象坐标，依赖函数位置0x20
//wow3.11 参考位置006E36EA 24h
//wow3.13 没有改变006E62AA mov     eax, [eax+24h]
//wow3.20 004BAC5A mov     eax, [eax+28h]
//wow3.22(12484) 004BAD7A                 mov     eax, [eax+28h]
//wow3.35            0051B85A                 mov     eax, [eax+2Ch]
//4.1 008399D0  mov     edx, [eax+2Ch]
void GetUnitPosition( void* p,float* v ){
	__asm{
		mov ecx,p
		mov eax,[ecx]
		mov eax,[eax+0x2c]
		mov edx,v
		push edx
		call eax
	}
}

//参考函数:lua_cGetUnitDirctorWrap(004B5D50) wow3.09
//参考位置:004B5D58
//取得对象的朝向，依赖函数位置0x24
//直接使用字节搜索法
//wow3.11 参考位置006C1848 28h
//wow3.13 没有改变006C4658 mov     edx, [eax+28h]
//wow3.20 004977D8 mov     edx, [eax+30h]
//wow3.22(12484) 00497678                 mov     edx, [eax+30h]
//wow3.35			  004F7068                 mov     edx, [eax+34h]
//4.1 0084AF88  mov     edx, [eax+34h]
float GetUnitAngle( void* p ){
	float a;
	__asm{
		mov ecx,p
		mov eax,[ecx]
		mov eax,[eax+0x34]
		call eax
		fstp a;
	}
	return a;
}

//取得对象的类型，依赖数据位置2,2
//参考函数:UnitTracking(004D7C60) 魔兽版本3.09
//参考位置:004D7CF2
//wow3.11 参考位置00731880 未改变
//wow3.13未验证
//3.20 00504E2C mov     eax, [edi+8]
//wow3.22(12484) 00504FFC                 mov     eax, [edi+8]
//wow3.35            005801FC                 mov     eax, [edi+8]
int UnitType( void* punit ){
	void* pa = *((void**)punit+2);
	return (int)(*((void**)pa+2));
}

//取得玩家的控制状态，依赖
//参考函数:AnyTest(00673490) 魔兽版本3.09
//参考位置:00673E88
//wow3.11 参考位置005B1E8C
//wow3.22(12484) 
//006B0AD1                 mov     eax, [ebx+0D8h]
//006B0AD7                 test    [eax+44h], ecx
//wow3.35
//0073A3EC                 mov     eax, [ebx+0D8h]
//0073A3F2                 test    [eax+44h], ecx
//4.1 005E5406 mov     ecx, [ebx+100h]
unsigned long GetMovementState(){
	void* s = GetUnitByName(sPlayer);
	if( s ){
		char* ebx = *((char**)s+0x100/4);
		if( ebx ){
			unsigned long state = *((unsigned long*)(ebx+0x44));
			return state;
		}
	}
	return 0;
}

//3.3.5新增加的
PDWORD pdwCastSpellByName = (PDWORD)feScriptEntry.func;
PDWORD pdwCallCheckerLow = (PDWORD)0x00d415b8;
PDWORD pdwSwapBuff = (PDWORD)0X00A2E360;
#endif
