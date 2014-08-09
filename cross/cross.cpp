// cross.cpp : 定义控制台应用程序的入口点。
//

#include "..\wowin\stdafx.h"
#include "..\wowin\waypoint.h"
#include <fstream>
extern Cross g_cross;
extern Way g_way;

int main(int argc, wchar_t* argv[])
{
	g_cross.Load();
	g_cross.ReCalc();
	g_cross.Save();

	int iFrom = 8;
	int iTo = 7;

	std::vector<int> vip;
	g_cross.WalkRegionNode( vip,iFrom,iTo );
	std::ofstream fs;
	fs.open("e:\\1.txt");
	//打印vRN
	{
		for( std::vector<RegionNode>::iterator i = g_cross.vRN.begin();i!=g_cross.vRN.end();i++ ){
			fs<<g_cross.vAR.at(i->iAreaIndex[0]).name<<"<---->"<<g_cross.vAR.at(i->iAreaIndex[1]).name<<std::endl;
		}
		fs<<"-----------------\n";
	}
	//打印vRL
	for( std::vector<RegionLink>::iterator k = g_cross.vRL.begin();k!=g_cross.vRL.end();k++ ){
		fs<<g_cross.vAR.at(k->iAreaIndex).name<<std::endl;
		int index = 0;
		for( std::vector<int>::iterator z = k->vLink.begin();z!=k->vLink.end();z++ ){
			fs<<"   "<<index<<g_cross.vAR.at(*z).name<<std::endl;
			index++;
		}
	}
	fs<<"vip.size()="<<vip.size()<<std::endl;
	for( std::vector<int>::iterator i = g_cross.vWalkLimit.begin();i!=g_cross.vWalkLimit.end();i++ ){
		fs<<*i<<","<<g_cross.vAR.at(*i).name<<std::endl;
	}
	fs.close();
	return 0;
}