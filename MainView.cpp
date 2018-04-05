// MainView.cpp : implementation file
//


#include "stdafx.h"
#include "vfw.h"
#include "GridCap.h"
#include "GridCapDoc.h"
#include "MainView.h"
#include "Vectorize.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainView

IMPLEMENT_DYNCREATE(CMainView, CFormView)

CMainView::CMainView()
	: CFormView(CMainView::IDD)
{
	//{{AFX_DATA_INIT(CMainView)
	m_R = _T("");
	m_FrameNo = _T("");
	m_IFileName = _T("");
	m_IFrames = _T("");
	m_IFrameSize = _T("");
	//}}AFX_DATA_INIT
}

CMainView::~CMainView()
{
}

void CMainView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMainView)
	DDX_Control(pDX, IDC_FRAME_SLIDER, m_sldFrame);
	DDX_Control(pDX, IDC_SLIDER1, m_Slider1);
	DDX_Text(pDX, IDT_COR, m_R);
	DDX_Text(pDX, IDC_E_FRAMENO, m_FrameNo);
	DDX_Text(pDX, IDC_IT_FILENAME, m_IFileName);
	DDX_Text(pDX, IDC_IT_FRAMES, m_IFrames);
	DDX_Text(pDX, IDC_IT_SIZE, m_IFrameSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMainView, CFormView)
	//{{AFX_MSG_MAP(CMainView)
	ON_BN_CLICKED(IDC_BTN_BROWSE, OnBtnBrowse)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_E_FRAMENO, OnChangeEFrameno)
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainView diagnostics

#ifdef _DEBUG
void CMainView::AssertValid() const
{
	CFormView::AssertValid();
}

void CMainView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainView message handlers

void CMainView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CGridDoc *doc = (CGridDoc *)GetDocument();
	char buf[4];
	char buf2[255];
	CStatic *fr;

	itoa(doc->CutOffThereR,buf,10);
	m_R = buf;

	m_IFileName = doc->AVIName;
	itoa(doc->Frames,buf2,10);
	m_IFrames = buf2;
	sprintf(buf2,"%d X %d", doc->szFrameX, doc->szFrameY);
	m_IFrameSize = buf2;
	sprintf(buf2,"%d", doc->CurFrame);
	m_FrameNo = buf2;

	if(m_sldFrame.m_hWnd != NULL)
	{
		if(doc->Frames == 0)
			m_sldFrame.SetRange(0,1,FALSE);
		else
			m_sldFrame.SetRange(0,doc->Frames-1);
		m_sldFrame.SetPos(doc->CurFrame);
	};


	UpdateData(FALSE);


	if(doc->AVIName.IsEmpty())
	{
		fr = (CStatic *)GetDlgItem(IDC_CUTOFF_FRAME);
		fr->SetBitmap(doc->hBmp14);
		return;
	};

	doc->LoadFrame(doc->CurFrame);
	
	CutColor();

	FXVectorize(&(doc->cfx));

	CBitmap bmp;
	CClientDC dc(this);

	doc->hBmp14 = ::CreateDIBitmap(dc.GetSafeHdc(),
									&(doc->dibImage14src->bmiHeader), CBM_INIT,
									doc->dibImage14src->bmiColors,doc->dibImage14src,
									DIB_RGB_COLORS);
	fr = (CStatic *)GetDlgItem(IDC_CUTOFF_FRAME);
	fr->SetBitmap(doc->hBmp14);

	doc->hBmp12 = ::CreateDIBitmap(dc.GetSafeHdc(),
									&(doc->dibImage12src->bmiHeader), CBM_INIT,
									doc->dibImage12src->bmiColors,doc->dibImage12src,
									DIB_RGB_COLORS);

	Invalidate();	
}

