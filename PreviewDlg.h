#if !defined(AFX_PREVIEWDLG_H__4E2E1D77_0C48_11D3_AB2A_000021C44C49__INCLUDED_)
#define AFX_PREVIEWDLG_H__4E2E1D77_0C48_11D3_AB2A_000021C44C49__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PreviewDlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CPreviewDlg dialog


class CPreviewDlg : public CDialog
{
// Construction
public:

	CButton	m_HideBtn;
	CSliderCtrl	m_Slider;
	CStatic	m_Frame;


	CPreviewDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPreviewDlg)
	enum { IDD = IDD_PREVIEW };
	CString	m_Edit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPreviewDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPreviewDlg)
	virtual void OnCancel();
	afx_msg void OnPaint();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREVIEWDLG_H__4E2E1D77_0C48_11D3_AB2A_000021C44C49__INCLUDED_)
