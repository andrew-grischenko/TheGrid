#include "stdafx.h"
#include <math.h>
#include "vectorize.h"

void FXVectorize(CFX *cfx)
{
	int line_no;
	int spl_no;

	FXSPLASHS line_splashs;
	FXHYPOTESA hyp;
	BOOL hyp_overflow;

	char* pszFileName = "c:\\myfile.txt";
	char bf[100];

	CStdioFile myFile;
	DWORD ltc;


	ltc = GetTickCount();
	myFile.Open( pszFileName, CFile::modeCreate |   
          CFile::modeReadWrite , NULL );


/// **** Первичная обработка изображения - сканирование и 
/// **** построение гипотез 

	// Обработка изображения начинается со сканирования по двум
	// направлениям - горизонтальное (dir==0) и вертикальному
	// (dir==1)
	for(int dir=0; dir < 2; dir++)
	{
		cfx->hyps2[dir].count = 0;

		//  Сканирование горизонтальной (dir==0) и
		//	вертикальной (dir==1) линии и нахождение всплесков
		//  с сохранением в массиве cfx->splashs
		for(line_no = 0; line_no < FX_SCAN_LINES; line_no++)
			FXScanLine(cfx, line_no, dir);

		// Построение гипотез по найденным вслескам версия 4
		// Устанавливается признак переполнения гипотез hyp_overflow
		hyp_overflow = FALSE;
		for(line_no = 0; line_no < (FX_SCAN_LINES - 1); line_no++)
		{
			if(hyp_overflow) 
				break;

			// line_splashs - всплески текущей линии сканорования
			line_splashs = cfx->splashs[dir][line_no];
			for(spl_no = 0; spl_no < line_splashs.count; spl_no++)
			{
				if(line_splashs.splashs[spl_no].acked)
					continue;

				// Трассировка лучей для определения преимущественного 
				// направления, принимаемого за направление линии
				FXTraceRay(	cfx, 
							dir, 
							line_no, 
							(line_splashs.splashs[spl_no].start + line_splashs.splashs[spl_no].end)/2, 
							&hyp);

				// Функция FXTraceRay возвращает -10 в случае если 
				// направление максимума на краю сектора сканирования
				// т.е. линия не удовлетворяет направлению
				if(hyp.a != FX_TRACE_FAILED)
//					FXAddHypotesa(cfx->hyps[dir],hyp.a,hyp.c,FXCheckHypotesa(cfx->hyps[dir],hyp.a,hyp.c));	
					FXAddHypotesa(cfx->hyps2[dir],hyp.a,hyp.c,0,-1);	

				// Возможен случай, когда количество гипотез 
				// слишком велико - отбрасываем 
				if(cfx->hyps2[dir].count == FX_MAX_HYP_COUNT )
				{
					hyp_overflow = TRUE;
					break;
				};
			};
		};
		
		// Уточнение гипотез по наименьшему отклонению от центров
		// всплесков
		int N;						// количество захваченных всплесков
		int dists[FX_SCAN_LINES];
		int  mids[FX_SCAN_LINES];
		double c,a;
		int hn, sln, splnn;
		FXHYPOTESA hyp;
		FXSPLASH spl;
		int midp;
		
		cfx->hyps[dir].count = 0;
		// Выполняется для каждой гипотезы
		for(hn = 0; hn < cfx->hyps2[dir].count; hn++)
		{
			N=0;
			hyp = cfx->hyps2[dir].hyps[hn];
			// Сначала происходит перебор линий сканирования...
			for(sln = 0; sln < FX_SCAN_LINES; sln++)
			{
				// ...и поиск в них захваченых всплесков
				for(splnn = 0; splnn < cfx->splashs[dir][sln].count; splnn++)
				{
					spl = cfx->splashs[dir][sln].splashs[splnn];
					if(dir)
						midp = hyp.c+(FXLineToX(sln)-FX_FRAME_SZX/2)*tan(hyp.a);
					else
						midp = hyp.c+(FXLineToY(sln)-FX_FRAME_SZY/2)*tan(hyp.a);

					if( (midp > spl.start) && (midp < spl.end) )
						// Всплеск захватывается линией
					{
						if(dir)
							dists[N] = FXLineToX(sln);
						else
							dists[N] = FXLineToY(sln);
						mids[N] = (spl.start + spl.end)/2;
						N++;
					};
				};
			};

			FXRegressia(dir, N, dists, mids, a, c);
			FXAddHypotesa(	cfx->hyps[dir],
							a,
							c,
							N,
							FXCheckHypotesa(cfx->hyps[dir],a,c));	

		};
	};


//**** Создание линий и сортировка их по коэффициенту С 
//****
	int hyp_no; // номер гипотезы/линии
	int ln_no;	// номер линии, вошедшей в список

	for(dir = 0; dir < 2; dir++)
	{
		// Преобразование гипотез в линии с отсевом 
		// мало-подтвердившихся гипотез
		ln_no = 0;
		for(hyp_no = 0; hyp_no < cfx->hyps[dir].count; hyp_no++)
		{
			if(cfx->hyps[dir].hyps[hyp_no].spl_ack > FX_MIN_SPLACK)
			{
				cfx->lines[dir].lines[ln_no].a = cfx->hyps[dir].hyps[hyp_no].a;
				cfx->lines[dir].lines[ln_no].c = cfx->hyps[dir].hyps[hyp_no].c;
				ln_no++;
			};
		};
		cfx->lines[dir].count = ln_no;

		// Непосредственно сортировка линий (QSort)
		FXSortLines(cfx->lines[dir],-1,-1);
	};

//**** Распознавание линий по их сочетанию ширины и промежутка
//**** между ними. При распознавании пренебрегаются перспективные 
//**** искажения как малые.

	int sl, spln,spl1n,spl2n;
	double w0,w1,w2,lx1,lx2;
	int res;
	FXLINE line1, line2;
	FXSPLASH spl;
	int acks;
	int ackarr[100];


	for(dir = 0; dir < 2; dir++)
	{
		// Поиск ведется по двум линиям и расстоянию между ними
		for(line_no = 0; line_no < cfx->lines[dir].count-1; line_no++)
		{
			line1 = cfx->lines[dir].lines[line_no + 0];
			line2 = cfx->lines[dir].lines[line_no + 1];

			// Переменная acks - подтверждения данного сочетания на других
			// линиях сканирования (вдоль направления линии)
			acks = 0;
			for(sl = 0; sl < FX_SCAN_LINES; sl++)
			{
				if(dir)
				{

					lx1 = // точка пересечения линии 1 с линией сканирования
						line1.c+(FXLineToX(sl)-FX_FRAME_SZX/2)*tan(line1.a);
					lx2 = // точка пересечения линии 2 с линией сканирования
						line2.c+(FXLineToX(sl)-FX_FRAME_SZX/2)*tan(line2.a);
				}
				else
				{
					lx1 = // точка пересечения линии 1 с линией сканирования
						line1.c+(FXLineToY(sl)-FX_FRAME_SZY/2)*tan(line1.a);
					lx2 = // точка пересечения линии 2 с линией сканирования
						line2.c+(FXLineToY(sl)-FX_FRAME_SZY/2)*tan(line2.a);
				};

				// Номера всплесков, подтверждающих данную линию
				// на текущем уровне сканирования
				spl1n=-1;	
				spl2n=-1;

				// Поиск подтверждающих всплесков
				for(spln=0; spln<cfx->splashs[dir][sl].count; spln++)
				{
					spl=cfx->splashs[dir][sl].splashs[spln];
					if((spl.start<lx1)&&(spl.end>lx1))
						spl1n=spln;
					else
					if((spl.start<lx2)&&(spl.end>lx2))
						spl2n=spln;

					// Экономия времени - случай когда оба
					// подтверждения найдены
					if((spl1n!=-1)&&(spl2n!=-1))
						break;
				};

				// Проверка на то что подтверждения обнаружены
				if((spl1n!=-1)&&(spl2n!=-1))
				{
					// W0 - ширина промежутка между линиями
					w0=(cfx->splashs[dir][sl].splashs[spl2n].start-cfx->splashs[dir][sl].splashs[spl1n].end);
					// W1 - ширина линии 1
					w1=(cfx->splashs[dir][sl].splashs[spl1n].end-cfx->splashs[dir][sl].splashs[spl1n].start);
					// W2 - ширина линии 2
					w2=(cfx->splashs[dir][sl].splashs[spl2n].end-cfx->splashs[dir][sl].splashs[spl2n].start);

					res=FXFindLine2(dir,w0/w1,w0/w2);
					if(res!=-1)
						// ackarr служит для отбора максимально 
						// подтвердившихся гипотез о номере линий
						ackarr[acks++] = res;
				};

				
			};


			int maxcnt = 0;
			int maxth  = -1;
			for(int th = 0; th < 20; th++)
			{
				int cnt=0;
				for(int ackn=0; ackn<acks; ackn++)
					if(ackarr[ackn] == th)
						cnt++;
				if(cnt>maxcnt)
				{
					maxcnt = cnt;
					maxth  = th;
				};
			};
			if(acks < 2)
			{
				cfx->lines[dir].lines[line_no + 0].no = -1;
			}
			else
			{
				cfx->lines[dir].lines[line_no + 0].no = maxth + 0;
				cfx->lines[dir].lines[line_no + 1].no = maxth + 1;
			};

		};
	};

// Фильтрация по набегу
	FXLINE line, next_line;
	double avg_nabeg;
	int nn, next_nn, next_line_no;
	int nabeg_acks[FX_MAX_NABEGS];
	int max_acks;
	int max_ack_nn;

	for(dir = 0; dir < 2; dir++)
	{
		cfx->nabegs[dir].count = 0;
		for(line_no = 0; line_no < cfx->lines[dir].count-1; line_no++)
		{
			for(next_line_no = line_no+1; next_line_no < cfx->lines[dir].count; next_line_no++)
			{
				line = cfx->lines[dir].lines[line_no];
				next_line = cfx->lines[dir].lines[next_line_no];
				cfx->nabegs[dir].nabeg[cfx->nabegs[dir].count] = (tan(next_line.a) - tan(line.a))/(next_line.c - line.c);
				cfx->nabegs[dir].count++;
			};
		};
		
		avg_nabeg = 0;
		for(nn = 0; nn < cfx->nabegs[dir].count; nn++)
			nabeg_acks[nn] = 0;

		for(nn = 0; nn < cfx->nabegs[dir].count; nn++)
			for(next_nn = 0; next_nn < cfx->nabegs[dir].count; next_nn++)
			{
				if(next_nn == nn)
					continue;
				if(fabs((cfx->nabegs[dir].nabeg[nn] - cfx->nabegs[dir].nabeg[next_nn])/cfx->nabegs[dir].nabeg[nn]) < FX_NABEG_THERE)
					nabeg_acks[nn]++;
			};

		max_acks = 0;
		max_ack_nn = -1;
		for(nn = 0; nn < cfx->nabegs[dir].count; nn++)
			if(nabeg_acks[nn] > max_acks)
			{
				max_acks = nabeg_acks[nn];
				max_ack_nn = nn;
			};
		cfx->nabegs[dir].nabeg[0] = cfx->nabegs[dir].nabeg[max_ack_nn];
	};



// Find Farthest

	int minv = 100000;
	int minh = 100000;
	int maxv = -100000;
	int maxh = -100000;
	int minvn = -1;
	int minhn = -1;
	int maxvn = -1;
	int maxhn = -1;
	double c;
	int n;

	for(line_no=0; line_no < cfx->lines[0].count; line_no++)
	{
		c = cfx->lines[0].lines[line_no].c;
		n = cfx->lines[0].lines[line_no].no;
		if(n < 0) 
			continue;

		if(c < minv)
		{
			minv  = c;
			minvn = line_no;
		};
		if(c > maxv)
		{
			maxv  = c;
			maxvn = line_no;
		};
	};

	for(line_no=0; line_no < cfx->lines[1].count; line_no++)
	{
		c = cfx->lines[1].lines[line_no].c;
		n = cfx->lines[1].lines[line_no].no;
		if(n < 0) 
			continue;

		if(c < minh)
		{
			minh  = c;
			minhn = line_no;
		};
		if(c > maxh)
		{
			maxh  = c;
			maxhn = line_no;
		};
	};

	CPoint pt1,pt2,pt3,pt4;
	FXLINE ln_left,ln_right,ln_top, ln_bottom;

	if( (minhn < 0) ||
		(minvn < 0) ||
		(maxhn < 0) ||
		(maxvn < 0)
		)
		return;

	ln_left  = cfx->lines[0].lines[minvn];
	ln_right = cfx->lines[0].lines[maxvn];
	ln_top   = cfx->lines[1].lines[minhn];
	ln_bottom= cfx->lines[1].lines[maxhn];


/*	FXFindIntersect(ln_left,ln_top,&pt1);
	FXFindIntersect(ln_right,ln_top,&pt2);
	FXFindIntersect(ln_left,ln_bottom,&pt3);
	FXFindIntersect(ln_right,ln_bottom,&pt4);

		ltc2 = GetTickCount();
		sprintf(bf,"Sorting&Other = %d\n",ltc2 - ltc);
	    myFile.WriteString(bf);
		ltc = GetTickCount();
*/
////////////////////////////////////////
	double tg_teta, tg_fi, fi, tg_r1, tg_r2, tg_ksi, ksi;
	double tg_dr, tg_r0;
	double dy,y1,z1, C0, mu;
	double dx, x1,vu, nabeg, FOVx_2;

	FOVx_2 = PI/6; // 30 градусов

//	nabeg	= (tan(ln_right.a) - tan(ln_left.a))/(ln_right.c - ln_left.c);
	nabeg = cfx->nabegs[0].nabeg[0];
	tg_teta	= -(tan(ln_left.a)-ln_left.c*nabeg);
	tg_fi	= tg_teta/tan(FOVx_2);
	fi = atan(tg_fi);

//	nabeg = (tan(ln_bottom.a) - tan(ln_top.a))/(ln_bottom.c - ln_top.c);
	nabeg = cfx->nabegs[1].nabeg[0];
	tg_r1 = tan(ln_top.a) - (ln_top.c)*nabeg;
	tg_r2 = tan(ln_bottom.a) + (FX_FRAME_SZY - ln_bottom.c)*nabeg;
	tg_dr = (tg_r2 - tg_r1);
	tg_r0 = (tg_r2 + tg_r1)/2;
	tg_ksi = tg_dr/(3*tan(FOVx_2)*cos(fi)/2);
	ksi = atan(tg_ksi);

	cfx->tg_teta = tg_teta;
	cfx->tg_r1 = tg_r1;
	cfx->tg_r2 = tg_r2;
	cfx->tg_r0 = tg_r0;



	double X[3];
	X[0] = atan(tg_fi);
	X[1] = atan(tg_ksi);
	X[2] = FOVx_2;

	FXNewton2( X, tg_teta, tg_dr, tg_r0);

	fi = X[0];
	ksi= X[1];
	FOVx_2 = X[2];

	C0 = -384/FOVx_2;
	

	int Y1, Y2, X1, X2;
	double z0;

	dy = FXGetDH(ln_top.no) - FXGetDH(ln_bottom.no);
	Y1 = FX_FRAME_SZY/2 - ln_top.c;
	Y2 = FX_FRAME_SZY/2 - ln_bottom.c;
	z1 = dy*(Y2*sin(fi) + C0*cos(fi))/(Y2-Y1);	

	y1 = Y1*(1-(z1 + C0)/C0);
	mu = FXGetDH(ln_top.no) - y1/cos(fi);

	z0 = z1 - y1*tan(fi);
	
	dx = FXGetDV(ln_left.no) - FXGetDV(ln_right.no);
	X1 = FX_FRAME_SZX/2 - ln_left.c;
	X2 = FX_FRAME_SZX/2 - ln_right.c;
	z1 = dx*(X2*sin(ksi) + C0*cos(ksi))/(X2-X1);	
	
	x1 = X1*(1-(z1 + C0)/C0);
	vu = FXGetDV(ln_left.no) - x1/cos(ksi);





	sprintf(bf,"%f\n", vu*MMtoD);
	myFile.WriteString(bf);
	sprintf(bf,"%f\n", mu*MMtoD);
	myFile.WriteString(bf);
	sprintf(bf,"%f\n",0.0);
	myFile.WriteString(bf);

	sprintf(bf,"%f\n",(vu+z0*cos(fi)*sin(ksi))*MMtoD);
	myFile.WriteString(bf);
	sprintf(bf,"%f\n",(mu-z0*sin(fi))*MMtoD);
	myFile.WriteString(bf);
	sprintf(bf,"%f\n",-z0*cos(fi)*cos(ksi)*MMtoD);
	myFile.WriteString(bf);
	sprintf(bf,"%f\n",-2*atan(384/C0)*180/PI);
	myFile.WriteString(bf);

	myFile.Close();
};

