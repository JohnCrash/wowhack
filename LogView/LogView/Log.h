#pragma once
#include <vector>
#include <string>

struct UnitState{
	std::wstring id;
	std::wstring name;
	std::wstring target;
	std::wstring race;
	bool isParty;
	int health;
	int healthMax;
	int mana;
	double x,y,z;
	std::vector<std::wstring> buff;
	std::vector<std::wstring> debuff;
};

struct UnitLog{
	std::wstring type;
	double time;
	std::wstring sourceId;
	std::wstring sourceName;
	std::wstring sourceFlags;
	std::wstring destId;
	std::wstring destName;
	std::wstring destFlags;
	std::vector<std::wstring> exs;
	std::vector<UnitState> states;
};

enum{
	type_rect,
	type_move,
	type_line,
	type_ellipse,
};

struct MapItem{
	int type; //RECT Eillipse Line
	double x0;
	double x1;
	double y0;
	double y1;
	MapItem();
	MapItem( int type,double x0,double y0,double x1,double y1 );
};

struct AreanaMap{
	std::wstring name;
	int type;
	double left;
	double right;
	double top;
	double bottom;
	std::vector<MapItem> mi;
	HBITMAP hmap;
	AreanaMap();
	std::string GetExePath(void);
	void clacz( double x0,double xx0,double x1,double xx1,double w,double& left,double& right );
	void clacImageRect( double x0,double y0,double x1,double y1,double xx0,double yy0,double xx1,double yy1 );
	bool InitMap( std::wstring s );
	POINT pttopt(const RECT& rect,double x,double y);
	HBITMAP loadimage( std::string fname );
	~AreanaMap();
};

class AreanaLog{
public:
	std::wstring zone;
	std::wstring subZone;
	std::vector<UnitLog> logs;
	double timeLength;
	int areanaType;//3 or 5
	std::vector<std::wstring> friendRaces;
	std::vector<std::wstring> enemyRaces;
	AreanaMap map;
	double left;
	double right;
	double top;
	double bottom;
public:
	void drawTraceLine( CDC* pdc,const RECT& rect,std::wstring name,double t,COLORREF color );
	COLORREF getRaceColor( std::wstring s );
	void drawUnitState(CDC* pdc,const RECT& rect,UnitLog& stat,std::vector<std::wstring>& name,double t);
	std::wstring realName(std::wstring name);
	void clacType(UnitLog& ul);
	std::wstring cnRace(std::wstring r);
	UnitState loadState(std::wstring wstr,int n );
	UnitLog loadLog(std::wstring wstr,int n );
	bool loadZone(std::wstring wstr,int n );
	int getWordCount(std::wstring s );
	std::wstring getWord( std::wstring& s,int i );
	bool isUnitLog( std::wstring s );
	bool load( std::wstring fname );
	double getTimeLength();
	UnitLog gotoLog( double t );
};