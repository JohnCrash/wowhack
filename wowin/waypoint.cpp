#include "stdafx.h"
#include "callhook.h"
#include "waypoint.h"
#include "wowentry.h"

const char* sWayPoint = ".\\WayPoint\\";
const char* sWay = ".waypoint";
const char* sArea = ".area";
const char* sBox = ".box";
const char* sCross = "all.cross";
const char* sAllArea = "all.area";
const char* sAllNode = "all.node";

Way g_way;
Cross g_cross;

Point::Point(){
	point[0] = 0;
	point[1] = 0;
	point[2] = 0;
	action = NOTHING;
	areaidex = -1;
}

Point::Point( float v[3],int a,int ind ){
	point[0] = v[0];
	point[1] = v[1];
	point[2] = v[2];
	action = a;
	areaidex = ind;
}

Point::Point( const Point& p ){
	point[0] = p.point[0];
	point[1] = p.point[1];
	point[2] = p.point[2];
	action = p.action;
	areaidex = p.areaidex;
}

Area::Area(){
}

void Way::operator=(Way& way){
	vPoints.clear();
	for( std::vector<Point>::iterator i = way.vPoints.begin();
		i != way.vPoints.end();i++ ){
			vPoints.push_back( *i );
		}
	iCurrent = way.iCurrent;
	vAreas.clear();
	for( std::vector<Area>::iterator k = way.vAreas.begin();
		k != way.vAreas.end();k++ ){
			vAreas.push_back( *k );
		}
	for( int j = 0;j < 6;j++ )
		box[j] = way.box[j];
	Name = way.Name;
}

bool Way::ReNew( const char* name ){
	if( name ){
		Name = name;
		vPoints.clear();
		vAreas.clear();
		iCurrent = 0;
	}
	return true;
}

Way::Way(){
	iCurrent = 0;
	CurrentAreaIndex = 0;
}

Way::~Way(){
	vPoints.clear();
	vAreas.clear();
	vAdds.clear();
	iCurrent = 0;
}

bool Way::Save(){
	std::ofstream fs,fs2,fs3;
	std::string fname;

	if( !vAdds.empty() ){
		Point& pt = vAdds.at(vAdds.size()-1);
		vPoints.push_back( pt );
		vAdds.clear();
	}
	fname = sWayPoint;
	fname += Name;
	fname += sWay;
	fs.open( fname.c_str() );
	if( fs.is_open() ){
		for( std::vector<Point>::iterator i = vPoints.begin();i!=vPoints.end();i++ ){
			fs << i->point[0] << " " << i->point[1] << " " << i->point[2] << " " << i->action << " " <<i->areaidex<<std::endl;
		}
		fs.close();
	}else return false;

	fname = sWayPoint;
	fname += Name;
	fname += sArea;
	fs2.open( fname.c_str() );
	if( fs2.is_open() ){
		for( std::vector<Area>::iterator i = vAreas.begin();i!=vAreas.end();i++){
			if( i->zone.empty() )
				fs2 << i->name << " -" << std::endl;
			else
				fs2 << i->name << " " << i->zone<<std::endl;
		}
		fs2.close();
	}else return false;
	fname = sWayPoint;
	fname += Name;
	fname += sBox;
	fs3.open( fname.c_str() );
	if( fs3.is_open() ){
		box[0] = (float)1.7e22;
		box[1] = -box[0];
		box[2] = box[0];
		box[3] = -box[0];
		box[4] = box[0];
		box[5] = -box[0];
		for( std::vector<Point>::iterator i = vPoints.begin();i!=vPoints.end();i++ ){
			box[0] = min(box[0],i->point[0]);
			box[1] = max(box[1],i->point[0]);
			box[2] = min(box[2],i->point[1]);
			box[3] = max(box[3],i->point[1]);
			box[4] = min(box[4],i->point[2]);
			box[5] = max(box[5],i->point[2]);
		}
		fs3<<box[0]<<" "<<box[1]<<" "<<box[2]<<" "<<box[3]<<" "<<box[4]<<" "<<box[5]<<std::endl;

		fs3.close();
	}else return false;
	return true;
}

bool Way::Load( const char* name ){
	std::ifstream is,is2,is3;
	std::string fname,n,z;
	float p[3];
	int a,i;

	if( Name == name )return true;

	Name = name;
	fname = sWayPoint;
	fname += name;
	fname += sWay;
	is.open( fname.c_str() );
	if( is.is_open() ){
		Name = name;
		iCurrent = 0;
		vPoints.clear();
		iCurrent = 0;
		while( is >>p[0]>>p[1]>>p[2]>>a>>i ){
			vPoints.push_back( Point(p,a,i) );
		}
		iCurrent = 0;
		is.close();
	}else return false;

	fname = sWayPoint;
	fname += name;
	fname += sArea;
	is2.open( fname.c_str() );
	if( is2.is_open() ){
		vAreas.clear();
		while( is2 >> n >> z ){
			if( z=="-" )z.clear();
			vAreas.push_back( Area(n.c_str(),z.c_str()) );
		}
		is2.close();
	}else return false;

	fname = sWayPoint;
	fname += name;
	fname += sBox;
	is3.open( fname.c_str() );
	if( is3.is_open() ){
		is3>>box[0]>>box[1]>>box[2]>>box[3]>>box[4]>>box[5];
		is3.close();
	}else return false;

	return true;
}

bool Way::AddPoint( float p[3],int action ){
	float v[3],v2[3];
	float maxa = 0;
	int   maxindex = -1;
	int   indarea = 0;

	if( !vPoints.empty() && action == 0 ){
		bool bOut = false;
		if( oldPt.point[0] == p[0] && oldPt.point[1] == p[1] && oldPt.point[2] == p[2] )
			return false;
		int index = 0;
		for( std::vector<Point>::iterator i = vAdds.begin();i!=vAdds.end();i++ ){
			vector_sub( v2,i->point,prevPt.point );
			vector_sub( v,p,prevPt.point );
			float a = vector_angle(v,v2);
			float d = vector_mod( v );
			d = fabs(a*d);
			if( d > maxa ){
				maxa = d;
				maxindex = index;
			}
			index++;
		}

		oldPt.point[0] = p[0];
		oldPt.point[1] = p[1];
		oldPt.point[2] = p[2];
		oldPt.action = action;
		if( maxa < 1 ){
			if( vAdds.size() > 8*1024 )//不能太多,这里做一个限定
				vAdds.clear();
			vAdds.push_back( Point(p,action,CurrentAreaIndex) );
			return false;
		}
		Point pp = vAdds.at(maxindex);
		vAdds.clear();
		vAdds.push_back( Point(p,action,CurrentAreaIndex) );
		p[0] = pp.point[0];
		p[1] = pp.point[1];
		p[2] = pp.point[2];
		action = pp.action;
		indarea = pp.areaidex;
	}
	if( vPoints.empty() ){
		vPoints.push_back( Point(p,action,CurrentAreaIndex) );
	}else{
		vPoints.insert( vPoints.begin()+iCurrent,Point(p,action,indarea) );
	}
	prevPt.point[0] = p[0];
	prevPt.point[1] = p[1];
	prevPt.point[2] = p[2];
	prevPt.action = action;
	oldPt.point[0] = p[0];
	oldPt.point[1] = p[1];
	oldPt.point[2] = p[2];
	oldPt.action = action;
	vAdds.clear();
	iCurrent++;
	return true;
}

