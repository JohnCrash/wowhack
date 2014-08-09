#include "stdafx.h"
#include "callhook.h"
#include "wowentry.h"
#include "wowin.h"
#include "waypoint.h"
#include "collide.h"
#include "render.h"
#include "movement.h"
#include "scout.h"
#include "ai.h"
#include "bot.h"

AI g_ai;

void Unit::Init(){
	id = 0;
	iBuff = 0;
	iDebuff = 0;
	uUpdateTime = 0;
	uStaticUpdateTime = 0;
	sTarget.clear();
	bStatic = false;
	distance = 1.7e22f;
}

AI::AI(){
	hBot = NULL;
	pfnThink = NULL;
	pfnChatEvent = NULL;
	iPlayer = 0;
	time = 0;
	iLeft = 0;
	cmdLeft = 0;
	iRight = 0;
	cmdRight = 0;
	iForward = 0;
	cmdForward = 0;
	iBack = 0;
	cmdBack = 0;
	iTurnLeft = 0;
	cmdTurnLeft = 0;
	iTurnRight = 0;
	cmdTurnRight = 0;
	iJump = 0;
	cmdJump = 0;
	cmdTarget = 0;
	cmdCastSpellID = 0;
	cmdCastSpellPet = false;
	fTurnArg = 0;
	cmdCastStop = 0;

	for( int i = 0;i < MAX_UNIT;i++ ){
		vUNIT[i].Init();
	}
}

Render* AI::GetRender(){
	return &g_Render;
}

Way* AI::GetWay(){
	return &g_way;
}

Cross* AI::GetCross(){
	return &g_cross;
}

static int EnumUnitFunc(long long id,void* param){
	if( id == 0 )return 1;

	AI* pai = (AI*)param;
	float vsub[3],pos[3],dir[3],dis;
	void* punit = GetUnit(id,1);
	
	if( punit == NULL )return 1;

	int type = UnitType(punit);
	if( type == UNIT_NPC || type == UNIT_PLAYER ){
		GetUnitPosition(punit,pos);//更新位置
		GetUnitDirection(punit,dir);
		vector_sub(vsub,pai->Pos,pos);
		dis = vector_mod2(vsub); //计算对象和玩家的距离的平方
		//看看对象还在列表中吗
		for( int i = 0;i < MAX_UNIT;i++ ){
			if( pai->vUNIT[i].id == id ){
				pai->vUNIT[i].bValid = true; //如果还在就标记一下
				vector_copy(pai->vUNIT[i].pos,pos);//更新位置
				vector_copy(pai->vUNIT[i].direct,dir); //更新朝向
				pai->vUNIT[i].distance = dis; 
				if( pai->pPlayer == punit ) //定位player在列表中的位置
					pai->iPlayer = i;
				return 1;
			}
		}
		//加入这个新的unit
		for( i = 0;i < MAX_UNIT;i++ ){
			if( pai->vUNIT[i].id == 0 ){ //找一个没有使用的slot
				pai->vUNIT[i].Init(); //因为是新的对象，标记对象没有更新
				pai->vUNIT[i].id = id;
				pai->vUNIT[i].bValid = true;
				pai->vUNIT[i].type = type; //标记对象类型
				vector_copy(pai->vUNIT[i].pos,pos);//更新位置
				vector_copy(pai->vUNIT[i].direct,dir); //更新朝向
				pai->vUNIT[i].distance = dis;
				if( pai->pPlayer == punit ) //定位player在列表中的位置
					pai->iPlayer = i;
				return 1;
			}
		}
		//如果列表已经满了,根据距离冲掉列表中的最远的
		float minDis = 0;
		int   minIndex = -1;
		for( i = 0;i < MAX_UNIT;i++ ){
			if( pai->vUNIT[i].id != 0 && pai->vUNIT[i].distance > minDis ){
				minDis = pai->vUNIT[i].distance;
				minIndex = i;
			}
		}
		if( minIndex != -1 && dis < minDis ){ //替换这个最远对象为新的对象
			pai->vUNIT[minIndex].Init(); //因为是新的对象，标记对象没有更新
			pai->vUNIT[minIndex].id = id;				
			pai->vUNIT[minIndex].bValid = true;
			pai->vUNIT[minIndex].type = type; //标记对象类型
			vector_copy(pai->vUNIT[minIndex].pos,pos);//更新位置
			vector_copy(pai->vUNIT[i].direct,dir); //更新朝向
			pai->vUNIT[minIndex].distance = dis; 
			if( pai->pPlayer == punit ) //定位player在列表中的位置
				pai->iPlayer = i;
			return 1;
		}
		//丢弃其他对象
	}
	return 1;
}

