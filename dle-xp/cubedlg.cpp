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
#include "mine.h"
#include "dlc.h"
#include "global.h"

                        /*--------------------------*/

BEGIN_MESSAGE_MAP (CCubeTool, CToolDlg)
	ON_BN_CLICKED (IDC_CUBE_SETCOORD, OnSetCoord)
	ON_BN_CLICKED (IDC_CUBE_RESETCOORD, OnResetCoord)
	ON_BN_CLICKED (IDC_CUBE_ADD, OnAddCube)
	ON_BN_CLICKED (IDC_CUBE_ADD_MATCEN, OnAddBotGen)
	ON_BN_CLICKED (IDC_CUBE_ADD_EQUIPMAKER, OnAddEquipGen)
	ON_BN_CLICKED (IDC_CUBE_ADD_FUELCEN, OnAddFuelCen)
	ON_BN_CLICKED (IDC_CUBE_ADD_REPAIRCEN, OnAddRepairCen)
	ON_BN_CLICKED (IDC_CUBE_ADD_CONTROLCEN, OnAddControlCen)
	ON_BN_CLICKED (IDC_CUBE_ADD_WATER, OnAddWaterCube)
	ON_BN_CLICKED (IDC_CUBE_ADD_LAVA, OnAddLavaCube)
	ON_BN_CLICKED (IDC_CUBE_SPLIT, OnSplitCube)
	ON_BN_CLICKED (IDC_CUBE_DEL, OnDeleteCube)
	ON_BN_CLICKED (IDC_CUBE_OTHER, OnOtherCube)
	ON_BN_CLICKED (IDC_CUBE_ENDOFEXIT, OnEndOfExit)
	ON_BN_CLICKED (IDC_CUBE_SIDE1, OnSide1)
	ON_BN_CLICKED (IDC_CUBE_SIDE2, OnSide2)
	ON_BN_CLICKED (IDC_CUBE_SIDE3, OnSide3)
	ON_BN_CLICKED (IDC_CUBE_SIDE4, OnSide4)
	ON_BN_CLICKED (IDC_CUBE_SIDE5, OnSide5)
	ON_BN_CLICKED (IDC_CUBE_SIDE6, OnSide6)
	ON_BN_CLICKED (IDC_CUBE_POINT1, OnPoint1)
	ON_BN_CLICKED (IDC_CUBE_POINT2, OnPoint2)
	ON_BN_CLICKED (IDC_CUBE_POINT3, OnPoint3)
	ON_BN_CLICKED (IDC_CUBE_POINT4, OnPoint4)
	ON_BN_CLICKED (IDC_CUBE_ADDBOT, OnAddObj)
	ON_BN_CLICKED (IDC_CUBE_DELBOT, OnDeleteObj)
	ON_BN_CLICKED (IDC_CUBE_TRIGGERDETAILS, OnTriggerDetails)
	ON_BN_CLICKED (IDC_CUBE_WALLDETAILS, OnWallDetails)
	ON_CBN_SELCHANGE (IDC_CUBE_CUBENO, OnSetCube)
	ON_CBN_SELCHANGE (IDC_CUBE_TYPE, OnSetType)
	ON_CBN_SELCHANGE (IDC_CUBE_OWNER, OnSetOwner)
	ON_EN_KILLFOCUS (IDC_CUBE_LIGHT, OnLight)
	ON_EN_KILLFOCUS (IDC_CUBE_GROUP, OnSetGroup)
	ON_EN_UPDATE (IDC_CUBE_LIGHT, OnLight)
	ON_LBN_DBLCLK (IDC_CUBE_TRIGGERS, OnTriggerDetails)
END_MESSAGE_MAP ()

                        /*--------------------------*/

CCubeTool::CCubeTool (CPropertySheet *pParent)
	: CToolDlg (nLayout ? IDD_CUBEDATA2 : IDD_CUBEDATA, pParent)
{
Reset ();
}

                        /*--------------------------*/

void CCubeTool::Reset ()
{
m_nCube =
m_nSide =
m_nPoint = 0;
m_nType = 0;
m_nVertex = 0;
m_bEndOfExit = 0;
m_nLight = 0;
m_nLastCube =
m_nLastSide = -1;
m_bSetDefTexture = 0;
if (GetMine ()) {
	m_nOwner = m_mine->Segments ()->owner;
	m_nGroup = m_mine->Segments ()->group;
	}
else {
	m_nOwner = -1;
	m_nGroup = -1;
	}
memset (m_nCoord, 0, sizeof (m_nCoord));
}

                        /*--------------------------*/

