// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include "afxpriv.h"
#include <malloc.h>
#include <stdlib.h>
#undef abs
#include <math.h>
#include <mmsystem.h>
#include <stdio.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "mine.h"
#include "dlc.h"
#include "global.h"
#include "toolview.h"
#include "textures.h"

typedef struct tSliderData {
	int	nId;
	int	nMin;
	int	nMax;
	long	nFactor;
	char	**pszLabels;
} tSliderData;

// list box tables
int model_num_list [N_D2_ROBOT_TYPES] = {
  0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x0f, 0x11,
  0x13, 0x14, 0x15, 0x16, 0x17, 0x19, 0x1a, 0x1c, 0x1d, 0x1f,
  0x21, 0x23, 0x25, 0x27, 0x28, 0x29, 0x2b, 0x2c, 0x2d, 0x2e,
  0x2f, 0x31, 0x32, 0x33, 0x34, 0x36, 0x37, 0x38, 0x3a, 0x3c,
  0x3e, 0x40, 0x41, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
  0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x50, 0x52, 0x53, 0x55, 0x56,
  0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c
};

#define MAX_EXP2_VCLIP_NUM_TABLE 4
UINT8 exp2_vclip_num_table [MAX_EXP2_VCLIP_NUM_TABLE] = {
	0x00, 0x03, 0x07, 0x3c
	};

#define MAX_WEAPON_TYPE_TABLE 30
UINT8 weapon_type_table [MAX_WEAPON_TYPE_TABLE] = {
	0x00, 0x05, 0x06, 0x0a, 0x0b, 0x0e, 0x11, 0x14, 0x15, 0x16,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
	0x2e, 0x30, 0x32, 0x34, 0x35, 0x37, 0x39, 0x3a, 0x3c, 0x3d
	};
/*
 1: 5, 6, 10, 11, 14, 17, 20, 21, 22,
10: 24, 25, 26, 27, 28, 41, 42, 43, 44, 45,
20: 46, 48, 50, 52, 53, 55, 57, 58, 60, 61
*/
#define MAX_BEHAVIOR_TABLE 8
UINT8 behavior_table [MAX_BEHAVIOR_TABLE] = {
	AIB_STILL, AIB_NORMAL, AIB_GET_BEHIND, AIB_RUN_FROM, AIB_FOLLOW_PATH, AIB_STATION, AIB_SNIPE
	};

char *szSkills [5] = {
	"Trainee",
	"Rookie",
	"Hotshot",
	"Ace",
	"Insane"
};

char *pszExplosionIds [] = {"small explosion", "medium explosion", "big explosion", "huge explosion", "red blast"};
int nExplosionIds [] = {7, 58, 0, 60, 106};

int powerupIdStrXlat [MAX_POWERUP_IDS2];

                        /*--------------------------*/

static tSliderData sliderData [] = {
	{IDC_OBJ_SCORE, 0, 600, 50, NULL},
	{IDC_OBJ_STRENGTH, 13, 20, 1, NULL},
	{IDC_OBJ_MASS, 10, 20, 1, NULL},
	{IDC_OBJ_DRAG, 1, 13, -F1_0 / 100, NULL},
	{IDC_OBJ_EBLOBS, 0, 100, 1, NULL},
	{IDC_OBJ_LIGHT, 0, 10, 1, NULL},
	{IDC_OBJ_GLOW, 0, 12, 1, NULL},
	{IDC_OBJ_AIM, 2, 4, -0x40, NULL},
	{IDC_OBJ_SKILL, 0, 4, 1, szSkills},
	{IDC_OBJ_FOV, -10, 10, -F1_0 / 10, NULL},
	{IDC_OBJ_FIREWAIT1, 1, 35, -F1_0 / 5, NULL},
	{IDC_OBJ_FIREWAIT2, 1, 35, -F1_0 / 5, NULL},
	{IDC_OBJ_TURNTIME, 0, 10, -F1_0 / 10, NULL},
	{IDC_OBJ_MAXSPEED, 0, 140, -F1_0, NULL},
	{IDC_OBJ_FIRESPEED, 1, 18, 1, NULL},
	{IDC_OBJ_EVADESPEED, 0, 6, 1, NULL},
	{IDC_OBJ_CIRCLEDIST, 0, 0, -F1_0, NULL},
	{IDC_OBJ_DEATHROLL, 0, 10, 1, NULL},
	{IDC_OBJ_EXPLSIZE, 0, 100, 1, NULL},
	{IDC_OBJ_CONT_COUNT, 0, 100, 1, NULL},
	{IDC_OBJ_CONT_PROB, 0, 16, 1, NULL}
	};

                        /*--------------------------*/

FIX fix_exp(int x);
int fix_log(FIX x);

                        /*--------------------------*/

BEGIN_MESSAGE_MAP (CObjectTool, CToolDlg)
	ON_WM_HSCROLL ()
	ON_WM_PAINT ()
	ON_BN_CLICKED (IDC_OBJ_ADD, OnAdd)
	ON_BN_CLICKED (IDC_OBJ_DELETE, OnDelete)
	ON_BN_CLICKED (IDC_OBJ_DELETEALL, OnDeleteAll)
	ON_BN_CLICKED (IDC_OBJ_MOVE, OnMove)
	ON_BN_CLICKED (IDC_OBJ_RESET, OnReset)
	ON_BN_CLICKED (IDC_OBJ_DEFAULT, OnDefault)
	ON_BN_CLICKED (IDC_OBJ_ADVANCED, OnAdvanced)
	ON_BN_CLICKED (IDC_OBJ_AI_KAMIKAZE, OnAIKamikaze)
	ON_BN_CLICKED (IDC_OBJ_AI_COMPANION, OnAICompanion)
	ON_BN_CLICKED (IDC_OBJ_AI_THIEF, OnAIThief)
	ON_BN_CLICKED (IDC_OBJ_AI_SMARTBLOBS, OnAISmartBlobs)
	ON_BN_CLICKED (IDC_OBJ_AI_PURSUE, OnAIPursue)
	ON_BN_CLICKED (IDC_OBJ_AI_CHARGE, OnAICharge)
	ON_BN_CLICKED (IDC_OBJ_AI_EDRAIN, OnAIEDrain)
	ON_BN_CLICKED (IDC_OBJ_BRIGHT, OnBright)
	ON_BN_CLICKED (IDC_OBJ_CLOAKED, OnCloaked)
	ON_BN_CLICKED (IDC_OBJ_MULTIPLAYER, OnMultiplayer)
	ON_BN_CLICKED (IDC_OBJ_SORT, OnSort)
	ON_CBN_SELCHANGE (IDC_OBJ_OBJNO, OnSetObject)
	ON_CBN_SELCHANGE (IDC_OBJ_TYPE, OnSetObjType)
	ON_CBN_SELCHANGE (IDC_OBJ_ID, OnSetObjId)
	ON_CBN_SELCHANGE (IDC_OBJ_TEXTURE, OnSetTexture)
	ON_CBN_SELCHANGE (IDC_OBJ_SPAWN_TYPE, OnSetSpawnType)
	ON_CBN_SELCHANGE (IDC_OBJ_CONT_TYPE, OnSetContType)
	ON_CBN_SELCHANGE (IDC_OBJ_SPAWN_ID, OnSetSpawnId)
	ON_CBN_SELCHANGE (IDC_OBJ_CONT_TYPE, OnSetContType)
	ON_CBN_SELCHANGE (IDC_OBJ_CONT_ID, OnSetContId)
	ON_CBN_SELCHANGE (IDC_OBJ_AI, OnSetObjAI)
	ON_CBN_SELCHANGE (IDC_OBJ_CLASS_AI, OnSetObjClassAI)
	ON_CBN_SELCHANGE (IDC_OBJ_AI_BOSSTYPE, OnAIBossType)
	ON_CBN_SELCHANGE (IDC_OBJ_TEXTURE, OnSetTexture)
	ON_CBN_SELCHANGE (IDC_OBJ_SOUND_EXPLODE, OnSetSoundExplode)
	ON_CBN_SELCHANGE (IDC_OBJ_SOUND_ATTACK, OnSetSoundAttack)
	ON_CBN_SELCHANGE (IDC_OBJ_SOUND_SEE, OnSetSoundSee)
	ON_CBN_SELCHANGE (IDC_OBJ_SOUND_CLAW, OnSetSoundClaw)
	ON_CBN_SELCHANGE (IDC_OBJ_SOUND_DEATH, OnSetSoundDeath)
	ON_CBN_SELCHANGE (IDC_OBJ_WEAPON1, OnSetWeapon1)
	ON_CBN_SELCHANGE (IDC_OBJ_WEAPON2, OnSetWeapon2)
	ON_EN_KILLFOCUS (IDC_OBJ_SPAWN_QTY, OnSetSpawnQty)
	ON_EN_UPDATE (IDC_OBJ_SPAWN_QTY, OnSetSpawnQty)
END_MESSAGE_MAP ()

                        /*--------------------------*/

