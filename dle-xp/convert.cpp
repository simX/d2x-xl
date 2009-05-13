// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "dlc.h"
#include "mine.h"
#include "global.h"
#include "dlcdoc.h"
#include "textures.h"

BEGIN_MESSAGE_MAP (CConvertDlg, CDialog)
	ON_WM_PAINT ()
	ON_CBN_SELCHANGE (IDC_CONVERT_D1, OnSetD1)
	ON_CBN_SELCHANGE (IDC_CONVERT_D2, OnSetD2)
END_MESSAGE_MAP ()

//------------------------------------------------------------------------
// DIALOG - CConvertDlg (constructor)
//------------------------------------------------------------------------

CConvertDlg::CConvertDlg (CWnd *pParent)
	: CDialog (IDD_CONVERT, pParent)
{
m_bInited = false;
m_mine = NULL;
}

//------------------------------------------------------------------------
// CConvertDlg - ~CConvertDlg (destructor)
//------------------------------------------------------------------------

void CConvertDlg::EndDialog (int nResult) 
{
if (m_bInited) {
	m_showD1.DestroyWindow ();
	m_showD2.DestroyWindow ();
	}
CDialog::EndDialog (nResult);
}

                        /*--------------------------*/

CMine *CConvertDlg::GetMine (void)
{
return m_mine = theApp.GetMine (); 
}

                        /*--------------------------*/

void CConvertDlg::CreateImgWnd (CWnd *pImgWnd, int nIdC)
{
CWnd *pParentWnd = GetDlgItem (nIdC);
CRect rc;
pParentWnd->GetClientRect (rc);
pImgWnd->Create (NULL, NULL, WS_CHILD | WS_VISIBLE, rc, pParentWnd, 0);
}

//------------------------------------------------------------------------
// CConvertDlg - SetupWindow
//------------------------------------------------------------------------

BOOL CConvertDlg::OnInitDialog () 
{
if (!GetMine ())
	return FALSE;

CDialog::OnInitDialog ();

CreateImgWnd (&m_showD1, IDC_CONVERT_SHOWD1);
CreateImgWnd (&m_showD2, IDC_CONVERT_SHOWD2);

m_hInst = AfxGetApp()->m_hInstance;
HRSRC hFind = FindResource (m_hInst, MAKEINTRESOURCE (IDR_TEXTURE_D1D2), "RC_DATA");
if (!hFind)
	return FALSE;
m_hTextures = LoadResource (m_hInst, hFind);
if (!m_hTextures)
	return FALSE;
m_pTextures = (INT16 *) LockResource (m_hTextures);
if (!m_pTextures)
	return FALSE;

CComboBox *pcb = CBD1 ();
INT16	nSeg,	nSide, nTextures;
INT16 tnum [2], segCount = m_mine->SegCount ();
char	szName [80];
int h;
CDSegment *seg = m_mine->Segments ();
CDSide *side;
// add textures that have been used to Texture 1 combo box
for (nSeg = segCount; nSeg; nSeg--, seg++) {
	for (side = seg->sides, nSide = 6; nSide; nSide--, side++) {
		tnum [0] = side->nBaseTex;
		tnum [1] = side->nOvlTex & 0x1fff;
		int i;
		for (i = 0; i < 2; i++) {
			if (tnum [i] != -1) {
				// read name of texture from Descent 1 texture resource
				LoadString (m_hInst, D1_TEXTURE_STRING_TABLE + tnum [i], szName, sizeof (szName));
				// if not already in list box, add it
				if (pcb->FindStringExact (-1, szName) < 0) {
					h = pcb->AddString (szName);
					pcb->SetItemData (h, tnum [i]);
					}
				}
			}
		}
	}
pcb->SetCurSel (0);

  // add complete set for Texture 2 combo box
nTextures = (file_type == RDL_FILE) ? MAX_D1_TEXTURES : MAX_D2_TEXTURES;
pcb = CBD2 ();
int i;
for (i = 0; i < nTextures; i++) {
// read szName of texture from Descent 2 texture resource
	LoadString (m_hInst, D2_TEXTURE_STRING_TABLE + i, szName, sizeof (szName));
	if (*szName != '*') {
		h = pcb->AddString (szName);
		pcb->SetItemData (h, i);
		}
	}
m_bInited = true;
Refresh ();
return TRUE;
}

