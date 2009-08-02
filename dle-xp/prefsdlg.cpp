// dlcView.cpp : implementation of the CMineView class
//

#include "stdafx.h"
#include "afxpriv.h"
#include "dlc.h"

#include "dlcDoc.h"
#include "mineview.h"
#include "toolview.h"

#include "palette.h"
#include "textures.h"
#include "global.h"
#include "render.h"
#include "io.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	MAX_VIEWDIST	30

/////////////////////////////////////////////////////////////////////////////
// CToolView

BEGIN_MESSAGE_MAP(CPrefsDlg, CToolDlg)
	ON_WM_HSCROLL ()
	ON_BN_CLICKED (IDC_PREFS_BROWSE_D1PIG, OnOpenD1PIG)
	ON_BN_CLICKED (IDC_PREFS_BROWSE_D2PIG, OnOpenD2PIG)
	ON_BN_CLICKED (IDC_PREFS_BROWSE_MISSIONS, OnOpenMissions)
	ON_BN_CLICKED (IDC_PREFS_VIEW_MINE_NONE, OnViewMineNone)
	ON_BN_CLICKED (IDC_PREFS_VIEW_MINE_ALL, OnViewMineAll)
	ON_BN_CLICKED (IDC_PREFS_VIEW_OBJECTS_NONE, OnViewObjectsNone)
	ON_BN_CLICKED (IDC_PREFS_VIEW_OBJECTS_ALL, OnViewObjectsAll)
	ON_BN_CLICKED (IDC_PREFS_VIEW_PLAYERS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_ROBOTS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_WEAPONS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_POWERUPS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_KEYS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_HOSTAGES, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_REACTOR, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_WALLS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_SPECIAL, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_LIGHTS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_SHADING, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_DELTALIGHTS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_HIDEMARKED, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_ALLTEXTURES, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_SKYBOX, OnOK)
	ON_BN_CLICKED (IDC_PREFS_DEPTH_OFF, OnOK)
	ON_BN_CLICKED (IDC_PREFS_DEPTH_LOW, OnOK)
	ON_BN_CLICKED (IDC_PREFS_DEPTH_MEDIUM, OnOK)
	ON_BN_CLICKED (IDC_PREFS_DEPTH_HIGH, OnOK)
	ON_BN_CLICKED (IDC_PREFS_ROTATE_45000, OnOK)
	ON_BN_CLICKED (IDC_PREFS_ROTATE_12500, OnOK)
	ON_BN_CLICKED (IDC_PREFS_ROTATE_06250, OnOK)
	ON_BN_CLICKED (IDC_PREFS_ROTATE_03125, OnOK)
	ON_BN_CLICKED (IDC_PREFS_ROTATE_015625, OnOK)
	ON_BN_CLICKED (IDC_PREFS_LAYOUT0, OnLayout0)
	ON_BN_CLICKED (IDC_PREFS_LAYOUT1, OnLayout1)
	ON_BN_CLICKED (IDC_PREFS_EXPERTMODE, OnOK)
	ON_BN_CLICKED (IDC_PREFS_SPLASHSCREEN, OnOK)
	ON_BN_CLICKED (IDC_PREFS_USETEXCOLORS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_UNDO, OnOK)
	ON_EN_KILLFOCUS (IDC_PREFS_MOVERATE, OnOK)
	ON_EN_KILLFOCUS (IDC_PREFS_BROWSE_D1PIG, OnOK)
	ON_EN_KILLFOCUS (IDC_PREFS_BROWSE_D2PIG, OnOK)
	ON_EN_KILLFOCUS (IDC_PREFS_BROWSE_MISSIONS, OnOK)
	ON_CBN_SELCHANGE (IDC_PREFS_MINECENTER, OnSetMineCenter)
//	ON_EN_UPDATE (IDC_PREFS_MOVERATE, OnOK)
END_MESSAGE_MAP()

                        /*--------------------------*/

void CPrefsDlg::CompletePath (LPSTR pszPath, LPSTR pszFile, LPSTR pszExt)
{
if (*pszPath && !strstr (strlwr (pszPath), pszExt)) {
	if (pszPath [strlen (pszPath) - 1] != '\\')
		strcat (pszPath, "\\");
	strcat (pszPath, pszFile);
	}
}

                        /*--------------------------*/

