// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <malloc.h>
#include <stdlib.h>
#undef abs
#include <math.h>
#include <mmsystem.h>
#include <stdio.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "dlc.h"
#include "mine.h"
#include "global.h"
#include "toolview.h"

                        /*--------------------------*/

BEGIN_MESSAGE_MAP (CTriggerTool, CTexToolDlg)
	ON_WM_PAINT ()
	ON_WM_HSCROLL ()
	ON_BN_CLICKED (IDC_TRIGGER_ADD, OnAddTrigger)
	ON_BN_CLICKED (IDC_TRIGGER_DELETE, OnDeleteTrigger)
	ON_BN_CLICKED (IDC_TRIGGER_DELETEALL, OnDeleteTriggerAll)
	ON_BN_CLICKED (IDC_TRIGGER_ADDTGT, OnAddTarget)
	ON_BN_CLICKED (IDC_TRIGGER_DELTGT, OnDeleteTarget)
	ON_BN_CLICKED (IDC_TRIGGER_ADDWALLTGT, OnAddWallTarget)
	ON_BN_CLICKED (IDC_TRIGGER_DELWALLTGT, OnDeleteWallTarget)
	ON_BN_CLICKED (IDC_TRIGGER_COPY, OnCopyTrigger)
	ON_BN_CLICKED (IDC_TRIGGER_PASTE, OnPasteTrigger)
	ON_BN_CLICKED (IDC_TRIGGER_STANDARD, OnStandardTrigger)
	ON_BN_CLICKED (IDC_TRIGGER_OBJECT, OnObjectTrigger)
	ON_BN_CLICKED (IDC_TRIGGER_NOMESSAGE, OnD2Flag1)
	ON_BN_CLICKED (IDC_TRIGGER_ONESHOT, OnD2Flag2)
	ON_BN_CLICKED (IDC_TRIGGER_PERMANENT, OnD2Flag3)
	ON_BN_CLICKED (IDC_TRIGGER_ALTERNATE, OnD2Flag4)
	ON_BN_CLICKED (IDC_TRIGGER_SET_ORIENT, OnD2Flag5)
	ON_BN_CLICKED (IDC_TRIGGER_CONTROL, OnD1Flag1)
	ON_BN_CLICKED (IDC_TRIGGER_SHIELDDRAIN, OnD1Flag2)
	ON_BN_CLICKED (IDC_TRIGGER_ENERGYDRAIN, OnD1Flag3)
	ON_BN_CLICKED (IDC_TRIGGER_ENDLEVEL, OnD1Flag4)
	ON_BN_CLICKED (IDC_TRIGGER_ACTIVE, OnD1Flag5)
	ON_BN_CLICKED (IDC_TRIGGER_ONESHOTD1, OnD1Flag6)
	ON_BN_CLICKED (IDC_TRIGGER_ROBOTMAKER, OnD1Flag7)
	ON_BN_CLICKED (IDC_TRIGGER_ILLUSIONOFF, OnD1Flag8)
	ON_BN_CLICKED (IDC_TRIGGER_SECRETEXIT, OnD1Flag9)
	ON_BN_CLICKED (IDC_TRIGGER_ILLUSIONON, OnD1Flag10)
	ON_BN_CLICKED (IDC_TRIGGER_ADD_OPENDOOR, OnAddOpenDoor)
	ON_BN_CLICKED (IDC_TRIGGER_ADD_ROBOTMAKER, OnAddRobotMaker)
	ON_BN_CLICKED (IDC_TRIGGER_ADD_SHIELDDRAIN, OnAddShieldDrain)
	ON_BN_CLICKED (IDC_TRIGGER_ADD_ENERGYDRAIN, OnAddEnergyDrain)
	ON_BN_CLICKED (IDC_TRIGGER_ADD_CONTROLPANEL, OnAddControlPanel)
	ON_CBN_SELCHANGE (IDC_TRIGGER_TRIGGERNO, OnSetTrigger)
	ON_CBN_SELCHANGE (IDC_TRIGGER_D2TYPE, OnSetType)
	ON_CBN_SELCHANGE (IDC_TRIGGER_TARGETLIST, OnSetTarget)
	ON_CBN_SELCHANGE (IDC_TRIGGER_TEXTURE1, OnSelect1st)
	ON_CBN_SELCHANGE (IDC_TRIGGER_TEXTURE2, OnSelect2nd)
	ON_EN_KILLFOCUS (IDC_TRIGGER_STRENGTH, OnStrength)
	ON_EN_KILLFOCUS (IDC_TRIGGER_TIME, OnTime)
	ON_EN_UPDATE (IDC_TRIGGER_STRENGTH, OnStrength)
	ON_EN_UPDATE (IDC_TRIGGER_TIME, OnTime)
END_MESSAGE_MAP ()

//------------------------------------------------------------------------
// DIALOG - CTriggerTool (constructor)
//------------------------------------------------------------------------

CTriggerTool::CTriggerTool (CPropertySheet *pParent)
	: CTexToolDlg (nLayout ? IDD_TRIGGERDATA2 : IDD_TRIGGERDATA, pParent, IDC_TRIGGER_SHOW, 6, RGB (0,0,0), true)
{
Reset ();
}

								/*--------------------------*/

CTriggerTool::~CTriggerTool ()
{
if (m_bInited) {
	m_showObjWnd.DestroyWindow ();
	m_showTexWnd.DestroyWindow ();
	}
}

                        /*--------------------------*/