bool Way::ClearWayPoint(){
	if( vPoints.empty() )return false;
	vPoints.erase( vPoints.begin()+iCurrent );
	if( iCurrent >= vPoints.size() ){
		if( !vPoints.empty() )
			iCurrent = vPoints.size()-1;
		else
			iCurrent = 0;
	}
	return true;
}

Point* Way::CurrentWayPoint(){
	if( vPoints.empty() )return NULL;
	if( iCurrent >= vPoints.size() )return NULL;
	return &(*(vPoints.begin()+iCurrent));
}

Point* Way::PrevWayPoint(){
	if( vPoints.empty() )return NULL;
	if( iCurrent <= 0 )
		return NULL;
	else
		iCurrent--;
	return &(*(vPoints.begin()+iCurrent));
}

Point* Way::NextWayPoint(){
	if( vPoints.empty() )return NULL;

	if( iCurrent >= vPoints.size() )return NULL;
	iCurrent++;
	if( iCurrent > vPoints.size()-1 )return NULL;
	return &(*(vPoints.begin()+iCurrent));
}

Point* Way::SearchWayPoint( float v[3] ){
	double mind;
	float  dv[3];
	bool   isf = false;
	mind = 1.7e307;
	std::vector<Point>::iterator mini;
	if( vPoints.empty() )return NULL;
	int index = 0;
	for( std::vector<Point>::iterator i = vPoints.begin();i!=vPoints.end();i++ ){
		Point& pt = *i;
		vector_sub( dv,pt.point,v );
		double d = vector_mod2( dv );
		if( mind > d ){
			mind = d;
			mini = i;
			iCurrent = index;
			isf = true;
		}
		index++;
	}
	if( !isf )return NULL;
	return &(*mini);
}

Area::Area( const char* na,const char* zo ){
	name = na;
	zone = zo;
}

void Way::SetWayPointArea( const char* name,const char* zone ){
	if( !name  || !zone )return;
	int ind = 0;
	for( std::vector<Area>::iterator i = vAreas.begin();i!=vAreas.end();i++ ){
		if( i->name==name&&i->zone==zone ){
			CurrentAreaIndex = ind;
			return;
		}
		ind++;
	}
	vAreas.push_back( Area(name,zone) );
	CurrentAreaIndex = (int)(vAreas.size()-1);
}

Cross::Cross(){
	isLoad = false;
}

Cross::~Cross(){
	clear();
}

bool Cross::Save(){
	std::ofstream fs,fs1,fs2;
	std::string fname;
	fname = sWayPoint;
	fname += sCross;
	fs.open( fname.c_str() );
	if( fs.is_open() ){
		for( std::vector<CrossPoint>::iterator i = vCP.begin();
			i!=vCP.end();i++ ){
				fs<<i->path1<<" "<<i->index1<<" "<<i->path2<<" "<<i->index2<<" "<<
					i->point[0]<<" "<<i->point[1]<<" "<<i->point[2]<<" "<<
					i->link[0]<<" "<<i->link[1]<<" "<<i->link[2]<<" "<<i->link[3]<<" "<<
					i->dic[0]<<" "<<i->dic[1]<<" "<<i->dic[2]<<" "<<i->dic[3]<<" "<<
					i->index_vcp<<" "<<i->areaIndex<<std::endl;
		}
		fs.close();
	}else return false;

	//存储区域
	fname = sWayPoint;
	fname += sAllArea;
	fs1.open( fname.c_str() );
	if( fs1.is_open() ){
		for( std::vector<Area>::iterator i = vAR.begin();i!=vAR.end();i++ ){
			fs1<<i->name<<std::endl;
		}
		fs1.close();
	}else return false;

	//存储区域节点
	fname = sWayPoint;
	fname += sAllNode;
	fs2.open( fname.c_str() );
	if( fs2.is_open() ){
		for( std::vector<RegionNode>::iterator i=vRN.begin();i!=vRN.end();i++ ){
			fs2<<i->iAreaIndex[0]<<" "<<i->iAreaIndex[1]<<" "<<i->WayName<<" "<<i->WayIndex<<std::endl;
		}
		fs2.close();
	}else return false;

	return true;
}

bool Cross::Load(){
	std::ifstream is,is1,is2;
	std::string fname;
	fname = sWayPoint;
	fname += sCross;

	if( isLoad )return true;

	clear();
	is.open( fname.c_str() );
	if( is.is_open() ){
		CrossPoint cp;
		while( is>>cp.path1>>cp.index1>>cp.path2>>cp.index2>>cp.point[0]>>cp.point[1]>>cp.point[2]>>
				cp.link[0]>>cp.link[1]>>cp.link[2]>>cp.link[3]>>
				cp.dic[0]>>cp.dic[1]>>cp.dic[2]>>cp.dic[3]>>cp.index_vcp>>cp.areaIndex ){
			vCP.push_back(cp);
		}
		is.close();
	}else return false;

	//装入区域
	fname = sWayPoint;
	fname += sAllArea;
	is1.open( fname.c_str() );
	if( is1.is_open() ){
		Area ar(" "," ");
		while( is1>>ar.name ){
			vAR.push_back( ar );
		}
		is1.close();
	}else return false;

	//装入区域节点
	fname = sWayPoint;
	fname += sAllNode;
	is2.open( fname.c_str() );
	if( is2.is_open() ){
		RegionNode rn;
		while( is2>>rn.iAreaIndex[0]>>rn.iAreaIndex[1]>>rn.WayName>>rn.WayIndex ){
			vRN.push_back( rn );
		}
		is2.close();
	}else return false;

	LoadAllWay();
	BuildRegionLink();
	isLoad = true;
	return true;
}

