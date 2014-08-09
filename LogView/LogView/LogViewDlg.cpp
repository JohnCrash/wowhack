// LogViewDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "LogView.h"
#include "LogViewDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CLogViewDlg 对话框




CLogViewDlg::CLogViewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLogViewDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLogViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCROLLBAR1, m_time);
	DDX_Control(pDX, IDC_LIST4, m_unit);
	DDX_Control(pDX, IDC_STATIC_TIME, m_timeInfo);
	DDX_Control(pDX, IDC_COMBO1, m_type);
	DDX_Control(pDX, IDC_STATIC_INFO, m_info);
	DDX_Control(pDX, IDC_LIST1, m_cast);
	DDX_Control(pDX, IDC_STATIC_RECT, m_showRect);
}

BEGIN_MESSAGE_MAP(CLogViewDlg, CDialog)
	ON_WM_SYSCOMMAND()
//	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CLogViewDlg::OnBnClickedButton1)
	ON_NOTIFY(NM_THEMECHANGED, IDC_SCROLLBAR1, &CLogViewDlg::OnNMThemeChangedScrollbar1)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_CBN_SELCHANGE(IDC_COMBO1, &CLogViewDlg::OnCbnSelchangeCombo1)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST4, &CLogViewDlg::OnLvnItemchangedList4)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CLogViewDlg::OnLvnItemchangedList1)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CLogViewDlg 消息处理程序

BOOL CLogViewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	m_unit.InsertColumn(0,TEXT("名称"),LVCFMT_LEFT,100);
	m_unit.InsertColumn(1,TEXT("#"),LVCFMT_LEFT,32);
	m_unit.InsertColumn(2,TEXT("职业"),LVCFMT_LEFT,48);
	m_unit.InsertColumn(3,TEXT("血量"),LVCFMT_LEFT,96);
	m_unit.InsertColumn(4,TEXT("魔法"),LVCFMT_LEFT,48);
	m_unit.InsertColumn(5,TEXT("目标"),LVCFMT_LEFT,100);
	m_unit.InsertColumn(6,TEXT("Buff"),LVCFMT_LEFT,400);
	m_unit.InsertColumn(7,TEXT("Debuff"),LVCFMT_LEFT,480);

	m_type.AddString(TEXT("法术施放"));
	m_type.AddString(TEXT("法术沉默"));
	m_type.AddString(TEXT("伤害"));
	m_type.AddString(TEXT("治疗"));
	m_type.AddString(TEXT("角色死亡"));
	m_type.AddString(TEXT("承受伤害"));
	m_type.AddString(TEXT("Buff"));
	m_type.AddString(TEXT("Debuff"));
	m_type.AddString(TEXT("血量<60%"));
	m_type.AddString(TEXT("血量<40%"));
	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CLogViewDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

//void CLogViewDlg::OnPaint()
//{
//	if (IsIconic())
//	{
//		CPaintDC dc(this); // 用于绘制的设备上下文
//
//		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
//
//		// 使图标在工作区矩形中居中
//		int cxIcon = GetSystemMetrics(SM_CXICON);
//		int cyIcon = GetSystemMetrics(SM_CYICON);
//		CRect rect;
//		GetClientRect(&rect);
//		int x = (rect.Width() - cxIcon + 1) / 2;
//		int y = (rect.Height() - cyIcon + 1) / 2;
//
//		// 绘制图标
//		dc.DrawIcon(x, y, m_hIcon);
//	}
//	else
//	{
//		CDialog::OnPaint();
//	}
//}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CLogViewDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//Load
void CLogViewDlg::OnBnClickedButton1()
{
	CFileDialog dlg( TRUE,TEXT("log"),NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		TEXT("Wow areana log file (*.log)|*.log|All (*.*)|*.*||"),this);
	if( dlg.DoModal()==IDOK ){
		log.load( dlg.GetPathName().GetBuffer() );
		CString str;
		double time = log.getTimeLength();
		str.Format(TEXT("%s (%d分钟%d秒)"),dlg.GetPathName().GetBuffer(),(int)(time/60),(int)((int)time%60) );
		SetWindowText( str );
		m_time.SetScrollRange(0,(int)log.getTimeLength()+1 );
		str.Format(TEXT("%s %d%d [ "),log.zone.c_str(),log.areanaType,log.areanaType);
		for(std::vector<std::wstring>::iterator i=log.friendRaces.begin();i!=log.friendRaces.end();i++ ){
			str += i->c_str();
			str += TEXT(" ");
		}
		str += TEXT("]->[ ");
		for(std::vector<std::wstring>::iterator i=log.enemyRaces.begin();i!=log.enemyRaces.end();i++ ){
			str += i->c_str();
			str += TEXT(" ");
		}
		str += TEXT("]");
		m_info.SetWindowText(str);
		Goto(0);
	}
}

