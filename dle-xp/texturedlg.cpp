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
#include "texedit.h"
#include "io.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE [] = __FILE__;
#endif

#define TEXTOOLDLG 1

/////////////////////////////////////////////////////////////////////////////
// CToolView

BEGIN_MESSAGE_MAP(CTextureTool, CTexToolDlg)
	ON_WM_PAINT ()
	ON_WM_TIMER ()
	ON_WM_HSCROLL ()
	ON_WM_VSCROLL ()
	ON_WM_LBUTTONDOWN ()
	ON_BN_CLICKED (IDC_TEXTURE_SHOWFRAMES, LoadTextureListBoxes)
	ON_BN_CLICKED (IDC_TEXTURE_EDIT, OnEditTexture)
	ON_BN_CLICKED (IDC_TEXTURE_COPY, OnSaveTexture)
	ON_BN_CLICKED (IDC_TEXTURE_PASTESIDE, OnPasteSide)
	ON_BN_CLICKED (IDC_TEXTURE_PASTETOUCHING, OnPasteTouching)
	ON_BN_CLICKED (IDC_TEXTURE_PASTEMARKED, OnPasteMarked)
	ON_BN_CLICKED (IDC_TEXTURE_REPLACE, OnReplace)
	ON_BN_CLICKED (IDC_TEXTURE_PASTE1ST, OnPaste1st)
	ON_BN_CLICKED (IDC_TEXTURE_PASTE2ND, OnPaste2nd)
	ON_BN_CLICKED (IDC_TEXALIGN_HALEFT, OnAlignLeft)
	ON_BN_CLICKED (IDC_TEXALIGN_HARIGHT, OnAlignRight)
	ON_BN_CLICKED (IDC_TEXALIGN_VAUP, OnAlignUp)
	ON_BN_CLICKED (IDC_TEXALIGN_VADOWN, OnAlignDown)
	ON_BN_CLICKED (IDC_TEXALIGN_RALEFT, OnAlignRotLeft)
	ON_BN_CLICKED (IDC_TEXALIGN_RARIGHT, OnAlignRotRight)
	ON_BN_CLICKED (IDC_TEXALIGN_HFLIP, OnHFlip)
	ON_BN_CLICKED (IDC_TEXALIGN_VFLIP, OnVFlip)
	ON_BN_CLICKED (IDC_TEXALIGN_HSHRINK, OnHShrink)
	ON_BN_CLICKED (IDC_TEXALIGN_VSHRINK, OnVShrink)
	ON_BN_CLICKED (IDC_TEXALIGN_RESET, OnAlignReset)
	ON_BN_CLICKED (IDC_TEXALIGN_RESETMARKED, OnAlignResetMarked)
	ON_BN_CLICKED (IDC_TEXALIGN_STRETCH2FIT, OnAlignStretch2Fit)
	ON_BN_CLICKED (IDC_TEXALIGN_CHILDALIGN, OnAlignChildren)
	ON_BN_CLICKED (IDC_TEXALIGN_ALIGNALL, OnAlignAll)
	ON_BN_CLICKED (IDC_TEXALIGN_ZOOMIN, OnZoomIn)
	ON_BN_CLICKED (IDC_TEXALIGN_ZOOMOUT, OnZoomOut)
	ON_BN_CLICKED (IDC_TEXALIGN_ROT0, OnRot2nd0)
	ON_BN_CLICKED (IDC_TEXALIGN_ROT90, OnRot2nd90)
	ON_BN_CLICKED (IDC_TEXALIGN_ROT180, OnRot2nd180)
	ON_BN_CLICKED (IDC_TEXALIGN_ROT270, OnRot2nd270)
	ON_BN_CLICKED (IDC_TEXLIGHT_1, OnLight1)
	ON_BN_CLICKED (IDC_TEXLIGHT_2, OnLight2)
	ON_BN_CLICKED (IDC_TEXLIGHT_3, OnLight3)
	ON_BN_CLICKED (IDC_TEXLIGHT_4, OnLight4)
	ON_BN_CLICKED (IDC_TEXLIGHT_5, OnLight5)
	ON_BN_CLICKED (IDC_TEXLIGHT_6, OnLight6)
	ON_BN_CLICKED (IDC_TEXLIGHT_7, OnLight7)
	ON_BN_CLICKED (IDC_TEXLIGHT_8, OnLight8)
	ON_BN_CLICKED (IDC_TEXLIGHT_9, OnLight9)
	ON_BN_CLICKED (IDC_TEXLIGHT_10, OnLight10)
	ON_BN_CLICKED (IDC_TEXLIGHT_11, OnLight11)
	ON_BN_CLICKED (IDC_TEXLIGHT_12, OnLight12)
	ON_BN_CLICKED (IDC_TEXLIGHT_13, OnLight13)
	ON_BN_CLICKED (IDC_TEXLIGHT_14, OnLight14)
	ON_BN_CLICKED (IDC_TEXLIGHT_15, OnLight15)
	ON_BN_CLICKED (IDC_TEXLIGHT_16, OnLight16)
	ON_BN_CLICKED (IDC_TEXLIGHT_17, OnLight17)
	ON_BN_CLICKED (IDC_TEXLIGHT_18, OnLight18)
	ON_BN_CLICKED (IDC_TEXLIGHT_19, OnLight19)
	ON_BN_CLICKED (IDC_TEXLIGHT_20, OnLight20)
	ON_BN_CLICKED (IDC_TEXLIGHT_21, OnLight21)
	ON_BN_CLICKED (IDC_TEXLIGHT_22, OnLight22)
	ON_BN_CLICKED (IDC_TEXLIGHT_23, OnLight23)
	ON_BN_CLICKED (IDC_TEXLIGHT_24, OnLight24)
	ON_BN_CLICKED (IDC_TEXLIGHT_25, OnLight25)
	ON_BN_CLICKED (IDC_TEXLIGHT_26, OnLight26)
	ON_BN_CLICKED (IDC_TEXLIGHT_27, OnLight27)
	ON_BN_CLICKED (IDC_TEXLIGHT_28, OnLight28)
	ON_BN_CLICKED (IDC_TEXLIGHT_29, OnLight29)
	ON_BN_CLICKED (IDC_TEXLIGHT_30, OnLight30)
	ON_BN_CLICKED (IDC_TEXLIGHT_31, OnLight31)
	ON_BN_CLICKED (IDC_TEXLIGHT_32, OnLight32)
	ON_BN_CLICKED (IDC_TEXLIGHT_OFF, OnLightOff)
	ON_BN_CLICKED (IDC_TEXLIGHT_ON, OnLightOn)
	ON_BN_CLICKED (IDC_TEXLIGHT_STROBE4, OnLightStrobe4)
	ON_BN_CLICKED (IDC_TEXLIGHT_STROBE8, OnLightStrobe8)
	ON_BN_CLICKED (IDC_TEXLIGHT_FLICKER, OnLightFlicker)
	ON_BN_CLICKED (IDC_TEXLIGHT_ADD, OnAddLight)
	ON_BN_CLICKED (IDC_TEXLIGHT_DELETE, OnDeleteLight)
	ON_BN_CLICKED (IDC_TEXLIGHT_RGBCOLOR, ChooseRGBColor)
	ON_CBN_SELCHANGE (IDC_TEXTURE_1ST, OnSelect1st)
	ON_CBN_SELCHANGE (IDC_TEXTURE_2ND, OnSelect2nd)
	ON_EN_KILLFOCUS (IDC_TEXTURE_LIGHT1, OnSetLight)
	ON_EN_KILLFOCUS (IDC_TEXTURE_LIGHT2, OnSetLight)
	ON_EN_KILLFOCUS (IDC_TEXTURE_LIGHT3, OnSetLight)
	ON_EN_KILLFOCUS (IDC_TEXTURE_LIGHT4, OnSetLight)
	ON_EN_KILLFOCUS (IDC_TEXALIGN_HALIGN, OnAlignX)
	ON_EN_KILLFOCUS (IDC_TEXALIGN_VALIGN, OnAlignY)
	ON_EN_KILLFOCUS (IDC_TEXALIGN_RALIGN, OnAlignRot)
	ON_EN_KILLFOCUS (IDC_TEXTURE_BRIGHTNESS, OnBrightnessEdit)
	ON_EN_KILLFOCUS (IDC_TEXLIGHT_TIMER, OnLightTimerEdit)
	ON_EN_KILLFOCUS (IDC_TEXLIGHT_EDIT, OnLightEdit)