void CTriggerTool::LoadTextureListBoxes () 
{
	HINSTANCE	hInst = AfxGetApp()->m_hInstance;
	char			name [80];
	int			nTextures, iTexture, index;
	CComboBox	*cbTexture1 = CBTexture1 ();
	CComboBox	*cbTexture2 = CBTexture2 ();

GetMine ();

INT16 texture1 = Texture1 ();
INT16 texture2 = Texture2 ();

if ((texture1 < 0) || (texture1 >= MAX_TEXTURES))
	texture1 = 0;
if ((texture2 < 0) || (texture2 >= MAX_TEXTURES))
	texture2 = 0;

cbTexture1->ResetContent ();
cbTexture2->ResetContent ();
index = cbTexture1->AddString ("(none)");
texture_resource = (file_type == RDL_FILE) ? D1_TEXTURE_STRING_TABLE : D2_TEXTURE_STRING_TABLE;
nTextures = (file_type == RDL_FILE) ? MAX_D1_TEXTURES : MAX_D2_TEXTURES;
for (iTexture = 0; iTexture < nTextures; iTexture++) {
#if 0
	if (iTexture >= 910)
		sprintf (name, "xtra #%d", iTexture);
	else
#endif
		LoadString (hInst, texture_resource + iTexture, name, sizeof (name));
	if (!strstr ((char *) name, "frame")) {
		index = cbTexture1->AddString (name);
		cbTexture1->SetItemData (index, iTexture);
		if (texture1 == iTexture)
			cbTexture1->SetCurSel (index);
		index = cbTexture2->AddString (iTexture ? name : "(none)");
		if (texture2 == iTexture)
			cbTexture2->SetCurSel (index);
		cbTexture2->SetItemData (index, iTexture);
		}
	}
}

								/*--------------------------*/

void CTriggerTool::Reset ()
{
m_nTrigger = 
m_nStdTrigger = 
m_nObjTrigger = -1;
m_nClass = 0;
m_nType = 0;
m_nStrength = 0;
m_nTime = 0;
m_bAutoAddWall = 1;
m_nTargets = 0;
m_iTarget = -1;
m_nSliderValue = 10;
m_bFindTrigger = true;
memset (m_bD1Flags, 0, sizeof (m_bD1Flags));
memset (m_bD2Flags, 0, sizeof (m_bD2Flags));
*m_szTarget = '\0';
}

                        /*--------------------------*/

typedef struct tTriggerData {
	char	*pszName;
	int	nType;
} tTriggerData;

static tTriggerData triggerData [] = {
	{"open door", TT_OPEN_DOOR},
	{"close door", TT_CLOSE_DOOR},
	{"make robots", TT_MATCEN},
	{"exit", TT_EXIT},
	{"secret exit", TT_SECRET_EXIT},
	{"illusion off", TT_ILLUSION_OFF},
	{"illusion on", TT_ILLUSION_ON},
	{"unlock door", TT_UNLOCK_DOOR},
	{"lock door", TT_LOCK_DOOR},
	{"open wall", TT_OPEN_WALL},
	{"close wall", TT_CLOSE_WALL},
	{"illusory wall", TT_ILLUSORY_WALL},
	{"light off", TT_LIGHT_OFF},
	{"light on", TT_LIGHT_ON},
	{"teleport", TT_TELEPORT},
	{"speed boost", TT_SPEEDBOOST},
	{"camera", TT_CAMERA},
	{"damage shields", TT_SHIELD_DAMAGE_D2},
	{"drain energy", TT_ENERGY_DRAIN_D2},
	{"change texture", TT_CHANGE_TEXTURE},
	{"countdown", TT_COUNTDOWN},
	{"spawn bot", TT_SPAWN_BOT},
	{"set spawn", TT_SET_SPAWN},
	{"message", TT_MESSAGE},
	{"sound", TT_SOUND},
	{"master", TT_MASTER}
	};


BOOL CTriggerTool::OnInitDialog ()
{
CTexToolDlg::OnInitDialog ();
CreateImgWnd (&m_showObjWnd, IDC_TRIGGER_SHOW_OBJ);
CreateImgWnd (&m_showTexWnd, IDC_TRIGGER_SHOW_TEXTURE);
CComboBox *pcb = CBType ();
pcb->ResetContent();
if (file_type != RDL_FILE) {
	int h, j = sizeof (triggerData) / sizeof (tTriggerData);
	for (int i = 0; i < j; i++) {
		h = pcb->AddString (triggerData [i].pszName);
		pcb->SetItemData (h, triggerData [i].nType);
		}
	}
else
	pcb->AddString ("n/a");
pcb->SetCurSel (0);
InitSlider (IDC_TRIGGER_SLIDER, 1, 10);
for (int i = 1; i <= 10; i++)
	SlCtrl (IDC_TRIGGER_SLIDER)->SetTic (i);
LoadTextureListBoxes ();
m_bInited = TRUE;
return TRUE;
}

								/*--------------------------*/

bool CTriggerTool::TriggerHasSlider (void)
{
return 
	(m_nType == TT_SPEEDBOOST) || 
	(m_nType == TT_TELEPORT) ||
	(m_nType == TT_SPAWN_BOT);
}

								/*--------------------------*/

