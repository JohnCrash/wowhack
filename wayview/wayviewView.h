// wayviewView.h : CwayviewView 类的接口
//


#pragma once


class CwayviewView : public CView
{
protected: // 仅从序列化创建
	CwayviewView();
	DECLARE_DYNCREATE(CwayviewView)

// 属性
public:
	CwayviewDoc* GetDocument() const;

// 操作
public:

// 重写
	public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CwayviewView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnButtonZoomin();
	afx_msg void OnButtonZoomout();
	afx_msg void OnButtonMoveright();
	afx_msg void OnButtonMoveleft();
	afx_msg void OnButtonMoveup();
	afx_msg void OnButtonMovedown();
	afx_msg void OnGoto();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // wayviewView.cpp 的调试版本
inline CwayviewDoc* CwayviewView::GetDocument() const
   { return reinterpret_cast<CwayviewDoc*>(m_pDocument); }
#endif

