#if !defined(AFX_PVS_ENGINE_H__F62191D7_62C4_4BB8_A31B_DD3C7B67DA0C__INCLUDED_)
#define AFX_PVS_ENGINE_H__F62191D7_62C4_4BB8_A31B_DD3C7B67DA0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SearchEngine.h"

//极小窗口搜索引擎
class CPVS_Engine:public CSearchEngine
{
public:
	CPVS_Engine();
	virtual ~CPVS_Engine();

public:
	 int virtual SearchAGoodMove(BYTE position[10][9]);

protected:
	int PrincipalVariation(int depth,int alpha,int beta);
};

#endif // !defined(AFX_PVS_ENGINE_H__F62191D7_62C4_4BB8_A31B_DD3C7B67DA0C__INCLUDED_)
