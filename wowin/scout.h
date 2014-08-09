#pragma once

struct UnitItem{
	unsigned int offset;
	void*		 punit_prev;
	void*		 punit_next;
};

#define UNIT_NPC	9 
#define UNIT_PLAYER 25 //玩家
#define UNIT_PLAYER_MASK -65537
#define UNIT_ITEM	33 //包括矿，药，升降梯等物体
#define UNIT_BODY	129//尸体
#define UNIT_PLAYERANDNPC 256
//template class DLLEXPORT std::vector<long long>;


struct UnitPosition{
	double x;
	double y;
	double z;
	double dis;
	UnitPosition(double x_,double y_,double z_,double d_):x(x_),y(y_),z(z_),dis(d_){}
};

struct Units{
	std::vector<long long> vUnits;
	std::vector<unsigned int>vType;
	std::vector<UnitPosition> vp;
	int	iCurrent;
	unsigned int type;
	float dis;
	float pos[3];

	bool Search( int type,float dis );
	bool Next();
	bool Prev();
	bool TargetUnit();
	bool FocusUnit(int b);
	bool FocusUnit2(double& a,double& b,double& x,double& y,double& z,double& d,unsigned int& t);
};

extern long long * g_pFocusID;
extern long long oldFocusID;
extern long long castFocusID;
extern char g_CastSpellName[];
extern char g_CastSpellTarget[];

extern int DLLEXPORT UnitType( void* punit );
//UnitItem* GetUnitItems();
int lua_FirstUnit(void*p);
int lua_NextUnit(void*p);
int lua_PrevUnit(void*p);
int lua_TargetUnit(void*p);
int lua_FocusUnit(void*p);
int lua_ResumeFocus(void*p);
int lua_SetCastSpellFocus(void*p);
int lua_SetCastSpellName(void*p);
int lua_FocusLock(void* p);
int lua_NextUnitAndFocus(void*p);

//增加一个数据收集器
int lua_LoadDiffText(void*);
int lua_AddDiffText(void*);
int lua_SaveDiffText(void*);