CPrefsDlg::CPrefsDlg (CPropertySheet *pParent)
	: CToolDlg (nLayout ? IDD_PREFSDATA2 : IDD_PREFSDATA, pParent)
{
	char	szMoveRate [20];

GetPrivateProfileString ("DLE-XP", "DescentDirectory", descent_path, descent_path, sizeof (descent_path), "dle-xp.ini");
strcpy (m_d1Path, descent_path);
CompletePath (m_d1Path, "descent.pig", ".pig");
GetPrivateProfileString ("DLE-XP", "Descent2Directory", descent2_path, descent2_path, sizeof (descent2_path), "dle-xp.ini");
strcpy (m_d2Path, descent2_path);
CompletePath (m_d2Path, "groupa.pig", ".pig");
GetPrivateProfileString ("DLE-XP", "LevelsDirectory", levels_path, levels_path, sizeof (levels_path), "dle-xp.ini");
strcpy (m_missionsPath, levels_path);
CompletePath (m_missionsPath, "descent2.hog", ".hog");
GetPrivateProfileString ("DLE-XP", "PlayerProfile", player_profile, player_profile, sizeof (player_profile), "dle-xp.ini");
m_depthPerceptions [0] = 10000;
m_depthPerceptions [1] = 1000;
m_depthPerceptions [2] = 100;
m_depthPerceptions [3] = 50;
m_iDepthPerception = GetPrivateProfileInt ("DLE-XP", "DepthPerception", 0, "dle-xp.ini");
m_rotateRates [4] = (double) PI / 4.0;
m_rotateRates [3] = (double) PI / 8.0;
m_rotateRates [2] = (double) PI / 16.0;
m_rotateRates [1] = (double) PI / 32.0;
m_rotateRates [0] = (double) PI / 64.0;
m_iRotateRate = GetPrivateProfileInt ("DLE-XP", "RotateRate", 3, "dle-xp.ini");
GetPrivateProfileString ("DLE-XP", "MoveRate", "1", szMoveRate, sizeof (szMoveRate), "dle-xp.ini");
m_moveRate = (double) atof (szMoveRate);
m_bExpertMode = GetPrivateProfileInt ("DLE-XP", "ExpertMode", 1, "dle-xp.ini");
m_bSplashScreen = GetPrivateProfileInt ("DLE-XP", "SplashScreen", 1, "dle-xp.ini");
bExpertMode = (m_bExpertMode != 0);
m_mineViewFlags = GetPrivateProfileInt ("DLE-XP", "MineViewFlags", m_mineViewFlags, "dle-xp.ini");
m_objViewFlags = GetPrivateProfileInt ("DLE-XP", "ObjViewFlags", m_objViewFlags, "dle-xp.ini");
m_texViewFlags = GetPrivateProfileInt ("DLE-XP", "TexViewFlags", m_texViewFlags, "dle-xp.ini");
m_bUseTexColors = GetPrivateProfileInt ("DLE-XP", "UseTexColors", m_bUseTexColors, "dle-xp.ini");
m_nViewDist = GetPrivateProfileInt ("DLE-XP", "ViewDistance", 0, "dle-xp.ini");
m_nMineCenter = GetPrivateProfileInt ("DLE-XP", "MineCenter", 0, "dle-xp.ini");
m_nMaxUndo = GetPrivateProfileInt ("DLE-XP", "MaxUndo", MAX_UNDOS, "dle-xp.ini");
m_bNoRefresh = false;
m_bInvalid = false;
}

                        /*--------------------------*/

CPrefsDlg::~CPrefsDlg ()
{
SaveAppSettings ();
}

                        /*--------------------------*/

