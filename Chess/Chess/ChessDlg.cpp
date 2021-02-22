
// ChessDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "Chess.h"
#include "ChessDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace cv;

#define _COM   3 //串口号

string strSerials = "";
string strTemp = "";
char receiveMessage[10] = "\0";
char moveTo[30];       //象棋移动结束的位置
char moveFrom[30];     //象棋移动开始的地方
char abandonArea[30]="";  //象棋被丢弃的区域
char cameraDetect[10]="M13\n"; //这是摄像头观察棋子的位置
int messageIndex = 0;
int index = 0;  //对于下象棋的每一步，需要对接受的每一个ok进行计数
bool hasChess = false; //判断是否会吃掉对面的子，吃子为true，否则为false；
bool newBegin = false;
char Reset[5] ="G28\n";//机械重置数组
char pumpOpen[4] = "M5\n"; //气泵开启指令
char pumpClose[4] = "M7\n"; //气泵开启指令
char chessOpen[5] = "M11\n";//取棋子指令
char chessClose[5] = "M15\n";//落子指令
char eatChess[5] = "M14\n";//吃子命令
const int BoardXValue[10][9] = { 
	{47,37,27,16,4,-8,-21,-34,-48},
	{47,36,25,14,2,-11,-23,-36,-50},
	{45,35,25,13,1,-11,-24,-37,-50},
	{45,35,24,12,0,-12,-25,-38,-51},
	{45,35,24,12,0,-12,-25,-39,-51},
	{45,35,24,12,0,-12,-25,-39,-51},
	{45,35,24,12,0,-12,-25,-38,-51},
	{45,35,25,13,1,-11,-24,-37,-50},
	{47,36,25,14,2,-11,-23,-36,-50},
	{47,37,27,16,4,-8,-21,-34,-48}
	
}; //x值的存储
const int BoardYValue[10][9] = {
	{62,60,58,56,54,52,50,48,46},
	{48,47,45,44,42,41,40,38,36},
	{35,33,32,31,30,30,28,27,25},
	{20,19,19,18,17,16,16,16,15},
	{7,6,6,6,6,6,6,6,6},
	{-8,-7,-7,-7,-7,-7,-7,-7,-7},
	{-21,-20,-20,-19,-18,-17,-17,-17,-16},
	{-36,-34,-33,-32,-31,-31,-29,-28,-26},
	{-49,-48,-46,-45,-43,-42,-41,-39,-37},
	{-63,-61,-59,-57,-55,-53,-51,-49,-47}
};//y值的存储

const BYTE InitChessBoard[10][9] = //初始化棋盘
{
	{   B_CAR, B_HORSE, B_ELEPHANT, B_BISHOP,  B_KING, B_BISHOP, B_ELEPHANT, B_HORSE,   B_CAR },
	{ NOCHESS, NOCHESS,    NOCHESS,  NOCHESS, NOCHESS,  NOCHESS,    NOCHESS, NOCHESS, NOCHESS },
	{ NOCHESS, B_CANON,    NOCHESS,  NOCHESS, NOCHESS,  NOCHESS,    NOCHESS, B_CANON, NOCHESS },
	{  B_PAWN, NOCHESS,     B_PAWN,  NOCHESS,  B_PAWN,  NOCHESS,     B_PAWN, NOCHESS,  B_PAWN },
	{ NOCHESS, NOCHESS,    NOCHESS,  NOCHESS, NOCHESS,  NOCHESS,    NOCHESS, NOCHESS, NOCHESS },
	//楚河                       汉界//
	{ NOCHESS, NOCHESS,    NOCHESS,  NOCHESS, NOCHESS,  NOCHESS,    NOCHESS, NOCHESS, NOCHESS },
	{  R_PAWN, NOCHESS,     R_PAWN,  NOCHESS,  R_PAWN,  NOCHESS,     R_PAWN, NOCHESS,  R_PAWN },
	{ NOCHESS, R_CANON,    NOCHESS,  NOCHESS, NOCHESS,  NOCHESS,    NOCHESS, R_CANON, NOCHESS },
	{ NOCHESS, NOCHESS,    NOCHESS,  NOCHESS, NOCHESS,  NOCHESS,    NOCHESS, NOCHESS, NOCHESS },
	{   R_CAR, R_HORSE, R_ELEPHANT, R_BISHOP,  R_KING, R_BISHOP, R_ELEPHANT, R_HORSE,   R_CAR }
};

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CChessDlg 对话框



