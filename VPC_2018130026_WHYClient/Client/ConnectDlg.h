#pragma once


// ConnectDlg ��ȭ �����Դϴ�.

class ConnectDlg : public CDialog
{
	DECLARE_DYNAMIC(ConnectDlg)

public:
	ConnectDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~ConnectDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CString m_strAddress;
	virtual BOOL OnInitDialog();
};