void CMainView::OnDraw(CDC* pDC) 
{
	RECT r;


	CPen Gpen(PS_SOLID,1,RGB(0,255,0));
	CPen Rpen(PS_SOLID,2,RGB(255,0,0));
	CPen Bpen(PS_SOLID,1,RGB(0,0,255));
	CPen Ypen(PS_SOLID,3,RGB(255,255,0));
	CPen Wpen(PS_SOLID,2,RGB(255,255,255));
	CPen *pOldpen;

	CGridDoc *doc=(CGridDoc *)GetDocument();
	CFX &cfx = doc->cfx;
	CStatic *fr;
	int lnno;
    
	if((doc->AVIName).IsEmpty()) return;
	
	fr = (CStatic *)GetDlgItem(IDC_FRAME_12);
	fr->GetWindowRect(&r);

	CBitmap bmp;
	bmp.Attach(doc->hBmp12);
	pDC->DrawState(CPoint(0,0), CSize(FX_FRAME_SZX/2,FX_FRAME_SZY/2), &bmp, DST_BITMAP);
	bmp.Detach();
	

	pOldpen = pDC->SelectObject(&Gpen);


	// Отрисовка квадрата с цветом прорезки
	CBrush br(doc->CutOffColor);
	CRect rct;
	fr = (CStatic *)GetDlgItem(IDC_CUTCOLOR);
	fr->GetClientRect(&rct);
	CClientDC dcr(fr);
	dcr.FillRect(&rct,&br);



	pDC->SelectObject(&Bpen);

	for(lnno=0; lnno < FX_SCAN_LINES; lnno++)
	{
		pDC->MoveTo(FX_SCAN_MINX/2,FXLineToY(lnno)/2);
		pDC->LineTo(FX_SCAN_MAXX/2,FXLineToY(lnno)/2);
		pDC->MoveTo(FXLineToX(lnno)/2,FX_SCAN_MINY/2);
		pDC->LineTo(FXLineToX(lnno)/2,FX_SCAN_MAXY/2);
	};

	pDC->SelectObject(&Ypen);

	for(int dir=0; dir <=1 ; dir++)
	{
		for(int line=0; line < FX_SCAN_LINES; line++)
		{
			for(int spln=0; spln < doc->cfx.splashs[dir][line].count; spln++)
			{
				if(!dir)
				{
					pDC->MoveTo((doc->cfx.splashs[dir][line].splashs[spln].start)/2,
								  FXLineToY(line)/2);
					pDC->LineTo((doc->cfx.splashs[dir][line].splashs[spln].end)/2,
								  FXLineToY(line)/2);
				}
				else
				{
					pDC->MoveTo(FXLineToX(line)/2,
								(doc->cfx.splashs[dir][line].splashs[spln].start)/2);
					pDC->LineTo(FXLineToX(line)/2,
								(doc->cfx.splashs[dir][line].splashs[spln].end)/2);
				};
			};
		};
	};


//** Hypotesas drawing **********************************************

	pDC->SelectObject(&Rpen);

	FXLINE ln;

	char str2[10];


	for(int dir2 = 0; dir2 < 2; dir2++)
		for(int hyp_no = 0; hyp_no < doc->cfx.hyps[dir2].count; hyp_no++)
		{
			ln = doc->cfx.lines[dir2].lines[hyp_no];
//			if(ln.ack < FX_MIN_ACKS) continue;
			if(dir2) 
			// Horizontal hyps
			{
				pDC->MoveTo(0/2,floor((ln.c-FX_FRAME_SZX*tan(ln.a)/2)/2));
				pDC->LineTo(FX_FRAME_SZX/2,floor((ln.c+FX_FRAME_SZX*tan(ln.a)/2)/2));
				if(ln.no >0)
				{
					itoa(ln.no,str2,10); 
					pDC->TextOut(20,(ln.c-FX_FRAME_SZX*tan(ln.a)/2)/2,CString(str2));
				};

			}
			else
			// Vertical hyps
			{
				pDC->MoveTo((ln.c-(FX_FRAME_SZY/2)*tan(ln.a))/2,0/2);
				pDC->LineTo((ln.c+(FX_FRAME_SZY/2)*tan(ln.a))/2,FX_FRAME_SZY/2);
				if(ln.no >0)
				{
					itoa(ln.no,str2,10); 
					pDC->TextOut((ln.c-FX_FRAME_SZX*tan(ln.a)/2)/2,20,CString(str2));
				};

			};
		};

	pDC->SelectObject(&Wpen);

	pDC->MoveTo(FX_FRAME_SZY/2*cfx.tg_teta/2,0);
	pDC->LineTo(-FX_FRAME_SZY/2*cfx.tg_teta/2,FX_FRAME_SZY/2);

	pDC->MoveTo(0,-FX_FRAME_SZX*cfx.tg_r1/4);
	pDC->LineTo(FX_FRAME_SZX/2,FX_FRAME_SZX*cfx.tg_r1/4);

	pDC->MoveTo(0,-FX_FRAME_SZX*cfx.tg_r2/4+576/2);
	pDC->LineTo(FX_FRAME_SZX/2,FX_FRAME_SZX*cfx.tg_r2/4+576/2);

	pDC->MoveTo(0,-FX_FRAME_SZX*cfx.tg_r0/4+576/4);
	pDC->LineTo(FX_FRAME_SZX/2,FX_FRAME_SZX*cfx.tg_r0/4+576/4);

	pDC->SelectObject(pOldpen);
}