CChessDlg::CChessDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHESS_DIALOG, pParent)
	, Tmessage(_T(""))
	, Treceive(_T(""))
	, Tsend(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bIsGameOver = false; //未分出胜负
	m_nUserChessColor = REDCHESS; //默认用户执红色
	m_bIsThinking = false; //电脑未在思考
	m_iBout = 0; //已下的回合数清零
	
	m_pSE = new CAlphabeta_HHEngine;//创建默认搜索引擎
	m_pMG = new CMoveGenerator;//创建走法产生器
	m_pEvel = new CEveluation; //创建估值核心

}

void CChessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MESSAGE, Tmessage);
	DDX_Text(pDX, IDC_RECEIVEDATA, Treceive);
	DDX_Text(pDX, IDC_SENDDATA, Tsend);
	DDX_Control(pDX, IDC_ENGINE, m_Engine);
	DDX_Control(pDX, IDC_SPIN, m_Spin);
	DDX_Control(pDX, IDC_EDIT, m_Edit);
}

BEGIN_MESSAGE_MAP(CChessDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPENSERIALS, &CChessDlg::OnBnClickedOpenserials)
	ON_BN_CLICKED(IDC_CLOSESERIALS, &CChessDlg::OnBnClickedCloseserials)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_UPDATE_MESSAGE, &CChessDlg::OnUpdateMessage)
	ON_MESSAGE(WM_COMM_RXCHAR, OnReceiveStr)
	ON_BN_CLICKED(IDC_STOP, &CChessDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_NEWGAME, &CChessDlg::OnBnClickedNewgame)
	ON_BN_CLICKED(IDC_OPENCAMERA, &CChessDlg::OnBnClickedOpencamera)
	ON_BN_CLICKED(IDC_SEND, &CChessDlg::OnBnClickedSend)
	ON_BN_CLICKED(IDC_SENDCLEAR, &CChessDlg::OnBnClickedSendclear)
	ON_BN_CLICKED(IDC_RECEIVECLEAR, &CChessDlg::OnBnClickedReceiveclear)
	ON_BN_CLICKED(IDC_CONFIRM, &CChessDlg::OnBnClickedConfirm)
	ON_CBN_SELCHANGE(IDC_ENGINE, &CChessDlg::OnCbnSelchangeEngine)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN, &CChessDlg::OnDeltaposSpin)
	ON_BN_CLICKED(IDC_RESET, &CChessDlg::OnBnClickedReset)
END_MESSAGE_MAP()


// CChessDlg 消息处理程序

BOOL CChessDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	AllocConsole();
	freopen("CONOUT$", "w+t", stdout);
	printf("Open console window\n");

	m_Chessman.Create(IDB_CHESSMAN, 36, 14, RGB(0, 255, 0));//创建含有棋子图形的ImgList，用于绘制棋子
	m_Box.Create(IDB_BOX, 32, 1, RGB(255, 255, 255));//图片为IDB_BOX，宽度为32像素，数量1，将RGB指定颜色改为透明


	////设置OpenCV窗口
	//namedWindow("OpenCV", CV_WINDOW_NORMAL);
	//HWND hWnd = (HWND)cvGetWindowHandle("OpenCV");
	//HWND hParent = ::GetParent(hWnd);
	//::SetParent(hWnd, GetDlgItem(IDC_OPENCV)->m_hWnd);
	//::ShowWindow(hParent, SW_HIDE);

	//获取picture控件大小

	//下面这段代码取棋盘图形的宽，高
	BITMAP BitMap;
	m_BoardBmp.LoadBitmap(IDB_CHESSBOARD);
	m_BoardBmp.GetBitmap(&BitMap); //取BitMap 对象
	m_nBoardWidth = BitMap.bmWidth;  //棋盘宽度
	m_nBoardHeight = BitMap.bmHeight;//棋盘高度

	m_BoardBmp.DeleteObject();

	memcpy(m_byChessBoard, InitChessBoard, 90);//初始化棋盘
	memcpy(m_byShowChessBoard, InitChessBoard, 90);
	memcpy(m_byBackupChessBoard, InitChessBoard, 90);

	m_pSE->SetSearchDepth(3);//设定搜索层数 *************************************************************
	m_pSE->SetMoveGenerator(m_pMG);//给搜索引擎设定走法产生器
	m_pSE->SetEveluator(m_pEvel);//给搜索引擎设定估值核心
	m_pSE->SetUserChessColor(m_nUserChessColor);//设定用户为黑方或红方

	m_MoveChess.nChessID = NOCHESS;//将移动的棋子清空

	GetDlgItem(IDC_STOP)->EnableWindow(FALSE);//冻结停止按钮


	redChessNum = blackChessNum = 16;//红黑棋子数量设为16
	GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("象棋对局"));

	//设置下拉框中的元素
	m_Engine.AddString(_T("ALphaBeta Searching Engine"));
	m_Engine.AddString(_T("Iterative Deepening AlphaBeta Searching Engine"));
	m_Engine.AddString(_T("PVS Searching Engine"));
	m_Engine.SetCurSel(0);


	m_Spin.SetRange32(2, 5);//表示数值只能在2到5内变化
	m_Spin.SetBase(10);//设置进制数,只能是10进制和16进制
	m_Spin.SetPos(3);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CChessDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

