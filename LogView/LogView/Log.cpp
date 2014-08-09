#include "stdafx.h"
#include "Log.h"
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <math.h>
#include <float.h>
#include <io.h>
#include "lossless.h"

std::wstring utf8w( std::string& s ){
	std::wstring wstr;
	std::string str;

	int len = MultiByteToWideChar(CP_UTF8,0,(char *)s.c_str(),-1,NULL,NULL);
	wstr.resize( len );
	MultiByteToWideChar(CP_UTF8,0,(char *)s.c_str(),-1,&wstr[0],wstr.size()); 
	return wstr;
}

bool AreanaLog::isUnitLog( std::wstring s ){
	return false;
}

std::wstring AreanaLog::cnRace(std::wstring r){
	if( r==TEXT("PALADIN") ){
		return TEXT("骑士");
	}else if( r==TEXT("WARRIOR") ){
		return TEXT("战士");
	}else if( r==TEXT("HUNTER") ){
		return TEXT("猎人");
	}else if( r==TEXT("DRUID") ){
		return TEXT("小德");
	}else if( r==TEXT("MAGE") ){
		return TEXT("法师");
	}else if( r==TEXT("WARLOCK") ){
		return TEXT("术士");
	}else if( r==TEXT("PRIEST") ){
		return TEXT("牧师");
	}else if( r==TEXT("SHAMAN") ){
		return TEXT("洒满");
	}else if( r==TEXT("DEATHKNIGHT") ){
		return TEXT("死骑");
	}else if( r==TEXT("ROGUE") ){
		return TEXT("盗贼");
	}

	return r;
}

int AreanaLog::getWordCount(std::wstring s ){
	int count = 0;
	std::wstring::size_type off,off2;
	off = 0;
	do{
		off2 = s.find(TEXT(" "),off);
		if( off2!=std::wstring::npos&&off2-off>0 )
			count++;
		if( off2==std::wstring::npos&&off<s.size() )
			count++;
		off = off2+1;
	}while( off2!=std::wstring::npos );
	return count;
}

std::wstring AreanaLog::getWord( std::wstring& s,int i ){
	int count = 0;
	std::string::size_type off,off2;
	off = 0;
	do{
		off2 = s.find(TEXT(" "),off);
		if( count==i )
			return s.substr(off,off2-off);
		if( off2!=std::wstring::npos&&off2-off>0 )
			count++;
		off = off2+1;
	}while( off2!=std::wstring::npos );
	return std::wstring(TEXT(""));
}

bool AreanaLog::loadZone(std::wstring wstr,int n ){
	if( n==3 || n==2 ){
		zone=getWord(wstr,1);
		if(n==3)
			subZone=getWord(wstr,2);
		return true;
	}
	return false;
}


MapItem::MapItem(){
}

UnitState AreanaLog::loadState(std::wstring wstr,int n ){
	UnitState us;
	if(n>=12){
		us.id = getWord(wstr,1);
		us.name = getWord(wstr,2);
		us.target = getWord(wstr,8);
		us.race = cnRace(getWord(wstr,3));
		us.isParty = _wtoi( getWord(wstr,4).c_str() )==1?false:true;
		us.health = _wtoi( getWord(wstr,5).c_str() );
		us.healthMax = _wtoi( getWord(wstr,6).c_str() );
		us.mana = _wtoi( getWord(wstr,7).c_str() );
		us.x = _wtof( getWord(wstr,9).c_str() );
		us.y = _wtof( getWord(wstr,10).c_str() );
		us.z = _wtof( getWord(wstr,11).c_str() );
		left = min(us.x,left);
		right = max(us.x,right);
		top = min(us.y,top);
		bottom = max(us.y,bottom);
	}
	bool isbuff=true;
	for( int i=12;i<n;i++ ){
		std::wstring ws = getWord(wstr,i);
		if( ws.size()>0 && ws[0]==L'D' )
			isbuff=false;
		else{
			if( isbuff ){
				us.buff.push_back(ws);
			}else{
				us.debuff.push_back(ws);
			}
		}
	}
	return us;
}

std::wstring AreanaLog::realName(std::wstring name){
	std::wstring::size_type pos = name.find(TEXT("-"));
	if( pos != std::wstring::npos ){
		return name.substr(0,pos);
	}else
		return name;
}

