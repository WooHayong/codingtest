// CildSocket.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "VPC_2018130026_WHY.h"
#include "CildSocket.h"


// CildSocket


// CildSocket 멤버 함수

SET CildSocket::Set;
StartFlag CildSocket::S1;
int CildSocket::startflag;


void CildSocket::OnReceive(int nErrorCode)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	int header[2];

	Receive(header, sizeof(int) * 2);

	if (header[0] == 1)
	{
		Receive(&Set, header[1]);
		startflag = 1;
	}
	else if (header[0] == 2)
	{
		Receive(&S1, header[1]);
		startflag = 2;
	}
	
	CSocket::OnReceive(nErrorCode);
}