void CTriggerTool::DoDataExchange (CDataExchange *pDX)
{
	static char *pszSmokeParams [] = {"life", "speed", "density", "volume", "drift", "", "", "brightness"};

if (!m_bInited)
	return;
DDX_CBIndex (pDX, IDC_TRIGGER_TRIGGERNO, m_nTrigger);
DDX_CBIndex (pDX, IDC_TRIGGER_D2TYPE, m_nType);
if (pDX->m_bSaveAndValidate)
	m_nType = CBType ()->GetItemData (CBType ()->GetCurSel ());
else
	SelectItemData (CBType (), m_nType);
if (TriggerHasSlider () || (m_nType == TT_SHIELD_DAMAGE_D2) || (m_nType == TT_ENERGY_DRAIN_D2))
	DDX_Double (pDX, IDC_TRIGGER_STRENGTH, m_nStrength, -100, 100, "%3.1f");
else if ((m_nType == TT_MESSAGE) || (m_nType == TT_SOUND))
	DDX_Double (pDX, IDC_TRIGGER_STRENGTH, m_nStrength, 0, 1000, "%1.0f");
DDX_Text (pDX, IDC_TRIGGER_TIME, m_nTime);
for (int i = 0; i < 2; i++)
	DDX_Check (pDX, IDC_TRIGGER_NOMESSAGE + i, m_bD2Flags [i]);
for (i = 2; i < 5; i++)
	DDX_Check (pDX, IDC_TRIGGER_NOMESSAGE + i, m_bD2Flags [i + 1]);
for (i = 0; i < 10; i++)
	DDX_Check (pDX, IDC_TRIGGER_CONTROL + i, m_bD1Flags [i]);
DDX_Text (pDX, IDC_TRIGGER_TARGET, m_szTarget, sizeof (m_szTarget));
DDX_Check (pDX, IDC_TRIGGER_AUTOADDWALL, m_bAutoAddWall);
char szLabel [40];
if (m_nType == TT_SPEEDBOOST) {
	DDX_Slider (pDX, IDC_TRIGGER_SLIDER, m_nSliderValue);
	sprintf (szLabel, "boost: %d%c", m_nSliderValue * 10, '%');
	}
else if ((m_nType == TT_TELEPORT) || (m_nType == TT_SPAWN_BOT)) {
	DDX_Slider (pDX, IDC_TRIGGER_SLIDER, m_nSliderValue);
	sprintf (szLabel, "damage: %d%c", m_nSliderValue * 10, '%');
	}
else
	strcpy (szLabel, "n/a");
SetDlgItemText (IDC_TRIGGER_SLIDER_TEXT, szLabel);
DDX_Radio (pDX, IDC_TRIGGER_STANDARD, m_nClass);
if (m_nType == TT_MESSAGE) {
	strcpy (szLabel, "msg #");
	SetDlgItemText (IDC_TRIGGER_STRENGTH_TEXT, szLabel);
	SetDlgItemText (IDC_TRIGGER_STRENGTH_TEXT2, "");
	}
else if (m_nType == TT_SOUND) {
	strcpy (szLabel, "sound #");
	SetDlgItemText (IDC_TRIGGER_STRENGTH_TEXT, szLabel);
	SetDlgItemText (IDC_TRIGGER_STRENGTH_TEXT2, "");
	}
else {
	strcpy (szLabel, "strength:");
	SetDlgItemText (IDC_TRIGGER_STRENGTH_TEXT, szLabel);
	SetDlgItemText (IDC_TRIGGER_STRENGTH_TEXT2, "%");
	}
}

								/*--------------------------*/

BOOL CTriggerTool::OnSetActive ()
{
Refresh ();
return TRUE; //CTexToolDlg::OnSetActive ();
}

								/*--------------------------*/

BOOL CTriggerTool::OnKillActive ()
{
Refresh ();
return CTexToolDlg::OnKillActive ();
}

								/*--------------------------*/

void CTriggerTool::EnableControls (BOOL bEnable)
{
CToolDlg::EnableControls (IDC_TRIGGER_STANDARD, IDC_TRIGGER_OBJECT, level_version >= 12);
CToolDlg::EnableControls (IDC_TRIGGER_SHOW_OBJ, IDC_TRIGGER_SHOW_OBJ, level_version >= 12);
CToolDlg::EnableControls (IDC_TRIGGER_TRIGGERNO + 1, IDC_TRIGGER_ADD_CONTROLPANEL, bEnable);
CToolDlg::EnableControls (IDC_TRIGGER_SLIDER, IDC_TRIGGER_SLIDER, bEnable && TriggerHasSlider ());
CToolDlg::EnableControls (IDC_TRIGGER_STRENGTH, IDC_TRIGGER_STRENGTH, bEnable && (m_nType != TT_SPEEDBOOST) && (m_nType != TT_CHANGE_TEXTURE));
CToolDlg::EnableControls (IDC_TRIGGER_SHOW_TEXTURE, IDC_TRIGGER_TEXTURE2, bEnable && (m_nType == TT_CHANGE_TEXTURE));
//for (int i = IDC_TRIGGER_TRIGGER_NO; i <= IDC_TRIGGER_PASTE; i++)
//	GetDlgItem (i)->EnableWindow (bEnable);
}

								/*--------------------------*/

int CTriggerTool::NumTriggers ()
{
return m_nClass ? m_mine->NumObjTriggers () : m_mine->GameInfo ().triggers.count;
}

								/*--------------------------*/

void CTriggerTool::InitCBTriggerNo ()
{
if (!GetMine ())
	return;
CComboBox *pcb = CBTriggerNo ();
pcb->ResetContent ();
int i, j = NumTriggers ();
for (i = 0; i < j; i++)
	pcb->AddString (itoa (i, message, 10));
pcb->SetCurSel (m_nTrigger);
}

								/*--------------------------*/