BOOL CPrefsDlg::OnInitDialog ()
{
if (!CToolDlg::OnInitDialog ())
   return FALSE;
m_btnBrowseD1PIG.AutoLoad (IDC_PREFS_BROWSE_D1PIG, this);
m_btnBrowseD2PIG.AutoLoad (IDC_PREFS_BROWSE_D2PIG, this);
m_btnBrowseMissions.AutoLoad (IDC_PREFS_BROWSE_MISSIONS, this);
#if 1
m_mineViewFlags = theApp.MineView ()->GetMineViewFlags ();
m_objViewFlags = theApp.MineView ()->GetObjectViewFlags ();
m_texViewFlags = theApp.TextureView ()->GetViewFlags ();
m_mineViewFlags = GetPrivateProfileInt ("DLE-XP", "MineViewFlags", m_mineViewFlags, "dle-xp.ini");
m_objViewFlags = GetPrivateProfileInt ("DLE-XP", "ObjViewFlags", m_objViewFlags, "dle-xp.ini");
m_texViewFlags = GetPrivateProfileInt ("DLE-XP", "TexViewFlags", m_texViewFlags, "dle-xp.ini");
m_nMaxUndo = GetPrivateProfileInt ("DLE-XP", "MaxUndo", MAX_UNDOS, "dle-xp.ini");
InitSlider (IDC_PREFS_VIEWDIST, 0, MAX_VIEWDIST);
int i;
for (i = 0; i <= MAX_VIEWDIST; i++)
	SlCtrl (IDC_PREFS_VIEWDIST)->SetTic (i);
CComboBox *pcb = CBMineCenter ();
pcb->AddString ("None");
pcb->AddString ("Crosshairs");
pcb->AddString ("Globe");
m_bInvalid = false;
SetAppSettings ();
#else
m_bInvalid = false;
#endif
return TRUE;
}

                        /*--------------------------*/


void CPrefsDlg::Refresh (void)
{
if (::IsWindow (m_hWnd) && !m_bNoRefresh) {
	OnCancel ();
/*
	m_mineViewFlags = theApp.MineView ()->GetMineViewFlags ();
	m_objViewFlags = theApp.MineView ()->GetObjectViewFlags ();
	m_texViewFlags = theApp.TextureView ()->GetViewFlags ();
	UpdateData (FALSE);
*/
	}
}

                        /*--------------------------*/

void CPrefsDlg::DoDataExchange (CDataExchange * pDX)
{
	int	h, i;

DDX_Text (pDX, IDC_PREFS_PATH_D1PIG, m_d1Path, sizeof (m_d1Path));
DDX_Text (pDX, IDC_PREFS_PATH_D2PIG, m_d2Path, sizeof (m_d2Path));
DDX_Text (pDX, IDC_PREFS_PATH_MISSIONS, m_missionsPath, sizeof (m_missionsPath));
DDX_Radio (pDX, IDC_PREFS_DEPTH_OFF, m_iDepthPerception);
DDX_Radio (pDX, IDC_PREFS_ROTATE_015625, m_iRotateRate);
#if 0
sprintf (szMoveRate, "%1.3f", m_moveRate);
DDX_Text (pDX, IDC_PREFS_MOVERATE, szMoveRate, sizeof (szMoveRate));
m_moveRate = atof (szMoveRate);
#else
DDX_Text (pDX, IDC_PREFS_MOVERATE, m_moveRate);
#endif
for (i = 0; i <= IDC_PREFS_VIEW_SKYBOX - IDC_PREFS_VIEW_WALLS; i++) {
	h = ((m_mineViewFlags & (1 << i)) != 0);
	DDX_Check (pDX, IDC_PREFS_VIEW_WALLS + i, h);
	if (h)
		m_mineViewFlags |= (1 << i);
	else
		m_mineViewFlags &= ~(1 << i);
	}
if (!pDX->m_bSaveAndValidate) {
	m_mineViewFlags = theApp.MineView ()->GetMineViewFlags ();
	m_objViewFlags = theApp.MineView ()->GetObjectViewFlags ();
	m_texViewFlags = theApp.TextureView ()->GetViewFlags ();
	}
h = ((m_texViewFlags & eViewMineUsedTextures) != 0);
DDX_Check (pDX, IDC_PREFS_VIEW_ALLTEXTURES, h);
m_texViewFlags = h ? eViewMineUsedTextures : 0;
for (i = 0; i <= IDC_PREFS_VIEW_REACTOR - IDC_PREFS_VIEW_ROBOTS; i++) {
	h = ((m_objViewFlags & (1 << i)) != 0);
	DDX_Check (pDX, IDC_PREFS_VIEW_ROBOTS + i, h);
	if (h)
		m_objViewFlags |= (1 << i);
	else
		m_objViewFlags &= ~(1 << i);
	}
DDX_Check (pDX, IDC_PREFS_EXPERTMODE, m_bExpertMode);
DDX_Check (pDX, IDC_PREFS_SPLASHSCREEN, m_bSplashScreen);
DDX_Text (pDX, IDC_PREFS_UNDO, m_nMaxUndo);
DDX_Radio (pDX, IDC_PREFS_LAYOUT0, nLayout);
DDX_Check (pDX, IDC_PREFS_USETEXCOLORS, m_bUseTexColors);
if (pDX->m_bSaveAndValidate)
	m_nMineCenter = CBMineCenter ()->GetCurSel ();
else {
	char	szViewDist [10];
	if (m_nViewDist)
		sprintf (szViewDist, "%d", theApp.MineView ()->ViewDist ());
	else
		strcpy (szViewDist, "all");
	((CWnd *) GetDlgItem (IDC_PREFS_VIEWDIST_TEXT))->SetWindowText (szViewDist);
	CBMineCenter ()->SetCurSel (m_nMineCenter);
	}
DDX_Text (pDX, IDC_PREFS_PLAYER, player_profile, sizeof (player_profile));
}

								/*--------------------------*/

