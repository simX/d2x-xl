// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <commdlg.h>
#include <math.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "dlc.h"
#include "mine.h"
#include "global.h"
#include "dlcres.h"
#include "textures.h"
#include "dlc.h"
#include "texedit.h"

                        /*--------------------------*/

BEGIN_MESSAGE_MAP (CPaletteWnd, CWnd)
#if 0
	ON_WM_LBUTTONDOWN ()
	ON_WM_RBUTTONDOWN ()
	ON_WM_LBUTTONUP ()
	ON_WM_RBUTTONUP ()
#endif
END_MESSAGE_MAP ()

                        /*--------------------------*/

BEGIN_MESSAGE_MAP (CTextureEdit, CDialog)
	ON_WM_PAINT ()
	ON_WM_MOUSEMOVE ()
	ON_WM_LBUTTONDOWN ()
	ON_WM_RBUTTONDOWN ()
	ON_WM_LBUTTONUP ()
	ON_WM_RBUTTONUP ()
	ON_BN_CLICKED (IDC_TEXEDIT_DEFAULT, OnDefault)
	ON_BN_CLICKED (IDC_TEXEDIT_UNDO, OnUndo)
	ON_BN_CLICKED (IDC_TEXEDIT_LOAD, OnLoad)
	ON_BN_CLICKED (IDC_TEXEDIT_SAVE, OnSave)
END_MESSAGE_MAP ()

                        /*--------------------------*/

CPaletteWnd::CPaletteWnd ()
{
m_nWidth =
m_nHeight = 0;
m_pDC = NULL;
m_pOldPal = NULL;
}

                        /*--------------------------*/

CPaletteWnd::~CPaletteWnd ()
{
}

                        /*--------------------------*/

#define	MINRGB(rgb)	(((rgb)->peRed < (rgb)->peGreen) ? ((rgb)->peRed < (rgb)->peBlue) ? (rgb)->peRed : (rgb)->peBlue : ((rgb)->peGreen < (rgb)->peBlue) ? (rgb)->peGreen : (rgb)->peBlue)
#define	MAXRGB(rgb)	(((rgb)->peRed > (rgb)->peGreen) ? ((rgb)->peRed > (rgb)->peBlue) ? (rgb)->peRed : (rgb)->peBlue : ((rgb)->peGreen > (rgb)->peBlue) ? (rgb)->peGreen : (rgb)->peBlue)

#define sqr(v)	(((int)(v))*((int)(v)))

int CPaletteWnd::CmpColors (PALETTEENTRY *c, PALETTEENTRY *m)
{
int i = c->peRed + c->peGreen + c->peBlue; //Luminance (c->peRed, c->peGreen, c->peBlue);
int j = m->peRed + m->peGreen + m->peBlue; //Luminance (m->peRed, m->peGreen, m->peBlue);
if (i < j)
	return -1;
if (i > j)
	return 1;
if (c->peRed < m->peRed)
	return -1;
if (c->peRed > m->peRed)
	return 1;
if (c->peGreen < m->peGreen)
	return -1;
if (c->peGreen > m->peGreen)
	return 1;
if (c->peBlue < m->peBlue)
	return -1;
if (c->peBlue > m->peBlue)
	return 1;
return 0;
}

                        /*--------------------------*/

void CPaletteWnd::SortPalette (int left, int right)
{
	int				l = left,
						r = right;
	PALETTEENTRY	m = m_palColors [(l + r) / 2];

do {
	while (CmpColors (m_palColors + l, &m) < 0)
		l++;
	while (CmpColors (m_palColors + r, &m) > 0)
		r--;
	if (l <= r) {
		if (l < r) {
			PALETTEENTRY h = m_palColors [l];
			m_palColors [l] = h = m_palColors [r];
			m_palColors [r] = h;
			UINT8 i = m_nSortedPalIdx [l];
			m_nSortedPalIdx [l] = m_nSortedPalIdx [r];
			m_nSortedPalIdx [r] = i;
			}
		l++;
		r--;
		}
	}
while (l <= r);
if (left < r)
	SortPalette (left, r);
if (l < right)
	SortPalette (l, right);
}

                        /*--------------------------*/

void CPaletteWnd::CreatePalette ()
{
int i;
for (i = 0; i < 256; i++) {
	m_nSortedPalIdx [i] = i;
	RgbFromIndex (i, m_palColors + i);
	}
}
                        /*--------------------------*/

void CPaletteWnd::Update ()
{
InvalidateRect (NULL);
UpdateWindow ();
}

                        /*--------------------------*/

int CPaletteWnd::Create (CWnd *pParentWnd, int nWidth, int nHeight)
{
	CRect	rc;

m_pParentWnd = pParentWnd;
pParentWnd->GetClientRect (rc);
m_nWidth = nWidth;
m_nHeight = nHeight;
if (m_nWidth < 0)
	m_nWidth = rc.Width () / 8;
if (m_nHeight < 0) {
	m_nHeight = rc.Height () / 8;
	if (m_nWidth * m_nHeight > 256)
		m_nHeight = (256 + m_nWidth - 1) / m_nWidth;
	}
return CWnd::Create (NULL, NULL, WS_CHILD | WS_VISIBLE, rc, pParentWnd, 0);
}

                        /*--------------------------*/

