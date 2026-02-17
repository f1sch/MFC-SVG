
// TestWindowDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "TestWindow.h"
#include "TestWindowDlg.h"
#include "afxdialogex.h"

#include "../svglib/Window.h"

#include <filesystem>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTestWindowDlg dialog
CTestWindowDlg::CTestWindowDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TESTWINDOW_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestWindowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PICTURE_CONTROL, d2dView_);
	DDX_Control(pDX, IDC_SVG_HIT_RESULT, staticHitResult_);
	DDX_Control(pDX, IDC_CURRENT_SELECTION, currentSelection_);
	DDX_Control(pDX, IDC_COMBO_SVGS, comboSvg_);
}

BEGIN_MESSAGE_MAP(CTestWindowDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
	ON_STN_CLICKED(IDC_SVG_HIT_RESULT, &CTestWindowDlg::OnStnClickedSvgHitResult)
	ON_CBN_SELCHANGE(IDC_COMBO_SVGS, &CTestWindowDlg::OnCbnSelchangeComboSvgs)
END_MESSAGE_MAP()


// CTestWindowDlg message handlers

BOOL CTestWindowDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED); // 

	CRect rect;
	GetDlgItem(IDC_PICTURE_CONTROL)->GetWindowRect(&rect);
	ScreenToClient(&rect);
	
	svgLibWindow_ = std::make_unique<SvgLib::Window::Window>();
	hr = svgLibWindow_->Initialize(this, rect);
	if (FAILED(hr))
	{
		// TODO: error handling (Initialize failed)
	}

	comboSvg_.AddString(L"assets\\humanBodyFront.svg");
	comboSvg_.AddString(L"assets\\humanBodyFrontTEST.svg");
	comboSvg_.AddString(L"assets\\d2d_transform_test_groups.svg");
	comboSvg_.AddString(L"assets\\Ghostscript_Tiger.svg");
	comboSvg_.AddString(L"assets\\d2d_stress_test_01_paths.svg");
	comboSvg_.AddString(L"assets\\d2d_stress_test_01_paths_supported.svg");
	comboSvg_.AddString(L"assets\\d2d_stress_test_02_strokes_transforms.svg");
	comboSvg_.AddString(L"assets\\d2d_stress_test_03_clip.svg");
	
		
	svgTestPath_ = AssetPath(L"assets\\humanBodyFront.svg");
	svgLibWindow_->SetGraphicToLoad(svgTestPath_);
	svgLibWindow_->SetEventListener(this);
	ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	currentSelection_.SetWindowTextW(L"Auswahl: ");
	staticHitResult_.SetWindowTextW(L"");

	CacheAnchors();
	

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTestWindowDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestWindowDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

void CTestWindowDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	UpdateAnchoredLayout(cx, cy);

	// svglib
	CWnd* pPic = GetDlgItem(IDC_PICTURE_CONTROL); 
	if (!pPic) return;

	CRect rect;
	pPic->GetWindowRect(&rect);
	ScreenToClient(&rect);

	svgLibWindow_.get()->MoveWindow(rect, TRUE);
}

void CTestWindowDlg::CacheAnchors()
{
	if (!::IsWindow(comboSvg_.m_hWnd))
		return;

	CRect client;
	GetClientRect(&client);

	CRect rcCombo;
	comboSvg_.GetWindowRect(&rcCombo);
	ScreenToClient(&rcCombo);

	comboMarginRight_ = client.right - rcCombo.right;
	comboMarginTop_ = rcCombo.top;
	comboSize_ = rcCombo.Size();
	comboAnchorCached_ = true;
}

void CTestWindowDlg::UpdateAnchoredLayout(int cx, int cy)
{
	if (!comboAnchorCached_ || !::IsWindow(comboSvg_.m_hWnd))
		return;

	const int left = max(0, cx - comboMarginRight_ - comboSize_.cx);
	const int top = max(0, comboMarginTop_);

	comboSvg_.SetWindowPos(
		nullptr,
		left,
		top,
		comboSize_.cx,
		comboSize_.cy,
		SWP_NOZORDER | SWP_NOACTIVATE
	);
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTestWindowDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTestWindowDlg::OnSvgHit(const std::wstring& id)
{
	if (!::IsWindow(staticHitResult_.m_hWnd)) {
		cachedHitId_ = id;
		return;
	}
	staticHitResult_.SetWindowTextW(id.c_str());
}

void CTestWindowDlg::OnShowSvgLibContextMenu(std::wstring shapeId, CPoint clickPos)
{
	// TESTING:
	CMenu hPopupMenu;
	hPopupMenu.CreatePopupMenu();

	if (hPopupMenu)
	{
		if (shapeId == L"Head")
		{
			// die IDs duerfen nicht bei 0 anfangen, weil ON_COMMAND(0, ...) reserviert ist
			hPopupMenu.AppendMenu(MF_STRING, 1001, _T("Vena jugularis interna dextra"));
			hPopupMenu.AppendMenu(MF_STRING, 1002, _T("Vena jugularis interna sinistra"));
			hPopupMenu.AppendMenu(MF_STRING, 1003, _T("Vena jugularis externa dextra"));
			hPopupMenu.AppendMenu(MF_STRING, 1004, _T("Vena jugularis externa sinistra"));
			hPopupMenu.AppendMenu(MF_STRING, 1005, _T("Vena jugularis subclavia dextra"));
			hPopupMenu.AppendMenu(MF_STRING, 1006, _T("Vena jugularis subclavia sinistra"));
		}

		POINT pt;
		GetCursorPos(&pt);
		UINT option = TrackPopupMenu(hPopupMenu, TPM_LEFTBUTTON, pt.x, pt.y, 0, m_hWnd, nullptr);

	}
}

void CTestWindowDlg::RepositionCtrl(int id)
{
	const int marginX = 10;
	const int marginY = 10;

	CWnd* pCtrl = GetDlgItem(id);
	if (!pCtrl) return;

	CRect rectCtrl;
	pCtrl->GetWindowRect(&rectCtrl);
	ScreenToClient(&rectCtrl);

	int width = rectCtrl.Width();
	int height = rectCtrl.Height();

	CRect rectClient;
	GetClientRect(&rectClient);

	rectCtrl.left = rectClient.right - width - marginX;
	rectCtrl.top = rectClient.bottom - height - marginY;
	rectCtrl.right = rectCtrl.left + width;
	rectCtrl.bottom = rectCtrl.top + height;

	pCtrl->MoveWindow(rectCtrl);
}

void CTestWindowDlg::OnStnClickedSvgHitResult()
{
	// TODO: Add your control notification handler code here
}

std::wstring CTestWindowDlg::ExeDir()
{
	wchar_t exePath[MAX_PATH]{};
	GetModuleFileNameW(nullptr, exePath, MAX_PATH);
	std::wstring exePathStr = std::filesystem::path(exePath).parent_path().wstring();
	return exePathStr;
}

std::wstring CTestWindowDlg::AssetPath(const wchar_t* rel)
{
	return (std::filesystem::path(ExeDir()) / rel).wstring();
}

void CTestWindowDlg::OnCbnSelchangeComboSvgs()
{
	int idx = comboSvg_.GetCurSel();
	if (idx != CB_ERR)
	{
		CString file;
		comboSvg_.GetLBText(idx, file);
		svgLibWindow_->SetGraphicToLoad(AssetPath(file));
		svgLibWindow_->Invalidate();
	}
}
