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

BEGIN_MESSAGE_MAP (CEffectTool, CToolDlg)
	ON_WM_HSCROLL ()
#if 1
	ON_CBN_SELCHANGE (IDC_SMOKE_LIFE, OnEdit)
	ON_CBN_SELCHANGE (IDC_SMOKE_SIZE, OnEdit)
	ON_CBN_SELCHANGE (IDC_SMOKE_DENSITY, OnEdit)
	ON_CBN_SELCHANGE (IDC_SMOKE_SPEED, OnEdit)
	ON_CBN_SELCHANGE (IDC_SMOKE_DRIFT, OnEdit)
	ON_CBN_SELCHANGE (IDC_SMOKE_BRIGHTNESS, OnEdit)
	ON_CBN_SELCHANGE (IDC_SMOKE_TYPE, OnEdit)
	ON_EN_KILLFOCUS (IDC_SMOKE_SIDE, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_ID, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_TARGET, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_BOLTS, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_NODES, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_CHILDREN, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_LIFE, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_DELAY, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_LENGTH, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_AMPLITUDE, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_ANGLE, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_OFFSET, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_SPEED, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_RED, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_GREEN, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_BLUE, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_ALPHA, OnEdit)
	ON_BN_CLICKED (IDC_LIGHTNING_SMOOTHE, OnEdit)
	ON_BN_CLICKED (IDC_LIGHTNING_CLAMP, OnEdit)
	ON_BN_CLICKED (IDC_LIGHTNING_SOUND, OnEdit)
	ON_BN_CLICKED (IDC_LIGHTNING_PLASMA, OnEdit)
	ON_BN_CLICKED (IDC_LIGHTNING_RANDOM, OnEdit)
#endif
	ON_BN_CLICKED (IDC_SMOKE_ADD, OnAddSmoke)
	ON_BN_CLICKED (IDC_LIGHTNING_ADD, OnAddLightning)
	ON_BN_CLICKED (IDC_EFFECT_DELETE, OnDelete)
	ON_BN_CLICKED (IDC_EFFECT_COPY, OnCopy)
	ON_BN_CLICKED (IDC_EFFECT_PASTE, OnPaste)
	ON_BN_CLICKED (IDC_EFFECT_PASTE_ALL, OnPasteAll)
	ON_CBN_SELCHANGE (IDC_EFFECT_OBJECTS, OnSetObject)
	ON_CBN_SELCHANGE (IDC_LIGHTNING_STYLE, OnSetStyle)
END_MESSAGE_MAP ()

//------------------------------------------------------------------------
// DIALOG - CEffectTool (constructor)
//------------------------------------------------------------------------

CEffectTool::CEffectTool (CPropertySheet *pParent)
	: CToolDlg (nLayout ? IDD_EFFECTDATA2 : IDD_EFFECTDATA, pParent)
{
Reset ();
}

								/*--------------------------*/

CEffectTool::~CEffectTool ()
{
}

								/*--------------------------*/

void CEffectTool::Reset ()
{
m_nBufferId = -1;
}

                        /*--------------------------*/

void CEffectTool::LoadEffectList () 
{
	HINSTANCE	hInst = AfxGetApp()->m_hInstance;
	CComboBox	*cbEffects = CBEffects ();
	char			szEffect [100];
	int			index, curSel = 0;

GetMine ();

cbEffects->ResetContent ();
CDObject *curObj = m_mine->CurrObj (),
			*obj = m_mine->Objects ();
int i;
for (i = 0; i < m_mine->GameInfo ().objects.count; i++, obj++) {
	if (obj->type != OBJ_EFFECT)
		continue;
	if (obj == curObj)
		curSel = i;
	if (obj->id == SMOKE_ID)
		sprintf (szEffect, "Smoke");
	else if (obj->id == LIGHTNING_ID)
		sprintf (szEffect, "Lightning %d (%d)", obj->rtype.lightningInfo.nId, i);
	else
		continue;
	index = cbEffects->AddString (szEffect);
	cbEffects->SetItemData (index, i);
	}
SelectItemData (cbEffects, curSel);
}

                        /*--------------------------*/

