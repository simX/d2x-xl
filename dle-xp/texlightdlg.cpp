 // dlcView.cpp : implementation of the CMineView class
//

#include <math.h>
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
static char THIS_FILE [] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CToolView


                        /*--------------------------*/

void CTextureTool::AnimateLight (void)
{
LightButton (m_nHighlight)->SetState (0);
m_nHighlight++;
m_nHighlight &= 0x1f; // 0..31

CButton *pb = LightButton (m_nHighlight);
pb->SetState (1);

CRect rc;
pb->GetClientRect (rc);
rc.InflateRect (-3, -4);
rc.left += 2;
rc.right++;
CDC *pDC = pb->GetDC ();
COLORREF	color = 
	pb->GetCheck () ? RGB (0,0,0) : RGB (196,196,196);
CBrush br;
br.CreateSolidBrush (color);
CBrush *pOldBrush = pDC->SelectObject (&br);
CPen pen;
pen.CreatePen (PS_SOLID, 1, color);
CPen *pOldPen = pDC->SelectObject (&pen);
pDC->Ellipse (&rc);
pDC->SelectObject (pOldPen);
pDC->SelectObject (pOldBrush);
pb->ReleaseDC (pDC);
UpdateColorCtrl (
	&m_lightWnd, 
	pb->GetCheck () ? 
		(m_nColorIndex > 0) ?
			RGB (m_rgbColor.peRed, m_rgbColor.peGreen, m_rgbColor.peBlue) :
			RGB (255,196,0) : 
		RGB (0,0,0));
}

                        /*--------------------------*/

void CTextureTool::ToggleLight (int i)
{
CButton *pb = LightButton (i - 1);
pb->SetCheck (!pb->GetCheck ());
SetLightString ();
}

                        /*--------------------------*/

void CTextureTool::UpdateLight (void)
{
	bool bUndo, bChange = false;

if (m_iLight < 0)
	return;
if (!GetMine ())
	return;
UINT32 nLightMask = 0;
for (int i = 0; i < 32; i++)
	if (m_szLight [i] == '1')
		nLightMask |= (1 << i);
bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (m_mine->FlickeringLights (m_iLight)->mask != nLightMask) {
	bChange = true;
	m_mine->FlickeringLights (m_iLight)->mask = nLightMask;
	}
long nDelay = (m_nLightDelay * F1_0 /*- F0_5*/) / 1000;
if (m_mine->FlickeringLights (m_iLight)->delay != nDelay) {
	bChange = true;
	m_mine->FlickeringLights (m_iLight)->delay = nDelay;
	}
if (bChange)
	theApp.UnlockUndo ();
else
	theApp.ResetModified (bUndo);
m_nLightDelay = (1000 * nDelay + F0_5) / F1_0;
}

                        /*--------------------------*/

bool CTextureTool::SetLightDelay (int nSpeed)
{
if (nSpeed < 0)
	return false;
if (m_nLightTimer)
	KillTimer (m_nLightTimer);
if ((m_iLight >= 0) && nSpeed) {
	m_nLightDelay = nSpeed;
	m_nLightTimer = SetTimer (2, m_nLightDelay, NULL);
	((CSliderCtrl *) TimerSlider ())->SetPos (20 - (m_nLightDelay + 25) / 50);
	}
else {
	m_nLightDelay = 0;
	m_nLightTimer = -1;
	}
m_nLightTime = m_nLightDelay / 1000.0;
UpdateLight ();
UpdateData (FALSE);
return true;
}

                        /*--------------------------*/

void CTextureTool::SetLightString (void)
{
	static char cLight [2] = {'0', '1'};
	char szLight [33];

for (int i = 0; i < 32; i++)
	szLight [i] = cLight [LightButton (i)->GetCheck ()];
szLight [32] = '\0';
if (strcmp (szLight, m_szLight)) {
	strcpy (m_szLight, szLight);
	UpdateLight ();
	UpdateData (FALSE);
	}
}
		
                        /*--------------------------*/

void CTextureTool::SetLightButtons (LPSTR szLight, int nSpeed)
{
	bool	bDefault = false;

if (szLight) {
	if (szLight != m_szLight)
		strcpy (m_szLight, szLight);
	}
else
	UpdateData (TRUE);
for (int i = 0; i < 32; i++) {
	if (!bDefault && (m_szLight [i] == '\0'))
		bDefault = true;
	if (bDefault)
		m_szLight [i] = '0';
	LightButton (i)->SetCheck (m_szLight [i] == '1');
	}
m_szLight [32] = '\0';
if (!SetLightDelay (nSpeed)) {
	UpdateLight ();
	UpdateData (FALSE);
	}
}

                        /*--------------------------*/

void CTextureTool::EnableLightControls (BOOL bEnable)
{
for (int i = IDC_TEXLIGHT_OFF; i <= IDC_TEXLIGHT_TIMER; i++)
	GetDlgItem (i)->EnableWindow (bEnable);
}

                        /*--------------------------*/

