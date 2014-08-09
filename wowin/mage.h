#pragma once

class Mage:public Player{
protected:
public:
	Mage( AI* ai );
	virtual ~Mage();
	virtual void Castspell();
};