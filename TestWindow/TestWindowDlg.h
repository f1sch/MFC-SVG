
// TestWindowDlg.h : header file
//

#pragma once

#include "../svglib/Window.h"
#include <memory>

// CTestWindowDlg dialog
class CTestWindowDlg : public CDialogEx, public IEventListener
{
// Construction
public:
	CTestWindowDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TESTWINDOW_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
public:
	afx_msg void OnCbnSelchangeComboSvgs();
	afx_msg void OnStnClickedSvgHitResult();
	virtual void OnSvgHit(const std::wstring& id) override;

protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	void OnShowSvgLibContextMenu(std::wstring shapeId, CPoint clickPos);
	void RepositionCtrl(int id);

	std::wstring ExeDir();
	std::wstring AssetPath(const wchar_t* rel);
	void CacheAnchors();
	void UpdateAnchoredLayout(int cx, int cy);

public:
	CStatic d2dView_;
	
private:
	CComboBox comboSvg_;
	bool comboAnchorCached_ = false;
	int comboMarginRight_ = 0;
	int comboMarginTop_ = 0;
	CSize comboSize_{ 0, 0 };

	CStatic staticHitResult_;
	CStatic currentSelection_;
	std::wstring cachedHitId_;
	std::wstring svgTestPath_ = L"";
	
	// svglib
	std::unique_ptr<SvgLib::Window::Window> svgLibWindow_;
};
