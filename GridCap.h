// GridCap.h : main header file for the GRIDCAP application
//

#if !defined(AFX_GRIDCAP_H__4E2E1D64_0C48_11D3_AB2A_000021C44C49__INCLUDED_)
#define AFX_GRIDCAP_H__4E2E1D64_0C48_11D3_AB2A_000021C44C49__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CGridApp:
// See GridCap.cpp for the implementation of this class
//

class CGridApp : public CWinApp
{
public:
	CGridApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CGridApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRIDCAP_H__4E2E1D64_0C48_11D3_AB2A_000021C44C49__INCLUDED_)