void CLogViewDlg::Goto( double time ){
	int n,count;
	CString str;
	m_unit.DeleteAllItems();
	cur_time = time;
	cur=log.gotoLog( time );
	count=1;
	if( !cur.states.empty() ){
		for( std::vector<UnitState>::iterator i=cur.states.begin();i!=cur.states.end();i++ ){
		//	if( i->isParty ){
		//		n = m_unit.InsertItem(0,i->name.c_str());
		//	}else{
				n = m_unit.InsertItem(m_unit.GetItemCount(),i->name.c_str());
		//	}
			str.Format(TEXT("%d"),count++);
			m_unit.SetItemText(n,1,str);

			m_unit.SetItemText(n,2,i->race.c_str());
			str.Format(TEXT("%d/%d"),i->health,i->healthMax);
			m_unit.SetItemText(n,3,str);
			str.Format(TEXT("%d"),i->mana);
			m_unit.SetItemText(n,4,str);
			m_unit.SetItemText(n,5,i->target.c_str());
			str.Empty();
			for( std::vector<std::wstring>::iterator j=i->buff.begin();j!=i->buff.end();j++){
				if( j!=i->buff.begin() )
					str += TEXT("|");
				str += j->c_str();
			}
			m_unit.SetItemText(n,6,str);
			str.Empty();
			for( std::vector<std::wstring>::iterator k=i->debuff.begin();k!=i->debuff.end();k++){
				if( k!=i->debuff.begin() )
					str += TEXT("|");
				str += k->c_str();
			}
			m_unit.SetItemText(n,7,str);
		}
	}
	m_time.SetScrollPos((int)time);
	str.Format(TEXT("%d分%d秒"),(int)(time/60),(int)((int)time%60) );
	m_timeInfo.SetWindowText( str );
	RECT rect;
	m_showRect.GetWindowRect(&rect);
	ScreenToClient(&rect);
	InvalidateRect(&rect);
}

void CLogViewDlg::OnNMThemeChangedScrollbar1(NMHDR *pNMHDR, LRESULT *pResult)
{
	int t = m_time.GetScrollPos();
	Goto(t);
	*pResult = 0;
}

void CLogViewDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CLogViewDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if( nSBCode==SB_LINELEFT ){
		int t = m_time.GetScrollPos();
		if( t>0 )
			Goto( t-1 );
	}else if(nSBCode==SB_LINERIGHT ){
		int t = m_time.GetScrollPos();
		Goto( t+1 );
	}else if( nSBCode==SB_PAGELEFT ){
		int t = m_time.GetScrollPos();
		if( t-log.getTimeLength()/10>=0 )
			Goto( t-log.getTimeLength()/10 );
		else
			Goto( 0 );
	}else if( nSBCode==SB_PAGERIGHT ){
		int t = m_time.GetScrollPos();
		if( t+log.getTimeLength()/10>=log.getTimeLength() )
			Goto( log.getTimeLength() );
		else
			Goto( t+log.getTimeLength()/10 );
	}else if( nSBCode==SB_THUMBPOSITION ){
		int t = nPos;
		Goto( t );
	}else if( nSBCode==SB_THUMBTRACK ){
		CString str;
		str.Format(TEXT("%d分%d秒"),(int)(nPos/60),(int)((int)nPos%60) );
		m_timeInfo.SetWindowText( str );
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CLogViewDlg::OnCbnSelchangeCombo1()
{
	CString str;
	int sel = m_type.GetCurSel();
	m_cast.DeleteAllItems();
	int nColnum = m_cast.GetHeaderCtrl()->GetItemCount();
	for( int z=0;z<nColnum;z++ )
		m_cast.DeleteColumn(0);
	POSITION pos = m_unit.GetFirstSelectedItemPosition();
	tracer.clear();
	while( pos ){
		int s = m_unit.GetNextSelectedItem(pos);
		tracer.push_back( std::wstring(m_unit.GetItemText(s,0).GetBuffer()) );
	}
	if( sel==0 ){//成功施法
		m_cast.InsertColumn(0,TEXT("施法者"),LVCFMT_LEFT,100);
		m_cast.InsertColumn(1,TEXT("法术"),LVCFMT_LEFT,96);
		m_cast.InsertColumn(2,TEXT("目标"),LVCFMT_LEFT,100);
		m_cast.InsertColumn(3,TEXT("时间"),LVCFMT_LEFT,64);
		for( std::vector<UnitLog>::iterator i=log.logs.begin();i!=log.logs.end();i++ ){
			if( i->type==TEXT("SPELL_CAST_SUCCESS") || i->type==TEXT("SPELL_CAST_START") ){
				if( tracer.empty() || find_name(i->sourceName) ){
					int n = m_cast.InsertItem(m_cast.GetItemCount(),i->sourceName.c_str() );
					m_cast.SetItemData(n,(DWORD)(i->time) );
					if( i->exs.size()>1 )
						m_cast.SetItemText(n,1,i->exs[1].c_str());
					m_cast.SetItemText(n,2,i->destName.c_str() );
					str.Format(TEXT("%d分%d秒"),(int)(i->time/60),(int)((int)i->time%60) );
					m_cast.SetItemText(n,3,str );
				}
			}
		}
	}else if(sel==1){//法术打断
		m_cast.InsertColumn(0,TEXT("施法者"),LVCFMT_LEFT,100);
		m_cast.InsertColumn(1,TEXT("法术"),LVCFMT_LEFT,96);
		m_cast.InsertColumn(2,TEXT("目标"),LVCFMT_LEFT,100);
		m_cast.InsertColumn(3,TEXT("被打断的法术"),LVCFMT_LEFT,96);
		m_cast.InsertColumn(4,TEXT("时间"),LVCFMT_LEFT,64);
		for( std::vector<UnitLog>::iterator i=log.logs.begin();i!=log.logs.end();i++ ){
			if( i->type==TEXT("SPELL_INTERRUPT") ){
				if( tracer.empty() || find_name(i->sourceName) ){
					int n = m_cast.InsertItem(m_cast.GetItemCount(),i->sourceName.c_str() );
					m_cast.SetItemData(n,(DWORD)(i->time) );
					if( i->exs.size()>1 )
						m_cast.SetItemText(n,1,i->exs[1].c_str());
					m_cast.SetItemText(n,2,i->destName.c_str() );
					if( i->exs.size()>4 )
						m_cast.SetItemText(n,3,i->exs[4].c_str());
					str.Format(TEXT("%d分%d秒"),(int)(i->time/60),(int)((int)i->time%60) );
					m_cast.SetItemText(n,4,str );
				}
			}
		}
	}else if(sel==2||sel==5){//伤害 ,承受伤害
		m_cast.InsertColumn(0,TEXT("施法者"),LVCFMT_LEFT,100);
		m_cast.InsertColumn(1,TEXT("法术"),LVCFMT_LEFT,96);
		m_cast.InsertColumn(2,TEXT("目标"),LVCFMT_LEFT,100);
		m_cast.InsertColumn(3,TEXT("伤害值"),LVCFMT_LEFT,96);
		m_cast.InsertColumn(4,TEXT("时间"),LVCFMT_LEFT,64);
		for( std::vector<UnitLog>::iterator i=log.logs.begin();i!=log.logs.end();i++ ){
			if( i->type==TEXT("SPELL_DAMAGE") || i->type==TEXT("SWING_DAMAGE") ||
				i->type==TEXT("SPELL_PERIODIC_DAMAGE") || i->type==TEXT("RANGE_DAMAGE") ){
				if( (sel==2 && (tracer.empty() || find_name(i->sourceName)))||
					(sel==5 && (tracer.empty() || find_name(i->destName)))){
					int n = m_cast.InsertItem(m_cast.GetItemCount(),i->sourceName.c_str() );
					m_cast.SetItemData(n,(DWORD)(i->time) );
					if( i->type==TEXT("SPELL_DAMAGE")||i->type==TEXT("SPELL_PERIODIC_DAMAGE")||
						i->type==TEXT("RANGE_DAMAGE")){
						if( i->exs.size()>1 )
							m_cast.SetItemText(n,1,i->exs[1].c_str());
					}else if( i->type==TEXT("SWING_DAMAGE") ){
						m_cast.SetItemText(n,1,TEXT("普通攻击"));
					}
					m_cast.SetItemText(n,2,i->destName.c_str() );
					if( i->type==TEXT("SPELL_DAMAGE")||i->type==TEXT("SPELL_PERIODIC_DAMAGE")||
						i->type==TEXT("RANGE_DAMAGE")){
						if( i->exs.size()>3 )
							m_cast.SetItemText(n,3,i->exs[3].c_str());
					}else if( i->type==TEXT("SWING_DAMAGE") ){
						if( i->exs.size()>0 )
							m_cast.SetItemText(n,3,i->exs[0].c_str());
					}
					str.Format(TEXT("%d分%d秒"),(int)(i->time/60),(int)((int)i->time%60) );
					m_cast.SetItemText(n,4,str );
				}
			}
		}
	}else if(sel==3){//治疗
		m_cast.InsertColumn(0,TEXT("施法者"),LVCFMT_LEFT,100);
		m_cast.InsertColumn(1,TEXT("法术"),LVCFMT_LEFT,96);
		m_cast.InsertColumn(2,TEXT("目标"),LVCFMT_LEFT,100);
		m_cast.InsertColumn(3,TEXT("治疗量"),LVCFMT_LEFT,96);
		m_cast.InsertColumn(4,TEXT("时间"),LVCFMT_LEFT,64);
		for( std::vector<UnitLog>::iterator i=log.logs.begin();i!=log.logs.end();i++ ){
			if( i->type==TEXT("SPELL_HEAL") ||
				i->type==TEXT("SPELL_PERIODIC_HEAL") ){
				if( tracer.empty() || find_name(i->sourceName) ){
					int n = m_cast.InsertItem(m_cast.GetItemCount(),i->sourceName.c_str() );
					m_cast.SetItemData(n,(DWORD)(i->time) );
					if( i->exs.size()>1 )
						m_cast.SetItemText(n,1,i->exs[1].c_str());
					m_cast.SetItemText(n,2,i->destName.c_str() );
					if( i->exs.size()>3 )
						m_cast.SetItemText(n,3,i->exs[3].c_str());
					str.Format(TEXT("%d分%d秒"),(int)(i->time/60),(int)((int)i->time%60) );
					m_cast.SetItemText(n,4,str );
				}
			}
		}
	}else if(sel==4){//死亡
		m_cast.InsertColumn(0,TEXT("死者"),LVCFMT_LEFT,100);
		m_cast.InsertColumn(1,TEXT("时间"),LVCFMT_LEFT,64);
		for( std::vector<UnitLog>::iterator i=log.logs.begin();i!=log.logs.end();i++ ){
			if( i->type==TEXT("UNIT_DIED") ){
				if( tracer.empty() || find_name(i->destName) ){
					int n = m_cast.InsertItem(m_cast.GetItemCount(),i->destName.c_str() );
					m_cast.SetItemData(n,(DWORD)(i->time) );
					str.Format(TEXT("%d分%d秒"),(int)(i->time/60),(int)((int)i->time%60) );
					m_cast.SetItemText(n,1,str );
				}
			}
		}
	}else if(sel==6||sel==7){//buff or debuff
		m_cast.InsertColumn(0,TEXT("施法者"),LVCFMT_LEFT,100);
		if( sel==6 )
			m_cast.InsertColumn(1,TEXT("Buff"),LVCFMT_LEFT,96);
		else
			m_cast.InsertColumn(1,TEXT("Debuff"),LVCFMT_LEFT,96);
		m_cast.InsertColumn(2,TEXT("目标"),LVCFMT_LEFT,100);
		m_cast.InsertColumn(3,TEXT("时间"),LVCFMT_LEFT,64);
		for( std::vector<UnitLog>::iterator i=log.logs.begin();i!=log.logs.end();i++ ){
			if( i->type==TEXT("SPELL_AURA_APPLIED") ||
				i->type==TEXT("SPELL_AURA_REMOVED") ){
				if( tracer.empty() || find_name(i->destName) ){
					if( (sel==6 && i->exs.size()>3 && wcscmp(i->exs[3].c_str(),TEXT("BUFF"))==0) ||
						(sel==7 && i->exs.size()>3 &&wcscmp(i->exs[3].c_str(),TEXT("DEBUFF"))==0)){
						int n = m_cast.InsertItem(m_cast.GetItemCount(),i->sourceName.c_str() );
						m_cast.SetItemData(n,(DWORD)(i->time) );
						std::wstring ss;
						if( i->type==TEXT("SPELL_AURA_APPLIED") )
							ss=TEXT("+");
						else
							ss=TEXT("-");
						if( i->exs.size()>1 ){
							ss+=i->exs[1];
							m_cast.SetItemText(n,1,ss.c_str());
						}
						m_cast.SetItemText(n,2,i->destName.c_str() );
						str.Format(TEXT("%d分%d秒"),(int)(i->time/60),(int)((int)i->time%60) );
						m_cast.SetItemText(n,3,str );
					}
				}
			}
		}
	}else if(sel==8||sel==9){//<60%
		m_cast.InsertColumn(0,TEXT("低血角色"),LVCFMT_LEFT,100);
		m_cast.InsertColumn(1,TEXT("血量"),LVCFMT_LEFT,100);
		m_cast.InsertColumn(2,TEXT("时间"),LVCFMT_LEFT,64);
		for( std::vector<UnitLog>::iterator i=log.logs.begin();i!=log.logs.end();i++ ){
			if( !i->states.empty() ){
				for( std::vector<UnitState>::iterator j=i->states.begin();j!=i->states.end();j++ ){
					float health = (float)j->health/(float)j->healthMax;
					if( tracer.empty() || find_name(j->name) ){
						int n;
						if( (sel==8 && health<0.6 ) || (sel==9 && health<0.4 )){
							CString str;
							n = m_cast.InsertItem(m_cast.GetItemCount(),j->name.c_str() );
							str.Format(TEXT("%0.2f"),health);
							m_cast.SetItemText(n,1,str );
							str.Format(TEXT("%d分%d秒"),(int)(i->time/60),(int)((int)i->time%60) );
							m_cast.SetItemText(n,2,str );
						}
					}
				}
			}
		}
	}
	RECT rect;
	m_showRect.GetWindowRect(&rect);
	ScreenToClient(&rect);
	InvalidateRect(&rect);
}

bool CLogViewDlg::find_name(std::wstring name){
	return std::find(tracer.begin(),tracer.end(),name )!=tracer.end();
}

void CLogViewDlg::OnLvnItemchangedList4(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	OnCbnSelchangeCombo1();
	*pResult = 0;
}

void CLogViewDlg::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	std::wstring caster;
	POSITION pos = m_cast.GetFirstSelectedItemPosition();
	if( pos ){
		int s = m_cast.GetNextSelectedItem(pos);
		DWORD t = (DWORD)m_cast.GetItemData(s);
		Goto( t );
	}
	*pResult = 0;
}

void CLogViewDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	if( !cur.states.empty() ){
		RECT rect;

		m_showRect.GetWindowRect(&rect);
		ScreenToClient(&rect);
		log.drawUnitState( &dc,rect,cur,tracer,cur_time );
	}
}