//	ON_EN_UPDATE (IDC_TEXALIGN_HALIGN, OnAlignX)
//	ON_EN_UPDATE (IDC_TEXALIGN_VALIGN, OnAlignY)
//	ON_EN_UPDATE (IDC_TEXALIGN_RALIGN, OnAlignRot)
	ON_EN_UPDATE (IDC_TEXTURE_BRIGHTNESS, OnBrightnessEdit)
	ON_EN_UPDATE (IDC_TEXLIGHT_TIMER, OnLightTimerEdit)
//	ON_EN_UPDATE (IDC_TEXLIGHT_EDIT, OnLightEdit)
END_MESSAGE_MAP()

                        /*--------------------------*/

CTextureTool::CTextureTool (CPropertySheet *pParent)
	: CTexToolDlg (nLayout ? IDD_TEXTUREDATA2 : IDD_TEXTUREDATA, pParent, 1, IDC_TEXTURE_SHOW, RGB (0,0,0))
{
last_texture1 = -1;
last_texture2 = 0;
save_texture1 = -1;
save_texture2 = 0;
int i;
for (i = 0; i < 4; i++)
	save_uvls [i].l = default_uvls [i].l;
#if TEXTOODLG == 0
m_frame [0] = 0;
m_frame [1] = 0;
#endif
m_bUse1st = TRUE;
m_bUse2nd = FALSE;
m_bShowChildren = TRUE;
m_bShowTexture = TRUE;
m_zoom = 1.0;
m_alignX = 0;
m_alignY = 0;
m_alignAngle = 0;
m_alignRot2nd = 0;
m_nLightDelay = 1000;
m_nLightTime = 1.0;
m_iLight = -1;
m_nHighlight = -1;
*m_szTextureBuf = '\0';
memset (m_szLight, 0, sizeof (m_szLight));
m_bLightEnabled = TRUE;
m_bIgnorePlane = FALSE;
m_nColorIndex = -1;
m_nEditFunc = -1;
}

                        /*--------------------------*/

CTextureTool::~CTextureTool ()
{
if (m_bInited) {
	if (IsWindow (m_textureWnd))
		m_textureWnd.DestroyWindow ();
	if (IsWindow (m_alignWnd))
		m_alignWnd.DestroyWindow ();
	if (IsWindow (m_lightWnd))
		m_lightWnd.DestroyWindow ();
	//if (!nLayout) 
		{
		if (IsWindow (m_colorWnd))
			m_colorWnd.DestroyWindow ();
		if (IsWindow (m_paletteWnd))
			m_paletteWnd.DestroyWindow ();
		}
	}
}

                        /*--------------------------*/

void CTextureTool::CreateColorCtrl (CWnd *pWnd, int nIdC)
{
CWnd *pParentWnd = GetDlgItem (nIdC);
CRect rc;
pParentWnd->GetClientRect (rc);
pWnd->Create (NULL, NULL, WS_CHILD | WS_VISIBLE, rc, pParentWnd, 0);
}

                        /*--------------------------*/

void CTextureTool::UpdateColorCtrl (CWnd *pWnd, COLORREF color)
{
CDC *pDC = pWnd->GetDC ();
CRect rc;
pWnd->GetClientRect (rc);
pDC->FillSolidRect (&rc, color);
pWnd->ReleaseDC (pDC);
pWnd->Invalidate ();
pWnd->UpdateWindow ();
}

                        /*--------------------------*/