void FXScanLine(CFX *cfx, int lineno, int dir)
{
	// dir		- направление: 
	//				1 - вертикальное сканирование; 
	//				0 - горизонтальное;
	int dx,dy;
	int _start, _end, _middle;
	FXSPLASHS &splashs = cfx->splashs[dir][lineno];

	switch(dir)
	{
		case 0:	dx		= 1; 
				dy		= 0; 
				_start	= FX_SCAN_MINX; 
				_end	= FX_SCAN_MAXX; 
				_middle = lineno*FX_SCAN_STEPY+FX_SCAN_MINY; 
				break;
		case 1:	dx		= 0; 
				dy		= 1; 
				_start	= FX_SCAN_MINY; 
				_end	= FX_SCAN_MAXY; 
				_middle	= lineno*FX_SCAN_STEPX+FX_SCAN_MINX; 
				break;
	};

	BYTE lvl_curr;
	BYTE nxt_bit_0,nxt_bit_1,nxt_bit_2;
	int spl_start= 0;					// Координата начала скачка
	int spl_end	= 0;					// Координата конца скачка
	int x,y, coord;
	splashs.count = 0;
	
	lvl_curr = 0;	
	for(coord = _start; coord < _end; coord++)
	{
		x = dx*coord+_middle*dy;
		y = dy*coord+_middle*dx;

		nxt_bit_0=FXGetBit(cfx->BITS,x,y);
		// Проверка на наличие скачка
		if(nxt_bit_0 != lvl_curr)
		{
			// Проверить следующие две точки
			nxt_bit_1 = FXGetBit(cfx->BITS,x+dx,y+dy);
			nxt_bit_2 = FXGetBit(cfx->BITS,x+2*dx,y+2*dy);
			if( (nxt_bit_0 == nxt_bit_1) && (nxt_bit_0 == nxt_bit_2) )
				if(nxt_bit_0 == 0)
				{
					// Или dx==0 или dy==0 одновременно
					spl_end = x*dx+y*dy;
					if(splashs.count < FX_MAX_SPLASHS)
					{
						if(abs(spl_start - spl_end) < FX_MAX_SPLASHW)
						{
							splashs.splashs[splashs.count].start = spl_start;
							splashs.splashs[splashs.count].end = spl_end;
							splashs.splashs[splashs.count].acked = FALSE;
							splashs.count++;
						};
						lvl_curr = 0;
					};
				}
				else
				{
					spl_start = x*dx+y*dy;
					lvl_curr = 0xff;
				};
		};
	};
};

