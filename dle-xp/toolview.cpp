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

/////////////////////////////////////////////////////////////////////////////
// CToolView

IMPLEMENT_DYNCREATE(CToolView, CWnd)

BEGIN_MESSAGE_MAP(CToolView, CWnd)
	ON_WM_CREATE ()
	ON_WM_DESTROY ()
	ON_WM_PAINT ()
	ON_WM_ERASEBKGND ()
	ON_WM_HSCROLL ()
	ON_WM_VSCROLL ()
	ON_COMMAND(ID_SEL_PREVTAB, OnSelectPrevTab)
	ON_COMMAND(ID_SEL_NEXTTAB, OnSelectNextTab)
END_MESSAGE_MAP()

                        /*--------------------------*/

CToolView::CToolView ()
	: CWnd ()
{
m_bRecalcLayout = FALSE;
m_cubeTool = NULL;
m_wallTool = NULL;
m_triggerTool = NULL;
m_textureTool = NULL;
m_objectTool = NULL;
m_effectTool = NULL;
m_advObjTool = NULL;
m_lightTool = NULL;
m_missionTool = NULL;
m_reactorTool = NULL;
m_diagTool = NULL;
m_prefsDlg = NULL;
m_txtFilterTool = NULL;
m_pTools = new CPropertySheet ();
m_scrollOffs [0] = 0;
m_scrollPage [0] = 0;
m_scrollRange [0] = 0;
m_bHScroll = 
m_bVScroll = FALSE;
}


                        /*--------------------------*/

CToolView::~CToolView ()
{
if (m_pTools) {
	delete m_pTools;
	m_pTools = NULL;
	}
if (m_cubeTool)
	delete m_cubeTool;
if (m_wallTool)
	delete m_wallTool;
if (m_triggerTool)
	delete m_triggerTool;
if (m_textureTool)
	delete m_textureTool;
if (m_objectTool)
	delete m_objectTool;
if (m_effectTool)
	delete m_effectTool;
if (m_advObjTool)
	delete m_advObjTool;
if (m_lightTool)
	delete m_lightTool;
if (m_reactorTool)
	delete m_reactorTool;
if (m_diagTool)
	delete m_diagTool;
if (m_missionTool)
	delete m_missionTool;
if (m_prefsDlg)
	delete m_prefsDlg;
if (m_txtFilterTool)
	delete m_txtFilterTool;
}

								/*---------------------------*/

void CToolView::Refresh ()
{
	CPropertyPage	*p = m_pTools->GetActivePage ();

if (p == m_prefsDlg)
	m_prefsDlg->Refresh ();
else if (p == m_textureTool)
	m_textureTool->Refresh ();
else if (p == m_wallTool)
	m_wallTool->Refresh ();
else if (p == m_triggerTool)
	m_triggerTool->Refresh ();
else if (p == m_cubeTool)
	m_cubeTool->Refresh ();
else if (p == m_objectTool)
	m_objectTool->Refresh ();
else if (p == m_effectTool)
	m_effectTool->Refresh ();
else if (p == m_advObjTool)
	m_advObjTool->Refresh ();
else if (p == m_missionTool)
	m_missionTool->Refresh ();
else if (p == m_reactorTool)
	m_reactorTool->Refresh ();
else if (p == m_diagTool)
	m_diagTool->Refresh ();
}

								/*---------------------------*/

void CToolView::Setup (void)
{
if (!m_pTools)
	return;
m_cubeTool = new CCubeTool (m_pTools);
m_wallTool = new CWallTool (m_pTools);
m_triggerTool = new CTriggerTool (m_pTools);
m_textureTool = new CTextureTool (m_pTools);
m_lightTool = new CLightTool (m_pTools);
m_objectTool = new CObjectTool (m_pTools);
m_effectTool = new CEffectTool (m_pTools);
m_advObjTool = new CAdvObjTool (m_pTools);
m_reactorTool = new CReactorTool (m_pTools);
m_missionTool = new CMissionTool (m_pTools);
m_diagTool = new CDiagTool (m_pTools);
m_prefsDlg = new CPrefsDlg (m_pTools);
m_txtFilterTool = new CTxtFilterTool (m_pTools);
m_pTools->AddPage (m_textureTool);
m_pTools->AddPage (m_cubeTool);
m_pTools->AddPage (m_wallTool);
m_pTools->AddPage (m_triggerTool);
m_pTools->AddPage (m_objectTool);
m_pTools->AddPage (m_effectTool);
m_pTools->AddPage (m_advObjTool);
m_pTools->AddPage (m_lightTool);
m_pTools->AddPage (m_reactorTool);
m_pTools->AddPage (m_missionTool);
m_pTools->AddPage (m_diagTool);
m_pTools->AddPage (m_txtFilterTool);
m_pTools->AddPage (m_prefsDlg);
//m_pTools->Create (this, WS_CHILD | WS_VISIBLE, 0);
}

                        /*--------------------------*/

