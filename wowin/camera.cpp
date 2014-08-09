#include "stdafx.h"
#include "callhook.h"
#include "wowentry.h"
#include "camera.h"

/*
void GetCameraDirection( float* v ){
	if( g_eye_position )
		vector_sub(v,g_eye_lookat,g_eye_position);
}

int lua_TurnCamera( void* p ){
	float x,y;
	x = 0;
	y = 0;
	if( Lua_isnumber(p,1) ){
		x = Lua_tonumber(p,1);
	}
	if( Lua_isnumber(p,2) ){
		y = Lua_tonumber(p,2);
	}
	TurnCamera(x,y);
	FaceCamera();
	return 0;
}
*/