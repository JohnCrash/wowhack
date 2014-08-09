// wayviewDoc.cpp :  CwayviewDoc 类的实现
//

#include "stdafx.h"
#include "wayview.h"

#include "wayviewDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CwayviewDoc

IMPLEMENT_DYNCREATE(CwayviewDoc, CDocument)

BEGIN_MESSAGE_MAP(CwayviewDoc, CDocument)
END_MESSAGE_MAP()


// CwayviewDoc 构造/析构

CwayviewDoc::CwayviewDoc()
{
	// TODO: 在此添加一次性构造代码

}

CwayviewDoc::~CwayviewDoc()
{
}

BOOL CwayviewDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)

	return TRUE;
}




// CwayviewDoc 序列化

void CwayviewDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}


// CwayviewDoc 诊断

#ifdef _DEBUG
void CwayviewDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CwayviewDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CwayviewDoc 命令