void CCubeTool::InitCBCubeNo ()
{
if (!GetMine ())
	return;
CComboBox *pcb = CBCubeNo ();
if (m_mine->SegCount () != pcb->GetCount ()) {
	pcb->ResetContent ();
	int i;
	for (i = 0; i < m_mine->SegCount (); i++)
		pcb->AddString (itoa (i, message, 10));
	}
pcb->SetCurSel (m_nCube);
}

                        /*--------------------------*/

BOOL CCubeTool::OnInitDialog ()
{
CToolDlg::OnInitDialog ();
CComboBox *pcb = CBType ();
pcb->AddString ("Normal");
pcb->AddString ("Fuel Center");
pcb->AddString ("Repair Center");
pcb->AddString ("Reactor");
pcb->AddString ("Robot Maker");
//if (file_type != RDL_FILE) {
	pcb->AddString ("Blue Goal");
	pcb->AddString ("Red Goal");
	pcb->AddString ("Water");
	pcb->AddString ("Lava");
	pcb->AddString ("Blue Team");
	pcb->AddString ("Red Team");
	pcb->AddString ("Speed Boost");
	pcb->AddString ("Blocked");
	pcb->AddString ("No Damage");
	pcb->AddString ("Sky Box");
	pcb->AddString ("Equip Maker");
	pcb->AddString ("Outdoor");
//	if (level_version >= 9) {
		pcb = CBOwner ();
		pcb->AddString ("Neutral");
		pcb->AddString ("Unowned");
		pcb->AddString ("Blue Team");
		pcb->AddString ("Red Team");
//		}
//	}
m_bInited = TRUE;
return TRUE;
}

                        /*--------------------------*/

void CCubeTool::DoDataExchange (CDataExchange *pDX)
{
if (!GetMine ())
	return;

DDX_CBIndex (pDX, IDC_CUBE_CUBENO, m_nCube);
DDX_CBIndex (pDX, IDC_CUBE_TYPE, m_nType);
DDX_Double (pDX, IDC_CUBE_LIGHT, m_nLight);
DDX_Radio (pDX, IDC_CUBE_SIDE1, m_nSide);
DDX_Radio (pDX, IDC_CUBE_POINT1, m_nPoint);
for (int	i = 0; i < 3; i++) {
	DDX_Double (pDX, IDC_CUBE_POINTX + i, m_nCoord [i]);
	if (m_nCoord [i] < -0x7fff)
		m_nCoord [i] = -0x7fff;
	else if (m_nCoord [i] > 0x7fff)
		m_nCoord [i] = 0x7fff;
//	DDV_MinMaxInt (pDX, (long) m_nCoord [i], -0x7fff, 0x7fff);
	}
DDX_Check (pDX, IDC_CUBE_ENDOFEXIT, m_bEndOfExit);
DDX_Check (pDX, IDC_CUBE_SETDEFTEXTURE, m_bSetDefTexture);
++m_nOwner;
DDX_CBIndex (pDX, IDC_CUBE_OWNER, m_nOwner);
m_nOwner--;
DDX_Text (pDX, IDC_CUBE_GROUP, m_nGroup);
if (m_nGroup < -1)
	m_nGroup = -1;
else if (m_nGroup > 127)
	m_nGroup = 127;
//DDV_MinMaxInt (pDX, m_nGroup, -1, 127);
}


                        /*--------------------------*/

BOOL CCubeTool::OnSetActive ()
{
Refresh ();
return CToolDlg::OnSetActive ();
}

                        /*--------------------------*/

bool CCubeTool::IsBotMaker (CDSegment *seg)
{
return 
	(seg->special == SEGMENT_IS_ROBOTMAKER) &&
	(seg->matcen_num >= 0) &&
	(seg->matcen_num < m_mine->GameInfo ().botgen.count);
}

                        /*--------------------------*/

bool CCubeTool::IsEquipMaker (CDSegment *seg)
{
return 
	(seg->special == SEGMENT_IS_EQUIPMAKER) &&
	(seg->matcen_num >= 0) &&
	(seg->matcen_num < m_mine->GameInfo ().equipgen.count);
}

                        /*--------------------------*/