RegionLink::~RegionLink(){
	vLink.clear();
}

void Cross::BuildRegionLink(){
	for( std::vector<Area>::iterator m = vAR.begin();m!=vAR.end();m++ ){
		RegionLink rl;
		rl.iAreaIndex = (int)(m-vAR.begin());
		vRL.push_back(rl);
	}
	for( std::vector<RegionNode>::iterator i = vRN.begin();i!=vRN.end();i++ ){
		vRL.at(i->iAreaIndex[0]).vLink.push_back(i->iAreaIndex[1]);
		vRL.at(i->iAreaIndex[1]).vLink.push_back(i->iAreaIndex[0]);
	}
}

void Cross::LoadAllWay(){
	std::string path,name;
	_finddata_t fd;
	intptr_t handle;

	path = sWayPoint;
	path += "*";
	path += sWay;

	handle = _findfirst( path.c_str(),&fd );
	if( handle!=-1 ){
		do{
			name = fd.name;
			Way* pway = new Way();
			if( pway->Load( name.substr(0,name.size()-strlen(sWay)).c_str() ) ){
				vWN.push_back( pway );
			}else{
				delete pway;
			}
		}while( !_findnext( handle,&fd ) );
	}
	_findclose( handle );
}

bool CrossPoint::operator<(CrossPoint& cp){
	if( path1 == cp.path1 )
		return index1 < cp.index1;
	if( path1 == cp.path2 )
		return index1 < cp.index2;
	if( path2 == cp.path1 )
		return index2 < cp.index1;
	if( path2 == cp.path2 )
		return index2 < cp.index2;
	return false;
}

int GetIndexByName( CrossPoint& cp,std::string& name ){
	if( cp.path1 == name )return cp.index1;
	if( cp.path2 == name )return cp.index2;
	return -1;
}

float Way::CalcLength( int iFrom,int iTo ){
	if( iFrom < 0 || iTo < 0 )return 0;
	if( iFrom == iTo )return 0;
	if( iFrom > (int)(vPoints.size()-1) || iTo > (int)(vPoints.size()-1) )return 0;
	int imin,imax;
	imin = min( iFrom,iTo );
	imax = max( iFrom,iTo );
	float d = 0;
	float p[3];
	for( std::vector<Point>::iterator i = vPoints.begin()+imin;i<=(vPoints.begin()+imax);i++ ){
		if( (i+1)==vPoints.end() )break;
		vector_sub(p,i->point,(i+1)->point);
		d += vector_mod(p);
	}
	return d;
}

void Cross::BuildCPLink(){
	{
		int ind = 0;
		for( std::vector<CrossPoint>::iterator i=vCP.begin();i!=vCP.end();i++ ){
			i->index_vcp = ind;
			for( int k = 0;k<4;k++ ){
				i->link[k] = -1;
				i->dic[k] = 0;
			}
			ind++;
		}
	}
	{
		std::vector<CrossPoint> vcp;
		for( std::vector<Way*>::iterator i = vWN.begin();i!=vWN.end();i++ ){
			//先找同一条Way上的交点
			for( std::vector<CrossPoint>::iterator k = vCP.begin();k!=vCP.end();k++ ){
				if( k->path1 == (*i)->Name||k->path2 == (*i)->Name ){
					vcp.push_back( *k );
				}
			}
			//然后按顺序排序
			std::sort(vcp.begin(),vcp.end());
			//然后顺序连接它们
			for( std::vector<CrossPoint>::iterator m = vcp.begin();m!=vcp.end();m++ ){
				if( (m+1)==vcp.end() )break;
				for( int n = 0;n<4;n++ ){
					if( vCP.at(m->index_vcp).link[n] == -1 ){
						vCP.at(m->index_vcp).link[n] = (m+1)->index_vcp;
						vCP.at(m->index_vcp).dic[n] = (*i)->CalcLength( GetIndexByName(*m,(*i)->Name),GetIndexByName(*(m+1),(*i)->Name) );
						for( int j = 0;j<4;j++ ){
							if( vCP.at((m+1)->index_vcp).link[j] == -1 ){
								vCP.at((m+1)->index_vcp).link[j] = m->index_vcp;
								vCP.at((m+1)->index_vcp).dic[j] = vCP.at(m->index_vcp).dic[n];
								break;
							}
						}
						break;
					}
				}
			}
			//清除准备连接下一条路线上的交点
			vcp.clear();
		}
	}
}

//重新计算所有的交叉点,及其他们的连接关系，和相邻交点间的距离
//如果两个交点有两条线连接，去距离近的连接，去掉距离远的
bool Cross::ReCalc(){
	vCP.clear();
	vAR.clear();
	vRN.clear();
	vRL.clear();
	//先计算全部交点
	{
		for( std::vector<Way*>::iterator i=vWN.begin();i!=vWN.end();i++ ){
			for( std::vector<Way*>::iterator j=i+1;j!=vWN.end();j++ ){
				CalcCross( *i,*j );
			}
		}
	}
	//建立连接
	BuildCPLink();
	//这里删除一些点，条件是如果有一个点可以在50码里面再次找到自己就删除自己
	//就是一个圈，并且圈的长度小于50
    //暂时不进行优化
	//删除完成重新建立连接
	//BuildCPLink();
	//搜索全部的区域节点
	{
		for( std::vector<Way*>::iterator i = vWN.begin();i!=vWN.end();i++ ){
			if( (*i)->vAreas.empty() )continue;
			for( std::vector<Point>::iterator k = (*i)->vPoints.begin();k!=(*i)->vPoints.end();k++ ){
				if( (k+1)==(*i)->vPoints.end() )break;

				if( (*i)->vAreas.at(k->areaidex).name != (*i)->vAreas.at((k+1)->areaidex).name //||
					//(*i)->vAreas.at(k->areaidex).zone != (*i)->vAreas.at((k+1)->areaidex).zone ){
					){
					//发现一个区域节点
					RegionNode rn;
					rn.iAreaIndex[0] = AddArea( (*i)->vAreas.at(k->areaidex) );
					rn.iAreaIndex[1] = AddArea( (*i)->vAreas.at((k+1)->areaidex) );
					rn.WayName = (*i)->Name;
					rn.WayIndex = (int)(k-(*i)->vPoints.begin());
					//这里加一个条件，就是同一条路线不可以多次穿越两个相同的区域
					if( std::find( vRN.begin(),vRN.end(),rn )==vRN.end() )
						vRN.push_back( rn );
				}
			}
		}
	}
	BuildRegionLink();
	return true;
}