CObjectTool::CObjectTool (CPropertySheet *pParent)
	: CToolDlg (nLayout ? IDD_OBJECTDATA2: IDD_OBJECTDATA, pParent)
{
//Reset ();
}

                        /*--------------------------*/

CObjectTool::~CObjectTool ()
{
if (m_bInited) {
	m_showObjWnd.DestroyWindow ();
	m_showSpawnWnd.DestroyWindow ();
	m_showTextureWnd.DestroyWindow ();
	}
}

                        /*--------------------------*/

void CObjectTool::UpdateSliders (int i)
{
CWnd *pWnd;
char szPos [10];
char *pszPos;
int min = (i < 0) ? 0: i;
int max = (i < 0) ? sizeof (sliderData) / sizeof (tSliderData): i + 1;
int nPos;
tSliderData *psd = sliderData + min;
for (i = min; i < max; i++, psd++) {
	pWnd = GetDlgItem (psd->nId);
	nPos = ((CSliderCtrl *) pWnd)->GetPos ();
	if (psd->pszLabels)
		pszPos = psd->pszLabels [nPos];
	else {
		if (psd->nFactor > 0)
			nPos *= psd->nFactor;
		sprintf (szPos, "%d", (int) nPos);
		pszPos = szPos;
		}
	AfxSetWindowText (GetDlgItem (psd->nId + 1)->GetSafeHwnd (), pszPos);
	}
}

                        /*--------------------------*/

void CObjectTool::InitSliders ()
{
int h = sizeof (sliderData) / sizeof (tSliderData);
tSliderData *psd = sliderData;
int i;
for (i = 0; i < h; i++, psd++) {
	InitSlider (psd->nId, psd->nMin, psd->nMax);
	((CSliderCtrl *) GetDlgItem (psd->nId))->SetPos (psd->nMin);
	}
}

                        /*--------------------------*/

void CObjectTool::CBInit (CComboBox *pcb, char** pszNames, UINT8 *pIndex, UINT8 *pItemData, int nMax, int nType, bool bAddNone)
{
	int h, j, l;
	HINSTANCE hInst;
	char szLabel [100];
	char *pszLabel;
	DWORD nErr;
	
if (nType & 1) {
	hInst = AfxGetApp()->m_hInstance;
	pszLabel = szLabel;
	}
else if (nType == 2)
	pszLabel = szLabel;
pcb->ResetContent ();
if (bAddNone) {
	j = pcb->AddString ("(none)");
	pcb->SetItemData (j, -1);
	}
int i;
for (i = 0; i < nMax; i++) {
	switch (nType) {
		case 0:
			h = pIndex ? pIndex [i]: i;
			sprintf (szLabel, "%s", pszNames [h]);
			pszLabel = szLabel;
//			pszLabel = pszNames [h];
			break;
		case 1:
			sprintf (szLabel, "%d: ", i);
			l = strlen (szLabel);
			LoadString (hInst, ((int) pszNames) + i, szLabel + l, sizeof (szLabel) - l);
			h = i;
			break;
		case 2:
			sprintf (szLabel, "%s %d", (char *) pszNames, i);
			h = pIndex ? pIndex [i]: i;
			break;
		case 3:
			LoadString (hInst, ((int) pszNames) + i, szLabel, sizeof (szLabel));
			nErr = GetLastError ();
			h = i;
			break;
		default:
			return;
		}
	if (!strstr (pszLabel, "(not used)")) {
		j = pcb->AddString (pszLabel);
		pcb->SetItemData (j, pItemData ? pItemData [h]: h);
		}
	}
pcb->SetCurSel (0);
}

                        /*--------------------------*/

int CObjectTool::CBAddString (CComboBox *pcb, char *str)
{
	int	i = 0, m = 0, l = 0, r = pcb->GetCount () - 1;
	char	h [80], *hsz, *psz;

psz = isalpha (*str) ? str: strstr (str, ":") + 1;
while (l <= r) {
	m = (l + r) / 2;
	pcb->GetLBText (m, h);
	hsz = isalpha (*h) ? h: strstr (h, ":") + 1;
	i = strcmp (psz, hsz);
	if (i < 0)
		r = m - 1;
	else if (i > 0)
		l = m + 1;
	else
		break;
	}
if (i > 0)
	m++;
return pcb->InsertString (m, str);
}

                        /*--------------------------*/

double CObjectTool::SliderFactor (int nId)
{
int h = sizeof (sliderData) / sizeof (tSliderData);
tSliderData *psd = sliderData;
int i;
for (i = 0; i < h; i++, psd++)
	if (psd->nId == nId)
		return (double) ((psd->nFactor < 0) ? -(psd->nFactor): psd->nFactor);
return 1.0;
}

                        /*--------------------------*/

char *pszBossTypes [] = {"none", "Boss 1", "Boss 2", "Red Fatty", "Water Boss", "Fire Boss", "Ice Boss", "Alien 1", "Alien 2", "Vertigo", "Red Guard", NULL};

BOOL CObjectTool::OnInitDialog ()
{
if (!GetMine ())
	return FALSE;
CToolDlg::OnInitDialog ();
CreateImgWnd (&m_showObjWnd, IDC_OBJ_SHOW);
CreateImgWnd (&m_showSpawnWnd, IDC_OBJ_SHOW_SPAWN);
CreateImgWnd (&m_showTextureWnd, IDC_OBJ_SHOW_TEXTURE);
InitSliders ();
UpdateSliders ();
CBInit (CBObjType (), (char**) object_names, object_list, NULL, MAX_OBJECT_NUMBER);
CBInit (CBSpawnType (), (char**) object_names, contains_list, NULL, MAX_CONTAINS_NUMBER, 0, true);
CBInit (CBObjAI (), (char**) ai_options, NULL, behavior_table, (file_type == RDL_FILE) ? MAX_D1_AI_OPTIONS: MAX_D2_AI_OPTIONS);
CBInit (CBObjClassAI (), (char**) ai_options, NULL, behavior_table, (file_type == RDL_FILE) ? MAX_D1_AI_OPTIONS: MAX_D2_AI_OPTIONS);

INT16 nTextures = (file_type == RDL_FILE) ? MAX_D1_TEXTURES: MAX_D2_TEXTURES;
INT16 i, j;
char sz [100], **psz;
HINSTANCE hInst = AfxGetApp()->m_hInstance;
CComboBox *pcb = CBObjTexture ();
pcb->AddString ("(none)");
for (i = 0; i < nTextures; i++) {
	LoadString (hInst, texture_resource + i, sz, sizeof (sz));
	if (!strstr((char *) sz, "frame")) {
		int index = pcb->AddString (sz);
		pcb->SetItemData(index++, i);
		}
	}
for (i = j = 0; i < MAX_POWERUP_IDS; i++) {
	LoadString (hInst, POWERUP_STRING_TABLE + i, sz, sizeof (sz));
	//if (strcmp (sz, "(not used)"))
		powerupIdStrXlat [j++] = i;
	}
pcb = CBBossType ();
pcb->ResetContent ();
for (psz = pszBossTypes; *psz; psz++) {
	int index = pcb->AddString (*psz);
	pcb->SetItemData(index++, (int) (psz - pszBossTypes));
	}
CDObject *obj = m_mine->CurrObj ();
//CBInit (CBObjProps (), (char **) ROBOT_STRING_TABLE, NULL, NULL, ROBOT_IDS2, 1);
//SelectItemData (CBObjProps (), (obj->type == OBJ_ROBOT) && (obj->id < N_D2_ROBOT_TYPES) ? obj->id: -1);
CBInit (CBExplType (), (char **) "explosion", NULL, exp2_vclip_num_table, MAX_EXP2_VCLIP_NUM_TABLE, 2);
CBInit (CBWeapon1 (), (char **) 7000, NULL, NULL, MAX_WEAPON_TYPES, 3, true);
CBInit (CBWeapon2 (), (char **) 7000, NULL, NULL, MAX_WEAPON_TYPES, 3, true);
i = CBContType ()->AddString ("Robot");
CBContType ()->SetItemData (i, OBJ_ROBOT);
i = CBContType ()->AddString ("Powerup");
CBContType ()->SetItemData (i, OBJ_POWERUP);
i = CBContType ()->AddString ("(none)");
CBContType ()->SetItemData (i, -1);
// setup sound list boxes
char szSound [100];
for (i = 0; i < 196; i++) {
	LoadString (hInst, 6000 + i, szSound,sizeof (szSound));
	// int nSound = (szSound [0] - '0') * 100 + (szSound [1] - '0') * 10 + (szSound [2] - '0');
	int nSound = atoi (szSound);
	int index = CBSoundExpl ()->AddString (szSound + 3);
	CBSoundExpl ()->SetItemData (index, nSound);
	index = CBSoundSee ()->AddString (szSound + 3);
	CBSoundSee ()->SetItemData (index, nSound);
	index = CBSoundAttack ()->AddString (szSound + 3);
	CBSoundAttack ()->SetItemData (index, nSound);
	index = CBSoundClaw ()->AddString (szSound + 3);
	CBSoundClaw ()->SetItemData (index, nSound);
	index = CBSoundDeath ()->AddString (szSound + 3);
	CBSoundDeath ()->SetItemData (index, nSound);
	}
Refresh ();
m_bInited = true;
return TRUE;
}

                        /*--------------------------*/