void CCubeTool::EnableControls (BOOL bEnable)
{
if (!GetMine ())
	return;
CDSegment *seg = m_mine->CurrSeg ();
// enable/disable "end of exit tunnel" button
EndOfExit ()->EnableWindow (seg->children [m_nSide] < 0);
// enable/disable add cube button
GetDlgItem (IDC_CUBE_ADD)->EnableWindow ((m_mine->SegCount () < MAX_SEGMENTS) &&
													  (m_mine->VertCount () < MAX_VERTICES - 4) &&
													  (seg->children [m_nSide] < 0));
GetDlgItem (IDC_CUBE_DEL)->EnableWindow (m_mine->SegCount () > 1);
// enable/disable add robot button
GetDlgItem (IDC_CUBE_ADDBOT)->EnableWindow ((IsBotMaker (seg) || IsEquipMaker (seg)) && (LBAvailBots ()->GetCount () > 0));
GetDlgItem (IDC_CUBE_DELBOT)->EnableWindow ((IsBotMaker (seg) || IsEquipMaker (seg)) && (LBUsedBots ()->GetCount () > 0));
GetDlgItem (IDC_CUBE_WALLDETAILS)->EnableWindow (LBTriggers ()->GetCount () > 0);
GetDlgItem (IDC_CUBE_TRIGGERDETAILS)->EnableWindow (LBTriggers ()->GetCount () > 0);
GetDlgItem (IDC_CUBE_ADD_REPAIRCEN)->EnableWindow (file_type != RDL_FILE);
GetDlgItem (IDC_CUBE_OWNER)->EnableWindow (level_version >= 9);
GetDlgItem (IDC_CUBE_GROUP)->EnableWindow (level_version >= 9);
}

                        /*--------------------------*/

void CCubeTool::OnSetCoord ()
{
if (!GetMine ())
	return;
UpdateData (TRUE);
theApp.SetModified (TRUE);
m_nVertex = m_mine->CurrSeg ()->verts[side_vert[m_mine->Current ()->side][m_mine->Current ()->point]];
m_mine->Vertices (m_nVertex)->x = (FIX) (m_nCoord [0] * 0x10000L);
m_mine->Vertices (m_nVertex)->y = (FIX) (m_nCoord [1] * 0x10000L);
m_mine->Vertices (m_nVertex)->z = (FIX) (m_nCoord [2] * 0x10000L);
theApp.MineView ()->Refresh (false);
}

                        /*--------------------------*/

void CCubeTool::OnResetCoord ()
{
if (!GetMine ())
	return;
m_nVertex = m_mine->CurrSeg ()->verts[side_vert[m_mine->Current ()->side][m_mine->Current ()->point]];
m_nCoord [0] = (double) m_mine->Vertices (m_nVertex)->x / 0x10000L;
m_nCoord [1] = (double) m_mine->Vertices (m_nVertex)->y / 0x10000L;
m_nCoord [2] = (double) m_mine->Vertices (m_nVertex)->z / 0x10000L;
UpdateData (FALSE);
theApp.MineView ()->Refresh (false);
}

                        /*--------------------------*/

void CCubeTool::OnSide (int nSide)
{
if (!GetMine ())
	return;
m_mine->Current ()->side = m_nSide = nSide;
theApp.MineView ()->Refresh ();
}

void CCubeTool::OnSide1 () { OnSide (0); }
void CCubeTool::OnSide2 () { OnSide (1); }
void CCubeTool::OnSide3 () { OnSide (2); }
void CCubeTool::OnSide4 () { OnSide (3); }
void CCubeTool::OnSide5 () { OnSide (4); }
void CCubeTool::OnSide6 () { OnSide (5); }

                        /*--------------------------*/

void CCubeTool::OnPoint (int nPoint)
{
if (!GetMine ())
	return;
m_mine->Current ()->point = m_nPoint = nPoint;
theApp.MineView ()->Refresh ();
}

void CCubeTool::OnPoint1 () { OnPoint (0); }
void CCubeTool::OnPoint2 () { OnPoint (1); }
void CCubeTool::OnPoint3 () { OnPoint (2); }
void CCubeTool::OnPoint4 () { OnPoint (3); }

                        /*--------------------------*/

void CCubeTool::SetDefTexture (INT16 tmapnum)
{
CDSegment *seg = m_mine->Segments () + m_nCube;
if (m_bSetDefTexture = ((CButton *) GetDlgItem (IDC_CUBE_SETDEFTEXTURE))->GetCheck ()) {
	int i;
	for (i = 0; i < 6; i++)
		if (seg->children [i] == -1)
			m_mine->SetTexture (m_nCube, i, tmapnum, 0);
	}
}

//------------------------------------------------------------------------
// CCubeTool - RefreshData
//------------------------------------------------------------------------