void CTriggerTool::InitLBTargets ()
{
if (!GetMine ())
	return;
CListBox *plb = LBTargets ();
m_iTarget = plb->GetCurSel ();
plb->ResetContent ();
if (m_pTrigger) {
	m_nTargets = m_pTrigger->num_links;
	for (int i = 0; i < m_nTargets ; i++) {
		sprintf (m_szTarget, "   %d, %d", m_pTrigger->seg [i], m_pTrigger->side [i] + 1);
		plb->AddString (m_szTarget);
		}
	if ((m_iTarget < 0) || (m_iTarget >= m_nTargets))
		m_iTarget = 0;
	*m_szTarget = '\0';
	}
else
	m_nTargets =
	m_iTarget = 0;
plb->SetCurSel (m_iTarget);
}

								/*--------------------------*/

void CTriggerTool::SetTriggerPtr (void)
{
if (m_nTrigger == -1) {
	m_pObjTrigger = NULL;
	m_pTrigger = NULL;
	m_nStdTrigger = 
	m_nObjTrigger = -1;
	ClearObjWindow ();
	}	
else if (m_nClass) {
	m_pObjTrigger = m_mine->ObjTriggerList (m_nTrigger);
	m_pTrigger = m_mine->ObjTriggers (m_nTrigger);
	DrawObjectImage ();
	}
else {
	m_pTrigger = m_mine->Triggers (m_nTrigger);
	ClearObjWindow ();
	}
}

//------------------------------------------------------------------------

void CTriggerTool::ClearObjWindow (void)
{
CDC *pDC = m_showObjWnd.GetDC ();
if (pDC) {
	CRect rc;
	m_showObjWnd.GetClientRect (rc);
	pDC->FillSolidRect (&rc, IMG_BKCOLOR);
	m_showObjWnd.ReleaseDC (pDC);
	}
}

//------------------------------------------------------------------------

void CTriggerTool::DrawObjectImage ()
{
if (m_nClass) {
	CDObject *obj = m_mine->CurrObj ();
	if ((obj->type == OBJ_ROBOT) || (obj->type == OBJ_CAMBOT) || (obj->type == OBJ_MONSTERBALL) || (obj->type == OBJ_SMOKE))
		m_mine->DrawObject (&m_showObjWnd, obj->type, obj->id);
	}
}

//------------------------------------------------------------------------

void CTriggerTool::OnPaint ()
{
CToolDlg::OnPaint ();
DrawObjectImage ();
}

//------------------------------------------------------------------------

bool CTriggerTool::FindTrigger (INT16 &trignum)
{
if (!m_bFindTrigger)
	trignum = m_nTrigger;
else {
	if (m_nClass) {
		if ((m_nTrigger != -1) && 
			 (m_mine->Current ()->object == m_mine->ObjTriggerList (m_nTrigger)->objnum))
			return false;
		// use current object's first trigger
		INT16 objnum = m_mine->FindTriggerObject (&trignum);
		m_nTrigger = (trignum == NO_TRIGGER) ? -1 : trignum;
		if ((m_nTrigger == -1) || (objnum < 0))
			return false;
		}
	else {
		// use current side's trigger
		UINT16 wallnum = m_mine->FindTriggerWall (&trignum);
		m_nTrigger = (trignum == NO_TRIGGER) ? -1 : trignum;
		// if found, proceed
		if ((m_nTrigger == -1) || (wallnum >= m_mine->GameInfo ().walls.count))
			return false;
		}
	}
return true;
}

//------------------------------------------------------------------------
// CTriggerTool - RefreshData
//------------------------------------------------------------------------