bool CPaletteWnd::SelectColor (CPoint& point, int& color, PALETTEENTRY *pRGB)
{
	CRect	rcPal;

GetClientRect (rcPal);
//ClientToScreen (rcPal);
// if over palette, redefine foreground color
if (PtInRect (rcPal, point)) {
	int x,y;
//	x = ((point.x - rcPal.left) >> 3)&127;
//	y = ((point.y - rcPal.top) >> 3)&31;
	x = (int) ((double) (point.x - rcPal.left) * ((double) m_nWidth / rcPal.Width ()));
	y = (int) ((double) (point.y - rcPal.top) * ((double) m_nHeight / rcPal.Height ()));
	int c = m_nWidth * y + x;
	if (c > 255)
		return false;
	color = m_nSortedPalIdx [c];
	if (pRGB)
		*pRGB = m_palColors [c];
	//RgbFromIndex (color, pRGB);
	return true;
	}
return false;
}

                        /*--------------------------*/

void CPaletteWnd::SetPalettePixel (int x, int y) 
{
	CRect	rc;

GetClientRect (&rc);
int dx, dy;
for (dy = 0; dy < 8; dy++)
	for (dx = 0; dx < 8; dx++)
		m_pDC->SetPixel ((x << 3) + dx + rc.left, (y << 3) + dy + rc.top,
							  PALETTEINDEX (y * m_nWidth + x));
}

                        /*--------------------------*/

void CPaletteWnd::DrawPalette (void) 
{
if (!BeginPaint ())
	return;
CreatePalette ();
//SortPalette (0, 255);
CRect rc;
GetClientRect (&rc);
UINT8 *pal_bitmap  = (UINT8 *) malloc (m_nWidth * m_nHeight);
int h, i, c, w, x, y;
for (c = 0, y = m_nHeight - 1; (y >= 0); y--) {
	for (x = 0, h = y * m_nWidth; x < m_nWidth; x++, h++) {
		if (!y)
			y = 0;
		pal_bitmap [h] = (c < 256) ? m_nSortedPalIdx [c++] : 0;
		}
	}
BITMAPINFO *bmi = MakeBitmap ();
bmi->bmiHeader.biWidth = m_nWidth;
bmi->bmiHeader.biHeight = m_nHeight;
if (m_nWidth & 1)
	for (i = 0; i < m_nHeight; i++) {
		w = (i == m_nHeight - 1) ? 256 % m_nWidth : m_nWidth;
		StretchDIBits (m_pDC->m_hDC, 0, i * 8, w * 8, 8, 0, 0, w, 1,
						   (void *) (pal_bitmap + (m_nHeight - i - 1) * m_nWidth), bmi, 
							DIB_RGB_COLORS, SRCCOPY);
		}
else
	StretchDIBits (m_pDC->m_hDC, 0, 0, m_nWidth * 8, m_nHeight * 8, 0, 0, m_nWidth, m_nHeight,
					   (void *) pal_bitmap, bmi, DIB_RGB_COLORS, SRCCOPY);
free (pal_bitmap);
EndPaint ();
}

                        /*--------------------------*/

bool CPaletteWnd::BeginPaint ()
{
if (!IsWindow (m_hWnd))
	return false;
if (m_pDC)
	return false;
if (!(m_pDC = GetDC ()))
	 return false;
m_pOldPal = m_pDC->SelectPalette (thePalette, FALSE);
m_pDC->RealizePalette ();
return true;
}

                        /*--------------------------*/

void CPaletteWnd::EndPaint ()
{
if (m_pDC) {
	if (m_pOldPal) {
		m_pDC->SelectPalette (m_pOldPal, FALSE);
		m_pOldPal = NULL;
		}
	ReleaseDC (m_pDC);
	m_pDC = NULL;
	}
Update ();
}

                        /*--------------------------*/
#if 0
void CPaletteWnd::OnLButtonDown (UINT nFlags, CPoint point)
{
m_pParentWnd->SendMessage (WM_LBUTTONDOWN, (WPARAM) nFlags, (LPARAM) point.x + (((LPARAM) point.y) << 16));
}

                        /*--------------------------*/

void CPaletteWnd::OnRButtonDown (UINT nFlags, CPoint point)
{
m_pParentWnd->SendMessage (WM_RBUTTONDOWN, (WPARAM) nFlags, (LPARAM) point.x + (((LPARAM) point.y) << 16));
}

                        /*--------------------------*/

void CPaletteWnd::OnLButtonUp (UINT nFlags, CPoint point)
{
m_pParentWnd->SendMessage (WM_LBUTTONUP, (WPARAM) nFlags, (LPARAM) point.x + (((LPARAM) point.y) << 16));
}

                        /*--------------------------*/

