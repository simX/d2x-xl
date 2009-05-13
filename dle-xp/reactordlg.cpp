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

BEGIN_MESSAGE_MAP (CReactorTool, CToolDlg)
	ON_BN_CLICKED (IDC_REACTOR_ADDTGT, OnAddTarget)
	ON_BN_CLICKED (IDC_REACTOR_DELTGT, OnDeleteTarget)
	ON_BN_CLICKED (IDC_REACTOR_ADDWALLTGT, OnAddWallTarget)
	ON_BN_CLICKED (IDC_REACTOR_DELWALLTGT, OnDeleteWallTarget)
	ON_EN_KILLFOCUS (IDC_REACTOR_COUNTDOWN, OnCountDown)
	ON_EN_KILLFOCUS (IDC_REACTOR_SECRETRETURN, OnSecretReturn)
	ON_EN_UPDATE (IDC_REACTOR_COUNTDOWN, OnCountDown)
	ON_EN_UPDATE (IDC_REACTOR_SECRETRETURN, OnSecretReturn)
	ON_LBN_SELCHANGE (IDC_REACTOR_TARGETLIST, OnSetTarget)
	ON_LBN_DBLCLK (IDC_REACTOR_TARGETLIST, OnSetTarget)
END_MESSAGE_MAP ()

//------------------------------------------------------------------------
// DIALOG - CReactorTool (constructor)
//------------------------------------------------------------------------

CReactorTool::CReactorTool (CPropertySheet *pParent)
	: CToolDlg (nLayout ? IDD_REACTORDATA2 : IDD_REACTORDATA, pParent)
{
m_pTrigger = NULL;
Reset ();
}

								/*--------------------------*/

void CReactorTool::Reset ()
{
m_nCountDown = 30;
m_nSecretReturn = 0;
m_nTrigger = 0;
m_nTargets = 0;
m_iTarget = -1;
*m_szTarget = '\0';
}

                        /*--------------------------*/

BOOL CReactorTool::OnInitDialog ()
{
CToolDlg::OnInitDialog ();
// Descent only uses the first control center, #0
m_bInited = TRUE;
return TRUE;
}

								/*--------------------------*/

void CReactorTool::DoDataExchange (CDataExchange *pDX)
{
if (!m_bInited)
	return;
DDX_Text (pDX, IDC_REACTOR_COUNTDOWN, m_nCountDown);
DDX_Text (pDX, IDC_REACTOR_SECRETRETURN, m_nSecretReturn);
DDX_Text (pDX, IDC_REACTOR_TARGET, m_szTarget, sizeof (m_szTarget));
}

								/*--------------------------*/

BOOL CReactorTool::OnSetActive ()
{
Refresh ();
return CToolDlg::OnSetActive ();
}

								/*--------------------------*/

void CReactorTool::EnableControls (BOOL bEnable)
{
CToolDlg::EnableControls (IDC_REACTOR_COUNTDOWN, IDC_REACTOR_SECRETRETURN, bEnable);
//for (int i = IDC_TRIGGER_TRIGGER_NO; i <= IDC_TRIGGER_PASTE; i++)
//	GetDlgItem (i)->EnableWindow (bEnable);
}

								/*--------------------------*/

void CReactorTool::InitLBTargets ()
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

//------------------------------------------------------------------------
// CReactorTool - RefreshData
//------------------------------------------------------------------------

void CReactorTool::Refresh ()
{
if (!m_bInited)
	return;
if (!GetMine ())
	return;
EnableControls (file_type != RDL_FILE);
m_pTrigger = GetMine ()->CCTriggers (m_nTrigger);
m_nCountDown = m_mine->ReactorTime ();
m_nSecretReturn = m_mine->SecretCubeNum ();
InitLBTargets ();
OnSetTarget ();
UpdateData (FALSE);
}

//------------------------------------------------------------------------
// CReactorTool - TrigValueMsg
//------------------------------------------------------------------------

void CReactorTool::OnCountDown () 
{
if (!GetMine ())
	return;
char szVal [5];
::GetWindowText (GetDlgItem (IDC_REACTOR_COUNTDOWN)->m_hWnd, szVal, sizeof(szVal));
if (!*szVal)
	return;
UpdateData (TRUE);
theApp.SetModified (TRUE);
m_mine->ReactorTime () = m_nCountDown;
}

//------------------------------------------------------------------------
// CReactorTool - TrigTimeMsg
//------------------------------------------------------------------------

void CReactorTool::OnSecretReturn () 
{
if (!GetMine ())
	return;
char szVal [5];
::GetWindowText (GetDlgItem (IDC_REACTOR_SECRETRETURN)->m_hWnd, szVal, sizeof(szVal));
if (!*szVal)
	return;
UpdateData (TRUE);
theApp.SetModified (TRUE);
m_mine->SecretCubeNum () = m_nSecretReturn;
}

//------------------------------------------------------------------------
// CReactorTool - Add cube/side to trigger list
//------------------------------------------------------------------------

void CReactorTool::AddTarget (INT16 segnum, INT16 sidenum) 
{
if (!GetMine ())
	return;
m_nTargets = m_pTrigger->num_links;
if (m_nTargets >= MAX_TRIGGER_TARGETS) {
	DEBUGMSG (" Reactor tool: No more targets possible for this trigger.");
	return;
	}
if (FindTarget (segnum, sidenum)) {
	DEBUGMSG (" Reactor tool: Trigger already has this target.");
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

void CReactorTool::OnAddTarget () 
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

void CReactorTool::OnAddWallTarget ()
{
if (!GetMine ())
	return;
CDSelection *other = (m_mine->Current () == &m_mine->Current1 ()) ? &m_mine->Current2 () : &m_mine->Current1 ();
int i = FindTarget (other->segment, other->side);
if (i < 0)
	return;
LBTargets ()->SetCurSel (i);
OnAddTarget ();
}

//------------------------------------------------------------------------
// CReactorTool - Delete cube/side
//------------------------------------------------------------------------

void CReactorTool::OnDeleteTarget ()
{
m_iTarget = LBTargets ()->GetCurSel ();
if ((m_iTarget < 0) || (m_iTarget >= MAX_TRIGGER_TARGETS))
	return;
theApp.SetModified (TRUE);
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

int CReactorTool::FindTarget (INT16 segnum, INT16 sidenum)
{
for (int i = 0; i < m_pTrigger->num_links; i++)
	if ((segnum = m_pTrigger->seg [i]) && (sidenum = m_pTrigger->seg [i]))
		return i;
return -1;
}

                        /*--------------------------*/

void CReactorTool::OnDeleteWallTarget ()
{
if (!GetMine ())
	return;
CDSelection *other = (m_mine->Current () == &m_mine->Current1 ()) ? &m_mine->Current2 () : &m_mine->Current1 ();
int i = FindTarget (other->segment, other->side);
if (i < 0) {
	DEBUGMSG (" Reactor tool: Trigger doesn't target other cube's current side.");
	return;
	}
LBTargets ()->SetCurSel (i);
OnDeleteTarget ();
}

//------------------------------------------------------------------------
// CReactorTool - Cube/Side list box message
//
// sets "other cube" to selected item
//------------------------------------------------------------------------

void CReactorTool::OnSetTarget () 
{
if (!GetMine ())
	return;
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

//eof reactordlg.cpp