void CCubeTool::Refresh () 
{
if (!m_bInited)
	return;
if (!GetMine ())
	return;
InitCBCubeNo ();
OnResetCoord ();

int h, i, j;

// update automatic data
m_mine->RenumberBotGens ();
m_mine->RenumberEquipGens ();
// update cube number combo box if number of cubes has changed
CDSegment *seg = m_mine->CurrSeg ();
m_bEndOfExit = (seg->children [m_mine->Current ()->side] == -2);
m_nCube = m_mine->Current ()->segment;
m_nSide = m_mine->Current ()->side;
m_nPoint = m_mine->Current ()->point;
m_nType = seg->special;
m_nOwner = seg->owner;
m_nGroup = seg->group;
CBType ()->SetCurSel (m_nType);
OnResetCoord ();
  // show Triggers () that point at this cube
LBTriggers()->ResetContent();
CDTrigger *trigger = m_mine->Triggers ();
int trignum;
for (trignum = 0; trignum < m_mine->GameInfo ().triggers.count; trignum++, trigger++) {
	for (i = 0; i < trigger->num_links; i++) {
		if ((trigger->seg [i] == m_nCube) && (trigger->side [i] == m_nSide)) {
			// find the wall with this trigger
			CDWall *wall = m_mine->Walls ();
			int wallnum;
			for (wallnum = 0; wallnum < m_mine->GameInfo ().walls.count ;wallnum++, wall++) {
				if (wall->trigger == trignum) 
					break;
				}
			if (wallnum < m_mine->GameInfo ().walls.count) {
				sprintf(message, "%d,%d", (int) wall->segnum, (int) wall->sidenum + 1);
				int h = LBTriggers ()->AddString (message);
				LBTriggers ()->SetItemData (h, (long) wall->segnum * 0x10000L + wall->sidenum);
				}
			}
		}
	}
// show if this is cube/side is triggered by the control_center
control_center_trigger	*ccTrigger = m_mine->CCTriggers ();
int control;
for (control = 0; control < MAX_CONTROL_CENTER_TRIGGERS; control++, ccTrigger++) {
	int num_links = ccTrigger->num_links;
	for (i = 0; i < num_links; i++) {
		if ((m_nCube == ccTrigger->seg [i]) && (m_nSide == ccTrigger->side [i])) {
			LBTriggers ()->AddString ("Reactor");
			break;
			}
		}
	if (i < num_links) 
		break; // quit if for loop broke above
	}

// show "none" if there is no Triggers ()
if (!LBTriggers()->GetCount()) {
	LBTriggers()->AddString ("none");
	}

m_nLight = ((double) seg->static_light) / (24 * 327.68);

CListBox *plb [2] = { LBAvailBots (), LBUsedBots () };
if (IsBotMaker (seg)) {
	int nMatCen = seg->matcen_num;
	// if # of items in list box totals to less than the number of robots
	//    if (LBAvailBots ()->GetCount() + LBAvailBots ()->GetCount() < MAX_ROBOT_IDS) {
	HINSTANCE hInst = AfxGetInstanceHandle ();
	char		szObj [80];
	INT32		objFlags [2];
	for (i = 0; i < 2; i++)
		objFlags [i] = m_mine->BotGens (nMatCen)->objFlags [i];
	if ((m_nLastCube != m_nCube) || (m_nLastSide != m_nSide)) {
		for (i = 0; i < 2; i++) {
			plb [i]->ResetContent ();
			for (j = 0; j < 64; j++) {
				if (i) {
					h = ((objFlags [j / 32] & (1L << (j % 32))) != 0);
					if (!h)	//only add flagged objects to 2nd list box
						continue;
					}
				LoadString (hInst, ROBOT_STRING_TABLE + j, szObj, sizeof (szObj));
				h = plb [i]->AddString (szObj);
				plb [i]->SetItemData (h, j);
				}
			plb [i]->SetCurSel (0);
			}
		}
	}
else if (IsEquipMaker (seg)) {
	int nMatCen = seg->matcen_num;
	// if # of items in list box totals to less than the number of robots
	//    if (LBAvailBots ()->GetCount() + LBAvailBots ()->GetCount() < MAX_ROBOT_IDS) {
	HINSTANCE hInst = AfxGetInstanceHandle ();
	char		szObj [80];
	INT32		objFlags [2];
	for (i = 0; i < 2; i++)
		objFlags [i] = m_mine->EquipGens (nMatCen)->objFlags [i];
	if ((m_nLastCube != m_nCube) || (m_nLastSide != m_nSide)) {
		for (i = 0; i < 2; i++) {
			plb [i]->ResetContent ();
			for (j = 0; j < MAX_POWERUP_IDS2; j++) {
				if (i) {
					h = ((objFlags [j / 32] & (1L << (j % 32))) != 0);
					if (!h)	//only add flagged objects to 2nd list box
						continue;
					}
				LoadString (hInst, POWERUP_STRING_TABLE + j, szObj, sizeof (szObj));
				if (!strcmp (szObj, "(not used)"))
					continue;
				h = plb [i]->AddString (szObj);
				plb [i]->SetItemData (h, j);
				}
			plb [i]->SetCurSel (0);
			}
		}
	}
else {
	sprintf(message,"n/a");
	for (i = 0; i < 2; i++) {
		plb [i]->ResetContent();
		plb [i]->AddString(message);
		}
	}
m_nLastCube = m_nCube;
m_nLastSide = m_nSide;
EnableControls (TRUE);
UpdateData (FALSE);
}