void CPaletteWnd::OnRButtonUp (UINT nFlags, CPoint point)
{
m_pParentWnd->SendMessage (WM_RBUTTONUP, (WPARAM) nFlags, (LPARAM) point.x + (((LPARAM) point.y) << 16));
}
#endif
//************************************************************************
// DIALOG - CTextureEdit (constructor)
//************************************************************************

CTextureEdit::CTextureEdit (CWnd *pParent)
	: CDialog (IDD_EDITTEXTURE, pParent) 
{
*m_szColors = '\0';
m_pDC = NULL;
m_pPaintWnd = NULL;
m_pOldPal = NULL;
m_lBtnDown =
m_rBtnDown = false;
m_bitmap = 
m_backupBM = NULL;
m_tga =
m_backupTGA = NULL;
strcpy (m_szDefExt, ".bmp");
}

                        /*--------------------------*/

CTextureEdit::~CTextureEdit ()
{
if (m_bitmap) {
	delete m_bitmap;
	m_bitmap = NULL;
	}
if (m_backupBM) {
	delete m_backupBM;
	m_backupBM = NULL;
	}
if (m_tga) {
	delete m_tga;
	m_tga = NULL;
	}
if (m_backupTGA) {
	delete m_backupTGA;
	m_backupTGA = NULL;
	}
}

                        /*--------------------------*/

BOOL CTextureEdit::OnInitDialog ()
{
CDialog::OnInitDialog ();

	CWnd *pWnd;
	CRect	rc;

pWnd = GetDlgItem (IDC_TEXEDIT_TEXTURE);
pWnd->GetClientRect (rc);
m_textureWnd.Create (NULL, NULL, WS_CHILD | WS_VISIBLE, rc, pWnd, 0);
pWnd = GetDlgItem (IDC_TEXEDIT_PALETTE);
pWnd->GetClientRect (rc);
m_paletteWnd.Create (pWnd, 32, 8);
pWnd = GetDlgItem (IDC_TEXEDIT_LAYERS);
pWnd->GetClientRect (rc);
m_layerWnd.Create (NULL, NULL, WS_CHILD | WS_VISIBLE, rc, pWnd, 0);
// set cursor styles for bitmap windows
HINSTANCE hInst = AfxGetApp()->m_hInstance;
SetCursor (LoadCursor (hInst, "PENCIL_CURSOR"));
//  PaletteButton->SetCursor(NULL,IDC_CROSS);
m_fgColor = 0; // black
m_bgColor = 1; // white
m_lBtnDown  = FALSE;
m_rBtnDown = FALSE;
m_bModified = FALSE;
m_iTexture = theApp.GetMine ()->CurrSide ()->nBaseTex;
if (m_iTexture >= MAX_D2_TEXTURES)
	m_iTexture = 0;
m_pTx = pTextures [file_type] + m_iTexture;
if (!(m_pTx->m_pDataBM && m_pTx->m_bValid)) {
	DEBUGMSG (" Texture tool: Invalid texture");
	EndDialog (IDCANCEL);
	}
m_nWidth = m_pTx->m_width;
m_nHeight = m_pTx->m_height;
m_nSize = m_pTx->m_size;
m_bitmap = new UINT8 [2048 * 2048];
if (!m_bitmap) {
	DEBUGMSG (" Texture tool: Not enough memory for texture editing");
	EndDialog (IDCANCEL);
	}
memcpy (m_bitmap, m_pTx->m_pDataBM, m_nSize);
m_tga = new tRGBA [2048 * 2048];
if (!m_tga) {
	DEBUGMSG (" Texture tool: Not enough memory for TGA texture editing");
	}
else if (m_nFormat = m_pTx->m_nFormat)
	memcpy (m_tga, m_pTx->m_pDataTGA, m_nSize * sizeof (tRGBA));
m_backupBM = new UINT8 [2048 * 2048];
m_backupTGA = new tRGBA [2048 * 2048];
if (!(m_backupBM && m_backupTGA))
	DEBUGMSG (" Texture tool: Not enough memory for undo function");
Backup ();
Refresh ();
return TRUE;
}

                        /*--------------------------*/

void CTextureEdit::DoDataExchange (CDataExchange *pDX)
{
DDX_Text (pDX, IDC_TEXEDIT_COLORS, m_szColors, sizeof (m_szColors));
}

                        /*--------------------------*/

void CTextureEdit::Backup (void)
{
if (m_backupBM) {
	memcpy (m_backupBM, m_bitmap, m_nSize);
	if (m_backupTGA)
		memcpy (m_backupTGA, m_tga, m_nSize * sizeof (tRGBA));
	m_nOldWidth = m_nWidth;
	m_nOldHeight = m_nHeight;
	m_nOldSize = m_nSize;
	m_nOldFormat = m_nFormat;
	}
}

                        /*--------------------------*/

bool CTextureEdit::PtInRect (CRect& rc, CPoint& pt)
{
return (pt.x >= rc.left) && (pt.x < rc.right) &&
 		 (pt.y >= rc.top) && (pt.y < rc.bottom);
}

                        /*--------------------------*/

