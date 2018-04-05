// GridCapDoc.h : interface of the CGridDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRIDCAPDOC_H__4E2E1D6A_0C48_11D3_AB2A_000021C44C49__INCLUDED_)
#define AFX_GRIDCAPDOC_H__4E2E1D6A_0C48_11D3_AB2A_000021C44C49__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "vectorize.h"

class CGridDoc : public CDocument
{
protected: // create from serialization only
	CGridDoc();
	DECLARE_DYNCREATE(CGridDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:

	BOOL NewAvi(CString path);
	BITMAPINFO *dibImage12src;
	BITMAPINFO *dibImage14src;
	BITMAPINFO *dibImage11;
	BITMAPINFO *dibImage12;
	BITMAPINFO *dibImage14;
	CFX cfx;


	int szFrameX;
	int szFrameY;
	int Frames;
	int CurFrame;
	HBITMAP hBmp14;
	HBITMAP hBmp12;

	CString AVIName;
	int CutOffThereR;

	COLORREF CutOffColor;

	int LoadFrame(int FrameNo);
	virtual ~CGridDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:



	//{{AFX_MSG(CGridDoc)

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRIDCAPDOC_H__4E2E1D6A_0C48_11D3_AB2A_000021C44C49__INCLUDED_)
