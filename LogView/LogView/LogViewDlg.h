// LogViewDlg.h : 头文件
//

#pragma once
#include "Log.h"
#include "afxwin.h"
#include "afxcmn.h"

// CLogViewDlg 对话框
class CLogViewDlg : public CDialog
{
// 构造
public:
	CLogViewDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_LOGVIEW_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	AreanaLog log;
	UnitLog	cur;
	std::vector<std::wstring> tracer;
	bool find_name(std::wstring name);
	double cur_time;
	void Goto( double time );
// 实现
protected:
	HICON m_hIcon;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
//	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	CScrollBar m_time;
	afx_msg void OnNMThemeChangedScrollbar1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CListBox m_buff;
	CListBox m_debuff;
	afx_msg void OnLbnSelchangeList1();
	CListCtrl m_unit;
	CStatic m_timeInfo;
	CComboBox m_type;
	CStatic m_info;
	afx_msg void OnCbnSelchangeCombo1();
	CListCtrl m_cast;
	afx_msg void OnLvnItemchangedList4(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnPaint();
	CStatic m_showRect;
};