void CTextureEdit::OnButtonDown (UINT nFlags, CPoint point, int& color)
{
	CRect	rcEdit, rcPal;

GetClientRect (&m_textureWnd, rcEdit);
GetClientRect (&m_paletteWnd, rcPal);
if (PtInRect (rcEdit, point)) {
	Backup ();
	ColorPoint (nFlags, point, color);
  }
else if (PtInRect (rcPal, point)) {
	point.x -= rcPal.left;
	point.y -= rcPal.top;
	if (m_paletteWnd.SelectColor (point, color))
		DrawLayers ();
	}
}

                        /*--------------------------*/

void CTextureEdit::OnOK ()
{
if ((m_pTx->m_width != m_nWidth) || (m_pTx->m_height != m_nHeight) || (m_pTx->m_nFormat != m_nFormat)) {
	UINT8 *pDataBM = new UINT8 [m_nWidth * m_nHeight];
	tRGBA  *pDataTGA = m_nFormat ? new tRGBA [m_nWidth * m_nHeight] : NULL;

	if (!pDataBM) {
		DEBUGMSG (" Texture tool: Not enough memory for the new texture");
		EndDialog (IDCANCEL);
		}
	delete m_pTx->m_pDataBM;
	if (m_pTx->m_pDataTGA)
		delete m_pTx->m_pDataTGA;
	m_pTx->m_pDataTGA = pDataTGA;
	m_pTx->m_pDataBM = pDataBM;
	m_pTx->m_width = m_nWidth;
	m_pTx->m_height = m_nHeight;
	m_pTx->m_size = m_nSize;
	m_pTx->m_nFormat = (unsigned char) m_nFormat;
	}
memcpy (m_pTx->m_pDataBM, m_bitmap, m_pTx->m_size);
if (m_pTx->m_nFormat)
	memcpy (m_pTx->m_pDataTGA, m_tga, m_pTx->m_size * sizeof (tRGBA));
m_pTx->m_bModified = m_bModified;
CDialog::OnOK ();
}

                        /*--------------------------*/

void CTextureEdit::OnLButtonDown (UINT nFlags, CPoint point)
{
m_lBtnDown = TRUE;
OnButtonDown (nFlags, point, m_fgColor);
}

                        /*--------------------------*/

void CTextureEdit::OnRButtonDown (UINT nFlags, CPoint point)
{
m_rBtnDown = TRUE;
OnButtonDown (nFlags, point, m_bgColor);
}

                        /*--------------------------*/

void CTextureEdit::OnLButtonUp (UINT nFlags, CPoint point)
{
m_lBtnDown = FALSE;
}

                        /*--------------------------*/

void CTextureEdit::OnRButtonUp (UINT nFlags, CPoint point)
{
m_rBtnDown = FALSE;
}

                        /*--------------------------*/

void CTextureEdit::OnMouseMove (UINT nFlags, CPoint point)
{
if (m_lBtnDown)
	ColorPoint (nFlags, point, m_fgColor);
else if (m_rBtnDown)
	ColorPoint (nFlags, point, m_bgColor);
}


                        /*--------------------------*/

bool CTextureEdit::BeginPaint (CWnd *pWnd)
{
if (m_pDC)
	return false;
if (!(m_pDC = pWnd->GetDC ()))
	 return false;
m_pPaintWnd = pWnd;
m_pOldPal = m_pDC->SelectPalette (thePalette, FALSE);
m_pDC->RealizePalette ();
return true;
}

                        /*--------------------------*/

void CTextureEdit::EndPaint ()
{
if (m_pPaintWnd) {
	if (m_pDC) {
		if (m_pOldPal) {
			m_pDC->SelectPalette (m_pOldPal, FALSE);
			m_pOldPal = NULL;
			}
		m_pPaintWnd->ReleaseDC (m_pDC);
		m_pDC = NULL;
		}
	Update (m_pPaintWnd);
	m_pPaintWnd = NULL;
	}
}

                        /*--------------------------*/

void CTextureEdit::GetClientRect (CWnd *pWnd, CRect& rc)
{
	CRect	rcc;
	int	dx, dy;

pWnd->GetClientRect (&rcc);
pWnd->GetWindowRect (rc);
dx = rc.Width () - rcc.Width ();
dy = rc.Height () - rcc.Height ();
ScreenToClient (rc);
rc.DeflateRect (dx / 2, dy / 2);
}

//************************************************************************
// CTextureEdit - ColorPoint
//
// Action - Uses coordinates to determine which pixel mouse cursor is
// over.  If it is over the palette, the palette box will be updated with
// the new color.  If it is over the texture, the texture will be updated
// with the color.
// If control key is held down, color is defined by bitmap instead.
//************************************************************************