void CObjectTool::DoDataExchange (CDataExchange *pDX)
{
if (!GetMine ())
	return;

DDX_Text (pDX, IDC_OBJ_SPAWN_QTY, m_nSpawnQty);
DDX_Text (pDX, IDC_OBJ_INFO, m_szInfo, sizeof (m_szInfo));
if (!pDX->m_bSaveAndValidate) {
	char szCount [4];

	sprintf (szCount, "%d", ObjOfAKindCount ());
	AfxSetWindowText (GetDlgItem (IDT_OBJ_COUNT)->GetSafeHwnd (), szCount);
	}
DDX_Check (pDX, IDC_OBJ_SORT, m_mine->m_bSortObjects);
}

                        /*--------------------------*/

BOOL CObjectTool::OnSetActive ()
{
Refresh ();
return CToolDlg::OnSetActive ();
}

                        /*--------------------------*/

void CObjectTool::Reset ()
{
//m_nSpawnQty = 0;
}

                        /*--------------------------*/

void CObjectTool::EnableControls (BOOL bEnable)
{
CToolDlg::EnableControls (IDC_OBJ_OBJNO, IDT_OBJ_CONT_PROB, bEnable);
}

                        /*--------------------------*/

int CObjectTool::GetSliderData (CScrollBar *pScrollBar)
{
int h = sizeof (sliderData) / sizeof (tSliderData);
int i;
for (i = 0; i < h; i++)
	if (pScrollBar == (CScrollBar *) GetDlgItem (sliderData [i].nId))
		return i;
return -1;
}

                        /*--------------------------*/

void CObjectTool::OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
int i = GetSliderData (pScrollBar);
if (i < 0)
	return;
tSliderData *psd = sliderData + i;
int nPos = pScrollBar->GetScrollPos ();
switch (scrollCode) {
	case SB_LINEUP:
		nPos++;
		break;
	case SB_LINEDOWN:
		nPos--;
		break;
	case SB_PAGEUP:
		nPos += (psd->nMax - psd->nMin) / 4;
		break;
	case SB_PAGEDOWN:
		nPos -= (psd->nMax - psd->nMin) / 4;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nPos = thumbPos;
		break;
	case SB_ENDSCROLL:
		return;
	}
if (nPos < psd->nMin)
	nPos = psd->nMin;
else if (nPos > psd->nMax)
	nPos = psd->nMax;
pScrollBar->SetScrollPos (nPos);
if (pScrollBar == SBCtrl (IDC_OBJ_SKILL))
	Refresh ();
else
	UpdateSliders (i);
UpdateRobot ();
}

                        /*--------------------------*/

void CObjectTool::Refresh ()
{
if (!m_bInited)
	return;
if (!GetMine ())
	return;
HINSTANCE hInst = AfxGetApp()->m_hInstance;

INT16 type;

// update object list box
CBObjNo ()->ResetContent ();
CDObject *obj = m_mine->Objects ();
int i;
for (i = 0; i < m_mine->GameInfo ().objects.count; i++, obj++) {
	switch(obj->type) {
		case OBJ_ROBOT: /* an evil enemy */
			LoadString (hInst, ROBOT_STRING_TABLE + obj->id, string, sizeof (string));
			break;
		case OBJ_HOSTAGE: // a hostage you need to rescue
			sprintf (string, "Hostage");
			break;
		case OBJ_PLAYER: // the player on the console
			sprintf (string,"Player #%d",obj->id);
			break;
		case OBJ_WEAPON: //
			strcpy(string, "Red Mine");
			break;
		case OBJ_POWERUP: // a powerup you can pick up
			LoadString(hInst, POWERUP_STRING_TABLE + powerupIdStrXlat [obj->id], string, sizeof (string));
			break;
		case OBJ_CNTRLCEN: // a control center */
			sprintf (string, "Reactor");
			break;
		case OBJ_COOP: // a cooperative player object
			sprintf (string, "Coop Player #%d", obj->id);
			break;
		case OBJ_CAMBOT: // a camera */
			sprintf (string, "Camera");
			break;
		case OBJ_MONSTERBALL: // a camera */
			sprintf (string, "Monsterball");
			break;
		case OBJ_EXPLOSION:
			sprintf (string, "Explosion");
			break;
		case OBJ_SMOKE: 
			sprintf (string, "Smoke");
			break;
		case OBJ_EFFECT:
			sprintf (string, "Effect");
			break;
		default:
			*string = '\0';
	}
	sprintf (message, (i < 10) ? "%3d: %s": "%d: %s", i, string);
	CBObjNo ()->AddString (message);
	}
// add secret object to list
for (i = 0; i < m_mine->GameInfo ().triggers.count; i++)
	if (m_mine->Triggers (i)->type == TT_SECRET_EXIT) {
		CBObjNo ()->AddString ("secret object");
		break;
		}
// select current object
CBObjNo ()->SetCurSel (m_mine->Current ()->object);

// if secret object, disable everything but the "move" button
// and the object list, then return
if (m_mine->Current ()->object == m_mine->GameInfo ().objects.count) {
	CToolDlg::EnableControls (IDC_OBJ_OBJNO, IDC_OBJ_SPAWN_QTY, FALSE);
	CBObjNo ()->EnableWindow (TRUE);
	BtnCtrl (IDC_OBJ_MOVE)->EnableWindow (TRUE);

	strcpy (m_szInfo, "Secret Level Return");
	CBObjType ()->SetCurSel (-1);
	CBObjId ()->SetCurSel (-1);
	CBObjAI ()->SetCurSel (-1);
	CBObjTexture ()->SetCurSel (-1);
	CBSpawnType ()->SetCurSel (-1);
	CBSpawnId ()->SetCurSel (-1);
	CBObjClassAI ()->SetCurSel (-1);

	CDC *pDC = m_showObjWnd.GetDC ();
	if (pDC) {
		CRect rc;
		m_showObjWnd.GetClientRect (rc);
		pDC->FillSolidRect (&rc, IMG_BKCOLOR);
		m_showObjWnd.ReleaseDC (pDC);
		CToolDlg::EnableControls (IDC_OBJ_DELETEALL, IDC_OBJ_DELETEALL, m_mine->GameInfo ().objects.count > 0);
		UpdateData (FALSE);
		}
	return;
	}

// otherwise (non-secret object), setup the rest of the
// dialog.
obj = m_mine->CurrObj ();
sprintf (m_szInfo , "cube %d", obj->segnum);
if (/*(object_selection [obj->type] == 0) &&*/ m_mine->RobotInfo (obj->id)->pad [0])
	strcat (m_szInfo, "\r\nmodified");

CBObjType ()->SetCurSel (object_selection [obj->type]);
SetObjectId (CBObjId (), obj->type, obj->id);

// ungray most buttons and combo boxes
CToolDlg::EnableControls (IDC_OBJ_OBJNO, IDC_OBJ_SPAWN_QTY, TRUE);

// gray contains and behavior if not a robot type object
if (obj->type != OBJ_ROBOT) {
//	CBObjProps ()->EnableWindow (FALSE);
	CToolDlg::EnableControls (IDC_OBJ_SPAWN_TYPE, IDC_OBJ_SPAWN_QTY, FALSE);
	CToolDlg::EnableControls (IDC_OBJ_BRIGHT, IDT_OBJ_CONT_PROB, FALSE);
	for (i = IDC_OBJ_SOUND_EXPLODE; i <= IDC_OBJ_SOUND_DEATH; i++)
		CBCtrl (i)->SetCurSel (-1);
	}
else {
	CToolDlg::EnableControls (IDC_OBJ_BRIGHT, IDT_OBJ_CONT_PROB, TRUE);
	for (i = IDC_OBJ_SOUND_EXPLODE; i <= IDC_OBJ_SOUND_DEATH; i++)
		CBCtrl (i)->SetCurSel (0);
	}

// gray texture override if not a poly object
if (obj->render_type != RT_POLYOBJ)
	CBObjTexture ()->EnableWindow (FALSE);

// gray edit if this is an RDL file
if (file_type == RDL_FILE)
	CToolDlg::EnableControls (IDC_OBJ_BRIGHT, IDT_OBJ_CONT_PROB, FALSE);

// set contains data
type = (obj->contains_type == -1) ? MAX_CONTAINS_NUMBER : contains_selection [obj->contains_type];
//if (type == -1)
//	type = MAX_CONTAINS_NUMBER;

CBSpawnType ()->SetCurSel (type + 1);
//SelectItemData (CBSpawnType (), type);
SetObjectId (CBSpawnId (), obj->contains_type, obj->contains_id, 1);
m_nSpawnQty = obj->contains_count;
//SelectItemData (CBObjProps (), (obj->type == OBJ_ROBOT) && (obj->id < N_D2_ROBOT_TYPES) ? obj->id: -1);
if ((obj->type == OBJ_ROBOT) || (obj->type == OBJ_CAMBOT)) {
	int index =
		((obj->ctype.ai_info.behavior == AIB_RUN_FROM) && (obj->ctype.ai_info.flags [4] & 0x02)) ? // smart bomb flag
		8 : obj->ctype.ai_info.behavior - 0x80;
	CBObjAI ()->SetCurSel (index);
	}
else
	CBObjAI ()->SetCurSel (1); // Normal
RefreshRobot ();
UpdateSliders ();
DrawObjectImages ();
SetTextureOverride ();
CToolDlg::EnableControls (IDC_OBJ_DELETEALL, IDC_OBJ_DELETEALL, m_mine->GameInfo ().objects.count > 0);
UpdateData (FALSE);
theApp.MineView ()->Refresh (FALSE);
}

                        /*--------------------------*/

