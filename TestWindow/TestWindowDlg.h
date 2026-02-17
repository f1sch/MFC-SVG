
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
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnCbnSelchangeComboSvgs();

	virtual void OnSvgHit(const std::wstring& id) override;
private:
	void OnShowSvgLibContextMenu(std::wstring shapeId, CPoint clickPos);
	void RepositionCtrl(int id);

	std::wstring ExeDir();
	std::wstring AssetPath(const wchar_t* rel);
private:
	CStatic staticHitResult_;
	CStatic currentSelection_;
	std::wstring cachedHitId_;
	// svglib
	std::unique_ptr<SvgLib::Window::Window> svgLibWindow_;
	// Testing
	std::wstring svgTestPath_ = L"";

public:
	CStatic d2dView_;
	CStatic controlContainer_;
	afx_msg void OnStnClickedSvgHitResult();
	CComboBox m_comboSvg;
};
