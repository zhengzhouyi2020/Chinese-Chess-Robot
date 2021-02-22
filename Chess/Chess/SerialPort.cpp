#include "pch.h"
#include "SerialPort.h"

HANDLE  m_hComm;/** 串口句柄 */
bool s_bExit = false;/** 线程退出标志变量 */
volatile HANDLE m_hListenThread;/** 线程句柄 */
CRITICAL_SECTION m_csCommunicationSync;//!< 互斥操作串口/** 同步互斥,临界区保护 */
const UINT SLEEP_TIME_INTERVAL = 5;

bool InitPort(UINT portNo /*= 1*/, UINT baud /*= CBR_9600*/, char parity /*= 'N'*/,
	UINT databits /*= 8*/, UINT stopsbits /*= 1*/, DWORD dwCommEvents /*= EV_RXCHAR*/)
{

	/** 临时变量,将制定参数转化为字符串形式,以构造DCB结构 */
	char szDCBparam[50];
	sprintf_s(szDCBparam, "baud=%d parity=%c data=%d stop=%d", baud, parity, databits, stopsbits);

	/** 打开指定串口,该函数内部已经有临界区保护,上面请不要加保护 */
	if (!openPort(portNo))
	{
		return false;
	}

	/** 进入临界段 */
	EnterCriticalSection(&m_csCommunicationSync);

	/** 是否有错误发生 */
	BOOL bIsSuccess = TRUE;

	/** 在此可以设置输入输出的缓冲区大小,如果不设置,则系统会设置默认值.
	*  自己设置缓冲区大小时,要注意设置稍大一些,避免缓冲区溢出
	*/
	/*if (bIsSuccess )
	{
	bIsSuccess = SetupComm(m_hComm,10,10);
	}*/

	/** 设置串口的超时时间,均设为0,表示不使用超时限制 */
	COMMTIMEOUTS  CommTimeouts;
	CommTimeouts.ReadIntervalTimeout = 0;
	CommTimeouts.ReadTotalTimeoutMultiplier = 0;
	CommTimeouts.ReadTotalTimeoutConstant = 0;
	CommTimeouts.WriteTotalTimeoutMultiplier = 0;
	CommTimeouts.WriteTotalTimeoutConstant = 0;
	if (bIsSuccess)
	{
		bIsSuccess = SetCommTimeouts(m_hComm, &CommTimeouts);
	}

	DCB  dcb;
	if (bIsSuccess)
	{
		DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, szDCBparam, -1, NULL, 0);// 将ANSI字符串转换为UNICODE字符串
		wchar_t* pwText = new wchar_t[dwNum];
		if (!MultiByteToWideChar(CP_ACP, 0, szDCBparam, -1, pwText, dwNum))
		{
			bIsSuccess = TRUE;
		}

		bIsSuccess = GetCommState(m_hComm, &dcb) && BuildCommDCB((LPCWSTR)pwText, &dcb);/** 获取当前串口配置参数,并且构造串口DCB参数 */
		dcb.fRtsControl = RTS_CONTROL_ENABLE;/** 开启RTS flow控制 */
		delete[] pwText;/** 释放内存空间 */
	}

	if (bIsSuccess)
	{
		bIsSuccess = SetCommState(m_hComm, &dcb);/** 使用DCB参数配置串口状态 */
	}

	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);/**  清空串口缓冲区 */
	LeaveCriticalSection(&m_csCommunicationSync);/** 离开临界段 */

	return bIsSuccess == TRUE;
}

bool openPort(UINT portNo)
{
	EnterCriticalSection(&m_csCommunicationSync);//进入临界区

	char szPort[50];
	sprintf_s(szPort, "COM%d", portNo);//把串口号转换为设备名

	/** 打开指定的串口 */
	m_hComm = CreateFileA(szPort,	/** 设备名,COM1,COM2等 */
		GENERIC_READ | GENERIC_WRITE, /** 访问模式,可同时读写 */
		0,                            /** 共享模式,0表示不共享 */
		NULL,						/** 安全性设置,一般使用NULL */
		OPEN_EXISTING,				/** 该参数表示设备必须存在,否则创建失败 */
		0,
		0);

	if (m_hComm == INVALID_HANDLE_VALUE)/** 如果打开失败，释放资源并返回 */
	{
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}

	LeaveCriticalSection(&m_csCommunicationSync);/** 退出临界区 */

	return true;
}

void ClosePort()
{
	/** 如果有串口被打开，关闭它 */
	if (m_hComm != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE;
	}
}