//------------------------------------------------------------------------
// CCubeTool - EndOfExitTunnel ()
//------------------------------------------------------------------------

void CCubeTool::OnEndOfExit ()
{
if (!GetMine ())
	return;
CDSegment *seg = m_mine->CurrSeg ();
theApp.SetModified (TRUE);
if (m_bEndOfExit = EndOfExit ()->GetCheck ()) {
	seg->children [m_nSide] = -2;
	seg->child_bitmask |= (1 << m_nSide);
	}
else {
	seg->children[m_nSide] = -1;
	seg->child_bitmask &= ~(1 << m_nSide);
	}
}

//------------------------------------------------------------------------
// CCubeTool - Add Cube
//------------------------------------------------------------------------

void CCubeTool::OnAddCube () 
{
if (!GetMine ())
	return;
m_mine->AddSegment ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CCubeTool - Delete Cube
//------------------------------------------------------------------------

void CCubeTool::OnDeleteCube () 
{
if (!GetMine ())
	return;
m_mine->DeleteSegment (m_mine->Current ()->segment);
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------

void CCubeTool::OnSetOwner ()
{
if (!GetMine ())
	return;

	BOOL	bChangeOk = TRUE;
	BOOL	bMarked = m_mine->GotMarkedSegments ();


bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
theApp.MineView ()->DelayRefresh (true);
UpdateData (TRUE);
if (bMarked) {
	CDSegment *seg = m_mine->Segments ();
	for (INT16 nSegNum = 0; nSegNum < m_mine->SegCount (); nSegNum++, seg++)
		if (seg->wall_bitmask & MARKED_MASK)
			seg->owner = m_nOwner;
	}
else 					
	m_mine->CurrSeg ()->owner = m_nOwner;
theApp.UnlockUndo ();
theApp.MineView ()->DelayRefresh (false);
}

//------------------------------------------------------------------------

void CCubeTool::OnSetGroup ()
{
if (!GetMine ())
	return;

	BOOL	bChangeOk = TRUE;
	BOOL	bMarked = m_mine->GotMarkedSegments ();

bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
theApp.MineView ()->DelayRefresh (true);
UpdateData (TRUE);
if (bMarked) {
	CDSegment *seg = m_mine->Segments ();
	for (INT16 nSegNum = 0; nSegNum < m_mine->SegCount (); nSegNum++, seg++)
		if (seg->wall_bitmask & MARKED_MASK)
			seg->group = m_nGroup;
	}
else 					
	m_mine->CurrSeg ()->group = m_nGroup;
theApp.UnlockUndo ();
theApp.MineView ()->DelayRefresh (false);
}

//------------------------------------------------------------------------
// CCubeTool - SpecialMsg
//------------------------------------------------------------------------

void CCubeTool::OnSetType ()
{
if (!GetMine ())
	return;

	BOOL			bChangeOk = TRUE;
	BOOL			bMarked = m_mine->GotMarkedSegments ();
	int			nSegNum, nMinSeg, nMaxSeg;
	CDSegment	*segP;

bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
theApp.MineView ()->DelayRefresh (true);
m_nLastCube = -1; //force Refresh() to rebuild all dialog data
UINT8 nType = CBType ()->GetCurSel ();
if (bMarked) {
	nMinSeg = 0;
	nMaxSeg = m_mine->SegCount ();
	}
else {
	nMinSeg = m_mine->CurrSeg () - m_mine->Segments ();
	nMaxSeg = nMinSeg + 1;
	}
segP = m_mine->Segments (nMinSeg);
for (nSegNum = nMinSeg; nSegNum < nMaxSeg; nSegNum++, segP++) {
	if (bMarked && !(segP->wall_bitmask & MARKED_MASK))
		continue;
	m_nType = segP->special;
	switch(nType) {
		// check to see if we are adding a robot maker
		case SEGMENT_IS_ROBOTMAKER:
			if (nType == m_nType)
				goto errorExit;
			if (!m_mine->AddRobotMaker (nSegNum, false, m_bSetDefTexture == 1)) {
				theApp.ResetModified (bUndo);
				goto funcExit;
				}
			break;

		// check to see if we are adding a fuel center
		case SEGMENT_IS_REPAIRCEN:
			if (level_version < 9) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}

		case SEGMENT_IS_FUELCEN:
			if (nType == m_nType)
				continue;
			if (!m_mine->AddFuelCenter (nSegNum, nType, false, (nType == SEGMENT_IS_FUELCEN) && (m_bSetDefTexture == 1))) {
				theApp.ResetModified (bUndo);
				goto funcExit;
				}
			break;

		case SEGMENT_IS_CONTROLCEN:
			if (nType == m_nType)
				continue;
			if (!m_mine->AddReactor (nSegNum, false, m_bSetDefTexture == 1)) {
				theApp.ResetModified (bUndo);
				goto funcExit;
				}
			break;

		case SEGMENT_IS_GOAL_BLUE:
		case SEGMENT_IS_GOAL_RED:
			if (nType == m_nType)
				continue;
			if (!m_mine->AddGoalCube (nSegNum, false, m_bSetDefTexture == 1, nType, -1))
				goto errorExit;		
			break;

		case SEGMENT_IS_TEAM_BLUE:
		case SEGMENT_IS_TEAM_RED:
			if (level_version < 9) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (nType == m_nType)
				continue;
			if (!m_mine->AddTeamCube (nSegNum, false, false, nType, -1))
				goto errorExit;		
			break;

		case SEGMENT_IS_WATER:
			if (level_version < 9) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (!m_mine->AddWaterCube (nSegNum, false, m_bSetDefTexture == 1))
				goto errorExit;
			break;

		case SEGMENT_IS_LAVA:
			if (level_version < 9) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (!m_mine->AddLavaCube (nSegNum, false, m_bSetDefTexture == 1))
				goto errorExit;
			break;

		case SEGMENT_IS_SPEEDBOOST:
			if (level_version < 9) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (!m_mine->AddSpeedBoostCube (nSegNum, false))
				goto errorExit;
			break;

		case SEGMENT_IS_BLOCKED:
			if (level_version < 9) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (!m_mine->AddBlockedCube (nSegNum, false))
				goto errorExit;
			break;

		case SEGMENT_IS_SKYBOX:
			if (level_version < 9) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (!m_mine->AddSkyboxCube (nSegNum, false))
				goto errorExit;
			break;

		case SEGMENT_IS_OUTDOOR:
			if (level_version < 9) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (!m_mine->AddOutdoorCube (nSegNum, false))
				goto errorExit;
			break;

		case SEGMENT_IS_NODAMAGE:
			if (level_version < 9) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (!m_mine->AddNoDamageCube (nSegNum, false))
				goto errorExit;
			break;

		case SEGMENT_IS_EQUIPMAKER:
			if (level_version < 9) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (!m_mine->AddEquipMaker (nSegNum, false))
				goto errorExit;
			break;

		case SEGMENT_IS_NOTHING:
			m_mine->UndefineSegment (nSegNum);
			break;

		default:
			break;
		}
#if 1
	m_nType = nType;
#else
	if (m_nType == SEGMENT_IS_ROBOTMAKER) {
		// remove matcen
		int nMatCens = (int) m_mine->GameInfo ().matcen.count;
		if (nMatCens > 0) {
			// fill in deleted matcen
			int nDelMatCen = m_mine->CurrSeg ()->value;
			memcpy (m_mine->BotGens (nDelMatCen), m_mine->BotGens (nDelMatCen + 1), (nMatCens - 1 - nDelMatCen) * sizeof(matcen_info));
			m_mine->GameInfo ().matcen.count--;
			int i;
			for (i = 0; i < 6; i++)
				m_mine->DeleteTriggerTargets (m_nCube, i);
			}
		}
	else if (m_nType == SEGMENT_IS_FUELCEN) { //remove all fuel cell Walls ()
		INT16 nSegNum = m_mine->Current ()->segment;
		CDSegment *childseg, *seg = m_mine->CurrSeg ();
		CDSide *oppside, *side = m_mine->CurrSide ();
		CDWall *wall;
		INT16 opp_segnum, opp_sidenum;
		for (INT16 sidenum = 0; sidenum < 6; sidenum++, side++) {
			if (seg->children [sidenum] < 0)	// assume no wall if no child segment at the current side
				continue;
			childseg = m_mine->Segments () + seg->children [sidenum];
			if (childseg->special == SEGMENT_IS_FUELCEN)	// don't delete if child segment is fuel center
				continue;
			// if there is a wall and it's a fuel cell delete it
			if ((wall = m_mine->GetWall (nSegNum, sidenum)) && 
				 (wall->type == WALL_ILLUSION) && (side->nBaseTex == (file_type == RDL_FILE) ? 322 : 333))
				m_mine->DeleteWall (side->nWall);
			// if there is a wall at the opposite side and it's a fuel cell delete it
			if (m_mine->GetOppositeSide (opp_segnum, opp_sidenum, nSegNum, sidenum) &&
				 (wall = m_mine->GetWall (nSegNum, sidenum)) && (wall->type == WALL_ILLUSION)) {
				oppside = m_mine->Segments (opp_segnum)->sides + opp_sidenum;
				if (oppside->nBaseTex == (file_type == RDL_FILE) ? 322 : 333)
					m_mine->DeleteWall (oppside->nWall);
				}
			}
		}
	// update "special"
	if (bChangeOk) {
		m_nType = nType;
		m_mine->CurrSeg ()->special = nType;
		if (nType == SEGMENT_IS_NOTHING)
			m_mine->CurrSeg ()->child_bitmask &= ~(1 << MAX_SIDES_PER_SEGMENT);
		else
			m_mine->CurrSeg ()->child_bitmask |= (1 << MAX_SIDES_PER_SEGMENT);
		}
#endif
	}

errorExit:

theApp.UnlockUndo ();
m_mine->AutoLinkExitToReactor ();
theApp.SetModified (TRUE);

funcExit:

theApp.MineView ()->DelayRefresh (false);
UpdateData (TRUE);
}

//------------------------------------------------------------------------
// CCubeTool - Cube Number Message
//------------------------------------------------------------------------

void CCubeTool::OnSetCube () 
{
if (!GetMine ())
	return;
m_mine->Current ()->segment = CBCubeNo ()->GetCurSel ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CCubeTool - LightMsg
//------------------------------------------------------------------------

void CCubeTool::OnLight () 
{
if (!GetMine ())
	return;
UpdateData (TRUE);
m_mine->CurrSeg ()->static_light = (FIX) (m_nLight * 24 * 327.68);
theApp.SetModified (TRUE);
}

                        /*--------------------------*/

int CCubeTool::FindBot (CListBox *plb, LPSTR pszObj)
{
	int i, j;

i = plb->GetCurSel ();
if (i < 0)
	return -1;
j = plb->GetItemData (i);
if (pszObj)
	LoadString (AfxGetInstanceHandle(), ROBOT_STRING_TABLE + j, pszObj, 80);
return j;
}

                        /*--------------------------*/

int CCubeTool::FindEquip (CListBox *plb, LPSTR pszObj)
{
	int i, j;

i = plb->GetCurSel ();
if (i < 0)
	return -1;
j = plb->GetItemData (i);
if (pszObj)
	LoadString (AfxGetInstanceHandle(), POWERUP_STRING_TABLE + j, pszObj, 80);
return j;
}

//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------

void CCubeTool::AddBot ()
{
if (!GetMine ())
	return;
CDSegment *seg = m_mine->CurrSeg ();
int matcen = seg->matcen_num;
char szObj [80];
int i = FindBot (LBAvailBots (), szObj);
if ((i < 0) || (i >= 64))
	return;
m_mine->BotGens (matcen)->objFlags [i / 32] |= (1L << (i % 32));
int h = LBAvailBots ()->GetCurSel ();
LBAvailBots ()->DeleteString (h);
LBAvailBots ()->SetCurSel (h);
h = LBUsedBots ()->AddString (szObj);
LBUsedBots ()->SetItemData (h, i);
theApp.SetModified (TRUE);
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------

void CCubeTool::AddEquip ()
{
if (!GetMine ())
	return;
CDSegment *seg = m_mine->CurrSeg ();
int matcen = seg->matcen_num;
char szObj [80];
int i = FindEquip (LBAvailBots (), szObj);
if ((i < 0) || (i >= MAX_POWERUP_IDS2))
	return;
m_mine->EquipGens (matcen)->objFlags [i / 32] |= (1L << (i % 32));
int h = LBAvailBots ()->GetCurSel ();
LBAvailBots ()->DeleteString (h);
LBAvailBots ()->SetCurSel (h);
h = LBUsedBots ()->AddString (szObj);
LBUsedBots ()->SetItemData (h, i);
theApp.SetModified (TRUE);
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CCubeTool - AddMsg
//------------------------------------------------------------------------

void CCubeTool::OnAddObj ()
{
if (!GetMine ())
	return;
CDSegment *seg = m_mine->CurrSeg ();
if (IsBotMaker (seg))
	AddBot ();
else if (IsEquipMaker (seg))
	AddEquip ();
}

//------------------------------------------------------------------------
// CCubeTool - DelMsg
//------------------------------------------------------------------------

void CCubeTool::DeleteBot () 
{
if (!GetMine ())
	return;
CDSegment *seg = m_mine->CurrSeg ();
int matcen = seg->matcen_num;
char szObj [80];
int i = FindBot (LBUsedBots (), szObj);
if ((i < 0) || (i >= 64))
	return;
m_mine->BotGens (matcen)->objFlags [i / 32] &= ~(1L << (i % 32));
int h = LBUsedBots ()->GetCurSel ();
LBUsedBots ()->DeleteString (h);
LBUsedBots ()->SetCurSel (h);
h = LBAvailBots ()->AddString (szObj);
LBAvailBots ()->SetItemData (h, i);
theApp.SetModified (TRUE);
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CCubeTool - DelMsg
//------------------------------------------------------------------------

void CCubeTool::DeleteEquip () 
{
if (!GetMine ())
	return;
CDSegment *seg = m_mine->CurrSeg ();
int matcen = seg->matcen_num;
char szObj [80];
int i = FindEquip (LBUsedBots (), szObj);
if ((i < 0) || (i >= 64))
	return;
m_mine->EquipGens (matcen)->objFlags [i / 32] &= ~(1L << (i % 32));
int h = LBUsedBots ()->GetCurSel ();
LBUsedBots ()->DeleteString (h);
LBUsedBots ()->SetCurSel (h);
h = LBAvailBots ()->AddString (szObj);
LBAvailBots ()->SetItemData (h, i);
theApp.SetModified (TRUE);
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CCubeTool - DelMsg
//------------------------------------------------------------------------

void CCubeTool::OnDeleteObj () 
{
if (!GetMine ())
	return;
CDSegment *seg = m_mine->CurrSeg ();
if (IsBotMaker (seg))
	DeleteBot ();
else if (IsEquipMaker (seg))
	DeleteEquip ();
}

//------------------------------------------------------------------------
// CCubeTool - OtherCubeMsg
//------------------------------------------------------------------------

void CCubeTool::OnOtherCube () 
{
theApp.MineView ()->SelectOtherCube ();
}

//------------------------------------------------------------------------
// CCubeTool - CubeButtonMsg
//------------------------------------------------------------------------

void CCubeTool::OnWallDetails () 
{
if (!GetMine ())
	return;
if (!LBTriggers ()->GetCount ())
	return;
int i = LBTriggers ()->GetCurSel ();
if (i < 0)
	return;
long h = LBTriggers ()->GetItemData (i);
m_mine->Current ()->segment = (INT16) (h / 0x10000L);
m_mine->Current ()->side = (INT16) (h % 0x10000L);
theApp.ToolView ()->EditWall ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CCubeTool - TriggerButtonMsg
//------------------------------------------------------------------------

void CCubeTool::OnTriggerDetails ()
{
if (!GetMine ())
	return;
if (!LBTriggers ()->GetCount ())
	return;
int i = LBTriggers ()->GetCurSel ();
if ((i < 0) || (i >= LBTriggers ()->GetCount ()))
	return;
long h = LBTriggers ()->GetItemData (i);
m_mine->Other ()->segment = m_mine->Current ()->segment;
m_mine->Other ()->side = m_mine->Current ()->side;
m_mine->Current ()->segment = (INT16) (h / 0x10000L);
m_mine->Current ()->side = (INT16) (h % 0x10000L);
theApp.ToolView ()->EditTrigger ();
theApp.MineView ()->Refresh ();
}

                        /*--------------------------*/

void CCubeTool::OnAddBotGen ()
{
if (!GetMine ())
	return;
m_mine->AddRobotMaker ();
m_nLastCube = -1;
Refresh ();
}

                        /*--------------------------*/

void CCubeTool::OnAddEquipGen ()
{
if (!GetMine ())
	return;
m_mine->AddEquipMaker ();
m_nLastCube = -1;
Refresh ();
}

                        /*--------------------------*/

void CCubeTool::OnAddFuelCen ()
{
if (!GetMine ())
	return;
m_mine->AddFuelCenter ();
}

                        /*--------------------------*/

void CCubeTool::OnAddRepairCen ()
{
if (!GetMine ())
	return;
m_mine->AddFuelCenter (-1, SEGMENT_IS_REPAIRCEN);
}

                        /*--------------------------*/

void CCubeTool::OnAddControlCen ()
{
if (!GetMine ())
	return;
m_mine->AddReactor ();
}

                        /*--------------------------*/

void CCubeTool::OnAddWaterCube ()
{
if (!GetMine ())
	return;
m_mine->AddWaterCube ();
}

                        /*--------------------------*/

void CCubeTool::OnAddLavaCube ()
{
if (!GetMine ())
	return;
m_mine->AddLavaCube ();
}

                        /*--------------------------*/

void CCubeTool::OnSplitCube ()
{
if (!GetMine ())
	return;
m_mine->SplitSegment ();
}

                        /*--------------------------*/

//eof cubedlg.cpp