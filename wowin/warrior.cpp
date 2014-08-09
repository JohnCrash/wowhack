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
#include <strstream>
#include "warrior.h"

Warrior::Warrior( AI* ai ):Player(ai){
	bInit = false;
}

Warrior::~Warrior(){
}

void Warrior::Think(){
	Player::Think();
	InitWarrior();
	AutoAttack();
}

void Warrior::InitWarrior(){
	if( !bInit ){
		iZhandaoZhiTai = WatchCD( "Õ½¶·×ËÌ¬" );
		iKuangBaoZhiTai = WatchCD( "¿ñ±©×ËÌ¬" );
		iGongJi = WatchCD( "¹¥»÷" );

		bInit = true;
	}
}

int Warrior::WatchCD( std::string skill ){
	std::string sk = utf8(skill);
	for( std::vector<Skill>::iterator i= pai->vSkill.begin();i!=pai->vSkill.end();i++ ){
		if( strcmp(i->name.c_str(),sk.c_str()) == 0  ){
			pai->AddCooldownTrack( i->id,false );
			for( std::vector<Cooldown>::iterator k = pai->vCD.begin();k!=pai->vCD.end();k++){
				if( k->spell == i->id ){
					return (k-pai->vCD.begin());
				}
			}
			printf("Can't find cooldown slot %s(%d)!\n",skill.c_str(),i->id );
		}
	}
	printf("Can't find skill %s!\n",skill.c_str() );
	return -1;
}

int c = 0;
void Warrior::AutoAttack(){
	if( c == 0 ){
		PrintCooldown();
	}
	c++;
	if( c > 100 )c = 0;
}