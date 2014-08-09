// wayviewView.cpp : CwayviewView 类的实现
//

#include "stdafx.h"
#include "wayview.h"
#include "GotoDlg.h"
#include "wayviewDoc.h"
#include "wayviewView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "..\wowin\waypoint.h"
#include <fstream>
#include <sstream>
#include ".\wayviewview.h"
extern Cross g_cross;
extern Way g_way;

// CwayviewView

IMPLEMENT_DYNCREATE(CwayviewView, CView)

BEGIN_MESSAGE_MAP(CwayviewView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_COMMAND(ID_BUTTON_ZOOMIN, OnButtonZoomin)
	ON_COMMAND(ID_BUTTON_ZOOMOUT, OnButtonZoomout)
	ON_COMMAND(ID_BUTTON_MOVERIGHT, OnButtonMoveright)
	ON_COMMAND(ID_BUTTON_MOVELEFT, OnButtonMoveleft)
	ON_COMMAND(ID_BUTTON_MOVEUP, OnButtonMoveup)
	ON_COMMAND(ID_BUTTON_MOVEDOWN, OnButtonMovedown)
	ON_COMMAND(ID_GOTO, OnGoto)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

// CwayviewView 构造/析构

CwayviewView::CwayviewView()
{
	// TODO: 在此处添加构造代码
	g_cross.Load();
}

CwayviewView::~CwayviewView()
{
}

BOOL CwayviewView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改 CREATESTRUCT cs 来修改窗口类或
	// 样式

	return CView::PreCreateWindow(cs);
}

// CwayviewView 绘制
float g_z = 1.7;
float g_x = -4000;
float g_y = 2000;
float g_xx = 0;
float g_yy = 0;

void Transfrom( float& x,float& y ){
	float a = x;
	x = -y;
	y = -a;
	y += g_y;
	x += g_x;
	x *= g_z;
	y *= g_z;
	x += g_xx;
	y += g_yy;
}

void Line( CDC* pdc,float x,float y,float x1,float y1 ){
	Transfrom( x,y );
	Transfrom( x1,y1 );
	pdc->MoveTo( x,y );
	pdc->LineTo( x1,y1 );
}

void DrawPoint( CDC* pdc,float x,float y ){
	Transfrom( x,y );
	pdc->Ellipse(x-5,y-5,x+5,y+5);
}

void CwayviewView::OnDraw(CDC* /*pDC*/pdc)
{
	CwayviewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	{
		for( std::vector<Way*>::iterator i = g_cross.vWN.begin();i!=g_cross.vWN.end();i++ ){
			for( std::vector<Point>::iterator j = (*i)->vPoints.begin();j!=(*i)->vPoints.end();j++ ){
				if( j+1==(*i)->vPoints.end())break;
				Line( pdc,j->point[0],j->point[1],(j+1)->point[0],(j+1)->point[1] );
			}
		}
	}
	{
		CBrush br;
		br.CreateSolidBrush( RGB(255,0,0) );
		CBrush* pbr = (CBrush*)pdc->SelectObject( &br );
		for( std::vector<CrossPoint>::iterator i = g_cross.vCP.begin();i!=g_cross.vCP.end();i++ ){
			DrawPoint( pdc,i->point[0],i->point[1] );
		}
		pdc->SelectObject( pbr );
	}
	{
		CPen pe;
		pe.CreatePen( PS_SOLID,1,RGB(255,0,0));
		CPen* pp = (CPen*)pdc->SelectObject( &pe );
		for( std::vector<Point>::iterator i = g_way.vPoints.begin();i!=g_way.vPoints.end();i++ ){
			if( i+1==g_way.vPoints.end() )break;
			Line( pdc,i->point[0],i->point[1],(i+1)->point[0],(i+1)->point[1] );
		}
		pdc->SelectObject( pp );
	}
}


// CwayviewView 打印

BOOL CwayviewView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CwayviewView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 打印前添加额外的初始化
}

void CwayviewView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 打印后添加清除过程
}


// CwayviewView 诊断

#ifdef _DEBUG
void CwayviewView::AssertValid() const
{
	CView::AssertValid();
}

void CwayviewView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CwayviewDoc* CwayviewView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CwayviewDoc)));
	return (CwayviewDoc*)m_pDocument;
}
#endif //_DEBUG


// CwayviewView 消息处理程序

void CwayviewView::OnButtonZoomin()
{
	g_z -= 1;
	Invalidate();
}

void CwayviewView::OnButtonZoomout()
{
	g_z += 1;
	Invalidate();
}

void CwayviewView::OnButtonMoveright()
{
	g_x += 500;
	Invalidate();
}

void CwayviewView::OnButtonMoveleft()
{
	g_x -= 500;
	Invalidate();
}

void CwayviewView::OnButtonMoveup()
{
	g_y -= 500;
	Invalidate();
}

void CwayviewView::OnButtonMovedown()
{
	g_y += 500;
	Invalidate();
}

void CwayviewView::OnGoto()
{
	float from[3] = {1511.74,-4425.13,20.6891};
	float to[3] = {-3650.87,-2752.97,36.1253};
	CGotoDlg dlg( this );
	if( dlg.DoModal()==IDOK ){
		std::istringstream is( dlg.m_From.GetBuffer() );
		std::istringstream is1( dlg.m_To.GetBuffer() );
		if( is>>from[0]>>from[1]>>from[2] && is1>>to[0]>>to[1]>>to[2] )
			g_cross.Goto( from,to );
		Invalidate();
	}
}

bool bDown = false;
CPoint pPt;
void CwayviewView::OnLButtonDown(UINT nFlags, CPoint point)
{
	bDown = true;
	pPt = point;
	CView::OnLButtonDown(nFlags, point);
}

void CwayviewView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if( bDown ){
		g_xx += (point.x - pPt.x);
		g_yy += (point.y - pPt.y);
		Invalidate();
	}
	bDown = false;
	CView::OnLButtonUp(nFlags, point);
}