int FXCheckHypotesa(FXHYPOTESAS &hyps, double a, double c/*, int dist*/)
{
	FXHYPOTESA hyp;

	// Если проверяемая гипотеза - единственная, - добавить ее
	if(hyps.count == 0)
		return -1;

	// Проверка существующих гипотез
	for(int i=0; i < hyps.count; i++)
	{
		hyp = hyps.hyps[i];
		// Гипотеза должна "схлопываться" при dа < A0 и dc < C0
		if(	(fabs(hyp.a - a) < FX_ALFA_THERESHOLD) &&
			(fabs(hyp.c - c)  < FX_C_THERESHOLD) )
			return i;
	};

	// Здесь ничего похожего не было найдено...
	return -1;
}

void FXAddHypotesa(FXHYPOTESAS &hyps, double a, double c, int spl_ack, int ackno)
{
	if(ackno == -1)
	{
		FXHYPOTESA &hyp = hyps.hyps[hyps.count];
		hyp.ack = 0;
		hyp.a = a;
		hyp.c = c;
		hyp.spl_ack = spl_ack;
		hyp.acks_a[0] = hyp.a;
		hyp.acks_c[0] = hyp.c;
		hyps.count++;
	}
	else
	{
		FXHYPOTESA &hyp = hyps.hyps[ackno];
		hyp.ack++;
		hyp.acks_a[hyp.ack] = a;
		hyp.acks_c[hyp.ack] = c;
		hyp.a = 0;
		hyp.c = 0;
		hyp.spl_ack = spl_ack;
		for(int i=0; i <= hyp.ack; i++)
		{
			hyp.a += hyp.acks_a[i]/(hyp.ack+1);
			hyp.c += hyp.acks_c[i]/(hyp.ack+1);
		};
	};
};