BOOL CEffectTool::OnInitDialog ()
{
CToolDlg::OnInitDialog ();
int i, nId;
for (nId = IDC_SMOKE_LIFE; nId <= IDC_SMOKE_BRIGHTNESS; nId++) {
	InitSlider (nId, 1, 10);
	for (i = 1; i <= 10; i++)
		SlCtrl (nId)->SetTic (i);
	}
CComboBox *pcb = CBStyle ();
pcb->AddString ("automatic");
pcb->AddString ("erratic");
pcb->AddString ("jaggy");
pcb->AddString ("smoothe");
pcb->SetCurSel (0);
pcb = CBType ();
pcb->AddString ("Smoke");
pcb->AddString ("Spray");
pcb->AddString ("Bubbles");
pcb->SetCurSel (0);
m_bInited = TRUE;
return TRUE;
}

								/*--------------------------*/

void CEffectTool::DoDataExchange (CDataExchange *pDX)
{
if (!m_bInited)
	return;
if (!GetMine ())
	return;
CDObject *obj = m_mine->CurrObj ();
if (obj->type != OBJ_EFFECT)
	return;
if (obj->id == SMOKE_ID) {
	DDX_Slider (pDX, IDC_SMOKE_LIFE, obj->rtype.smokeInfo.nLife);
	DDX_Slider (pDX, IDC_SMOKE_SIZE, obj->rtype.smokeInfo.nSize [0]);
	DDX_Slider (pDX, IDC_SMOKE_DENSITY, obj->rtype.smokeInfo.nParts);
	DDX_Slider (pDX, IDC_SMOKE_SPEED, obj->rtype.smokeInfo.nSpeed);
	DDX_Slider (pDX, IDC_SMOKE_DRIFT, obj->rtype.smokeInfo.nDrift);
	DDX_Slider (pDX, IDC_SMOKE_BRIGHTNESS, obj->rtype.smokeInfo.nBrightness);
	int i;
	for (i = 0; i < 4; i++)
		DDX_Text (pDX, IDC_SMOKE_RED + i, obj->rtype.smokeInfo.color [i]);
	obj->rtype.smokeInfo.nSide = (char) DDX_Int (pDX, IDC_SMOKE_SIDE, (int) obj->rtype.smokeInfo.nSide);
	if (pDX->m_bSaveAndValidate)
		obj->rtype.smokeInfo.nType = CBType ()->GetCurSel ();
	else
		CBType ()->SetCurSel (obj->rtype.smokeInfo.nType);
	}
else if (obj->id == LIGHTNING_ID) {
	obj->rtype.lightningInfo.nId = DDX_Int (pDX, IDC_LIGHTNING_ID, obj->rtype.lightningInfo.nId);
	obj->rtype.lightningInfo.nTarget = DDX_Int (pDX, IDC_LIGHTNING_TARGET, obj->rtype.lightningInfo.nTarget);
	obj->rtype.lightningInfo.nLightnings = DDX_Int (pDX, IDC_LIGHTNING_BOLTS, obj->rtype.lightningInfo.nLightnings);
	obj->rtype.lightningInfo.nNodes = DDX_Int (pDX, IDC_LIGHTNING_NODES, obj->rtype.lightningInfo.nNodes);
	obj->rtype.lightningInfo.nChildren = DDX_Int (pDX, IDC_LIGHTNING_CHILDREN, obj->rtype.lightningInfo.nChildren);
	obj->rtype.lightningInfo.nLife = DDX_Int (pDX, IDC_LIGHTNING_LIFE, obj->rtype.lightningInfo.nLife);
	obj->rtype.lightningInfo.nDelay = DDX_Int (pDX, IDC_LIGHTNING_DELAY, obj->rtype.lightningInfo.nDelay);
	obj->rtype.lightningInfo.nLength = DDX_Int (pDX, IDC_LIGHTNING_LENGTH, obj->rtype.lightningInfo.nLength);
	obj->rtype.lightningInfo.nAmplitude = DDX_Int (pDX, IDC_LIGHTNING_AMPLITUDE, obj->rtype.lightningInfo.nAmplitude);
	obj->rtype.lightningInfo.nSteps = DDX_Int (pDX, IDC_LIGHTNING_SPEED, obj->rtype.lightningInfo.nSteps);
	obj->rtype.lightningInfo.nAngle = DDX_Int (pDX, IDC_LIGHTNING_ANGLE, obj->rtype.lightningInfo.nAngle);
	obj->rtype.lightningInfo.nOffset = DDX_Int (pDX, IDC_LIGHTNING_OFFSET, obj->rtype.lightningInfo.nOffset);
	int i;
	for (i = 0; i < 4; i++)
		DDX_Text (pDX, IDC_LIGHTNING_RED + i, obj->rtype.lightningInfo.color [i]);
	obj->rtype.lightningInfo.nSmoothe = DDX_Flag (pDX, IDC_LIGHTNING_SMOOTHE, obj->rtype.lightningInfo.nSmoothe);
	obj->rtype.lightningInfo.bClamp = DDX_Flag (pDX, IDC_LIGHTNING_CLAMP, obj->rtype.lightningInfo.bClamp);
	obj->rtype.lightningInfo.bSound = DDX_Flag (pDX, IDC_LIGHTNING_SOUND, obj->rtype.lightningInfo.bSound);
	obj->rtype.lightningInfo.bPlasma = DDX_Flag (pDX, IDC_LIGHTNING_PLASMA, obj->rtype.lightningInfo.bPlasma);
	obj->rtype.lightningInfo.bRandom = DDX_Flag (pDX, IDC_LIGHTNING_RANDOM, obj->rtype.lightningInfo.bRandom);
	obj->rtype.lightningInfo.bInPlane = DDX_Flag (pDX, IDC_LIGHTNING_INPLANE, obj->rtype.lightningInfo.bInPlane);
	if (pDX->m_bSaveAndValidate)
		obj->rtype.lightningInfo.nStyle = CBStyle ()->GetCurSel () - 1;
	else {
		CBStyle ()->SetCurSel (obj->rtype.lightningInfo.nStyle + 1);
		HiliteTarget ();
		}
	}
}

								/*--------------------------*/

