
// ChessDlg.h: 头文件
//

#pragma once
#include "Resource.h"
#include "Define.h"
#include "MoveGenerator.h"
#include "AlphaBetaEngine.h"
#include "PVS_Engine.h"
#include "IDAlphabetaEngine.h"
#include "Alphabeta_HH.h"
#include "Opencv.h"
#include  "CSerialPort.h"

#include"afxwin.h"
#include "afxcmn.h"
#include<stack>

using namespace std;
#define WM_UPDATE_MESSAGE (WM_USER+200)

typedef struct//移动棋子结构体
{
	BYTE nChessID;//棋子ID
	POINT ptMovePoint;//移动的坐标
}MOVECHESS;

// CChessDlg 对话框
class CChessDlg : public CDialogEx
{
	// 构造
public:
	CChessDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHESS_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持



// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg LRESULT OnUpdateMessage(WPARAM wParam, LPARAM lParam);//子线程用来调用程序进行updataData
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnReceiveStr(WPARAM ch, LPARAM port);//串口接收函数
	DECLARE_MESSAGE_MAP()


private:
	int IsGameOver(BYTE position[][9]);//判断游戏是否结束，玩家胜利返回-1

public:


	// 思考线程函数
	void Think();
	// 棋子移动函数
	void MoveChess(char xFrom, char yFrom, char xTo, char yTo);
	/*
	//线程开启函数
	bool OpenListenThread();
	//线程监听程序
	static UINT  ListenThread(void* pParam);
	//线程关闭函数
	bool CloseListenThread();
	*/

	//串口类
	CSerialPort  m_SerialPort;
	

protected:
	// 定义电脑思考线程句柄
	HANDLE m_hHandle;
	// 线程ID
	DWORD m_dwThreadID;

	// 走法生成器指针
	CMoveGenerator* m_pMG;
	// 估值函数指针
	CEveluation* m_pEvel;
	//搜索引擎指针
	CSearchEngine* m_pSE;

	CString Tmessage;
	CString Treceive;

	//用于显示棋盘的位图
	CBitmap m_BoardBmp;
	//保存电脑走起路线
	CHESSMOVE PCMovePath;

	//棋盘数据，用于显示棋盘
	BYTE m_byChessBoard[10][9];
	//备份棋盘数据数组，用于出错恢复
	BYTE m_byBackupChessBoard[10][9];
	//用于显示的棋盘
	BYTE m_byShowChessBoard[10][9];

	//用于绘制棋子的ImageList对象
	CImageList m_Chessman;		
	//用于绘制移动棋子外框的ImageList对象
	CImageList m_Box;		         


	//用于保存当前拖曳的棋子结构
	MOVECHESS m_MoveChess;
	//棋盘宽度
	int m_nBoardWidth;
	//棋盘长度
	int m_nBoardHeight;
	//用于保存被拖曳的棋子的位置
	POINT m_ptMoveChess;
	//用户棋子颜色
	int m_nUserChessColor;
	//电脑是否正在思考
	bool m_bIsThinking;
	//是否已经分出胜负
	bool m_bIsGameOver;
	//已下的回合数，悔棋算减
	int m_iBout;
	//最佳走法
	CHESSMOVE m_cmBestMove;
	//悔棋走法
	UNDOMOVE m_umUndoMove;
	//记录走法的栈，便于悔棋
	stack<UNDOMOVE>m_stackUndoMove;
	//记录已走棋的走法的栈，便于还原
	stack<UNDOMOVE>m_stackRedoMove;

public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	// 提示信息

	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedNewgame();
	afx_msg void OnBnClickedOpenserials();
	afx_msg void OnBnClickedCloseserials();
	afx_msg void OnBnClickedOpencamera();
	afx_msg void OnBnClickedSend();
	afx_msg void OnBnClickedSendclear();
	afx_msg void OnBnClickedReceiveclear();
	CString Tsend;
	afx_msg void OnBnClickedConfirm();
	
	void SplitCString(const CString& _cstr, CStringArray& _resultArray, const CString& _flag);
	CComboBox m_Engine;
	afx_msg void OnCbnSelchangeEngine();
	CSpinButtonCtrl m_Spin;
	CEdit m_Edit;
	afx_msg void OnDeltaposSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedReset();
};
