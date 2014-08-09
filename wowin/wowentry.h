#pragma once

//extern callhook g_chCheckHash;
//extern callhook g_chRegistALLFuncs;
//extern callhook g_chCallChecker;
//extern callhook g_chDeniedOpenProcess;
//extern callhook g_chRegistFunction;

extern callhook g_chCheckProhibitFunc;
extern callhook g_ch_lua_isnumber;
extern callhook g_ch_lua_isstring;
extern callhook g_ch_lua_tonumber;
extern callhook g_ch_lua_tolstring;
extern callhook g_ch_lua_pushnumber;
extern callhook g_ch_lua_pushnil;
extern callhook g_chGetUnitByName;
extern callhook g_chPlayerDirection;
extern callhook g_chHitWrap;
extern callhook g_chCreateGXInterface;
extern callhook g_chSetEyeMatrix;
extern callhook g_chEnumUnit;
extern callhook g_chGetUnit;
extern callhook g_chUnitFocus;
extern callhook g_chSwapBuff;
extern callhook g_chGetCamera;
extern callhook g_chTurnCamera;
extern callhook g_chDirectCamera;
//extern callhook g_chScanDLLStart;
extern callhook g_chScriptEntry;

//extern callhook g_chHashEncode;
//extern callhook g_chHashEncode1;
//extern callhook g_chHashEncode2;
//extern callhook g_chBotDetected;
//extern callhook g_chGetTable;
//extern callhook g_chGetMovementInfo;
extern const char* sWowTitle;

typedef void*	(*t_CheckHash)(void* a,void* b,void* c);
typedef int		(*t_lua)(void* a);
typedef void	(*t_RegistALLFuncs)();
typedef void	(*t_RegistFunction)(char*,void*);
typedef void	(*t_CallChecker)(void*);
//typedef void	(*t_DeniedOpenProcess)();
typedef int		(*t_CheckProhibitFunc)(int);
typedef int		(*t_lua_isnumber)(void *, int);
typedef int		(*t_lua_isstring)(void *, int);
typedef const char *(*t_lua_tolstring)(void*, int, size_t*);
typedef double	(*t_lua_tonumber)(void*,int);
typedef void	(*t_lua_pushnumber)(void *, double);
typedef void	(*t_lua_pushnil)(void *);
typedef long long (*t_PlayID)();
typedef void*	(*t_GetUnitByID)(long long,int);
typedef void*	(*t_GetUnitByName)(const char*);
//typedef char	(*t_HitWrap)(float* p,void* c,unsigned long f,int a);
//4.1 去掉了最后一个参数
typedef char	(*t_HitWrap)(float* p,void* c,unsigned long f);
typedef void*	(*t_CreateGXInterface)(void* a,void* b,void* c);
typedef void	(*t_SetEyeMatrix)(float* eye,float* eyeat );
typedef int		(*t_EnumUnitFunc)(long long a,void* b);
typedef int		(*t_EnumUnit)(t_EnumUnitFunc func,void* b);
typedef void*	(*t_GetUnit)(long long a,int b,int c,int e);
typedef void    (*t_UnitFocus)(long long* p);
typedef void*	(*t_GetCamera)();
typedef void	(__stdcall *t_TurnCamera)(float a,float b,float* c);
typedef void	(*t_DirectCamera)();
typedef int		(*t_ScanDLLStart)(void*);
typedef int		(*t_HashEncode)(void*,void*);
typedef void	(*t_HashEncode1)();
typedef void*	(*t_BotDetected)();
//typedef void*	(*t_GetTable)();
//typedef void*   (*t_GetMovementInfo)();

#define RegistFunction(a,b) ((t_RegistFunction)g_chRegistFunction.pEntryFunc)(a,(void*)b)

#ifdef _HARDBREAK_
#define Lua_isstring(a,b) CALL(g_ch_lua_isstring,t_lua_isstring)(a,b+1)
#define Lua_isnumber(a,b) CALL(g_ch_lua_isnumber,t_lua_isnumber)(a,b+1)
#define Lua_tonumber(a,b) CALL(g_ch_lua_tonumber,t_lua_tonumber)(a,b+1)
#define Lua_tolstring(a,b,c) CALL(g_ch_lua_tolstring,t_lua_tolstring)(a,b+1,c)
#else
#define Lua_isstring(a,b) CALL(g_ch_lua_isstring,t_lua_isstring)(a,b)
#define Lua_isnumber(a,b) CALL(g_ch_lua_isnumber,t_lua_isnumber)(a,b)
#define Lua_tonumber(a,b) CALL(g_ch_lua_tonumber,t_lua_tonumber)(a,b)
#define Lua_tolstring(a,b,c) CALL(g_ch_lua_tolstring,t_lua_tolstring)(a,b,c)
#endif

#define Lua_pushnumber(a,b) CALL(g_ch_lua_pushnumber,t_lua_pushnumber)(a,b)
#define Lua_pushnil(a) CALL(g_ch_lua_pushnil,t_lua_pushnil)(a)
#define GetUnitByName(a) CALL(g_chGetUnitByName,t_GetUnitByName)(a)
//4.1减少一个参数
#define HitWrap(a,b,c,e) CALL(g_chHitWrap,t_HitWrap)((float*)a,(void*)b,(unsigned long)c)
#define EnumUnit(a,b) CALL(g_chEnumUnit,t_EnumUnit)(a,b)
#define GetUnit(a,b) CALL(g_chGetUnit,t_GetUnit)(a,b,0,0)
#define UnitFocus(a) CALL(g_chUnitFocus,t_UnitFocus)(a)
#define GetCamera() CALL(g_chGetCamera,t_GetCamera)()
#define DirectCamera() CALL(g_chDirectCamera,t_DirectCamera)()

//#define GetTable() CALL(g_chGetTable,t_GetTable)()
//#define GetMovementInfo() CALL(g_chGetMovementInfo,t_GetMovementInfo)()
extern unsigned int *g_pdwProhibitSpell;

extern float* g_eye_position;
extern float* g_eye_lookat;
extern float* g_matrix_eye;
extern float* g_matrix_projection;
extern long long * g_pFocusID;

void CheckHookEntry( callhook& ch );
void CheckAllHookEntry();
void SetAllHook();
void ClearAllHook();

extern PDWORD pdwCastSpellByName;
extern PDWORD pdwCallCheckerLow;
extern PDWORD pdwSwapBuff;