BOOL FXCheckFalse(BYTE *BITS, int dir, int mid1, int mid2, int dist1, int dist2)
{
	int start_x, start_y;
	double step_x, step_y;
	int holes;

	if(!dir)
	{
		start_x = mid1;
		step_x	= (mid2 - mid1)/(double)FX_ACK_STEPS;

		start_y = dist1+FX_FRAME_SZY/2;
		step_y	= (dist2+FX_FRAME_SZY/2 - start_y)/(double)FX_ACK_STEPS;
	}
	else
	{
		start_y = mid1;
		step_y	= (mid2 - mid1)/(double)FX_ACK_STEPS;

		start_x = dist1+FX_FRAME_SZX/2;
		step_x	= (dist2+FX_FRAME_SZX/2 - start_x)/(double)FX_ACK_STEPS;
	};
	
	holes = 0;
	for(int i = 0; i< FX_ACK_STEPS; i++)
		if(FXGetBit(BITS, start_x+i*step_x,start_y+i*step_y) == 0x00) 
			holes++;

	return (holes > FX_ACK_STEPS/4);
};


// Процедура обмена местами двух линий, применяется
// для сортировки
void FXSwap(FXLINE &l1, FXLINE &l2)
{
	FXLINE tmp;

	tmp.a = l1.a;
	tmp.c = l1.c;
	l1.a  = l2.a;
	l1.c  = l2.c;
	l2.a  = tmp.a;
	l2.c  = tmp.c;
};