void CPrefsDlg::OnSetMineCenter (void)
{
UpdateData (TRUE);
m_bNoRefresh = true;
SetAppSettings ();
m_bNoRefresh = false;
}

								/*--------------------------*/

BOOL CPrefsDlg::OnSetActive ()
{
Refresh ();
return CToolDlg::OnSetActive ();
}

                        /*--------------------------*/

bool CPrefsDlg::BrowseFile (LPSTR fileType, LPSTR fileName, LPSTR fileExt, BOOL bOpen)
{
   char        s [128];
   int         nResult;
   char		   pn [256];

strcpy (pn, fileName);
sprintf (s, "%s (%s)|%s|all files (*.*)|*.*||", fileType, fileExt, fileExt);
CFileDialog d (bOpen, fileExt, pn, 0, s, this);
d.m_ofn.hInstance = AfxGetInstanceHandle ();
d.m_ofn.lpstrInitialDir = pn;
if ((nResult = d.DoModal ()) != IDOK)
	return false;
strcpy (fileName, d.m_ofn.lpstrFile);
return true;
}

                        /*--------------------------*/

void CPrefsDlg::WritePrivateProfileInt (LPSTR szKey, int nValue)
{
	char	szValue [20];

sprintf (szValue, "%d", nValue);
WritePrivateProfileString ("DLE-XP", szKey, szValue, "dle-xp.ini");
}

                        /*--------------------------*/

void CPrefsDlg::WritePrivateProfileDouble (LPSTR szKey, double nValue)
{
	char	szValue [20];

sprintf (szValue, "%1.3f", nValue);
WritePrivateProfileString ("DLE-XP", szKey, szValue, "dle-xp.ini");
}

                        /*--------------------------*/

void CPrefsDlg::GetAppSettings ()
{
	int	i;

strcpy (m_d1Path, descent_path);
strcpy (m_d2Path, descent2_path);
strcpy (m_missionsPath, levels_path);
m_mineViewFlags = theApp.MineView ()->GetMineViewFlags ();
m_objViewFlags = theApp.MineView ()->GetObjectViewFlags ();
m_texViewFlags = theApp.TextureView ()->GetViewFlags ();
m_nMaxUndo = theApp.m_undoList.GetMaxSize ();
for (i = 0; i < 4; i++)
	if (depth_perception == m_depthPerceptions [i]) {
		m_iDepthPerception = i;
		break;
		}
for (i = 0; i < 5; i++)
	if (angle_rate <= m_rotateRates [i]) {
		m_iRotateRate = i;
		break;
		}
m_moveRate = (double) move_rate / 0x10000L;
m_bUseTexColors = theApp.GetMine ()->UseTexColors ();
m_bSplashScreen = theApp.m_bSplashScreen;
}

                        /*--------------------------*/