void CTriggerTool::Refresh ()
{
if (!m_bInited)
	return;
if (!GetMine ())
	return;

	int			i;
	INT16			trignum;
	CComboBox	*cbTexture1 = CBTexture1 ();
	CComboBox	*cbTexture2 = CBTexture2 ();
	CDSide		*side;

FindTrigger (trignum);
InitCBTriggerNo ();
if (m_nClass || (m_nTrigger == -1)) {
	SetTriggerPtr ();
	EnableControls (FALSE);
	CToolDlg::EnableControls (IDC_TRIGGER_STANDARD, IDC_TRIGGER_OBJECT, TRUE);
	CToolDlg::EnableControls (IDC_TRIGGER_ADD_OPENDOOR, IDC_TRIGGER_ADD_CONTROLPANEL, TRUE);
	if (file_type != RDL_FILE)
		CToolDlg::EnableControls (IDC_TRIGGER_ADD_SHIELDDRAIN, IDC_TRIGGER_ADD_ENERGYDRAIN, FALSE);
	GetDlgItem (IDC_TRIGGER_ADD)->EnableWindow (TRUE);
	GetDlgItem (IDC_TRIGGER_D2TYPE)->EnableWindow (TRUE);
	InitLBTargets ();
	ClearObjWindow ();
	}
if (m_nTrigger != -1) {
	SetTriggerPtr ();
	m_nType = m_pTrigger->type;
	if (m_nType != TT_CHANGE_TEXTURE) {
		cbTexture1->SetCurSel (cbTexture1->SelectString (-1, "(none)"));  // unselect if string not found
		cbTexture2->SetCurSel (cbTexture2->SelectString (-1, "(none)"));  // unselect if string not found
		}
	else {
		LoadString (hInst, texture_resource + Texture1 (), message, sizeof (message));
		cbTexture1->SetCurSel (cbTexture1->SelectString (-1, message));  // unselect if string not found
		if (Texture2 ()) {
			LoadString (hInst, texture_resource + Texture2 (), message, sizeof (message));
			cbTexture2->SetCurSel (cbTexture2->SelectString (-1, message));  // unselect if string not found
			}
		else
			cbTexture2->SetCurSel (cbTexture2->SelectString (-1, "(none)"));  // unselect if string not found
		}

	EnableControls (TRUE);
	if (!m_nClass)
		GetDlgItem (IDC_TRIGGER_ADD)->EnableWindow (FALSE);
	m_nTime = m_pTrigger->time;
	m_nTargets = m_pTrigger->num_links;
	InitLBTargets ();
	//TriggerCubeSideList ();
	// if D2 file, use trigger.type
	if (file_type != RDL_FILE) {
		SelectItemData (CBType (), m_nType);
		CToolDlg::EnableControls (IDC_TRIGGER_CONTROL, IDC_TRIGGER_ILLUSIONON, FALSE);
		CToolDlg::EnableControls (IDC_TRIGGER_ADD_SHIELDDRAIN, IDC_TRIGGER_ADD_ENERGYDRAIN, FALSE);
		m_bD2Flags [0] = ((m_pTrigger->flags & TF_NO_MESSAGE) != 0);
		m_bD2Flags [1] = ((m_pTrigger->flags & TF_ONE_SHOT) != 0);
		m_bD2Flags [2] = 0;
		m_bD2Flags [3] = ((m_pTrigger->flags & TF_PERMANENT) != 0);
		m_bD2Flags [4] = ((m_pTrigger->flags & TF_ALTERNATE) != 0);
		m_bD2Flags [5] = ((m_pTrigger->flags & TF_SET_ORIENT) != 0);
		if (m_nType == TT_SPEEDBOOST)
			m_nSliderValue = m_pTrigger->value;
		if (m_nType == TT_TELEPORT)
			m_nSliderValue = m_pTrigger->value;
		if (m_nType == TT_SPAWN_BOT)
			m_nSliderValue = m_pTrigger->value;
		else if (m_nType != TT_CHANGE_TEXTURE)
			m_nStrength = (double) m_pTrigger->value / F1_0;
		}
	else {
		CBType ()->EnableWindow (FALSE);
		CToolDlg::EnableControls (IDC_TRIGGER_NOMESSAGE, IDC_TRIGGER_ONESHOT, FALSE);
		for (i = 0; i < MAX_TRIGGER_FLAGS; i++)
			m_bD1Flags [i] = ((m_pTrigger->flags & (1 << i)) != 0);
		m_nStrength = (double) m_pTrigger->value / F1_0;
		}
	OnSetTarget ();
	}
CToolDlg::EnableControls (IDC_TRIGGER_TRIGGERNO, IDC_TRIGGER_TRIGGERNO, NumTriggers () > 0);
CToolDlg::EnableControls (IDC_TRIGGER_DELETEALL, IDC_TRIGGER_DELETEALL, NumTriggers () > 0);
side = m_mine->OtherSide ();
CTexToolDlg::Refresh (side->nBaseTex, side->nOvlTex, 1);
if ((m_nTrigger >= 0) && (m_nType == TT_CHANGE_TEXTURE))
	PaintTexture (&m_showTexWnd, RGB (128,128,128), -1, -1, Texture1 (), Texture2 ());
else
	PaintTexture (&m_showTexWnd, RGB (128,128,128), -1, -1, MAX_TEXTURES);
UpdateData (FALSE);
}

//------------------------------------------------------------------------

void CTriggerTool::OnStandardTrigger (void)
{
m_nObjTrigger = m_nTrigger;
m_nClass = 0;
UpdateData (FALSE);
m_pTrigger = m_pStdTrigger;
m_nTrigger = m_nStdTrigger;
Refresh ();
}

//------------------------------------------------------------------------

void CTriggerTool::OnObjectTrigger (void)
{
if (!GetMine ())
	return;
m_nStdTrigger = m_nTrigger;
m_pStdTrigger = m_pTrigger;
m_nClass = 1;
UpdateData (FALSE);
m_pTrigger = m_pObjTrigger ? m_mine->ObjTriggers (m_nTrigger) : NULL;
m_nTrigger = m_nObjTrigger;
Refresh ();
}

//------------------------------------------------------------------------
// CTriggerTool - Add Trigger
//------------------------------------------------------------------------

