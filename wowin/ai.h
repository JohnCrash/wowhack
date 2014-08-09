#pragma once

#define NONE -1
#define MAX_UNIT 32
#define YARD40 800.0f

#define UNKNOW 0
//职业
#define ROGUE	1
#define MAGE	2
#define DEATHNIGHT	3
#define DRUID	4
#define WARRIOR	5
#define HUNTER	6
#define WARLOCK	7
#define PALADIN	8
#define PRIEST	9

//种族
#define TROLL	1
#define ORC	2
#define TAUREN	3
#define UNDEAD	4
#define BLOODELF	5
#define NIGHTELF	6
#define GNOME	7
#define HUMAN	8
#define DWARF	9
#define DREANEI	10

#define POWER_HEALTH -2
#define POWER_MANA	0
#define POWER_RAGE	1
#define POWER_FOCUS	2
#define POWER_ENERGY	3

//技能
struct DLLEXPORT Skill{
	std::string name; //名称
	std::string rank; //级别,直接来自于魔兽没有分解
	std::string icon; //技能图标
	int id; //技能在技能书中的位置
	int powerType; //施法需要的类型-2生命,0Mana,1Rage,2Focus,3Energy
	int powerCost; //释放消耗
	int minRang;//技能最小距离
	int maxRang;//技能最大距离
	int castingTime;//施法需要的时间,毫秒
	bool bPet;//Pet

	Skill();
	Skill( std::string n,std::string r,std::string icon_,
			int id_,int powertype,int powercost,int minrang,int maxrang,
			int castingtime,bool bpet);
};

//Buff or Debuff
struct DLLEXPORT BUFF{
	std::string name; //名称
	int count;//层数

	float timeLeft;//结束时间,单位秒
	float duration;//可以持续的总时间,单位妙
};

//递交给脚本用来侦测的魔法或者slot的Cooldown
struct DLLEXPORT Cooldown{
	int spell; //魔法索引,和Skill中的id具有相同含义,IN
	bool pet; //是不是宠物魔法,IN
	float start; //开始冷却时间,单位是妙,OUT
	float duration;//持续时间,OUT
	bool enable; //是否在冷却中,OUT
	bool iscurrent; //是不是被选中
	Cooldown();
	Cooldown(int s,bool p );
};

//对象，包括玩家和NPC,宠物
struct DLLEXPORT Unit{
	//下面可以直接取得
	bool bValid;   //对象是否有效
	long long id; //玩家id
	float pos[3]; //对象位置
	float direct[3];//对象方向
	float distance;//和玩家距离的平方
	int   type; //类型NPC,或者玩家

	//下面通过lua取得
	unsigned int uStaticUpdateTime;//静态数据更新周期,毫秒GetTickCount基于
	unsigned int uUpdateTime; //对象数据被更新的时间
	bool bStatic; //静态数据没有更新
	std::string sName;//名称
	int iRace; //种族
	int iClass;//职业
	int iLevel;//级别
	int iMaxHealth; //最大生命
	int iMaxMana; //最大魔法值
	bool bCanAttack; //目标是否可以攻击

	int iHealth;    //目前生命
	int iMana;	//魔法值
	std::string sTarget;//对象目标名称
	std::string sSpell; //正在释放的魔法
	float SpellBegin; //魔法释放开始时间,单位秒
	float SpellEnd;	//魔法释放结束时间,单位秒
	bool bWarring; //敌对
	bool bPlayer; //是不是玩家
	bool bCombat; //是否战斗状态
	bool bDeath; //是不是死亡
	bool bGhost; //是不是亡灵状态
	bool bCasting;//是否在施法
	int  iBuff; //buff数量
	int  iDebuff;//Debuff数量
	BUFF Buff[32];
	BUFF Debuff[32];

	void Init();
};

#define CMD_NONE 0
//下面命令用来更新角色数据
#define CMD_UPDATE_SKILL 1
#define CMD_UPDATE_STATIC 2

//下面命令用来控制角色移动
#define CMD_LEFT 3 //左平移
#define CMD_LEFT_STOP 4
#define CMD_RIGHT 5 //右平移
#define CMD_RIGHT_STOP 6
#define CMD_FORWARD 7 //前进
#define CMD_FORWARD_STOP 8
#define CMD_BACK 9 //后退
#define CMD_BACK_STOP 10
#define CMD_TURNLEFT 11 //左转
#define CMD_TURNLEFT_STOP 12
#define CMD_TURNRIGHT 13//右转
#define CMD_TURNRIGHT_STOP 14
#define CMD_JUMP 15 //跳跃
#define CMD_JUMP_STOP 16
#define CMD_TOGGLE_RUN 17 //一直向前

//下面命令用来释放技能
#define CMD_CASTSPELL 18 //施法魔法

struct DLLEXPORT AI{
	//数据区
	//======
	std::string sRegion; //玩家所在区域
	std::string sSubReg; //玩家所在子区域
	std::vector<Skill> vSkill; //角色的全部技能
	std::vector<Cooldown> vCD; //一个需要侦测的技能冷却表
	int iPlayer; //玩家对象索引
	float Pos[3];  //玩家位置
	float Direct[3];//玩家方向
	unsigned long uMoveState;//玩家移动状态
	int iCurrent; //仅用于更新状态时的临时数据
	float time;//Lua GetTime的时间值,单位秒
	Unit vUNIT[MAX_UNIT]; //对象数目