void CConvertDlg::DoDataExchange (CDataExchange *pDX)
{
Refresh ();
}

//------------------------------------------------------------------------
// CConvertDlg - RefreshData
//------------------------------------------------------------------------

void CConvertDlg::Refresh () 
{
if (!m_bInited)
	return;

	INT16 texture1,texture2;
	INT16 old_file_type;
#ifdef _DEBUG
	DWORD nError;
#endif

// find matching entry for Texture 1
//  CBD1 ()->GetSelString (message,sizeof(message));
//  texture1 = GetTextureID (message);
texture1 = (INT16) (CBD1 ()->GetItemData (CBD1 ()->GetCurSel ()));
texture2 = m_pTextures [texture1];
if (LoadString (m_hInst, D2_TEXTURE_STRING_TABLE + texture2, message, sizeof (message)))
	CBD2 ()->SelectString (-1, message);
#ifdef _DEBUG
else
	nError = GetLastError ();
#endif
old_file_type = file_type;
file_type = RL2_FILE;
m_mine->LoadPalette ();
PaintTexture (&m_showD2, 0, -1, -1, texture2, 0);
file_type = RDL_FILE;
m_mine->LoadPalette ();
PaintTexture (&m_showD1, 0, -1, -1, texture1, 0);

#if 0
  INT16 i;
  HRSRC hFind;
  HGLOBAL hPalette;
  UINT8 *palette;
  int rc;
  // define DIB header
  INT16 x_offset,y_offset;
  BITMAPINFO *bmi;
  BITMAPINFOHEADER *bi;
  typedef struct tagMyBMI {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD bmiColors[256];
  } MyBMI;

  MyBMI my_bmi;

  // point bitmap info to structure
  bmi = (BITMAPINFO *)&my_bmi;

  // point the info to the bitmap header structure
  bi = &bmi->bmiHeader;

  // define bit map info header elements
  bi->biSize          = sizeof(BITMAPINFOHEADER);
  bi->biWidth         = 64;
  bi->biHeight        = 64;
  bi->biPlanes        = 1;
  bi->biBitCount      = 8;
  bi->biCompression   = BI_RGB;
  bi->biSizeImage     = 0;
  bi->biXPelsPerMeter = 0;
  bi->biYPelsPerMeter = 0;
  bi->biClrUsed       = 0;
  bi->biClrImportant  = 0;

  HDC hdc = GetDC(HWindow);

  // define d1 palette from resource
  old_file_type = file_type;
  file_type = RDL_FILE;
  hFind = FindResource( hInst,PaletteResource(), RT_RCDATA);
  hPalette = LoadResource( hInst, hFind);
  palette = (UINT8 *) LockResource(hPalette);
  for (i=0;i<256;i++) {
    bmi->bmiColors[i].rgbRed    = (*palette++)<<2;
    bmi->bmiColors[i].rgbGreen  = (*palette++)<<2;
    bmi->bmiColors[i].rgbBlue   = (*palette++)<<2;
    bmi->bmiColors[i].rgbReserved = 0;
  }
  FreeResource(hPalette);

  // draw texture 1
  UINT8 bitmap_array[64*64];
  rc = ReadTextureFromFile(texture1,bitmap_array);
  if (rc) {
    // TODO: show message instead of picture
  } else {
	x_offset = Texture1Button->Attr.X;
	y_offset = Texture1Button->Attr.Y;
//	x_offset = 55;
//  y_offset = 45;
    HPALETTE OldPalette = SelectPalette(hdc, ThePalette, FALSE);
    RealizePalette(hdc);
    SetDIBitsToDevice(hdc, x_offset, y_offset, 64, 64, 0, 0, 0, 64, bitmap_array, bmi, DIB_RGB_COLORS);
    SelectPalette(hdc, OldPalette, FALSE);
  }

  // define d2 palette from resource
  file_type = RL2_FILE;
  hFind = FindResource( hInst,PaletteResource(), RT_RCDATA);
  hPalette = LoadResource( hInst, hFind);
  palette = (UINT8 *) LockResource(hPalette);
  for (i=0;i<256;i++) {
	bmi->bmiColors[i].rgbRed    = (*palette++)<<2;
    bmi->bmiColors[i].rgbGreen  = (*palette++)<<2;
    bmi->bmiColors[i].rgbBlue   = (*palette++)<<2;
	bmi->bmiColors[i].rgbReserved = 0;
  }
  FreeResource(hPalette);

  // draw texture 2
  rc = ReadTextureFromFile(texture2,bitmap_array);
  if (rc) { // if error occured
    // TODO: show message instead of picture
  } else {
	x_offset = Texture2Button->Attr.X;
	y_offset = Texture2Button->Attr.Y;
//	x_offset = 215;
//	y_offset = 45;
	HPALETTE OldPalette = SelectPalette(hdc, ThePalette, FALSE);
	RealizePalette(hdc);
	SetDIBitsToDevice(hdc, x_offset, y_offset, 64, 64, 0, 0, 0, 64, bitmap_array, bmi, DIB_RGB_COLORS);
	SelectPalette(hdc, OldPalette, FALSE);
  }

  ReleaseDC(HWindow, hdc);
#endif
  // restore file type (should always be RDL_TYPE)
  file_type = old_file_type;
}

