#include "stdafx.h"
#include "callhook.h"
#include "wowentry.h"
#include "dllentry.h"
#include "wowin.h"
#include "scout.h"
#include "render.h"

/*
#define NEXT(a) *((void**)a+0x1c/4+1)

UnitItem* GetUnitItems(){
	void* p = GetTable();

	return (UnitItem*)*((void**)p+0x1c/4);
}

int GetUnitSize(){
	UnitItem* punits = GetUnitItems();
	int i = 0;
	int count = 0;
	int totolcount = 0;
	
	//printf("punits = %X,%s\n",punits,UnitName(GetUnitByName("player")));
	if( punits ){
		printf("=================================\n");
		while( punits[i].offset==0x1c ){
			void* pnext = punits[i].punit_next;
			int subcount = 0;
			while( pnext && !((unsigned int)pnext&1) ){
				printf("%X-",pnext);

				pnext = NEXT(pnext);
				totolcount++;
				subcount++;
			}
			count++;
			printf("%d\n",subcount);
			i++;johnxj
		}
	}
	printf("count = %d,totol = %d\n",count,totolcount);
	return count;
}

char* UnitName( void*punit ){
	char* name = NULL;
	PROC GetUnitName = (PROC)0X0066FBA0;
	__asm{
		push ecx
		mov ecx,punit
		push 1
		push 0
		call GetUnitName
		pop ecx
		mov name,eax
	}
	return name;
}
*/

/*
int printUnitName(long long id,void* param){
	int* pcount = (int*)param;
	void* punit = GetUnit(id,1);
	void* pplayer = GetUnitByName(sPlayer);
	if( punit&&pplayer ){
		float v[4],vv[4];
		GetUnitPosition(punit,v);
		GetUnitPosition(pplayer,vv);
		
		int type = UnitType(punit);
		switch(type){
			case UNIT_NPC:g_Render.Color(0,0,1);break;
			case UNIT_ITEM:g_Render.Color(0,1,0);break;
			case UNIT_PLAYER:g_Render.Color(1,0,0);break;
			case UNIT_BODY:g_Render.Color(1,1,0);break;
			default:g_Render.Color(1,1,1);
					printf("%d\n",type);
		}
		if( v[0] != 0 && v[1] != 0 && v[2] != 0 ){
			g_Render.MoveTo( v[0],v[1],v[2] );
			g_Render.LineTo( vv[0],vv[1],vv[2] );
		}
	}else
		printf("NULL\n");

	(*pcount)++;
	return 1;
}

void EnumAll(){
	int count = 0;
	
	g_Render.Begin("link");
	g_Render.LineWidth(1);
	g_Render.Color(1,0,0);
	EnumUnit(printUnitName,&count);
	
	g_Render.End();

	printf("count=%d\n",count);
}
*/
//取得Unit的类型
Units g_Units;
long long * g_pFocusID = NULL;
long long oldFocusID = 0;
long long castFocusID = 0;

static int EnumUnitFunc(long long id,void* param){
	float v[3],vv[3],dd;
	Units* punits = (Units*)param;
	void* punit = GetUnit(id,1);
	unsigned int type = UnitType(punit);

	type &= UNIT_PLAYER_MASK;

	if( type == punits->type||punits->type==-1||
		(punits->type==UNIT_PLAYERANDNPC&&(type==UNIT_NPC||type==UNIT_PLAYER))){
		GetUnitPosition(punit,v);
		vector_sub(vv,v,punits->pos);
		dd = vector_mod2(vv);
		
		if( dd <= punits->dis ){
			punits->vUnits.push_back(id);
			punits->vType.push_back(type);
			punits->vp.push_back(UnitPosition(v[0],v[1],v[2],sqrt(dd)));
		}
	}
#ifdef _OUT
/*	else if(type!=7){
		if( type!=3 && type!= 9 && type!=25 
			&& type!=33 && type!=129 && type!=256){
		GetUnitPosition(punit,v);
		printf("type:%d (%f,%f,%f) 0x%x\n",type,v[0],v[1],v[2],punit);
		}
	}*/
#endif
	return 1;
}