void CTriggerTool::OnAddTrigger ()
{
if (!GetMine ())
	return;

//m_nTrigger = trignum;
m_bAutoAddWall = ((CButton *) GetDlgItem (IDC_TRIGGER_AUTOADDWALL))->GetCheck ();
if (m_nClass) {
	m_pObjTrigger = m_mine->AddObjTrigger (-1, m_nType);
	m_pTrigger = m_pObjTrigger ? m_mine->ObjTriggers (m_mine->NumObjTriggers () - 1) : NULL;
	m_nTrigger = m_pObjTrigger ? m_pObjTrigger - m_mine->ObjTriggerList () : -1; 
	}
else {
	m_pObjTrigger = NULL;
	m_pTrigger = m_mine->AddTrigger (-1, m_nType, (BOOL) m_bAutoAddWall /*TT_OPEN_DOOR*/);
	m_nTrigger = m_pTrigger ? m_pTrigger - m_mine->Triggers () : -1;
	}
// Redraw trigger window
Refresh ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CTriggerTool - Delete Trigger
//------------------------------------------------------------------------

void CTriggerTool::OnDeleteTrigger () 
{
if (!GetMine ())
	return;
// check to see if trigger already exists on wall
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nClass)
	m_mine->DeleteObjTrigger (m_nTrigger);
else
	m_mine->DeleteTrigger (m_nTrigger);
// Redraw trigger window
Refresh ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CTriggerTool - Delete All (Marked) Triggers
//------------------------------------------------------------------------

void CTriggerTool::OnDeleteTriggerAll () 
{
if (!GetMine ())
	return;
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
theApp.MineView ()->DelayRefresh (true);
CDSegment *seg = m_mine->Segments ();
CDSide *side;
bool bAll = (m_mine->MarkedSegmentCount (true) == 0);
int nDeleted = 0;
for (int i = m_mine->SegCount (); i; i--, seg++) {
	side = seg->sides;
	for (int j = 0; j < MAX_SIDES_PER_SEGMENT; j++, side++) {
		if (side->nWall >= MAX_WALLS)
			continue;
		CDWall *wall = m_mine->Walls (side->nWall);
		if (wall->trigger >= NumTriggers ())
			continue;
		if (bAll || m_mine->SideIsMarked (i, j)) {
			m_mine->DeleteTrigger (wall->trigger);
			nDeleted++;
			}
		}
	}
theApp.MineView ()->DelayRefresh (false);
if (nDeleted) {
	theApp.UnlockUndo ();
	theApp.MineView ()->Refresh ();
	Refresh ();
	}
else
	theApp.ResetModified (bUndo);
}

//------------------------------------------------------------------------
// CTriggerTool - TrigNumberMsg
//------------------------------------------------------------------------

void CTriggerTool::OnSetTrigger ()
{
if (!GetMine ())
	return;

UINT16 wallnum;
CDWall *wall;

// find first wall with this trigger
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if ((m_nTrigger == -1) || (m_nTrigger >= NumTriggers ()))
	return;
if (m_nClass) {
	m_mine->Current ()->object = m_mine->ObjTriggerList (m_nTrigger)->objnum;
	}
else {
	for (wallnum = 0, wall = m_mine->Walls (); wallnum < m_mine->GameInfo ().walls.count; wallnum++, wall++)
		if (wall->trigger == m_nTrigger)
			break;
	if (wallnum >= m_mine->GameInfo ().walls.count) {
		EnableControls (FALSE);
		GetDlgItem (IDC_TRIGGER_DELETE)->EnableWindow (TRUE);
		return;
		}
	if ((wall->segnum >= m_mine->SegCount ()) || (wall->segnum < 0) || 
		 (wall->sidenum < 0) || (wall->sidenum > 5)) {
		EnableControls (FALSE);
		GetDlgItem (IDC_TRIGGER_DELETE)->EnableWindow (TRUE);
		return;
		}
	if ((m_mine->Current ()->segment != wall->segnum) ||
		 (m_mine->Current ()->side != wall->sidenum)) {
		m_mine->SetCurrent (wall->segnum, wall->sidenum);
		}
	}
SetTriggerPtr ();
m_bFindTrigger = false;
Refresh ();
m_bFindTrigger = true;
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CTriggerTool - TrigTypeMsg
//------------------------------------------------------------------------

void CTriggerTool::OnSetType ()
{
if (!GetMine ())
	return;
int nType = CBType ()->GetItemData (CBType ()->GetCurSel ());
if ((nType == TT_SMOKE_BRIGHTNESS) || ((nType >= TT_SMOKE_LIFE) && (nType <= TT_SMOKE_DRIFT))) {
	ErrorMsg ("This trigger type is not supported any more.\nYou can use the effects tool to edit smoke emitters.");
	return;
	}
if ((nType >= TT_TELEPORT) && (level_version < 9)) {
	SelectItemData (CBType (), m_nType);
	return;
	}
m_nType = nType;
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nTrigger == -1)
	return;
SetTriggerPtr ();
theApp.SetModified (TRUE);
m_pTrigger->type = m_nType;
Refresh ();
}

//------------------------------------------------------------------------
// CTriggerTool - TrigValueMsg
//------------------------------------------------------------------------

void CTriggerTool::OnStrength () 
{
if (!GetMine ())
	return;
UpdateData (TRUE);
if ((m_nTrigger == -1) || (m_nType == TT_SPEEDBOOST) || (m_nType == TT_CHANGE_TEXTURE))
	return;
SetTriggerPtr ();
theApp.SetModified (TRUE);
UpdateData (FALSE);
m_pTrigger->value = (INT32) (m_nStrength * F1_0);
}

//------------------------------------------------------------------------
// CTriggerTool - TrigTimeMsg
//------------------------------------------------------------------------

void CTriggerTool::OnTime () 
{
if (!GetMine ())
	return;
UpdateData (TRUE);
if (m_nTrigger == -1)
	return;
SetTriggerPtr ();
theApp.SetModified (TRUE);
m_pTrigger->time = m_nTime;
}

//------------------------------------------------------------------------
// CTriggerTool - TriggerFlags0Msg
//------------------------------------------------------------------------

void CTriggerTool::OnD1Flag (int i)
{
if (!GetMine ())	
	return;
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nTrigger == -1)
	return;
SetTriggerPtr ();
theApp.SetModified (TRUE);
int h = 1 << i;
m_pTrigger->flags ^= h;
m_bD1Flags [i] = ((m_pTrigger->flags & h) != 0);
((CButton *) GetDlgItem (IDC_TRIGGER_CONTROL + i))->SetCheck (m_bD1Flags [i]);
UpdateData (FALSE);
}

                        /*--------------------------*/

void CTriggerTool::OnD2Flag (int i, int j)
{
if (!GetMine ())	
	return;
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nTrigger == -1)
	return;
SetTriggerPtr ();
theApp.SetModified (TRUE);
int h = 1 << i;
m_pTrigger->flags ^= h;
m_bD2Flags [i] = ((m_pTrigger->flags & h) != 0);
((CButton *) GetDlgItem (IDC_TRIGGER_NOMESSAGE + i + j))->SetCheck (m_bD2Flags [i]);
UpdateData (FALSE);
}