	//对象移动的控制状态
	//==================
	int iLeft,cmdLeft; //0 没动作 1 Left 2 Left_stop
	int iRight,cmdRight;
	int iForward,cmdForward;
	int iBack,cmdBack;
	int iTurnLeft,cmdTurnLeft;
	int iTurnRight,cmdTurnRight;
	int iJump,cmdJump;
	float fTurnArg;
	int cmdTarget; //发出选择目标的命令
	int cmdCastSpellID;//发出施法魔法的命令
	bool cmdCastSpellPet;
	int cmdCastStop;//停止施法

	HMODULE hBot; //bot动态库句柄
	PROC pfnThink; //bot思考函数
	PROC pfnChatEvent; //聊天事件

	//临时数据区
	//===========
	long long oldFocusID;
	unsigned int frameTickCount;
	void* pPlayer; //仅使用来发现iPlayer
	//===========

	//函数区域
	//==========
	AI();
	//下面的函数都是Lua调用，用来收集游戏数据的
	//=========================================
	void UnitSearch( float time ); //搜索周围的Unit,这一来收集的开始时间Lua GetTime
	int IsUpdate( int i ); //第i个对象是否需要更新

	void UpdateStatic( std::string name,int race,int clas,int level,bool bwarring,bool canattack); //更新静态数据，相对不变的数据
	//更新状态,包括生命，魔法
	void UpdateHealth( int health,int mana,int maxhealth,int maxmana );
	//更新区域
	void UpdateRegion( std::string reg,std::string sub );
	//更新是否鬼魂，是否死亡，是否坐骑
	void UpdateState( bool ghost,bool death,bool taxi );
	//更新施法状态，魔法名称，施法时间，结束时间
	void UpdateCasting( std::string spell,unsigned int begin,unsigned int end );
	//更新对象的目标
	void UpdateTarget( std::string target );
	//更新目标身上的Buff
	void UpdateBuff( std::string buff,int count,float timeleft,float duration,int i );
	//更新目标身上的Debuff
	void UpdateDebuff( std::string buff,int count,float timeleft,float duration,int i );
	//恢复焦点目标
	void ResumeFocus();
	
	//下面的代码让Lua调用来更新技能冷却情况
	//=====================================
	int GetCooldownListSize(); //取得vCD尺寸
	int GetCooldownSpell(int i); //取得第i个Cooldown的Spell
	bool GetCooldownPet(int i);
	void UpdateCooldown(int i,float start,float duration,bool enable,bool iscurr); //更新第i个Cooldown信息

	//下面函数Lua调用，用来收集玩家角色信息
	//=====================================
	//更新玩家的技能信息
	void ClearSkill(); //清除技能
	void AddSkill( std::string sill,std::string rank,std::string icon,
					int id,int powertype,int powercost,int minrang,int maxrang,
					int castingtime,bool isfunnel ); //加入一个可用技能
	//下面函数Lua调用，用来监听聊天
	//=====================================
	void ChatEvent( const char* sender,const char* msg,const char* channel );

	void Think(); //Lua用这个函数来启动AI

	//C++ bot 动态库控制
	//==================
	bool LoadBot( std::string bot ); //载入一个ai
	void FreeBot(); //卸载当前ai

	//C++调用用来加入Cooldown
	void AddCooldownTrack( int spell,bool pet );
	void ClearCooldownTrack();
	//C++调用用来移动角色
	//===================
	void MoveLeft(); //左平移
	void MoveRight();//右平移
	void MoveForward(); //向前
	void MoveBack(); //后退
	void MoveJump(); //跳
	void MoveTurnLeft(); //左转
	void MoveTurnRight();//右转
	void MoveLRStop(); //停止左右移动
	void MoveFBStop(); //停止前后移动
	void MoveTurnStop(); //停止转动
	void MoveTurn( float a ); //立刻旋转一个角度
	//C++调用用来选择目标和释放法术
	//=============================
	void Target(long long id);//设置单位为你的目标
	void CastSpell(int id,bool pet); //释放一个魔法，b=true表示是宠物魔法
	void SpellStopCasting(); //停止释放

	Render* GetRender();
	Way*	GetWay();
	Cross*	GetCross();
};

/*
template class DLLEXPORT std::vector<Cooldown>;
template class DLLEXPORT std::vector<Skill>;
*/

int lua_UnitSearch(void*);
int lua_IsUpdate(void*);
int lua_UpdateStatic(void*);
int lua_UpdateHealth(void*p);
int lua_UpdateState(void*);
int lua_UpdateCasting(void*);
int lua_UpdateTarget(void*);
int lua_UpdateBuff(void*);
int lua_UpdateDebuff(void*);
int lua_AIResumeFocus(void*p);
int lua_AIClearSkill(void*p);
int lua_AIAddSkill(void*p);
int lua_AILoadBot(void*);
int lua_AIFreeBot(void*);
int lua_AIThink(void*);
int lua_AIGetCooldownListSize(void*p);
int lua_AIGetCooldownSpell(void*p);
int lua_AIUpdateCooldown(void*p);
int lua_AIMove(void*);
int lua_AITurn(void*p);
int lua_AIUpdateRegion(void*p);
int lua_AIAction(void*p);
int lua_AIBotIsReady(void*p);
int lua_AIChatEvent(void*p);