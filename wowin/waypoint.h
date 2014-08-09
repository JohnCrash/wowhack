#pragma once
#include <vector>
#include <string>

#define NOTHING 0
#define FRAME	1
#define NPC		2

struct DLLEXPORT Point{
	float point[3];
	int	action;
	int areaidex;

	Point();
	Point( float v[3],int a,int ind );
	Point( const Point& p );
};

struct DLLEXPORT Area{
	std::string name;
	std::string zone; //子区域
	Area();
	Area( const char* na,const char* zo );
};

struct DLLEXPORT Way{
	std::vector<Point> vPoints;
	std::string	Name;
	size_t iCurrent;
	std::vector<Point> vAdds;
	Point prevPt;
	Point oldPt;
	
	std::vector<Area> vAreas;
	int  CurrentAreaIndex;
	
	float box[6];

	Way();
	~Way();
	bool ReNew( const char* name );
	bool Save();
	bool Load( const char* name );
	bool AddPoint( float p[3],int action );
	Point* PrevWayPoint();
	Point* NextWayPoint();
	Point* CurrentWayPoint();
	Point* SearchWayPoint( float v[3] );
	bool ClearWayPoint();
	void SetWayPointArea( const char* name,const char* zone );
	float CalcLength( int iFrom,int iTo );
	void operator=(Way& way);
};

struct DLLEXPORT CrossPoint{
	std::string path1;
	std::string path2;
	int index1;
	int index2;
	float point[3];
	int link[4]; //其他和他相连接的CrossPoint,没有就-1
	float dic[4]; //到邻近点的距离
	int index_vcp;//自己在vcp中的索引
	int areaIndex; //区域索引

	CrossPoint();
	//CrossPoint( CrossPoint& cp );
	CrossPoint( float p[3],std::string name1,int index1,std::string name2,int index2,int inda );
	bool operator==(CrossPoint& cp);
	bool operator<(CrossPoint& cp);
	//CrossPoint& operator=(CrossPoint& cp);
};

struct DLLEXPORT WalkParam{
	std::vector<CrossPoint>* vcp;
	std::vector<CrossPoint>* vcp_min;
	CrossPoint* pFrom;
	CrossPoint* pTo;
	float dis0;
	float dis1;
	float minDis;
};

struct DLLEXPORT RegionNode{
	int iAreaIndex[2]; //一个区域节点连接两个区域,vAR的索引
	std::string WayName; //区域节点所在的路线
	int WayIndex;//区域节点所在的路线索引
//	int Index; //自己在vRN中的位置
	bool operator==(const RegionNode& rn);
};

struct DLLEXPORT RegionLink{
	int iAreaIndex; //自己在vAR中的索引
	std::vector<int> vLink; //和他相连的其他区域索引
	~RegionLink();
};


struct DLLEXPORT Cross{
	std::vector<CrossPoint> vCP;
	std::vector<Way*> vWN;

	std::vector<Area> vAR;
	std::vector<RegionNode> vRN; //区域节点
	std::vector<int> vWalkLimit; //限定WalkCross可以搜索的区域

	std::vector<RegionLink> vRL; //一个区域和其他区域的连接

	bool isLoad;

	Cross();
	~Cross();
	bool Save();
	bool Load();
	bool ReCalc();
	void LoadAllWay();
	bool CalcCross( Way* pway1,Way* pway2 );
	bool IsCrossBox( float box1[6],float box2[6] );
	bool IsCrossLine( float p[3],float line1[6],float line2[6] );
	void AddCrossPoint( float p[3],std::string name1,int index1,std::string name2,int index2,int inda );
	bool WalkCross( WalkParam& wp );
	void AddPointCross( std::vector<Point>& vp,CrossPoint& cp0,CrossPoint& cp1 );
	bool AddPointRang( std::vector<Point>& vp,Way* pway,int iFrom,int iTo );
	bool AddPointRang2( std::vector<Point>& vp,Way* pway,int iFrom,int iTo );
	float CalcCrossDistance( std::vector<CrossPoint>& vcp );
	bool SreachNearstWay( Way& way,Way* pFrom,int iFrom,Way* pTo,int iTo );
	bool Goto(float from[3],float to[3] );
	Way* FindWay( std::string name );
	bool GetNearstPath(float pos[3],std::string& name,int& ind);
	bool GetAreaPointByName(float pos[3],std::string name);
	void clear();
	int AddArea( Area& a );
	void GetNearstCross( int iCrossFrom[4],int iCrossTo[4],Way* pFrom,int iFrom,Way* pTo,int iTo );
	void CalcWalkLimit( Area& From,Area& to );
	void WalkRegionNode( std::vector<int>& vip,int iFrom,int iTo );
	void BuildRegionLink();
	void BuildCPLink();
};

/*
template class DLLEXPORT std::vector<CrossPoint>;
template class DLLEXPORT std::vector<Way*>;
template class DLLEXPORT std::vector<Area>;
template class DLLEXPORT std::vector<RegionNode>;
template class DLLEXPORT std::vector<int>;
template class DLLEXPORT std::vector<RegionLink>;
template class DLLEXPORT std::vector<Point>;
template class DLLEXPORT std::basic_string<char>;
*/
extern Way g_way;
extern Cross g_cross;

//lua_CreateWay("way name")
int lua_CreateWay( void* p );
//lua_SaveWay()
int lua_SaveWay( void* p );
//lua_LoadWay("way name")
int lua_LoadWay( void* p );
//lua_AddWayPoint(x,y,z,r,a)
int lua_AddWayPoint( void* p );
//x,y,z,a = lua_CurrentWayPoint()
int lua_CurrentWayPoint( void* p );
//x,y,z,a = lua_PrevWayPoint()
int lua_PrevWayPoint( void* p );
//x,y,z,a = lua_NextWayPoint()
int lua_NextWayPoint( void* p );
//x,y,z,a = lua_ScreahWayPoint(x,y,z)
int lua_SearchWayPoint( void* p );
//lua_ClearWayPoint()
int lua_ClearWayPoint( void* p );

int lua_SetWayPointArea( void* p );
int lua_LoadCross( void* p );
int lua_SaveCross( void* p );
int lua_RecalcCross( void* p );
int lua_CalcGotoCross( void* p );
int lua_IsCalcingCross( void* p );