void Cross::clear(){
	vRL.clear();
	vCP.clear();
	vAR.clear();
	vWalkLimit.clear();
	for( std::vector<Way*>::iterator i=vWN.begin();i!=vWN.end();i++ )
		delete (*i);
	vWN.clear();
}
//判断两个空间盒子是不是重叠
bool Cross::IsCrossBox( float box1[6],float box2[6] ){
	if( box1[0] > box2[1] || box1[1] < box2[0] )return false;
	if( box1[2] > box2[3] || box1[3] < box2[2] )return false;
	if( box1[4]-5 > box2[5] || box1[5] < box2[4]-5 )return false;
	return true;
}

//计算两条直线的交点，在x,y平面上的交点
bool Cross::IsCrossLine( float p[3],float line1[6],float line2[6] ){
	if( min(line1[0],line1[3]) > max(line2[0],line2[3])||
		max(line1[0],line1[3]) < min(line2[0],line2[3]))return false;
	if( min(line1[1],line1[4]) > max(line2[1],line2[4])||
		max(line1[1],line1[4]) < min(line2[1],line2[4]))return false;
	if( min(line1[2],line1[5])-5 > max(line2[2],line2[5])||
		max(line1[2],line1[5]) < min(line2[2],line2[5])-5 )return false; //z允许稍微不在一个面上
///////////////////
//1
//	x = (line1[3]-line1[0])*t + line1[0];
//	y = (line1[4]-line1[1])*t + line1[1];
//
//	xx = (line2[3]-line2[0])*tt + line2[0];
//	yy = (line2[4]-line2[1])*tt + line2[1];
//2
//	(line1[3]-line1[0])*t + line1[0] = (line2[3]-line2[0])*tt + line2[0];
//	(line1[4]-line1[1])*t + line1[1] = (line2[4]-line2[1])*tt + line2[1];
//3
//	(line2[4]-line2[1])*(line1[3]-line1[0])*t + (line2[4]-line2[1])*line1[0] = (line2[3]-line2[0])*(line2[4]-line2[1])*tt + (line2[4]-line2[1])*line2[0];
//	(line2[3]-line2[0])*(line1[4]-line1[1])*t + (line2[3]-line2[0])*line1[1] = (line2[3]-line2[0])*(line2[4]-line2[1])*tt + (line2[3]-line2[0])*line2[1];
//4
//	(line2[4]-line2[1])*(line1[3]-line1[0])*t + (line2[4]-line2[1])*line1[0] - (line2[3]-line2[0])*(line1[4]-line1[1])*t - (line2[3]-line2[0])*line1[1] = 
//	(line2[4]-line2[1])*line2[0] - (line2[3]-line2[0])*line2[1];
//5
//  ( (line2[4]-line2[1])*(line1[3]-line1[0])-(line2[3]-line2[0])*(line1[4]-line1[1]) )*t = 
//  (line2[4]-line2[1])*line2[0] - (line2[3]-line2[0])*line2[1] - (line2[4]-line2[1])*line1[0] + (line2[3]-line2[0])*line1[1]
//6
//	( (line2[4]-line2[1])*(line1[3]-line1[0])-(line2[3]-line2[0])*(line1[4]-line1[1]) )*t = 
//	( (line2[4]-line2[1])*(line2[0]-line1[0])-(line2[3]-line2[0])*(line2[1]-line1[1]) )
//=====================================================================================
//1 
//	(line1[3]-line1[0])*t + line1[0] = (line2[3]-line2[0])*tt + line2[0]
//  ( (line1[3]-line1[0])*t + line1[0]-line2[0] )/(line2[3]-line2[0]) = tt
//2 (line1[4]-line1[1])*t + line1[1] = (line2[4]-line2[1])*tt + line2[1]
//	( (line1[4]-line1[1])*t + line1[1]-line2[1] )/(line2[4]-line2[1]) = tt
///////////////////
	double a = ( (line2[4]-line2[1])*(line1[3]-line1[0])-(line2[3]-line2[0])*(line1[4]-line1[1]) );
	double b = ( (line2[4]-line2[1])*(line2[0]-line1[0])-(line2[3]-line2[0])*(line2[1]-line1[1]) );
	double t,tt;
	if( a != 0 )
		t = b/a;
	else
		return false; //平行
	if( (line2[3]-line2[0]) != 0 )
		tt = ( (line1[3]-line1[0])*t + line1[0]-line2[0] )/(line2[3]-line2[0]);
	else
		tt =( (line1[4]-line1[1])*t + line1[1]-line2[1] )/(line2[4]-line2[1]);
	if( t >= 0 && t <= 1 && tt >= 0 && tt <= 1 ){
		p[0] = (float)((line1[3]-line1[0])*t+line1[0]);
		p[1] = (float)((line1[4]-line1[1])*t+line1[1]);
		p[2] = line1[2];
	}else return false;
	return true;
}

void Cross::AddCrossPoint( float p[3],std::string name1,int index1,std::string name2,int index2,int inda ){
	vCP.push_back( CrossPoint( p,name1,index1,name2,index2,inda ) );
}

CrossPoint::CrossPoint(){
	for( int i = 0;i < 4;i++ ){
		link[i] = -1;
		dic[i] = 0;
	}
}

CrossPoint::CrossPoint( float p[3],std::string name1,int ind1,std::string name2,int ind2,int inda ){
	point[0] = p[0];
	point[1] = p[1];
	point[2] = p[2];
	path1 = name1;
	path2 = name2;
	index1 = ind1;
	index2 = ind2;
	areaIndex = inda;
	for( int i = 0;i < 4;i++ ){
		link[i] = -1;
		dic[i] = 0;
	}
}

//区域有交叉，因为副本的坐标和世界坐标可能相同，使用区域可以避免
bool IsCrossArea(std::vector<Area>& vA1,std::vector<Area>& vA2 ){
	for( std::vector<Area>::iterator i = vA1.begin();i!=vA1.end();i++ ){
		for( std::vector<Area>::iterator k = vA2.begin();k!=vA2.end();k++ ){
			if( i->name == k->name )
				return true;
		}
	}
	return false;
}

int Cross::AddArea( Area& a ){
	for( std::vector<Area>::iterator i = vAR.begin();i!=vAR.end();i++ ){
		if( i->name == a.name )//&& i->zone == a.zone )
			return (int)(i-vAR.begin());
	}
	vAR.push_back( a );
	return (int)(vAR.size()-1);
}