int CToolView::OnCreate (LPCREATESTRUCT lpCreateStruct) 
{
if (CWnd::OnCreate (lpCreateStruct) == -1)
		return -1;
Setup ();
if (!m_pTools->Create (this, WS_CHILD | WS_VISIBLE, 0))
	return -1;
CalcToolSize ();
return 0;
}

                        /*--------------------------*/

void CToolView::CalcToolSize (void) 
{
CRect	rc;
m_pTools->GetWindowRect (rc);
m_toolSize.cx = rc.Width ();
m_toolSize.cy = rc.Height () + 6/*+ 12*/;
RecalcLayout ();
}

                        /*--------------------------*/

void CToolView::OnDestroy (void) 
{
CWnd::OnDestroy ();
m_pTools->DestroyWindow ();
}

								/*---------------------------*/

afx_msg void CToolView::OnPaint ()
{
	CRect	rc;
	CDC	*pDC;
	PAINTSTRUCT	ps;
	CSize	toolSize;

if (!GetUpdateRect (rc))
	return;
pDC = BeginPaint (&ps);
RecalcLayout ();
EndPaint (&ps);
}

								/*---------------------------*/

afx_msg BOOL CToolView::OnEraseBkgnd (CDC* pDC)
{
	CRect			rc;

GetClientRect (rc);
pDC->FillSolidRect (rc, RGB (128,128,128));
return TRUE;
/*
   CRect    rc;
   CBrush   bkGnd, * pOldBrush;
   CPoint   pt (0,0);

ClientToScreen (&pt);
bkGnd.CreateStockObject (BLACK_BRUSH);
bkGnd.UnrealizeObject ();
pDC->SetBrushOrg (pt);
pOldBrush = pDC->SelectObject (&bkGnd);
GetClientRect (rc);
pDC->FillRect (&rc, &bkGnd);
pDC->SelectObject (pOldBrush);
bkGnd.DeleteObject ();
return 1;
*/
}

void CToolView::MoveWindow (int x, int y, int nWidth, int nHeight, BOOL bRepaint)
{
/*
if (nHeight > 100) {
	x += nHeight - 100;
	nHeight = 100;
	}
*/
CWnd::MoveWindow (x, y, nWidth, nHeight, bRepaint);
}

void CToolView::MoveWindow (LPCRECT lpRect, BOOL bRepaint)
{
MoveWindow (lpRect->left, lpRect->top, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top, bRepaint);
}

                        /*--------------------------*/

void CToolView::OnHScroll (UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
switch (nSBCode) {
	case SB_LEFT:
		--m_scrollOffs [0];
		break;
	case SB_ENDSCROLL:
		break;
	case SB_LINELEFT:
		m_scrollOffs [0] -= m_scrollPage [0];
		break;
	case SB_LINERIGHT:
		m_scrollOffs [0] += m_scrollPage [0];
		break;
	case SB_PAGELEFT:
		m_scrollOffs [0] -= m_scrollPage [0];
		break;
	case SB_PAGERIGHT:
		m_scrollOffs [0] += m_scrollPage [0];
		break;
	case SB_RIGHT:
		m_scrollOffs [0] = m_scrollRange [0];
		break;
	case SB_THUMBPOSITION:
		m_scrollOffs [0] = nPos;
		break;
	case SB_THUMBTRACK:
		m_scrollOffs [0] = nPos;
		break;
	default:
		CWnd::OnHScroll (nSBCode, nPos, pScrollBar);
	}
if (m_scrollOffs [0] < 0)
	m_scrollOffs [0] = 0;
else if (m_scrollOffs [0] >= m_scrollRange [0])
	m_scrollOffs [0] = m_scrollRange [0];
SetScrollPos (SB_HORZ, m_scrollOffs [0], TRUE);
RecalcLayout ();
}

                        /*--------------------------*/

void CToolView::OnVScroll (UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
switch (nSBCode) {
	case SB_LEFT:
		--m_scrollOffs [1];
		break;
	case SB_ENDSCROLL:
		break;
	case SB_LINELEFT:
		m_scrollOffs [1] -= m_scrollPage [1];
		break;
	case SB_LINERIGHT:
		m_scrollOffs [1] += m_scrollPage [1];
		break;
	case SB_PAGELEFT:
		m_scrollOffs [1] -= m_scrollPage [1];
		break;
	case SB_PAGERIGHT:
		m_scrollOffs [1] += m_scrollPage [1];
		break;
	case SB_RIGHT:
		m_scrollOffs [1] = m_scrollRange [1];
		break;
	case SB_THUMBPOSITION:
		m_scrollOffs [1] = nPos;
		break;
	case SB_THUMBTRACK:
		m_scrollOffs [1] = nPos;
		break;
	default:
		CWnd::OnHScroll (nSBCode, nPos, pScrollBar);
	}
if (m_scrollOffs [1] < 0)
	m_scrollOffs [1] = 0;
else if (m_scrollOffs [1] >= m_scrollRange [1])
	m_scrollOffs [1] = m_scrollRange [1];
SetScrollPos (SB_VERT, m_scrollOffs [1], TRUE);
RecalcLayout ();
}

                        /*--------------------------*/