void AI::UnitSearch( float t ){
	if( t > time )time = t; //设置Lua时间

	void* pplayer = GetUnitByName(sPlayer);
	if( pplayer==NULL )return;
	pPlayer = pplayer;
	GetUnitPosition(pplayer,Pos);
	GetUnitDirection(pplayer,Direct );
	uMoveState = GetMovementState();
	frameTickCount = GetTickCount();

	for( int i = 0;i < MAX_UNIT;i++ ){
		vUNIT[i].bValid = false;
	}
	EnumUnit(EnumUnitFunc,(void*)this);
	for( int i = 0;i < MAX_UNIT;i++ ){
		if( !vUNIT[i].bValid ) //腾出没有使用的slot
			vUNIT[i].id = 0;
	}
	iCurrent = 0;
	//这里检查移动状态
	iLeft = uMoveState&LEFT;
	iRight = uMoveState&RIGHT;
	iForward = uMoveState&FORWARD;
	iBack = uMoveState&BACK;
	iTurnLeft = uMoveState&TURN_LEFT;
	iTurnRight = uMoveState&TURN_RIGHT;
	iJump = uMoveState&JUMP;

	oldFocusID = *g_pFocusID;
}

void AI::ResumeFocus(){
	*g_pFocusID = oldFocusID;
	//==========================================
	//这里打印调试信息
	/*
	printf("================================================\n");
	for( int i = 0;i < MAX_UNIT;i++ ){
		if( vUNIT[i].bValid && vUNIT[i].id != 0 ){
			if( i == iPlayer )
				printf("*Buff(%d) Debuff(%d) Lv(%d) HP(%d) MP(%d) Casting(%s)\n",
				vUNIT[i].iBuff,vUNIT[i].iDebuff,
						vUNIT[i].iLevel,vUNIT[i].iHealth,vUNIT[i].iMana,
						vUNIT[i].sSpell.empty()?"NONE":"CAST" );
			else
				printf(" Buff(%d) Debuff(%d) Lv(%d) HP(%d) MP(%d) Casting(%s)\n",
				vUNIT[i].iBuff,vUNIT[i].iDebuff,
						vUNIT[i].iLevel,vUNIT[i].iHealth,vUNIT[i].iMana,
						vUNIT[i].sSpell.empty()?"NONE":"CAST" );
		}
	}*/
}

int AI::IsUpdate( int i ){
	if( i >= MAX_UNIT || i < 0 )return 0; //不正确的参数

	if( vUNIT[i].bValid && vUNIT[i].id != 0 ){
		iCurrent = i;

		*g_pFocusID = vUNIT[i].id; //设置当前焦点对象为

		//静态数据也进行周期更新，解决未知对象的问题
		if( !vUNIT[i].bStatic || frameTickCount-vUNIT[i].uStaticUpdateTime >1000 ){
			return 1; //需要完全更新
		}
		if( vUNIT[i].type == UNIT_NPC ){
			if( vUNIT[i].distance > YARD40 ){
				if( frameTickCount - vUNIT[i].uUpdateTime > 500 ) //距离大于40码的NPC数据每1秒更新一次
					return 2; //仅需要更新动态数据
			}else{
				if( frameTickCount - vUNIT[i].uUpdateTime > 250 ) //距离小于40码的NPC数据每.5秒更新一次
					return 2;
			}
		}else if( vUNIT[i].type == UNIT_PLAYER ){
			if( vUNIT[i].distance > YARD40 ){
				if( frameTickCount - vUNIT[i].uUpdateTime > 200 ) //距离大于40码的玩家数据每.2秒更新一次
					return 2;
			}else{
				return 2; //距离小于40码的实时更新
			}
		}
	}
	iCurrent = NONE;
	return 0; //不需要更新
}

void AI::UpdateStatic( std::string name,int race,int clas,int level,
					   bool bwarring,bool canattack ){ //更新静态数据，相对不变的数据
	if( iCurrent >= MAX_UNIT || iCurrent<0 )return;

	vUNIT[iCurrent].bStatic = true; //表示静态数据已经更新
	vUNIT[iCurrent].sName = name;
	vUNIT[iCurrent].iRace = race;
	vUNIT[iCurrent].iClass = clas;
	vUNIT[iCurrent].iLevel = level;
	vUNIT[iCurrent].bWarring = bwarring;
	vUNIT[iCurrent].bCanAttack = canattack;

	vUNIT[iCurrent].uStaticUpdateTime = frameTickCount;
}

