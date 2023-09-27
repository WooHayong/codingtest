// ConnectDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Client.h"
#include "ConnectDlg.h"
#include "afxdialogex.h"


// ConnectDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(ConnectDlg, CDialog)

ConnectDlg::ConnectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG1, pParent)
{

}

ConnectDlg::~ConnectDlg()
{
}

void ConnectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ConnectDlg, CDialog)
	ON_BN_CLICKED(IDOK, &ConnectDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// ConnectDlg 메시지 처리기입니다.


void ConnectDlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItemText(IDC_IPADDRESS1, m_strAddress);
	CDialog::OnOK();
}


BOOL ConnectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	SetDlgItemText(IDC_IPADDRESS1, _T("127.0.0.1"));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}