UnitLog AreanaLog::loadLog(std::wstring wstr,int n ){
	UnitLog ul;
	if(n>=8){
		ul.type=getWord(wstr,0);
		ul.time = _wtof( getWord(wstr,1).c_str() );
		ul.sourceId=getWord(wstr,2);
		ul.sourceName=realName(getWord(wstr,3));
		ul.sourceFlags=getWord(wstr,4);
		ul.destId=getWord(wstr,5);
		ul.destName=realName(getWord(wstr,6));
		ul.destFlags=getWord(wstr,7);
	}
	for( int i=8;i<n;i++ )
		ul.exs.push_back( getWord(wstr,i) );
	return ul;
}

void AreanaLog::clacType(UnitLog& ul){
	//下面代码统计是33还是55
	if( ul.states.size() == 10 &&areanaType==0){//55
		areanaType=5;
		for(std::vector<UnitState>::iterator i=ul.states.begin();i!=ul.states.end();i++){
			if( i->isParty )
				friendRaces.push_back(i->race);
			else
				enemyRaces.push_back(i->race);
		}
	}else if(ul.states.size() == 6 &&areanaType==0 ){//可能是33，进一步判断是不是有3个敌人
		areanaType=3;
		for(std::vector<UnitState>::iterator i=ul.states.begin();i!=ul.states.end();i++){
			if( i->isParty )
				friendRaces.push_back(i->race);
			else
				enemyRaces.push_back(i->race);
		}
		if(enemyRaces.size()!=3){
			enemyRaces.clear();
			friendRaces.clear();
			areanaType=0;
		}
	}
}

bool AreanaLog::load( std::wstring fname ){
	std::ifstream is;
	std::string str;
	std::wstring wstr,type;
	UnitLog cur;
	UnitState sta;
	bool first=true;
	is.open( fname.c_str() );
	if( is.is_open() ){
		logs.clear();
		str.resize(2048);
		areanaType=0;
		friendRaces.clear();
		enemyRaces.clear();
		left=FLT_MAX;
		right=-FLT_MAX;
		top=FLT_MAX;
		bottom=-FLT_MAX;
		while( is.getline((char*)str.c_str(),2048) ){
			wstr = utf8w(str);
			int size = getWordCount(wstr);
			if( size>0 ){
				type=getWord(wstr,0);
				if( type==TEXT("ZONE") ){
					loadZone(wstr,size);
				}else if(type==TEXT("STATE") ){
					sta=loadState(wstr,size);
					cur.states.push_back(sta);
				}else{
					if( !first ){
						clacType(cur);
						logs.push_back(cur);
					}
					cur=loadLog(wstr,size);
					first=false;
				}
			}
		}
		if( !first )
			logs.push_back(cur);

		is.close();
		map.InitMap( zone );
//		map.left = left;
//		map.right = right;
//		map.top = top;
//		map.bottom = bottom;
		if( logs.size()>0 ){
			double time = logs[0].time;
			for( std::vector<UnitLog>::iterator i=logs.begin();i!=logs.end();i++ )
				i->time-=time;
			timeLength = logs.back().time;
		}
		return true;
	}
	return false;
}

double AreanaLog::getTimeLength(){
	return timeLength;
}

MapItem::MapItem( int t,double xx0,double yy0,double xx1,double yy1 ){
	type = t;
	x0 = xx0;
	y0 = yy0;
	x1 = xx1;
	y1 = yy1;
}

UnitLog AreanaLog::gotoLog( double t ){
	for( std::vector<UnitLog>::iterator i=logs.begin();i!=logs.end();i++ ){
		if( i->time>=t && !i->states.empty() )
			return *i;
	}
	return logs.back();
}

AreanaMap::~AreanaMap(){
	if( hmap )
		DeleteObject( hmap );
	hmap = NULL;
}

double clac_left(double x0,double y0,double x1,double y1,double xx0,double yy0,double xx1,double yy1 ){
	return (x0*x1*(yy0-yy1)+x0*y1*xx1-x1*y0*xx0)/(x0*y1-x1*y0);
}

double clac_top(double x0,double y0,double x1,double y1,double xx0,double yy0,double xx1,double yy1 ){
	return (y0*y1*(xx1-xx0)+x0*y1*yy0-x1*y0*yy1)/(x0*y1-x1*y0);
}

void AreanaMap::clacImageRect( double x0,double y0,double x1,double y1,double xx0,double yy0,double xx1,double yy1 ){
	BITMAP bmp;
	GetObject( hmap,sizeof(BITMAP),&bmp );
	left = clac_left( x0,y0,x1,y1,xx0,yy0,xx1,yy1 );
	top = clac_top( x0,y0,x1,y1,xx0,yy0,xx1,yy1 );
	right = bmp.bmWidth*(xx0-left)/x0 + left;
	bottom = bmp.bmHeight*(yy0-top)/y0 + top;
}