void AI::UpdateState( bool ghost,bool death,bool combat ){
	if( iCurrent >= MAX_UNIT || iCurrent<0 )return;
	
	vUNIT[iCurrent].bGhost = ghost;
	vUNIT[iCurrent].bDeath = death;
	vUNIT[iCurrent].bCombat = combat;
}

void AI::UpdateHealth(int health,int mana,int maxhealth,int maxmana){
	if( iCurrent >= MAX_UNIT || iCurrent<0 )return;

	vUNIT[iCurrent].iHealth = health;
	vUNIT[iCurrent].iMana = mana;
	vUNIT[iCurrent].iMaxHealth = maxhealth;
	vUNIT[iCurrent].iMaxMana = maxmana;

	vUNIT[iCurrent].uUpdateTime = frameTickCount;
}

void AI::UpdateCasting( std::string spell,unsigned int begin,unsigned int end ){
	if( iCurrent >= MAX_UNIT || iCurrent<0 )return;

	vUNIT[iCurrent].sSpell = spell;
	vUNIT[iCurrent].SpellBegin = (float)begin/1000.0f; //将毫秒转换为妙
	vUNIT[iCurrent].SpellEnd = (float)end/1000.0f;
}

void AI::UpdateTarget( std::string target ){
	if( iCurrent >= MAX_UNIT || iCurrent<0 )return;

	vUNIT[iCurrent].sTarget = target;
}

void AI::UpdateBuff( std::string name,int count,float timeleft,float duration,int i ){
	if( iCurrent >= MAX_UNIT || iCurrent<0 )return;
	if( i < 0 || i >= 32 ){
		//让脚本，使用一个超出的索引来清除Buff
		vUNIT[iCurrent].iBuff=0;
		return;
	}

	vUNIT[iCurrent].iBuff = i+1;
	vUNIT[iCurrent].Buff[i].name = name;
	vUNIT[iCurrent].Buff[i].count = count;
	vUNIT[iCurrent].Buff[i].timeLeft = timeleft;
	vUNIT[iCurrent].Buff[i].duration = duration;
}

void AI::UpdateDebuff( std::string name,int count,float timeleft,float duration,int i ){
	if( iCurrent >= MAX_UNIT || iCurrent<0 )return;
	if( i < 0 || i >= 32 ){
		vUNIT[iCurrent].iDebuff = 0;
		return;
	}

	vUNIT[iCurrent].iDebuff = i+1;
	vUNIT[iCurrent].Debuff[i].name = name;
	vUNIT[iCurrent].Debuff[i].count = count;
	vUNIT[iCurrent].Debuff[i].timeLeft = timeleft;
	vUNIT[iCurrent].Debuff[i].duration = duration;
}

Skill::Skill(){
}

Skill::Skill( std::string n,std::string r,std::string icon_,
			int id_,int powertype,int powercost,int minrang,int maxrang,
			int castingtime,bool isfunnel){
	name = n;
	rank = r;
	icon = icon_;
	id = id_;
	powerType = powertype;
	powerCost = powercost;
	minRang = minrang;
	maxRang = maxrang;
	castingTime = castingtime<0?0:castingtime;//发现有些应该是0的为-999500可能是blz的bug
	bPet = isfunnel;
}

void AI::ClearSkill(){
	vSkill.clear();
}

void AI::AddSkill( std::string skill,std::string rank,std::string icon,
					int id,int powertype,int powercost,int minrang,int maxrang,
					int castingtime,bool isfunnel ){
	vSkill.push_back( Skill(skill,rank,icon,
		id,powertype,powercost,minrang,maxrang,
		castingtime,isfunnel) );
}

void AI::Think(){
	__try{
		if( hBot && pfnThink )
			((t_Think)pfnThink)();
	}__except(1){
		printf("Think Exception!\n");
	}
}

static void sInitBot( t_Init pfnInit,AI* this_ ){
	__try{
		pfnInit(this_);
	}__except(1){
		printf("Bot init Exception!\n");
	}
}