LRESULT CChessDlg::OnUpdateMessage(WPARAM wParam, LPARAM lParam)
{
	UpdateData(false); 
	return 0;
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CChessDlg::OnPaint()
{
	CPaintDC dc(this);
	CDC MemDC;
	int i, j;
	POINT pt;
	CBitmap* pOldBmp;

	MemDC.CreateCompatibleDC(&dc);
	m_BoardBmp.LoadBitmap(IDB_CHESSBOARD);
	pOldBmp = MemDC.SelectObject(&m_BoardBmp);

	for (i = 0; i < 10; i++)//绘制棋盘上的棋子
	{
		for (j = 0; j < 9; j++)
		{
			if (m_byShowChessBoard[i][j] == NOCHESS)
				continue;

			pt.x = j * GRILLEHEIGHT + 14;
			pt.y = i * GRILLEWIDTH + 15;
			m_Chessman.Draw(&MemDC, m_byShowChessBoard[i][j] - 1, pt, ILD_TRANSPARENT);
		}
	}

	//绘制电脑走棋外框
	pt.x = PCMovePath.From.x * GRILLEHEIGHT + 14 + 2;
	pt.y = PCMovePath.From.y * GRILLEWIDTH + 15 + 2;
	m_Box.Draw(&MemDC, 0, pt, ILD_TRANSPARENT);//绘制起点坐标框
	pt.x = PCMovePath.To.x * GRILLEHEIGHT + 14 + 2;
	pt.y = PCMovePath.To.y * GRILLEWIDTH + 15 + 2;
	m_Box.Draw(&MemDC, 0, pt, ILD_TRANSPARENT);//绘制终点坐标框

	if (m_MoveChess.nChessID != NOCHESS)//绘制用户正在拖动的棋子
		m_Chessman.Draw(&MemDC, m_MoveChess.nChessID - 1, m_MoveChess.ptMovePoint, ILD_TRANSPARENT);
	dc.BitBlt(0, 0, m_nBoardWidth, m_nBoardHeight, &MemDC, 0, 0, SRCCOPY);
	//将绘制的内容刷新到屏幕
	MemDC.SelectObject(&pOldBmp);//恢复内存Dc的原位图	
	MemDC.DeleteDC();            //释放内存	 
	m_BoardBmp.DeleteObject();   //删除棋盘位图对象
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CChessDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CChessDlg::OnReceiveStr(WPARAM ch, LPARAM port)
{ 
 	if ((char)ch != '\r') {
		strSerials = strSerials + (char)ch;
	}
	if (ch == '\n') {
		this->Treceive = strSerials.c_str();
		this->PostMessage(WM_UPDATE_MESSAGE, NULL, NULL);
		if (newBegin&& strSerials == "ok\n") {
			index = 100;
			this->m_SerialPort.WriteToPort(cameraDetect);
			newBegin = false;
		}
		else {

			if (!hasChess&& strSerials == "ok\n") {
				if (index == 0) {
					this->m_SerialPort.WriteToPort(Reset);
					index++;
				}
				else if (index == 1) {

					std::sprintf(moveFrom, "G1 X%d Y%d Z20\n", BoardXValue[m_cmBestMove.From.y][m_cmBestMove.From.x], BoardYValue[m_cmBestMove.From.y][m_cmBestMove.From.x]);
					this->m_SerialPort.WriteToPort(moveFrom);
					index++;
				}
				else if (index == 2) {
					this->m_SerialPort.WriteToPort(chessOpen);
					index++;
				}
				else if (index == 3) {
					std::sprintf(moveTo, "G1 X%d Y%d Z20\n", BoardXValue[m_cmBestMove.To.y][m_cmBestMove.To.x], BoardYValue[m_cmBestMove.To.y][m_cmBestMove.To.x]);
					this->m_SerialPort.WriteToPort(moveTo);
					index++;
				}
				else if (index == 4) {
					this->m_SerialPort.WriteToPort(chessClose);
					index++;
				}
				else if (index == 5) {
					this->m_SerialPort.WriteToPort(Reset);
					index++;
				}
				else if (index == 6) {
					this->m_SerialPort.WriteToPort(cameraDetect);
					index++;
				}
				
			}
			else if (hasChess && strSerials == "ok\n"&&(!m_bIsGameOver)) {
				if (index == 0) {
					this->m_SerialPort.WriteToPort(Reset);
					index++;
				}
				else if (index == 1) {
					std::sprintf(moveTo, "G1 X%d Y%d Z20\n", BoardXValue[m_cmBestMove.To.y][m_cmBestMove.To.x], BoardYValue[m_cmBestMove.To.y][m_cmBestMove.To.x]);
					this->m_SerialPort.WriteToPort(moveTo);
					index++;
				}
				else if (index == 2) {
					this->m_SerialPort.WriteToPort(eatChess);
					index++;
				}
				else if (index == 3) {
					std::sprintf(moveFrom, "G1 X%d Y%d Z20\n", BoardXValue[m_cmBestMove.From.y][m_cmBestMove.From.x], BoardYValue[m_cmBestMove.From.y][m_cmBestMove.From.x]);
					this->m_SerialPort.WriteToPort(moveFrom);
					index++;
				}
				else if (index == 4) {
					this->m_SerialPort.WriteToPort(chessOpen);
					index++;
				}
				else if (index == 5) {
					std::sprintf(moveTo, "G1 X%d Y%d Z20\n", BoardXValue[m_cmBestMove.To.y][m_cmBestMove.To.x], BoardYValue[m_cmBestMove.To.y][m_cmBestMove.To.x]);
					this->m_SerialPort.WriteToPort(moveTo);
					index++;
				}
				else if (index == 6) {
					this->m_SerialPort.WriteToPort(chessClose);
					index++;
				}
				else if (index == 7) {
					this->m_SerialPort.WriteToPort(Reset);
					index++;
				}
				else if (index == 8) {
					this->m_SerialPort.WriteToPort(cameraDetect);
					index++;
				}
				
			}
		}
		strSerials.clear();
	}
 	return true;
}


//创建opencv线程函数↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
DWORD WINAPI CameraProc(LPVOID pParam)//opencv线程函数
{
	CChessDlg* pDlg = (CChessDlg*)pParam;
	opencv_test();
	return 0;
}
//创建opencv线程函数↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑


//电脑思考线程函数↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
DWORD WINAPI ThinkProc(LPVOID pParam)//电脑思考线程函数
{
	CChessDlg* pDlg = (CChessDlg*)pParam;
	pDlg->Think();
	return 0;
}


//游戏是否结束
int CChessDlg::IsGameOver(BYTE position[][9])
{
	int i, j;
	BOOL RedLive = FALSE, BlackLive = FALSE;

	for (i = 7; i < 10; i++) //检查红方九宫是否有帅
		for (j = 3; j < 6; j++)
		{
			if (position[i][j] == B_KING) BlackLive = TRUE;
			if (position[i][j] == R_KING) RedLive = TRUE;
		}

	for (i = 0; i < 3; i++) //检查黑方九宫是否有将
		for (j = 3; j < 6; j++)
		{
			if (position[i][j] == B_KING) BlackLive = TRUE;
			if (position[i][j] == R_KING) RedLive = TRUE;
		}

	if (!RedLive) return 1; //玩家失败返回1
	if (!BlackLive) return -1; //玩家胜利返回-1

	return 0; //未分出胜负返回0
}




// 思考线程函数
void CChessDlg::Think()
{
	// TODO: 在此处添加实现代码.
	int timecount;
	CString sNodeCount;

	timecount = GetTickCount(); //取初始时间
	GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("正在思考中"));

	m_bIsThinking = true; //电脑正在思考

	m_pSE->SearchAGoodMove(m_byChessBoard);
	
	m_cmBestMove = m_pSE->GetBestMove(); //得到最佳走法

	PCMovePath.From.x = m_cmBestMove.From.x;
	PCMovePath.From.y = m_cmBestMove.From.y;
	PCMovePath.To.x = m_cmBestMove.To.x;
	PCMovePath.To.y = m_cmBestMove.To.y;

	//输出电脑走棋信息↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
	hasChess = IsRed(m_byShowChessBoard[m_cmBestMove.To.y][m_cmBestMove.To.x]);
	//if(hasChess){
	//	//此处用来判别原来是否有棋子，否则先发送数据
	//	chessMovePlus(m_cmBestMove.From.x, m_cmBestMove.From.y, m_cmBestMove.To.x, m_cmBestMove.To.y);
	//}
	//串口发送下棋数据

	//chessMove(m_cmBestMove.From.x, m_cmBestMove.From.y, m_cmBestMove.To.x, m_cmBestMove.To.y);
	
	printf("电脑：(%d,%d)->(%d,%d)\n", m_cmBestMove.From.x, m_cmBestMove.From.y, m_cmBestMove.To.x, m_cmBestMove.To.y);
	//输出电脑走棋信息↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑
	memcpy(m_byShowChessBoard, m_byChessBoard, 90); //将棋盘复制到显示棋盘
	memcpy(m_byBackupChessBoard, m_byChessBoard, 90); //将棋盘赋值到备份棋盘
	m_umUndoMove = m_pSE->GetUndoMove(); //悔棋走法
	m_stackUndoMove.push(m_umUndoMove); //将悔棋走法压栈，便于悔棋
	//this->AddChessRecord(m_cmBestMove.From.x + 1, m_cmBestMove.From.y + 1, m_cmBestMove.To.x + 1, \
				//	m_cmBestMove.To.y + 1, m_nUserChessColor, m_cmBestMove.nChessID);//向走棋记录表添加信息
	m_bIsThinking = false; //电脑未在思考

	//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
	memcpy(chessBoardNew, m_byShowChessBoard, sizeof(m_byShowChessBoard));

	redChessNum = 0;
	for (int i = 0; i < 10; i++)//重新统计红黑棋子数量
	{
		for (int j = 0; j < 9; j++)
			if (IsRed(chessBoardNew[i][j])) redChessNum++;
	}
	//↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑

	InvalidateRect(NULL, FALSE);//重绘屏幕
	UpdateWindow();

	sNodeCount.Format(TEXT(R"(共访问%d个节点,花费时间%.3fs)"), m_pEvel->GetAccessCount(), (GetTickCount() - timecount) / 1000.0);
	GetDlgItem(IDC_MESSAGE)->SetWindowText(sNodeCount);


	GetDlgItem(IDC_STOP)->EnableWindow(FALSE);//冻结停止按钮

	m_pEvel->ClearAccessCount(); //重置访问节点数为0

	switch (IsGameOver(m_byChessBoard)) //判断游戏是否结束
	{
	case -1:
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("玩家胜利"));; break;

	case 1:
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("电脑胜利"));; break;

	default:return;
	}

	m_bIsGameOver = true; //已分出胜负
}


