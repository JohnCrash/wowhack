#pragma once

struct Triangle{
	float n[4]; //三角形法平面方程,n[0],n[1],n[2]是一个平面法向量
				//平面为方程n[0]*x + n[1]*y + n[2]*z + n[3] = 0
	float a[3]; //三角形的3个角
	float b[3];
	float c[3];
};

struct DLLEXPORT Collide{
	int	iDownMax;
	int iDownCount;
	Triangle* pDowns;
	int iDownMax2;  //用处未知

	int iUpMax;
	int iUpCount;
	Triangle* pUps;
	int iUpMax2;    //用处未知

	Collide( int down,Triangle* pdowns,int up,Triangle* pups );
};

DLLEXPORT Collide* CalcCollide( float p[6] );
int lua_IsCollide( void* p );