void CObjectTool::RefreshRobot ()
{
if (!GetMine ())
	return;

  int i,j;
  ROBOT_INFO rInfo;

  // get selection
if (object_list [CBObjType ()->GetCurSel ()] != OBJ_ROBOT) {
	CBContId ()->SetCurSel (-1);
	CBWeapon1 ()->SetCurSel (-1);
	CBWeapon2 ()->SetCurSel (-1);
	CBSoundExpl ()->SetCurSel (-1);
	CBSoundSee ()->SetCurSel (-1);
	CBSoundAttack ()->SetCurSel (-1);
	CBSoundClaw ()->SetCurSel (-1);
	CBSoundDeath ()->SetCurSel (-1);
	CBObjClassAI ()->SetCurSel (-1);
	CBExplType ()->SetCurSel (-1);
	CBContType ()->SetCurSel (-1);
	// update check boxes
	BtnCtrl (IDC_OBJ_AI_KAMIKAZE)->SetCheck (FALSE);
	BtnCtrl (IDC_OBJ_AI_COMPANION)->SetCheck (FALSE);
	BtnCtrl (IDC_OBJ_AI_THIEF)->SetCheck (FALSE);
	BtnCtrl (IDC_OBJ_AI_SMARTBLOBS)->SetCheck (FALSE);
	BtnCtrl (IDC_OBJ_AI_PURSUE)->SetCheck (FALSE);
	BtnCtrl (IDC_OBJ_AI_CHARGE)->SetCheck (FALSE);
	BtnCtrl (IDC_OBJ_AI_EDRAIN)->SetCheck (FALSE);
	BtnCtrl (IDC_OBJ_BRIGHT)->SetCheck (FALSE);
	BtnCtrl (IDC_OBJ_CLOAKED)->SetCheck (FALSE);
	// update scroll bars
	SlCtrl (IDC_OBJ_SCORE)->SetPos (0);
	SlCtrl (IDC_OBJ_STRENGTH)->SetPos (0);
	SlCtrl (IDC_OBJ_MASS)->SetPos (0);
	SlCtrl (IDC_OBJ_DRAG)->SetPos (0);
	SlCtrl (IDC_OBJ_EBLOBS)->SetPos (0);
	SlCtrl (IDC_OBJ_LIGHT)->SetPos (0);
	SlCtrl (IDC_OBJ_GLOW)->SetPos (0);
	SlCtrl (IDC_OBJ_AIM)->SetPos (0);
	SlCtrl (IDC_OBJ_FOV)->SetPos (0);
	SlCtrl (IDC_OBJ_FIREWAIT1)->SetPos (0);
	SlCtrl (IDC_OBJ_FIREWAIT2)->SetPos (0);
	SlCtrl (IDC_OBJ_TURNTIME)->SetPos (0);
	SlCtrl (IDC_OBJ_MAXSPEED)->SetPos (0);
	SlCtrl (IDC_OBJ_CIRCLEDIST)->SetPos (0);
	SlCtrl (IDC_OBJ_FIRESPEED)->SetPos (0);
	SlCtrl (IDC_OBJ_EVADESPEED)->SetPos (0);
	SlCtrl (IDC_OBJ_DEATHROLL)->SetPos (0);
	SlCtrl (IDC_OBJ_EXPLSIZE)->SetPos (0);
	SlCtrl (IDC_OBJ_CONT_PROB)->SetPos (0);
	SlCtrl (IDC_OBJ_CONT_COUNT)->SetPos (0);
	return;
	}
i = CBObjId ()->GetItemData (CBObjId ()->GetCurSel ());
rInfo = *m_mine->RobotInfo (i);
j = SlCtrl (IDC_OBJ_SKILL)->GetPos ();
CBContId ()->ResetContent ();
switch (rInfo.contains_type) {
	case OBJ_ROBOT: /* an evil enemy */
		CBInit (CBContId (), (char **) ROBOT_STRING_TABLE, NULL, NULL, ROBOT_IDS2, 1, true);
		break;
	case OBJ_POWERUP: // a powerup you can pick up
		CBInit (CBContId (), (char **) POWERUP_STRING_TABLE, NULL, NULL, MAX_POWERUP_IDS, 1, true);
		break;
	}
// update list boxes
SelectItemData (CBContId (), (int) rInfo.contains_id);
SelectItemData (CBWeapon1 (), (int) (rInfo.weapon_type ? rInfo.weapon_type : -1));
SelectItemData (CBWeapon2 (), (int) rInfo.weapon_type2);
SelectItemData (CBSoundExpl (), (int) rInfo.exp2_sound_num);
SelectItemData (CBSoundSee (), (int) rInfo.see_sound);
SelectItemData (CBSoundAttack (), (int) rInfo.attack_sound);
SelectItemData (CBSoundClaw (), (int) rInfo.claw_sound);
SelectItemData (CBSoundDeath (), (int) rInfo.deathroll_sound);
SelectItemData (CBObjClassAI (), (int) rInfo.behavior);
SelectItemData (CBExplType (), (int) rInfo.exp2_vclip_num);
SelectItemData (CBContType (), (int) rInfo.contains_type);
SelectItemData (CBBossType (), (int) (rInfo.boss_flag < 21) ? rInfo.boss_flag : rInfo.boss_flag - 18);
// update check boxes
BtnCtrl (IDC_OBJ_AI_KAMIKAZE)->SetCheck (rInfo.kamikaze);
BtnCtrl (IDC_OBJ_AI_COMPANION)->SetCheck (rInfo.companion);
BtnCtrl (IDC_OBJ_AI_THIEF)->SetCheck (rInfo.thief);
BtnCtrl (IDC_OBJ_AI_SMARTBLOBS)->SetCheck (rInfo.smart_blobs);
BtnCtrl (IDC_OBJ_AI_PURSUE)->SetCheck (rInfo.pursuit);
BtnCtrl (IDC_OBJ_AI_CHARGE)->SetCheck (rInfo.attack_type);
BtnCtrl (IDC_OBJ_AI_EDRAIN)->SetCheck (rInfo.energy_drain);
BtnCtrl (IDC_OBJ_BRIGHT)->SetCheck (rInfo.lighting);
BtnCtrl (IDC_OBJ_CLOAKED)->SetCheck (rInfo.cloak_type);
// update scroll bars
SlCtrl (IDC_OBJ_SCORE)->SetPos ((int) (rInfo.score_value / SliderFactor (IDC_OBJ_SCORE)));
SlCtrl (IDC_OBJ_STRENGTH)->SetPos (fix_log(rInfo.strength));
SlCtrl (IDC_OBJ_MASS)->SetPos (fix_log(rInfo.mass));
SlCtrl (IDC_OBJ_DRAG)->SetPos ((int) (rInfo.drag / SliderFactor (IDC_OBJ_DRAG)));
SlCtrl (IDC_OBJ_EBLOBS)->SetPos ((int) (rInfo.energy_blobs / SliderFactor (IDC_OBJ_EBLOBS)));
SlCtrl (IDC_OBJ_LIGHT)->SetPos ((int) (rInfo.lightcast / SliderFactor (IDC_OBJ_LIGHT)));
SlCtrl (IDC_OBJ_GLOW)->SetPos ((int) (rInfo.glow / SliderFactor (IDC_OBJ_GLOW)));
SlCtrl (IDC_OBJ_AIM)->SetPos ((int) ((rInfo.aim + 1) / SliderFactor (IDC_OBJ_AIM)));
SlCtrl (IDC_OBJ_FOV)->SetPos ((int) (rInfo.field_of_view [j] / SliderFactor (IDC_OBJ_FOV)));
SlCtrl (IDC_OBJ_FIREWAIT1)->SetPos ((int) (rInfo.firing_wait [j] / SliderFactor (IDC_OBJ_FIREWAIT1)));
SlCtrl (IDC_OBJ_FIREWAIT2)->SetPos ((int) (rInfo.firing_wait2 [j] / SliderFactor (IDC_OBJ_FIREWAIT2)));
SlCtrl (IDC_OBJ_TURNTIME)->SetPos ((int) (rInfo.turn_time [j] / SliderFactor (IDC_OBJ_TURNTIME)));
SlCtrl (IDC_OBJ_MAXSPEED)->SetPos ((int) (rInfo.max_speed [j] / SliderFactor (IDC_OBJ_MAXSPEED)));
SlCtrl (IDC_OBJ_CIRCLEDIST)->SetPos ((int) (rInfo.circle_distance [j] / SliderFactor (IDC_OBJ_CIRCLEDIST)));
SlCtrl (IDC_OBJ_FIRESPEED)->SetPos ((int) (rInfo.rapidfire_count [j] / SliderFactor (IDC_OBJ_FIRESPEED)));
SlCtrl (IDC_OBJ_EVADESPEED)->SetPos ((int) (rInfo.evade_speed [j] / SliderFactor (IDC_OBJ_EVADESPEED)));
SlCtrl (IDC_OBJ_DEATHROLL)->SetPos ((int) (rInfo.death_roll / SliderFactor (IDC_OBJ_DEATHROLL)));
SlCtrl (IDC_OBJ_EXPLSIZE)->SetPos ((int) (rInfo.badass / SliderFactor (IDC_OBJ_EXPLSIZE)));
SlCtrl (IDC_OBJ_CONT_PROB)->SetPos ((int) (rInfo.contains_prob / SliderFactor (IDC_OBJ_CONT_PROB)));
SlCtrl (IDC_OBJ_CONT_COUNT)->SetPos ((int) (rInfo.contains_count / SliderFactor (IDC_OBJ_CONT_COUNT)));
}
  
                        /*--------------------------*/