// 棋子移动函数
void CChessDlg::MoveChess(char xFrom, char yFrom, char xTo, char yTo)
{
	// TODO: 在此处添加实现代码.
	printf("玩家：(%d,%d)->(%d,%d)\n", xFrom, yFrom, xTo, yTo);

	BOOL bTurnSide = FALSE;//玩家是否走棋标志位
	//判断鼠标是否在棋盘内，并且点中了玩家棋子
	if (yFrom >= 0 && yFrom < 10 && xFrom >= 0 && xFrom < 9 && (m_nUserChessColor == \
		REDCHESS ? IsRed(m_byChessBoard[yFrom][xFrom]) : IsBlack(m_byChessBoard[yFrom][xFrom])))
	{
		memcpy(m_byBackupChessBoard, m_byChessBoard, 90);//备份棋盘

		//将当前棋子的信息装入，记录移动棋子的结构中
		m_ptMoveChess.x = xFrom;
		m_ptMoveChess.y = yFrom;
		m_MoveChess.nChessID = m_byChessBoard[yFrom][xFrom];

		//将该棋子原位置棋子去掉
		m_byChessBoard[yFrom][xFrom] = NOCHESS;
		m_byShowChessBoard[yFrom][xFrom] = NOCHESS;
	}
	else
	{
		if (yFrom >= 0 && yFrom < 10 && xFrom >= 0 && xFrom < 9 && (m_nUserChessColor != \
			REDCHESS ? IsRed(m_byChessBoard[yFrom][xFrom]) : IsBlack(m_byChessBoard[yFrom][xFrom])))
		{
			GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("这里我的棋子"));//起点错误，为玩家棋子
		}
		else
		{
			GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("这里没有棋子"));//起点错误，没有棋子
		}
	}

	/////////////////////////////////

	//判断是否有棋子移动，并且该棋子移动合法
	if (m_MoveChess.nChessID && m_pMG->IsValidMove(m_byBackupChessBoard, m_ptMoveChess.x, m_ptMoveChess.y, xTo, yTo, m_nUserChessColor))
	{
		//---------将用户走法压栈---------
		m_cmBestMove.From.x = (BYTE)m_ptMoveChess.x;
		m_cmBestMove.From.y = (BYTE)m_ptMoveChess.y;
		m_cmBestMove.To.x = xTo;
		m_cmBestMove.To.y = yTo;
		m_cmBestMove.nChessID = m_MoveChess.nChessID;
		m_umUndoMove.cmChessMove = m_cmBestMove;
		m_umUndoMove.nChessID = m_byChessBoard[yTo][xTo];
		m_stackUndoMove.push(m_umUndoMove);
		//--------------------------------



		if (m_nUserChessColor == REDCHESS) m_iBout++;//已下的回合数+1
		//this->AddChessRecord(m_ptMoveChess.x + 1, m_ptMoveChess.y + 1, x + 1, y + 1, m_nUserChessColor, m_MoveChess.nChessID);
		m_byChessBoard[yTo][xTo] = m_MoveChess.nChessID;
		m_byShowChessBoard[yTo][xTo] = m_MoveChess.nChessID;
		bTurnSide = TRUE;//玩家走了一步棋
	}
	else//否则恢复移动前的棋盘状态
	{
		memcpy(m_byShowChessBoard, m_byBackupChessBoard, 90);
		memcpy(m_byChessBoard, m_byBackupChessBoard, 90);
	}

	m_MoveChess.nChessID = NOCHESS;//将移动的棋子清空

	InvalidateRect(NULL, FALSE);//重绘屏幕
	//UpdateWindow();

	if (bTurnSide == TRUE)//当玩家走了一步合法的棋，电脑才开始思考
	{
		GetDlgItem(IDC_STOP)->EnableWindow(TRUE);//激活停止按钮
		m_hHandle = CreateThread(0, 0, ThinkProc, this, 0, &m_dwThreadID);//创建电脑思考线程
	}
	else
	{
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("走法有误，违反规则"));//终点错误，走法无效
	}
}