void CTextureEdit::ColorPoint (UINT nFlags, CPoint& point, int& color) 
{
	CRect		rcEdit;

GetClientRect (&m_textureWnd, rcEdit);

if (m_nFormat) {
	m_lBtnDown = m_rBtnDown = false;
	ErrorMsg ("Cannot edit TGA images.");
	}
else if (PtInRect (rcEdit, point)) {
	int x,y;
	m_bModified = TRUE;  // mark this as m_bModified
//	x = ((point.x - rcEdit.left) >> 2) & 63;
//	y = ((point.y - rcEdit.top) >> 2) & 63;
	x = (int) ((double) (point.x - rcEdit.left) * (64.0 / rcEdit.Width ()));
	y = (int) ((double) (point.y - rcEdit.top) * (64.0 / rcEdit.Height ()));
	if (nFlags & MK_CONTROL) {
		color = m_bitmap [m_nWidth * (m_nHeight - 1 - y) + x];
		DrawLayers ();
		}
	else if (BeginPaint (&m_textureWnd)) {
		m_bitmap [m_nWidth * (m_nHeight - 1 - y) + x] = (UINT8) color;
		SetTexturePixel (x, y);
		EndPaint ();
		}
	}
}

//************************************************************************
// TTextureDialog - WMDrawItem
//************************************************************************

void CTextureEdit::OnPaint () //EvDrawItem(UINT, DRAWITEMSTRUCT &) 
{
CDialog::OnPaint ();
Refresh ();
}

//************************************************************************
// TTextureDialog - Open Message
//************************************************************************

inline int Sqr (int i)
{
return i * i;
}

//************************************************************************
// 
//************************************************************************

inline int ColorDelta (RGBQUAD *bmPal, PALETTEENTRY *sysPal, int j)
{
sysPal += j;
return 
	Sqr (bmPal->rgbBlue - sysPal->peBlue) + 
	Sqr (bmPal->rgbGreen - sysPal->peGreen) + 
	Sqr (bmPal->rgbRed - sysPal->peRed);
}

//************************************************************************
// 
//************************************************************************

bool CTextureEdit::LoadTGA (FILE *file)
{
	tTgaHeader	tgaHeader;
	char			imgIdent [255];
	int			h, i, j, s;
	tBGRA			bgra;

if (!m_tga) {
	DEBUGMSG (" Texture tool: Not enough memory for TGA texture editing");
	return false;
	}
fread (&tgaHeader, sizeof (tgaHeader), 1, file);
if ((tgaHeader.width * tgaHeader.height > 2048 * 2048)) {
	ErrorMsg ("Image too large.");
	return false;
	}
m_nWidth = tgaHeader.width;
m_nHeight = tgaHeader.height;
m_nSize = tgaHeader.width * tgaHeader.height;
if (tgaHeader.identSize)
	fread (imgIdent, tgaHeader.identSize, 1, file);
#if 1
h = 0; //m_nWidth * (m_nHeight - 1);
s = (tgaHeader.bits == 32) ? 4 : 3;
bgra.a = 255;
h = m_nWidth * (m_nHeight - 1);
for (i = m_nHeight; i; i--) {
	for (j = m_nWidth; j; j--, h++) {
		fread (&bgra, s, 1, file);
		m_tga [h].a = bgra.a;
		m_tga [h].r = bgra.r;
		m_tga [h].g = bgra.g;
		m_tga [h].b = bgra.b;
		}
	h -= 2 * m_nWidth;
	}
#else
fread (m_tga, m_nSize * sizeof (tRGBA), 1, file);
#endif
m_bModified = TRUE;
if (TGA2Bitmap (m_tga, m_bitmap, (int) tgaHeader.width, (int) tgaHeader.height)) {
	m_nFormat = 1;
	return true;
	}
return false;
}

//************************************************************************
// 
//************************************************************************