bool Cross::CalcCross( Way* pway1,Way* pway2 ){
	float pcross[3],line1[6],line2[6];
	float prevCross[3];
	bool b = false;
	if( pway1 == pway2 ){
		//自身相交
	}else{
		if( IsCrossBox( pway1->box,pway2->box )&&IsCrossArea(pway1->vAreas,pway2->vAreas)  ){
			for( std::vector<Point>::iterator i = pway1->vPoints.begin();i!=pway1->vPoints.end();i++){
				std::vector<Point>::iterator i2 = i+1;
				if( i2 == pway1->vPoints.end() )break;
				for( std::vector<Point>::iterator j = pway2->vPoints.begin();j!=pway2->vPoints.end();j++){
					std::vector<Point>::iterator j2 = j+1;
					if( j2 == pway2->vPoints.end() )break;
					line1[0] = i->point[0];
					line1[1] = i->point[1];
					line1[2] = i->point[2];
					line1[3] = i2->point[0];
					line1[4] = i2->point[1];
					line1[5] = i2->point[2];
					line2[0] = j->point[0];
					line2[1] = j->point[1];
					line2[2] = j->point[2];
					line2[3] = j2->point[0];
					line2[4] = j2->point[1];
					line2[5] = j2->point[2];
					if( IsCrossLine( pcross,line1,line2 ) ){
						if( b ){ //确保两条线在100码内最多有1个交点
							float v[3];
							vector_sub(v,prevCross,pcross);
							if( vector_mod(v) < 100 )
								continue;
						}
						prevCross[0] = pcross[0];
						prevCross[1] = pcross[1];
						prevCross[2] = pcross[2];
						int inda = AddArea( pway1->vAreas.at( i->areaidex ) );
						AddCrossPoint( pcross,pway1->Name,(int)(i-pway1->vPoints.begin()),pway2->Name,(int)(j-pway2->vPoints.begin()),inda );
						b = true;
					}
				}
			}
		}
	}
	return true;
}

//计算一条从点from到点to的最近路线
bool Cross::Goto(float from[3],float to[3]){
	std::string path_from,path_to;
	int iFrom,iTo;

	iFrom = 0;
	iTo = 0;
	if( !GetNearstPath( from,path_from,iFrom )||
		!GetNearstPath( to,path_to,iTo ) )	
		return false;

	Way* pFrom = FindWay( path_from );
	Way* pTo = FindWay( path_to );
	if( pFrom && pTo ){
		Way way;
		if( SreachNearstWay( way,pFrom,iFrom,pTo,iTo ) ){
			g_way = way;
			return true;
		}
	}
	return false;
}

bool CrossPoint::operator==(CrossPoint& cp){
	if( path1==cp.path1&&path2==cp.path2&&
		index1==cp.index1&&index2 == cp.index2 )
		return true;
	return false;
}
/*
CrossPoint::CrossPoint( CrossPoint& cp ){
	path1 = cp.path1;
	path2 = cp.path2;
	index1 = cp.index1;
	index2 = cp.index2;
	point[0] = cp.point[0];
	point[1] = cp.point[1];
	point[2] = cp.point[2];
	link[0] = cp.link[0];
	link[1] = cp.link[1];
	link[2] = cp.link[2];
	link[3] = cp.link[3];
}

CrossPoint& CrossPoint::operator=(CrossPoint& cp){
	path1 = cp.path1;
	path2 = cp.path2;
	index1 = cp.index1;
	index2 = cp.index2;
	point[0] = cp.point[0];
	point[1] = cp.point[1];
	point[2] = cp.point[2];
	link[0] = cp.link[0];
	link[1] = cp.link[1];
	link[2] = cp.link[2];
	link[3] = cp.link[3];
	return *this;
}*/

bool RegionNode::operator==(const RegionNode& rn){
	if( iAreaIndex[0]==rn.iAreaIndex[0]&&
		iAreaIndex[1]==rn.iAreaIndex[1])
		return true;
	if( iAreaIndex[1]==rn.iAreaIndex[0]&&
		iAreaIndex[0]==rn.iAreaIndex[1])
		return true;
	return false;
}

//已知way和路上的一个点iP,找到和他相邻的两个交点的索引
void Cross::GetNearstCross( int iCrossFrom[4],int iCrossTo[4],Way* pFrom,int iFrom,Way* pTo,int iTo ){
	int iF0,iF1,iT0,iT1,iFF0,iFF1,iTT0,iTT1;
	size_t if0,if1,it0,it1;
	//先找路径上和From To点相邻的交点
	iF0 = INT_MAX;
	iF1 = INT_MIN;
	iT0 = INT_MAX;
	iT1 = INT_MIN;
	for( std::vector<CrossPoint>::iterator i = vCP.begin();i!=vCP.end();i++ ){
		if( i->path1 == pFrom->Name ){
			if( i->index1 >= iFrom ){
				if( iF0 > i->index1-iFrom ){
					iF0 = i->index1-iFrom;
					if0 = i->index_vcp;
					iFF0 = i->index1;
				}
			}else{
				if( iF1 < i->index1-iFrom ){
					iF1 = i->index1-iFrom;
					if1 = i->index_vcp;
					iFF1 = i->index1;
				}
			}
		}
		if( i->path2 == pFrom->Name ){
			if( i->index2 >= iFrom ){
				if( iF0 > i->index2-iFrom ){
					iF0 = i->index2-iFrom;
					if0 = i->index_vcp;
					iFF0 = i->index2;
				}
			}else{
				if( iF1 < i->index2-iFrom ){
					iF1 = i->index2-iFrom;
					if1 = i->index_vcp;
					iFF1 = i->index2;
				}
			}
		}
		if( i->path1 == pTo->Name ){
			if( i->index1 >= iTo ){
				if( iT0 > i->index1-iTo ){
					iT0 = i->index1-iTo;
					it0 = i->index_vcp;
					iTT0 = i->index1;
				}
			}else{
				if( iT1 < i->index1-iTo ){
					iT1 = i->index1-iTo;
					it1 = i->index_vcp;
					iTT1 = i->index1;
				}
			}
		}
		if( i->path2 == pTo->Name ){
			if( i->index2 >= iTo ){
				if( iT0 > i->index2-iTo ){
					iT0 = i->index2-iTo;
					it0 = i->index_vcp;
					iTT0 = i->index2;
				}
			}else{
				if( iT1 < i->index2-iTo ){
					iT1 = i->index2-iTo;
					it1 = i->index_vcp;
					iTT1 = i->index2;
				}
			}
		}
	}
	for( int m = 0;m<4;m++ ){
		iCrossFrom[m] = -1;
		iCrossTo[m] = -1;
	}
	if( iF0 != INT_MAX ){
		iCrossFrom[0]=iFF0; //交点在pFrom上的索引
		iCrossFrom[1]=(int)if0;  //交点在vCP中的索引
	}
	if( iF1 != INT_MIN ){
		iCrossFrom[2]=iFF1;
		iCrossFrom[3]=(int)if1;
	}
	if( iT0 != INT_MAX ){
		iCrossTo[0]=iTT0;
		iCrossTo[1]=(int)it0;
	}
	if( iT1 != INT_MIN ){
		iCrossTo[2]=iTT1;
		iCrossTo[3]=(int)it1;
	}
}