void CPrefsDlg::SaveAppSettings (bool bSaveFolders)
{
	char	szMoveRate [20];

if (bSaveFolders) {
	WritePrivateProfileString ("DLE-XP", "DescentDirectory", descent_path, "dle-xp.ini");
	WritePrivateProfileString ("DLE-XP", "Descent2Directory", descent2_path, "dle-xp.ini");
	WritePrivateProfileString ("DLE-XP", "levelsDirectory", levels_path, "dle-xp.ini");
	}
WritePrivateProfileString ("DLE-XP", "PlayerProfile", player_profile, "dle-xp.ini");
WritePrivateProfileInt ("DepthPerception", m_iDepthPerception);
WritePrivateProfileInt ("RotateRate", m_iRotateRate);
sprintf (szMoveRate, "%1.3f", m_moveRate);
WritePrivateProfileDouble ("MoveRate", m_moveRate);
WritePrivateProfileInt ("ExpertMode", m_bExpertMode);
WritePrivateProfileInt ("SplashScreen", m_bSplashScreen);
WritePrivateProfileInt ("MineViewFlags", m_mineViewFlags);
WritePrivateProfileInt ("ObjViewFlags", m_objViewFlags);
WritePrivateProfileInt ("TexViewFlags", m_texViewFlags);
WritePrivateProfileInt ("UseTexColors", m_bUseTexColors);
WritePrivateProfileInt ("ViewDistance", m_nViewDist);
WritePrivateProfileInt ("MineCenter", *theApp.MineView ()->MineCenter ());
WritePrivateProfileInt ("MaxUndo", m_nMaxUndo);
WritePrivateProfileInt ("TxtFilter", theApp.TextureView ()->TxtFilter ());
}
                        /*--------------------------*/

void CPrefsDlg::SetAppSettings (bool bInitApp)
{
if (m_bInvalid)
	return;
if (strcmp (descent_path, strlwr (m_d1Path))) {
	strcpy (descent_path, m_d1Path);
	WritePrivateProfileString ("DLE-XP", "DescentDirectory", descent_path, "dle-xp.ini");
	if (file_type == RDL_FILE)
		FreeTextureHandles();
	}
if (strcmp(descent2_path, strlwr (m_d2Path))) {
	bool	bChangePig = true;
	if (HasCustomTextures() &&
		 (QueryMsg ("Changing the pig file will affect the custom textures\n"
						"in this level because of the change in palette.\n"
						"(Reload the level to fix custom texture appeareance.)\n\n"
						"Are you sure you want to do this?") != IDOK))
		bChangePig = false;
	if (bChangePig) {
		strcpy (descent2_path, m_d2Path);
		WritePrivateProfileString ("DLE-XP", "Descent2Directory", descent2_path, "dle-xp.ini");
		if (file_type == RL2_FILE)
			FreeTextureHandles (false);
		GetMine ()->LoadPalette ();
		theApp.MineView ()->ResetView (true);
		}
	}
if (strcmp (levels_path, strlwr (m_missionsPath))) {
	strcpy (levels_path, m_missionsPath);
	WritePrivateProfileString ("DLE-XP", "levelsDirectory", levels_path, "dle-xp.ini");
	}
if (!bInitApp)
	theApp.MineView ()->DelayRefresh (true);
theApp.MineView ()->m_nViewDist = m_nViewDist;
theApp.MineView ()->SetViewMineFlags (m_mineViewFlags);
theApp.MineView ()->SetViewObjectFlags (m_objViewFlags);
theApp.TextureView ()->SetViewFlags (m_texViewFlags);
depth_perception = m_depthPerceptions [m_iDepthPerception];
theApp.MineView ()->DepthPerception () = depth_perception;
*(theApp.MineView ()->MineCenter ()) = m_nMineCenter;
if (!bInitApp) {
	theApp.MineView ()->DelayRefresh (false);
	theApp.MineView ()->Refresh (false);
	}
angle_rate = m_rotateRates [m_iRotateRate];
move_rate = (long) (m_moveRate * 0x10000L);
bExpertMode = (m_bExpertMode != 0);
theApp.GetMine ()->UseTexColors () = m_bUseTexColors != 0;
if (!bInitApp)
	SaveAppSettings (false);
theApp.m_bSplashScreen = m_bSplashScreen;
theApp.m_undoList.SetMaxSize (m_nMaxUndo);
}

                        /*--------------------------*/