void CTextureTool::UpdateLightWnd (void)
{
if (!GetMine ())
	return;
CDWall *pWall = m_mine->CurrWall ();
if (!SideHasLight ()) {
	if (m_bLightEnabled)
		EnableLightControls (m_bLightEnabled = FALSE);
	if (level_version >= 9)
		memset (m_mine->CurrLightColor (), 0, sizeof (CDColor));
	}
else {
	if (!m_bLightEnabled)
		EnableLightControls (m_bLightEnabled = TRUE);
	if (level_version >= 9) {
		CDColor *plc = m_mine->CurrLightColor ();
		if (!plc->index) {	// set light color to white for new lights
			plc->index = 255;
			plc->color.r =
			plc->color.g =
			plc->color.b = 1.0;
			}
		}
	}
m_iLight = m_mine->GetFlickeringLight ();
if (m_iLight < 0) {
	OnLightOff ();
	return;
	}

long nLightMask = m_mine->FlickeringLights (m_iLight)->mask;
for (int i = 0; i < 32; i++)
	m_szLight [i] = (nLightMask & (1 << i)) ? '1' : '0';
m_szLight [32] = '\0';
SetLightButtons (m_szLight, (int) (((1000 * m_mine->FlickeringLights (m_iLight)->delay + F0_5) / F1_0)));
}

                        /*--------------------------*/

void CTextureTool::OnLightEdit ()
{
if (m_iLight < 0)
	UpdateData (FALSE);
else {
	UpdateData (TRUE);
	SetLightButtons ();
	}
}

                        /*--------------------------*/

void CTextureTool::OnLightTimerEdit ()
{
if (m_iLight < 0)
	UpdateData (FALSE);
else {
	UpdateData (TRUE);
	SetLightDelay ((int) (1000 * m_nLightTime));
	}
}

                        /*--------------------------*/

void CTextureTool::OnAddLight ()
{
if (m_iLight >= 0)
	INFOMSG (" There is already a flickering light.")
else if (0 <= (m_iLight = GetMine ()->AddFlickeringLight (-1, -1, 0xAAAAAAAAL, F1_0 / 4))) {
	UpdateLightWnd ();
	theApp.MineView ()->Refresh ();
	}
}

                        /*--------------------------*/

void CTextureTool::OnDeleteLight ()
{
if (m_iLight < 0)
	INFOMSG (" There is no flickering light.")
else if (GetMine ()->DeleteFlickeringLight ()) {
	m_iLight = -1;
	UpdateLightWnd ();
	theApp.MineView ()->Refresh ();
	}
}

                        /*--------------------------*/

void CTextureTool::OnLightOff () { SetLightButtons ("", 1000); }
void CTextureTool::OnLightOn () { SetLightButtons ("11111111111111111111111111111111", 1000); }
void CTextureTool::OnLightStrobe4 () { SetLightButtons ("10000000100000001000000010000000", 250); }
void CTextureTool::OnLightStrobe8 () { SetLightButtons ("10001000100010001000100010001000", 250); }
void CTextureTool::OnLightFlicker () { SetLightButtons ("11111111000000111100010011011110", 100); }
void CTextureTool::OnLightDefault () { SetLightButtons ("10101010101010101010101010101010", 250); }

                        /*--------------------------*/

void CTextureTool::OnLight1 () { ToggleLight (1); }
void CTextureTool::OnLight2 () { ToggleLight (2); }
void CTextureTool::OnLight3 () { ToggleLight (3); }
void CTextureTool::OnLight4 () { ToggleLight (4); }
void CTextureTool::OnLight5 () { ToggleLight (5); }
void CTextureTool::OnLight6 () { ToggleLight (6); }
void CTextureTool::OnLight7 () { ToggleLight (7); }
void CTextureTool::OnLight8 () { ToggleLight (8); }
void CTextureTool::OnLight9 () { ToggleLight (9); }
void CTextureTool::OnLight10 () { ToggleLight (10); }
void CTextureTool::OnLight11 () { ToggleLight (11); }
void CTextureTool::OnLight12 () { ToggleLight (12); }
void CTextureTool::OnLight13 () { ToggleLight (13); }
void CTextureTool::OnLight14 () { ToggleLight (14); }
void CTextureTool::OnLight15 () { ToggleLight (15); }
void CTextureTool::OnLight16 () { ToggleLight (16); }
void CTextureTool::OnLight17 () { ToggleLight (17); }
void CTextureTool::OnLight18 () { ToggleLight (18); }
void CTextureTool::OnLight19 () { ToggleLight (19); }
void CTextureTool::OnLight20 () { ToggleLight (20); }
void CTextureTool::OnLight21 () { ToggleLight (21); }
void CTextureTool::OnLight22 () { ToggleLight (22); }
void CTextureTool::OnLight23 () { ToggleLight (23); }
void CTextureTool::OnLight24 () { ToggleLight (24); }
void CTextureTool::OnLight25 () { ToggleLight (25); }
void CTextureTool::OnLight26 () { ToggleLight (26); }
void CTextureTool::OnLight27 () { ToggleLight (27); }
void CTextureTool::OnLight28 () { ToggleLight (28); }
void CTextureTool::OnLight29 () { ToggleLight (29); }
void CTextureTool::OnLight30 () { ToggleLight (30); }
void CTextureTool::OnLight31 () { ToggleLight (31); }
void CTextureTool::OnLight32 () { ToggleLight (32); }

                        /*--------------------------*/