bool Units::Search( int t,float d ){
	vUnits.clear();
	vp.clear();
	vType.clear();
	iCurrent = -1;
	type = t;
	dis = d*d;

	oldFocusID = *g_pFocusID; //保存以前的FocusID
	void* pplayer = GetUnitByName(sPlayer);
	if( pplayer==NULL )return false;
	GetUnitPosition(pplayer,pos);
	EnumUnit(EnumUnitFunc,(void*)this);
	if( vUnits.empty() )return false;
	
	return true;
}

bool Units::Next(){
	if( iCurrent < (int)vUnits.size() ){
		iCurrent++;
		return true;
	}else{
		return false;
	}
}

bool Units::Prev(){
	if( iCurrent >= 0 ){
		iCurrent--;
		return true;
	}else{
		return false;
	}
}

bool Units::TargetUnit(){
	return false;
}

bool Units::FocusUnit2(double& a,double& b,double& x,double& y,double& z,double& d,unsigned int&type){
	if( iCurrent<0||iCurrent>=vUnits.size() )return false;
	long long& id = vUnits.at(iCurrent);
	//UnitFocus检查dwProhibitSpell是否为0
	//使用EnableProhibitFunc设置为0
	//EnableProhibitFunc();
	//UnitFocus(&id);
	UnitPosition up = vp.at(iCurrent);
	type = vType.at(iCurrent);
	x = up.x;
	y = up.y;
	z = up.z;
	d = up.dis;
	*g_pFocusID = id;
	a = (double)((unsigned int)id);
	b = (double)((unsigned int)(id>>32));
	return true;
}

bool Units::FocusUnit(int b){
	if( iCurrent<0||iCurrent>=vUnits.size() )return false;
	long long& id = vUnits.at(iCurrent);
	//UnitFocus检查dwProhibitSpell是否为0
	//使用EnableProhibitFunc设置为0
	//EnableProhibitFunc();
	//UnitFocus(&id);
	if( b==0 )
		*g_pFocusID = id;
	else if( b==1 ){
		unsigned int olddwProhibitSpell = *g_pdwProhibitSpell;
		*g_pdwProhibitSpell = 0;
		*g_pFocusID = 0;
#ifdef _OUT
		printf("FocusUnit2\n");
#endif
		UnitFocus(&id);
		*g_pdwProhibitSpell = olddwProhibitSpell;
	}else if(b==2){
		unsigned int olddwProhibitSpell = *g_pdwProhibitSpell;
		*g_pdwProhibitSpell = 0;
		*g_pFocusID = 0;
#ifdef _OUT
		printf("FocusUnit2\n");
#endif
		id = -1;
		UnitFocus(&id);
		*g_pdwProhibitSpell = olddwProhibitSpell;
	}
	return true;
}

int lua_FirstUnit(void*p){
	int type;
	float dis;

	if( Lua_isnumber(p,1) ){
		type = (int)Lua_tonumber(p,1);
		switch( type ){
			case 1:type = UNIT_PLAYER;break;
			case 2:type = UNIT_NPC;break;
			case 3:type = UNIT_ITEM;break;
			case 4:type = UNIT_BODY;break;
			case 5:type = UNIT_PLAYERANDNPC;break;
			default:type = -1;
		}
	}else{
		type = -1; //ALL
	}
	if( Lua_isnumber(p,2) ){
		dis = Lua_tonumber(p,2);
		if( dis < 0 )dis=0;
		if( dis > 106 )dis=666.66f;
	}else{
		dis = 666.66f;
	}
	if( g_Units.Search(type,dis) ){
		Lua_pushnumber(p,1);
	}else
		Lua_pushnil(p);
	return 1;
}

int lua_NextUnit(void*p){
	if( g_Units.Next() )
		Lua_pushnumber(p,1);
	else
		Lua_pushnil(p);
	return 1;
}

int lua_PrevUnit(void*p){
	if( g_Units.Prev() )
		Lua_pushnumber(p,1);
	else
		Lua_pushnil(p);
	return 1;
}

int lua_TargetUnit(void*p){
	if( g_Units.TargetUnit() )
		Lua_pushnumber(p,1);
	else
		Lua_pushnil(p);
	return 1;
}

