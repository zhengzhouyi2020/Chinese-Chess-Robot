#include <wtypes.h>
#ifndef define_h_
#define define_h_

//#define BLACKCHESS 1//黑方
#define REDCHESS   2//红方

//深度方式DepthSort
#define DS_DEFAULTSET 1//默认设置
#define DS_USERDEFINE 2//用户定义

#define GRILLEWIDTH  39//棋盘上每个格子的宽度
#define GRILLEHEIGHT 39//棋盘上每个格子的高度

//--------棋子--------
#define NOCHESS    0 //没有棋子

#define B_CHESS    1 //黑将暂时替代黑子
#define R_CHESS    8 //红帅暂时替代红子

#define B_KING	   1 //黑将
#define B_CAR	   2 //黑车
#define B_HORSE	   3 //黑马
#define B_CANON	   4 //黑炮
#define B_BISHOP   5 //黑士
#define B_ELEPHANT 6 //黑象
#define B_PAWN     7 //黑卒
#define B_BEGIN    B_KING //黑旗起始旗子号
#define B_END      B_PAWN //黑旗终止旗子号

#define R_KING	   8 //红帅
#define R_CAR      9 //红车
#define R_HORSE    10//红马
#define R_CANON    11//红炮
#define R_BISHOP   12//红士
#define R_ELEPHANT 13//红相
#define R_PAWN     14//红兵
#define R_BEGIN    R_KING //红棋起始旗子号
#define R_END      R_PAWN //红棋终止旗子号
//--------------------

#define IsBlack(x) (x>=B_BEGIN && x<=B_END)//判断某个棋子是不是黑色
#define IsRed(x)   (x>=R_BEGIN && x<=R_END)//判断某个棋子是不是红色

//判断两个棋子是不是同色
#define IsSameSide(x,y) ((IsBlack(x) && IsBlack(y)) || (IsRed(x) && IsRed(y)))

//棋子位置
typedef struct
{
	BYTE x;
	BYTE y;
}CHESSMANPOS;

//棋子走法
typedef struct
{
	short nChessID;  //表明是什么棋子
	CHESSMANPOS From;//起始位置
	CHESSMANPOS To;  //走到什么位置
	int Score;       //走法的分数
}CHESSMOVE;

//悔棋时需要的数据结构
typedef struct
{
	CHESSMOVE cmChessMove;
	short nChessID;//被吃掉的棋子
}UNDOMOVE;

#endif