BOOL CTextureTool::OnInitDialog ()
{
	CRect	rc;

if (!CToolDlg::OnInitDialog ())
   return FALSE;
/*
m_btnZoomIn.SubclassDlgItem (IDC_TEXALIGN_ZOOMIN, this);
m_btnZoomOut.SubclassDlgItem (IDC_TEXALIGN_ZOOMOUT, this);
m_btnHShrink.SubclassDlgItem (IDC_TEXALIGN_HSHRINK, this);
m_btnVShrink.SubclassDlgItem (IDC_TEXALIGN_VSHRINK, this);
m_btnHALeft.SubclassDlgItem (IDC_TEXALIGN_HALEFT, this);
m_btnHARight.SubclassDlgItem (IDC_TEXALIGN_HARIGHT, this);
m_btnVAUp.SubclassDlgItem (IDC_TEXALIGN_VAUP, this);
m_btnVADown.SubclassDlgItem (IDC_TEXALIGN_VADOWN, this);
m_btnRALeft.SubclassDlgItem (IDC_TEXALIGN_RALEFT, this);
m_btnRARight.SubclassDlgItem (IDC_TEXALIGN_RARIGHT, this);
*/
m_btnZoomIn.AutoLoad (IDC_TEXALIGN_ZOOMIN, this);
m_btnZoomOut.AutoLoad (IDC_TEXALIGN_ZOOMOUT, this);
m_btnHShrink.AutoLoad (IDC_TEXALIGN_HSHRINK, this);
m_btnVShrink.AutoLoad (IDC_TEXALIGN_VSHRINK, this);
m_btnHALeft.AutoLoad (IDC_TEXALIGN_HALEFT, this);
m_btnHARight.AutoLoad (IDC_TEXALIGN_HARIGHT, this);
m_btnVAUp.AutoLoad (IDC_TEXALIGN_VAUP, this);
m_btnVADown.AutoLoad (IDC_TEXALIGN_VADOWN, this);
m_btnRALeft.AutoLoad (IDC_TEXALIGN_RALEFT, this);
m_btnRARight.AutoLoad (IDC_TEXALIGN_RARIGHT, this);

m_btnStretch2Fit.AutoLoad (IDC_TEXALIGN_STRETCH2FIT, this);
m_btnReset.AutoLoad (IDC_TEXALIGN_RESET, this);
m_btnResetMarked.AutoLoad (IDC_TEXALIGN_RESETMARKED, this);
m_btnChildAlign.AutoLoad (IDC_TEXALIGN_CHILDALIGN, this);
m_btnAlignAll.AutoLoad (IDC_TEXALIGN_ALIGNALL, this);
m_btnAddLight.AutoLoad (IDC_TEXLIGHT_ADD, this);
m_btnDelLight.AutoLoad (IDC_TEXLIGHT_DELETE, this);
m_btnHFlip.AutoLoad (IDC_TEXALIGN_HFLIP, this);
m_btnVFlip.AutoLoad (IDC_TEXALIGN_VFLIP, this);
#if 0
m_btnZoomIn.EnableToolTips (TRUE);
m_btnZoomOut.EnableToolTips (TRUE);
m_btnHShrink.EnableToolTips (TRUE);
m_btnVShrink.EnableToolTips (TRUE);
m_btnHALeft.EnableToolTips (TRUE);
m_btnHARight.EnableToolTips (TRUE);
m_btnVAUp.EnableToolTips (TRUE);
m_btnVADown.EnableToolTips (TRUE);
m_btnRALeft.EnableToolTips (TRUE);
m_btnRARight.EnableToolTips (TRUE);
m_btnStretch2Fit.EnableToolTips (TRUE);
m_btnReset.EnableToolTips (TRUE);
m_btnResetMarked.EnableToolTips (TRUE);
m_btnChildAlign.EnableToolTips (TRUE);
m_btnAlignAll.EnableToolTips (TRUE);
m_btnAddLight.EnableToolTips (TRUE);
m_btnDelLight.EnableToolTips (TRUE);
#endif
InitSlider (IDC_TEXTURE_BRIGHTSLIDER, 0, 100);
InitSlider (IDC_TEXLIGHT_TIMERSLIDER, 0, 20);
LoadTextureListBoxes ();
CreateImgWnd (&m_textureWnd, IDC_TEXTURE_SHOW);
CreateImgWnd (&m_alignWnd, IDC_TEXALIGN_SHOW);
HScrollAlign ()->SetScrollRange (-100, 100, FALSE);
HScrollAlign ()->SetScrollPos (0, TRUE);
VScrollAlign ()->SetScrollRange (-100, 100, FALSE);
VScrollAlign ()->SetScrollPos (0, TRUE);
CreateColorCtrl (&m_lightWnd, IDC_TEXLIGHT_SHOW);
//if (!nLayout) 
	{
	CreateColorCtrl (&m_colorWnd, IDC_TEXLIGHT_COLOR);
	m_paletteWnd.Create (GetDlgItem (IDC_TEXLIGHT_PALETTE), -1, -1);
	}
m_nTimer = -1; 
m_nLightTimer = -1;
m_nEditTimer = -1;
UpdateLightWnd ();
UpdateData (FALSE);
m_bInited = TRUE;
return TRUE;
}

                        /*--------------------------*/

void CTextureTool::DoDataExchange (CDataExchange *pDX)
{
	int		i, nBrightness;
	char		szBrightness [20];

for (i = 0; i < 4; i++)
	DDX_Double (pDX, IDC_TEXTURE_LIGHT1 + i, m_lights [i], 0, 200, "%1.1f");
DDX_Check (pDX, IDC_TEXTURE_PASTE1ST, m_bUse1st);
DDX_Check (pDX, IDC_TEXTURE_PASTE2ND, m_bUse2nd);
DDX_Check (pDX, IDC_TEXALIGN_SHOWTEXTURE, m_bShowTexture);
DDX_Check (pDX, IDC_TEXALIGN_SHOWCHILDREN, m_bShowChildren);
DDX_Check (pDX, IDC_TEXALIGN_IGNOREPLANE, m_bIgnorePlane);
DDX_Double (pDX, IDC_TEXALIGN_HALIGN, m_alignX);
DDX_Double (pDX, IDC_TEXALIGN_VALIGN, m_alignY);
DDX_Double (pDX, IDC_TEXALIGN_RALIGN, m_alignAngle);
DDX_Radio (pDX, IDC_TEXALIGN_ROT0, m_alignRot2nd);
DDX_Double (pDX, IDC_TEXLIGHT_TIMER, m_nLightTime);
DDX_Text (pDX, IDC_TEXTURE_PASTEBUF, m_szTextureBuf, sizeof (m_szTextureBuf));
//if (!nLayout) 
	{
	DDX_Text (pDX, IDC_TEXLIGHT_EDIT, m_szLight, sizeof (m_szLight));
	DDX_Text (pDX, IDC_TEXLIGHT_COLORINDEX, m_nColorIndex);
	}
sprintf (szBrightness, "%d", m_nBrightness);
DDX_Text (pDX, IDC_TEXTURE_BRIGHTNESS, szBrightness, sizeof (szBrightness));
if (pDX->m_bSaveAndValidate && *szBrightness) {
	m_nBrightness = atoi (szBrightness);
	nBrightness = (m_nBrightness < 0) ? 0 : (m_nBrightness > 100) ? 100 : m_nBrightness;
	((CSliderCtrl *) BrightnessSlider ())->SetPos (nBrightness);
	SetBrightness (nBrightness);
	}
}

                        /*--------------------------*/

