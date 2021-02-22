#if !defined(AFX_SEARCHENGINE_H__7A7237B9_0908_45D8_B102_94E342B174A5__INCLUDED_)
#define AFX_SEARCHENGINE_H__7A7237B9_0908_45D8_B102_94E342B174A5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Eveluation.h"
#include "MoveGenerator.h"

class CSearchEngine //搜索引擎的基类
{
public:
	CSearchEngine(); //构造函数
	virtual ~CSearchEngine(); //析构函数

public:
	int virtual SearchAGoodMove(BYTE position[10][9])=0;    //走下一步，虚函数，在其他派生类中重写
	CHESSMOVE GetBestMove(){return m_cmBestMove;};			//得到最佳走法
	UNDOMOVE GetUndoMove(){return m_umUndoMove;};			//得到悔棋走法
	void SetSearchDepth(int nDepth){m_nSearchDepth = nDepth;};//设定最大搜索深度
	void SetEveluator(CEveluation* pEval){m_pEval = pEval;};  //设定估值引擎
	void SetMoveGenerator(CMoveGenerator* pMG){m_pMG = pMG;};//设定走法产生器
	void SetUserChessColor(int nUserChessColor){m_nUserChessColor = nUserChessColor;};//设定用户为黑方或红方
	//void SetProgress(CProgressCtrl* Progress){ m_Progress = Progress; }//设定进度条变量

	//在搜索引擎基类中只实现了以下5个方法*************************************************
	void UndoChessMove(BYTE position[10][9],CHESSMOVE* move,BYTE nChessID);//悔棋
	void RedoChessMove(BYTE position[10][9],CHESSMOVE* move);//还原

protected:
	int IsGameOver(BYTE position[10][9],int nDepth);//判断是否已分胜负
	BYTE MakeMove(CHESSMOVE* move);//根据某一走法产生走了之后的棋盘
	void UnMakeMove(CHESSMOVE* move,BYTE nChessID);//恢复为走过之前的棋盘
	//在搜索引擎基类中只实现了以上5个方法*************************************************

public:
	int m_nUserChessColor;//用户棋子颜色
//	CProgressCtrl* m_Progress;//进度条变量

protected:
	BYTE CurPosition[10][9];		//搜索时用于记录当前节点棋盘状态的数组,中间变量
	CHESSMOVE m_cmBestMove;			//记录最佳走法
	UNDOMOVE m_umUndoMove;          //记录悔棋走法
	CMoveGenerator* m_pMG;			//走法产生器
	CEveluation* m_pEval;			//估值核心
	int m_nSearchDepth;				//最大搜索深度
	int m_nMaxDepth;				//当前搜索的最大搜索深度
};

#endif // !defined(AFX_SEARCHENGINE_H__7A7237B9_0908_45D8_B102_94E342B174A5__INCLUDED_)
