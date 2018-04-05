#ifndef VECTRZH
#define VECTRZH

#include "stdafx.h"

#define		PI					3.1415926
#define		RAD					(PI/180)
#define		FX_DIRS				2
#define		FX_FRAME_SZX		768
#define		FX_FRAME_SZY		576
#define		FX_SCAN_LINES		20  
#define		FX_MAX_SPLASHS		100
#define		FX_SCAN_MINX		20
#define		FX_SCAN_MAXX		(FX_FRAME_SZX - FX_SCAN_MINX)
#define		FX_SCAN_MINY		20
#define		FX_SCAN_MAXY		(FX_FRAME_SZY - FX_SCAN_MINY)
#define		FX_SCAN_STEPX		((FX_SCAN_MAXX-FX_SCAN_MINX)/FX_SCAN_LINES)
#define		FX_SCAN_STEPY		((FX_SCAN_MAXY-FX_SCAN_MINY)/FX_SCAN_LINES)
#define		FX_MAX_HYP_COUNT	500
#define		FX_MAX_ACK_COUNT	100
#define		FX_MIN_ACKS			3
#define		FX_MIN_DELTA		2000
#define		FX_NEXTLINE_THERESHOLD (FX_SCAN_STEPX*2)
#define		FX_POINT_THERESHOLD	10
#define		FX_ALFA_THERESHOLD	0.14
#define		FX_C_THERESHOLD		10
#define		FX_ACK_STEPS		20  
#define		FX_MAX_LINES		100  
#define		FX_MAX_POINTS		300  
#define		FX_VERT_LINES		20
#define		FX_HORZ_LINES		14
#define		FX_BLUE_THERE		0.4
#define		FX_MAX_SPLASHW		60
#define		FX_MIN_SPLACK		3
#define		FX_MAX_NABEGS		(FX_MAX_LINES*FX_MAX_LINES)
#define		FX_NABEG_THERE		0.25		// 25%

#define		FX_MAX_COLORS		100		

// Константы трассировки линии
#define		FX_MIN_LINELEN		(FX_FRAME_SZX/6)
#define		FX_TRACE_A0			(-60*RAD)
#define		FX_TRACE_A1			(60*RAD)

#define		gr_a1				95.0
#define		gr_b1				63.0
#define		gr_c1				42.0
#define		gr_A1				278.0
#define		gr_B1				185.0
#define		gr_C1				123.0

#define		gr_a2				63.0
#define		gr_b2				42.0
#define		gr_c2				28.0
#define		gr_A2				185.0
#define		gr_B2				123.0
#define		gr_C2				82.0

#define		gr_W				5006.0
#define		gr_H				2382.0
#define		PtoMM				(gr_W/FX_FRAME_SZX)
#define		MMtoP				(FX_FRAME_SZX/gr_W)
#define		MMtoD				(1/25.4)

#define		FX_TG_FOVX			0.47
#define		FX_TG_FOVY			(3*FX_TG_FOVX/4)
#define		FX_C0				4000.0

#define		FX_TRACE_FAILED		-10


/*class FXSPLASH
{
private:
	int start;
	int end;
public:
	FXSplash(){start = -1; end = -1;};
	FXSplash(int st, int en){start = st; end = en;}
	int GetStart(){return start};
	int GetEnd(){return end};
	int GetMid(){return (start+end)/2};
};

class FXSPLASHS
{
private:
	int count;
	FXSPLASH splashs[FX_MAX_SPLASHS];
	int CheckBound(int no){ if(no>=count) return 0;
							else
							if(no<0) return 0;
							else
							return 1; }
public:
	FXSPLASHS(){count = 0};
	~FXSPLASHS(){while (count--) delete splashs[count] }
	Add(int st, int en){splashs[count++] = new FXSPLASH(st,en); };
	int GetStart(int no){	if(!CheckBound(no)) return -1; 
							return splashs[no].GetStart()};
	int GetEnd(int no){		if(!CheckBound(no)) return -1; 
							return splashs[no].GetEnd()};
	int GetMid(int no){		if(!CheckBound(no)) return -1; 
							return splashs[no].GetMid()};
};*/

