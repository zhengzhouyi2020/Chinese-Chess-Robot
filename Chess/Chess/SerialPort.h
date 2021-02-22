#ifndef serialport_h_
#define serialport_h_

extern HANDLE  m_hComm;/** 串口句柄 */
extern bool s_bExit;/** 线程退出标志变量 */
extern volatile HANDLE m_hListenThread;/** 线程句柄 */
extern CRITICAL_SECTION m_csCommunicationSync;//!< 互斥操作串口/** 同步互斥,临界区保护 */
extern const UINT SLEEP_TIME_INTERVAL;

bool openPort(UINT  portNo);
void ClosePort();
/*打开串口*/
//static UINT WINAPI ListenThread(void* pParam);/*串口监听线程*/
bool InitPort(UINT  portNo = 1, UINT  baud = CBR_115200, char  parity = 'N', UINT  databits = 8, UINT  stopsbits = 1, DWORD dwCommEvents = EV_RXCHAR);
//bool OpenListenThread();
bool WriteData(unsigned char* pData, unsigned int length);
UINT GetBytesInCOM();
bool ReadChar(char &cRecved);

#endif

