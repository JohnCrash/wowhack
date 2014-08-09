#include "stdafx.h"
#include "callhook.h"
#include "wowentry.h"
#include "dllentry.h"
#include "movement.h"

extern const char* sPlayer;

int lua_IsMoving(void*p){
	if( GetMovementState()&ISMOVING ){
		Lua_pushnumber(p,1);
		return 1;
	}
	Lua_pushnil(p);
	return 1;
}

//Ë²¼äÐý×ª
int lua_TurnBlink(void*p){
	Lua_pushnil(p);
	return 1;
}

Ground::Ground(){
	pAll = NULL;
	pEntry = NULL;
	pFree = NULL;
	size = 0;
}

Ground::~Ground(){
	ReleaseGround();
}

void Ground::ResetAllGPoint(){
	if( pAll ){
		pFree = pAll;
		pEntry = BORDER;
		for( int i = 0;i < size-1;i++ ){
			pFree[i].gp[0] = &pFree[i+1];
			pFree[i].gp[1] = BORDER;
			pFree[i].gp[2] = BORDER;
			pFree[i].gp[3] = BORDER;
		}
		pFree[i].gp[0] = BORDER;
		pFree[i].gp[1] = BORDER;
		pFree[i].gp[2] = BORDER;
		pFree[i].gp[3] = BORDER;
	}
}

bool Ground::InitGround( int n,float x_,float y_ ){
	x = x_;
	y = y_;
	step = 1;
	size = n*n;

	ReleaseGround();
	pAll = new GPoint[n*n];
	if( pAll ){
		ResetAllGPoint();
		return true;
	}
	return false;
}

void Ground::ReleaseGround(){
	if( pAll ){
		delete pAll;
		pAll = NULL;
		pEntry = NULL;
		pFree = NULL;
	}
}

GPoint* Ground::AllocGPoint(){
	return NULL;
}

void Ground::ReExpand( float x_,float y_,float z_ ){
	x = x_;
	y = y_;
	ResetAllGPoint();
	pEntry = AllocGPoint();
	if( pEntry ){
		pEntry->z = z_;
	}
}