//------------------------------------------------------------------------
// CTriggerTool - TriggerFlags1Msg
//------------------------------------------------------------------------

void CTriggerTool::OnD1Flag1 () { OnD1Flag (0); }
void CTriggerTool::OnD1Flag2 () { OnD1Flag (1); }
void CTriggerTool::OnD1Flag3 () { OnD1Flag (2); }
void CTriggerTool::OnD1Flag4 () { OnD1Flag (3); }
void CTriggerTool::OnD1Flag5 () { OnD1Flag (4); }
void CTriggerTool::OnD1Flag6 () { OnD1Flag (5); }
void CTriggerTool::OnD1Flag7 () { OnD1Flag (6); }
void CTriggerTool::OnD1Flag8 () { OnD1Flag (7); }
void CTriggerTool::OnD1Flag9 () { OnD1Flag (8); }
void CTriggerTool::OnD1Flag10 () { OnD1Flag (9); }

void CTriggerTool::OnD2Flag1 () { OnD2Flag (0); }
void CTriggerTool::OnD2Flag2 () { OnD2Flag (1); }
// caution: 4 is TF_DISABLED in Descent 2 - do not use here!
void CTriggerTool::OnD2Flag3 () { OnD2Flag (3, -1); }
void CTriggerTool::OnD2Flag4 () { OnD2Flag (4, -1); }
void CTriggerTool::OnD2Flag5 () { OnD2Flag (5, -1); }

//------------------------------------------------------------------------
// CTriggerTool - Add cube/side to trigger list
//------------------------------------------------------------------------

void CTriggerTool::AddTarget (INT16 segnum, INT16 sidenum) 
{
if (!GetMine ())
	return;
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nTrigger == -1)
	return;
SetTriggerPtr ();
m_nTargets = m_pTrigger->num_links;
if (m_nTargets >= MAX_TRIGGER_TARGETS) {
	DEBUGMSG (" Trigger tool: No more targets possible for this trigger.");
	return;
	}
if (FindTarget (segnum, sidenum) > -1) {
	DEBUGMSG (" Trigger tool: Trigger already has this target.");
	return;
	}
theApp.SetModified (TRUE);
m_pTrigger->seg [m_nTargets] = segnum;
m_pTrigger->side [m_nTargets] = sidenum - 1;
m_pTrigger->num_links++;
sprintf (m_szTarget, "   %d,%d", segnum, sidenum);
LBTargets ()->AddString (m_szTarget);
LBTargets ()->SetCurSel (m_nTargets++);
*m_szTarget = '\0';
Refresh ();
}


                        /*--------------------------*/

void CTriggerTool::OnAddTarget () 
{
if (!GetMine ())
	return;
int segnum, sidenum;
UpdateData (TRUE);
sscanf (m_szTarget, "%d,%d", &segnum, &sidenum);
if ((segnum < 0) || (segnum >= m_mine->SegCount ()) || (sidenum < 1) || (sidenum > 6))
	return;
AddTarget (segnum, sidenum);
}

                        /*--------------------------*/

void CTriggerTool::OnAddWallTarget ()
{
if (!GetMine ())
	return;
CDSelection *other = (m_mine->Current () == &m_mine->Current1 ()) ? &m_mine->Current2 () : &m_mine->Current1 ();
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nTrigger == -1)
	return;
SetTriggerPtr ();
if ((file_type == RDL_FILE) ? 
	 (m_pTrigger->flags & TRIGGER_MATCEN) != 0 : 
	 (m_pTrigger->type == TT_MATCEN) && 
	 (m_mine->Segments (other->segment)->special != SEGMENT_IS_ROBOTMAKER)) {
	DEBUGMSG (" Trigger tool: Target is no robot maker");
	return;
	}
int i = FindTarget (other->segment, other->side);
if (i > -1)
	return;
AddTarget (other->segment, other->side + 1);
}

//------------------------------------------------------------------------
// CTriggerTool - Delete cube/side
//------------------------------------------------------------------------

void CTriggerTool::OnDeleteTarget ()
{
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nTrigger == -1)
	return;
m_iTarget = LBTargets ()->GetCurSel ();
if ((m_iTarget < 0) || (m_iTarget >= MAX_TRIGGER_TARGETS))
	return;
theApp.SetModified (TRUE);
SetTriggerPtr ();
m_nTargets = --(m_pTrigger->num_links);
m_pTrigger->seg [m_iTarget] = 0;
m_pTrigger->side [m_iTarget] = 0;
if (m_iTarget < m_nTargets) {
	memcpy (m_pTrigger->seg + m_iTarget, m_pTrigger->seg + m_iTarget + 1, (m_nTargets - m_iTarget) * sizeof (*(m_pTrigger->seg)));
	memcpy (m_pTrigger->side + m_iTarget, m_pTrigger->side + m_iTarget + 1, (m_nTargets - m_iTarget) * sizeof (*(m_pTrigger->side)));
	}
LBTargets ()->DeleteString (m_iTarget);
if (m_iTarget >= LBTargets ()->GetCount ())
	m_iTarget--;
LBTargets ()->SetCurSel (m_iTarget);
Refresh ();
}

                        /*--------------------------*/

int CTriggerTool::FindTarget (INT16 segnum, INT16 sidenum)
{
for (int i = 0; i < m_pTrigger->num_links; i++)
	if ((segnum == m_pTrigger->seg [i]) && (sidenum == m_pTrigger->seg [i]))
		return i;
return -1;
}

                        /*--------------------------*/

