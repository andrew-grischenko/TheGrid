// GridCapDoc.cpp : implementation of the CGridDoc class
//

#include "stdafx.h"
#include "GridCap.h"

#include "GridCapDoc.h"
#include "vfw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridDoc

IMPLEMENT_DYNCREATE(CGridDoc, CDocument)

BEGIN_MESSAGE_MAP(CGridDoc, CDocument)
	//{{AFX_MSG_MAP(CGridDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridDoc construction/destruction

CGridDoc::CGridDoc()
{
}

CGridDoc::~CGridDoc()
{
}

BOOL CGridDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	::GlobalFree(dibImage12);
	::GlobalFree(dibImage14);
	AVIName = "";
	Frames = 0;
	szFrameX = FX_FRAME_SZX;
	szFrameY = FX_FRAME_SZY;
	CurFrame = 0;
	CutOffThereR = 12;
	CutOffColor = RGB(80,120,255);

	



	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	CBitmap bmp;
	bmp.LoadBitmap(IDB_BITMAP14_NO);
	hBmp14 = (HBITMAP)bmp;

	UpdateAllViews(NULL);

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CGridDoc serialization

void CGridDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << AVIName;
		ar << CutOffColor;
		ar << CutOffThereR;
	}
	else
	{
		ar << CutOffThereR;
		ar >> CutOffColor;
		ar >> AVIName;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CGridDoc diagnostics

#ifdef _DEBUG
void CGridDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CGridDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGridDoc commands

BOOL CGridDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	if(!AVIName.IsEmpty()) NewAvi(AVIName);
	return TRUE;
}

void CGridDoc::OnCloseDocument() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CDocument::OnCloseDocument();
}

BOOL CGridDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CDocument::OnSaveDocument(lpszPathName);
}

int CGridDoc::LoadFrame(int FrameNo)
{
	PAVIFILE avi;
	PAVISTREAM pavis;
	PGETFRAME pf;
	BITMAPINFO *theFrame;
	BITMAPINFOHEADER DIB;
	int sz;

	AVIFileOpen(&avi,LPCSTR(AVIName),OF_READ,NULL);

    AVIFileGetStream(avi,&pavis,streamtypeVIDEO,0);

	DIB.biSize			= sizeof(BITMAPINFOHEADER); 
	DIB.biPlanes		= 1;
	DIB.biBitCount		= 32;
	DIB.biCompression	= BI_RGB;
	DIB.biSizeImage		= 0;
	DIB.biXPelsPerMeter	= 3200;
	DIB.biYPelsPerMeter	= 3200;
	DIB.biClrUsed		= 0;
	DIB.biClrImportant	= 0;

    DIB.biWidth			= FX_FRAME_SZX; 
	DIB.biHeight		= FX_FRAME_SZY;
	
    pf = AVIStreamGetFrameOpen(pavis,&DIB); 
    theFrame = (BITMAPINFO *)AVIStreamGetFrame(pf,FrameNo);
	sz = sizeof(BITMAPINFOHEADER)+DIB.biWidth*DIB.biHeight*4;
	if(dibImage11	!= NULL)
		::GlobalFree(dibImage11);
	dibImage11   =(BITMAPINFO *)::GlobalAlloc(0,sz);

	CopyMemory(dibImage11,theFrame,sz);

	AVIStreamGetFrameClose(pf);

    DIB.biWidth			= FX_FRAME_SZX/2; 
	DIB.biHeight		= FX_FRAME_SZY/2;
 
    pf = AVIStreamGetFrameOpen(pavis,&DIB); 
    theFrame = (BITMAPINFO *)AVIStreamGetFrame(pf,FrameNo);
	sz = sizeof(BITMAPINFOHEADER)+DIB.biWidth*DIB.biHeight*4;
	if(dibImage12src!= NULL)
		::GlobalFree(dibImage12src);
	if(dibImage12	!= NULL)
		::GlobalFree(dibImage12);

	dibImage12src=(BITMAPINFO *)::GlobalAlloc(0,sz);
	dibImage12   =(BITMAPINFO *)::GlobalAlloc(0,sz);

	CopyMemory(dibImage12src,theFrame,sz);
	CopyMemory(dibImage12   ,theFrame,sizeof(BITMAPINFOHEADER));

	AVIStreamGetFrameClose(pf);

    DIB.biWidth			= FX_FRAME_SZX/4; 
	DIB.biHeight		= FX_FRAME_SZY/4;
 
    pf = AVIStreamGetFrameOpen(pavis,&DIB); 
    theFrame = (BITMAPINFO *)AVIStreamGetFrame(pf,FrameNo);
	sz = sizeof(BITMAPINFOHEADER)+DIB.biWidth*DIB.biHeight*4;
	if(dibImage14src != NULL)
		::GlobalFree(dibImage14src);
	if(dibImage14    != NULL)
		::GlobalFree(dibImage14);

	dibImage14src=(BITMAPINFO *)::GlobalAlloc(0,sz);
	dibImage14   =(BITMAPINFO *)::GlobalAlloc(0,sz);

	CopyMemory(dibImage14src,theFrame,sz);
	CopyMemory(dibImage14   ,theFrame,sizeof(BITMAPINFOHEADER));

	AVIStreamGetFrameClose(pf);

	AVIStreamRelease(pavis);

	AVIFileRelease(avi);

	return TRUE;
}


BOOL CGridDoc::NewAvi(CString path)
{
 
	PAVIFILE avi;
	AVIFILEINFO pfi;
	int res;

	AVIName = path; 

	res = AVIFileOpen(&avi,LPCSTR(AVIName),OF_READ,NULL);
	if(res)
	{
		AVIName = "";
		return FALSE;
	};
	AVIFileInfo(avi,&pfi,sizeof(pfi));
	szFrameX = pfi.dwWidth;
	szFrameY = pfi.dwHeight;
	CurFrame = 0;
	Frames = pfi.dwLength;
    AVIFileRelease(avi);

	LoadFrame(0);

	UpdateAllViews(NULL);
	return TRUE;
}