void FXSortLines(FXLINES &lines,int l, int u)
{
	int i,j,_l,_u;
	double mid;


	if(l == -1) 
		_l = 0;
	else
		_l = l;

	if(u == -1) 
		_u = lines.count-1;
	else
		_u = u;

	if(_l >= _u) return;
	if((_l - _u) == 1)
	{
		if(lines.lines[_l].c > lines.lines[_u].c)
			FXSwap(lines.lines[_l],lines.lines[_u]);
		return;	
	};

	mid = lines.lines[_u].c;
	do
	{
		i = _l;
		j = _u;

		while( (i < j) && (lines.lines[i].c <= mid))
			i++;
		while( (j > i) && (lines.lines[j].c >= mid))
			j--;
		
		if( i < j ) FXSwap(lines.lines[i],lines.lines[j]);

	} while(i<j);

	FXSwap(lines.lines[i],lines.lines[_u]);
	
	if((i-_l) < (_u - i))
	{
		FXSortLines(lines,_l,i-1);
		FXSortLines(lines,i+1,_u);
	}
	else
	{
		FXSortLines(lines,i+1,_u);
		FXSortLines(lines,_l,i-1);
	};

};

BYTE FXGetBit(BYTE *bits,int x, int y)
{
	return *(bits+x+(FX_FRAME_SZY-y)*FX_FRAME_SZX);
};

int FXGetX(int idx)
{
	return idx%FX_FRAME_SZX;
};

int FXGetY(int idx)
{
	return idx/FX_FRAME_SZX;
};

int FXLineToX(double lineno)
{
	return (FX_SCAN_MINX + lineno*FX_SCAN_STEPX);
}

int FXXToLine(double x)
{
	return (int)x / FX_SCAN_STEPX;
}

int FXYToLine(double y)
{
	return  (int)y / FX_SCAN_STEPY;
}

int FXLineToY(double lineno)
{
	return (FX_SCAN_MINY + lineno*FX_SCAN_STEPY);
}

/*double FXFindIntersectX(FXLINES &lines, int line_1, int line_2)
{
	return (lines[line_2].c - lines[line_1].c)/(lines[line_1].a - lines[line_2].a);
};*/



/*double FXFindMin(double arr[FX_CUTCELL_SZX][FX_CUTCELL_SZY])
{
	double min = 100000000;
	for(int x=0; x< FX_CUTCELL_SZX;x++)
		for(int y=0; y< FX_CUTCELL_SZY; y++)
			if(arr[x][y] < min)
				min = arr[x][y];
	return min;
};

double FXFindMax(double arr[FX_CUTCELL_SZX][FX_CUTCELL_SZY], double limit)
{
	double max = -100000000;
	for(int x=0; x< FX_CUTCELL_SZX;x++)
		for(int y=0; y< FX_CUTCELL_SZY; y++)
			if((arr[x][y] > max)&&(arr[x][y] < limit))
				max = arr[x][y];
	return max;
};

void FXFilterHypotesas(CFX *cfx)
{
	int dir, ln,hn,sn,an;
	FXHYPOTESA hyp;
	FXSPLASH spl;
	double hyp_x,hyp_d;
	BOOL rest;
	int first_ack, last_ack;

	for(dir = 0; dir < 2; dir++)
	{
		for(hn = 0; hn < cfx->hyps[dir].count; hn++)
		{
			hyp = cfx->hyps[dir].hyps[hn];
			hyp.ack = 0;
			first_ack = -1;
			last_ack=-1;
			for(ln = 0; ln < FX_SCAN_LINES; ln++)
			{
				if(!dir)
					hyp_x = hyp.c - (FXLineToY(ln) - FX_FRAME_SZY/2)*tan(hyp.a);
				else
					hyp_x = hyp.c - (FXLineToX(ln) - FX_FRAME_SZX/2)*tan(hyp.a);
				for(sn = 0; sn < cfx->splashs[dir][ln].count; sn++)
				{
					spl = cfx->splashs[dir][ln].splashs[sn];
					if((spl.start < hyp_x)&&(spl.end > hyp_x))
					{
						if(first_ack == -1)
							first_ack = ln;
						last_ack = ln;
						hyp_d = hyp_x - (spl.start+spl.end)/2;
						hyp.acks_d[hyp.ack] = hyp_d*hyp_d/2;
						hyp.ack++;
					};
				};
			};
			if(hyp.ack < FX_MIN_ACKS)
			{
				cfx->hyps[dir].hyps[hn].ack = -1;
				continue;
			};
			
			if(first_ack != -1)
			{

				if(!dir)
					rest = FXCheckFalse(	cfx->BITS, 
									dir, 
									hyp.c - (FXLineToY(first_ack) - FX_FRAME_SZY/2)*tan(hyp.a),
									hyp.c - (FXLineToY(last_ack) - FX_FRAME_SZY/2)*tan(hyp.a),
									FXLineToY(first_ack) - FX_FRAME_SZY/2,
									FXLineToY(last_ack)  - FX_FRAME_SZY/2);
				else
					rest = FXCheckFalse(	cfx->BITS, 
									dir, 
									hyp.c - (FXLineToX(first_ack) - FX_FRAME_SZX/2)*tan(hyp.a),
									hyp.c - (FXLineToX(last_ack) - FX_FRAME_SZX/2)*tan(hyp.a),
									FXLineToX(first_ack) - FX_FRAME_SZX/2,
									FXLineToX(last_ack)  - FX_FRAME_SZX/2);

				if(rest)
				{
					cfx->hyps[dir].hyps[hn].ack = -1;
					continue;
				};	
			};

			hyp.d = 0;
			for(an=0; an < hyp.ack; an++)
				hyp.d += hyp.acks_d[an];

			cfx->hyps[dir].hyps[hn].d = hyp_d/hyp.ack;
			if(hyp.d > FX_MIN_DELTA) 
			{
				cfx->hyps[dir].hyps[hn].ack = -1;
				continue;
			};

			
		cfx->hyps[dir].hyps[hn].ack = 1;	

		};



	};
};
*/