void CObjectTool::UpdateRobot ()
{
if (!GetMine ())
	return;

  int i,j;
  ROBOT_INFO rInfo;

  // get selection
i = CBObjId ()->GetItemData (CBObjId ()->GetCurSel ());
if (i < 0 || i >= ROBOT_IDS2)
	i = 0;
j = SlCtrl (IDC_OBJ_SKILL)->GetPos ();
rInfo = *m_mine->RobotInfo (i);
theApp.SetModified (TRUE);
rInfo.pad [0] |= 1;
rInfo.score_value = (int) (SlCtrl (IDC_OBJ_SCORE)->GetPos () * SliderFactor (IDC_OBJ_SCORE));
rInfo.strength = (int) fix_exp (SlCtrl (IDC_OBJ_STRENGTH)->GetPos ());
rInfo.mass = (int) fix_exp (SlCtrl (IDC_OBJ_MASS)->GetPos ());
rInfo.drag = (int) (SlCtrl (IDC_OBJ_DRAG)->GetPos () * SliderFactor (IDC_OBJ_DRAG));
rInfo.energy_blobs = (int) (SlCtrl (IDC_OBJ_EBLOBS)->GetPos ()  * SliderFactor (IDC_OBJ_EBLOBS));
rInfo.lightcast = (int) (SlCtrl (IDC_OBJ_LIGHT)->GetPos () * SliderFactor (IDC_OBJ_LIGHT));
rInfo.glow = (int) (SlCtrl (IDC_OBJ_GLOW)->GetPos () * SliderFactor (IDC_OBJ_GLOW));
rInfo.aim = (int) ((SlCtrl (IDC_OBJ_AIM)->GetPos ()) * SliderFactor (IDC_OBJ_AIM)) - 1;
rInfo.field_of_view [j] = (int) (SlCtrl (IDC_OBJ_FOV)->GetPos () * SliderFactor (IDC_OBJ_FOV));
rInfo.firing_wait [j] = (int) (SlCtrl (IDC_OBJ_FIREWAIT1)->GetPos () * SliderFactor (IDC_OBJ_FIREWAIT1));
rInfo.firing_wait2 [j] = (int) (SlCtrl (IDC_OBJ_FIREWAIT2)->GetPos () * SliderFactor (IDC_OBJ_FIREWAIT2));
rInfo.turn_time [j] = (int) (SlCtrl (IDC_OBJ_TURNTIME)->GetPos () * SliderFactor (IDC_OBJ_TURNTIME));
rInfo.max_speed [j] = (int) (SlCtrl (IDC_OBJ_MAXSPEED)->GetPos () * SliderFactor (IDC_OBJ_MAXSPEED));
rInfo.circle_distance [j] = (int) (SlCtrl (IDC_OBJ_CIRCLEDIST)->GetPos () * SliderFactor (IDC_OBJ_CIRCLEDIST));
rInfo.rapidfire_count [j] = (int) (SlCtrl (IDC_OBJ_FIRESPEED)->GetPos () * SliderFactor (IDC_OBJ_FIRESPEED));
rInfo.evade_speed [j] = (int) (SlCtrl (IDC_OBJ_EVADESPEED)->GetPos () * SliderFactor (IDC_OBJ_EVADESPEED));
rInfo.death_roll = (int) (SlCtrl (IDC_OBJ_DEATHROLL)->GetPos () * SliderFactor (IDC_OBJ_DEATHROLL));
rInfo.badass = (int) (SlCtrl (IDC_OBJ_EXPLSIZE)->GetPos () * SliderFactor (IDC_OBJ_EXPLSIZE));
rInfo.contains_prob = (int) (SlCtrl (IDC_OBJ_CONT_PROB)->GetPos () * SliderFactor (IDC_OBJ_CONT_PROB));
rInfo.contains_count = (int) (SlCtrl (IDC_OBJ_CONT_COUNT)->GetPos () * SliderFactor (IDC_OBJ_CONT_COUNT));

rInfo.kamikaze = BtnCtrl (IDC_OBJ_AI_KAMIKAZE)->GetCheck ();
rInfo.companion = BtnCtrl (IDC_OBJ_AI_COMPANION)->GetCheck ();
rInfo.thief = BtnCtrl (IDC_OBJ_AI_THIEF)->GetCheck ();
rInfo.smart_blobs = BtnCtrl (IDC_OBJ_AI_SMARTBLOBS)->GetCheck ();
rInfo.pursuit = BtnCtrl (IDC_OBJ_AI_PURSUE)->GetCheck ();
rInfo.attack_type = BtnCtrl (IDC_OBJ_AI_CHARGE)->GetCheck ();
rInfo.energy_drain = BtnCtrl (IDC_OBJ_AI_EDRAIN)->GetCheck ();
rInfo.lighting = BtnCtrl (IDC_OBJ_BRIGHT)->GetCheck ();
rInfo.cloak_type = BtnCtrl (IDC_OBJ_CLOAKED)->GetCheck ();
m_mine->CurrObj ()->bMultiplayer = BtnCtrl (IDC_OBJ_MULTIPLAYER)->GetCheck ();

// get list box changes
int index;
if (0 <= (index = CBBossType ()->GetCurSel ())) {
	rInfo.boss_flag = (UINT8) CBBossType ()->GetItemData (index);
	if ((rInfo.boss_flag = (UINT8) CBBossType ()->GetItemData (index)) > 2)
			rInfo.boss_flag += 18;
	}
if (0 <= (index = CBWeapon1 ()->GetCurSel ())) {
	rInfo.weapon_type = (UINT8) CBWeapon1 ()->GetItemData (index);
	if (rInfo.weapon_type < 0)
		rInfo.weapon_type = 0;
	}
if (0 <= (index = CBWeapon2 ()->GetCurSel ()))
	rInfo.weapon_type2 = (UINT8) CBWeapon2 ()->GetItemData (index);
if (0 <= (index = CBSoundExpl ()->GetCurSel ()))
	rInfo.exp2_sound_num = (UINT8) CBSoundExpl ()->GetItemData (index);
if (0 <= (index = CBSoundSee ()->GetCurSel ()))
	rInfo.see_sound = (UINT8) CBSoundSee ()->GetItemData (index);
if (0 <= (index = CBSoundAttack ()->GetCurSel ()))
	rInfo.attack_sound = (UINT8) CBSoundAttack ()->GetItemData (index);
if (0 <= (index = CBSoundClaw ()->GetCurSel ()))
	rInfo.claw_sound = (UINT8) CBSoundClaw ()->GetItemData (index);
if (0 <= (index = CBSoundDeath ()->GetCurSel ()))
	rInfo.deathroll_sound = (UINT8) CBSoundDeath ()->GetItemData (index);
if (0 <= (index = CBObjClassAI ()->GetCurSel ()))
	rInfo.behavior = (UINT8) CBObjClassAI ()->GetItemData (index);
if (0 <= (index = CBExplType ()->GetCurSel ()))
	rInfo.exp2_vclip_num = (UINT8) CBExplType ()->GetItemData (index);
if (0 <= (index = CBContType ()->GetCurSel ()))
	rInfo.contains_type = (UINT8) CBContType ()->GetItemData (index);
if (0 <= (index = CBContId ()->GetCurSel ()) - 1)
	rInfo.contains_id = (UINT8) CBContId ()->GetItemData (index);
*m_mine->RobotInfo (i) = rInfo;
}