void CToolView::RecalcLayout (int nToolMode, int nTextureMode)
{
	CRect	rc;
	CSize	paneSize;

if (m_bRecalcLayout)
	return;
m_bRecalcLayout = TRUE;
if (theApp.MainFrame ())
	theApp.MainFrame ()->RecalcLayout (nToolMode, nTextureMode);
GetWindowRect (rc);
paneSize.cx = rc.Width ();
paneSize.cy = rc.Height ();
if (rc.Width () >= m_toolSize.cx) {
	ShowScrollBar (SB_HORZ, FALSE);
	rc.left = 0;
	if (m_bHScroll) {
		m_bHScroll = FALSE;
		m_toolSize.cy -= GetSystemMetrics (SM_CXVSCROLL);
		}
	}
else {
	m_scrollRange [0] = m_toolSize.cx - paneSize.cx;
	m_scrollPage [0] = (m_scrollRange [0] < m_toolSize.cx) ? m_scrollRange [0] : m_toolSize.cx;
	SetScrollRange (SB_HORZ, 0, m_scrollRange [0], TRUE);
	ShowScrollBar (SB_HORZ, TRUE);
	rc.left = -GetScrollPos (SB_HORZ);
	if (!m_bHScroll) {
		m_bHScroll = TRUE;
		m_toolSize.cy += GetSystemMetrics (SM_CXVSCROLL);
		}
	}
if (rc.Height () >= m_toolSize.cy - 11) {
	ShowScrollBar (SB_VERT, FALSE);
	rc.top = 0;
	if (m_bVScroll) {
		m_bVScroll = FALSE;
		m_toolSize.cx -= GetSystemMetrics (SM_CYHSCROLL);
		}
	}
else {
	m_scrollRange [1] = m_toolSize.cy - paneSize.cy;
	m_scrollPage [1] = (m_scrollRange [1] < m_toolSize.cy) ? m_scrollRange [1] : m_toolSize.cy;
	SetScrollRange (SB_VERT, 0, m_scrollRange [1], TRUE);
	ShowScrollBar (SB_VERT, TRUE);
	rc.top = -GetScrollPos (SB_VERT);
	if (!m_bVScroll) {
		m_bVScroll = TRUE;
		m_toolSize.cx += GetSystemMetrics (SM_CYHSCROLL);
		}
	}
rc.right = (paneSize.cx > m_toolSize.cx) ? paneSize.cx : m_toolSize.cx;
rc.bottom = (paneSize.cy > m_toolSize.cy) ? paneSize.cy : m_toolSize.cy;
m_pTools->MoveWindow (rc);
m_bRecalcLayout = FALSE;
}


                        /*--------------------------*/

void CToolView::SetActive (int nPage)
{
if (m_pTools) {
	m_pTools->SetActivePage (nPage);
	theApp.MainFrame ()->RecalcLayout (FALSE, 1);
	}
}	

                        /*--------------------------*/

void CToolView::CycleTab (int nDir)
{
int i = m_pTools->GetPageIndex (m_pTools->GetActivePage ()) + nDir;
int h = m_pTools->GetPageCount ();
if (i < 0)
	i = h - 1;
else if (i >= h)
	i = 0;
m_pTools->SetActivePage (i);
}

                        /*--------------------------*/

void CToolView::NextTab ()
{
CycleTab (1);
}

                        /*--------------------------*/

void CToolView::PrevTab ()
{
CycleTab (-1);
}

                        /*--------------------------*/

void CToolView::OnSelectPrevTab ()
{
theApp.MainFrame ()->ShowTools ();
PrevTab ();
}

                        /*--------------------------*/

void CToolView::OnSelectNextTab ()
{
theApp.MainFrame ()->ShowTools ();
NextTab ();
}

                        /*--------------------------*/

#if 0
int CDlcBitmapButton::OnToolHitTest (CPoint point, TOOLINFO* pTI)
{
	CRect	rc;

GetClientRect (rc);
if (!rc.PtInRect (point))
	return -1;
return m_nId;
}
#endif
                        /*--------------------------*/

//eof toolview.cpp