void FXTraceRay(CFX *cfx, int dir, int line_no, int cent, FXHYPOTESA *hyp)
{
	int rays[120];
	int rays_f[120];
	int rays_t[120];

	int ray, maxray, maxidx;
	int i;
	int l_from, l_to;
	int ln, l_start, l_end,sn;
	FXSPLASHS line_splashs;
	FXSPLASH spl;

	double mid;


	for(i = 0; i < 120; i++)
	{
		ray = i - 60;
		FXScanRay(cfx->BITS, dir, cent, line_no, ray, l_from, l_to);
		rays_f[i] = l_from;
		rays_t[i] = l_to;
	};

	maxray = 0;
	for(i = 0; i < 120; i++)
		if(rays_f[i]+rays_t[i] > maxray)
		{
			maxidx = i;
			l_from = rays_f[i];
			l_to   = rays_t[i];
			maxray = l_from+l_to;
		};

	if(	(maxidx != 0)  &&
		(maxidx != 119) &&
		(maxray > FX_MIN_LINELEN)
		)
	{
		hyp->a = (maxidx - 60)*PI/180L;
		if(!dir)
			hyp->c = cent - (FXLineToY(line_no) - FX_FRAME_SZY/2)*tan(hyp->a);
		else
			hyp->c = cent - (FXLineToX(line_no) - FX_FRAME_SZX/2)*tan(hyp->a);

		if(!dir)
		{
			l_start = FXYToLine(l_from);
			l_end = FXYToLine(l_to);
		}
		else
		{
			l_start = FXXToLine(l_from);
			l_end = FXXToLine(l_to);
		};
		
		for(ln = l_start; ln <= l_end; ln++)
		{
			for(sn = 0; sn < cfx->splashs[dir][ln].count; sn++)
			{
				spl = cfx->splashs[dir][ln].splashs[sn];
				if(!dir)
					mid = hyp->c + ( FXLineToY(ln) - FX_FRAME_SZY/2)*tan(hyp->a);
				else
					mid = hyp->c + ( FXLineToX(ln) - FX_FRAME_SZX/2)*tan(hyp->a);

				if((spl.start <= mid) && (spl.end >= mid))
					cfx->splashs[dir][ln].splashs[sn].acked = TRUE;
			};

		};
					
	}
	else
	{
		hyp->a = FX_TRACE_FAILED;
	};

};


void FXScanRay(BYTE *BITS, int dir, int mid1, int ln, int ray, int &l_from, int &l_to)
{
	int len1 = 0, len2 = 0;
	double step_x, step_y;
	double _x,_y, x, y;

	if(!dir)
	{
		_x = mid1;
		step_x	= tan(ray*PI/180.0);

		_y = FXLineToY(ln);
		step_y	= 1;
	}
	else
	{
		_y = mid1;
		step_y	= tan(ray*PI/180.0);

		_x = FXLineToX(ln);
		step_x	= 1;
	};

	
	x = _x;
	y = _y;
	do
	{
		len1++;
		x += step_x;
		y += step_y;
	}
	while(	(x < 768) && (x >= 0) &&
			(y < 576) && (y >= 0) &&
			(FXGetBit(BITS, x,y) != 0x00) );
	if(dir)
		l_to = x;
	else
		l_to = y;

	x = _x;
	y = _y;
	do
	{
		len2++;
		x -= step_x;
		y -= step_y;
	}
	while(	(x < 768) && (x >= 0) &&
			(y < 576) && (y >= 0) &&
			(FXGetBit(BITS, x,y) != 0x00) );

	if(dir)
		l_from = x;
	else
		l_from = y;

};