bool AreanaMap::InitMap( std::wstring s ){
	name = s;
	std::string fname;
	mi.clear();
	if( hmap )
		DeleteObject( hmap );
	if( name==TEXT("洛丹伦废墟") ){
		//出生地1
		mi.push_back( MapItem(type_move,1280.64,1731.75,1280.64,1757.30 ) );
		mi.push_back( MapItem(type_line,1275.67,1757.30,1275.67,1730.87 ) );
		//外围1
		mi.push_back( MapItem(type_move,1285.72,1720.49,1300.71,1713.22 ) );
		mi.push_back( MapItem(type_line,1338.95,1683.29,1334.37,1652.35 ) );
		mi.push_back( MapItem(type_line,1320.99,1626.27,1301.13,1615.62 ) );
		//出生地2
		mi.push_back( MapItem(type_move,1295.89,1606.51,1301.09,1580.91 ) );
		mi.push_back( MapItem(type_line,1290.67,1579.99,1290.58,1598.71 ) );
		//外围2
		mi.push_back( MapItem(type_move,1288.13,1612.50,1259.53,1626.69 ) );
		mi.push_back( MapItem(type_line,1231.50,1653.60,1237.46,1679.38 ) );
		mi.push_back( MapItem(type_line,1237.90,1696.37,1271.01,1715.07 ) );
		//墓地
		mi.push_back( MapItem(type_rect,1275.89,1670.84,1295.91,1663.93 ) );
		fname="g3059.png";
		hmap = loadimage( fname );
		type = 0;
		//进入点，中点
		clacImageRect(183,584,245,45,1295,1586,1278,1745); //ok
	}else if( name==TEXT("刀锋山竞技场") ){
		fname="g3888.png";
		hmap = loadimage( fname );
		type = 1;
		clacImageRect(568,198,65,429,6186,-238,6292,-288); //ok
	}else if( name==TEXT("达拉然竞技场") ){
		fname="g3945.png";
		hmap = loadimage( fname );
		type = 1;
		clacImageRect(638+170,89,-210,447,1357,-816,1218,-765); //ok
	}else if( name==TEXT("纳格兰竞技场") ){
		fname="g4041.png";
		hmap = loadimage( fname );
		type = 0;
		clacImageRect(135,575,417,55,4085,2867,4027,2972); //ok
	}else if( name==TEXT("勇气竞技场") ){
		fname="g5208.png";
		hmap = loadimage( fname );
		type = 1;
		clacImageRect(305,298,305,197,763,275,764,295);
	}
	/*
	if( right<left ){
		double temp = left;
		left=right;
		right=temp;
	}
	if( bottom<top ){
		double temp = top;
		top = bottom;
		bottom = temp;
	}*/
	return true;
}

std::string AreanaMap::GetExePath(void)
{
	std::string path;
	char inof[255];

	GetModuleFileNameA( AfxGetInstanceHandle(),inof,255 );
   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];
	_splitpath( inof, drive, dir, fname, ext );
	path = drive;
	path += dir;

	return path;
}

HBITMAP AreanaMap::loadimage( std::string fname ){
	DWORD size;
	HBITMAP hmap;
	std::string path = GetExePath();
	path += fname;
	Lossless* ploss = load_png(path.c_str(),&size );
	if( ploss!=NULL ){
		hmap = lossless_to_hbitmap( ploss );
		delete [] ploss;
	}else{
		hmap = NULL;
	}
	return hmap;
}

AreanaMap::AreanaMap(){
}

POINT AreanaMap::pttopt(const RECT& rect,double x,double y){
	POINT pt;
	BITMAP bmp;
	GetObject( hmap,sizeof(BITMAP),&bmp );

	int x0 = ((rect.right-rect.left)-bmp.bmWidth)/2;
	int y0 = ((rect.bottom-rect.top)-bmp.bmHeight)/2;
	pt.x = bmp.bmWidth*(x-left)/(right-left)+rect.left + x0;
	if( type==0 )
		pt.y = bmp.bmHeight*(y-top)/(bottom-top)+rect.top + y0;
	else
		pt.y = bmp.bmHeight*(-y-top)/(bottom-top)+rect.top + y0;
	return pt;
}