int lua_FocusLock(void* p){
	unsigned int a,b;
	if( Lua_isnumber(p,1) ){
		a = (unsigned int)Lua_tonumber(p,1);
		if( Lua_isnumber(p,2) ){
			b = (unsigned int)Lua_tonumber(p,2);
			long long id;
			bool focusit = false;
			id = b;
			id<<=32;
			id |= a;
			if( Lua_isnumber(p,3) ){
				int i = (int)Lua_tonumber(p,3);
				if( i==1)
					focusit=true;
			}
			if( !focusit ){
				oldFocusID = *g_pFocusID; 
				*g_pFocusID = id;
			}else{
				unsigned int olddwProhibitSpell = *g_pdwProhibitSpell;
				*g_pdwProhibitSpell = 0;
				*g_pFocusID = 0;
#ifdef _OUT
		printf("lua_FocusLock\n");
#endif
				UnitFocus(&id);
				*g_pdwProhibitSpell = olddwProhibitSpell;
			}
			Lua_pushnumber(p,1);
			return 1;
		}
	}
	Lua_pushnil(p);
	return 1;
}

int lua_NextUnitAndFocus(void*p){
	if( g_Units.Next() ){
		double a,b;
		double x,y,z,d;
		unsigned int type;
		if( g_Units.FocusUnit2(a,b,x,y,z,d,type) ){
			Lua_pushnumber(p,a);
			Lua_pushnumber(p,b);
			Lua_pushnumber(p,x);
			Lua_pushnumber(p,y);
			Lua_pushnumber(p,z);
			Lua_pushnumber(p,d);
			Lua_pushnumber(p,type);
			return 7;
		}
	}
	Lua_pushnil(p);
	return 1;
}

int lua_FocusUnit(void*p){
	int b=0;
	if( Lua_isnumber(p,1) ){
		int i = (int)Lua_tonumber(p,1);
		b = i;
	}
	if( g_Units.FocusUnit(b) )
		if( b )
			Lua_pushnumber(p,2);
		else
			Lua_pushnumber(p,1);
	else
		Lua_pushnil(p);
	return 1;
}

int lua_SetCastSpellFocus(void*p){
	castFocusID = *g_pFocusID;
	return 0;
}

char g_CastSpellName[20];
char g_CastSpellTarget[16];

int lua_SetCastSpellName(void*p){
	if( Lua_isstring(p,1) ){
		size_t len;
		const char* s = Lua_tolstring(p,1,&len);
		if( len > sizeof(g_CastSpellName) )
			len = sizeof(g_CastSpellName);
		memset(g_CastSpellName,0,sizeof(g_CastSpellName));
		memcpy(g_CastSpellName,s,len);
	}
	if( Lua_isstring(p,2) ){
		size_t len;
		const char* s = Lua_tolstring(p,2,&len);
		if( len > sizeof(g_CastSpellTarget) )
			len = sizeof(g_CastSpellTarget);
		memset(g_CastSpellTarget,0,sizeof(g_CastSpellTarget));
		memcpy(g_CastSpellTarget,s,len);
	}
	return 0;
}

int lua_ResumeFocus(void*p){
	*g_pFocusID = oldFocusID;
	Lua_pushnil(p);
	return 1;
}

std::vector<std::string> g_text;

int lua_LoadDiffText(void*p){
	std::string name,text;
	std::ifstream is;

	if( Lua_isstring(p,1) ){
		size_t len;
		const char* s = Lua_tolstring(p,1,&len);
		name = s;
		is.open(name.c_str());
		if( is.is_open() ){
			g_text.clear();
			while( is>>text ){
				g_text.push_back(text);
			}
			is.close();
		}
	}
	Lua_pushnil(p);
	return 1;
}

int lua_AddDiffText(void*p){
	std::string text;

	if( Lua_isstring(p,1) ){
		size_t len;
		const char* s = Lua_tolstring(p,1,&len);
		text = s;
		if( std::find(g_text.begin(),g_text.end(),text)==g_text.end() ){
			g_text.push_back(text);
		}
	}
	Lua_pushnil(p);
	return 1;
}

int lua_SaveDiffText(void*p){
	std::ofstream fs;
	std::string name;

	if( Lua_isstring(p,1) ){
		size_t len;
		const char* s = Lua_tolstring(p,1,&len);
		name = s;
		fs.open( name.c_str() );
		if( fs.is_open() ){
			for(std::vector<std::string>::iterator i = g_text.begin();
				i != g_text.end();i++ ){
					fs<<*i<<std::endl;
				}
			fs.close();
		}
	}
	Lua_pushnil(p);
	return 1;
}