bool AI::LoadBot( std::string bot ){
	if( bot.empty() )return false;
	FreeBot(); //先卸载当前AI
	
	hBot = LoadLibrary( bot.c_str() );
	if( hBot ){
		pfnThink = GetProcAddress( hBot,"Think" );
		pfnChatEvent = GetProcAddress( hBot,"ChatEvent" );
		t_Init pfnInit = (t_Init)GetProcAddress( hBot,"Init" );
		if( pfnInit ){
			sInitBot(pfnInit,this);
		}else
			printf("Can't local Init proc in %s\n",bot.c_str() );
		return true;
	}else printf("Can't find %s\n",bot.c_str() );

	return false;
}

void AI::FreeBot(){
	if( hBot ){
		t_Release pfnRelease = (t_Release)GetProcAddress( hBot,"Release" );
		if( pfnRelease ){
			__try{
				pfnRelease();
			}__except(1){
				printf("Bot release Exception!\n");
			}
		}
		FreeLibrary( hBot );
	}
	hBot = NULL;
	pfnThink = NULL;
	pfnChatEvent = NULL;
}

Cooldown::Cooldown(){
}

Cooldown::Cooldown(int s,bool p ){
	spell = s;
	pet = p;
}

int AI::GetCooldownListSize(){
	return vCD.size();
}

int AI::GetCooldownSpell(int i){
	if( i >= 0 && i < vCD.size() )
		return vCD.at(i).spell;
	else
		return 0;
}

bool AI::GetCooldownPet(int i){
	if( i >= 0 && i < vCD.size() )
		return vCD.at(i).pet;
	else
		return 0;
}

void AI::UpdateCooldown(int i,float start,float duration,bool enable,bool iscurr){
	if( i >= 0 && i < vCD.size() ){
		Cooldown& cd = vCD.at(i);
		cd.start = start;
		cd.duration = duration;
		cd.enable = enable;
		cd.iscurrent = iscurr;
	}
}

void AI::AddCooldownTrack( int spell,bool pet ){
	vCD.push_back( Cooldown(spell,pet) );
}

void AI::ClearCooldownTrack(){
	vCD.clear();
}

void AI::MoveLeft(){
	if( iLeft )return;
	
	if( cmdRight )
		cmdRight = 0;
	if( iRight )
		cmdRight = 2; //停止向右

	cmdLeft = 1; //开始向左
}

void AI::MoveRight(){
	if( iRight )return;

	if( cmdLeft )
		cmdLeft = 0;
	if( iLeft )
		cmdLeft = 2;
		
	cmdRight = 1;
}

void AI::MoveForward(){
	if( iForward )return;

	if( cmdBack ) //和后退不能同时发出
		cmdBack = 0;
	if( iBack ) //如果在后退发出停止命令
		cmdBack = 2;
	cmdForward = 1;
}

void AI::MoveBack(){
	if( iBack )return;

	if( cmdForward ) //和前进不能同时发出
		cmdForward = 0;
	if( iForward ) //如果在前进发出停止命令
		cmdForward = 2;
	cmdBack = 1;
}

void AI::MoveJump(){
	if( iJump )return;

	cmdJump = 1;
}

void AI::MoveTurnLeft(){
	if( iTurnLeft )return;

	if( cmdTurnRight )
		cmdTurnRight = 0;
	if( iTurnRight )
		cmdTurnRight = 2;

	cmdTurnLeft = 1;
}

void AI::MoveTurnRight(){
	if( iTurnRight )return;

	if( cmdTurnLeft )
		cmdTurnLeft = 0;
	if( iTurnLeft )
		cmdTurnLeft = 2;

	cmdTurnRight = 1;
}

void AI::MoveLRStop(){
	cmdLeft = 0;
	cmdRight = 0;
	if( iLeft )
		cmdLeft = 2;
	if( iRight )
		cmdRight = 2;
}

void AI::MoveFBStop(){
	cmdForward = 0;
	cmdBack = 0;
	if( iForward )
		cmdForward = 2;
	if( iBack )
		cmdBack = 2;
}

void AI::MoveTurnStop(){
	cmdTurnLeft = 0;
	cmdTurnRight = 0;
	if( iTurnLeft )
		cmdTurnLeft = 2;
	if( iTurnRight )
		cmdTurnRight = 2;
}

void AI::MoveTurn( float a ){
	fTurnArg = a;
}

void AI::UpdateRegion( std::string reg,std::string sub ){
	sRegion = reg;
	sSubReg = sub;
}

