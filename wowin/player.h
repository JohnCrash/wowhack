#pragma once

class Player{
protected:
	AI* pai; //ai界面
	Unit* pself; //自己
	bool bCreateWay;
	bool bShowWay;
	int WayPointCount;
	bool bStop;
	bool bForward;
	float fShift;
	bool bShift;

	void PrintWayArea( Way* pway );
	void CmdListWP( std::string arg1,std::string arg2 );
public:
	Player( AI* ai );
	virtual ~Player();
	bool IsSelf(const char* name);
	virtual void Think();
	virtual void Movement();
	virtual void Castspell();
	virtual void ChatEvent(const char* sender,const char* msg,const char* channel );
	virtual void Command( std::string cmd );
	virtual void CreateWay();
	virtual void Go(bool moveOrstop,bool forwardOrbackward );
};
