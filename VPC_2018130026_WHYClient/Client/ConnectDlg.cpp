// ConnectDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "Client.h"
#include "ConnectDlg.h"
#include "afxdialogex.h"


// ConnectDlg ��ȭ �����Դϴ�.

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


// ConnectDlg �޽��� ó�����Դϴ�.


void ConnectDlg::OnBnClickedOk()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	GetDlgItemText(IDC_IPADDRESS1, m_strAddress);
	CDialog::OnOK();
}


BOOL ConnectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	SetDlgItemText(IDC_IPADDRESS1, _T("127.0.0.1"));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}
