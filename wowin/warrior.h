#pragma once

class Warrior:public Player{
protected:
	bool bInit;
	int iZhandaoZhiTai;//Õ½¶·×ËÌ¬
	int iKuangBaoZhiTai;//¿ñ±©×ËÌ¬
	int iGongJi; //¹¥»÷
public:
	Warrior( AI* ai );
	virtual ~Warrior();
	virtual void Think();
	void InitWarrior();
	void AutoAttack();
	int WatchCD( std::string skill );
};