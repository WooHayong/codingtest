
// ClientView.cpp : CClientView Ŭ������ ����
//

#include "stdafx.h"
// SHARED_HANDLERS�� �̸� ����, ����� �׸� �� �˻� ���� ó���⸦ �����ϴ� ATL ������Ʈ���� ������ �� ������
// �ش� ������Ʈ�� ���� �ڵ带 �����ϵ��� �� �ݴϴ�.
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
	// ǥ�� �μ� ����Դϴ�.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_COMMAND(ID_32773, &CClientView::On32773)
	ON_COMMAND(ID_32772, &CClientView::OnSetting)
	ON_COMMAND(ID_32771, &CClientView::OnStart)
END_MESSAGE_MAP()

// CClientView ����/�Ҹ�

CClientView::CClientView()
{
	// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.

}

CClientView::~CClientView()
{
}

BOOL CClientView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs�� �����Ͽ� ���⿡��
	//  Window Ŭ���� �Ǵ� ��Ÿ���� �����մϴ�.

	return CView::PreCreateWindow(cs);
}

// CClientView �׸���

void CClientView::OnDraw(CDC* /*pDC*/)
{
	CClientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: ���⿡ ���� �����Ϳ� ���� �׸��� �ڵ带 �߰��մϴ�.
}


// CClientView �μ�

BOOL CClientView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// �⺻���� �غ�
	return DoPreparePrinting(pInfo);
}

void CClientView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: �μ��ϱ� ���� �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
}

void CClientView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: �μ� �� ���� �۾��� �߰��մϴ�.
}


// CClientView ����

#ifdef _DEBUG
void CClientView::AssertValid() const
{
	CView::AssertValid();
}

void CClientView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CClientDoc* CClientView::GetDocument() const // ����׵��� ���� ������ �ζ������� �����˴ϴ�.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CClientDoc)));
	return (CClientDoc*)m_pDocument;
}
#endif //_DEBUG


// CClientView �޽��� ó����


void CClientView::On32773() //Onconnect
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	ConnectDlg dlg;
	if (dlg.DoModal() == IDOK)
	{
		m_Socket.Create();
		m_Socket.Connect(dlg.m_strAddress, 10000);
	}
}


void CClientView::OnSetting()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
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
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	SendStart S1;
	S1.id = 2;
	S1.size = sizeof(StartFlag);
	S1.s1.startflag = 2;
	m_Socket.Send(&S1, sizeof(S1));
}
