#if !defined(AFX_ALPHABETA_HH_H__150772CA_2A04_457E_8B7A_3B9DBB6DCB66__INCLUDED_)
#define AFX_ALPHABETA_HH_H__150772CA_2A04_457E_8B7A_3B9DBB6DCB66__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SearchEngine.h"
#include "HistoryHeuristic.h"

//带历史启发的Alpha-Beta搜索引擎
class CAlphabeta_HHEngine:public CSearchEngine,public CHistoryHeuristic  
{
public:
	CAlphabeta_HHEngine();
	virtual ~CAlphabeta_HHEngine();

public:
	int virtual SearchAGoodMove(BYTE position[10][9]);

protected:
	int AlphaBeta(int nDepth,int alpha,int beta);
};

#endif // !defined(AFX_ALPHABETA_HH_H__150772CA_2A04_457E_8B7A_3B9DBB6DCB66__INCLUDED_)
