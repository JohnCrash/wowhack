#include "stdafx.h"
#include "callhook.h"
#include "wowentry.h"
#include "soundnote.h"

HWND g_hSWnd = NULL;
const char* myWndClassName = "wowSoundClass";
const char* myWndName = "wowsound";

void PostSoudNote( UINT cmd,UINT p1,UINT p2 ){
	if( g_hSWnd==NULL||!IsWindow(g_hSWnd) ){
		g_hSWnd = FindWindow(myWndClassName,myWndName);
	}
	if( g_hSWnd ){
		PostMessage(g_hSWnd,cmd,p1,p2 );
	}
}

void PostSoundString( const char* wave,size_t len ){
	BYTE buf[64];
	if( len>=64 )return;
	memset(buf,0,64);
	memcpy(buf,wave,len);
	int count = (int)(len/8)+1;
	for( int i = 0;i < count;i++ ){
		PostSoudNote(CMD_STRING,*((UINT*)(buf+8*i)),*((UINT*)(buf+8*i+4)));
	}
}

int lua_PlaySound(void*p){
	if( Lua_isstring(p,1) ){
		size_t len;
		const char* wave = Lua_tolstring(p,1,&len);
		if( wave ){
			PostSoundString( wave,len );
			Lua_pushnumber(p,1);
			return 1;
		}
	}
	Lua_pushnil( p );
	return 1;
}

int lua_SoundNote(void*p){
	int type,p1,p2;
	float dis;

	if( Lua_isnumber(p,1) ){
		type = (int)Lua_tonumber(p,1);
		p1 = 0;
		p2 = 0;
		if( Lua_isnumber(p,2) ){
			p1 = (int)Lua_tonumber(p,2);
		}
		if( Lua_isnumber(p,3) ){
			p2 = (int)Lua_tonumber(p,3);
		}
		switch( type ){
			case 1:type = CMD_ATTACK;break;
			case 2:type = CMD_DEFANCE;break;
			case 3:type = CMD_YUN;break;
			case 4:type = CMD_ZHIYOU;break;
			case 5:type = CMD_TARGET_HEAL;break;
			case 6:type = CMD_NOTARGET;break;
			default:type = CMD_NONOTE;
		}
		PostSoudNote(type,p1,p2);
	}

	Lua_pushnil(p);
	return 1;
}