void CTextureTool::LoadTextureListBoxes () 
{
	HINSTANCE	hInst = AfxGetApp()->m_hInstance;
	char			name [80];
	int			bShowFrames;
	int			nTextures, iTexture, index;
	CComboBox	*cbTexture1 = CBTexture1 ();
	CComboBox	*cbTexture2 = CBTexture2 ();

GetMine ();
bShowFrames = GetCheck (IDC_TEXTURE_SHOWFRAMES);

INT16 texture1 = m_mine ? m_mine->CurrSide ()->nBaseTex : 0;
INT16 texture2 = m_mine ? m_mine->CurrSide ()->nOvlTex & 0x3fff : 0;

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
	if (bShowFrames || !strstr ((char *) name, "frame")) {
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

bool CTextureTool::SideHasLight (void)
{
if	((m_mine->IsLight (m_mine->CurrSide ()->nBaseTex) != -1) ||
	 (((m_mine->CurrSide ()->nOvlTex & 0x3fff) != 0) &&
	  (m_mine->IsLight (m_mine->CurrSide ()->nOvlTex & 0x3fff) != -1)))
	return true;
CDWall *pWall = m_mine->CurrWall ();
return pWall && (pWall->type == WALL_TRANSPARENT);

}


                        /*--------------------------*/

void CTextureTool::UpdatePaletteWnd (void)
{
if (/*!nLayout && (level_version >= 9) &&*/ SideHasLight ()) {
	EnableControls (IDC_TEXLIGHT_PALETTE + 1, IDC_TEXLIGHT_COLOR, TRUE);
	m_paletteWnd.ShowWindow (SW_SHOW);
	m_paletteWnd.DrawPalette ();
	UpdateColorCtrl (
		&m_colorWnd, 
		(m_nColorIndex > 0) ? 
		RGB (m_rgbColor.peRed, m_rgbColor.peGreen, m_rgbColor.peBlue) :
		RGB (0,0,0));
	}
else {
	EnableControls (IDC_TEXLIGHT_PALETTE + 1, IDC_TEXLIGHT_COLOR, FALSE);
	m_paletteWnd.ShowWindow (SW_HIDE);
	}
}

                        /*--------------------------*/

void CTextureTool::OnPaint ()
{
CTexToolDlg::OnPaint ();
#if TEXTOOLDLG == 0
UpdateTextureWnd ();
#endif
UpdateAlignWnd ();
UpdatePaletteWnd ();
}

                        /*--------------------------*/

void CTextureTool::UpdateTextureWnd (void)
{
#if TEXTOOLDLG == 0
RefreshTextureWnd ();
m_textureWnd.InvalidateRect (NULL);
m_textureWnd.UpdateWindow ();
#endif
}

                        /*--------------------------*/

int lightIdxFromMode [4] = {0, 3, 2, 1};

void CTextureTool::Refresh ()
{
if (!m_bInited)
	return;
if (!GetMine ())
	return;
//Beep (1000,100);
	HINSTANCE	hInst = AfxGetApp()->m_hInstance;
	CComboBox	*cbTexture1 = CBTexture1 ();
	CComboBox	*cbTexture2 = CBTexture2 ();
	INT16			texture1, texture2, mode;
	bool			bShowTexture;
	int			i, j;
	CDSegment	*seg;
	CDSide		*side;
	CDWall		*pWall;
	CDColor		*color;
// enable buttons as required
/*
EditButton->EnableWindow((file_type == RDL_FILE || path [0] == NULL) ? FALSE: TRUE);
LightButton->EnableWindow((file_type==RDL_FILE) ? FALSE:TRUE);
PickButton->EnableWindow(path [0] ? TRUE:FALSE);
*/
// set animation frames to zero
#if TEXTOOLDLG == 0
m_frame [0] = 0;
m_frame [1] = 0;
#endif

seg = m_mine->CurrSeg ();
side = m_mine->CurrSide ();
color = m_mine->CurrLightColor ();
int nSide = m_mine->Current ()->side;
texture1 = side->nBaseTex;
texture2 = side->nOvlTex & 0x3fff;
pWall = m_mine->CurrWall ();
m_nColorIndex = (pWall && (pWall->type == WALL_TRANSPARENT)) ? pWall->cloak_value : color->index;
m_rgbColor.peRed = (char) (255.0 * color->color.r);
m_rgbColor.peGreen = (char) (255.0 * color->color.g);
m_rgbColor.peBlue = (char) (255.0 * color->color.b);
if ((texture1 < 0) || (texture1 >= MAX_TEXTURES))
	texture1 = 0;
if ((texture2 < 0) || (texture2 >= MAX_TEXTURES))
	texture2 = 0;
GetBrightness ((m_bUse2nd && !m_bUse1st) ? texture2 : texture1);
mode = side->nOvlTex & 0xc000;
// set edit fields to % of light and enable them
for (i = 0; i < 4; i++) {
/*
	switch(mode) {
		case (INT16) 0x0000: 
			j = i;       
			break;
		case (INT16) 0x4000: 
			j = (i + 3) % 4; 
			break;
		case (INT16) 0x8000: 
			j = (i + 2) % 4;
			break;
		case (INT16) 0xC000: 
			j = (i + 1) % 4;
			break;
		default: 
			j = i;
		}
*/
	j = (i + lightIdxFromMode [mode / 0x4000]) % 4;
	m_lights [j] = (double) ((UINT16) side->uvls [i].l) / 327.68;
	}

if (seg->children [nSide]==-1)
	bShowTexture = TRUE;
else {
	UINT16 nWall = side->nWall;
	bShowTexture = (nWall < m_mine->GameInfo ().walls.count);
	}
if (bShowTexture) {
	if ((texture1!=last_texture1) || (texture2!=last_texture2) || (mode!=last_mode)) {
		last_texture1 = texture1;
		last_texture2 = texture2;
		last_mode = mode;
		LoadString (hInst, texture_resource + texture1, message, sizeof (message));
		cbTexture1->SetCurSel (i = cbTexture1->SelectString (-1, message));  // unselect if string not found
		if (side->nOvlTex) {
			LoadString (hInst, texture_resource + texture2, message, sizeof (message));
			cbTexture2->SetCurSel (cbTexture2->SelectString (-1, message));  // unselect if string not found
			}
		else
			cbTexture2->SelectString (-1, "(none)");
		}
	}
else {
	last_texture1 = -1;
	last_texture2 = -1;
	cbTexture1->SelectString (-1, "(none)");
	cbTexture2->SelectString (-1, "(none)");
	}
RefreshAlignment ();
UpdateData (FALSE);
#if TEXTOOLDLG
CTexToolDlg::Refresh ();
#else
UpdateTextureWnd ();
#endif
UpdateAlignWnd ();
UpdateLightWnd ();
UpdatePaletteWnd ();
}

                        /*--------------------------*/

void CTextureTool::RefreshTextureWnd ()
{
#if TEXTOOLDLG
if (!CTexToolDlg::Refresh ())
#else
if (!PaintTexture (&m_textureWnd))
#endif
	last_texture1 =
	last_texture2 = -1;
}

                        /*--------------------------*/
#if TEXTOOLDLG == 0
void CTextureTool::DrawTexture (INT16 texture1, INT16 texture2, int x0, int y0) 
{
	UINT8 bitmap_array [64*64];
	int x_offset;
	int y_offset;
	x_offset = 0;
	y_offset = 0;
	CDC	*pDC = m_textureWnd.GetDC ();
	CDTexture tx (bmBuf);

memset (bitmap_array,0,sizeof (bitmap_array));
if (DefineTexture(texture1,texture2,&tx,x0,y0))
	return;
BITMAPINFO *bmi;
bmi = MakeBitmap ();
CPalette	*oldPalette = pDC->SelectPalette (thePalette, FALSE);
pDC->RealizePalette ();
//SetDIBitsToDevice (pDC->m_hDC, x_offset, y_offset, 64, 64, 0, 0, 0, 64, bitmap_array, bmi, DIB_RGB_COLORS);
CRect	rc;
m_textureWnd.GetClientRect (&rc);
StretchDIBits (pDC->m_hDC, x_offset, y_offset, rc.Width (), rc.Height (), 0, 0, tx.width, tx.height,
		        	(void *) bitmap_array, bmi, DIB_RGB_COLORS, SRCCOPY);
pDC->SelectPalette(oldPalette, FALSE);
m_textureWnd.ReleaseDC (pDC);
}
#endif
                        /*--------------------------*/

BOOL CTextureTool::OnSetActive ()
{
#if TEXTOOLDLG
CTexToolDlg::OnSetActive ();
#else
m_nTimer = SetTimer (1, 100U, NULL);
#endif
if (m_iLight >= 0)
	m_nLightTimer = SetTimer (2, m_nLightDelay, NULL);
Refresh ();
return TRUE; //CTexToolDlg::OnSetActive ();
}

                        /*--------------------------*/

BOOL CTextureTool::OnKillActive ()
{
#if TEXTOOLDLG
CTexToolDlg::OnKillActive ();
#else
if (m_nTimer >= 0) {
	KillTimer (m_nTimer);
	m_nTimer = -1;
	}
#endif
if (m_nLightTimer >= 0) {
	KillTimer (m_nLightTimer);
	m_nLightTimer = -1;
	}
if (m_nEditTimer >= 0) {
	KillTimer (m_nEditTimer);
	m_nEditTimer = -1;
	}
return CToolDlg::OnKillActive ();
}

                        /*--------------------------*/

#if TEXTOOLDLG == 0

void CTextureTool::AnimateTexture (void)
{
if (!GetMine ())
	return;

	CDSegment *seg = m_mine->CurrSeg ();

	UINT16 texture [2];
	static int scroll_offset_x = 0;
	static int scroll_offset_y = 0;
	int bScroll;
	int x,y;
	static int old_x,old_y;

	CDSide	*side = m_mine->CurrSide ();

texture [0] = side->nBaseTex & 0x3fff;
texture [1] = side->nOvlTex;

// if texture1 is a scrolling texture, then offset the textures and
// redraw them, then return
bScroll = m_mine->ScrollSpeed (texture [0], &x, &y);
if (bScroll) {
	DrawTexture (texture [0], texture [1], scroll_offset_x, scroll_offset_y);
	if (old_x != x || old_y != y) {
		scroll_offset_x = 0;
		scroll_offset_y = 0;
		}
	old_x = x;
	old_y = y;
	scroll_offset_x += x;
	scroll_offset_y += y;
	scroll_offset_x &= 63;
	scroll_offset_y &= 63;
	return;
	}

scroll_offset_x = 0;
scroll_offset_y = 0;

// abort if this is not a wall
#ifndef _DEBUG
UINT16 nWall = side->nWall;
if (nWall >= m_mine->GameInfo ().walls.count)
	return;

// abort if this wall is not a door
if (m_mine->Walls () [nWall].type != WALL_DOOR)
	return;
#endif
	int i;
	static int hold_time [2] = {0,0};
	static int inc [2]= {1,1}; // 1=forward or -1=backwards
	int index [2];
	static UINT16 d1_anims [] = {
		371, 376, 387, 399, 413, 419, 424, 436, 444, 459,
		472, 486, 492, 500, 508, 515, 521, 529, 536, 543,
		550, 563, 570, 577, 584, 0
		};
// note: 584 is not an anim texture, but it is used to calculate
//       the number of frames in 577
// The 0 is used to end the search

	static UINT16 d2_anims [] = {
		435, 440, 451, 463, 477, 483, 488, 500, 508, 523,
		536, 550, 556, 564, 572, 579, 585, 593, 600, 608,
		615, 628, 635, 642, 649, 664, 672, 687, 702, 717,
		725, 731, 738, 745, 754, 763, 772, 780, 790, 806,
		817 ,827 ,838 ,849 ,858 ,863 ,871 ,886, 901 ,910,
		0
		};
// note: 910 is not an anim texture, but it is used to calculate
//       the number of frames in 901
// The 0 is used to end the search
	UINT16 *anim; // points to d1_anim or d2_anim depending on file_type

// first find out if one of the textures is animated
anim = (file_type == RDL_FILE) ? d1_anims : d2_anims;

for (i=0; i<2;i++)
	for (index [i] = 0; anim [index [i]]; index [i]++)
		if (texture [i] == anim [index [i]])
			break;

if (anim [index [0]] || anim [index [1]]) {
	// calculate new texture numbers
	for (i = 0; i < 2; i++) {
		if (anim [index [i]]) {
		// if hold time has not expired, then return
			if (hold_time [i] < 5)
				hold_time [i]++;
			else
				m_frame [i] += inc [i];
			if (m_frame [i] < 0) {
				m_frame [i] = 0;
				hold_time [i] = 0;
				inc [i] = 1;
				}
			if (anim [index [i]] + m_frame [i] >= anim [index [i] + 1]) {
				m_frame [i] = (anim [index [i] + 1] - anim [index [i]]) - 1;
				hold_time [i] = 0;
				inc [i] = -1;
				}
			texture [i] = anim [index [i]] + m_frame [i];
			}
		}
	DrawTexture (texture [0], texture [1], 0, 0);
	}
}
#endif
                        /*--------------------------*/

void CTextureTool::OnTimer (UINT nIdEvent)
{
#if TEXTOOLDLG
if (nIdEvent == 2)
	AnimateLight ();
else if (nIdEvent == 3)
	OnEditTimer ();
else 
	CTexToolDlg::OnTimer (nIdEvent);
#else
if (nIdEvent == 1)
	AnimateTexture ();
else if (nIdEvent == 2)
	AnimateLight ();
else if (nIdEvent == 3)
	OnEditTimer ();
else 
	CToolDlg::OnTimer (nIdEvent);
#endif
}

                        /*--------------------------*/

void CTextureTool::SelectTexture (int nIdC, bool bFirst)
{
if (!GetMine ())
	return;

	CDSide		*side = m_mine->CurrSide ();
	CComboBox	*pcb = bFirst ? CBTexture1 () : CBTexture2 ();
	int			index = pcb->GetCurSel ();
	
if (index <= 0)
	side->nOvlTex = 0;
else {
	INT16 texture = (INT16) pcb->GetItemData (index);
	if (bFirst)
		m_mine->SetTexture (-1, -1, texture, -1);
	else
		m_mine->SetTexture (-1, -1, -1, texture);
	}
Refresh ();
theApp.MineView ()->Refresh ();
}

                        /*--------------------------*/

void CTextureTool::OnSetLight () 
{
if (!GetMine ())
	return;
UpdateData (TRUE);
CDSide *side = m_mine->CurrSide ();
INT16 mode = side->nOvlTex & 0xc000;
int i, j;
for (i = 0; i < 4; i++) {
	j = (i + lightIdxFromMode [mode / 0x4000]) % 4;
	side->uvls [i].l = (UINT16) (m_lights [j] * 327.68);
	}
theApp.MineView ()->Refresh ();
}

                        /*--------------------------*/

void CTextureTool::OnSelect1st () 
{
SelectTexture (IDC_TEXTURE_1ST, true);
}

                        /*--------------------------*/

void CTextureTool::OnSelect2nd () 
{
SelectTexture (IDC_TEXTURE_2ND, false);
}

                        /*--------------------------*/

void CTextureTool::OnEditTexture () 
{
	CTextureEdit	e (NULL);

int i = e.DoModal ();
theApp.MineView ()->Refresh (false);
Refresh ();
}

                        /*--------------------------*/

void CTextureTool::OnSaveTexture () 
{
if (!GetMine ())
	return;

	HINSTANCE	hInst = AfxGetApp()->m_hInstance;
	char			t1Name [20],
					t2Name [20];
	CDSide		*side = m_mine->CurrSide ();
	CComboBox	*pcb;

save_texture1 = side->nBaseTex & 0x3fff;
save_texture2 = side->nOvlTex & 0x3fff;
int i;
for (i = 0; i < 4; i++)
	save_uvls [i].l = side->uvls [i].l;

//CBTexture1 ()->SelectString (-1, texture_name1);
//CBTexture2 ()->SelectString (-1, texture_name2);
pcb = CBTexture1 ();
LoadString (hInst, texture_resource + pcb->GetItemData (pcb->GetCurSel ()), t1Name, sizeof (t1Name));
pcb = CBTexture2 ();
if (i = pcb->GetItemData (pcb->GetCurSel ()))
	LoadString (hInst, texture_resource + i, t2Name, sizeof (t2Name));
else
	strcpy (t2Name, "(none)");
sprintf(m_szTextureBuf ,"%s,%s", t1Name, t2Name);
UpdateData (FALSE);
//SaveTextureStatic->SetText(message);
}

                        /*--------------------------*/

void CTextureTool::OnPaste1st () 
{
m_bUse1st = !m_bUse1st;
Refresh ();
}

                        /*--------------------------*/

void CTextureTool::OnPaste2nd () 
{
m_bUse2nd = !m_bUse2nd;
Refresh ();
}

                        /*--------------------------*/

void CTextureTool::OnPasteSide () 
{
UpdateData (TRUE);
if (!(m_bUse1st || m_bUse2nd))
	return;
if (!GetMine ())
	return;

	CDSide *side = m_mine->CurrSide ();

//CheckForDoor ();
theApp.SetModified (TRUE);
m_mine->SetTexture (m_mine->Current ()->segment, m_mine->Current ()->side, 
						  m_bUse1st ? save_texture1 : -1, m_bUse2nd ? save_texture2 : -1);
int i;
for (i = 0; i < 4; i++)
	side->uvls [i].l = save_uvls [i].l;
Refresh ();
theApp.MineView ()->Refresh ();
}


                        /*--------------------------*/

void CTextureTool::OnPasteTouching ()
{
UpdateData (TRUE);
if (!(m_bUse1st || m_bUse2nd))
	return;
if (!GetMine ())
	return;
if (save_texture1 == -1 || save_texture2 == -1)
	return;
//CheckForDoor ();
// set all segment sides as not "pasted" yet
	CDSegment *seg = m_mine->Segments ();
int segnum;
for (segnum = m_mine->SegCount (); segnum; segnum--, seg++)
    seg->seg_number = 0;
theApp.SetModified (TRUE);
theApp.LockUndo ();
PasteTexture (m_mine->Current ()->segment, m_mine->Current ()->side, 100);
theApp.UnlockUndo ();
Refresh ();
theApp.MineView ()->Refresh ();
}

                        /*--------------------------*/

void CTextureTool::OnPasteMarked () 
{
UpdateData (TRUE);
if (!(m_bUse1st || m_bUse2nd))
	return;
if (!GetMine ())
	return;

	INT16			segnum,
					sidenum;
	CDSegment	*seg = m_mine->Segments ();
	CDSide		*side;
	bool			bChange = false,
					bAll = !m_mine->GotMarkedSides ();

if (bAll && (QueryMsg ("Paste texture to entire mine?") != IDYES))
	return;
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bAll)
	INFOMSG (" Pasting texture in entire mine.");
for (segnum = 0; segnum < m_mine->SegCount (); segnum++, seg++) {
	for (sidenum = 0, side = seg->sides; sidenum < 6; sidenum++, side++) {
		if (bAll || m_mine->SideIsMarked (segnum, sidenum)) {
			if (seg->children [sidenum] == -1) {
				bChange = true;
				m_mine->SetTexture (segnum, sidenum, m_bUse1st ? save_texture1 : -1, m_bUse2nd ? save_texture2 : -1);
				int i;
				for (i = 0; i < 4; i++)
					side->uvls [i].l = save_uvls [i].l;
				}
			}
		}
	}
if (bChange)
	theApp.UnlockUndo ();
else
	theApp.ResetModified (bUndo);
Refresh ();
theApp.MineView ()->Refresh ();
}

                        /*--------------------------*/

void CTextureTool::OnReplace () 
{
UpdateData (TRUE);
if (!(m_bUse1st || m_bUse2nd))
	return;
if (!GetMine ())
	return;

	INT16			segnum,
					sidenum;
	CDSegment	*seg = m_mine->Segments ();
	CDSide		*side;
	bool			bChange = false,
					bAll = !m_mine->GotMarkedSides ();

if (bAll && (QueryMsg ("Replace textures in entire mine?") != IDYES))
	return;
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bAll)
	INFOMSG (" Replacing textures in entire mine.");
for (segnum = 0; segnum < m_mine->SegCount (); segnum++, seg++)
	for (sidenum = 0, side = seg->sides; sidenum < 6; sidenum++, side++)
		if (bAll || m_mine->SideIsMarked (segnum, sidenum)) {
			if (m_bUse1st && (side->nBaseTex != last_texture1))
				continue;
			if (m_bUse2nd && ((side->nOvlTex & 0x3FFF) != last_texture2))
				continue;
			if ((seg->children [sidenum] >= 0) && (side->nWall == NO_WALL))
				 continue;
			if (m_mine->SetTexture (segnum, sidenum, m_bUse1st ? save_texture1 : -1, m_bUse2nd ? save_texture2 : -1))
				bChange = true;
//			int i;
//			for (i = 0; i < 4; i++)
//				side->uvls [i].l = save_uvls [i].l;
			}
if (bChange)
	theApp.UnlockUndo ();
else
	theApp.ResetModified (bUndo);
Refresh ();
theApp.MineView ()->Refresh ();
}

                        /*--------------------------*/