BOOL CEffectTool::OnSetActive ()
{
Refresh ();
return TRUE; //CTexToolDlg::OnSetActive ();
}

								/*--------------------------*/

BOOL CEffectTool::OnKillActive ()
{
Refresh ();
return CToolDlg::OnKillActive ();
}

								/*--------------------------*/

void CEffectTool::EnableControls (BOOL bEnable)
{
if (!m_bInited)
	return;
if (!GetMine ())
	return;
CDObject *obj = m_mine->CurrObj ();
CToolDlg::EnableControls (IDC_SMOKE_LIFE, IDC_SMOKE_BRIGHTNESS, (obj->type == OBJ_EFFECT) && (obj->id == SMOKE_ID));
CToolDlg::EnableControls (IDC_LIGHTNING_ID, IDC_LIGHTNING_RANDOM, (obj->type == OBJ_EFFECT) && (obj->id == LIGHTNING_ID));
}

//------------------------------------------------------------------------
// CEffectTool - RefreshData
//------------------------------------------------------------------------

void CEffectTool::Refresh ()
{
if (!m_bInited)
	return;
if (!GetMine ())
	return;

EnableControls (true);
LoadEffectList ();
UpdateData (FALSE);
}

//------------------------------------------------------------------------
// CEffectTool - Add Effect
//------------------------------------------------------------------------

bool CEffectTool::AddEffect ()
{
if (m_mine->GameInfo ().objects.count >= MAX_OBJECTS) {
	ErrorMsg("Maximum numbers of objects reached");
	return false;
	}
UpdateData (TRUE);
m_mine->CopyObject (OBJ_EFFECT);
return true;
}

//------------------------------------------------------------------------