void CTriggerTool::OnDeleteWallTarget ()
{
if (!GetMine ())
	return;
CDSelection *other = (m_mine->Current () == &m_mine->Current1 ()) ? &m_mine->Current2 () : &m_mine->Current1 ();
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nTrigger == -1)
	return;
SetTriggerPtr ();
int i = FindTarget (other->segment, other->side);
if (i < 0) {
	DEBUGMSG (" Trigger tool: Trigger doesn't target other cube's current side.");
	return;
	}
LBTargets ()->SetCurSel (i);
OnDeleteTarget ();
}

//------------------------------------------------------------------------
// CTriggerTool - Cube/Side list box message
//
// sets "other cube" to selected item
//------------------------------------------------------------------------

void CTriggerTool::OnSetTarget () 
{
if (!GetMine ())
	return;
INT16 trignum;
if (!FindTrigger (trignum))
	return;
SetTriggerPtr ();
// get affected cube/side list box index
m_iTarget = LBTargets ()->GetCurSel ();
// if selected and within range, then set "other" cube/side
if ((m_iTarget < 0) || (m_iTarget >= MAX_TRIGGER_TARGETS) || (m_iTarget >= m_pTrigger->num_links))
	return;
INT16 segnum = m_pTrigger->seg [m_iTarget];
if ((segnum < 0) || (segnum >= m_mine->SegCount ()))
	 return;
INT16 sidenum = m_pTrigger->side [m_iTarget];
if ((sidenum < 0) || (sidenum > 5))
	return;

CDSelection *other = m_mine->Other ();
if ((m_mine->Current ()->segment == segnum) && (m_mine->Current ()->side == sidenum))
	return;
other->segment = m_pTrigger->seg [m_iTarget];
other->side = m_pTrigger->side [m_iTarget];
theApp.MineView ()->Refresh ();
}

                        /*--------------------------*/

void CTriggerTool::OnCopyTrigger ()
{
if (!GetMine ())
	return;
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nTrigger == -1)
	return;
m_defTrigger = m_mine->Triggers () [m_nTrigger];
}

                        /*--------------------------*/

void CTriggerTool::OnPasteTrigger ()
{
if (!GetMine ())
	return;
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nTrigger == -1)
	return;
theApp.SetModified (TRUE);
m_mine->Triggers () [m_nTrigger] = m_defTrigger;
Refresh ();
theApp.MineView ()->Refresh ();
}

                        /*--------------------------*/

afx_msg void CTriggerTool::OnAddOpenDoor ()
{
if (!GetMine ())
	return;
m_mine->AddOpenDoorTrigger ();
}

                        /*--------------------------*/

afx_msg void CTriggerTool::OnAddRobotMaker ()
{
if (!GetMine ())
	return;
m_mine->AddRobotMakerTrigger ();
}

                        /*--------------------------*/

afx_msg void CTriggerTool::OnAddShieldDrain ()
{
if (!GetMine ())
	return;
m_mine->AddShieldTrigger ();
}

                        /*--------------------------*/

afx_msg void CTriggerTool::OnAddEnergyDrain ()
{
if (!GetMine ())
	return;
m_mine->AddEnergyTrigger ();
}

                        /*--------------------------*/

afx_msg void CTriggerTool::OnAddControlPanel ()
{
if (!GetMine ())
	return;
m_mine->AddUnlockTrigger ();
}

                        /*--------------------------*/

BOOL CTriggerTool::TextureIsVisible ()
{
return !m_nClass && (m_pTrigger != NULL) && (m_iTarget >= 0) && (m_iTarget < m_pTrigger->num_links);
}

                        /*--------------------------*/

void CTriggerTool::SelectTexture (int nIdC, bool bFirst)
{
if (!GetMine ())
	return;

	CDSide		*side = m_mine->CurrSide ();
	CComboBox	*pcb = bFirst ? CBTexture1 () : CBTexture2 ();
	int			index = pcb->GetCurSel ();
	
if (index <= 0)
	SetTexture (0, 0);
else {
	INT16 texture = (INT16) pcb->GetItemData (index);
	if (bFirst)
		SetTexture (texture, -1);
	else
		SetTexture (-1, texture);
	}
Refresh ();
}

                        /*--------------------------*/

void CTriggerTool::OnSelect1st () 
{
SelectTexture (IDC_TRIGGER_TEXTURE1, true);
}

                        /*--------------------------*/

void CTriggerTool::OnSelect2nd () 
{
SelectTexture (IDC_TRIGGER_TEXTURE2, false);
}

                        /*--------------------------*/

void CTriggerTool::OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
	int	nPos = pScrollBar->GetScrollPos ();
	CRect	rc;

if (!m_pTrigger || !TriggerHasSlider ())
	return;
if (pScrollBar == SpeedBoostSlider ()) {
	switch (scrollCode) {
		case SB_LINEUP:
			nPos--;
			break;
		case SB_LINEDOWN:
			nPos++;
			break;
		case SB_PAGEUP:
			nPos -= 1;
			break;
		case SB_PAGEDOWN:
			nPos += 1;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nPos = thumbPos;
			break;
		case SB_ENDSCROLL:
			return;
		}
	if (nPos < 1)
		nPos = 1;
	else if (nPos > 10)
		nPos = 10;
	m_nSliderValue = m_pTrigger->value = nPos;
	UpdateData (FALSE);
#if 0
	pScrollBar->SetScrollPos (nPos, TRUE);
	if (!(nPos = pScrollBar->GetScrollPos ())) {
		int	h, i, j;
		pScrollBar->GetScrollRange (&i, &j);
		h = i;
		h = j;
		}
#endif
	}
}

                        /*--------------------------*/


//eof triggerdlg.cpp