//线程开启函数


//线程监听程序




void CChessDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_bIsThinking) return;//电脑正在思考

	if (m_bIsGameOver)//如果已分胜负
	{
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("游戏结束"));
		return;
	}

	//将坐标换算成棋盘上的格子
	int y = (point.y - 14) / GRILLEHEIGHT;
	int x = (point.x - 15) / GRILLEWIDTH;

	//判断鼠标是否在棋盘内，并且点中了玩家棋子
	if (y >= 0 && y < 10 && x >= 0 && x < 9 && (m_nUserChessColor == REDCHESS ? IsRed(m_byChessBoard[y][x]) : IsBlack(m_byChessBoard[y][x])))
	{
		//m_ctrlProgress.SetPos(0);//设置进度条的当前位置

		memcpy(m_byBackupChessBoard, m_byChessBoard, 90);//备份棋盘

		//将当前棋子的信息装入，记录移动棋子的结构中
		m_ptMoveChess.x = x;
		m_ptMoveChess.y = y;
		m_MoveChess.nChessID = m_byChessBoard[y][x];

		//将该棋子原位置棋子去掉
		m_byChessBoard[y][x] = NOCHESS;
		m_byShowChessBoard[y][x] = NOCHESS;

		//让棋子中点坐标位于鼠标所在点
		point.x -= 18;
		point.y -= 18;
		m_MoveChess.ptMovePoint = point;
		//重绘屏幕
		InvalidateRect(NULL, FALSE);
		UpdateWindow();
		SetCapture();//独占鼠标焦点
	}
	else
		if (y >= 0 && y < 10 && x >= 0 && x < 9 && (m_nUserChessColor != REDCHESS ? IsRed(m_byChessBoard[y][x]) : IsBlack(m_byChessBoard[y][x])))
			GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("这是我的棋子"));
		else
			GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("这里没有棋子"));

	CDialogEx::OnLButtonDown(nFlags, point);
}


void CChessDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_bIsThinking) return;//电脑正在思考

	if (m_bIsGameOver)//如果已分胜负
	{
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("游戏结束"));
		return;
	}

	//将坐标换算成棋盘上的格子
	int y = (point.y - 14) / GRILLEHEIGHT;
	int x = (point.x - 15) / GRILLEWIDTH;

	//判断鼠标是否在棋盘内，并且点中了玩家棋子
	if (y >= 0 && y<10 && x >= 0 && x<9 && (m_nUserChessColor == REDCHESS ? IsRed(m_byChessBoard[y][x]) : IsBlack(m_byChessBoard[y][x])))
	{
		//m_ctrlProgress.SetPos(0);//设置进度条的当前位置

		memcpy(m_byBackupChessBoard, m_byChessBoard, 90);//备份棋盘

		//将当前棋子的信息装入，记录移动棋子的结构中
		m_ptMoveChess.x = x;
		m_ptMoveChess.y = y;
		m_MoveChess.nChessID = m_byChessBoard[y][x];

		//将该棋子原位置棋子去掉
		m_byChessBoard[y][x] = NOCHESS;
		m_byShowChessBoard[y][x] = NOCHESS;

		//让棋子中点坐标位于鼠标所在点
		point.x -= 18;
		point.y -= 18;
		m_MoveChess.ptMovePoint = point;
		//重绘屏幕
		InvalidateRect(NULL, FALSE);
		UpdateWindow();
		SetCapture();//独占鼠标焦点
	}
	else
		if (y >= 0 && y<10 && x >= 0 && x<9 && (m_nUserChessColor != REDCHESS ? IsRed(m_byChessBoard[y][x]) : IsBlack(m_byChessBoard[y][x])))
			GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("这是我的棋子"));
		else
			GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("这里没有棋子"));

	CDialogEx::OnLButtonUp(nFlags, point);
}


void CChessDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
 	if (m_bIsThinking) return;//如果电脑正在思考

	if (m_MoveChess.nChessID)
	{
		//防止将棋子拖出棋盘
		if (point.x < 15)//左边
			point.x = 15;
		if (point.y < 15)//上边
			point.y = 15;
		if (point.x > m_nBoardWidth - 15)//右边				
			point.x = m_nBoardWidth - 15;
		if (point.y > m_nBoardHeight - 15)//下边				
			point.y = m_nBoardHeight - 15;

		//让棋子中心位于鼠标所在处
		point.x -= 18;
		point.y -= 18;

		m_MoveChess.ptMovePoint = point;//保存移动棋子的坐标	

		InvalidateRect(NULL, FALSE);//刷新窗口
		UpdateWindow();//立即执行刷新
	}
	CDialogEx::OnMouseMove(nFlags, point);
}



void CChessDlg::OnBnClickedStop()
{
	// TODO: 在此添加控件通知处理程序代码
	TerminateThread(m_hHandle, 0);
	WaitForSingleObject(m_hHandle, INFINITE);
	CloseHandle(m_hHandle);

	GetDlgItem(IDC_STOP)->EnableWindow(FALSE);//冻结停止按钮

	m_bIsThinking = 0;

	GetDlgItem(IDC_RECEIVEDATA)->SetWindowText(_T("停止思考"));

	//撤销用户走法
	memcpy(m_byShowChessBoard, m_byBackupChessBoard, 90);
	memcpy(m_byChessBoard, m_byBackupChessBoard, 90);
	m_stackUndoMove.pop();
	//m_lstChessRecord.DeleteString(m_lstChessRecord.GetCount() - 1);//停止搜索删除记录表中玩家的一步走法
	m_iBout--;

	//重绘屏幕  
	InvalidateRect(NULL, FALSE);
	UpdateWindow();
}


void CChessDlg::OnBnClickedNewgame()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bIsThinking) return;//如果电脑正在思考,退出

	if (MessageBox(TEXT("你确定要重来一局吗?"), TEXT("提示"), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDNO) return;

	memcpy(m_byChessBoard, InitChessBoard, 90);//初始化棋盘
	memcpy(m_byShowChessBoard, InitChessBoard, 90);//初始化棋盘
	memcpy(m_byBackupChessBoard, InitChessBoard, 90);//初始化棋盘
	m_MoveChess.nChessID = NOCHESS;//清除移动棋子
	m_bIsGameOver = false;//游戏未结束

	//刷新屏幕
	InvalidateRect(NULL, FALSE);
	UpdateWindow();

	//清空栈
	while (!m_stackUndoMove.empty())
		m_stackUndoMove.pop();
	while (!m_stackRedoMove.empty())
		m_stackRedoMove.pop();

	redChessNum = blackChessNum = 16;//红黑棋子数量设为16

	GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("你好"));
	GetDlgItem(IDC_STOP)->EnableWindow(FALSE);//冻结停止按钮
}


void CChessDlg::OnBnClickedOpencamera()
{
	// TODO: 在此添加控件通知处理程序代码
	HANDLE m_hHandle2;              //定义电脑思考线程句柄
	DWORD m_dwThreadID2;			   //线程ID

	m_hHandle2 = CreateThread(0, 0, CameraProc, this, 0, &m_dwThreadID2);//创建电脑思考线程
}



