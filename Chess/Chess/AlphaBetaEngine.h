#if !defined(AFX_ALPHABETAENGINE_H__C39E0A9F_C8EA_42E7_A56F_B286548405F6__INCLUDED_)
#define AFX_ALPHABETAENGINE_H__C39E0A9F_C8EA_42E7_A56F_B286548405F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SearchEngine.h"

class CAlphaBetaEngine:public CSearchEngine//alpha-beta搜索引擎
{
public:
	CAlphaBetaEngine();
	virtual ~CAlphaBetaEngine();

public:
	int virtual SearchAGoodMove(BYTE position[10][9]);//供界面调用的接口，为当前局面产生一步好棋

protected:
	int AlphaBeta(int depth,int alpha,int beta);//alpha-beta的搜索函数
};

#endif // !defined(AFX_ALPHABETAENGINE_H__C39E0A9F_C8EA_42E7_A56F_B286548405F6__INCLUDED_)
