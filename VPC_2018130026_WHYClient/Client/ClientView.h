
// ClientView.h : CClientView 클래스의 인터페이스
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
protected: // serialization에서만 만들어집니다.
	CClientView();
	DECLARE_DYNCREATE(CClientView)

// 특성입니다.
public:
	CClientDoc* GetDocument() const;

// 작업입니다.
public:

// 재정의입니다.
public:
	virtual void OnDraw(CDC* pDC);  // 이 뷰를 그리기 위해 재정의되었습니다.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 구현입니다.
public:
	virtual ~CClientView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void On32773();
	CSocket m_Socket;
	afx_msg void OnSetting();
	afx_msg void OnStart();
};

#ifndef _DEBUG  // ClientView.cpp의 디버그 버전
inline CClientDoc* CClientView::GetDocument() const
   { return reinterpret_cast<CClientDoc*>(m_pDocument); }
#endif

