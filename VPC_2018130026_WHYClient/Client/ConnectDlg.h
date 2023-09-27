#pragma once


// ConnectDlg 대화 상자입니다.

class ConnectDlg : public CDialog
{
	DECLARE_DYNAMIC(ConnectDlg)

public:
	ConnectDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~ConnectDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CString m_strAddress;
	virtual BOOL OnInitDialog();
};
