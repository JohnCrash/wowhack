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

const char* sWayPoint = ".\\WayPoint\\";
const char* sWay = ".waypoint";
const char* sArea = ".area";
const char* sBox = ".box";
const char* sCurrentWay = "CurrentWay";

extern Player* g_player;

HANDLE g_hGoto = NULL;
DWORD  g_dwGoto = 0;
std::string g_strGotoName;
float  g_vFrom[3];
float  g_vTo[3];

static std::string AllocWayPointName(){
	std::string path,name,fname;
	_finddata_t fd;
	intptr_t handle;
	std::vector<std::string> vName;
	int i = 1;

	path = sWayPoint;
	path += "*";
	path += sWay;

	handle = _findfirst( path.c_str(),&fd );
	if( handle!=-1 ){
		do{
			name = fd.name;
			vName.push_back(name);
		}while( !_findnext( handle,&fd ) );
	}
	_findclose( handle );
	do{
		char buf[32];
		sprintf(buf,"%d",i);
		i++;
		name = buf;
		fname = name;
		fname += sWay;
	}while( std::find(vName.begin(),vName.end(),fname) != vName.end() );

	return name;
}

Player::Player( AI* ai ){
	pai = ai;
	bCreateWay = false;
	bShowWay = false;
	g_hGoto = NULL;
	bStop = true;
	bForward = true;
	fShift = 0;
	bShift = false;
}

Player::~Player(){
}

void Player::Think(){
	pself = &(pai->vUNIT[pai->iPlayer]);
	Movement();
	Castspell();
	CreateWay();
}

void Player::Castspell(){
}

void Player::Go(bool moveOrstop,bool forwardOrbackward ){
	bStop = !moveOrstop;
	if( bStop ){
		pai->MoveFBStop();
		pai->MoveTurnStop();
		pai->MoveLRStop();
	}
	WayPointCount = -1;
	bForward = forwardOrbackward;
	pai->GetWay()->SearchWayPoint(pai->Pos);
}

void Player::Movement(){
	Way* pway = pai->GetWay();
	if( bStop || pway->vPoints.empty() )return;
	
	Point* pp = pway->CurrentWayPoint();
	Point* ppr; //上一个点
	float sv[3],av[3],ssv[3];
	int n;
	//下面代码去上一个点
	if( bForward ){
		n = pway->iCurrent-1;
	}else{
		n = pway->iCurrent+1;
	}
	if( n < 0 || n >= pway->vPoints.size() )
		ppr = NULL;
	else
		ppr = &(*(pway->vPoints.begin()+n));

	vector_sub(sv,pp->point,pai->Pos);
	vector_copy( av,pai->Direct );
	sv[2] = 0;
	float a = vector_angle(av,sv);
	//a是玩家面向和要去点的角度
	//调整角度面对要去点
	if( a > M_PI/9 ){
		pai->MoveTurnRight();
	}else if( a < -M_PI/9 ){
		pai->MoveTurnLeft();
	}else if( a < M_PI/36 && a > -M_PI/36 ){
		pai->MoveTurnStop();
	}
	//和要去点的距离
	double dis = vector_mod(sv);
	//做左右平移操作，让角色在路径上面
	if( ppr ){//上一个点不是NULL

		vector_sub(ssv,pp->point,ppr->point);
		vector_sub(av,pp->point,pai->Pos);
		float aa = vector_angle(av,ssv);
		float dis2 = abs(aa*dis);

		if( dis2 > 2 ){ //大于2码就平移
			if( aa > 0 ){
				pai->MoveLeft();
			}else{
				pai->MoveRight();
			}
			fShift = dis2;
			bShift = true;
		}else{ //小于1码就停止
			if( dis2 < fShift )
				fShift = dis2;
			else if( bShift ){
				pai->MoveLRStop();
				bShift = false;
			}
		}
	}
	//和要去点的距离如果小于5码
	if(  dis < 5 ){
		if( bForward ){
			if( !pway->NextWayPoint() ){
				pai->MoveFBStop();
				pai->MoveLRStop();
				pai->MoveTurnStop();
				bStop = true;
			}
		}else{
			if( !pway->PrevWayPoint() ){
				pai->MoveFBStop();
				pai->MoveLRStop();
				pai->MoveTurnStop();
				bStop = true;
			}
		}
	}else{
		pai->MoveForward();
	}
	//角度太大停止移动，等待调整好角度在移动
	if( a > M_PI/3 || a < -M_PI/3 ){
		pai->MoveFBStop();
		pai->MoveLRStop();
	}
}