//bool OpenListenThread()
//{
//	if (m_hListenThread != INVALID_HANDLE_VALUE)/** 检测线程是否已经开启了 */
//	{
//		/** 线程已经开启 */
//		return false;
//	}
//
//	s_bExit = false;
//	UINT threadId;/** 线程ID */
//	m_hListenThread = (HANDLE)_beginthreadex(NULL, 0, ListenThread, OpenListenThread, 0, &threadId);/** 开启串口数据监听线程 */
//	//m_hListenThread = (HANDLE)_beginthreadex(NULL, 0, ListenThread, this, 0, &threadId);/** 开启串口数据监听线程 */
//	if (!m_hListenThread)
//	{
//		return false;
//	}
//	
//	if (!SetThreadPriority(m_hListenThread, THREAD_PRIORITY_ABOVE_NORMAL))/** 设置线程的优先级,高于普通线程 */
//	{
//		return false;
//	}
//
//	return true;
//}

UINT GetBytesInCOM()
{
	DWORD dwError = 0;	/** 错误码 */
	COMSTAT  comstat;   /** COMSTAT结构体,记录通信设备的状态信息 */
	memset(&comstat, 0, sizeof(COMSTAT));

	UINT BytesInQue = 0;
	/** 在调用ReadFile和WriteFile之前,通过本函数清除以前遗留的错误标志 */
	if (ClearCommError(m_hComm, &dwError, &comstat))
	{
		BytesInQue = comstat.cbInQue; /** 获取在输入缓冲区中的字节数 */
	}

	return BytesInQue;
}

//UINT WINAPI ListenThread(void* pParam)
//{
//	/** 得到本类的指针 */
//	//CChessDlg *pSerialPort = reinterpret_cast<CChessDlg*>(pParam);
//
//	// 线程循环,轮询方式读取串口数据
//	//while (!pSerialPort->s_bExit)
//	while (!s_bExit)
//	{
//		//UINT BytesInQue = pSerialPort->GetBytesInCOM();
//		UINT BytesInQue = GetBytesInCOM();
//		/** 如果串口输入缓冲区中无数据,则休息一会再查询 */
//		if (BytesInQue == 0)
//		{
//			Sleep(SLEEP_TIME_INTERVAL);
//			continue;
//		}
//
//		/** 读取输入缓冲区中的数据并输出显示 */
//		char cRecved = 0x00;
//		do
//		{
//			cRecved = 0x00;
//			//if (pSerialPort->ReadChar(cRecved) == true)
//			if (ReadChar(cRecved) == true)
//			{
//				//printf("%c", cRecved);
//				//continue;
//				receiveMessage[messageIndex++] = cRecved;
//				if (messageIndex >= 4)
//				{
//					//std::cout << receiveMessage << std::endl;
//					printf("%s\n", receiveMessage);
//					messageIndex = 0;
//					sss.MoveChess(receiveMessage[0] - 48, receiveMessage[1] - 48, receiveMessage[2] - 48, receiveMessage[3] - 48);
//				}
//			}
//		} while (--BytesInQue);
//	}
//
//	return 0;
//}

bool ReadChar(char& cRecved)
{
	BOOL  bResult = TRUE;
	DWORD BytesRead = 0;
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	EnterCriticalSection(&m_csCommunicationSync);/** 临界区保护 */
	bResult = ReadFile(m_hComm, &cRecved, 1, &BytesRead, NULL);/** 从缓冲区读取一个字节的数据 */
	if ((!bResult))
	{
		/** 获取错误码,可以根据该错误码查出错误原因 */
		DWORD dwError = GetLastError();

		/** 清空串口缓冲区 */
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);

		return false;
	}

	LeaveCriticalSection(&m_csCommunicationSync);/** 离开临界区 */

	return (BytesRead == 1);

}

bool WriteData(unsigned char* pData, unsigned int length)
{
	BOOL   bResult = TRUE;
	DWORD  BytesToSend = 0;
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	EnterCriticalSection(&m_csCommunicationSync);/** 临界区保护 */

	/** 向缓冲区写入指定量的数据 */
	bResult = WriteFile(m_hComm, pData, length, &BytesToSend, NULL);
	if (!bResult)
	{
		DWORD dwError = GetLastError();
		/** 清空串口缓冲区 */
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);

		return false;
	}

	LeaveCriticalSection(&m_csCommunicationSync);/** 离开临界区 */

	return true;
}