//------------------------------------------------------------------------
// CObjectTool - SetTextureOverride
//------------------------------------------------------------------------

void CObjectTool::SetTextureOverride ()
{
if (!GetMine ())
	return;
CDObject *obj = m_mine->CurrObj ();
#if 0
CRect rc;
m_showTextureWnd.GetClientRect (&rc);
pDC->FillSolidRect (&rc, IMG_BKCOLOR);
#endif
INT16 tnum = 0, tnum2 = -1;

if (obj->render_type != RT_POLYOBJ)
	CBObjTexture ()->SetCurSel (0);
else {
	tnum = (INT16) m_mine->CurrObj ()->rtype.pobj_info.tmap_override;
	if ((tnum < 0) || (tnum >= ((file_type == RDL_FILE) ? MAX_D1_TEXTURES: MAX_D2_TEXTURES))) {
		CBObjTexture ()->SetCurSel (0);
		tnum = 0;	// -> force PaintTexture to clear the texture display window
		}
	else {
		tnum2 = 0;
#if 0
		// texture is overrides, select index in list box
		CDC *pDC = m_showTextureWnd.GetDC ();
		if (!pDC)
			return;
		HINSTANCE hInst = AfxGetApp()->m_hInstance;
		LoadString (hInst,texture_resource + tnum, message, sizeof(message));
		CBObjTexture ()->SelectString (-1, message);
		// and show bitmap
		CDTexture tx (bmBuf);
		if (!DefineTexture(tnum,0,&tx,0,0)) {
			CPalette * pOldPalette = pDC->SelectPalette(thePalette, FALSE);
			pDC->RealizePalette ();
			BITMAPINFO *bmi;
			bmi = MakeBitmap ();
			CRect rc;
			m_showTextureWnd.GetClientRect (rc);
			StretchDIBits (pDC->m_hDC, 0, 0, rc.Width (), rc.Height (), 0, 0, tx.width, tx.height,
								(void *) bmBuf, bmi, DIB_RGB_COLORS,SRCCOPY);
			pDC->SelectPalette(pOldPalette, FALSE);
			}
#endif
		}
	}
#if 0
m_showTextureWnd.ReleaseDC (pDC);
m_showTextureWnd.InvalidateRect (NULL, TRUE);
m_showTextureWnd.UpdateWindow ();
#else
PaintTexture (&m_showTextureWnd, IMG_BKCOLOR, -1, -1, tnum, tnum2);
#endif
}

//------------------------------------------------------------------------
// CObjectTool - DrawObjectImage
//------------------------------------------------------------------------

void CObjectTool::DrawObjectImages () 
{
if (!GetMine ())
	return;
CDObject *obj = m_mine->CurrObj ();
m_mine->DrawObject (&m_showObjWnd, obj->type, obj->id);
m_mine->DrawObject (&m_showSpawnWnd, obj->contains_type, obj->contains_id);
}


//------------------------------------------------------------------------
// CObjectTool - Set Object Id Message
//------------------------------------------------------------------------

int bbb = 1;

void CObjectTool::SetObjectId (CComboBox *pcb, INT16 type, INT16 id, INT16 flag) 
{
char str [40];
int h, i, j;
INT16 max_robot_ids = flag ? (file_type==RDL_FILE) ? ROBOT_IDS1: 64: 
									 (file_type==RDL_FILE) ? ROBOT_IDS1: ROBOT_IDS2;
pcb->ResetContent ();
HINSTANCE hInst = AfxGetApp ()->m_hInstance;
switch(type) {
	case OBJ_ROBOT: /* an evil enemy */
		for (i = 0; i < max_robot_ids; i++) {
			sprintf (string, (i < 10) ? "%3d: ": "%d: ", i);
			h = strlen (string);
			LoadString (hInst, ROBOT_STRING_TABLE + i, string + h, sizeof(string) - h);
			if (!strcmp (string, "(not used)"))
				continue;
			h = CBAddString (pcb, string);
			pcb->SetItemData (h, i);
			}
		if (id < 0 || id >= max_robot_ids) {
			sprintf (message," ObjectTool: Unknown robot id (%d)",id);
			DEBUGMSG (message);
			}
		SelectItemData (pcb, id); // if out of range, nothing is selected
		break;

	case OBJ_HOSTAGE: // a hostage you need to rescue
		for (i = 0; i <= 1; i++) {
			sprintf (str, "%d", i);
			h = pcb->AddString (str);
			pcb->SetItemData (h, i);
			}
		SelectItemData (pcb, id);
		break;

	case OBJ_PLAYER: // the player on the console
		for (i = 0; i <= 7; i++) {
			sprintf (str,"%d",i);
			h = pcb->AddString (str);
			pcb->SetItemData (h, i);
			}
		SelectItemData (pcb, id);
		break;

	case OBJ_WEAPON: //
		h = pcb->AddString ("Mine");
		pcb->SetItemData (0, h);
		pcb->SetCurSel (0);
		break;

	case OBJ_CAMBOT: //
		h = pcb->AddString ("Camera");
		pcb->SetItemData (0, h);
		pcb->SetCurSel (0);
		break;

	case OBJ_MONSTERBALL: //
		h = pcb->AddString ("Monsterball");
		pcb->SetItemData (0, h);
		pcb->SetCurSel (0);
		break;

	case OBJ_EXPLOSION: //
		for (i = 0; i < 5; i++) {
			h = pcb->AddString (pszExplosionIds [i]);
			pcb->SetItemData (h, nExplosionIds [i]);
			}
		SelectItemData (pcb, id); // if out of range, nothing is selected
		break;

	case OBJ_SMOKE: //
		h = pcb->AddString ("Smoke");
		pcb->SetItemData (0, h);
		pcb->SetCurSel (0);
		break;

	case OBJ_EFFECT: //
		h = pcb->AddString ("Effect");
		pcb->SetItemData (0, h);
		pcb->SetCurSel (0);
		break;

	case OBJ_POWERUP: // a powerup you can pick up
		int xlat [100];
		memset (xlat, 0xff, sizeof (xlat));
		h = pcb->AddString ("(none)");
		pcb->SetItemData (h, -1);
		for (i = 0; i < MAX_POWERUP_IDS; i++) {
			j = powerupIdStrXlat [i];
			LoadString (hInst, POWERUP_STRING_TABLE + j, string, sizeof(string));
			if (!strcmp (string, "(not used)"))
				continue;
			h = pcb->AddString (string);
			xlat [i] = j;
			pcb->SetItemData (h, j);
			}
#if 0//def _DEBUG // hack to fix bogus powerup ids
		CDObject *objP;
		for (i = 0, objP = m_mine->Objects (); i < m_mine->ObjCount (); i++, objP++)
			if ((objP->type == OBJ_POWERUP) && (xlat [objP->id] == -1)) {
				for (i = 0, objP = m_mine->Objects (); i < m_mine->ObjCount (); i++, objP++)
					if (objP->type == OBJ_POWERUP)
						objP->id = xlat [objP->id]; 
				break;
				}
#endif
		SelectItemData (pcb, id);
		break;

	case OBJ_CNTRLCEN: // a control center */
		if (file_type == RDL_FILE) {
			for ( i = 0; i <= 25; i++) { //??? not sure of max
				sprintf (str,"%d",i);
				h = pcb->AddString (str);
				pcb->SetItemData (h, i);
				}
			}
		else {
			for (i = 1; i <= 6; i++) {
				sprintf (str,"%d",i);
				h = pcb->AddString (str);
				pcb->SetItemData (h, i);
				}
			}
		SelectItemData (pcb, id);
		break;

	case OBJ_COOP: // a cooperative player object
		for (i = 8; i <= 10; i++) {
			sprintf (str,"%d", i);
			h = pcb->AddString (str);
			pcb->SetItemData (h, i - 8);
			}
		SelectItemData (pcb, id - 8);
		break;

	default:
		h = pcb->AddString ("(none)");
		pcb->SetItemData (0, h);
		pcb->SetCurSel (0);
	break;
	}
}

//------------------------------------------------------------------------
// CObjectTool - WMDrawItem
//------------------------------------------------------------------------

void CObjectTool::OnPaint ()
{
CToolDlg::OnPaint ();
DrawObjectImages ();
SetTextureOverride ();
}

//------------------------------------------------------------------------
// CObjectTool - Add Object Message
//------------------------------------------------------------------------

void CObjectTool::OnAdd () 
{
if (!GetMine ())
	return;
if (m_mine->Current ()->object == m_mine->GameInfo ().objects.count) {
	ErrorMsg ("Cannot add another secret return.");
	return;
 }

if (m_mine->GameInfo ().objects.count >= MAX_OBJECTS) {
	ErrorMsg("Maximum numbers of objects reached");
	return;
	}
m_mine->CopyObject (OBJ_NONE);
Refresh ();
}

