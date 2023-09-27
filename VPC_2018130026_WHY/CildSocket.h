#pragma once

#include<afxsock.h>
// CildSocket 명령 대상입니다.
struct SET {
	int m_CellNum;
	int m_Anum;
	int m_Bnum;
	int m_Cnum;
	int m_Atime;
	int m_Btime;
	int m_Ctime;
	int m_Start;
};
struct StartFlag {
	int startflag;
};
class CildSocket : public CSocket
{
public:
	static SET Set;
	static StartFlag S1;
	static int startflag;

	virtual void OnReceive(int nErrorCode);
};