void CTextureTool::PasteTexture (INT16 segnum, INT16 sidenum, INT16 nDepth) 
{
if (nDepth <= 0) 
	return;

if (!GetMine ())
	return;

	CDSegment	*seg = m_mine->Segments (segnum);
	CDSide		*side = seg->sides + sidenum;
	INT16			old_texture1, 
					old_texture2;
	int			i;

// remember these texture for a comparison below
old_texture1 = side->nBaseTex;
old_texture2 = side->nOvlTex;
if ((old_texture1 < 0) || (old_texture1 >= MAX_TEXTURES))
	old_texture1 = 0;
if ((old_texture2 < 0) || (old_texture2 >= MAX_TEXTURES))
	old_texture2 = 0;
// mark segment as "pasted"
seg->seg_number = 1;
// paste texture
m_mine->SetTexture (segnum, sidenum, m_bUse1st ? save_texture1 : -1, m_bUse2nd ? save_texture2 : -1);
for (i = 0; i < 4; i++)
	side->uvls [i].l = save_uvls [i].l;

// now check each adjing side to see it has the same texture
for (i = 0; i < 4; i++) {
	INT16 adj_segnum, adj_sidenum;
	if (GetAdjacentSide (segnum, sidenum, i, &adj_segnum, &adj_sidenum)) {
		// if adj matches and its not "pasted" yet
		seg = m_mine->Segments (adj_segnum);
		side = seg->sides + adj_sidenum;
#if 0
		if (seg->seg_number)
			continue;
		if (m_bUse1st && (side->nBaseTex != old_texture1))
			continue;
		if (m_bUse2nd && (side->nOvlTex != old_texture2))
			continue;
		PasteTexture (adj_segnum, adj_sidenum, --nDepth);
#else
		if ((seg->seg_number == 0) &&
			 (!m_bUse1st || (side->nBaseTex == old_texture1)) &&
			 (!m_bUse2nd || (side->nOvlTex == old_texture2))) {
			PasteTexture (adj_segnum, adj_sidenum, --nDepth);
			}
#endif
		}
	}
}

                        /*--------------------------*/

