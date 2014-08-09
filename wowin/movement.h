#pragma once

#define READY	0x80000000
#define FORWARD	1
#define BACK	2
#define LEFT	4
#define RIGHT	8
#define TURN_LEFT 16
#define TURN_RIGHT 32
#define WALK	0x100
#define JUMP	0x1000
#define FALL	0x5000

//单独存在
#define FLYING 1

#define ISMOVING (FORWARD|BACK|LEFT|RIGHT|JUMP)

//返回值是上面的组合
unsigned long DLLEXPORT GetMovementState();

int lua_IsMoving(void*p);
int lua_TurnBlink(void*p);

#define BORDER	NULL
#define FALLOFF	1
#define IMPASSABILITY 2

#define EAST 0
#define WEST 1
#define SOUTH 2
#define NORTH 3

struct GPoint{ //地面点
	float z;
	GPoint* gp[4];
};

struct Ground{
	float x;
	float y;
	float step;
	int size;
	GPoint* pEntry; //地面入口
	GPoint* pAll; //一个大的内存块包括全部N*N的GPoint
	GPoint* pFree; //可用的连接表
	std::vector<GPoint*> Border[4]; //4个方向上的边界

	Ground();
	~Ground();
	bool InitGround( int n,float x,float y );
	void ReleaseGround();
	void ResetAllGPoint();
	GPoint* AllocGPoint();
	void ReExpand( float x,float y,float z ); //在新的位置展开
	void Move( float x,float y ); //
};
