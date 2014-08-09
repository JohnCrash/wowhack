#pragma once

#define CMD_NONOTE (WM_USER+1)
#define CMD_ATTACK (WM_USER+2) //集火目标
#define CMD_DEFANCE (WM_USER+3) //被集火目标
#define CMD_YUN (WM_USER+4) //解羊
#define CMD_ZHIYOU (WM_USER+5) //给自由
#define CMD_TARGET_HEAL (WM_USER+6) //目标低血量
#define CMD_NOTARGET (WM_USER+7) //没有正确集火
#define CMD_STRING (WM_USER+32)

extern void PostSoudNote( UINT cmd,UINT p1,UINT p2 );
extern int lua_SoundNote(void*p);
extern int lua_PlaySound(void*p);