void CEffectTool::OnAddSmoke ()
{
if (!GetMine ())
	return;
if (!AddEffect ())
	return;
CDObject *obj = m_mine->CurrObj ();
obj->type = OBJ_EFFECT;
obj->id = SMOKE_ID;
obj->render_type = RT_SMOKE;
memset (&obj->rtype.smokeInfo, 0, sizeof (obj->rtype.smokeInfo));
Refresh ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::OnAddLightning ()
{
if (!GetMine ())
	return;
if (!AddEffect ())
	return;
CDObject *obj = m_mine->CurrObj ();
obj->type = OBJ_EFFECT;
obj->id = LIGHTNING_ID;
obj->render_type = RT_LIGHTNING;
memset (&obj->rtype.lightningInfo, 0, sizeof (obj->rtype.lightningInfo));
Refresh ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::OnDelete ()
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
if (m_mine->CurrObj ()->type != OBJ_EFFECT) {
	ErrorMsg("No effect object currently selected");
	return;
	}
if (QueryMsg ("Are you sure you want to delete this object?") == IDYES) {
	m_mine->DeleteObject ();
	Refresh ();
	theApp.MineView ()->Refresh (false);
	}
}

//------------------------------------------------------------------------

void CEffectTool::OnCopy ()
{
if (!GetMine ())
	return;
CDObject *obj = m_mine->CurrObj ();
if (obj->type != OBJ_EFFECT) {
	ErrorMsg ("No effect object currently selected");
	return;
	}
m_nBufferId = obj->id;
if (m_nBufferId == SMOKE_ID)
	m_smoke = obj->rtype.smokeInfo;
else if (m_nBufferId == LIGHTNING_ID)
	m_lightning = obj->rtype.lightningInfo;
}

//------------------------------------------------------------------------

void CEffectTool::OnPaste ()
{
if (!GetMine ())
	return;
CDObject *obj = m_mine->CurrObj ();
if (obj->type != OBJ_EFFECT) {
	ErrorMsg ("No effect object currently selected");
	return;
	}
if (obj->id != m_nBufferId) {
	ErrorMsg ("No effect data of that type currently available (copy data first)");
	return;
	}
if (obj->id == SMOKE_ID)
	obj->rtype.smokeInfo = m_smoke;
else if (obj->id == LIGHTNING_ID)
	obj->rtype.lightningInfo = m_lightning;
Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::OnPasteAll ()
{
if (m_nBufferId < 0) {
	ErrorMsg ("No effect data currently available (copy data first)");
	return;
	}
if (!GetMine ())
	return;
CDObject *obj = m_mine->Objects ();
boolean bAll = !m_mine->GotMarkedSegments ();

int i;
for (i = m_mine->ObjCount (); i; i--, obj++)
	if ((obj->type == OBJ_EFFECT) && (obj->id == m_nBufferId) && (bAll || m_mine->SegmentIsMarked (obj->segnum)))
		if (m_nBufferId == SMOKE_ID)
			obj->rtype.smokeInfo = m_smoke;
		else if (m_nBufferId == LIGHTNING_ID)
			obj->rtype.lightningInfo = m_lightning;
Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::OnSetObject ()
{
if (!GetMine ())
	return;
INT16 nOld = m_mine->Current ()->object;
INT16 nNew = CBEffects ()->GetItemData (CBEffects ()->GetCurSel ());
if (nOld != nNew) {
	UpdateData (TRUE);
	theApp.MineView ()->RefreshObject (nOld, nNew);
	HiliteTarget ();
	}
//Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::OnSetStyle ()
{
if (!GetMine ())
	return;
CDObject *obj = m_mine->CurrObj ();
obj->rtype.lightningInfo.nStyle = CBStyle ()->GetCurSel () - 1;
//Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::HiliteTarget (void)
{
#if 0
	int i, nTarget;

if (!GetMine ())
	return;
CDObject *obj = m_mine->CurrObj ();
if ((obj->type != OBJ_EFFECT) || (obj->id != LIGHTNING_ID))
	return;
m_mine->Other ()->object = m_mine->Current ()->object;
if (nTarget = obj->rtype.lightningInfo.nTarget)
	for (i = 0, obj = m_mine->Objects (); i < m_mine->GameInfo ().objects.count; i++, obj++)
		if ((obj->type == OBJ_EFFECT) && (obj->id == LIGHTNING_ID) && (obj->rtype.lightningInfo.nId == nTarget)) {
			m_mine->Other ()->object = i;
			break;
			return;
			}
theApp.MineView ()->Refresh ();
#endif
}
 
//------------------------------------------------------------------------

void CEffectTool::OnEdit (void)
{
UpdateData (TRUE);
}

                       /*--------------------------*/

bool CEffectTool::FindSlider (CScrollBar *pScrollBar)
{
int i;
for (i = IDC_SMOKE_LIFE; i <= IDC_SMOKE_BRIGHTNESS; i++)
	if (pScrollBar == (CScrollBar *) GetDlgItem (i))
		return true;
return false;
}

								/*--------------------------*/

void CEffectTool::OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
if (FindSlider (pScrollBar))
	UpdateData (TRUE);
}

//eof effectdlg.cpp