bool CTextureTool::GetAdjacentSide (INT16 start_segment, INT16 start_side, INT16 linenum,
												INT16 *adj_segnum, INT16 *adj_sidenum) 
{
if (!GetMine ())
	return false;

	CDSegment *seg;
	INT16 sidenum,childnum;
	INT16 point0,point1,vert0,vert1;
	INT16 childs_side,childs_line;
	INT16 childs_point0,childs_point1,childs_vert0,childs_vert1;
	int side_child[6][4] = {
		{4,3,5,1},
		{2,4,0,5},
		{5,3,4,1},
		{0,4,2,5},
		{2,3,0,1},
		{0,3,2,1}
		};

  // figure out which side of child shares two points w/ start_side
  // find vert numbers for the line's two end points
point0 = line_vert[side_line[start_side][linenum]][0];
point1 = line_vert[side_line[start_side][linenum]][1];
seg = m_mine->Segments () + start_segment;
vert0  = seg->verts[point0];
vert1  = seg->verts[point1];

sidenum = side_child[start_side][linenum];
childnum = seg->children[sidenum];
if (childnum < 0 || childnum >= m_mine->SegCount ())
	return false;
for (childs_side=0;childs_side<6;childs_side++) {
	seg = m_mine->Segments () + childnum;
	if ((seg->children[childs_side] == -1) ||
	    (seg->sides[childs_side].nWall < m_mine->GameInfo ().walls.count)) {
		for (childs_line=0;childs_line<4;childs_line++) {
			// find vert numbers for the line's two end points
			childs_point0 = line_vert[side_line[childs_side][childs_line]][0];
			childs_point1 = line_vert[side_line[childs_side][childs_line]][1];
			childs_vert0  = seg->verts[childs_point0];
			childs_vert1  = seg->verts[childs_point1];
			// if points of child's line == corresponding points of parent
			if (childs_vert0 == vert1 && childs_vert1 == vert0 ||
				childs_vert0 == vert0 && childs_vert1 == vert1) {
				// now we know the child's side & line which touches the parent
				// child:  childnum, childs_side, childs_line, childs_point0, childs_point1
				// parent: start_segment, start_side, linenum, point0, point1
				*adj_segnum = childnum;
				*adj_sidenum = childs_side;
				return true;
				}
			}
		}
	}
return false;
}
                        /*--------------------------*/