//替归搜索vCP
//加入一个限定条件，一个区域只能去一次
bool Cross::WalkCross( WalkParam& wp ){
	bool b = false;
	if( *(wp.pFrom) == *(wp.pTo) ){
		float d = CalcCrossDistance( *(wp.vcp) )+wp.dis0+wp.dis1;
		if( d < wp.minDis ){
			wp.minDis = d;
			wp.vcp_min->clear();
			for( std::vector<CrossPoint>::iterator i = wp.vcp->begin();i!=wp.vcp->end();i++ ){
				wp.vcp_min->push_back( *i );
			}
			wp.vcp_min->push_back( *(wp.pTo) );
		}
		return true;
	}
	//区域限定
	if( std::find( vWalkLimit.begin(),vWalkLimit.end(),wp.pFrom->areaIndex )==vWalkLimit.end() )return false;
	//路径里面不能放入重复的交点
	for( std::vector<CrossPoint>::reverse_iterator i = wp.vcp->rbegin();i!=wp.vcp->rend();i++ ){
		if( *i == *(wp.pFrom) )return false;
		if( i->areaIndex != wp.pFrom->areaIndex )b = true;
		if( b && i->areaIndex == wp.pFrom->areaIndex )return false;//发现有要穿越以前来过的区域
	}
	//搜索相邻的4个交点
	wp.vcp->push_back( *(wp.pFrom) );
	for( int k = 0;k<4;k++ ){
		if( wp.pFrom->link[k] != -1 ){ //=-1表示没有对应的交点
			CrossPoint* pbak = wp.pFrom;
			wp.pFrom = &vCP.at(wp.pFrom->link[k]);
			WalkCross( wp );
			wp.pFrom = pbak;
		}
	}
	wp.vcp->pop_back();
	return false;
}

void Cross::WalkRegionNode( std::vector<int>& vip,int iFrom,int iTo ){
	if( iFrom == iTo ){
		vip.push_back( iTo );
		if( vWalkLimit.empty() ){
			vWalkLimit.resize( vip.size() );
			std::copy( vip.begin(),vip.end(),vWalkLimit.begin() );
		}else if( vip.size() < vWalkLimit.size() ){
			vWalkLimit.resize( vip.size() );
			std::copy( vip.begin(),vip.end(),vWalkLimit.begin() );
		}
		vip.pop_back();
		return;
	}
	if( std::find( vip.begin(),vip.end(),iFrom ) !=  vip.end() )return;

	vip.push_back( iFrom );
	for( std::vector<int>::iterator i = vRL.at(iFrom).vLink.begin();i!=vRL.at(iFrom).vLink.end();i++ ){
		WalkRegionNode( vip,*i,iTo );
	}
	vip.pop_back();
}

void Cross::CalcWalkLimit( Area& From,Area& To ){
	int iFrom = AddArea(From);
	int iTo = AddArea(To);
	vWalkLimit.clear();
	std::vector<int> vip;
	WalkRegionNode( vip,iFrom,iTo );
}

//搜索一个冲pFrom的iFrom到pTo的iTo的最近线路,并且把路线存入way
bool Cross::SreachNearstWay( Way& way,Way* pFrom,int iFrom,Way* pTo,int iTo ){
	std::vector<CrossPoint> vcp;
	std::vector<CrossPoint> vcp_min;
	int iCrossFrom[4],iCrossTo[4],it;
	WalkParam wp;

	//先找路径上和From To点相邻的交点
	GetNearstCross( iCrossFrom,iCrossTo,pFrom,iFrom,pTo,iTo );
	//从上面的可能最多4的交点，搜索
	wp.vcp_min = &vcp_min;
	wp.vcp = &vcp;
	wp.minDis = (float)1.7e20;
	wp.dis0 = 0;
	wp.dis1 = 0;
	
	//限定可以走的区域
	CalcWalkLimit( pFrom->vAreas.at(pFrom->vPoints.at(iFrom).areaidex),pTo->vAreas.at(pTo->vPoints.at(iTo).areaidex) );
	for( int i = 0;i < 2;i++ ){
		if( iCrossFrom[2*i]!=-1 ){
			wp.dis0 = pFrom->CalcLength(iFrom,iCrossFrom[2*i]);
			wp.pFrom = &vCP.at(iCrossFrom[2*i+1]);
			for( int j = 0;j < 2;j++ ){
				if( iCrossTo[2*j]!=-1 ){
					wp.dis1 = pTo->CalcLength(iTo,iCrossTo[2*j]);
					wp.pTo = &vCP.at(iCrossTo[2*j+1]);
					WalkCross( wp );
				}
			}
		}
	}
	//调试打印序列
	//{	for(std::vector<CrossPoint>::iterator i = vcp_min.begin();i!=vcp_min.end();i++){
	//	printf("%d\n",i->index_vcp );
	//	}
	//}
	//找到距离最近的点然后用这些交点构造way
	way.Name = "CalcWay";
	if( vcp_min.empty() ){
		if( pFrom->Name == pTo->Name ){
			AddPointRang( way.vPoints,pFrom,iFrom,iTo );
			return true;
		}
	}else{
		way.vAdds.clear();
		way.vAreas.clear();
		way.vPoints.clear();

		if( pFrom->Name == vcp_min.at(0).path1 )
			it = vcp_min.at(0).index1;
		else
			it = vcp_min.at(0).index2;
		AddPointRang( way.vPoints,pFrom,iFrom,it );
		for( std::vector<CrossPoint>::iterator i = vcp_min.begin();i!=vcp_min.end();i++ ){
			if( (i+1)==vcp_min.end() )break;
			AddPointCross( way.vPoints,*i,*(i+1) );
		}
		if( pTo->Name == vcp_min.at(vcp_min.size()-1).path1 )
			it = vcp_min.at(vcp_min.size()-1).index1;
		else
			it = vcp_min.at(vcp_min.size()-1).index2;
		AddPointRang( way.vPoints,pTo,it,iTo );
		return true;
	}
	return false;
}