//发送文本框内容
void CChessDlg::OnBnClickedSend()
{
	// TODO: 在此添加控件通知处理程序代码
	
	//获取文本框内容
	UpdateData(true);
	Tsend = Tsend+_T("\n");
	//int len = Tsend.GetLength(); //获取str的字符数  
	USES_CONVERSION;
	//函数T2A和W2A均支持ATL和MFC中的字符
	char* senddata=T2A(Tsend);
	int len= strlen(senddata);
	printf("%s", senddata);
	m_SerialPort.WriteToPort(senddata);

	//代码测试，通过在文本框中输入棋子的起始和终末位置点来测试程序
	CStringArray dest;
	CString division = _T(",");
	//Treceive = _T("ok");
	SplitCString(Tsend,dest,division);
	//MoveChess(_ttoi(dest.GetAt(0)), _ttoi(dest.GetAt(1)), _ttoi(dest.GetAt(2)), _ttoi(dest.GetAt(3)));
	//Tsend = "";
	UpdateData(false);

}


void CChessDlg::OnBnClickedSendclear()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_SENDDATA)->SetWindowText(_T(""));
}


void CChessDlg::OnBnClickedReceiveclear()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_RECEIVEDATA)->SetWindowText(_T(""));
	 
}

void CChessDlg::OnBnClickedOpenserials()
{
	// TODO: 在此添加控件通知处理程序代码


	if (!m_SerialPort.InitPort(this->m_hWnd,_COM))//打开串口
		printf("initPort fail !\n");
	else
		printf("initPort success !\n");

	if (!m_SerialPort.StartMonitoring())//打开监听线程
		printf("OpenListenThread fail !\n");
	else
		printf("OpenListenThread success !\n");
}


void CChessDlg::OnBnClickedCloseserials()
{
	// TODO: 在此添加控件通知处理程序代码
	m_SerialPort.ClosePort();

}

void CChessDlg::OnBnClickedConfirm()
{
	// TODO: 在此添加控件通知处理程序代码
	strSerials.clear();
	opencvRun = true;
	printf("OpenCV run\n");
	while (opencvRun);//等待opencv操作完成
	printf("SerialPort run complete\n");
	index = 0;
	if (!m_bIsGameOver) {
		m_SerialPort.WriteToPort(Reset);
	}
	MoveChess(userMovePath[0], userMovePath[1], userMovePath[2], userMovePath[3]);
}


//分割字符串
void CChessDlg::SplitCString(const CString& _cstr, CStringArray& _resultArray, const CString& _flag)
{
	CString strSrc(_cstr);

	CStringArray& strResult = _resultArray;
	CString strLeft = _T("");

	int nPos = strSrc.Find(_flag);
	while (0 <= nPos)
	{
		strLeft = strSrc.Left(nPos);
		if (!strLeft.IsEmpty())
		{
			strResult.Add(strLeft);
		}
		strSrc = strSrc.Right(strSrc.GetLength() - nPos - 1);
		nPos = strSrc.Find(_flag);
	}

	if (!strSrc.IsEmpty()) {
		strResult.Add(strSrc);
	}
}





void CChessDlg::OnCbnSelchangeEngine()
{
	// TODO: 在此添加控件通知处理程序代码
	int index = m_Engine.GetCurSel();
	switch (index)
	{
		if (m_pSE)
			delete m_pSE;//释放旧的搜索引擎
	case 0:
		m_pSE = new CAlphaBetaEngine;//创建默认搜索引擎
		break;
	case 1:
		delete m_pSE;
		m_pSE = new CIDAlphabetaEngine;//创建迭代搜索引擎
		break;
	case 2:
		delete m_pSE;
		m_pSE = new CPVS_Engine;//创建搜索引擎
		break;
	default:
		m_pSE = new CAlphabeta_HHEngine;
		break;
	}
	m_pMG = new CMoveGenerator;//创建走法产生器
	m_pEvel = new CEveluation; //创建估值核心

	m_pSE->SetSearchDepth(m_Spin.GetPos());//设定搜索层数 *************************************************************
	m_pSE->SetMoveGenerator(m_pMG);//给搜索引擎设定走法产生器
	m_pSE->SetEveluator(m_pEvel);//给搜索引擎设定估值核心
	m_pSE->SetUserChessColor(m_nUserChessColor);//设定用户为黑方或红方
	m_pEvel->ClearAccessCount();
}


void CChessDlg::OnDeltaposSpin(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	m_pSE->SetSearchDepth(m_Spin.GetPos());
	*pResult = 0;
}



void CChessDlg::OnBnClickedReset()
{
	// TODO: 在此添加控件通知处理程序代码
	newBegin = true;
	m_SerialPort.WriteToPort(Reset);
	

}



