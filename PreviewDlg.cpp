// PreviewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GridCap.h"
#include "PreviewDlg.h"
#include "vfw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPreviewDlg dialog


CPreviewDlg::CPreviewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPreviewDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPreviewDlg)
	m_Edit = _T("");
	//}}AFX_DATA_INIT

}


void CPreviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPreviewDlg)
	DDX_Text(pDX, IDC_EDIT, m_Edit);
	DDV_MaxChars(pDX, m_Edit, 4);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPreviewDlg, CDialog)
	//{{AFX_MSG_MAP(CPreviewDlg)
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPreviewDlg message handlers

void CPreviewDlg::OnCancel() 
{
	DestroyWindow();
}

void CPreviewDlg::OnPaint() 
{

}

void CPreviewDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{

}


