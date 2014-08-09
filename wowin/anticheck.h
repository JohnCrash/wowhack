#pragma once

extern void HookToolhelp32Snapshot( HMODULE hmodCaller );
extern void AllThreadHardBreakPoint(BOOL b);
extern void SetHardBreakPoint( DWORD ThreadId,BOOL b );
extern void RegVEH();
extern void RegVEH2();
extern void UnregVEH();

void SetMainThreadHardBreakPoint();

extern void TestAntiCheck();

HWND GetWowWnd();
WNDPROC GetWowWndProc();
BOOL myProc(UINT uMsg,WPARAM wParam,LPARAM lParam);
void InitDynamicHook();
int ScriptEntry( void* p );
void CastSpellByNameReturn();