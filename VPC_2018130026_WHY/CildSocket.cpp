// CildSocket.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "VPC_2018130026_WHY.h"
#include "CildSocket.h"


// CildSocket


// CildSocket ��� �Լ�

SET CildSocket::Set;
StartFlag CildSocket::S1;
int CildSocket::startflag;


void CildSocket::OnReceive(int nErrorCode)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
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