//把从cp0到cp1中间的点加入到vp
void Cross::AddPointCross( std::vector<Point>& vp,CrossPoint& cp0,CrossPoint& cp1 ){
	if( cp0.path1 == cp1.path1 ){
		vp.push_back( Point(cp0.point,0,0) );
		AddPointRang2( vp,FindWay(cp0.path1),cp0.index1,cp1.index1 );
		vp.push_back( Point(cp1.point,0,0) );
		return;
	}
	if( cp0.path1 == cp1.path2 ){
		vp.push_back( Point(cp0.point,0,0) );
		AddPointRang2( vp,FindWay(cp0.path1),cp0.index1,cp1.index2 );
		vp.push_back( Point(cp1.point,0,0) );
		return;
	}
	if( cp0.path2 == cp1.path1 ){
		vp.push_back( Point(cp0.point,0,0) );
		AddPointRang2( vp,FindWay(cp0.path2),cp0.index2,cp1.index1 );
		vp.push_back( Point(cp1.point,0,0) );
		return;
	}
	if( cp0.path2 == cp1.path2 ){
		vp.push_back( Point(cp0.point,0,0) );
		AddPointRang2( vp,FindWay(cp0.path2),cp0.index2,cp1.index2 );
		vp.push_back( Point(cp1.point,0,0) );
		return;
	}
}

bool Cross::AddPointRang2( std::vector<Point>& vp,Way* pway,int iFrom,int iTo ){
	if( iFrom < iTo ){
		iFrom++;
		return AddPointRang( vp,pway,iFrom,iTo );
	}else if( iFrom > iTo ){
		iTo++;
		return AddPointRang( vp,pway,iFrom,iTo );
	}
	return true;
}

//把路径pway从iFrom到iTo的点加入vp
bool Cross::AddPointRang( std::vector<Point>& vp,Way* pway,int iFrom,int iTo ){
	if( !pway )return false;
	if( (int)pway->vPoints.size() <= max(iFrom,iTo) )return false;
	if( iFrom < 0 || iTo < 0 || iFrom == iTo )return false;
	int e;
	if( iFrom > iTo )
		e = -1;
	else
		e = 1;
	for( std::vector<Point>::iterator i = pway->vPoints.begin()+iFrom;
		i!=pway->vPoints.begin()+iTo;i+=e ){
			vp.push_back( *i );
	}
	return true;
}

//计算交点与交点连接起来的长度
float Cross::CalcCrossDistance( std::vector<CrossPoint>& vcp ){
	float d = 0;
	if( vcp.size() < 2 )return 0;
	for( std::vector<CrossPoint>::iterator i = vcp.begin();i!=vcp.end();i++ ){
		if( i+1 == vcp.end() )break;
		for( int k = 0;k < 4;k++ ){
			if( i->link[k] != -1 ){
				if( vCP.at(i->link[k]) == *(i+1) ){
					d += i->dic[k];
				}
			}
		}
	}
	return d;
}

//找到一条名字为name的路线并且返回指针
Way* Cross::FindWay( std::string name ){
	for( std::vector<Way*>::iterator i = vWN.begin();i!=vWN.end();i++ ){
		if( (*i)->Name == name )
			return (*i);
	}
	return NULL;
}

//找到一条离pos最近的线路上的点，找到把路线名称放入name,ind为索引
bool Cross::GetNearstPath(float pos[3],std::string& name,int& ind){
	float v[3];
	double d = 1.7e20;
	for( std::vector<Way*>::iterator i = vWN.begin();i!=vWN.end();i++ ){
		//先和way的box扩大50码做比较，在里面就继续
		if( pos[0] < max((*i)->box[0],(*i)->box[1])+50&&
			pos[0] > min((*i)->box[0],(*i)->box[1])-50&&
			pos[1] < max((*i)->box[2],(*i)->box[3])+50&&
			pos[1] > min((*i)->box[2],(*i)->box[3])-50 ){
				Point* p = (*i)->SearchWayPoint(pos);
				if( p ){
					vector_sub( v,p->point,pos );
					float a = vector_mod2( v );
					if( d > a ){
						d = a;
						name = (*i)->Name;
						ind = (int)((*i)->iCurrent);
					}
				}
		}
	}
	if( d == 1.7e20 )return false;
	return true;
}

//去一个区域的中心点，区域名称为name,pos为取出的点,也为设置点
bool Cross::GetAreaPointByName(float pos[3],std::string name){
	int index;
	bool bFind;
	std::vector<Point> vp;
	for( std::vector<Way*>::iterator i = vWN.begin();i!=vWN.end();i++ ){
		index = 0;
		bFind = false;
		for( std::vector<Area>::iterator k = (*i)->vAreas.begin();k!=(*i)->vAreas.end();k++ ){
			if( k->name == name || k->zone == name ){
				bFind = true;
				break;
			}
			index++;
		}
		if( bFind ){
			for( std::vector<Point>::iterator m = (*i)->vPoints.begin();m!=(*i)->vPoints.end();m++ ){
				if( m->areaidex == index ){
					vp.push_back( *m );
				}
			}
		}
	}
	if( !vp.empty() ){
		float v[3],vv[3];
		float mind = (float)1.7e20;
		for( std::vector<Point>::iterator i = vp.begin();i!=vp.end();i++ ){
			vector_sub( v,i->point,pos );
			float d = vector_mod2( v );
			if( d < mind ){
				mind = d;
				vv[0] = i->point[0];
				vv[1] = i->point[1];
				vv[2] = i->point[2];
			}
		}
		pos[0] = vv[0];
		pos[1] = vv[1];
		pos[2] = vv[2];
		return true;
	}
	return false;
}

int lua_CreateWay( void* p ){
	if( Lua_isstring(p,1) ){
		size_t len;
		const char* name = Lua_tolstring(p,1,&len);
		if( name ){
			if( g_way.ReNew( name ) )
				Lua_pushnumber(p,1);
			else
				Lua_pushnil( p );
			return 1;
		}
	}
	Lua_pushnil( p );
	return 1;
}

int lua_SaveWay( void* p ){
	if( !g_way.Name.empty() ){
		if( g_way.Save() )
			Lua_pushnumber(p,1);
		else
			Lua_pushnil( p );
		return 1;
	}
	Lua_pushnil( p );
	return 1;
}

