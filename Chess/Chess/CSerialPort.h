#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__

#ifndef WM_COMM_MSG_BASE 
#define WM_COMM_MSG_BASE		WM_USER + 617		//!< 消息编号的基点  
#endif

#define WM_COMM_BREAK_DETECTED		WM_COMM_MSG_BASE + 1	// A break was detected on input.
#define WM_COMM_CTS_DETECTED		WM_COMM_MSG_BASE + 2	// The CTS (clear-to-send) signal changed state. 
#define WM_COMM_DSR_DETECTED		WM_COMM_MSG_BASE + 3	// The DSR (data-set-ready) signal changed state. 
#define WM_COMM_ERR_DETECTED		WM_COMM_MSG_BASE + 4	// A line-status error occurred. Line-status errors are CE_FRAME, CE_OVERRUN, and CE_RXPARITY. 
#define WM_COMM_RING_DETECTED		WM_COMM_MSG_BASE + 5	// A ring indicator was detected. 
#define WM_COMM_RLSD_DETECTED		WM_COMM_MSG_BASE + 6	// The RLSD (receive-line-signal-detect) signal changed state. 
#define WM_COMM_RXCHAR				WM_COMM_MSG_BASE + 7	// A character was received and placed in the input buffer. 
#define WM_COMM_RXFLAG_DETECTED		WM_COMM_MSG_BASE + 8	// The event character was received and placed in the input buffer.  
#define WM_COMM_TXEMPTY_DETECTED	WM_COMM_MSG_BASE + 9	// The last character in the output buffer was sent.  

#define MaxSerialPortNum 20   ///有效的串口总个数，不是串口的号 //add by itas109 2014-01-09
class CSerialPort
{
public:
	// contruction and destruction
	CSerialPort();
	virtual		~CSerialPort();

	// port initialisation											
	BOOL		InitPort(HWND pPortOwner, UINT portnr = 1, UINT baud = 115200,
		char parity = 'N', UINT databits = 8, UINT stopsbits = 1,
		DWORD dwCommEvents = EV_RXCHAR | EV_CTS, UINT nBufferSize = 512,

		DWORD ReadIntervalTimeout = 1000,
		DWORD ReadTotalTimeoutMultiplier = 1000,
		DWORD ReadTotalTimeoutConstant = 1000,
		DWORD WriteTotalTimeoutMultiplier = 1000,
		DWORD WriteTotalTimeoutConstant = 1000);

	// start/stop comm watching
	///控制串口监视线程
	BOOL		 StartMonitoring();//开始监听
	BOOL		 RestartMonitoring();//重新监听
	BOOL		 StopMonitoring();//停止监听

	DWORD		 GetWriteBufferSize();///获取写缓冲大小
	DWORD		 GetCommEvents();///获取事件
	DCB			 GetDCB();///获取DCB

///写数据到串口
	void		WriteToPort(char* string);
	void		WriteToPort(char* string, int n); // add by mrlong 2007-12-25
	void		WriteToPort(BYTE* Buffer, int n);// add by mrlong
	void		ClosePort();					 // add by mrlong 2007-12-2  
	BOOL		IsOpen();

	BOOL RecvData(LPTSTR lpszData, const int nSize);	  //串口接收函数 by mrlong 2008-2-15
	void QueryKey(HKEY hKey);///查询注册表的串口号，将值存于数组中
	void Hkey2ComboBox(CComboBox& m_PortNO);///将QueryKey查询到的串口号添加到CComboBox控件中

protected:
	// protected memberfunctions
	void		ProcessErrorMessage(char* ErrorText);///错误处理
	static DWORD WINAPI CommThread(LPVOID pParam);///线程函数
	static void	ReceiveChar(CSerialPort* port);
	static void	WriteChar(CSerialPort* port);

	// thread
	//CWinThread*			m_Thread;
	HANDLE			  m_Thread;
	BOOL                m_bIsSuspened;///thread监视线程是否挂起

	// synchronisation objects
	CRITICAL_SECTION	m_csCommunicationSync;///临界资源
	BOOL				m_bThreadAlive;///监视线程运行标志

	// handles
	HANDLE				m_hShutdownEvent;  //stop发生的事件
	HANDLE				m_hComm;		   // 串口句柄 
	HANDLE				m_hWriteEvent;	 // write

	// Event array. 
	// One element is used for each event. There are two event handles for each port.
	// A Write event and a receive character event which is located in the overlapped structure (m_ov.hEvent).
	// There is a general shutdown when the port is closed. 
	///事件数组，包括一个写事件，接收事件，关闭事件
	///一个元素用于一个事件。有两个事件线程处理端口。
	///写事件和接收字符事件位于overlapped结构体（m_ov.hEvent）中
	///当端口关闭时，有一个通用的关闭。
	HANDLE				m_hEventArray[3];

	// structures
	OVERLAPPED			m_ov;///异步I/O
	COMMTIMEOUTS		m_CommTimeouts;///超时设置
	DCB					m_dcb;///设备控制块

	// owner window
	//CWnd*				m_pOwner;
	HWND				m_pOwner;


	// misc
	UINT				m_nPortNr;		//?????
	char* m_szWriteBuffer;///写缓冲区
	DWORD				m_dwCommEvents;
	DWORD				m_nWriteBufferSize;///写缓冲大小

	int				 m_nWriteSize;//写入字节数 //add by mrlong 2007-12-25
};

#endif __SERIALPORT_H__