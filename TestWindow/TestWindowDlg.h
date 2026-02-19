
// TestWindowDlg.h : header file
//

#pragma once

#include "../svglib/Window.h"
#include <memory>
#include <map>

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
	afx_msg void OnBnClickedCheckDoc();
	afx_msg void OnBnClickedCheckGeom();

	// Testing negative example
	afx_msg void OnBlutdruckMessen();
	afx_msg void OnPulsMessen();
	afx_msg void OnSauerstoffMessen();
	afx_msg void OnTemperaturMessen();
	afx_msg void OnPupillenreaktionPruefen();
	afx_msg void OnBewusstseinslageBeurteilen();
	afx_msg void OnArmbewegelichkeitTesten();
	afx_msg void OnBeinkraftTesten();
	afx_msg void OnFussreflexPruefen();
	afx_msg void OnHautinspektion();
	afx_msg void OnSchwellungPruefen();
	afx_msg void OnInspektionWundkontrolle();
	afx_msg void OnInspektionDeformitPruefen();
	afx_msg void OnKatheterBlasenkatheterLegen();
	afx_msg void OnKatheterZVKLegen();
	afx_msg void OnArteriellezugangLegenRadialis();
	afx_msg void OnArteriellezugangLegenFemoralis();
	afx_msg void OnInjektionIntramuskulaerDeltamuskel();
	afx_msg void OnInjektionIntramuskulaerGluteal();
	afx_msg void OnInjektionIntramuskulaerVastuslateralis();
	afx_msg void OnInjektionSubkutanAbdomen();
	afx_msg void OnInjektionSubkutanOberarm();
	afx_msg void OnInjektionSubkutanOberschenkel();
	afx_msg void OnZugangLegenHandvene();
	afx_msg void OnZugangLegenUnterarmvene();
	afx_msg void OnIntravenoesMedikamentApplizieren();
	afx_msg void OnDrainagenThoraxDrainage();
	afx_msg void OnDrainagenWundDrainage();
	afx_msg void OnVerbandstechnikenKopfverband();
	afx_msg void OnVerbandstechnikenSchulterverband();
	afx_msg void OnVerbandstechnikenArmverband();
	afx_msg void OnVerbandstechnikenBeinverband();
	afx_msg void OnVerbandstechnikenFussverband();
	afx_msg void OnRuhigstellungArmschienenAnlegen();
	afx_msg void OnRuhigstellungBeinschienenAnlegen();
	afx_msg void OnRuhigstellungHalskrauseAnlegen();
	afx_msg void OnKuehlungKuehlpackungAuflegen();
	afx_msg void OnKuehlungWaermepackungAuflegen();
	afx_msg void OnHochlagernArmHochlagern();
	afx_msg void OnHochlagernBeinHochLagern();
	afx_msg void OnNahtHautnaht();
	afx_msg void OnNahtKlammernSetzen();
	afx_msg void OnNahtFaedenEntfernen();
	afx_msg void OnAmputationArm();
	afx_msg void OnAmputationBein();
	afx_msg void OnAmputationFuss();
	afx_msg void OnChirurgischeMassnahmeWundrevision();
	afx_msg void OnRoentgenSchaedel();
	afx_msg void OnRoentgenThorax();
	afx_msg void OnRoentgenArm();
	afx_msg void OnRoentgenBein();
	afx_msg void OnRoentgenFuss();
	afx_msg void OnBildgebungCT();
	afx_msg void OnBildgebungMRT();
	afx_msg void OnDokumentationMassnahme();
	afx_msg void OnDokumentationVerlaufEintragen();
	afx_msg void OnDokumentationFotoHinzufuegen();
	afx_msg void OnDokumentationBerichtExportieren();


protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()

private:
	void RepositionCtrl(int id);

	std::wstring ExeDir();
	std::wstring AssetPath(const wchar_t* rel);
	void CacheAnchors();
	void UpdateAnchoredLayout(int cx, int cy);

public:
	CStatic d2dView_;
	
private:
	enum class BodyPart { Head, Torso, LeftArm, RightArm, LeftHand, RightHand, LeftLeg, RightLeg, LeftFoot, RightFoot, Unknown };
	struct Action { UINT menuId; std::wstring displayText; };
	std::map<BodyPart, std::vector<Action>> bodyPartActions_;
	BodyPart ParseBodyPart(const std::wstring& id);
	void SetBodyPartMapping();
	void ShowContextMenuForBodyPart(BodyPart part, CPoint screenPoint);

	CComboBox comboSvg_;
	bool comboAnchorCached_ = false;
	int comboMarginRight_ = 0;
	int comboMarginTop_ = 0;
	CSize comboSize_{ 0, 0 };

	CButton checkSvgDoc_;
	BOOL checkSvgDocEnableDisable_;
	bool checkSvgDocAnchorCached_ = false;
	int checkSvgDocMarginRight_ = 0;
	int checkSvgDocMarginTop_ = 0;
	CSize checkSvgDocSize_{ 0, 0 };

	CButton checkSvgGeometry_;
	BOOL checkSvgGeomEnableDisable_;
	bool checkSvgGeomAnchorCached_ = false;
	int checkSvgGeomMarginRight_ = 0;
	int checkSvgGeomMarginTop_ = 0;
	CSize checkSvgGeomSize_{ 0, 0 };

	CStatic staticHitResult_;
	CStatic currentSelection_;
	std::wstring cachedHitId_;
	std::wstring svgTestPath_ = L"";
	
	// svglib
	std::unique_ptr<SvgLib::Window::Window> svgLibWindow_;
};