void AI::Target(long long id){
	*g_pFocusID = id;
	cmdTarget = 1;
}

void AI::CastSpell(int id,bool pet){
	cmdCastSpellID = id;
	cmdCastSpellPet = pet;
}

void AI::SpellStopCasting(){
	cmdCastStop = 1;
}

void AI::ChatEvent( const char* sender,const char* msg,const char* channel ){
	__try{
		if( hBot && pfnChatEvent ){
			((t_ChatEvent)pfnChatEvent)( sender,msg,channel );
		}
	}__except(1){
		printf("ChatEvent Exception!\n");
	}
}

int lua_AIChatEvent(void*p){
	const char* sender;
	const char* msg;
	const char* channel;
	sender = NULL;
	msg = NULL;
	channel = NULL;
	if( Lua_isstring(p,1) ){
		size_t len;
		sender = Lua_tolstring(p,1,&len);
	}
	if( Lua_isstring(p,2) ){
		size_t len;
		msg = Lua_tolstring(p,2,&len);
	}
	if( Lua_isstring(p,3) ){
		size_t len;
		channel = Lua_tolstring(p,3,&len);
	}

	g_ai.ChatEvent(sender,msg,channel);
	return 0;
}

int lua_AIAction(void*p){
	Lua_pushnumber(p,g_ai.cmdTarget);
	Lua_pushnumber(p,g_ai.cmdCastSpellID);
	if( g_ai.cmdCastSpellPet )
		Lua_pushnumber(p,1);
	else
		Lua_pushnumber(p,0);
	Lua_pushnumber(p,g_ai.cmdCastStop);

	g_ai.cmdCastStop = 0;
	g_ai.cmdTarget = 0;
	g_ai.cmdCastSpellID = 0;
	g_ai.cmdCastSpellPet = false;
	return 4;
}

int lua_AIUpdateRegion(void*p){
	std::string reg,sub;
	if( Lua_isstring(p,1) ){
		size_t len;
		reg = Lua_tolstring(p,1,&len);
	}
	if( Lua_isstring(p,2) ){
		size_t len;
		sub = Lua_tolstring(p,2,&len);
	}
	g_ai.UpdateRegion( reg,sub );
	return 0;
}

int lua_AITurn(void*p){
	if( g_ai.fTurnArg != 0 ){
		Lua_pushnumber(p,g_ai.fTurnArg);
		g_ai.fTurnArg = 0;
	}else{
		Lua_pushnil(p);
	}
	return 1;
}

int lua_AIMove(void*p){
	Lua_pushnumber(p,g_ai.cmdLeft);
	Lua_pushnumber(p,g_ai.cmdRight);
	Lua_pushnumber(p,g_ai.cmdForward);
	Lua_pushnumber(p,g_ai.cmdBack);
	Lua_pushnumber(p,g_ai.cmdTurnLeft);
	Lua_pushnumber(p,g_ai.cmdTurnRight);
	Lua_pushnumber(p,g_ai.cmdJump);

	if( g_ai.cmdLeft == 1 ) //如果左移
		g_ai.iLeft = 1; //状态改成移动状态
	else if( g_ai.cmdLeft == 2 )
		g_ai.iLeft = 0; //状态改成停止状态

	g_ai.cmdLeft = 0;
	g_ai.cmdRight = 0;
	g_ai.cmdForward = 0;
	g_ai.cmdBack = 0;
	g_ai.cmdTurnLeft = 0;
	g_ai.cmdTurnRight = 0;
	g_ai.cmdJump = 0;

	return 7;
}

int lua_AIGetCooldownListSize(void*p){
	Lua_pushnumber(p,g_ai.GetCooldownListSize());
	return 1;
}

int lua_AIGetCooldownSpell(void*p){
	int i = 0;
	if( Lua_isnumber(p,1) ){
		i = (int)Lua_tonumber(p,1);
		int spell = g_ai.GetCooldownSpell(i);
		if( spell ){
			Lua_pushnumber(p,spell);
		}
		bool pet = g_ai.GetCooldownPet(i);
		if( pet ){
			Lua_pushnumber(p,1);
			return 2;
		}else{
			Lua_pushnil(p);
			return 2;
		}
	}
	Lua_pushnil(p);
	return 1;
}