//------------------------------------------------------------------------
// CConvertDlg - Sent if we need to redraw an item
//------------------------------------------------------------------------

void CConvertDlg::OnPaint ()
{
CDialog::OnPaint ();
Refresh ();
}

//------------------------------------------------------------------------
// CConvertDlg - Ok
//------------------------------------------------------------------------
/*
void CConvertDlg::OnOK ()
{
Convert ();
CDialog::OnOK ();
}
*/
//------------------------------------------------------------------------
// CConvertDlg - Texture1 Msg
//------------------------------------------------------------------------

void CConvertDlg::OnSetD1 ()
{
Refresh ();
}

//------------------------------------------------------------------------
// CConvertDlg - Texture1 Msg
//------------------------------------------------------------------------

void CConvertDlg::OnSetD2 ()
{
INT16 texture1 = (INT16) (CBD1 ()->GetItemData (CBD1 ()->GetCurSel ()));
INT16 texture2 = (INT16) (CBD2 ()->GetItemData (CBD2 ()->GetCurSel ()));
m_pTextures [texture1] = texture2;
Refresh ();
}

//------------------------------------------------------------------------
// CConvertDlg - Convert
//------------------------------------------------------------------------

void CConvertDlg::OnOK () 
{
if (!GetMine ())
	return;

  INT16		i,j;
  CDSegment *seg;
  CDSide		*side;
  CDWall		*wall;
  CDTrigger	*trigger;
  CDObject	*obj;
  INT16		segnum, sidenum, d1Texture, mode,
				segCount = m_mine->SegCount (),
				wallCount = m_mine->GameInfo ().walls.count;

FreeTextureHandles ();
theApp.ResetUndoBuffer ();	//no undo possible; palette changes to difficult to handle
// reload internal stuff for d2
file_type = RL2_FILE;
texture_resource = D2_TEXTURE_STRING_TABLE;
m_mine->LoadPalette ();

  // convert textures
for (segnum = 0, seg = m_mine->Segments (); segnum < segCount; segnum++, seg++) {
	seg->s2_flags = 0;
	for (sidenum = 0, side = seg->sides; sidenum < 6; sidenum++) {
		if ((seg->children [sidenum] == -1) || (seg->sides [sidenum].nWall < wallCount)) {
			d1Texture = seg->sides [sidenum].nBaseTex;
			if ((d1Texture >= 0) && (d1Texture < MAX_D1_TEXTURES))
				seg->sides[sidenum].nBaseTex = m_pTextures [d1Texture];
			else { 
				DEBUGMSG (" Level converter: Invalid texture 1 found")
				seg->sides [sidenum].nBaseTex = 0;
				}
			d1Texture = seg->sides [sidenum].nOvlTex & 0x1fff;
			mode = seg->sides[sidenum].nOvlTex & 0xc000;
			if (d1Texture > 0 && d1Texture < MAX_D1_TEXTURES)
				seg->sides [sidenum].nOvlTex = m_pTextures [d1Texture] | mode;
			else if (d1Texture < 0) {
				DEBUGMSG (" Level converter: Invalid texture 2 found")
				seg->sides [sidenum].nOvlTex = 0;
				}
			}
		}
	}

// defined D2 wall parameters
//--------------------------------------
for (i = 0, wall = m_mine->Walls (); i < wallCount; i++, wall++) {
	wall->controlling_trigger = 0;
	wall->cloak_value = 0;
	}

// change trigger type and flags
//-------------------------------------------
for (i = 0, trigger = m_mine->Triggers (); i < m_mine->GameInfo ().triggers.count; i++, trigger++) {
	switch (trigger->flags) {
		case TRIGGER_CONTROL_DOORS:
			trigger->type = TT_OPEN_DOOR;
			break;
		case TRIGGER_EXIT:
			trigger->type = TT_EXIT;
			break;
		case TRIGGER_MATCEN:
			trigger->type = TT_MATCEN;
			break;
		case TRIGGER_ILLUSION_OFF:
			trigger->type = TT_ILLUSION_OFF;
			break;
		case TRIGGER_ILLUSION_ON:
			trigger->type = TT_ILLUSION_ON;
			break;
		case TRIGGER_SECRET_EXIT:
			trigger->type = TT_SECRET_EXIT;
			break;

		// unsupported types
		case TRIGGER_ON:
		case TRIGGER_ONE_SHOT:
		case TRIGGER_SHIELD_DAMAGE:
		case TRIGGER_ENERGY_DRAIN:
		default:
			DEBUGMSG (" Level converter: Unsupported trigger type; trigger deleted")
			m_mine->DeleteTrigger (i);
			i--;
			trigger--;
			continue;
		}
	trigger->flags = 0;
	}

// set robot_center fuelcen_num and robot_flags2
//-----------------------------------------------
for (i = 0; i < m_mine->GameInfo ().botgen.count; i++) {
	m_mine->BotGens (i)->objFlags [1] = 0;
	for (j = 0, seg = m_mine->Segments (); j <= segCount; j++, seg++)
		if ((seg->special == SEGMENT_IS_ROBOTMAKER) && (seg->matcen_num == i))
				m_mine->BotGens (i)->fuelcen_num = (INT16)(seg->value);
	}

// set equip_center fuelcen_num and robot_flags2
//-----------------------------------------------
for (i = 0; i < m_mine->GameInfo ().equipgen.count; i++) {
	m_mine->EquipGens (i)->objFlags [1] = 0;
	for (j = 0, seg = m_mine->Segments (); j <= segCount; j++, seg++)
		if ((seg->special == SEGMENT_IS_EQUIPMAKER) && (seg->matcen_num == i))
				m_mine->EquipGens (i)->fuelcen_num = (INT16)(seg->value);
	}

// Objects ()
//-----------------------------------------------

for (i = 0, obj = m_mine->Objects (); i < m_mine->GameInfo ().objects.count; i++, obj++) {
// fix clip numbers for poly Objects () (except robots)
	switch (obj->type) {
		case OBJ_PLAYER   : // the player on the console
			obj->rtype.pobj_info.model_num = D2_PLAYER_CLIP_NUMBER;
			break;
		case OBJ_CNTRLCEN : // the control center
			obj->rtype.pobj_info.model_num = D2_REACTOR_CLIP_NUMBER;
			break;
		case OBJ_COOP     : // a cooperative player object
			obj->rtype.pobj_info.model_num = D2_COOP_CLIP_NUMBER;
			break;
		}
	}

// d2 light data and indicies
//--------------------------------------------
m_mine->GameInfo ().dl_indices.count = 0;
m_mine->GameInfo ().delta_lights.count = 0;
m_mine->FlickerLightCount () = 0;
m_mine->AutoAdjustLight (50.0, true);
m_mine->CalcAverageCornerLight (true);
m_mine->ScaleCornerLight (100.0, true);
m_mine->SetCubeLight (50.0, true);
m_mine->CalcDeltaLightData (50.0, (int) true);

// d2 reactor and secret cube
//----------------------------------------------
m_mine->ReactorTime () = 0x1e;
m_mine->ReactorStrength () = 0xffffffffL;
m_mine->SecretCubeNum () = 0L;

m_mine->SecretOrient ().rvec.x = F1_0;
m_mine->SecretOrient ().rvec.y = 0L;
m_mine->SecretOrient ().rvec.z = 0L;

m_mine->SecretOrient ().uvec.x = 0L;
m_mine->SecretOrient ().uvec.y = 0L;
m_mine->SecretOrient ().uvec.z = F1_0;

m_mine->SecretOrient ().fvec.x = 0L;
m_mine->SecretOrient ().fvec.y = F1_0;
m_mine->SecretOrient ().fvec.z = 0L;
theApp.MineView ()->ResetView (true);
CDialog::OnOK ();
}

// eof convert.cpp