//name表示绘制这个人的走位线
void AreanaLog::drawUnitState(CDC* pdc,const RECT& rect,UnitLog& stat,std::vector<std::wstring>& tracer,double t){
	CString str;
	int count=1;
	double cx,cy;
	//绘制矢量地图
	/*
	for( std::vector<MapItem>::iterator k=map.mi.begin();k!=map.mi.end();k++ ){
		if( k->type==type_move ){
			pdc->MoveTo( map.pttopt(rect,k->x0,k->y0) );
			pdc->LineTo( map.pttopt(rect,k->x1,k->y1) );
			cx = k->x1;
			cy = k->y1;
		}else if( k->type==type_line ){
			pdc->LineTo( map.pttopt(rect,k->x0,k->y0) );
			pdc->LineTo( map.pttopt(rect,k->x1,k->y1) );
			cx = k->x1;
			cy = k->y1;
		}else if( k->type==type_rect ){
			POINT pt0 = map.pttopt(rect,k->x0,k->y0);
			POINT pt1 = map.pttopt(rect,k->x1,k->y1);
			CGdiObject* pobj = pdc->SelectStockObject(NULL_BRUSH);
			pdc->Rectangle(pt0.x,pt0.y,pt1.x,pt1.y );
			pdc->SelectObject( pobj );
		}
	}*/
	//绘制位图地图
	if( map.hmap ){
		BLENDFUNCTION bf;
		POINT pt;
		if( map.type==0 )
			pt = map.pttopt( rect,map.left,map.top );
		else
			pt = map.pttopt( rect,map.left,-map.top );
		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.SourceConstantAlpha = 255;
		bf.AlphaFormat = AC_SRC_ALPHA;
		CDC memdc;
		BITMAP bmp;
		GetObject( map.hmap,sizeof(BITMAP),&bmp );
		memdc.CreateCompatibleDC(pdc);
		HBITMAP hold=(HBITMAP)SelectObject(memdc.m_hDC,map.hmap );
		AlphaBlend( pdc->m_hDC,
			pt.x,pt.y,bmp.bmWidth,bmp.bmHeight,
			memdc.m_hDC,
			0,0,bmp.bmWidth,bmp.bmHeight,
					bf );	
		SelectObject(memdc.m_hDC,hold );
	}
	//绘制角色连线
	if( !tracer.empty() ){
		for( std::vector<std::wstring>::iterator i=tracer.begin();i!=tracer.end();i++ ){
			std::wstring race;
			for( std::vector<UnitState>::iterator k = stat.states.begin();k!=stat.states.end();k++ ){
				if( k->name==(*i) ){
					race = k->race;
				}
			}
			drawTraceLine(pdc,rect,*i,t,getRaceColor(race));
		}
	}
	//绘制单元编号
	pdc->SetBkMode(TRANSPARENT);
	for( std::vector<UnitState>::iterator i=stat.states.begin();i!=stat.states.end();i++ ){
		POINT pt = map.pttopt(rect,i->x,i->y);
		str.Format(TEXT("%d"),count++);
		CBrush br;
		br.CreateSolidBrush(getRaceColor(i->race));
		CBrush* pb = (CBrush*)pdc->SelectObject( &br );
		pdc->Ellipse(pt.x-8,pt.y-8,pt.x+8,pt.y+8);
		pdc->SelectObject( pb );
		pdc->TextOut(pt.x-4,pt.y-8,str);
	}
}

void AreanaLog::drawTraceLine( CDC* pdc,const RECT& rect,std::wstring name,double t,COLORREF color ){
	CPen pen;
	pen.CreatePen(PS_SOLID,1,color);
	CPen* pold = (CPen*)pdc->SelectObject(&pen);

	for( int i = 0;i < 20;i++ ){
		UnitLog ul = gotoLog( t );
		for( std::vector<UnitState>::iterator k=ul.states.begin();k!=ul.states.end();k++ ){
			if( name==k->name ){
				if( i==0 )
					pdc->MoveTo( map.pttopt(rect,k->x,k->y) );
				else
					pdc->LineTo( map.pttopt(rect,k->x,k->y) );
			}
		}
		t-=1;
		if( t<0 )break;
	}
	pdc->SelectObject( pold );
}

COLORREF AreanaLog::getRaceColor( std::wstring s ){
	if( s == TEXT("战士") ){
		return RGB(184,112,112);
	}else if( s == TEXT("法师") ){
		return RGB(128,255,255);
	}else if( s == TEXT("骑士") ){
		return RGB(255,128,255);
	}else if( s == TEXT("牧师") ){
		return RGB(255,255,255);
	}else if( s == TEXT("死骑") ){
		return RGB(255,0,0);
	}else if( s == TEXT("猎人") ){
		return RGB(128,255,0);
	}else if( s == TEXT("小德") ){
		return RGB(255,128,0);
	}else if( s == TEXT("盗贼") ){
		return RGB(255,255,128);
	}else if( s == TEXT("术士") ){
		return RGB(128,0,255);
	}else if( s == TEXT("洒满") ){
		return RGB(0,128,255);
	}
	return RGB(0,0,0);
}