int lua_AIUpdateCooldown(void*p){
	int i = 0;
	float start = 0;
	float duration = 0;
	bool enable = false;
	bool iscurr = false;

	if( Lua_isnumber(p,1) ){
		i = (int)Lua_tonumber(p,1);
	}
	if( Lua_isnumber(p,2) ){
		start = (float)Lua_tonumber(p,2);
	}
	if( Lua_isnumber(p,3) ){
		duration = (float)Lua_tonumber(p,3);
	}
	if( Lua_isnumber(p,4) ){
		int en = (int)Lua_tonumber(p,4);
		if( en == 1 )enable = true;
	}
	if( Lua_isnumber(p,5) ){
		int en = (int)Lua_tonumber(p,5);
		if( en == 1 )iscurr = true;
	}

	g_ai.UpdateCooldown(i,start,duration,enable,iscurr);
	return 0;
}

int lua_AILoadBot(void*p){
	std::string dll;
	
	if( Lua_isstring(p,1) ){
		size_t len;
		dll = Lua_tolstring(p,1,&len);
		if( g_ai.LoadBot( dll ) ){
			Lua_pushnumber(p,1);
			return 1;
		}
	}
	Lua_pushnil(p);
	return 1;
}

int lua_AIFreeBot(void*p){
	g_ai.FreeBot();
	return 0;
}

int lua_AIThink(void*p){
	g_ai.Think();
	return 0;
}

int lua_AIBotIsReady(void*p){
	if( g_ai.pfnThink )
		Lua_pushnumber(p,1);
	else
		Lua_pushnil(p);
	return 1;
}

int lua_AIClearSkill(void*p){
	g_ai.ClearSkill();
	return 0;
}

int lua_AIAddSkill(void*p){
	std::string skill,rank,icon;
	int id,powertype,powercost;
	int minrang,maxrang,castingtime;
	bool isfunnel;

	id = 0;
	powertype = 0;
	powercost = 0;
	minrang = 0;
	maxrang = 0;
	castingtime = 0;
	isfunnel = false;

	if( Lua_isstring(p,1) ){
		size_t len;
		skill = Lua_tolstring(p,1,&len);
	}
	if( Lua_isstring(p,2) ){
		size_t len;
		rank = Lua_tolstring(p,2,&len);
	}
	if( Lua_isstring(p,3) ){
		size_t len;
		icon = Lua_tolstring(p,3,&len);
	}
	if( Lua_isnumber(p,4) )
		id = (int)Lua_tonumber(p,4);
	if( Lua_isnumber(p,5) )
		powertype = (int)Lua_tonumber(p,5);
	if( Lua_isnumber(p,6) )
		powercost = (int)Lua_tonumber(p,6);
	if( Lua_isnumber(p,7) )
		minrang = (int)Lua_tonumber(p,7);
	if( Lua_isnumber(p,8) )
		maxrang = (int)Lua_tonumber(p,8);
	if( Lua_isnumber(p,9) )
		castingtime = (int)Lua_tonumber(p,9);
	if( Lua_isnumber(p,10) ){
		int isf = (int)Lua_tonumber(p,10);
		isfunnel = isf==1?true:false;
	}

	g_ai.AddSkill( skill,rank,icon,
		id,powertype,powercost,minrang,maxrang,
		castingtime,isfunnel );
	return 0;
}

int lua_UnitSearch(void*p){
	float time = 0;

	if( Lua_isnumber(p,1) ){
		time = (float)Lua_tonumber(p,1);
	}
	g_ai.UnitSearch( time );
	return 0;
}

int lua_IsUpdate(void*p){
	int reti = 0;
	if( Lua_isnumber(p,1) ){
		int index = (int)Lua_tonumber(p,1);
		reti = g_ai.IsUpdate(index);
	}
	Lua_pushnumber(p,(double)reti);
	return 1;
}

int lua_UpdateStatic(void*p){
	bool isok = true;
	const char* name;
	int race = UNKNOW;
	int clas = UNKNOW;
	int level = 0;
	int bwarring = 0;
	int bcanattack = 0;

	if( Lua_isstring(p,1) ){
		size_t len;
		name = Lua_tolstring(p,1,&len);
	}else{
		printf("lua_UpdateStatic param invalid,Name=nil\n");
		Lua_pushnil(p);
		return 1;
	}
	if( Lua_isnumber(p,2) ){
		race = (int)Lua_tonumber(p,2);
	}
	if( Lua_isnumber(p,3) ){
		clas = (int)Lua_tonumber(p,3);
	}
	if( Lua_isnumber(p,4) ){
		level = (int)Lua_tonumber(p,4);
	}
	if( Lua_isnumber(p,5) ){
		bwarring = (int)Lua_tonumber(p,5);
	}
	if( Lua_isnumber(p,6) ){
		bcanattack = (int)Lua_tonumber(p,6);
	}
	g_ai.UpdateStatic(name,race,clas,level,
		bwarring==1?true:false,bcanattack==1?true:false);
	Lua_pushnumber(p,1);
	return 1;
}