class FXColorTable
{
	private:
		COLORREF colors[FX_MAX_COLORS];
		int count;
	public:
		FXColorTable(void);
		~FXColorTable(void);
		BOOL Add(COLORREF cl);
		BOOL Remove(COLORREF cl);
		BOOL Remove(int idx);
		int GetCount(void);
		COLORREF GetColor(int idx);
};
	

struct FXSPLASH
{
	int start;
	int end;
	BOOL acked;
};

struct FXSPLASHS
{
	int count;
	FXSPLASH splashs[FX_MAX_SPLASHS];
};

struct FXHYPOTESA
{
	double a;
	double c;
	int ack;
	int spl_ack;
	double acks_a[FX_MAX_ACK_COUNT];
	double acks_c[FX_MAX_ACK_COUNT];

};

struct FXHYPOTESAS
{
	int count;
	FXHYPOTESA hyps[FX_MAX_HYP_COUNT];
};

struct FXLINE
{
	double a;
	double c;
	int no;
};

struct FXLINES
{
	FXLINE lines[FX_MAX_LINES];
	int count;
};

struct FXPOINT
{
	double x;
	double y;
	int no;
};

struct FXPOINTS
{
	FXPOINT points[FX_MAX_POINTS];
	int count;
};

struct FXNABEG
{
	int count;
	double nabeg[FX_MAX_NABEGS];
};

struct CFX
{
	BYTE BITS[FX_FRAME_SZX*FX_FRAME_SZY];
	FXSPLASHS splashs[FX_DIRS][FX_SCAN_LINES];
	FXHYPOTESAS hyps[FX_DIRS];
	FXHYPOTESAS hyps2[FX_DIRS];
	FXLINES lines[FX_DIRS];
	FXNABEG nabegs[FX_DIRS];
	double tg_teta;
	double tg_r1;
	double tg_r2;
	double tg_r0;
};


void FXVectorize(CFX *cfx);
void FXScanLine(CFX *cfx, int lineno, int dir);
BYTE FXGetBit(BYTE *bits,int x, int y);
int FXGetX(int idx);
int FXGetY(int idx);
int FXCheckHypotesa(FXHYPOTESAS &hyps, double a, double c/*, int dist*/);
void FXAddHypotesa(FXHYPOTESAS &hyps, double a, double c, int spl_ack, int ackno);
BOOL FXCheckFalse(BYTE *BITS, int dir, int mid1, int mid2, int dist1, int dist2);
int FXLineToX(double lineno);
int FXLineToY(double lineno);
int FXXToLine(double x);
int FXYToLine(double y);
void FXSortLines(FXLINES &lines,int l, int u);
double FXLineGetX(FXLINE ln, int y);
int FXFindLine(int dir, double rel1, double rel2);
void FXSwap(FXLINE &l1, FXLINE &l2);
/*double FXFindMax(double arr[FX_CUTCELL_SZX][FX_CUTCELL_SZY], double limit);
double FXFindMin(double arr[FX_CUTCELL_SZX][FX_CUTCELL_SZY]);*/
void FXDrawLine(int dir, int ln00, int ln01, int ln02, FXSPLASH s00,FXSPLASH s01,FXSPLASH s02,FXLINE &tryline);
void FXFilterHypotesas(CFX *cfx);
void FXTraceRay(CFX *cfx, int dir, int line_no, int cent, FXHYPOTESA *hyp);
void FXScanRay(BYTE *BITS, int dir, int mid1, int ln, int ray, int &l_from, int &l_to);
int FXFindLine2(int dir, double rel1, double rel2);
void FXFindIntersect(FXLINE vline, FXLINE hline, CPoint *pt);
double FXGetDV(int line_no);
double FXGetDH(int line_no);
void FXRegressia(int dir, int N, int *dists, int *mids, double &a, double &c);
BOOL FXNewton(double *X, 
			  double tan_t, 
			  double tan_dr, 
			  double tan_r0 
			  );
BOOL FXNewton2(double *X, 
			  double tan_t, 
			  double tan_dr, 
			  double tan_r0 
			  );
void FXNewtonFunc(double *X, 
				  double *F, 
				  double tan_t, 
				  double tan_dr, 
				  double tan_r0);

#endif