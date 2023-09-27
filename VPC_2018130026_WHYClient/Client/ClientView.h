
// ClientView.h : CClientView Ŭ������ �������̽�
//

#pragma once

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
struct SendSet {
	int id;
	int size;
	SET kdata;
};

struct StartFlag {
	int startflag;
};

struct SendStart {
	int id;
	int size;
	StartFlag s1;
};



class CClientView : public CView
{
protected: // serialization������ ��������ϴ�.
	CClientView();
	DECLARE_DYNCREATE(CClientView)

// Ư���Դϴ�.
public:
	CClientDoc* GetDocument() const;

// �۾��Դϴ�.
public:

// �������Դϴ�.
public:
	virtual void OnDraw(CDC* pDC);  // �� �並 �׸��� ���� �����ǵǾ����ϴ�.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// �����Դϴ�.
public:
	virtual ~CClientView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ������ �޽��� �� �Լ�
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void On32773();
	CSocket m_Socket;
	afx_msg void OnSetting();
	afx_msg void OnStart();
};

#ifndef _DEBUG  // ClientView.cpp�� ����� ����
inline CClientDoc* CClientView::GetDocument() const
   { return reinterpret_cast<CClientDoc*>(m_pDocument); }
#endif

