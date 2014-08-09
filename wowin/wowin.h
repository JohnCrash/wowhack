#pragma once

extern const char* sPlayer;
extern void* g_pGfx;  //wow的图形引擎
extern int	  g_iGfxType;//引擎类型，如果0=GL,1D3D 
extern void* g_pWndProc; //wow的窗口过程函数

extern void RegistALLFuncs();
extern void RegisterMyFuncs();

extern int lua_CalcAngle( void* l );
extern int lua_GetAngle( void* l );
extern int lua_GetDistance( void* l );
extern int lua_GetKeyState( void* l );
extern int lua_GetUnitDirection( void* l );
extern int lua_GetUnitPosition( void* l );
extern int lua_AccessOn( void* l );

extern float DLLEXPORT GetUnitAngle( void* p );
extern void DLLEXPORT EnableProhibitFunc();
extern void DLLEXPORT GetUnitDirection( void* p,float* v );
extern void DLLEXPORT GetUnitPosition( void* p,float* v );