void CTextureTool::GetBrightness (int nTexture)
{
	int nBrightness;

if (nTexture < 0)
	nBrightness = 0;
else {
	nBrightness = lightMap [nTexture];
	if (nBrightness == MAX_BRIGHTNESS)
		nBrightness = 100;
	else
		nBrightness = (100 * nBrightness + MAX_BRIGHTNESS / 2) / MAX_BRIGHTNESS;
	((CSliderCtrl *) BrightnessSlider ())->SetPos (nBrightness);
	}
m_nBrightness = nBrightness;
}

                        /*--------------------------*/

void CTextureTool::SetBrightness (int nBrightness)
{
	static	BOOL	bSemaphore = FALSE;

if (!bSemaphore) {
	bSemaphore = TRUE;

	CComboBox	*pcb = (m_bUse2nd && !m_bUse1st) ? CBTexture2 () : CBTexture1 ();
	int			index = pcb->GetCurSel ();
	INT16			texture = (INT16) pcb->GetItemData (index);

	if (texture >= 0) {
		m_nBrightness = nBrightness;
		lightMap [texture] = ((nBrightness == 100) ? MAX_BRIGHTNESS : nBrightness * (MAX_BRIGHTNESS / 100));
		}
	bSemaphore = FALSE;
	}
}

                        /*--------------------------*/