void Player::CreateWay(){
	Way* pway = pai->GetWay();
	if( bCreateWay ){
		pway->SetWayPointArea(pai->sRegion.c_str(),pai->sSubReg.c_str());
		pway->AddPoint( pai->Pos,0 );
	}
	if( bShowWay ){
		if( WayPointCount != pway->vPoints.size() ){
			WayPointCount = pway->vPoints.size();
			Render* prender = pai->GetRender();

			prender->Begin( sCurrentWay );
			prender->LineWidth( 3 );
			prender->Color(0,0,1);
			for( std::vector<Point>::iterator i = pway->vPoints.begin();
				i!=pway->vPoints.end();i++ ){
				if( i == pway->vPoints.begin() )
					prender->MoveTo( i->point[0],i->point[1],i->point[2] );
				else
					prender->LineTo( i->point[0],i->point[1],i->point[2] );
			}
			prender->End();
		}
	}
}

bool Player::IsSelf(const char* name){
	if( pai->vUNIT[pai->iPlayer].sName == name )
		return true;
	return false;
}

void Player::ChatEvent(const char* sender_,const char* msg_,const char* channel_ ){
	//if( channel_&&strcmp(channel_,"CHAT_MSG_WHISPER")==0 ){ //自己发出的信息
	if( IsSelf( sender_ ) ){
		std::string sender(sender_),msg(msg_),channel(channel_);
		Command( msg_ );
	}
}

void Player::PrintWayArea( Way* pway ){
	if( pway->vAreas.empty() ){
		printf("%s %d\n",ansi(pway->Name).c_str(),pway->vPoints.size() );
	}else{
		printf("%s %d (%s,%s)->(%s,%s)\n",ansi(pway->Name).c_str(),pway->vPoints.size(),
			ansi(pway->vAreas.front().name).c_str(),ansi(pway->vAreas.front().zone).c_str(),
			ansi(pway->vAreas.back().name).c_str(),ansi(pway->vAreas.back().zone).c_str());
	}
}

void Player::CmdListWP( std::string arg1,std::string arg2 ){
	Cross* pcross = pai->GetCross();
	if( pcross->Load() ){
		for( std::vector<Way*>::iterator i = pcross->vWN.begin();
			i!=pcross->vWN.end();i++ ){

			if( arg1.empty() && arg2.empty() ){//全部
				PrintWayArea( *i );
			}else if( arg1 == "*" && !arg2.empty() ){//目的地是arg2
				if( !(*i)->vAreas.empty() && (*i)->vAreas.back().name == arg2 ){
					PrintWayArea( *i );
				}
			}else if( !arg1.empty() && arg2 == "*" ){//起点是arg1
				if( !(*i)->vAreas.empty() && (*i)->vAreas.front().name == arg1 ){
					PrintWayArea( *i );
				}
			}else if( !arg1.empty() && arg2.empty() ){//有此区域
				for( std::vector<Area>::iterator j = (*i)->vAreas.begin();j!=(*i)->vAreas.end();
					j++ ){
						if( j->name == arg1 ){
							PrintWayArea( *i );
							break;
						}
					}
			}else if( !arg1.empty() && !arg2.empty() ){
				if( !(*i)->vAreas.empty() && (*i)->vAreas.back().name == arg2&&
					(*i)->vAreas.front().name == arg1 ){
						PrintWayArea( *i );
					}
			}
		}
	}else{
		printf("Cross load false\n");
	}
}