int FXFindLine2(int dir, double rel1, double rel2)
{
	double *arr;
	int end;

	static double vert[2*(FX_VERT_LINES-1)] = 
	{
		6.6,	6.6,
		3.0,	1.3,
		1.3,	3.0,
		6.6,	4.4,
		2.0,	2.0,
		3.0,	4.4,
		4.4,	3.0,
		2.0,	1.3,
		2.0,	4.4,
		6.6,	3.0,
		1.3,	2.0,
		4.4,	4.4,
		3.0,	3.0,
		3.0,	2.0,
		2.0,	3.0,
		4.4,	6.6,
		4.4,	2.0,
		1.3,	1.3,
		3.0,	6.6
	};


	static double horz[2*(FX_HORZ_LINES-1)] = 
	{
		4.4,	3.0,	
		1.3,	1.3,	
		3.0,	6.6,	
		4.4,	2.0,	
		2.0,	3.0,	
		3.0,	3.0,	
		4.4,	4.4,	
		2.0,	1.3,	
		1.3,	3.0,	
		6.6,	6.6,	
		3.0,	1.3,	
		2.0,	4.4,	
		6.6,	4.4
	};

	if( (rel1 > 10.0) ||
		(rel2 > 10.0) ||
		(rel1 < 1.0 ) ||
		(rel2 < 1.0 )
		)
		return -1;


	if(!dir)
	{
		arr = vert;
		end =  FX_VERT_LINES-1;
	}
	else
	{
		arr = horz;
		end =  FX_HORZ_LINES-1;
	};
		
	for(int i=0; i < end; i++)
		if( (rel1 <= arr[2*i  ]*1.25) &&
			(rel1 >= arr[2*i  ]/1.2) &&
			(rel2 <= arr[2*i+1]*1.25) &&
			(rel2 >= arr[2*i+1]/1.2)
			)
			return i;
	
	return -1;
};

void FXFindIntersect(FXLINE vline, FXLINE hline, CPoint *pt)
{

	double a1,a2,c1,c2,ddist, dmid,c10,c20;

	c1=vline.c;
	a1=vline.a;
	c2=hline.c;
	a2=hline.a;
	c10 = FX_FRAME_SZX/2;
	c20 = FX_FRAME_SZY/2;

	ddist = tan(a1)*(c2-c20+tan(a2)*(c1-c10))/(1-tan(a1)*tan(a2));
	dmid  = tan(a2)*(c1-c10+ddist);
	pt->y = c2+dmid;
	pt->x = c1+ddist;
};

double FXGetDV(int line_no)
{
	static double VGRID[20+19] = 
	{
		gr_c1,gr_A1,gr_c1,gr_C1,gr_a1,gr_C1,gr_c1,gr_A1,
		gr_b1,gr_C1,gr_b1,gr_B1,gr_c1,gr_B1,gr_b1,gr_C1,
		gr_a1,gr_B1,gr_c1,gr_A1,gr_a1,gr_C1,gr_b1,gr_A1,
		gr_b1,gr_B1,gr_b1,gr_B1,gr_a1,gr_B1,gr_b1,gr_A1,
		gr_c1,gr_B1,gr_a1,gr_C1,gr_a1,gr_A1,gr_c1
	};

	double x = 0;
	
	for(int ln = 0; ln < line_no; ln++)
		x += VGRID[2*ln] + VGRID[2*ln+1];

	x += VGRID[line_no]/2;
	
	return  x - gr_W/2;
};

double FXGetDH(int line_no)
{
	static double HGRID[14+13] = 
	{
		gr_b2,gr_A2,gr_a2,gr_C2,
		gr_a2,gr_A2,gr_c2,gr_B2,
		gr_a2,gr_B2,gr_b2,gr_B2,
		gr_b2,gr_A2,gr_b2,gr_C2,
		gr_a2,gr_C2,gr_c2,gr_A2,
		gr_c2,gr_C2,gr_a2,gr_B2,
		gr_c2,gr_A2,gr_b2
	};

	double y = 0;
		
	for(int ln = 0; ln < line_no; ln++)
		y += HGRID[2*ln] + HGRID[2*ln+1];

	y += HGRID[line_no]/2;
	
	return  gr_H/2 - y;
};