//------------------------------------------------------------------------
// CObjectTool - Delete Object Message
//------------------------------------------------------------------------

void CObjectTool::OnDelete ()
{
if (!GetMine ())
	return;
if (m_mine->Current ()->object == m_mine->GameInfo ().objects.count) {
	ErrorMsg("Cannot delete the secret return.");
	return;
	}
if (m_mine->GameInfo ().objects.count == 1) {
	ErrorMsg("Cannot delete the last object");
	return;
	}
if (QueryMsg ("Are you sure you want to delete this object?") == IDYES) {
	m_mine->DeleteObject ();
	Refresh ();
	theApp.MineView ()->Refresh (false);
	}
}

//------------------------------------------------------------------------
// CObjectTool - Delete All (Marked) Triggers
//------------------------------------------------------------------------

void CObjectTool::OnDeleteAll () 
{
if (!GetMine ())
	return;
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
theApp.MineView ()->DelayRefresh (true);
CDObject *obj = m_mine->Objects ();
bool bAll = (m_mine->MarkedSegmentCount (true) > 0);
int nDeleted = 0;
int i;
for (i = m_mine->GameInfo ().objects.count; i; i--, obj++) {
	if (bAll || (m_mine->Segments (obj->segnum)->wall_bitmask &= MARKED_MASK)) {
		m_mine->DeleteObject (i);
		nDeleted++;
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
// CObjectTool - Reset Object Message
//------------------------------------------------------------------------

void CObjectTool::OnReset () 
{
if (!GetMine ())
	return;
vms_matrix *orient;
theApp.SetModified (TRUE);
theApp.LockUndo ();
if (m_mine->Current ()->object == m_mine->GameInfo ().objects.count) {
	orient = &m_mine->SecretOrient ();
	orient->rvec.x = F1_0;
	orient->rvec.y = 0L;
	orient->rvec.z = 0L;
	orient->uvec.x = 0L;
	orient->uvec.y = 0L;
	orient->uvec.z = F1_0;
	orient->fvec.x = 0L;
	orient->fvec.y = F1_0;
	orient->fvec.z = 0L;
} else {
	orient = &m_mine->CurrObj ()->orient;
	orient->rvec.x = F1_0;
	orient->rvec.y = 0L;
	orient->rvec.z = 0L;
	orient->uvec.x = 0L;
	orient->uvec.y = F1_0;
	orient->uvec.z = 0L;
	orient->fvec.x = 0L;
	orient->fvec.y = 0L;
	orient->fvec.z = F1_0;
	}
theApp.UnlockUndo ();
Refresh ();
theApp.MineView ()->Refresh (false);
}

//------------------------------------------------------------------------
// CObjectTool - AdvancedMsg
//------------------------------------------------------------------------

void CObjectTool::OnAdvanced () 
{
}


//------------------------------------------------------------------------
// CObjectTool - Move Object Message
//------------------------------------------------------------------------

void CObjectTool::OnMove ()
{
if (!GetMine ())
	return;
#if 0
if (QueryMsg ("Are you sure you want to move the\n"
				 "current object to the current cube?\n") != IDYES)
	return;
#endif
theApp.SetModified (TRUE);
if (m_mine->Current ()->object == m_mine->GameInfo ().objects.count)
	m_mine->SecretCubeNum () = m_mine->Current ()->segment;
else {
	CDObject *obj = m_mine->CurrObj ();
	m_mine->CalcSegCenter (obj->pos, m_mine->Current ()->segment);
	// bump position over if this is not the first object in the cube
	int i, count = 0;
	for (i = 0; i < m_mine->GameInfo ().objects.count;i++)
		if (m_mine->Objects (i)->segnum == m_mine->Current ()->segment)
			count++;
	obj->pos.y += count*2*F1_0;
	obj->last_pos.y += count*2*F1_0;
	obj->segnum = m_mine->Current ()->segment;
	Refresh ();
	theApp.MineView ()->Refresh (false);
	}
}

//------------------------------------------------------------------------
// CObjectTool - Object Number Message
//------------------------------------------------------------------------

void CObjectTool::OnSetObject ()
{
if (!GetMine ())
	return;
INT16 old_object = m_mine->Current ()->object;
INT16 new_object = CBObjNo ()->GetCurSel ();
theApp.MineView ()->RefreshObject (old_object, new_object);
//Refresh ();
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

bool CObjectTool::SetPlayerId (CDObject *obj, int objType, int *ids, int numIds, char *pszError)
{
CDObject *o = m_mine->Objects ();
int		i, n = 0;

for (i = m_mine->ObjCount (); i && (n < numIds); i--, o++)
	if (o->type == objType)
		ids [n++] = -1;
if (n == numIds) {
	ErrorMsg (pszError);
	return false;
	}
for (i = 0; i < numIds; i++)
	if (ids [i] >= 0) {
		obj->id = ids [i];
		break;
		}
return true;
}

//------------------------------------------------------------------------
// CObjectTool - Type ComboBox Message
//------------------------------------------------------------------------

void CObjectTool::OnSetObjType () 
{
if (!GetMine ())
	return;

CDObject *obj = m_mine->CurrObj ();
int selection = object_list [CBObjType ()->GetCurSel ()];
if ((file_type == RDL_FILE) && (selection == OBJ_WEAPON)) {
	ErrorMsg ("You can not use this type of object in a Descent 1 level");
	return;
	}
if ((selection == OBJ_SMOKE) || (selection == OBJ_EFFECT)) {
	ErrorMsg ("You can use the effects tool to create and edit this type of object");
	return;
	}
// set id
int playerIds [8] = {0,1,2,3,4,5,6,7};
int coopIds [3] = {8,9,10};

switch (selection) {
	case OBJ_PLAYER:
		if (!SetPlayerId (obj, selection, playerIds, 8, "Only 8 players allowed."))
			return;
		break;

	case OBJ_COOP:
		if (!SetPlayerId (obj, selection, coopIds, 3, "Only 3 coop players allowed."))
			return;
		break;

	case OBJ_WEAPON:
		obj->id = WEAPON_MINE;
		break;

	case OBJ_CNTRLCEN:
		obj->id = (file_type == RDL_FILE) ? 0: 2;
		break;

	case OBJ_EXPLOSION:
		obj->id = 0;
		break;

	default:
		obj->id = 0;
	}
obj->type = selection;
SetObjectId (CBObjId (), selection, 0);
m_mine->SetObjectData (obj->type);
Refresh ();
theApp.MineView ()->Refresh (false);
}

//------------------------------------------------------------------------
// CObjectTool - IdMsg Message
//
// This routine resets the size, shield, vclip if the id changes.
//------------------------------------------------------------------------

int CObjectTool::GetObjectsOfAKind (int nType, CDObject *objList [])
{
	int i, nObjects = 0;
	CDObject *obj;

for (i = m_mine->GameInfo ().objects.count, obj = m_mine->Objects (); i; i--, obj++)
	if (obj->type == nType)
		objList [nObjects++] = obj;
return nObjects;
}

//------------------------------------------------------------------------

void CObjectTool::SetNewObjId (CDObject *obj, int nType, int nId, int nMaxId)
{
if (nId = obj->id)
	return;

	int nObjects = ObjOfAKindCount (nType);

CDObject **objList = new CDObject* [nObjects];
GetObjectsOfAKind (nType, objList);
if ((nMaxId > 0) && (nId >= nMaxId)) {
	nId = nMaxId;
	if (nId == obj->id)
		return;
	}
// find object that currently has id nCurSel and swap ids
int i;
for (i = 0; i < nObjects; i++)
	if (objList [i]->id == nId) {
		objList [i]->id = obj->id;
		break;
		}
obj->id = nId;
delete objList;
}

//------------------------------------------------------------------------

void CObjectTool::OnSetObjId ()
{
	int	id;

if (!GetMine ())
	return;

CDObject *obj = m_mine->CurrObj ();
CComboBox *pcb = CBObjId ();
int nCurSel = pcb->GetItemData (pcb->GetCurSel ());

theApp.SetModified (TRUE);
theApp.LockUndo ();
switch (obj->type) {
	case OBJ_PLAYER:
		SetNewObjId (obj, OBJ_PLAYER, nCurSel, 8);
		break;

	case OBJ_COOP:
		SetNewObjId (obj, OBJ_COOP, nCurSel + 8, 3);
		break;

	case OBJ_WEAPON:
		obj->id = WEAPON_MINE;
		break;

	case OBJ_CNTRLCEN:
		obj->id = nCurSel; // + (file_type != RDL_FILE);
		obj->rtype.vclip_info.vclip_num = nCurSel;
		break;

	default:
		obj->id = nCurSel;
	}

switch (obj->type) {
	case OBJ_POWERUP:
		id = (obj->id < MAX_POWERUP_IDS_D2) ? obj->id : POW_AMMORACK;
		obj->size = powerup_size [id];
		obj->shields = DEFAULT_SHIELD;
		obj->rtype.vclip_info.vclip_num = powerup_clip [id];
		break;

	case OBJ_ROBOT:
		obj->size = robot_size [obj->id];
		obj->shields = robot_shield [obj->id];
		obj->rtype.pobj_info.model_num = robot_clip [obj->id];
		break;

	case OBJ_CNTRLCEN:
		obj->size = REACTOR_SIZE;
		obj->shields = REACTOR_SHIELD;
		if (file_type == RDL_FILE)
			obj->rtype.pobj_info.model_num = REACTOR_CLIP_NUMBER;
		else {
			INT32 model;
			switch(obj->id) {
				case 1: model = 95; break;
				case 2: model = 97; break;
				case 3: model = 99; break;
				case 4: model = 101; break;
				case 5: model = 103; break;
				case 6: model = 105; break;
				default: model = 97; break; // level 1's reactor
				}
			obj->rtype.pobj_info.model_num = model;
		}
		break;

	case OBJ_PLAYER:
		obj->size = PLAYER_SIZE;
		obj->shields = DEFAULT_SHIELD;
		obj->rtype.pobj_info.model_num = PLAYER_CLIP_NUMBER;
		break;

	case OBJ_WEAPON:
		obj->size = WEAPON_SIZE;
		obj->shields = WEAPON_SHIELD;
		obj->rtype.pobj_info.model_num = MINE_CLIP_NUMBER;
		break;

	case OBJ_COOP:
		obj->size = PLAYER_SIZE;
		obj->shields = DEFAULT_SHIELD;
		obj->rtype.pobj_info.model_num = COOP_CLIP_NUMBER;
		break;

	case OBJ_HOSTAGE:
		obj->size = PLAYER_SIZE;
		obj->shields = DEFAULT_SHIELD;
		obj->rtype.vclip_info.vclip_num = HOSTAGE_CLIP_NUMBER;
		break;
	}
m_mine->SortObjects ();
SelectItemData (pcb, obj->id);
theApp.UnlockUndo ();
Refresh ();
}

//------------------------------------------------------------------------
// CObjectTool - ContainsQtyMsg
//------------------------------------------------------------------------

void CObjectTool::OnSetSpawnQty ()
{
if (!GetMine ())
	return;
UpdateData (TRUE);
theApp.SetModified (TRUE);
m_mine->CurrObj ()->contains_count = m_nSpawnQty;
Refresh ();
}

//------------------------------------------------------------------------
// CObjectTool - Container Type ComboBox Message
//------------------------------------------------------------------------

void CObjectTool::OnSetSpawnType () 
{
if (!GetMine ())
	return;
CDObject *obj = m_mine->CurrObj ();
int selection;
theApp.SetModified (TRUE);
theApp.UnlockUndo ();
int i = CBSpawnType ()->GetCurSel () - 1;
if ((i < 0) || (i == MAX_CONTAINS_NUMBER)) {
	obj->contains_count = 0;
	obj->contains_type = -1;
	obj->contains_id = -1;
	}
else {
	obj->contains_type = 
	selection = contains_list [i];
	SetObjectId (CBSpawnId (),selection,0,1);
	UpdateData (TRUE);
	if (m_nSpawnQty < 1) {
		m_nSpawnQty = 1;
		UpdateData (FALSE);
		}
	OnSetSpawnQty ();
	OnSetSpawnId ();
	}
theApp.LockUndo ();
}

//------------------------------------------------------------------------
// CObjectTool - ContainsIdMsg
//------------------------------------------------------------------------

void CObjectTool::OnSetSpawnId () 
{
if (!GetMine ())
	return;
CDObject *obj = m_mine->CurrObj ();

theApp.SetModified (TRUE);
if (obj->contains_count < -1)
	obj->contains_count = -1;
int i = CBSpawnType ()->GetCurSel () - 1;
if ((i > -1) && (obj->contains_count > 0)) {
	obj->contains_type = contains_list [i];
	obj->contains_id = (INT8) CBSpawnId ()->GetItemData (CBSpawnId ()->GetCurSel ());
	}
else {
	obj->contains_type = -1;
	obj->contains_id = -1;
	}
Refresh ();
theApp.SetModified (TRUE);
}

//------------------------------------------------------------------------
// CObjectTool - Options ComboBox Message
//------------------------------------------------------------------------

void CObjectTool::OnSetObjAI ()
{
if (!GetMine ())
	return;
theApp.SetModified (TRUE);
CDObject *obj = m_mine->CurrObj ();
if ((obj->type == OBJ_ROBOT) || (obj->type == OBJ_CAMBOT)) {
 	int index = CBObjAI ()->GetCurSel ();
	if (index == 8) {
		index = AIB_RUN_FROM;
		obj->ctype.ai_info.flags [4] |= 2; // smart bomb flag
		}
	else {
		index += 0x80;
		obj->ctype.ai_info.flags [4] &= ~2;
		}
	obj->ctype.ai_info.behavior = index;
	}
else
	CBObjAI ()->SetCurSel (1); // Normal
Refresh ();
theApp.SetModified (TRUE);
}

//------------------------------------------------------------------------
// CObjectTool - Texture ComboBox Message
//------------------------------------------------------------------------

void CObjectTool::OnSetTexture ()
{
if (!GetMine ())
	return;
CDObject *obj = m_mine->CurrObj ();

if (obj->render_type == RT_POLYOBJ) {
	theApp.SetModified (TRUE);
	int index = CBObjTexture ()->GetCurSel ();
	obj->rtype.pobj_info.tmap_override = 
		(index > 0) ? (INT16)CBObjTexture ()->GetItemData (index): -1;
	Refresh ();
	}
}

//------------------------------------------------------------------------
// CObjectTool - Texture ComboBox Message
//------------------------------------------------------------------------

void CObjectTool::OnDefault ()
{
if (object_list [CBObjType ()->GetCurSel ()] != OBJ_ROBOT)
	return;
if (!GetMine ())
	return;
int i = CBObjId ()->GetItemData (CBObjId ()->GetCurSel ());
memcpy (m_mine->RobotInfo (i), m_mine->DefRobotInfo (i), sizeof (ROBOT_INFO));
Refresh ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetSoundExplode ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetSoundSee ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetSoundAttack ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetSoundClaw ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetSoundDeath ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetObjClassAI ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetWeapon1 ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetWeapon2 ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetContType ()
{
if (!GetMine ())
	return;
int i = CBContType ()->GetCurSel ();
if (0 > i)
	return;
int j = CBObjId ()->GetItemData (CBObjId ()->GetCurSel ());
ROBOT_INFO *rInfo = m_mine->RobotInfo (j);
rInfo->contains_type = (UINT8) CBContType ()->GetItemData (i);
RefreshRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetContId ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnAIKamikaze ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnAICompanion ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnAIThief ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnAISmartBlobs ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnAIPursue ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnAICharge ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnAIEDrain ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnAIBossType ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnBright ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnCloaked ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnMultiplayer ()
{
if (!GetMine ())
	return;
CDObject *obj = m_mine->CurrObj ();
theApp.SetModified (TRUE);
theApp.UnlockUndo ();

int i = CBSpawnType ()->GetCurSel () - 1;
if ((i < 0) || (i == MAX_CONTAINS_NUMBER)) {
	obj->contains_count = 0;
	obj->contains_type = -1;
	obj->contains_id = -1;
	}
else {
	obj->contains_type = 
	selection = contains_list [i];
	SetObjectId (CBSpawnId (),selection,0,1);
	UpdateData (TRUE);
	if (m_nSpawnQty < 1) {
		m_nSpawnQty = 1;
		UpdateData (FALSE);
		}
	OnSetSpawnQty ();
	OnSetSpawnId ();
	}
theApp.LockUndo ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSort ()
{
if (!GetMine ())
	return;
if (m_mine->m_bSortObjects = BtnCtrl (IDC_OBJ_SORT)->GetCheck ()) {
	m_mine->SortObjects ();
	Refresh ();
	}
}

                        /*--------------------------*/

int CObjectTool::ObjOfAKindCount (int nType, int nId)
{
if (!GetMine ())
	return 0;
if (nType < 0)
	nType = m_mine->CurrObj ()->type;
if (nId < 0)
	nId =  m_mine->CurrObj ()->id;
int nCount = 0;
CDObject *obj = m_mine->Objects ();
int i;
for (i = m_mine->GameInfo ().objects.count; i; i--, obj++)
	if ((obj->type == nType) && ((obj->type == OBJ_PLAYER) || (obj->type == OBJ_COOP) || (obj->id == nId))) 
		nCount++;
return nCount;
}

                        /*--------------------------*/

int fix_log(FIX x) 
{
return (x >= 1) ? (int) (log ((double) x) + 0.5): 0; // round (assume value is positive)
}

                        /*--------------------------*/

FIX fix_exp(int x) 
{
return (x >= 0 && x <= 21) ? (FIX) (exp ((double) x) + 0.5): 1; // round (assume value is positive)
}

                        /*--------------------------*/

//eof objectdlg.cpp