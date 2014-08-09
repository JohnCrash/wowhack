#pragma once

class Druid:public Player{
protected:
public:
	Druid( AI* ai );
	virtual ~Druid();
	virtual void Castspell();
	void CastBuff();
};