#include "stdafx.h"
#include "callhook.h"
#include "wowentry.h"
#include "wowin.h"
#include "waypoint.h"
#include "collide.h"
#include "render.h"
#include "movement.h"
#include "scout.h"
#include "camera.h"
#include "ai.h"
#include "bot.h"
#include "player.h"
#include "druid.h"
#include "warrior.h"
#include "paladin.h"
#include "priest.h"
#include "warlock.h"
#include "hunter.h"
#include "deathnight.h"
#include "rogue.h"
#include "mage.h"

AI* g_ai = NULL;
Player* g_player = NULL;
bool g_init = false;
int g_iClass = -1;

const char* sRace[]={
	"None",
	"Troll","Orc","Tauren","Undead","Blood Elf",
	"Night Elf","Gnome","Human","Dwarf","Dreanei"
};
const char* sClass[]={
	"None","Rogue","Mage","Death Knight","Druid",
	"Warrior","Hunter","Warlock","Paladin","Priest"
};

const char* Race( int r ){
	if( r < 0 || r > sizeof(sRace)/sizeof(const char*) )
		r = 0;
	return sRace[r];
}

const char* Class( int c ){
	if( c < 0 || c > sizeof(sClass)/sizeof(const char*) )
		c = 0;
	return sClass[c];
}

//打印单元信息
void PrintUnit( Unit* p ){
	printf("[%s,%s,%s] %.0f (%.0f,%.0f,%.0f)\n",
			ansi(p->sName).c_str(),Race(p->iRace),Class(p->iClass),
			sqrt(p->distance),p->pos[0],p->pos[1],p->pos[2] );
	printf("	lv%d hp%d/%d mp%d/%d ",
			p->iLevel,p->iHealth,p->iMaxHealth,p->iMana,p->iMaxMana );
	if( p->sTarget.empty() )
		printf("TARGET[None] ");
	else
		printf("TARGET[%s] ",ansi(p->sTarget).c_str() );
	printf("CA[%s]\n",p->bCanAttack?"Yes":"No");
	printf("	CASTING[%s] %f - %f\n",ansi(p->sSpell).c_str(),p->SpellBegin,p->SpellEnd );
	printf("	BUFF:");
	for( int i = 0;i < p->iBuff;i++ ){
		printf("%s,%d,%f->%f|",ansi(p->Buff[i].name).c_str(),p->Buff[i].count,p->Buff[i].timeLeft,p->Buff[i].duration );
	}
	printf("\n	DEBUFF:");
	for( i = 0;i < p->iDebuff;i++ ){
		printf("%s,%d,%f->%f|",ansi(p->Debuff[i].name).c_str(),p->Debuff[i].count,p->Debuff[i].timeLeft,p->Debuff[i].duration );
	}
	printf("\n");
}

//打印玩家
void PrintPlayer(){
	for( int i = 0;i < MAX_UNIT;i++ ){
		if( g_ai->vUNIT[i].type == UNIT_PLAYER )
			PrintUnit( &g_ai->vUNIT[i] );
	}
}

//打印NPC
void PrintNPC(){
	for( int i = 0;i < MAX_UNIT;i++ ){
		if( g_ai->vUNIT[i].type == UNIT_NPC )
			PrintUnit( &g_ai->vUNIT[i] );
	}
}

//打印技能
void PrintSkill( Skill* p ){
	printf("%s %s %s\n",ansi(p->name).c_str(),ansi(p->rank).c_str(),ansi(p->icon).c_str() );
	printf("	ID[%d] Cost[%d,%d] Range[%d,%d]\n",p->id,p->powerCost,p->castingTime,
		p->minRang,p->maxRang );
}

//打印玩家技能表
void PrintSkill(){
	for( std::vector<Skill>::iterator i = g_ai->vSkill.begin();
		i!=g_ai->vSkill.end();i++ ){
			PrintSkill( &(*i) );
		}
}

void PrintCooldown( Cooldown* p ){
	printf("[%d] %f->%f %s %s\n",p->spell,p->start,p->duration,p->enable?"true":"false",p->iscurrent?"true":"false");
}

//打印玩家Cooldown
void PrintCooldown(){
	for( std::vector<Cooldown>::iterator i = g_ai->vCD.begin();
		i!=g_ai->vCD.end();i++ ){
			PrintCooldown( &(*i) );
		}
}

std::string reg,sub;
//打印所在区域
void PrintRegion(){
	if( reg != g_ai->sRegion || sub != g_ai->sSubReg ){
		printf( "reg:%s,sub:%s\n",ansi(g_ai->sRegion).c_str(),ansi(g_ai->sSubReg).c_str() );
		reg = g_ai->sRegion;
		sub = g_ai->sSubReg;
	}
}

Player* AllocPlayer(AI* pai){
	if( pai == NULL )return NULL;
	if( pai->iPlayer < 0 || pai->iPlayer >= MAX_UNIT )return NULL;

	g_iClass = pai->vUNIT[pai->iPlayer].iClass;
	switch( pai->vUNIT[pai->iPlayer].iClass ){
		case ROGUE:
			return new Rogue(pai);
		case MAGE:
			return new Mage(pai);
		case DEATHNIGHT:
			return new Deathnight(pai);
		case DRUID:
			return new Druid(pai);
		case WARRIOR:
			printf("Warrior bot!\n");
			return new Warrior(pai);
		case HUNTER:
			return new Hunter(pai);
		case WARLOCK:
			return new Warlock(pai);
		case PALADIN:
			return new Paladin(pai);
		case PRIEST:
			return new Priest(pai);
		default:
			printf( "AllocPlayer Error iClass = %d!\n",pai->vUNIT[pai->iPlayer].iClass );
	}
	return NULL;
}

void FreePlayer(Player* p){
	delete p;
}

DLL_EXPORT void Think(){
	if( g_init ){
		if( g_ai == NULL )return;
		if( g_ai->iPlayer < 0 || g_ai->iPlayer >= MAX_UNIT )return;

		if( g_iClass != g_ai->vUNIT[g_ai->iPlayer].iClass ||
			g_player == NULL ){
			if( g_player )FreePlayer( g_player );
			g_player = AllocPlayer( g_ai );
		}else
			g_player->Think();
	}
}

DLL_EXPORT void ChatEvent(const char* sender_,const char* msg_,const char* channel_){
//	std::string sender(sender_),msg(msg_),channel(channel_);
//	printf("[%s]%s:%s\n",ansi(channel).c_str(),ansi(sender).c_str(),ansi(msg).c_str() );
	if( g_player )
		g_player->ChatEvent( sender_,msg_,channel_ );
}

DLL_EXPORT int Init( AI* pai ){
	if( !g_init ){
		printf("Initialize bot succeed!\n");
		g_ai = pai;
		g_player = AllocPlayer( pai );
		g_init = true;
	}else{
		printf("Initialize already!\n");
	}
	return 1;
}

DLL_EXPORT void Release(){
	if( g_init ){
		printf("Release bot succeed!\n");
		g_ai = NULL;
		if( g_player ){
			FreePlayer( g_player );
			g_player = NULL;
		}
		g_init = false;
	}
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}