int lua_LoadWay( void* p ){
	if( Lua_isstring(p,1) ){
		size_t len;
		const char* name = Lua_tolstring(p,1,&len);
		if( name ){
			if( g_way.Load( name ) )
				Lua_pushnumber(p,1);
			else
				Lua_pushnil( p );
			return 1;
		}
	}
	Lua_pushnil( p );
	return 1;
}

int lua_AddWayPoint( void* p ){
	float v[3];
	int   action = NOTHING;
	bool isok = true;
	if( Lua_isnumber(p,1) ){
		v[0] = (float)Lua_tonumber(p,1);
	}else isok = false;
	if( Lua_isnumber(p,2) ){
		v[1] = (float)Lua_tonumber(p,2);
	}else isok = false;
	if( Lua_isnumber(p,3) ){
		v[2] = (float)Lua_tonumber(p,3);
	}else isok = false;
	if( Lua_isnumber(p,4) ){
		action = (int)Lua_tonumber(p,4);
	}
	if( isok ){
		if( g_way.AddPoint( v,action ) ){
			Lua_pushnumber(p,1);
			return 1;
		}
	}
	Lua_pushnil( p );
	return 1;
}

int lua_CurrentWayPoint( void* p ){
	Point* pt = g_way.CurrentWayPoint();
	if( pt ){
		Lua_pushnumber(p,pt->point[0] );
		Lua_pushnumber(p,pt->point[1] );
		Lua_pushnumber(p,pt->point[2] );
		Lua_pushnumber(p,pt->action );
		return 4;
	}
	Lua_pushnil( p );
	return 1;
}

int lua_PrevWayPoint( void* p ){
	Point* pt = g_way.PrevWayPoint();
	if( pt ){
		Lua_pushnumber(p,pt->point[0] );
		Lua_pushnumber(p,pt->point[1] );
		Lua_pushnumber(p,pt->point[2] );
		Lua_pushnumber(p,pt->action );
		return 4;
	}
	Lua_pushnil( p );
	return 1;
}

int lua_NextWayPoint( void* p ){
	Point* pt = g_way.NextWayPoint();
	if( pt ){
		Lua_pushnumber(p,pt->point[0] );
		Lua_pushnumber(p,pt->point[1] );
		Lua_pushnumber(p,pt->point[2] );
		Lua_pushnumber(p,pt->action );
		return 4;
	}
	Lua_pushnil( p );
	return 1;
}

int lua_SearchWayPoint( void* p ){
	float v[3];
	bool isok = true;
	if( Lua_isnumber(p,1) ){
		v[0] = (float)Lua_tonumber(p,1);
	}else isok = false;
	if( Lua_isnumber(p,2) ){
		v[1] = (float)Lua_tonumber(p,2);
	}else isok = false;
	if( Lua_isnumber(p,3) ){
		v[2] = (float)Lua_tonumber(p,3);
	}else isok = false;

	if( isok ){
		Point* pt = g_way.SearchWayPoint(v);
		if( pt ){
			Lua_pushnumber(p,pt->point[0] );
			Lua_pushnumber(p,pt->point[1] );
			Lua_pushnumber(p,pt->point[2] );
			Lua_pushnumber(p,pt->action );
			return 4;
		}
	}
	Lua_pushnil( p );
	return 1;
}

int lua_ClearWayPoint( void* p ){
	if( g_way.ClearWayPoint() ){
		Lua_pushnumber(p,1);
		return 1;
	}
	Lua_pushnil( p );
	return 1;
}

int lua_SetWayPointArea( void* p ){
	if( Lua_isstring(p,1)){
		size_t len;
		const char* name = Lua_tolstring(p,1,&len);
		if( Lua_isstring(p,2) ){
			const char* zone = Lua_tolstring(p,2,&len);
			std::string na,zo;
			na = name;
			zo = zone;
			if( na.size()==0 )na="nil";
			if( zo.size()==0 )zo="nil";
			g_way.SetWayPointArea(na.c_str(),zo.c_str());
			Lua_pushnumber(p,1);
			return 1;
		}
	}
	Lua_pushnil( p );
	return 1;
}

int lua_LoadCross( void* p ){
	if( g_cross.Load() ){
		Lua_pushnumber(p,1);
		return 1;
	}
	Lua_pushnil( p );
	return 1;
}

int lua_SaveCross( void* p ){
	if( g_cross.Save() ){
		Lua_pushnumber(p,1);
		return 1;
	}
	Lua_pushnil( p );
	return 1;
}

int lua_RecalcCross( void* p ){
	if( g_cross.ReCalc() ){
		Lua_pushnumber(p,1);
		return 1;
	}
	Lua_pushnil( p );
	return 1;
}

HANDLE g_hGoto = NULL;
DWORD  g_dwGoto = 0;
std::string g_strGotoName;
float  g_vFrom[3];
float  g_vTo[3];

DWORD WINAPI GotoCalc( LPVOID lpParam ){
	if( g_cross.GetAreaPointByName( g_vTo,g_strGotoName ) ){
		if( g_cross.Goto( g_vFrom,g_vTo ) ){
			g_hGoto = NULL;
			return 1;
		}
	}
	g_hGoto = NULL;
	return 0;
}

int lua_IsCalcingCross( void* p ){
	if( g_hGoto ){
		Lua_pushnumber(p,1);
		return 1;
	}
	Lua_pushnil( p );
	return 1;
}

int lua_CalcGotoCross( void* p ){
	float v[3];
	bool isok = true;

	if( Lua_isnumber(p,1) ){
		v[0] = (float)Lua_tonumber(p,1);
	}else isok = false;
	if( Lua_isnumber(p,2) ){
		v[1] = (float)Lua_tonumber(p,2);
	}else isok = false;
	if( Lua_isnumber(p,3) ){
		v[2] = (float)Lua_tonumber(p,3);
	}else isok = false;

	if( isok && Lua_isstring(p,4) ){
		size_t len;
		const char* name = Lua_tolstring(p,4,&len);
		g_cross.Load();
		g_vFrom[0] = v[0];
		g_vFrom[1] = v[1];
		g_vFrom[2] = v[2];
		g_vTo[0] = v[0];
		g_vTo[1] = v[1];
		g_vTo[2] = v[2];
		g_strGotoName = name;
		if( g_hGoto == NULL ){
			g_hGoto = CreateThread(NULL,1024*1024,GotoCalc,0,0,&g_dwGoto);
			if( g_hGoto ){
				Lua_pushnumber(p,1);
				return 1;
			}
		}
	}
	Lua_pushnil( p );
	return 1;
}