// Функция решения системы нелинейных уравнений методом
// Ньютона-Рафсона минимизацией вектора невязки системы
BOOL FXNewton(double *X,	// Массив искомых значений 
							// X0 = fi
							// X1 = ksi
							// X2 = fovx/2
			  double tan_t, // Тангенс наклона вертикалей 
			  double tan_dr,// Тангенс наклона горизонталей 
			  double tan_r0)// Тангенс наклона центральной горизонтали
{
	double	A[3][3];	// 
	double	B[3];		// 
	double	F[3];		// Вектор невязки системы

	int		M = 10;		// Максимальное код-во итераций при 
						// расхождении системы
	double	E = 0.02;   // Относительная погрешность решения
	int		N = 3;		// Количество уравнений
	int		S = 0;		// Количество итераций 

	int		i,j,k,R;	// Прочие переменные
	double	x,H;		//
	double  MinErr = (PI/180)*(PI/180);

	do
	{
		// Вычисление вектора невязки для первичных приближений
		FXNewtonFunc(X,F, tan_t, tan_dr, tan_r0);

		// Подготовка вектора решений
		for(i=0; i < N; i++)
			B[i]=-F[i];

		// Созданий матрицы Якоби
		for(j=0; j< N; j++)
		{
			x=X[j];
//			H=E*fabs(x);
			H=E;
			X[j]=x+H;
			FXNewtonFunc(X,F, tan_t, tan_dr, tan_r0);
			for(i=0;i<N;i++)
				A[i][j]=(F[i]+B[i])/H;
			X[j]=x;
		};

		// При привышение количества итераций лимита M
		// считать систему расходящейся
		S++;
		if(S == M+1)
			return FALSE;

		// Решение системы линейных уравнений мсетодом Гаусса
		// **************************************************
		for(i=0; i< N-1; i++ )
			for(j=i+1; j<N; j++)
			{
				A[j][i] = -A[j][i]/A[i][i];
				for(k=i+1; k<N; k++)
					A[j][k] += A[j][i]*A[i][k];
				B[j] += A[j][i]*B[i];
			};
	
		F[N-1]=B[N-1]/A[N-1][N-1];
		for(i=N-2; i>=0; i--)
		{
			H=B[i];
			for(j=i+1; j<N; j++)
				H -= F[j]*A[i][j];
			F[i]=H/A[i][i];
		};
		// ***************************************************


		// Вычисление уточненных значений X+dX
		R=0;
/*		for(i=0; i<N; i++)
		{
			X[i] += F[i];
			if(fabs(F[i]/X[i]) > E)
				R=1;
		};
*/
		double _err = (B[0]*B[0]+B[1]*B[1]+B[2]*B[2]);
		if( _err > MinErr)
		{
			R=0;
			for(i=0; i<N; i++)
				X[i] += F[i];
		}
		else
			R = 1;

	} while(R==0);

	return TRUE;

};

BOOL FXNewton2(double *X,	// Массив искомых значений 
							// X0 = fi
							// X1 = ksi
							// X2 = fovx/2
			  double tan_t, // Тангенс наклона вертикалей 
			  double tan_dr,// Тангенс наклона горизонталей 
			  double tan_r0)// Тангенс наклона центральной горизонтали
{
	double	F[3];		// Вектор невязки системы
	int		N = 3;		// Количество уравнений

	double  MinErr = (30*PI/180)*(30*PI/180); 
	double _err;
	int found = 0;



	for(int FOV = 59; (FOV > 10) && (found == 0); FOV--)
	{
		X[2] = FOV*PI/360;
		X[0] = atan(tan_t/X[2]);
		X[1] = atan(tan_dr/(3*tan(X[2])*cos(X[0])/2));
//		X[1] = atan(tan_dr/(3*tan(X[2])/2));
		FXNewtonFunc(X,F, tan_t, tan_dr, tan_r0);
		_err = F[0]*F[0]+F[1]*F[1]+F[2]*F[2];
		if( _err < MinErr)
			MinErr = _err;
		else
			found = 1;
	};

	return (found == 1);
};

// Функция для расчета вектора невязки системы уравнений F
void FXNewtonFunc(double *X,		// Массив искомых значений
				  double *F,		// Массив невязки
				  double tan_t,		// Наклон боковых линий
				  double tan_dr,	// Наклон горизонталей
				  double tan_r0)	// Наклон центральной горизонтали
{
	F[0] = tan_r0 + sin(X[0])*tan(X[1]);
	F[1] = tan_dr - 1.5*tan(X[1])*tan(X[2])*cos(X[0]);
//	F[1] = tan_dr - 1.5*tan(X[1])*tan(X[2]);
	F[2] = tan_t - tan(X[0])*tan(X[2]);
};

FXColorTable::FXColorTable(void)
{
	count = 0;
};

FXColorTable::~FXColorTable(void)
{
	count = 0;
};


BOOL FXColorTable::Add(COLORREF cl)
{
	if(count < FX_MAX_COLORS)
		colors[count++] = cl;
	else
		return FALSE;

	return TRUE;
};

BOOL FXColorTable::Remove(COLORREF cl)
{
	if( count == 0 )
		return FALSE;

	for(int i = 0; i < count; i++)
		if(cl == colors[i])
		{
			count--;
			for(int j=i; j < count ; j++ )
				colors[j] = colors[j+1];
			return TRUE;
		};

	return FALSE;
};

BOOL FXColorTable::Remove(int idx)
{
	if( (idx < 0) || (idx >= count) || (count == 0))
		return FALSE;

	count--;
	for(int j=idx; j < count ; j++ )
		colors[j] = colors[j+1];
	return TRUE;
};

 int FXColorTable::GetCount(void)
{
	return count;
};

COLORREF FXColorTable::GetColor(int idx)
{
	if( (idx < 0) || (idx >= count) || (count == 0))
		return COLORREF(-1);

	return colors[idx];
};

void FXRegressia(int dir, int N, int *dists, int *mids, double &a, double &c)
{
	int A = 0,B = 0, C = 0, D = 0;
	double B1;

	for(int i = 0; i < N; i++)
	{
		A += dists[i];
		B += mids[i];
		C += dists[i]*dists[i];
		D += dists[i]*mids[i];
	};

	B1 = (A*B-N*D+0.00001)/(A*A-N*C);
	if(dir)
		c = (B - B1*A)/N + FX_FRAME_SZX/2*B1;
	else
		c = (B - B1*A)/N + FX_FRAME_SZY/2*B1;
	a = atan(B1);
};