bool CTextureEdit::LoadBitmap (FILE *file)
{
	RGBQUAD *palette=NULL;
	PALETTEENTRY *sysPal=NULL;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	UINT8 color_map[256];
	bool bFuncRes = false;
	int x, y, width, palette_size;

   palette = (RGBQUAD *) malloc(256*sizeof(RGBQUAD));
	if (!palette) {
	  ErrorMsg("Not enough memory for palette.");
	  goto errorExit;
	}

	sysPal = (PALETTEENTRY *) malloc(256*sizeof(PALETTEENTRY));
	if (!sysPal) {
	  ErrorMsg("Not enough memory for palette.");
	  goto errorExit;
	}

	// read the header information
	fread(&bmfh,sizeof(bmfh),1,file);
	fread(&bmih,sizeof(bmih),1,file);

	// handle exceptions
	if (bmih.biClrUsed==0)  
		bmih.biClrUsed = 256;
	if (bmih.biHeight < 0) 
		bmih.biHeight *= -1;

	// make sure it is a bitmap file
	if (bmfh.bfType != 'B' + (((UINT16)'M')<<8) ) {
	  ErrorMsg("This is not a bitmap file.");
	  goto errorExit;
	}

	// make sure it is a 256 or 16 color bitmap
	if (bmih.biBitCount != 8 && bmih.biBitCount != 4) {
	  ErrorMsg("DLE-XP only reads 16 or 256 color bitmap files.\n\n"
		   "Hint: Load this image into a paint program\n"
		   "then save it as a 16 or 256 color *.bmp file.");
	  goto errorExit;
	}

	// make sure the data is not compressed
	if (bmih.biCompression != BI_RGB) {
	  ErrorMsg("Cannot read compressed bitmap files.\n\n"
		   "Hint: Try loading this image into a paint program\n"
		   "then save it as a 256 color *.bmp file with the\n"
		   "compression option off.");
	  goto errorExit;
	}

	// read palette
	palette_size = min((int)bmih.biClrUsed,256);
	if (palette_size == 0) {
	  palette_size = 1 << bmih.biBitCount;
	}
	fread(palette,sizeof(RGBQUAD),palette_size,file);

	// read the logical palette entries
	thePalette->GetPaletteEntries (0, 256, sysPal);

	// check color palette
	int i;
	for (i=0;i<palette_size;i++) {
	  color_map[i] = i;
	  if (palette[i].rgbRed != sysPal[i].peRed ||
			palette[i].rgbGreen != sysPal[i].peGreen ||
			palette[i].rgbBlue != sysPal[i].peBlue) {
			break;
	  }
	}
	if (i!=palette_size) {
		if (!bExpertMode)
			ErrorMsg("The palette of this bitmap file is not exactly the\n"
					  "the same as the Descent palette. Therefore, some color\n"
					  "changes may occur.\n\n"
					  "Hint: If you want the palettes to match, then save one of\n"
					  "the Descent textures to a file an use it as a starting point.\n"
					  "If you plan to use transparencies, then you may want to start\n"
					  "with the texture called 'empty'.");
		for (i=0;i<palette_size;i++) {
			unsigned int closest_index = i;
			if ((palette [i].rgbRed != sysPal [i].peRed) ||
				 (palette [i].rgbGreen != sysPal [i].peGreen) ||
				 (palette [i].rgbBlue != sysPal [i].peBlue)) {
				unsigned int closest_delta = 0x7fffffff;
				int j;
				for (j = 0; (j < 255) && closest_delta; j++) {
					unsigned int delta = ColorDelta (palette + i, sysPal, j);
					if (delta < closest_delta) {
						closest_index = j;
						closest_delta = delta;
						}
					}
				}
			color_map[i] = closest_index;
			}
		}

	int x0,x1,y0,y1;
	// if size is not 64 x 64, ask if they want to "size to fit"
	if ((bmih.biWidth != m_nWidth) || (bmih.biHeight != m_nHeight)) {
		sprintf(message,"The bitmap being loaded is a %d x %d image.\n"
				  "Do you want the image to be sized to fit the\n"
				  "the current %d x %d texture size?\n\n"
			     "(press no to see another option)",
			     (int) bmih.biWidth, (int) bmih.biHeight,
				  (int) m_nWidth, (int) m_nHeight);
		switch (Query2Msg (message, MB_YESNOCANCEL)) {
			case IDYES:
				Backup();
				x0 = 0;
				y0 = 0;
				x1 = (int)bmih.biWidth+1;
				y1 = (int)bmih.biHeight+1;
				break;

			case IDNO:
				Backup();
				if (Query2Msg("Would you like to center/tile the image?", MB_YESNO) == IDYES) {
					x0 = (int)(bmih.biWidth - m_nWidth)/2;
					y0 = (int)(bmih.biHeight - m_nHeight)/2;
					x1 = x0+m_nWidth;
					y1 = y0+m_nHeight;
					}
				else if ((bmih.biWidth > 1024) || (bmih.biHeight > 1024)) 
					goto errorExit;
				else {
					x0 = 0;
					y0 = 0;
					x1 = m_nWidth = (UINT16) bmih.biWidth;
					y1 = m_nHeight = (UINT16) bmih.biHeight;
					}
				break;

			default:
				goto errorExit;
				}
			}
	else {
	  x0 = 0;
	  y0 = 0;
	  x1 = m_nWidth;
	  y1 = m_nHeight;
	}

	// save bitmap for undo command
   m_nSize = m_nWidth * m_nHeight;

	// read data into bitmap
	m_bModified = TRUE;  // mark this as m_bModified
	width = (((int)(bmih.biWidth*bmih.biBitCount + 31)>>3)) & ~3;
	double mx,my;
	mx = (x1 - x0) / (double) m_nWidth;
	my = (y1 - y0) / (double) m_nHeight;
	for (y = 0; y < m_nHeight; y++) {
		for (x = 0; x < m_nWidth; x++) {
			int u = (int) (mx * x + x0);
			int v = (int) (my * y + y0);
			u %= (int)bmih.biWidth;          //  -width to width
			if (u<0) 
				u+= (int)bmih.biWidth;  //       0 to width
			v %= (int)bmih.biHeight;         // -height to height
			if (v<0) 
			v+= (int)bmih.biHeight; //       0 to height
			
			UINT8 byte;

			if (bmih.biBitCount == 4) {
				long offset = (long)v*(long)width + (long)u / 2;
				fseek(file,(long)bmfh.bfOffBits + offset,SEEK_SET);
				fread(&byte,1,1,file);
				if (!(u&1))
					byte >>=4;
				byte &= 0x0f;
				m_bitmap [y*m_nWidth+x] = color_map[byte];
				}
			else {
				fseek(file,(long)bmfh.bfOffBits + (long)v*(long)width + (long)u,SEEK_SET);
				fread(&byte,1,1,file);
				m_bitmap [y*m_nWidth+x] = color_map[byte];
				}
			}
		}
m_nFormat = 0;
bFuncRes = true;

errorExit:

if (palette) 
  free(palette);
if (sysPal) 
  free(sysPal);
return bFuncRes;
}