static DWORD WINAPI CalcCross( LPVOID lpParam ){
	Cross* pcross = (Cross*)lpParam;
	pcross->clear();
	pcross->LoadAllWay();
	pcross->ReCalc();
	pcross->Save();
	pcross->isLoad = true;
	return 0;
}

static DWORD WINAPI GotoCalc( LPVOID lpParam ){
	Cross* pcross = (Cross*)lpParam;

	pcross->Load();
	if( pcross->GetAreaPointByName( g_vTo,g_strGotoName ) ){
		if( pcross->Goto( g_vFrom,g_vTo ) ){
			g_hGoto = NULL;
			g_player->Go(true,true);
			printf("Calc complate...\n");
			return 1;
		}else{
			printf("Can't go %s\n",ansi(g_strGotoName).c_str());
		}
	}else{
		printf("Can't find local %s\n",ansi(g_strGotoName).c_str());
	}
	g_hGoto = NULL;
	
	return 0;
}

void Player::Command( std::string msg ){
	std::istrstream is(msg.c_str());
	std::string cmd;
	if( is>>cmd ){
		if( cmd == "beginwp" ){ //开始创建路点
			bCreateWay = true;
			Way* pway = pai->GetWay();
			std::string fname = AllocWayPointName();
			if( !pway->ReNew( fname.c_str() ) ){
				printf("Can't create waypoint name:%s\n",fname.c_str());
			}
		}else if( cmd == "endwp" ){ //保存路点
			Way* pway = pai->GetWay();
			bCreateWay = false;
			if( !pway->Save() ){
				printf("Can't save waypoint name:%s\n",pway->Name.c_str());
			}
		}else if( cmd == "showwp" ){ //显示当前路径
			bShowWay = true;
			WayPointCount = -1;
		}else if( cmd == "hidewp" ){ //不显示当前路径
			bShowWay = false;
			Render* prender = pai->GetRender();
			prender->Clear( sCurrentWay );
		}else if( cmd == "listwp" ){ //列出特定路点
			std::string arg1,arg2;
			is>>arg1>>arg2;
			CmdListWP( arg1,arg2 );
		}else if( cmd == "delwp" ){ //删除一个路点文件
			std::string name;
			if( is>>name && !name.empty() ){
				std::string fname;
				fname = name;
				fname += sWayPoint;
				if( remove( fname.c_str() ) == 0 ){
					fname = name;
					fname += sArea;
					remove( fname.c_str() );
					fname = name;
					fname += sBox;
					remove( fname.c_str() );
				}else{
					printf("Can't remove file:%s\n",fname.c_str() );
				}
			}else{
				printf("delwp waypoint\n");
			}
		}else if( cmd == "loadwp" ){ //转入路点文件
			std::string name;
			if( is>>name && !name.empty() ){
				if(!pai->GetWay()->Load( name.c_str() )){
					printf("Can't load way:%s",name.c_str() );
				}
			}else{
				printf("loadwp name\n");
			}
		}else if( cmd == "calccross" ){ //重新计算焦点
			DWORD dwCC;
			CreateThread(NULL,1024*1024,CalcCross,pai->GetCross(),0,&dwCC);
		}else if( cmd == "go"||cmd == "goto"  ){ //移动到指定位置
			g_strGotoName.clear();
			if( g_hGoto==NULL && is>>g_strGotoName && !g_strGotoName.empty() ){
				vector_copy( g_vFrom,pai->Pos );
				g_hGoto = CreateThread(NULL,1024*1024,GotoCalc,pai->GetCross(),0,&g_dwGoto);
			}else{
				printf("go name:%s or Calcing..\n",g_strGotoName.c_str() );
			}
		}else if( cmd == "stop"||cmd== "pause" ){//停止移动
			Go(false,true);
		}else if( cmd == "forward" ){ //向前终点移动
			Go(true,true);
		}else if( cmd == "backward" ){ //向起点移动
			Go(true,false);
		}
	}
}

