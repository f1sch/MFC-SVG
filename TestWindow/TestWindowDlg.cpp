
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
	, checkSvgDocEnableDisable_(FALSE)
	, checkSvgGeomEnableDisable_(FALSE)
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
	DDX_Control(pDX, IDC_CHECK_DOC, checkSvgDoc_);
	DDX_Control(pDX, IDC_CHECK_GEOM, checkSvgGeometry_);
	DDX_Check(pDX, IDC_CHECK_DOC, checkSvgDocEnableDisable_);
	DDX_Check(pDX, IDC_CHECK_GEOM, checkSvgGeomEnableDisable_);
}

BEGIN_MESSAGE_MAP(CTestWindowDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
	ON_STN_CLICKED(IDC_SVG_HIT_RESULT, &CTestWindowDlg::OnStnClickedSvgHitResult)
	ON_CBN_SELCHANGE(IDC_COMBO_SVGS, &CTestWindowDlg::OnCbnSelchangeComboSvgs)
	ON_BN_CLICKED(IDC_CHECK_DOC, &CTestWindowDlg::OnBnClickedCheckDoc)
	ON_BN_CLICKED(IDC_CHECK_GEOM, &CTestWindowDlg::OnBnClickedCheckGeom)
	ON_WM_CONTEXTMENU()
	// Testing negative example
	ON_COMMAND(ID_VITALPARAMETER_BLUTDRUCKMESSEN, &CTestWindowDlg::OnBlutdruckMessen)
	ON_COMMAND(ID_VITALPARAMETER_PULSMESSEN, &CTestWindowDlg::OnPulsMessen)
	ON_COMMAND(ID_VITALPARAMETER_SAUERSTOFFMESSEN, &CTestWindowDlg::OnSauerstoffMessen)
	ON_COMMAND(ID_VITALPARAMETER_TEMPERATURMESSEN, &CTestWindowDlg::OnTemperaturMessen)
	ON_COMMAND(ID_NEUROLOGISCHEUNTERSUCHUNG_PUPILLENREAKTIONPRUEFEN, &CTestWindowDlg::OnPupillenreaktionPruefen)
	ON_COMMAND(ID_NEUROLOGISCHEUNTERSUCHUNG_BEWUSSTSEINSLAGEBEURTEILEN, &CTestWindowDlg::OnBewusstseinslageBeurteilen)
	ON_COMMAND(ID_MOTORIKTESTEN_ARMBEWEGLICHKEITTESTEN, &CTestWindowDlg::OnArmbewegelichkeitTesten)
	ON_COMMAND(ID_MOTORIKTESTEN_BEINKRAFTTESTEN, &CTestWindowDlg::OnBeinkraftTesten)
	ON_COMMAND(ID_MOTORIKTESTEN_FUSSREFLEXPRUEFEN, &CTestWindowDlg::OnFussreflexPruefen)
	ON_COMMAND(ID_INSPEKTION_HAUTINSPEKTION, &CTestWindowDlg::OnHautinspektion)
	ON_COMMAND(ID_INSPEKTION_SCHWELLUNGPRUEFEN, &CTestWindowDlg::OnSchwellungPruefen)
	ON_COMMAND(ID_INSPEKTION_WUNDKONTROLLE, &CTestWindowDlg::OnInspektionWundkontrolle)
	ON_COMMAND(ID_INSPEKTION_DEFORMITPRUEFEN, &CTestWindowDlg::OnInspektionDeformitPruefen)
	ON_COMMAND(ID_KATHETER_BLASENKATHETERLEGEN, &CTestWindowDlg::OnKatheterBlasenkatheterLegen)
	ON_COMMAND(ID_KATHETER_ZVKLEGEN, &CTestWindowDlg::OnKatheterZVKLegen)
	ON_COMMAND(ID_ARTERIELLEZUGANGLEGEN_RADIALIS, &CTestWindowDlg::OnArteriellezugangLegenRadialis)
	ON_COMMAND(ID_ARTERIELLEZUGANGLEGEN_FEMORALIS, &CTestWindowDlg::OnArteriellezugangLegenFemoralis)
	ON_COMMAND(ID_INTRAMUSKULAER_DELTAMUSKEL, &CTestWindowDlg::OnInjektionIntramuskulaerDeltamuskel)
	ON_COMMAND(ID_INTRAMUSKULAER_GLUTEAL, &CTestWindowDlg::OnInjektionIntramuskulaerGluteal)
	ON_COMMAND(ID_INTRAMUSKULAER_VASTUSLATERALIS, &CTestWindowDlg::OnInjektionIntramuskulaerVastuslateralis)
	ON_COMMAND(ID_SUBKUTAN_ABDOMEN, &CTestWindowDlg::OnInjektionSubkutanAbdomen)
	ON_COMMAND(ID_SUBKUTAN_OBERARM, &CTestWindowDlg::OnInjektionSubkutanOberarm)
	ON_COMMAND(ID_SUBKUTAN_OBERSCHENKEL, &CTestWindowDlg::OnInjektionSubkutanOberschenkel)
	ON_COMMAND(ID_ZUGANGLEGEN_HANDVENE, &CTestWindowDlg::OnZugangLegenHandvene)
	ON_COMMAND(ID_ZUGANGLEGEN_UNTERARMVENE, &CTestWindowDlg::OnZugangLegenUnterarmvene)
	ON_COMMAND(ID_INTRAVENOES_MEDIKAMENTAPPLIZIEREN, &CTestWindowDlg::OnIntravenoesMedikamentApplizieren)
	ON_COMMAND(ID_DRAINAGEN_THORAXDRAINAGE, &CTestWindowDlg::OnDrainagenThoraxDrainage)
	ON_COMMAND(ID_DRAINAGEN_WUNDDRAINAGE, &CTestWindowDlg::OnDrainagenWundDrainage)
	ON_COMMAND(ID_VERBANDSTECHNIKEN_KOPFVERBAND, &CTestWindowDlg::OnVerbandstechnikenKopfverband)
	ON_COMMAND(ID_VERBANDSTECHNIKEN_SCHULTERVERBAND, &CTestWindowDlg::OnVerbandstechnikenSchulterverband)
	ON_COMMAND(ID_VERBANDSTECHNIKEN_ARMVERBAND, &CTestWindowDlg::OnVerbandstechnikenArmverband)
	ON_COMMAND(ID_VERBANDSTECHNIKEN_BEINVERBAND, &CTestWindowDlg::OnVerbandstechnikenBeinverband)
	ON_COMMAND(ID_VERBANDSTECHNIKEN_FUSSVERBAND, &CTestWindowDlg::OnVerbandstechnikenFussverband)
	ON_COMMAND(ID_RUHIGSTELLUNG_ARMSCHIENEANLEGEN, &CTestWindowDlg::OnRuhigstellungArmschienenAnlegen)
	ON_COMMAND(ID_RUHIGSTELLUNG_BEINSCHIENEANLEGEN, &CTestWindowDlg::OnRuhigstellungBeinschienenAnlegen)
	ON_COMMAND(ID_RUHIGSTELLUNG_HALSKRAUSEANLEGEN, &CTestWindowDlg::OnRuhigstellungHalskrauseAnlegen)
	ON_COMMAND(ID_KUEHLUNG_KUEHLPACKAUFLEGEN, &CTestWindowDlg::OnKuehlungKuehlpackungAuflegen)
	ON_COMMAND(ID_KUEHLUNG_WAERMEPACKAUFLEGEN, &CTestWindowDlg::OnKuehlungWaermepackungAuflegen)
	ON_COMMAND(ID_HOCHLAGERN_ARMHOCHLAGERN, &CTestWindowDlg::OnHochlagernArmHochlagern)
	ON_COMMAND(ID_HOCHLAGERN_BEINHOCHLAGERN, &CTestWindowDlg::OnHochlagernBeinHochLagern)
	ON_COMMAND(ID_NAHT_HAUTNAHT, &CTestWindowDlg::OnNahtHautnaht)
	ON_COMMAND(ID_NAHT_KLAMMERNSETZEN, &CTestWindowDlg::OnNahtKlammernSetzen)
	ON_COMMAND(ID_NAHT_FAEDENENTFERNEN, &CTestWindowDlg::OnNahtFaedenEntfernen)
	ON_COMMAND(ID_AMPUTATION_ARM, &CTestWindowDlg::OnAmputationArm)
	ON_COMMAND(ID_AMPUTATION_BEIN, &CTestWindowDlg::OnAmputationBein)
	ON_COMMAND(ID_AMPUTATION_FUSS, &CTestWindowDlg::OnAmputationFuss)
	ON_COMMAND(ID_CHIRURGISCHEMA32835, &CTestWindowDlg::OnChirurgischeMassnahmeWundrevision)
	ON_COMMAND(ID_ROENTGEN_SCHAEDEL, &CTestWindowDlg::OnRoentgenSchaedel)
	ON_COMMAND(ID_ROENTGEN_THORAX, &CTestWindowDlg::OnRoentgenThorax)
	ON_COMMAND(ID_ROENTGEN_ARM, &CTestWindowDlg::OnRoentgenArm)
	ON_COMMAND(ID_ROENTGEN_BEIN, &CTestWindowDlg::OnRoentgenBein)
	ON_COMMAND(ID_ROENTGEN_FUSS, &CTestWindowDlg::OnRoentgenFuss)
	ON_COMMAND(ID_BILDGEBUNG_CT, &CTestWindowDlg::OnBildgebungCT)
	ON_COMMAND(ID_BILDGEBUNG_MRT, &CTestWindowDlg::OnBildgebungMRT)
	ON_COMMAND(ID_DOKUMENTATION_MASSNAHME, &CTestWindowDlg::OnDokumentationMassnahme)
	ON_COMMAND(ID_DOKUMENTATION_VERLAUFEINTRAGEN, &CTestWindowDlg::OnDokumentationVerlaufEintragen)
	ON_COMMAND(ID_DOKUMENTATION_FOTOHINZUFUEGEN, &CTestWindowDlg::OnDokumentationFotoHinzufuegen)
	ON_COMMAND(ID_DOKUMENTATION_BERICHTEXPORTIEREN, &CTestWindowDlg::OnDokumentationBerichtExportieren)
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
	comboSvg_.AddString(L"assets\\Ghostscript_Tiger.svg");
	comboSvg_.AddString(L"assets\\d2d_transform_test_groups.svg");
	//comboSvg_.AddString(L"assets\\d2d_stress_test_01_paths.svg");
	//comboSvg_.AddString(L"assets\\d2d_stress_test_01_paths_supported.svg");
	//comboSvg_.AddString(L"assets\\d2d_stress_test_02_strokes_transforms.svg");
	comboSvg_.AddString(L"assets\\d2d_stress_test_03_clip.svg");
	
		
	svgTestPath_ = AssetPath(L"assets\\humanBodyFront.svg");
	svgLibWindow_->SetGraphicToLoad(svgTestPath_);
	svgLibWindow_->SetEventListener(this);
	ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	currentSelection_.SetWindowTextW(L"Selection: ");
	staticHitResult_.SetWindowTextW(L"");

	CacheAnchors();
	
	checkSvgDoc_.SetCheck(true);
	checkSvgGeometry_.SetCheck(true);

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

	CRect rcCheckSvgDoc;
	checkSvgDoc_.GetWindowRect(&rcCheckSvgDoc);
	ScreenToClient(&rcCheckSvgDoc);

	checkSvgDocMarginRight_ = client.right - rcCheckSvgDoc.right;
	checkSvgDocMarginTop_ = rcCheckSvgDoc.top;
	checkSvgDocSize_ = rcCheckSvgDoc.Size();
	checkSvgDocAnchorCached_ = true;

	CRect rcCheckSvgGeom;
	checkSvgGeometry_.GetWindowRect(&rcCheckSvgGeom);
	ScreenToClient(&rcCheckSvgGeom);

	checkSvgGeomMarginRight_ = client.right - rcCheckSvgGeom.right;
	checkSvgGeomMarginTop_ = rcCheckSvgGeom.top;
	checkSvgGeomSize_ = rcCheckSvgGeom.Size();
	checkSvgGeomAnchorCached_ = true;
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

	if (!checkSvgDocAnchorCached_ || !::IsWindow(checkSvgDoc_.m_hWnd))
		return;

	const int leftDoc = max(0, cx - checkSvgDocMarginRight_ - checkSvgDocSize_.cx);
	const int topDoc = max(0, checkSvgDocMarginTop_);

	checkSvgDoc_.SetWindowPos(
		nullptr,
		leftDoc,
		topDoc,
		checkSvgDocSize_.cx,
		checkSvgDocSize_.cy,
		SWP_NOZORDER | SWP_NOACTIVATE
	);

	if (!checkSvgGeomAnchorCached_ || !::IsWindow(checkSvgGeometry_.m_hWnd))
		return;

	const int leftGeom = max(0, cx - checkSvgGeomMarginRight_ - checkSvgGeomSize_.cx);
	const int topGeom = max(0, checkSvgGeomMarginTop_);

	checkSvgGeometry_.SetWindowPos(
		nullptr,
		leftGeom,
		topGeom,
		checkSvgGeomSize_.cx,
		checkSvgGeomSize_.cy,
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

void CTestWindowDlg::OnBnClickedCheckDoc()
{
	UpdateData(TRUE);
	if (checkSvgDocEnableDisable_)
		svgLibWindow_->SetRendererStates("doc", true);
	else
		svgLibWindow_->SetRendererStates("doc", false);
	
	svgLibWindow_->Invalidate();
}

void CTestWindowDlg::OnBnClickedCheckGeom()
{
	UpdateData(TRUE);
	if (checkSvgGeomEnableDisable_)
		svgLibWindow_->SetRendererStates("geom", true);
	else
		svgLibWindow_->SetRendererStates("geom", false);
	
	svgLibWindow_->Invalidate();
}

void CTestWindowDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CMenu menu;
	menu.LoadMenuW(IDR_NEGATIVE_CONTEXT_MENU);

	CMenu* pPopup = menu.GetSubMenu(0);
	if (pPopup != nullptr)
	{
		pPopup->TrackPopupMenu(
			TPM_LEFTALIGN | TPM_RIGHTBUTTON,
			point.x,
			point.y,
			this
		);
	}
}

void CTestWindowDlg::OnBlutdruckMessen()
{
	AfxMessageBox(L"Blutdruck messen gewählt");
}

void CTestWindowDlg::OnPulsMessen()
{
	AfxMessageBox(L"Puls messen gewählt");
}

void CTestWindowDlg::OnSauerstoffMessen()
{
	AfxMessageBox(L"Sauerstoff messen gewählt");
}

void CTestWindowDlg::OnTemperaturMessen()
{
	AfxMessageBox(L"Temperatur messen gewählt");
}

void CTestWindowDlg::OnPupillenreaktionPruefen()
{
	AfxMessageBox(L"Pupillenreaktion pruefen gewählt");
}

void CTestWindowDlg::OnBewusstseinslageBeurteilen()
{
	AfxMessageBox(L"Bewusstseinslage beurteilen gewählt");
}

void CTestWindowDlg::OnArmbewegelichkeitTesten()
{
	AfxMessageBox(L"Armbeweglichkeit testen gewählt");
}

void CTestWindowDlg::OnBeinkraftTesten()
{
	AfxMessageBox(L"Beinkraft testen gewählt");
}

void CTestWindowDlg::OnFussreflexPruefen()
{
	AfxMessageBox(L"Fussreflex pruefen gewählt");
}

void CTestWindowDlg::OnHautinspektion()
{
	AfxMessageBox(L"Hautinspektion gewählt");
}

void CTestWindowDlg::OnSchwellungPruefen()
{
	AfxMessageBox(L"Schwellung pruefen gewählt");
}

void CTestWindowDlg::OnInspektionWundkontrolle()
{
	AfxMessageBox(L"Inspektion Wundkontrolle gewählt");
}

void CTestWindowDlg::OnInspektionDeformitPruefen()
{
	AfxMessageBox(L"Inspektion Deformit pruefen gewählt");
}

void CTestWindowDlg::OnKatheterBlasenkatheterLegen()
{
	AfxMessageBox(L"Blasenkatheter legen gewählt");
}

void CTestWindowDlg::OnKatheterZVKLegen()
{
	AfxMessageBox(L"ZVK legen gewählt");
}
void CTestWindowDlg::OnArteriellezugangLegenRadialis()
{
	AfxMessageBox(L"Arterieller Zugang legen Radialis punktieren gewählt");
}

void CTestWindowDlg::OnArteriellezugangLegenFemoralis()
{
	AfxMessageBox(L"Arterieller Zugang legen Femoralis punktieren gewählt");
}

void CTestWindowDlg::OnInjektionIntramuskulaerDeltamuskel()
{
	AfxMessageBox(L"Injektion intramuskulaer Deltamuskel gewählt");
}

void CTestWindowDlg::OnInjektionIntramuskulaerGluteal()
{
	AfxMessageBox(L"Injektion intramuskulaer Gluteal gewählt");
}

void CTestWindowDlg::OnInjektionIntramuskulaerVastuslateralis()
{
	AfxMessageBox(L"Injektion intramuskulaer Vastus lateralis gewählt");
}

void CTestWindowDlg::OnInjektionSubkutanAbdomen()
{
	AfxMessageBox(L"Injektion subkutan Abdomen gewählt");
}

void CTestWindowDlg::OnInjektionSubkutanOberarm()
{
	AfxMessageBox(L"Injektion subkutan Oberarm gewählt");
}

void CTestWindowDlg::OnInjektionSubkutanOberschenkel()
{
	AfxMessageBox(L"Injektion subkutan Oberschenkel gewählt");
}

void CTestWindowDlg::OnZugangLegenHandvene()
{
	AfxMessageBox(L"Zugang legen Handvene gewählt");
}

void CTestWindowDlg::OnZugangLegenUnterarmvene()
{
	AfxMessageBox(L"Zugang legen Unterarmvene gewählt");
}

void CTestWindowDlg::OnIntravenoesMedikamentApplizieren()
{
	AfxMessageBox(L"Intravenoes Medikament applizieren gewählt");
}

void CTestWindowDlg::OnDrainagenThoraxDrainage()
{
	AfxMessageBox(L"Thoraxdrainage gewählt");
}

void CTestWindowDlg::OnDrainagenWundDrainage()
{
	AfxMessageBox(L"Wunddrainage gewählt");
}

void CTestWindowDlg::OnVerbandstechnikenKopfverband()
{
	AfxMessageBox(L"Kopfverband gewählt");
}

void CTestWindowDlg::OnVerbandstechnikenSchulterverband()
{
	AfxMessageBox(L"Schulterverband gewählt");
}

void CTestWindowDlg::OnVerbandstechnikenArmverband()
{
	AfxMessageBox(L"Armverband gewählt");
}

void CTestWindowDlg::OnVerbandstechnikenBeinverband()
{
	AfxMessageBox(L"Beinverband gewählt");
}

void CTestWindowDlg::OnVerbandstechnikenFussverband()
{
	AfxMessageBox(L"Fussverband gewählt");
}

void CTestWindowDlg::OnRuhigstellungArmschienenAnlegen()
{
	AfxMessageBox(L"Ruhigstellung Armschienen anlegen gewählt");
}

void CTestWindowDlg::OnRuhigstellungBeinschienenAnlegen()
{
	AfxMessageBox(L"Ruhigstellung Beinschienen anlegen gewählt");
}

void CTestWindowDlg::OnRuhigstellungHalskrauseAnlegen()
{
	AfxMessageBox(L"Ruhigstellung Halskrause anlegen gewählt");
}

void CTestWindowDlg::OnKuehlungKuehlpackungAuflegen()
{
	AfxMessageBox(L" Kuehlpackung auflegen gewählt");
}

void CTestWindowDlg::OnKuehlungWaermepackungAuflegen()
{
	AfxMessageBox(L"Waermepackung auflegen gewählt");
}

void CTestWindowDlg::OnHochlagernArmHochlagern()
{
	AfxMessageBox(L"Arm hochlagern gewählt");
}

void CTestWindowDlg::OnHochlagernBeinHochLagern()
{
	AfxMessageBox(L"Bein hochlagern gewählt");
}

void CTestWindowDlg::OnNahtHautnaht()
{
	AfxMessageBox(L"Hautnaht gewählt");
}

void CTestWindowDlg::OnNahtKlammernSetzen()
{
	AfxMessageBox(L"Naht Klammern setzen gewählt");
}

void CTestWindowDlg::OnNahtFaedenEntfernen()
{
	AfxMessageBox(L"Naht Faeden entfernen gewählt");
}

void CTestWindowDlg::OnAmputationArm()
{
	AfxMessageBox(L"Amputation Arm gewählt");
}

void CTestWindowDlg::OnAmputationBein()
{
	AfxMessageBox(L"Amputation Bein gewählt");
}

void CTestWindowDlg::OnAmputationFuss()
{
	AfxMessageBox(L"Amputation Fuss gewählt");
}

void CTestWindowDlg::OnChirurgischeMassnahmeWundrevision()
{
	AfxMessageBox(L"Wundrevision gewählt");
}

void CTestWindowDlg::OnRoentgenSchaedel()
{
	AfxMessageBox(L"Schaedel roentgen gewählt");
}

void CTestWindowDlg::OnRoentgenThorax()
{
	AfxMessageBox(L"Thorax roentgen gewählt");
}

void CTestWindowDlg::OnRoentgenArm()
{
	AfxMessageBox(L"Arm roentgen gewählt");
}

void CTestWindowDlg::OnRoentgenBein()
{
	AfxMessageBox(L"Bein roentgen gewählt");
}

void CTestWindowDlg::OnRoentgenFuss()
{
	AfxMessageBox(L"Fuss roentgen gewählt");
}

void CTestWindowDlg::OnBildgebungCT()
{
	AfxMessageBox(L"CT Bildgebung gewählt");
}

void CTestWindowDlg::OnBildgebungMRT()
{
	AfxMessageBox(L"MRT Bildgebung gewählt");
}

void CTestWindowDlg::OnDokumentationMassnahme()
{
	AfxMessageBox(L"Dokumentation Massnahme gewählt");
}

void CTestWindowDlg::OnDokumentationVerlaufEintragen()
{
	AfxMessageBox(L"Dokumentation Verlauf eintragen gewählt");
}

void CTestWindowDlg::OnDokumentationFotoHinzufuegen()
{
	AfxMessageBox(L"Dokumentation Foto hinzufuegen gewählt");
}

void CTestWindowDlg::OnDokumentationBerichtExportieren()
{
	AfxMessageBox(L"Dokumentation Bericht exportieren gewählt");
}