//************************************************************************
// 
//************************************************************************

void CTextureEdit::OnLoad () 
{
  OPENFILENAME ofn;
  char szFile[80] = "\0";
  FILE *file=NULL;
  bool bFuncRes;

  strcpy (szFile, "*");
  strcat (szFile, m_szDefExt);
  memset(&ofn, 0, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = m_hWnd;
  ofn.lpstrFilter = "Bitmap Files\0*.bmp\0TGA Files\0*.tga\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = szFile;
  ofn.lpstrDefExt = m_szDefExt;
  ofn.nMaxFile = sizeof(szFile);
  ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

if (GetOpenFileName (&ofn)) {
	if (strchr (ofn.lpstrFile, '.'))
		strncpy (m_szDefExt, strchr (ofn.lpstrFile, '.'), 4);
	file = fopen(ofn.lpstrFile,"rb");
	if (!file) {
		ErrorMsg("Could not open texture file.");
		goto errorExit;
		}
	Backup ();
	if (!strcmp (strlwr (m_szDefExt), ".bmp"))
		bFuncRes = LoadBitmap (file);
	else
		bFuncRes = LoadTGA (file);
	if (bFuncRes)
		Refresh ();
	else
		OnUndo ();
	}

errorExit:

if (file) 
	fclose (file);
}

//************************************************************************
// TTextureDialog - Save Message
//************************************************************************

void CTextureEdit::SaveBitmap (FILE *file)
{
BITMAPFILEHEADER bmfh;

bmfh.bfType = 'B' + ('M'<<8);
bmfh.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+256*4+m_nSize;
bmfh.bfReserved1 = 0;
bmfh.bfReserved2 = 0;
bmfh.bfOffBits   = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+256*4;

// define the bitmap header
BITMAPINFO *bmi;
bmi = MakeBitmap();
bmi->bmiHeader.biWidth = m_nWidth;
bmi->bmiHeader.biHeight = m_nHeight;

// write the headers
fwrite(&bmfh,sizeof(BITMAPFILEHEADER),1,file);
fwrite(&bmi->bmiHeader,sizeof(BITMAPINFOHEADER),1,file);

// write palette
fwrite(bmi->bmiColors,sizeof(RGBQUAD),256,file);

// save bitmap data
fwrite(m_bitmap,m_nSize,1,file);
}

//************************************************************************

void CTextureEdit::SaveTGA (FILE *file)
{
	tTgaHeader	h;
	int			i, j;
	tBGRA			c;
	tRGBA			*pc;

memset (&h, 0, sizeof (h));
h.imageType = 2;
h.width = m_nWidth;
h.height = m_nHeight;
h.bits = 32;
fwrite (&h, sizeof(h), 1, file);
pc = m_tga + m_nWidth * (m_nHeight - 1);
for (i = m_nHeight; i; i--) {
	for (j = m_nWidth; j; j--, pc++) {
		c.r = pc->r;
		c.g = pc->g;
		c.b = pc->b;
		c.a = pc->a;
		fwrite (&c, sizeof (c), 1, file);
		}
	pc -= 2 * m_nWidth;
	}
}

//************************************************************************

void CTextureEdit::OnSave ()
{
OPENFILENAME ofn;
char szFile[80] = "\0";
FILE *file;

memset(&ofn, 0, sizeof(OPENFILENAME));

ofn.lStructSize = sizeof(OPENFILENAME);
ofn.hwndOwner = m_hWnd;
ofn.lpstrFilter = m_nFormat ? "Truevision Targa\0*.tga\0" : "256 color Bitmap Files\0*.bmp\0";
ofn.nFilterIndex = 1;
ofn.lpstrFile= szFile;
ofn.lpstrDefExt = "bmp";
ofn.nMaxFile = sizeof(szFile);
ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
if (GetSaveFileName(&ofn)) {
	strlwr(ofn.lpstrFile);
	file = fopen(ofn.lpstrFile,"wb");
	if (!file) {
		ErrorMsg("Could not create bitmap file.");
		return;
		}
	if (m_nFormat)
		SaveTGA (file);
	else
		SaveBitmap (file);
	// define the file header
	// close file
	fclose(file);
	}
}

//************************************************************************
// TTextureDialog -  Undo Message
//************************************************************************

void CTextureEdit::OnUndo ()
{
if (m_backupBM) {
	memcpy (m_bitmap, m_backupBM, m_nOldSize);
	if (m_backupTGA)
		memcpy (m_tga, m_backupTGA, m_nOldSize * sizeof (tRGBA));
	m_nWidth = m_nOldWidth;
	m_nHeight = m_nOldHeight;
	m_nSize = m_nOldSize;
	m_nFormat = m_nOldFormat;
	Refresh ();
	}
}

void CTextureEdit::Update (CWnd *pWnd) 
{
pWnd->InvalidateRect (NULL);
pWnd->UpdateWindow ();
}

//************************************************************************
// TTextureDialog -  Default Message
//************************************************************************

void CTextureEdit::OnDefault (void) 
{
if (QueryMsg("Are you sure you want to restore this texture\n"
				 "back to its original texture\n") == IDYES) {
	Backup ();
	m_pTx->m_bModified = m_bModified = FALSE;
	m_pTx->Read (m_iTexture);
	memcpy (m_bitmap, m_pTx->m_pDataBM, m_pTx->m_size);
	m_nWidth = m_pTx->m_width;
	m_nHeight = m_pTx->m_height;
	m_nSize = m_pTx->m_size;
	m_nFormat = 0;
	Refresh ();
	}
}

//************************************************************************
// TTextureDialog - DrawTexture
//************************************************************************

void CTextureEdit::DrawTexture (void) 
{
if (!BeginPaint (&m_textureWnd))
	return;
BITMAPINFO *bmi = MakeBitmap ();
bmi->bmiHeader.biWidth = m_nWidth;
bmi->bmiHeader.biHeight = m_nWidth;
CRect	rc;
m_textureWnd.GetClientRect (&rc);
StretchDIBits (m_pDC->m_hDC, 0, 0, rc.right, rc.bottom, 0, 0, m_nWidth, m_nWidth,
					(void *)m_bitmap, bmi, DIB_RGB_COLORS,SRCCOPY);
EndPaint ();
}

//************************************************************************
// TTextureDialog - DrawPalette
//************************************************************************

void CTextureEdit::DrawPalette (void) 
{
m_paletteWnd.DrawPalette ();
}

//************************************************************************
// TTextureDialog - DrawLayers
//************************************************************************

void CTextureEdit::DrawLayers () 
{
if (!BeginPaint (&m_layerWnd))
	return;

CRect		rc;
m_layerWnd.GetClientRect (&rc);
rc.DeflateRect (10, 10);
rc.right -= 10;
rc.bottom -= 10;
m_pDC->FillSolidRect (&rc, PALETTEINDEX (m_bgColor));
rc.OffsetRect (10, 10);
m_pDC->FillSolidRect (&rc, PALETTEINDEX (m_fgColor));
EndPaint ();

// set message
char fg_color[30];
char bg_color[30];
switch(m_fgColor) {
	case 255: 
		strcpy(fg_color,"transparent"); 
		break;
	case 254: 
		strcpy(fg_color,"see thru"); 
		break;
	default : 
		sprintf(fg_color,"color %d",m_fgColor); 
		break;
	}
switch(m_bgColor) {
	case 255: 
		strcpy(bg_color,"transparent"); 
		break;
	case 254: 
		strcpy(bg_color,"see thru"); 
		break;
	default : 
		sprintf(bg_color,"color %d",m_bgColor); 
		break;
	}
sprintf(m_szColors, "foreground = %s, background = %s.", fg_color, bg_color);
UpdateData (FALSE);
}

//************************************************************************
// TTextureDialog - Refresh
//************************************************************************

void CTextureEdit::Refresh (void) 
{
DrawTexture ();
DrawPalette ();
DrawLayers ();
}


//************************************************************************
//************************************************************************

void CTextureEdit::SetTexturePixel (int x, int y) 
{
	CRect		rc;
	int		cx, cy;
	double	xs, ys;
	int		color = PALETTEINDEX (m_bitmap [(63 - y) * 64 + x]);

m_textureWnd.GetClientRect (&rc);
cx = rc.Width ();
cy = rc.Height ();
xs = (double) cx / 64.0;
ys = (double) cy / 64.0;
x = rc.left + (int) ((double) x * xs);
y = rc.top + (int) ((double) y * ys);
int dx, dy;
xs /= 4.0;
ys /= 4.0;
for (dy = 0; dy < 4; dy++)
	for (dx = 0; dx < 4; dx++)
		m_pDC->SetPixel (x + (int) ((double) dx * xs),
							  y + (int) ((double) dy * ys),
							  color);
//		m_pDC->SetPixel((x<<2)+dx+rc.left,(y<<2)+dy+rc.top,
//							 PALETTEINDEX(m_bitmap [(63-y)*64+x]));
}

//************************************************************************
//************************************************************************

void CTextureEdit::SetPalettePixel (int x, int y) 
{
	CRect	rc;

m_paletteWnd.GetClientRect (&rc);
int dx,dy;
for (dy=0;dy<8;dy++)
	for (dx=0;dx<8;dx++)
		m_pDC->SetPixel((x<<3)+dx+rc.left,(y<<3)+dy+rc.top,
							 PALETTEINDEX(y*32+x));
}

//************************************************************************
