
// ClientView.cpp : CClientView 클래스의 구현
//

#include "stdafx.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "Client.h"
#endif

#include "ClientDoc.h"
#include "ClientView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CClientView
#include "ConnectDlg.h"
#include "SettingDlg.h"

IMPLEMENT_DYNCREATE(CClientView, CView)

BEGIN_MESSAGE_MAP(CClientView, CView)
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_COMMAND(ID_32773, &CClientView::On32773)
	ON_COMMAND(ID_32772, &CClientView::OnSetting)
	ON_COMMAND(ID_32771, &CClientView::OnStart)
END_MESSAGE_MAP()

// CClientView 생성/소멸

CClientView::CClientView()
{
	// TODO: 여기에 생성 코드를 추가합니다.

}

CClientView::~CClientView()
{
}

BOOL CClientView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

// CClientView 그리기

void CClientView::OnDraw(CDC* /*pDC*/)
{
	CClientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.
}


// CClientView 인쇄

BOOL CClientView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CClientView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CClientView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}


// CClientView 진단

#ifdef _DEBUG
void CClientView::AssertValid() const
{
	CView::AssertValid();
}

void CClientView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CClientDoc* CClientView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CClientDoc)));
	return (CClientDoc*)m_pDocument;
}
#endif //_DEBUG


// CClientView 메시지 처리기


void CClientView::On32773() //Onconnect
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	ConnectDlg dlg;
	if (dlg.DoModal() == IDOK)
	{
		m_Socket.Create();
		m_Socket.Connect(dlg.m_strAddress, 10000);
	}
}


void CClientView::OnSetting()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	SettingDlg dlg;

	dlg.m_CellNum = 0;
	dlg.m_ANum = 0;
	dlg.m_BNum = 0;
	dlg.m_CNum = 0;
	dlg.m_ATime = 0;
	dlg.m_BTime = 0;
	dlg.m_CTime = 0;
	dlg.m_Start = 0;

	dlg.DoModal();
	SendSet Set;
	Set.id = 1;
	Set.size = sizeof(SET);
	Set.kdata.m_CellNum = dlg.m_CellNum;
	Set.kdata.m_Anum = dlg.m_ANum;
	Set.kdata.m_Bnum = dlg.m_BNum;
	Set.kdata.m_Cnum = dlg.m_CNum;
	Set.kdata.m_Atime = dlg.m_ATime;
	Set.kdata.m_Btime = dlg.m_BTime;
	Set.kdata.m_Ctime = dlg.m_CTime;
	Set.kdata.m_Start = dlg.m_Start;
	m_Socket.Send(&Set, sizeof(Set));
}

void CClientView::OnStart()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	SendStart S1;
	S1.id = 2;
	S1.size = sizeof(StartFlag);
	S1.s1.startflag = 2;
	m_Socket.Send(&S1, sizeof(S1));
}