int lua_UpdateHealth(void*p){
	bool isok = true;
	int health;
	int mana;
	int maxhealth = 0;
	int maxmana = 0;

	if( Lua_isnumber(p,1) ){
		health= (int)Lua_tonumber(p,1);
	}else isok = false;
	if( Lua_isnumber(p,2) ){
		mana = (int)Lua_tonumber(p,2);
	}else isok = false;

	if( Lua_isnumber(p,3) ){
		maxhealth = (int)Lua_tonumber(p,3);
	}
	if( Lua_isnumber(p,4) ){
		maxmana = (int)Lua_tonumber(p,4);
	}

	if( isok )
		g_ai.UpdateHealth( health,mana,maxhealth,maxmana );
	else
		printf("lua_UpdateHealth param invalid\n");

	return 0;
}

int lua_UpdateState(void*p){
	bool isok = true;
	int ghost = 0;
	int death = 0;
	int combat = 0;

	if( Lua_isnumber(p,1) ){
		ghost = (int)Lua_tonumber(p,1);
	}
	if( Lua_isnumber(p,2) ){
		death = (int)Lua_tonumber(p,2);
	}
	if( Lua_isnumber(p,3) ){
		combat = (int)Lua_tonumber(p,3);
	}

	g_ai.UpdateState( ghost==1?true:false,death==1?true:false,combat==1?true:false );

	return 0;
}

int lua_UpdateCasting(void*p){
	std::string name;
	unsigned int begin = 0;
	unsigned int end = 0;

	if( Lua_isstring(p,1) ){
		size_t len;
		name = Lua_tolstring(p,1,&len);
	}
	if( Lua_isnumber(p,2) ){
		begin = (unsigned int)Lua_tonumber(p,2);
	}
	if( Lua_isnumber(p,3) ){
		end = (unsigned int)Lua_tonumber(p,3);
	}

	g_ai.UpdateCasting( name,begin,end );
	return 0;
}

int lua_UpdateTarget(void*p){
	std::string name;

	if( Lua_isstring(p,1) ){
		size_t len;
		name = Lua_tolstring(p,1,&len);
	}
	g_ai.UpdateTarget( name );
	return 0;
}

int lua_AIResumeFocus(void*p){
	g_ai.ResumeFocus();
	return 0;
}

int lua_UpdateBuff(void*p){
	std::string name;
	int count = 0;
	float timeleft = 0;
	float duration = 0;
	int i = 0;

	if( Lua_isstring(p,1) ){
		size_t len;
		name = Lua_tolstring(p,1,&len);
	}
	if( Lua_isnumber(p,2) ){
		count = (int)Lua_tonumber(p,2);
	}
	if( Lua_isnumber(p,3) ){
		timeleft = (float)Lua_tonumber(p,3);
	}
	if( Lua_isnumber(p,4) ){
		duration = (float)Lua_tonumber(p,4);
	}
	if( Lua_isnumber(p,5) ){
		i = (int)Lua_tonumber(p,5);
	}

	g_ai.UpdateBuff( name,count,timeleft,duration,i );
	return 0;
}

int lua_UpdateDebuff(void*p){
	std::string name;
	int count = 0;
	float timeleft = 0;
	float duration = 0;
	int i = 0;

	if( Lua_isstring(p,1) ){
		size_t len;
		name = Lua_tolstring(p,1,&len);
	}
	if( Lua_isnumber(p,2) ){
		count = (int)Lua_tonumber(p,2);
	}
	if( Lua_isnumber(p,3) ){
		timeleft = (float)Lua_tonumber(p,3);
	}
	if( Lua_isnumber(p,4) ){
		duration = (float)Lua_tonumber(p,4);
	}
	if( Lua_isnumber(p,5) ){
		i = (int)Lua_tonumber(p,5);
	}

	g_ai.UpdateDebuff( name,count,timeleft,duration,i );
	return 0;
}