void CTextureTool::OnBrightnessEdit ()
{
UpdateData (TRUE);
}

                        /*--------------------------*/

void CTextureTool::OnHScroll(UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
	int	nPos = pScrollBar->GetScrollPos ();
	CRect	rc;

if (pScrollBar == BrightnessSlider ()) {
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
	else if (nPos > 100)
		nPos = 100;
	SetBrightness (nPos);
	UpdatePaletteWnd ();
	UpdateData (FALSE);
//	pScrollBar->SetScrollPos (nPos, TRUE);
	}
else {
	m_alignWnd.GetClientRect (rc);
	switch (scrollCode) {
	case SB_LINEUP:
		nPos--;
		break;
	case SB_LINEDOWN:
		nPos++;
		break;
	case SB_PAGEUP:
		nPos -= rc.Width () / 4;
		break;
	case SB_PAGEDOWN:
		nPos += rc.Width () / 4;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nPos = thumbPos;
		break;
	case SB_ENDSCROLL:
		return;
	}
  }
pScrollBar->SetScrollPos (nPos, TRUE);
UpdateAlignWnd ();
}

                        /*--------------------------*/

void CTextureTool::OnVScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
	int	nPos = pScrollBar->GetScrollPos ();
	CRect	rc;

if (pScrollBar == TimerSlider ()) {
	if (m_iLight < 0)
		return;
	switch (scrollCode) {
		case SB_LINEUP:
			nPos++;
			break;
		case SB_LINEDOWN:
			nPos--;
			break;
		case SB_PAGEUP:
			nPos += 5;
			break;
		case SB_PAGEDOWN:
			nPos -= 5;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nPos = 20 - thumbPos;
			break;
		case SB_ENDSCROLL:
			return;
		}
	if (nPos < 0)
		nPos = 0;
	else if (nPos > 20)
		nPos = 20;
	SetLightDelay (nPos * 50);
//	pScrollBar->SetScrollPos (nPos, TRUE);
	}
else if (pScrollBar == BrightnessSlider ()) {
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
	else if (nPos > 100)
		nPos = 100;
	SetBrightness (nPos);
	UpdateData (FALSE);
//	pScrollBar->SetScrollPos (nPos, TRUE);
	}
else {
	m_alignWnd.GetClientRect (rc);
	switch (scrollCode) {
		case SB_LINEUP:
			nPos--;
			break;
		case SB_LINEDOWN:
			nPos++;
			break;
		case SB_PAGEUP:
			nPos -= rc.Height () / 4;
			break;
		case SB_PAGEDOWN:
			nPos += rc.Height () / 4;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nPos = thumbPos;
			break;
		case SB_ENDSCROLL:
			return;
		}
	pScrollBar->SetScrollPos (nPos, TRUE);
	UpdateAlignWnd ();
	}
}

                        /*--------------------------*/

BOOL CTextureTool::OnNotify (WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	LPNMHDR	nmHdr = (LPNMHDR) lParam;
	int		nMsg = nmHdr->code;

switch (wParam) {
	case IDC_TEXALIGN_HALIGN:
	case IDC_TEXALIGN_VALIGN:
	case IDC_TEXALIGN_RALIGN:
	case IDC_TEXALIGN_ZOOMIN:
	case IDC_TEXALIGN_ZOOMOUT:
	case IDC_TEXALIGN_HALEFT:
	case IDC_TEXALIGN_HARIGHT:
	case IDC_TEXALIGN_VAUP:
	case IDC_TEXALIGN_VADOWN:
	case IDC_TEXALIGN_RALEFT:
	case IDC_TEXALIGN_RARIGHT:
	case IDC_TEXALIGN_HSHRINK:
	case IDC_TEXALIGN_VSHRINK:
		if (((LPNMHDR) lParam)->code == WM_LBUTTONDOWN) {
			m_nEditFunc = wParam;
			m_nEditTimer = SetTimer (3, m_nTimerDelay = 250U, NULL);
			}
		else {
			m_nEditFunc = -1;
			if (m_nEditTimer >= 0) {
				KillTimer (m_nEditTimer);
				m_nEditTimer = -1;
				}
			}
		break;
	case IDC_TEXLIGHT_COLOR:
		return 0;
	default:
/*		if (((LPNMHDR) lParam)->code == WM_LBUTTONDOWN)
			OnLButtonDown ();
		else 
*/		return CTexToolDlg::OnNotify (wParam, lParam, pResult);
	}
*pResult = 0;
return TRUE;
}
		
                        /*--------------------------*/

void CTextureTool::OnEditTimer (void)
{		
switch (m_nEditFunc) {
	case IDC_TEXALIGN_HALIGN:
		OnAlignX ();
		break;
	case IDC_TEXALIGN_VALIGN:
		OnAlignY ();
		break;
	case IDC_TEXALIGN_ZOOMIN:
		OnZoomIn ();
		break;
	case IDC_TEXALIGN_ZOOMOUT:
		OnZoomOut ();
		break;
	case IDC_TEXALIGN_HALEFT:
		OnAlignLeft ();
		break;
	case IDC_TEXALIGN_HARIGHT:
		OnAlignRight ();
		break;
	case IDC_TEXALIGN_VAUP:
		OnAlignUp ();
		break;
	case IDC_TEXALIGN_VADOWN:
		OnAlignDown ();
		break;
	case IDC_TEXALIGN_RALEFT:
		OnAlignRotLeft ();
		break;
	case IDC_TEXALIGN_RARIGHT:
		OnAlignRotRight ();
		break;
	case IDC_TEXALIGN_HSHRINK:
		OnHShrink ();
		break;
	case IDC_TEXALIGN_VSHRINK:
		OnVShrink ();
		break;
	default:
		break;
	}
UINT i = (m_nTimerDelay * 9) / 10;
if (i >= 25) {
	KillTimer (m_nTimer);
	m_nTimer = SetTimer (3, m_nTimerDelay = i, NULL);
	}
}

                        /*--------------------------*/
		
		//eof texturedlg.cpp