void CPrefsDlg::OnOK (void)
{
UpdateData (TRUE);
m_bNoRefresh = true;
SetAppSettings ();
m_bNoRefresh = false;
Refresh ();
}

                        /*--------------------------*/

void CPrefsDlg::FreeTextureHandles (bool bDeleteModified)
{
::FreeTextureHandles (bDeleteModified);
theApp.TextureView ()->Refresh ();
}

                        /*--------------------------*/

void CPrefsDlg::OnCancel (void)
{
GetAppSettings ();
UpdateData (FALSE);
}

                        /*--------------------------*/

void CPrefsDlg::OnOpenD1PIG (void)
{
if (BrowseFile ("Descent 1 PIG", m_d1Path, "*.pig", TRUE)) {
	UpdateData (FALSE);
	OnOK ();
	}
}

                        /*--------------------------*/

void CPrefsDlg::OnOpenD2PIG (void)
{
if (BrowseFile ("Descent 2 PIG", m_d2Path, "*.pig", TRUE)) {
	UpdateData (FALSE);
	OnOK ();
	}
}

                        /*--------------------------*/

void CPrefsDlg::OnOpenMissions (void)
{
if (BrowseFile ("Descent mission file", m_missionsPath, "*.hog", TRUE)) {
	UpdateData (FALSE);
	OnOK ();
	}
}

                        /*--------------------------*/

void CPrefsDlg::OnViewObjectsNone (void)
{
m_objViewFlags = eViewObjectsNone;
theApp.MineView ()->SetViewObjectFlags (m_objViewFlags);
UpdateData (FALSE);
OnOK ();
}

                        /*--------------------------*/

void CPrefsDlg::OnViewObjectsAll (void)
{
m_objViewFlags = eViewObjectsAll;
theApp.MineView ()->SetViewObjectFlags (m_objViewFlags);
UpdateData (FALSE);
OnOK ();
}

                        /*--------------------------*/

void CPrefsDlg::OnViewMineNone (void)
{
m_mineViewFlags &= ~(eViewMineLights | eViewMineSpecial | eViewMineWalls | eViewMineDeltaLights);
theApp.MineView ()->SetViewMineFlags (m_mineViewFlags);
UpdateData (FALSE);
OnOK ();
}

                        /*--------------------------*/

void CPrefsDlg::OnViewMineAll (void)
{
m_mineViewFlags |= (eViewMineLights | eViewMineSpecial | eViewMineWalls | eViewMineDeltaLights);
theApp.MineView ()->SetViewMineFlags (m_mineViewFlags);
UpdateData (FALSE);
OnOK ();
}

                        /*--------------------------*/

void CPrefsDlg::SetLayout (int nLayout)
{
WritePrivateProfileInt ("Layout", nLayout);
}

                        /*--------------------------*/

void CPrefsDlg::OnLayout0 (void)
{
SetLayout (0);
}

                        /*--------------------------*/

void CPrefsDlg::OnLayout1 (void)
{
SetLayout (1);
}

                        /*--------------------------*/

void CPrefsDlg::OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
	int	nPos = pScrollBar->GetScrollPos ();
	CRect	rc;

if (pScrollBar == ViewDistSlider ()) {
	switch (scrollCode) {
		case SB_LINEUP:
			nPos--;
			break;
		case SB_LINEDOWN:
			nPos++;
			break;
		case SB_PAGEUP:
			nPos -= 10;
			break;
		case SB_PAGEDOWN:
			nPos += 10;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nPos = thumbPos;
			break;
		case SB_ENDSCROLL:
			return;
		}
	if (nPos < 0)
		nPos = 0;
	else if (nPos > MAX_VIEWDIST)
		nPos = MAX_VIEWDIST;
	theApp.MineView ()->SetViewDist (m_nViewDist = nPos);
	UpdateData (FALSE);
//	pScrollBar->SetScrollPos (nPos, TRUE);
	}
pScrollBar->SetScrollPos (nPos, TRUE);
}

                        /*--------------------------*/

//eof prefsdlg.cpp