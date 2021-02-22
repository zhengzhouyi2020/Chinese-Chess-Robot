#include "pch.h"
#include "SearchEngine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CSearchEngine::CSearchEngine()
{
}

CSearchEngine::~CSearchEngine()
{
	delete m_pMG;//释放走法产生器
	delete m_pEval;//释放估值核心
}

BYTE CSearchEngine::MakeMove(CHESSMOVE* move)//根据某一走法产生走了之后的棋盘
{
	BYTE nChessID;

	nChessID = CurPosition[move->To.y][move->To.x];//取目标位置棋子的标号
    CurPosition[move->To.y][move->To.x] = CurPosition[move->From.y][move->From.x];//把棋子移动到目标位置
	CurPosition[move->From.y][move->From.x] = NOCHESS;//将起点位置清空
	
	return nChessID;//返回被吃掉的棋子（如果未吃子就是吃空子）
}

void CSearchEngine::UnMakeMove(CHESSMOVE* move, BYTE nChessID)//恢复为走过之前的棋盘
{
	CurPosition[move->From.y][move->From.x] = CurPosition[move->To.y][move->To.x];//将目标位置棋子拷回原位  	
	CurPosition[move->To.y][move->To.x] = nChessID;//将目标位置恢复为走之前的棋子
}

void CSearchEngine::RedoChessMove(BYTE position[][9],CHESSMOVE* move)//还原
{
    position[move->To.y][move->To.x] = position[move->From.y][move->From.x];//把棋子移动到目标位置
	position[move->From.y][move->From.x] = NOCHESS;//将起点位置清空
}

void CSearchEngine::UndoChessMove(BYTE position[][9],CHESSMOVE* move, BYTE nChessID)//悔棋
{
	position[move->From.y][move->From.x]=position[move->To.y][move->To.x];//将目标位置棋子拷回原位  	
	position[move->To.y][move->To.x]=nChessID;//将目标位置恢复为走之前的棋子
}

int CSearchEngine::IsGameOver(BYTE position[][9], int nDepth)//判断游戏是否结束
{
	int i,j;
	BOOL RedLive=FALSE,BlackLive=FALSE;

	for(i=7;i<10;i++)//检查红方九宫是否有帅
		for(j=3;j<6;j++)
		{
			if(position[i][j]==B_KING) BlackLive=TRUE;
			if(position[i][j]==R_KING) RedLive=TRUE;
		}

	for(i=0;i<3;i++)//检查黑方九宫是否有将
		for(j=3;j<6;j++)
		{
			if(position[i][j]==B_KING) BlackLive=TRUE;
			if(position[i][j]==R_KING) RedLive=TRUE;
		}

	i=(m_nMaxDepth-nDepth+1)%2;//取当前奇偶标志,奇数层为电脑方,偶数层为用户方
	
	if(!RedLive)//红方不在
		if(i)
			return 19990+nDepth; //奇数层返回极大值
		else
			return -19990-nDepth;//偶数层返回极小值

	if(!BlackLive)//黑方不在
		if(i)
			return -19990-nDepth;//奇数层返回极小值
		else
			return 19990+nDepth; //偶数层返回极大值
		
	return 0;//将帅都在，返回0
}