void CMainView::OnBtnBrowse() 
{
	CGridDoc *doc = (CGridDoc *)GetDocument();
	CFileDialog dlgAvi(	TRUE,
						NULL,
						NULL,
						OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
						"AVI Files (*.avi)|*.avi||");

	if(dlgAvi.DoModal()==IDOK) 
		doc->NewAvi(dlgAvi.GetPathName());
};


void CMainView::CutColor(void)
{
	CGridDoc *doc=(CGridDoc *)GetDocument();
	CFX &cfx = doc->cfx;
	int _off11;

	RGBQUAD *src, *dst, *p_src, *p_dst;
	BYTE fill_byte;

	int x,y;

	// Источник изображения 
	src = doc->dibImage11->bmiColors;	
	
	// Прорезанная картинка
	dst = doc->dibImage14src->bmiColors;

	for(y = 0; y < FX_FRAME_SZY; y++)
		for(x = 0; x < FX_FRAME_SZX; x++)
		{
			_off11  = x + FX_FRAME_SZX*y;
			p_dst = dst + x/4 + y/4*FX_FRAME_SZX/4;
			p_src = src + _off11;
	
			if(ColorNear(p_src, doc->CutOffColor))
				fill_byte = 0xff;
			else
				fill_byte = 0x00;

			p_dst->rgbRed	= fill_byte;
			p_dst->rgbGreen	= fill_byte;
			p_dst->rgbBlue	= fill_byte;

			cfx.BITS[_off11] = fill_byte;
		};
	
};

void CMainView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	CGridDoc *doc = (CGridDoc *)GetDocument();
	char buf[4];

	itoa(doc->CutOffThereR,buf,10);
	m_R = buf;

	m_Slider1.SetRange(0,100);
	m_Slider1.SetPos(doc->CutOffThereR);
	m_Slider1.SetTicFreq(10);
	m_sldFrame.SetRange(0,1);
	m_sldFrame.SetPos(doc->CurFrame);
}


void CMainView::OnButton1() 
{
	UpdateData(TRUE);
	CGridDoc *doc = (CGridDoc *)GetDocument();

	doc->UpdateAllViews(NULL);
}

void CMainView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CGridDoc *doc = (CGridDoc *)GetDocument();
	if(nSBCode == SB_ENDSCROLL)
	{
		UpdateData(TRUE);

		doc->CutOffThereR = m_Slider1.GetPos();
		doc->CurFrame     = m_sldFrame.GetPos();
		doc->UpdateAllViews(NULL);
	};
/*	if(nSBCode == SB_THUMBTRACK)
	{
		if(pScrollBar->m_hWnd == m_Slider1.m_hWnd)
			doc->CutOffThereR = nPos;
		else
		if(pScrollBar->m_hWnd == m_sldFrame.m_hWnd)
			doc->CurFrame     = nPos;
		UpdateData(FALSE);
	};
*/
	CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMainView::OnChangeEFrameno() 
{
	UpdateData(TRUE);
	CGridDoc *doc = (CGridDoc *)GetDocument();

	doc->CurFrame = atoi(m_FrameNo);

	doc->UpdateAllViews(NULL);
	
	// TODO: Add your control notification handler code here
	
}


BOOL CMainView::ColorNear(RGBQUAD *clsrc, COLORREF clmask)
{
	CGridDoc *doc = (CGridDoc *)GetDocument();

	double r = abs(GetRValue(clmask) - clsrc->rgbRed);
	double g = abs(GetGValue(clmask) - clsrc->rgbGreen);
	double b = abs(GetBValue(clmask) - clsrc->rgbBlue);

	if( (r+g+b)/2.55/3 < doc->CutOffThereR )
		return TRUE;
	else
		return FALSE;
}

void CMainView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CClientDC dc(this);
	CGridDoc *doc = (CGridDoc *)GetDocument();
	
//	doc->ColorTable.Add(dc.GetPixel(point));
	doc->CutOffColor = dc.GetPixel(point);
	
	doc->UpdateAllViews(NULL);
	CFormView::OnLButtonDblClk(nFlags, point);
}
