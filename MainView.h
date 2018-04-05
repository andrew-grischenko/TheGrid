#if !defined(AFX_MAINVIEW_H__E61A93E2_0DCB_11D3_AB2A_000021C44C49__INCLUDED_)
#define AFX_MAINVIEW_H__E61A93E2_0DCB_11D3_AB2A_000021C44C49__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MainView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMainView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CMainView : public CFormView
{
protected:
	CMainView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CMainView)

// Form Data
public:
	//{{AFX_DATA(CMainView)
	enum { IDD = IDD_MAINFORM };
	CSliderCtrl	m_sldFrame;
	CSliderCtrl	m_Slider1;
	CString	m_R;
	CString	m_FrameNo;
	CString	m_IFileName;
	CString	m_IFrames;
	CString	m_IFrameSize;
	//}}AFX_DATA

// Attributes
public:
// Operations
public:
	BOOL ColorNear(RGBQUAD *clsrc, COLORREF clmask);
	void CutColor(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CMainView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CMainView)
	afx_msg void OnBtnBrowse();
	afx_msg void OnButton1();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnChangeEFrameno();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINVIEW_H__E61A93E2_0DCB_11D3_AB2A_000021C44C49__INCLUDED_)