void CTextureTool::SetWallColor (void)
{
if (m_mine->UseTexColors ()) {
	INT16			segnum, sidenum;
	INT16			nBaseTex = m_mine->CurrSide ()->nBaseTex;
	CDSegment	*seg = m_mine->Segments ();
	CDSide		*side;
	CDWall		*wall;
	bool			bAll = !m_mine->GotMarkedSides ();

	for (segnum = 0; segnum < m_mine->SegCount (); segnum++, seg++) {
		for (sidenum = 0, side = seg->sides; sidenum < 6; sidenum++, side++) {
			if (side->nWall < 0)
				continue;
			wall = m_mine->Walls (side->nWall);
			if (wall->type != WALL_TRANSPARENT)
				continue;
			if (!(bAll || m_mine->SideIsMarked (segnum, sidenum)))
				continue;
			if (side->nBaseTex != nBaseTex)
				continue;
			wall->cloak_value = m_nColorIndex;
			}
		}
	}
}

                        /*--------------------------*/

void CTextureTool::OnLButtonDown (UINT nFlags, CPoint point)
{
if (!GetMine ())
	return;

	CRect		rcPal;

if (/*(level_version >= 9) &&*/ SideHasLight ()) {
	GetCtrlClientRect (&m_paletteWnd, rcPal);
	if (PtInRect (rcPal, point)) {
		point.x -= rcPal.left;
		point.y -= rcPal.top;
		if (m_paletteWnd.SelectColor (point, m_nColorIndex, &m_rgbColor)) {
			CDWall *pWall = m_mine->CurrWall ();
			if (pWall && (pWall->type == WALL_TRANSPARENT)) {
				pWall->cloak_value = m_nColorIndex;
				SetWallColor ();
				}
			CDColor *psc = m_mine->CurrLightColor ();
			if (psc->index = m_nColorIndex) {
				psc->color.r = (double) m_rgbColor.peRed / 255.0;
				psc->color.g = (double) m_rgbColor.peGreen / 255.0;
				psc->color.b = (double) m_rgbColor.peBlue / 255.0;
				}
			else {
				psc->color.r =
				psc->color.g =
				psc->color.b = 1.0;
				}
			//if (!pWall || (pWall->type != WALL_TRANSPARENT)) 
				{
				m_mine->SetTexColor (m_mine->CurrSide ()->nBaseTex, psc);
				m_mine->SetTexColor (m_mine->CurrSide ()->nOvlTex, psc);
				}
			UpdateData (FALSE);
			UpdatePaletteWnd ();
			}
		}
	}
}

                        /*--------------------------*/
		
void CTextureTool::ChooseRGBColor (void)
{
	CHOOSECOLOR	cc;

memset (&cc, 0, sizeof (cc));
cc.lStructSize = sizeof (cc);
cc.hwndOwner = theApp.MainFrame ()->m_hWnd;
cc.rgbResult = RGB (m_rgbColor.peRed, m_rgbColor.peGreen, m_rgbColor.peBlue);
cc.lpCustColors = m_custColors;
cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT | CC_SHOWHELP;
if (ChooseColor (&cc)) {
	CDColor *psc = m_mine->CurrLightColor ();
	psc->index = m_nColorIndex = 255;
	m_rgbColor.peBlue = ((BYTE) (cc.rgbResult >> 16)) & 0xFF;
	m_rgbColor.peGreen = ((BYTE) (cc.rgbResult >> 8)) & 0xFF;
	m_rgbColor.peRed = ((BYTE) cc.rgbResult) & 0xFF;
	psc->color.r = (double) m_rgbColor.peRed / 255.0;
	psc->color.g = (double) m_rgbColor.peGreen / 255.0;
	psc->color.b = (double) m_rgbColor.peBlue / 255.0;
	m_mine->SetTexColor (m_mine->CurrSide ()->nBaseTex, psc);
	m_mine->SetTexColor (m_mine->CurrSide ()->nOvlTex, psc);
	UpdatePaletteWnd ();
	}
}

                        /*--------------------------*/
		
		//eof texturedlg.cpp