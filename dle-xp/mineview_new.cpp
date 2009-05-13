// dlcView.cpp : implementation of the CMineView class
//

#include "stdafx.h"
#include "dlc.h"

#include "dlcDoc.h"
#include "mineview.h"

#include "palette.h"
#include "textures.h"
#include "global.h"
#include "render.h"
#include "io.h"

#include <math.h>
#include <time.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE [] = __FILE__;
#endif

static UINT8 bmBuf [64*64];

static LPSTR szMouseStates [] = {
	"IDLE",
	"BUTTON DOWN",
	"PAN",
	"ROTATE",
	"ZOOM",
	"ZOOM IN",
	"ZOOM OUT",
	"INIT DRAG",
	"DRAG",
	"RUBBERBAND"
	};


#define BETWEEN(a,b,c) ((a<c) ? (a<b)&&(b<c) : (c<b)&&(b<a))
#define  RUBBER_BORDER     1

#define f2fl(f)	(((float) f) / (float) 65536.0)
#define UV_FACTOR ((double)640.0/(double)0x10000L)

#define OGL_MAPPED 1

/////////////////////////////////////////////////////////////////////////////
// CMineView

IMPLEMENT_DYNCREATE(CMineView, CView)

BEGIN_MESSAGE_MAP(CMineView, CView)
	//{{AFX_MSG_MAP(CMineView)
	ON_WM_TIMER ()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_COMMAND(ID_SEL_PREVTAB, OnSelectPrevTab)
	ON_COMMAND(ID_SEL_NEXTTAB, OnSelectNextTab)
END_MESSAGE_MAP()

BOOL CMineView::PreCreateWindow(CREATESTRUCT& cs)
{
return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMineView construction/destruction

CMineView::CMineView()
{
static LPSTR nIdCursors [eMouseStateCount] = {
	IDC_ARROW,
	IDC_ARROW,
	MAKEINTRESOURCE (IDC_CURSOR_PAN),
	MAKEINTRESOURCE (IDC_CURSOR_ROTATE),
	MAKEINTRESOURCE (IDC_CURSOR_ZOOM),
	MAKEINTRESOURCE (IDC_CURSOR_ZOOMIN),
	MAKEINTRESOURCE (IDC_CURSOR_ZOOMOUT),
	MAKEINTRESOURCE (IDC_CURSOR_DRAG),
	MAKEINTRESOURCE (IDC_CURSOR_DRAG),
	IDC_ARROW
	};

m_penCyan    = new CPen(PS_SOLID, 1, RGB(255,196,  0)); //ok, that's gold, but cyan doesn't work - palette problem?
//m_penCyan    = new CPen(PS_SOLID, 1, RGB(  0,255,255)); //doesn't work (rather green) - palette problem?
m_penRed     = new CPen(PS_SOLID, 1, RGB(255,  0,  0));
m_penMedRed  = new CPen(PS_SOLID, 1, RGB(255,  0,  0));
m_penGray    = new CPen(PS_SOLID, 1, RGB(128,128,128));
m_penLtGray  = new CPen(PS_SOLID, 1, RGB(160,160,160));
m_penGreen   = new CPen(PS_SOLID, 1, RGB(  0,255,  0));
m_penDkGreen = new CPen(PS_SOLID, 1, RGB(  0,128,  0));
m_penDkCyan  = new CPen(PS_SOLID, 1, RGB(  0,128,128));
m_penBlue	 = new CPen(PS_SOLID, 1, RGB(  0,  0,255));
m_penMedBlue = new CPen(PS_SOLID, 1, RGB(  0,  0,255));
m_penLtBlue	 = new CPen(PS_SOLID, 1, RGB(  0,255,255));
m_penYellow  = new CPen(PS_SOLID, 1, RGB(255,196,  0));
m_penOrange  = new CPen(PS_SOLID, 1, RGB(255,128,  0));
m_penMagenta = new CPen(PS_SOLID, 1, RGB(255,  0,255));
m_penHiCyan    = new CPen(PS_SOLID, 2, RGB(255,196,  0)); //ok, that's gold, but cyan doesn't work - palette problem?
//m_penHiCyan    = new CPen(PS_SOLID, 2, RGB(  0,255,255)); //doesn't work (rather green) - palette problem?
m_penHiRed     = new CPen(PS_SOLID, 2, RGB(255,  0,  0));
m_penHiGray    = new CPen(PS_SOLID, 2, RGB(128,128,128));
m_penHiLtGray  = new CPen(PS_SOLID, 2, RGB(160,160,160));
m_penHiGreen   = new CPen(PS_SOLID, 2, RGB(  0,255,  0));
m_penHiDkGreen = new CPen(PS_SOLID, 2, RGB(  0,128,  0));
m_penHiDkCyan  = new CPen(PS_SOLID, 2, RGB(  0,128,128));
m_penHiBlue	 = new CPen(PS_SOLID, 2, RGB(  0,  0,255));
m_penHiYellow  = new CPen(PS_SOLID, 2, RGB(255,196,  0));
m_penHiOrange  = new CPen(PS_SOLID, 2, RGB(255,128,  0));
m_penHiMagenta = new CPen(PS_SOLID, 2, RGB(255,  0,255));
for (int i = eMouseStateIdle; i < eMouseStateCount; i++)
	m_hCursors [i] = LoadCursor ((nIdCursors [i] == IDC_ARROW) ? NULL : theApp.m_hInstance, nIdCursors [i]);
m_viewObjectFlags = eViewObjectsAll;
m_viewMineFlags = eViewMineLights | eViewMineWalls | eViewMineSpecial;
m_viewOption = eViewTextureMapped;
m_nDelayRefresh = 0;
#if OGL_RENDERING
m_glRC = NULL;
m_glDC = NULL;
#endif
Reset ();
}


void CMineView::Reset ()
{
m_viewWidth = m_viewHeight = m_viewDepth = 0;	// force OnDraw to initialize these
if (GetMine ())
	m_mine->SetSplineActive (false);
m_bUpdate = true;
m_mouseState  = 
m_lastMouseState = eMouseStateIdle;
m_selectMode = eSelectSide;
m_lastSegment = 0;

m_x0 = 0;
m_y0 = 0;
m_z0 = 0;
m_spinx = M_PI/4.f;
m_spiny = M_PI/4.f;
m_spinz = 0.0;
m_movex = 0.0f;
m_movey = 0.0f;
m_movez = 0.0f;
m_sizex = 1.0f;
m_sizey = 1.0f;
m_sizez = 1.0f;
m_DepthPerception = 10000.0f;

// calculate transformation m_matrix based on move, size, and spin
m_matrix.Set(m_movex, m_movey, m_movez,
				 m_sizex, m_sizey, m_sizez,
				 m_spinx, m_spiny, m_spinz);
m_lightTimer = -1;
m_nFrameRate = 100;
m_bShowLightSource = 0;
}


CMineView::~CMineView()
{
#if OGL_RENDERING
GLKillWindow ();
#endif
delete m_penCyan;
delete m_penRed;
delete m_penMedRed;
delete m_penGray;
delete m_penGreen;
delete m_penDkGreen;
delete m_penDkCyan;
delete m_penBlue;
delete m_penMedBlue;
delete m_penLtBlue;
delete m_penLtGray;
delete m_penYellow;
delete m_penOrange;
delete m_penMagenta;
delete m_penHiCyan;
delete m_penHiRed;
delete m_penHiGray;
delete m_penHiGreen;
delete m_penHiDkGreen;
delete m_penHiDkCyan;
delete m_penHiLtGray;
delete m_penHiYellow;
delete m_penHiOrange;
delete m_penHiMagenta;
}

void CMineView::OnDestroy ()
{
if (m_lightTimer >= 0) {
	KillTimer (m_lightTimer);
	m_lightTimer = -1;
	}
CView::OnDestroy ();
}

/////////////////////////////////////////////////////////////////////////////
// CMineView drawing

void CMineView::OnDraw(CDC* pViewDC)
{
#if OGL_RENDERING
SetViewPoints ();
for (;;) {
	GLRenderScene ();
	if (SwapBuffers (m_glDC->m_hDC))
		break;
	break;
	int err = GetLastError ();
	GLKillWindow ();
	}
#else
	CDlcDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;
	if (!pDoc->m_mine) return;
	bool bPartial = false;

	// Initialize/Reinitialize the View's device context
	// and other handy variables
	if (DrawRubberBox () || DrawDragPos ()) {
		if (m_DIB)
			pViewDC->BitBlt (0,0,m_viewWidth, m_viewHeight, &m_DC, 0, 0, SRCCOPY);
		return;
		}
	InitView (pViewDC);

	if (m_bUpdate) {
		// Clear the View
		ClearView();

		// Calculate m_matrix M based on IM and "move x,y,z"
		//m_matrix.Calculate(m_movex, m_movey, m_movez);

		// Set view m_matrix misc. information
		//m_matrix.SetViewInfo(m_DepthPerception, m_viewWidth, m_viewHeight);
		SetViewPoints ();
		// Transform points
//		UINT32 i;
//		for (i=0;i<mine->VertCount ();i++)
//			m_matrix.SetPoint(mine->Vertices (i), m_viewPoints [i]);

		// make a local copy the mine's selection
		m_Current = m_mine->Current ();

		// draw the level
		switch(m_viewOption)		{
			case eViewAllLines:
				DrawWireFrame(m_mine, false);
				break;

			case eViewHideLines:
				DrawWireFrame(m_mine, false); // temporary
				break;

			case eViewNearbyCubeLines:
				DrawWireFrame(m_mine, false); // temporary
				break;

			case eViewPartialLines:
				DrawWireFrame(m_mine, bPartial = true);
				break;

			case eViewTextureMapped:
				DrawTextureMappedCubes(m_mine);
				break;
			}
		}
/*
	if (m_viewFlags & eViewMineWalls)
		DrawWalls (mine);
	if (m_viewFlags & eViewMineLights)
		DrawLights (mine);
	if (m_viewObjects)
		DrawObjects (mine, bPartial);
*/
	DrawHighlight (m_mine);
	// if we are using our own DC, then copy it to the display
	if (m_DIB)
		pViewDC->BitBlt (0,0,m_viewWidth, m_viewHeight, &m_DC, 0, 0, SRCCOPY);
#endif
	m_bUpdate = false;
}

								/*---------------------------*/

afx_msg void CMineView::OnPaint ()
{
	CRect	rc;
	CDC	*pDC;
	PAINTSTRUCT	ps;

if (!GetUpdateRect (rc))
	return;
pDC = BeginPaint (&ps);
DrawRubberBox ();
EndPaint (&ps);
}

                        /*--------------------------*/

void CMineView::AdvanceLightTick (void)
{
	CMine *mine = GetMine ();
	LIGHT_TIMER *plt = light_timers;
	FLICKERING_LIGHT *pfl = mine->FlickeringLights ();
	int i, light_delay;

for (i = mine->FlickerLightCount (); i; i--, pfl++, plt++) {
	light_delay = (pfl->delay * 100 /*+ F0_5*/) / F1_0;
	if (light_delay) {
		if (++plt->ticks == light_delay) {
			plt->ticks = 0;
			plt->impulse = (plt->impulse + 1) % 32;
			}
		}
	else
		plt->impulse = (plt->impulse + 1) % 32;
	}
}

                        /*--------------------------*/
#ifdef _DEBUG
static qqq1 = -1, qqq2 = 0;
#endif

bool CMineView::SetLightStatus (void)
{
	CMine *mine = GetMine ();
	int h, i, j;
	dl_index *pdli = mine->DLIndex ();
	LIGHT_TIMER *plt;
	FLICKERING_LIGHT *pfl = mine->FlickeringLights ();
	LIGHT_STATUS *pls;
	bool bChange = false;
	INT16 source_segnum, source_sidenum, segnum, sidenum;

delta_light *dll = mine->DeltaLights ();
if (!dll)
	return false;
// search delta light index to see if current side has a light
pls = light_status [0];
for (i = mine->SegCount (); i; i--)
	for (j = 0; j < MAX_SIDES_PER_SEGMENT; j++, pls++)
		pls->bWasOn = pls->bIsOn;
for (h = 0; h < mine->GameInfo ().dl_indices.count; h++, pdli++) {
	source_segnum = pdli->segnum;
	source_sidenum = pdli->sidenum;
	j = mine->GetFlickeringLight (source_segnum, source_sidenum);
	if (j < 0)
		continue;	//shouldn't happen here, as there is a delta light value, but you never know ...
	dll = mine->DeltaLights (pdli->index);
	for (i = pdli->count; i; i--, dll++) {
		segnum = dll->segnum;
		sidenum = dll->sidenum;
		if (m_bShowLightSource) {
			if ((segnum != source_segnum) || (sidenum != source_sidenum)) 
				continue;
			if (0 > mine->GetFlickeringLight (segnum, sidenum))
				continue;
			}
		else if (((segnum != source_segnum) || (sidenum != source_sidenum)) &&
			 (0 <= mine->GetFlickeringLight (segnum, sidenum)))
			continue;
#ifdef _DEBUG
		CBRK (segnum < 0 || segnum >= mine->SegCount () || sidenum < 0 || sidenum >= MAX_SIDES_PER_SEGMENT);
		CBRK (segnum == qqq1 && sidenum == qqq2);
#endif
		pls = light_status [segnum] + sidenum;
		plt = light_timers + j;
		pls->bIsOn = (pfl [j].mask & (1 << light_timers [j].impulse)) != 0;
		if (pls->bWasOn != pls->bIsOn)
			bChange = true;
		}
	}
return bChange;
}

                        /*--------------------------*/

void CMineView::OnTimer (UINT nIdEvent)
{
if (enable_delta_shading && (nIdEvent == 3)) {
	AdvanceLightTick ();
	if (SetLightStatus ()) {
		m_bUpdate = TRUE;
		InvalidateRect (NULL, TRUE);
		UpdateWindow ();
		}
	}
else 
	CView::OnTimer (nIdEvent);
}

								/*---------------------------*/

void CMineView::EnableDeltaShading (int bEnable, int nFrameRate, int bShowLightSource)
{
if (enable_delta_shading = bEnable) {
	m_lightTimer = SetTimer (3, (UINT) (m_nFrameRate + 5) / 10, NULL);
	if ((nFrameRate >= 10) && (nFrameRate <= 100))
		m_nFrameRate = nFrameRate;
	if (bShowLightSource != -1)
		m_bShowLightSource = bShowLightSource;
	memset (light_timers, 0, sizeof (light_timers));
	memset (light_status, 0xff, sizeof (light_status));
	}
else if (m_lightTimer >= 0) {
	KillTimer (m_lightTimer);
	m_lightTimer = -1;
	}
}

								/*---------------------------*/

BOOL CMineView::SetWindowPos(const CWnd *pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags)
{
	CRect	rc;

	GetClientRect (rc);

if ((rc.Width () != cx) || (rc.Height () != cy))
	theApp.MainFrame ()->ResetPaneMode ();
return CView::SetWindowPos (pWndInsertAfter, x, y, cx, cy, nFlags);
}



void CMineView::InitViewDimensions (void)
{
	CRect	rc;

GetClientRect (rc);
int width = (rc.Width () + 3) & ~3; // long word align
int height = rc.Height ();
m_viewWidth  = width;
m_viewHeight = height;
}

//----------------------------------------------------------------------------
// InitView()
//----------------------------------------------------------------------------

void CMineView::ResetView (bool bRefresh)
{
#if OGL_RENDERING
GLKillWindow ();
#else
if (m_DIB) {
	DeleteObject(m_DIB);
	m_DIB = 0;
	}
if (m_DC.m_hDC)
	m_DC.DeleteDC();
#endif
if (bRefresh)
	Refresh (true);
}

//----------------------------------------------------------------------------
// InitView()
//----------------------------------------------------------------------------

void CMineView::InitView(CDC* pViewDC)
{
	// if all else fails, then return the original device context
#if 1//OGL_RENDERING == 0
	m_pDC = pViewDC;
#endif
	// if view size is new, then reset dib and delete the current device context
	CRect Rect;
	GetClientRect(Rect);

//	int depth = m_pDC->GetDeviceCaps(BITSPIXEL) / 8;
	int depth = 1; // force 8-bit DIB

	int width = (Rect.Width() + 3) & ~3; // long word align
	int height = Rect.Height();

	if (width != m_viewWidth || height != m_viewHeight || depth != m_viewDepth) {
		m_bUpdate = true;
#if 1//OGL_RENDERING == 0
		ResetView ();
#endif
	}
	m_viewWidth  = width;
	m_viewHeight = height;
	m_viewDepth  = depth;

#if 1//OGL_RENDERING == 0
	if (!m_DIB) {
		if (!m_DC.m_hDC)
			m_DC.CreateCompatibleDC(pViewDC);
		if (m_DC.m_hDC) {
			typedef struct {
				BITMAPINFOHEADER    bmiHeader;
				RGBQUAD             bmiColors [256];
			} MYBITMAPINFO;

			// define the bitmap parameters
			MYBITMAPINFO mybmi;
			mybmi.bmiHeader.biSize              = sizeof(BITMAPINFOHEADER);
			mybmi.bmiHeader.biWidth             = m_viewWidth;
			mybmi.bmiHeader.biHeight            = m_viewHeight;
			mybmi.bmiHeader.biPlanes            = 1;
			mybmi.bmiHeader.biBitCount          = m_viewDepth*8;
			mybmi.bmiHeader.biCompression       = BI_RGB;
			mybmi.bmiHeader.biSizeImage         = 0;
			mybmi.bmiHeader.biXPelsPerMeter     = 1000;
			mybmi.bmiHeader.biYPelsPerMeter     = 1000;
			mybmi.bmiHeader.biClrUsed           = 256;
			mybmi.bmiHeader.biClrImportant      = 256;

			// copy the bitmap palette
			HINSTANCE hInst = AfxGetInstanceHandle();
			HRSRC hFind     = FindResource( hInst, PaletteResource(), "RC_DATA");
			ASSERT(hFind);
			if (hFind) {
				HGLOBAL hGlobal = LoadResource( hInst, hFind);
				ASSERT(hGlobal);
				if (hGlobal) {
					UINT8 *palette = (UINT8 *) LockResource(hGlobal);
					int i, j;
					for (i = j = 0; i < 256; i++) {
						mybmi.bmiColors [i].rgbRed   = palette [j++]<<2;
						mybmi.bmiColors [i].rgbGreen = palette [j++]<<2;
						mybmi.bmiColors [i].rgbBlue  = palette [j++]<<2;
						mybmi.bmiColors [i].rgbReserved = 0;
					}
				}
			}
			m_DIB = ::CreateDIBSection(m_DC.m_hDC, (BITMAPINFO *) &mybmi, DIB_RGB_COLORS, &m_pvBits, NULL, 0);
			if (m_DIB) {
				m_DC.SelectObject(m_DIB);
			}
		}
	}
	// if DIB exists, then use our own DC instead of the View DC
	if (m_DIB) {
		m_pDC = &m_DC;
	}
#endif
}

//----------------------------------------------------------------------------
// ClearView()
//
// TODO: only clear the dirty area defined by the clip region
//----------------------------------------------------------------------------
void CMineView::ClearView()
{
	// clear the dib or the view
	if (m_DIB)
	{
		memset(m_pvBits, 0, m_viewWidth * m_viewHeight * m_viewDepth);
	}
	else
	{
		CRect rect;
		GetClientRect(rect);
		FillRect(m_pDC->m_hDC, rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}
}

//----------------------------------------------------------------------------
// DrawWireFrame
//----------------------------------------------------------------------------
void CMineView::DrawWireFrame(CMine *mine, bool bPartial)
{
	INT32			segnum;
	CDSegment	*seg;

m_pDC->SelectObject(m_penGray);
for (segnum=0, seg = mine->Segments ();segnum<mine->SegCount ();segnum++, seg++) {
	DrawCube (seg, bPartial);
	if (segnum == m_Current->segment) {
		DrawCurrentCube (seg, bPartial);
		m_pDC->SelectObject (m_penGray);
		}
	}
}

//----------------------------------------------------------------------------
// DrawTextureMappedCubes
//----------------------------------------------------------------------------

typedef struct tSegZOrder {
	INT32		zMax;
	INT16		iSeg;
} tSegZOrder;

typedef tSegZOrder *pSegZOrder;

static tSegZOrder szo [MAX_SEGMENTS2];

void QSortCubes (INT16 left, INT16 right)
{
	INT32		m = szo [(left + right) / 2].zMax;
	tSegZOrder	h;
	INT16	l = left, r = right;
do {
	while (szo [l].zMax > m)
		l++;
	while (szo [r].zMax < m)
		r--;
	if (l <= r) {
		if (l < r) {
			h = szo [l];
			szo [l] = szo [r];
			szo [r] = h;
			}
		l++;
		r--;
		}
	}
while (l < r);
if (l < right)
	QSortCubes (l, right);
if (left < r)
	QSortCubes (left, r);
}

void CMineView::DrawTextureMappedCubes(CMine *mine)
{
	UINT32 segnum;
	INT16	 iVertex;
	INT32	 z, zMax;
	CDSegment *seg;

	// Get shading table data
HINSTANCE hInst = AfxGetInstanceHandle();
HRSRC hFind = m_viewMineFlags & eViewMineShading ? FindResource( hInst, PaletteResource(), "RC_DATA" /*"RC_DATA"*/) : NULL;
UINT8* light_index = 0;
if (hFind) {
	HGLOBAL hGlobal = LoadResource( hInst, hFind);
	if (hGlobal) {
		light_index = (UINT8 *) LockResource(hGlobal);
		if (light_index)
			light_index += 256*5; // skip 3-byte palette + 1st 2 light tables
		}
	}

// Draw Segments ()
for (segnum = 0, seg = mine->Segments (); segnum < mine->SegCount (); segnum++) {
	szo [segnum].iSeg = segnum;
	for (iVertex = 0, zMax = LONG_MIN; iVertex < MAX_VERTICES_PER_SEGMENT; iVertex++)
		if (zMax < (z = m_viewPoints [seg->verts [iVertex]].z))
			zMax = z;
	szo [segnum].zMax = zMax;
	}
QSortCubes (0, mine->SegCount () - 1);
for (segnum = 0; segnum < mine->SegCount (); segnum++)
 	DrawCubeTextured (mine->Segments (szo [segnum].iSeg), light_index);
}

//--------------------------------------------------------------------------
// DrawCube()
//--------------------------------------------------------------------------
#define IN_RANGE(value,absolute_range) ((-absolute_range <= value) && (value <= absolute_range))

bool CMineView::InRange (INT16 *pv, INT16 i)
{
	int	v;

for (; i; i--, pv++) {
	v = *pv;
	if (!(IN_RANGE (m_viewPoints [v].x, x_max) &&
			IN_RANGE (m_viewPoints [v].y, y_max)))
		return false;
	}
return true;
}

								/*-----------------------*/

void CMineView::DrawCube (CDSegment *seg, bool bPartial)
{
DrawCubeQuick (seg, bPartial);
}

void CMineView::DrawCube (CMine *mine, INT16 segnum,INT16 sidenum, INT16 linenum, INT16 pointnum, INT16 clear_it) 
{
	CDSegment *seg = mine->Segments (segnum);
	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

	// clear segment and point
	if (clear_it) {
		m_pDC->SelectObject ((HBRUSH)GetStockObject(NULL_BRUSH));
		m_pDC->SelectObject (GetStockObject(BLACK_PEN)); // BLACK
		int nVert = seg->verts [side_vert [sidenum] [pointnum]];
		if (IN_RANGE (m_viewPoints [nVert].x,x_max) &&
			 IN_RANGE (m_viewPoints [nVert].y,y_max)) {
			m_pDC->Ellipse(m_viewPoints [nVert].x - 4,
			m_viewPoints [nVert].y - 4,
			m_viewPoints [nVert].x + 4,
			m_viewPoints [nVert].y + 4);
			}
		if (seg->wall_bitmask & MARKED_MASK) {
			m_pDC->SelectObject (m_penCyan);
			DrawCubeQuick (seg);
			} 
		else {
			if (m_viewOption == eViewPartialLines) {
				m_pDC->SelectObject (GetStockObject(BLACK_PEN));  // BLACK
				DrawCubeQuick (seg);   // clear all cube lines
				m_pDC->SelectObject (m_penGray); // GRAY
				DrawCubePartial (seg); // then redraw the ones we need
				}
			if ((m_viewOption == eViewAllLines) || 
				 (m_viewOption == eViewNearbyCubeLines) || 
				 (m_viewOption == eViewTextureMapped)) {
				m_pDC->SelectObject (m_penGray); // GRAY
				DrawCubeQuick (seg);
				}
			if (m_viewOption == eViewHideLines) {
				m_pDC->SelectObject (GetStockObject(BLACK_PEN));  // BLACK
				DrawCubeQuick (seg);   // clear all cube lines
				}
			}
			if (m_viewOption == eViewNearbyCubeLines) {
			m_pDC->SelectObject (GetStockObject(BLACK_PEN));  // BLACK
			DrawCubeQuick (seg);   // clear all cube lines
			m_pDC->SelectObject (GetStockObject(WHITE_PEN)); // WHITE
			DrawCubePoints (seg);  // then draw the points
			}
		} 
	else {
		if (seg->wall_bitmask & MARKED_MASK)
			m_pDC->SelectObject (m_penHiCyan);
		else if (segnum == mine->Current ()->segment)
			if (SelectMode (eSelectCube)) // && edit_mode != EDIT_OFF) {
				m_pDC->SelectObject (m_penHiRed);         // RED
			else
				m_pDC->SelectObject (GetStockObject (WHITE_PEN)); // WHITE
			else
				m_pDC->SelectObject (m_penHiGray);        // LIGHT_GRAY
		if (m_viewOption == eViewPartialLines)
			DrawCubePartial (seg); // then redraw the ones we need
		else
			DrawCubeQuick (seg);
		}

	// draw current side
	// must draw in same order as segment to avoid leftover pixels on screen
	if (!clear_it) {
		if (segnum == mine->Current ()->segment)
			if (SelectMode (eSelectSide)) // && edit_mode != EDIT_OFF) {
				m_pDC->SelectObject (m_penHiRed);        // RED
			else
				m_pDC->SelectObject (m_penHiGreen); // GREEN
		else
			m_pDC->SelectObject (m_penHiDkGreen);         // DARK_GREEN
		if (IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [0]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [0]]].y,y_max) &&
			 IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [1]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [1]]].y,y_max) &&
			 IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [2]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [2]]].y,y_max) &&
			 IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [3]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [3]]].y,y_max))
			 DrawLine (seg,side_vert [sidenum] [0],side_vert [sidenum] [1]);
			 DrawLine (seg,side_vert [sidenum] [1],side_vert [sidenum] [2]);
			 DrawLine (seg,side_vert [sidenum] [2],side_vert [sidenum] [3]);
			 DrawLine (seg,side_vert [sidenum] [3],side_vert [sidenum] [0]);

		// draw current line
		// must draw in same order as segment to avoid leftover pixels on screen
		if (segnum == mine->Current ()->segment)
			if (SelectMode (eSelectLine)) // && edit_mode != EDIT_OFF) {
				m_pDC->SelectObject (m_penHiRed);  // RED
			else 
				m_pDC->SelectObject (m_penHiCyan);  // BLUE/CYAN
		else
			m_pDC->SelectObject (m_penDkCyan);  // BLUE/CYAN
		if (IN_RANGE(m_viewPoints [seg->verts [line_vert [side_line [sidenum] [linenum]] [0]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [seg->verts [line_vert [side_line [sidenum] [linenum]] [0]]].y,y_max) &&
			 IN_RANGE(m_viewPoints [seg->verts [line_vert [side_line [sidenum] [linenum]] [1]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [seg->verts [line_vert [side_line [sidenum] [linenum]] [1]]].y,y_max))
			DrawLine (seg,
						 line_vert [side_line [sidenum] [linenum]] [0],
						 line_vert [side_line [sidenum] [linenum]] [1]);
		}

	// draw a circle around the current point
	if (!clear_it) {
		m_pDC->SelectObject ((HBRUSH)GetStockObject(NULL_BRUSH));
		if (segnum == mine->Current ()->segment)
			if (SelectMode (eSelectPoint)) //  && edit_mode != EDIT_OFF) {
				m_pDC->SelectObject (m_penHiRed); // RED
			else
				m_pDC->SelectObject (m_penHiCyan); // CYAN
		else
			m_pDC->SelectObject (m_penHiDkCyan); // CYAN
		if (IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [pointnum]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [pointnum]]].y,y_max))
			m_pDC->Ellipse(m_viewPoints [seg->verts [side_vert [sidenum] [pointnum]]].x - 4,
								m_viewPoints [seg->verts [side_vert [sidenum] [pointnum]]].y - 4,
								m_viewPoints [seg->verts [side_vert [sidenum] [pointnum]]].x + 4,
								m_viewPoints [seg->verts [side_vert [sidenum] [pointnum]]].y + 4);
		}
}

//--------------------------------------------------------------------------
//			 draw_partial_segment()
//--------------------------------------------------------------------------

void CMineView::DrawCubePartial (CDSegment *seg) {
  INT16 line;
  INT16 vert0,vert1;

for (line=0;line<12;line++) {
	if (seg->map_bitmask & (1<<line)) {
      if (IN_RANGE(m_viewPoints [seg->verts [line_vert [line] [0]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [seg->verts [line_vert [line] [0]]].y,y_max) &&
			 IN_RANGE(m_viewPoints [seg->verts [line_vert [line] [1]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [seg->verts [line_vert [line] [1]]].y,y_max)) {
			vert0 = line_vert [line] [0];
			vert1 = line_vert [line] [1];
			if (vert1>vert0) {
				m_pDC->MoveTo (m_viewPoints [seg->verts [vert0]].x,m_viewPoints [seg->verts [vert0]].y);
				m_pDC->LineTo (m_viewPoints [seg->verts [vert1]].x,m_viewPoints [seg->verts [vert1]].y);
				}
			else {
				m_pDC->MoveTo (m_viewPoints [seg->verts [vert1]].x,m_viewPoints [seg->verts [vert1]].y);
				m_pDC->LineTo (m_viewPoints [seg->verts [vert0]].x,m_viewPoints [seg->verts [vert0]].y);
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
// DrawCube()
//--------------------------------------------------------------------------

void QSortLineRef (POINT *lineRef, INT16 left, INT16 right)
{
	INT32		m = lineRef [(left + right) / 2].y;
	INT16	l = left, r = right;
do {
	while (lineRef [l].y < m)
		l++;
	while (lineRef [r].y > m)
		r--;
	if (l <= r) {
		if (l < r) {
			POINT h = lineRef [l];
			lineRef [l] = lineRef [r];
			lineRef [r] = h;
			}
		l++;
		r--;
		}
	}
while (l < r);
if (l < right)
	QSortLineRef (lineRef, l, right);
if (left < r)
	QSortLineRef (lineRef, left, r);
}



void CMineView::DrawCubeQuick	(CDSegment *seg, bool bPartial)
{
if (!GetMine ())
	return;

	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;
	int	chSegI, chSideI, chVertI, i, commonVerts;
	CDSegment	*child;
	INT16 *pv = seg->verts;

for (i = 0; i < 8; i++, pv++) {
	int	v = *pv;
	if (!(IN_RANGE (m_viewPoints [v].x, x_max) &&
			IN_RANGE (m_viewPoints [v].y, y_max)))
		return;
	}
if (bPartial) {
	UINT32 sidenum;
	for (sidenum=0; sidenum<6; sidenum++) {
		if (seg->children [sidenum] >= 0)
			continue;
		
		POINT	side [4], line [2], vert;
		for (i = 0; i < 4; i++) {
			side [i].x = m_viewPoints [seg->verts [side_vert [sidenum] [i]]].x; 
			side [i].y = m_viewPoints [seg->verts [side_vert [sidenum] [i]]].y; 
			}
		vms_vector a,b;
		a.x = side [1].x - side [0].x;
		a.y = side [1].y - side [0].y;
		b.x = side [3].x - side [0].x;
		b.y = side [3].y - side [0].y;
		if (a.x*b.y < a.y*b.x)
			m_pDC->SelectObject((HPEN)GetStockObject(WHITE_PEN));
		else
			m_pDC->SelectObject(m_penGray);
		// draw each line of the current side separately
		// only draw if there is no child cube of the current cube with a common side
		for (i = 0; i < 4; i++) {
			for (int j = 0; j < 2; j++)
				line [j] = side [(i+j)%4];

			// check child cubes
			commonVerts = 0;
			for (chSegI = 0; (chSegI < 6) && (commonVerts < 2); chSegI++) {
				if (seg->children [chSegI] < 0)
					continue;
				child = m_mine->Segments (seg->children [chSegI]);
				// walk through child cube's sides
				commonVerts = 0;
				for (chSideI = 0; (chSideI < 6) && (commonVerts < 2); chSideI++) {
					// check current child cube side for common line
					// i.e. check each line for common vertices with the parent line
					for (commonVerts = 0, chVertI = 0; (chVertI < 4) && (commonVerts < 2); chVertI++) {
						vert.x = m_viewPoints [child->verts [side_vert [chSideI] [chVertI]]].x;
						vert.y = m_viewPoints [child->verts [side_vert [chSideI] [chVertI]]].y;
						for (int h = 0; h < 2; h++) {
							if ((line [h].x == vert.x) && (line [h].y == vert.y)) {
								++commonVerts;
								break;
								}
							}
						}
					}
				}
			if (commonVerts < 2)
				m_pDC->Polyline (line, 2);
			}
		}
	}
else {	//!bPartial
	POINT	lines [12][2];
	POINT lineRef [12];
/*
	static	int poly1 [] = {4,0,1,2,3};
	static	int poly2 [] = {3,0,3,7};
	static	int poly3 [] = {5,0,4,5,6,7};
	static	int poly4 [] = {2,4,7};
	static	int poly5 [] = {2,2,6};
	static	int poly6 [] = {2,1,5};
	static	int* polys [] = {poly1, poly2, poly3, poly4, poly5, poly6};
*/
	static	int points [] = {0,1,1,2,2,3,3,0,0,4,4,5,5,6,6,7,7,4,3,7,2,6,1,5,-1};
	int		i, j, k, v, l;

	for (i = 0;; i++) {
		k = points [i];
		if (0 > k)
			break;
		v = seg->verts [k];
		l = i/2;
		j = i&1;
		if (!j)
			lineRef [l].y = LONG_MIN;
		lines [l][j].x = m_viewPoints [v].x;
		lines [l][j].y = m_viewPoints [v].y;
		lineRef [l].x = l;
		if (lineRef [l].y < m_viewPoints [v].z)
			lineRef [l].y = m_viewPoints [v].z;
		}
	QSortLineRef (lineRef, 0, 11);
	for (i = 0; i < 12; i++)
		m_pDC->Polyline (lines [lineRef [i].x], 2);
	}
}

//--------------------------------------------------------------------------
// draw_line()
//
// works for all angles
//--------------------------------------------------------------------------

void CMineView::DrawLine (UINT8 *bitmapBuffer, POINT pt0, POINT pt1, UINT8 color) 
{
	int i,x,y;
	int dx = pt1.x - pt0.x;
	int dy = pt1.y - pt0.y;

#if 1
	int xInc, yInc;
	int nStep = 0;

if (dx > 0)
	xInc = 1;
else {
	xInc = -1;
	dx = -dx;
	}
if (dy > 0)
	yInc = 1;
else {
	yInc = -1;
	dy = -dy;
	}

x = pt0.x;
y = pt0.y;

#if 0	//most universal
int xStep = 0, yStep = 0;
int dd = (dx >= dy) ? dx : dy;
for (i = dd + 1; i; i--) {
	bitmapBuffer [y*64+x] = color;
	yStep += dy;
	if (yStep >= dx) {
		y += yInc;
		yStep = dx ? yStep % dx : 0;
		}
	xStep += dx;
	if (xStep >= dy) {
		x += xInc;
		xStep = dy ? xStep % dy : 0;
		}
	}
#else //0; faster
if (dx >= dy) {
	for (i = dx + 1; i; i--, x += xInc) {
		bitmapBuffer [y*64+x] = color;
		nStep += dy;
		if (nStep >= dx) {
			y += yInc;
			nStep -= dx;
			}
		}
	}
else {
	for (i = dy + 1; i; i--, y += yInc) {
		bitmapBuffer [y*64+x] = color;
		nStep += dx;
		if (nStep >= dy) {
			x += xInc;
			nStep -= dy;
			}
		}
	}
#endif //0
#else //0
if (dx == 0) {
	x = pt0.x;
	if (dy>0)
		for (y=pt0.y;y<=pt1.y;y++)
			bitmapBuffer [y*64+x] = color;
	else
		for (y=pt0.y;y>=pt1.y;y--)
			bitmapBuffer [y*64+x] = color;
	return;
	}

if (dy == 0) {
	y = pt0.y;
	if (dx > 0)
		for (x=pt0.x;x<=pt1.x;x++)
			bitmapBuffer [y*64+x] = color;
	else
		for (x=pt0.x;x>=pt1.x;x--)
			bitmapBuffer [y*64+x] = color;
	return;
	}

if (dx > 0)
	if (dy > 0)
		for (y=pt0.y,x=pt0.x;y<=pt1.y,x<=pt1.x;y++,x++)
			bitmapBuffer [y*64+x] = color;
	else
		for (y=pt0.y,x=pt0.x;y>=pt1.y,x<=pt1.x;y--,x++)
			bitmapBuffer [y*64+x] = color;
else
	if (dy > 0)
		for (y=pt0.y,x=pt0.x;y<=pt1.y,x>=pt1.x;y++,x--)
			bitmapBuffer [y*64+x] = color;
	else
		for (y=pt0.y,x=pt0.x;y>=pt1.y,x>=pt1.x;y--,x--)
			bitmapBuffer [y*64+x] = color;
#endif //0
}

//--------------------------------------------------------------------------
// DrawAnimDirArrows()
//--------------------------------------------------------------------------

void CMineView::DrawAnimDirArrows (INT16 texture1, UINT8 *bitmapBuffer)
{
	int sx,sy;
	int bScroll = GetMine ()->ScrollSpeed (texture1, &sx, &sy);

if (!bScroll)
	return;

	POINT *pt;
	static POINT ptp0 [4] = {{54,32},{12,32},{42,42},{42,22}};
	static POINT pt0n [4] = {{32,12},{32,54},{42,22},{22,22}};
	static POINT ptn0 [4] = {{12,32},{54,32},{22,22},{22,42}};
	static POINT pt0p [4] = {{32,54},{32,12},{22,42},{42,42}};
	static POINT ptpn [4] = {{54,12},{12,54},{54,22},{42,12}};
	static POINT ptnn [4] = {{12,12},{54,54},{22,12},{12,22}};
	static POINT ptnp [4] = {{12,54},{54,12},{12,42},{22,54}};
	static POINT ptpp [4] = {{54,54},{12,12},{42,54},{54,42}};

if (sx >0 && sy==0) pt = ptp0;
else if (sx >0 && sy >0) pt = ptpp;
else if (sx==0 && sy >0) pt = pt0p;
else if (sx <0 && sy >0) pt = ptnp;
else if (sx <0 && sy==0) pt = ptn0;
else if (sx <0 && sy <0) pt = ptnn;
else if (sx==0 && sy <0) pt = pt0n;
else if (sx >0 && sy <0) pt = ptpn;

DrawLine (bitmapBuffer, pt [0], pt [1], 1);
DrawLine (bitmapBuffer, pt [0], pt [2], 1);
DrawLine (bitmapBuffer, pt [0], pt [3], 1);
}

//--------------------------------------------------------------------------
// DrawCubeTextured()
//--------------------------------------------------------------------------

void CMineView::DrawCubeTextured(CDSegment *seg, UINT8* light_index) 
{

	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

	if (IN_RANGE(m_viewPoints [seg->verts [0]].x,x_max) &&
		IN_RANGE(m_viewPoints [seg->verts [0]].y,y_max) &&
		IN_RANGE(m_viewPoints [seg->verts [1]].x,x_max) &&
		IN_RANGE(m_viewPoints [seg->verts [1]].y,y_max) &&
		IN_RANGE(m_viewPoints [seg->verts [2]].x,x_max) &&
		IN_RANGE(m_viewPoints [seg->verts [2]].y,y_max) &&
		IN_RANGE(m_viewPoints [seg->verts [3]].x,x_max) &&
		IN_RANGE(m_viewPoints [seg->verts [3]].y,y_max) &&
		IN_RANGE(m_viewPoints [seg->verts [4]].x,x_max) &&
		IN_RANGE(m_viewPoints [seg->verts [4]].y,y_max) &&
		IN_RANGE(m_viewPoints [seg->verts [5]].x,x_max) &&
		IN_RANGE(m_viewPoints [seg->verts [5]].y,y_max) &&
		IN_RANGE(m_viewPoints [seg->verts [6]].x,x_max) &&
		IN_RANGE(m_viewPoints [seg->verts [6]].y,y_max) &&
		IN_RANGE(m_viewPoints [seg->verts [7]].x,x_max) &&
		IN_RANGE(m_viewPoints [seg->verts [7]].y,y_max)   )
	{

		int resolution = 0;
		UINT8 *bitmapBuffer = bmBuf;
		UINT8 *pm_viewPointsMem = (UINT8 *)m_pvBits;
		UINT16 width = m_viewWidth;
		UINT16 height = m_viewHeight;
		UINT16 rowOffset = (m_viewWidth + 3) & ~3;
		UINT16 sidenum = 5;
		CDWall *pWall;
		UINT16 wallnum = NO_WALL;
		CDTexture tx (bmBuf);
		
		for (sidenum=0; sidenum<6; sidenum++)
		{
//			if (seg->wall_bitmask & (1 << sidenum))
//				_asm int 3;
			pWall = ((wallnum = seg->sides [sidenum].wall_num) == NO_WALL) ? NULL : ((CDlcDoc*) GetDocument ())->m_mine->Walls () + wallnum;
			if ((seg->children [sidenum] == -1) ||
				(pWall && (pWall->type != WALL_OPEN) && ((pWall->type != WALL_CLOAKED) || pWall->cloak_value))
				)
			{
				APOINT& p0 = m_viewPoints [seg->verts [side_vert [sidenum] [0]]];
				APOINT& p1 = m_viewPoints [seg->verts [side_vert [sidenum] [1]]];
				APOINT& p3 = m_viewPoints [seg->verts [side_vert [sidenum] [3]]];

				vms_vector a,b;
				a.x = p1.x - p0.x;
				a.y = p1.y - p0.y;
				b.x = p3.x - p0.x;
				b.y = p3.y - p0.y;
				if (a.x*b.y > a.y*b.x) {

					INT16 texture1 = seg->sides [sidenum].tmap_num;
					INT16 texture2 = seg->sides [sidenum].tmap_num2;
					if (!DefineTexture(texture1,texture2, &tx, 0, 0)) {
						DrawAnimDirArrows (texture1, &tx);
						TextureMap(resolution, seg, sidenum, tx.data, tx.width, tx.height, 
									  light_index, pm_viewPointsMem, m_viewPoints, width, height, rowOffset);
						}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
//                        draw_segment_points()
//--------------------------------------------------------------------------

void CMineView::DrawCubePoints (CDSegment *seg)
{
	INT16		*pv = seg->verts;
	COLORREF	color = RGB (128,128,128);

for (int h, i = 0; i < 8; i++, pv++) {
	h = *pv;
	m_pDC->SetPixel (m_viewPoints [h].x, m_viewPoints [h].y, color);
	}
#if 0
pDC->SetPixel (m_viewPoints [seg.verts [0]].x,m_viewPoints [seg.verts [0]].y,RGB(128,128,128));
pDC->SetPixel (m_viewPoints [seg.verts [1]].x,m_viewPoints [seg.verts [1]].y,RGB(128,128,128));
pDC->SetPixel (m_viewPoints [seg.verts [2]].x,m_viewPoints [seg.verts [2]].y,RGB(128,128,128));
pDC->SetPixel (m_viewPoints [seg.verts [3]].x,m_viewPoints [seg.verts [3]].y,RGB(128,128,128));
pDC->SetPixel (m_viewPoints [seg.verts [4]].x,m_viewPoints [seg.verts [4]].y,RGB(128,128,128));
pDC->SetPixel (m_viewPoints [seg.verts [5]].x,m_viewPoints [seg.verts [5]].y,RGB(128,128,128));
pDC->SetPixel (m_viewPoints [seg.verts [6]].x,m_viewPoints [seg.verts [6]].y,RGB(128,128,128));
pDC->SetPixel (m_viewPoints [seg.verts [7]].x,m_viewPoints [seg.verts [7]].y,RGB(128,128,128));
#endif
}

//--------------------------------------------------------------------------
//			draw_marked_segments()
//--------------------------------------------------------------------------
void CMineView::DrawMarkedCubes (CMine *mine, INT16 clear_it) {
	CDSegment	*seg;
	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;
	INT16 i;

	// draw marked/special Segments () and Walls ()
	if (!clear_it) {
		for (i=0;i<mine->SegCount ();i++) {
			seg = mine->Segments () + i;
			if (seg->wall_bitmask & MARKED_MASK) {
				m_pDC->SelectObject (SelectMode (eSelectBlock) ? m_penRed : m_penCyan);
				DrawCubeQuick (seg);
				}
			else {
				//    if (show_special) {
				if (ViewFlag (eViewMineSpecial) && !(m_viewOption == eViewTextureMapped) ) {
					switch(seg->special) {
					case SEGMENT_IS_FUELCEN:
						m_pDC->SelectObject (m_penYellow);
						DrawCubeQuick (seg);
						break;
					case SEGMENT_IS_CONTROLCEN:
						m_pDC->SelectObject (m_penOrange);
						DrawCubeQuick (seg);
						break;
					case SEGMENT_IS_REPAIRCEN:
						m_pDC->SelectObject (m_penLtBlue);
						DrawCubeQuick (seg);
						break;
					case SEGMENT_IS_ROBOTMAKER:
						m_pDC->SelectObject (m_penMagenta);
						DrawCubeQuick (seg);
						break;
					case SEGMENT_IS_GOAL_BLUE:
						m_pDC->SelectObject (m_penBlue);
						DrawCubeQuick (seg);
						break;
					case SEGMENT_IS_GOAL_RED:
						m_pDC->SelectObject (m_penRed);
						DrawCubeQuick (seg);
						break;
					case SEGMENT_IS_WATER:
						m_pDC->SelectObject (m_penMedBlue);
						DrawCubeQuick (seg);
						break;
					case SEGMENT_IS_LAVA:
						m_pDC->SelectObject (m_penMedRed);
						DrawCubeQuick (seg);
						break;
					default:
						break;
					}
				}
			}
		}
	}

	// draw a square around all marked points
	m_pDC->SelectObject((HBRUSH)GetStockObject(NULL_BRUSH));
	if (clear_it)
		m_pDC->SelectObject(GetStockObject(BLACK_PEN));
	else if (SelectMode (eSelectBlock)) // && edit_mode != EDIT_OFF) {
		m_pDC->SelectObject (m_penRed);
	else
		m_pDC->SelectObject (m_penCyan);
	for (i=0;i<mine->VertCount ();i++)
		if (*mine->VertStatus (i) & MARKED_MASK)
			if (IN_RANGE(m_viewPoints [i].x,x_max) && IN_RANGE(m_viewPoints [i].y,y_max))
				m_pDC->Rectangle(m_viewPoints [i].x - 4,m_viewPoints [i].y - 4, m_viewPoints [i].x + 4,m_viewPoints [i].y + 4);
}

//--------------------------------------------------------------------------
// DrawCurrentCube()
//--------------------------------------------------------------------------
void CMineView::DrawCurrentCube(CDSegment *seg, bool bPartial)
{
	INT16 sidenum = m_Current->side;
	INT16 linenum = m_Current->point;
	INT16 pointnum = m_Current->point;

	if (seg->wall_bitmask & MARKED_MASK) {
		m_pDC->SelectObject(m_penCyan);
	}
	else
	{
		if (m_selectMode == CUBE_MODE)
		{
			m_pDC->SelectObject(m_penRed);
		} else {
			m_pDC->SelectObject(GetStockObject(WHITE_PEN));
		}
	}

	// draw current side
	// must draw in same order as segment to avoid leftover pixels on screen
	if (m_selectMode == SIDE_MODE)
		m_pDC->SelectObject(m_penRed);
	else
		m_pDC->SelectObject(m_penGreen);

// Select this pen if this is the "other current" cube
//	m_pDC->SelectObject(m_penDkGreen);

	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

	if (IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [0]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [0]]].y,y_max) &&
		 IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [1]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [1]]].y,y_max) &&
		 IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [2]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [2]]].y,y_max) &&
		 IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [3]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [3]]].y,y_max)    ) {

		DrawLine(seg, side_vert [sidenum] [0], side_vert [sidenum] [1]);
		DrawLine(seg, side_vert [sidenum] [1], side_vert [sidenum] [2]);
		DrawLine(seg, side_vert [sidenum] [2], side_vert [sidenum] [3]);
		DrawLine(seg, side_vert [sidenum] [3], side_vert [sidenum] [0]);
	}

	// draw current line
	// must draw in same order as segment to avoid leftover pixels on screen
	if (m_selectMode == LINE_MODE) { // && edit_mode != EDIT_OFF) {
		m_pDC->SelectObject(m_penRed);  // RED
	} else {
		m_pDC->SelectObject(m_penCyan);  // BLUE/CYAN
	}

	if (IN_RANGE(m_viewPoints [seg->verts [line_vert [side_line [sidenum] [linenum]] [0]]].x,x_max) &&
		IN_RANGE(m_viewPoints [seg->verts [line_vert [side_line [sidenum] [linenum]] [0]]].y,y_max) &&
		IN_RANGE(m_viewPoints [seg->verts [line_vert [side_line [sidenum] [linenum]] [1]]].x,x_max) &&
		IN_RANGE(m_viewPoints [seg->verts [line_vert [side_line [sidenum] [linenum]] [1]]].y,y_max)   ) {

		DrawLine(seg, line_vert [side_line [sidenum] [linenum]] [0],
			line_vert [side_line [sidenum] [linenum]] [1]);
	}

	// draw a circle around the current point
	m_pDC->SelectObject((HBRUSH)GetStockObject(NULL_BRUSH));
	if (m_selectMode == POINT_MODE) { //  && edit_mode != EDIT_OFF) {
		m_pDC->SelectObject(m_penRed); // RED
	} else {
		m_pDC->SelectObject(m_penCyan); // CYAN
	}

	if (IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [pointnum]]].x,x_max) &&
		IN_RANGE(m_viewPoints [seg->verts [side_vert [sidenum] [pointnum]]].y,y_max)     ) {

		m_pDC->Ellipse(m_viewPoints [seg->verts [side_vert [sidenum] [pointnum]]].x - 4,
			m_viewPoints [seg->verts [side_vert [sidenum] [pointnum]]].y - 4,
			m_viewPoints [seg->verts [side_vert [sidenum] [pointnum]]].x + 4,
			m_viewPoints [seg->verts [side_vert [sidenum] [pointnum]]].y + 4);
	}
}

//--------------------------------------------------------------------------
// DrawLine ()
//
// Action - draws a line starting with lowest vert
//--------------------------------------------------------------------------

void CMineView::DrawLine(CDSegment *seg,INT16 vert1,INT16 vert2) 
{
	if (vert2 > vert1) {
		m_pDC->MoveTo(m_viewPoints [seg->verts [vert1]].x,m_viewPoints [seg->verts [vert1]].y);
		m_pDC->LineTo(m_viewPoints [seg->verts [vert2]].x,m_viewPoints [seg->verts [vert2]].y);
	} else {
		m_pDC->MoveTo(m_viewPoints [seg->verts [vert2]].x,m_viewPoints [seg->verts [vert2]].y);
		m_pDC->LineTo(m_viewPoints [seg->verts [vert1]].x,m_viewPoints [seg->verts [vert1]].y);
	}
}
//--------------------------------------------------------------------------
// DrawWalls()
//--------------------------------------------------------------------------

void CMineView::DrawWalls(CMine *mine) 
{
	CDWall	*walls = mine->Walls ();
	CDSegment *segments = mine->Segments ();
	vms_vector	*vertices = mine->Vertices ();
	CDSegment	*seg;
	INT16 i,j;
	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

  for (i=0;i<mine->GameInfo ().walls.count;i++) {
	seg = segments + (int)walls [i].segnum;
	switch(walls [i].type) {
      case WALL_NORMAL    :
	m_pDC->SelectObject(m_penLtGray);
	break;
      case WALL_BLASTABLE :
	m_pDC->SelectObject(m_penLtGray);
	break;
	  case WALL_DOOR      :
	switch(walls [i].keys) {
	  case KEY_NONE :
	    m_pDC->SelectObject(m_penLtGray);
	    break;
	  case KEY_BLUE :
	    m_pDC->SelectObject(m_penBlue);
		break;
	  case KEY_RED  :
	    m_pDC->SelectObject(m_penRed);
	    break;
	  case KEY_GOLD :
	    m_pDC->SelectObject(m_penYellow);
	    break;
	  default:
	 m_pDC->SelectObject(m_penGray);
	}
	break;
      case WALL_ILLUSION  :
	m_pDC->SelectObject(m_penLtGray);
	break;
      case WALL_OPEN      :
	m_pDC->SelectObject(m_penLtGray);
	break;
	  case WALL_CLOSED    :
	m_pDC->SelectObject(m_penLtGray);
	break;
      default:
	m_pDC->SelectObject(m_penLtGray);
    }
	if (IN_RANGE(m_viewPoints [seg->verts [side_vert [(INT16)walls [i].sidenum] [0]]].x,x_max) &&
		IN_RANGE(m_viewPoints [seg->verts [side_vert [(INT16)walls [i].sidenum] [0]]].y,y_max) &&
		IN_RANGE(m_viewPoints [seg->verts [side_vert [(INT16)walls [i].sidenum] [1]]].x,x_max) &&
		IN_RANGE(m_viewPoints [seg->verts [side_vert [(INT16)walls [i].sidenum] [1]]].y,y_max) &&
		IN_RANGE(m_viewPoints [seg->verts [side_vert [(INT16)walls [i].sidenum] [2]]].x,x_max) &&
		IN_RANGE(m_viewPoints [seg->verts [side_vert [(INT16)walls [i].sidenum] [2]]].y,y_max) &&
		IN_RANGE(m_viewPoints [seg->verts [side_vert [(INT16)walls [i].sidenum] [3]]].x,x_max) &&
		IN_RANGE(m_viewPoints [seg->verts [side_vert [(INT16)walls [i].sidenum] [3]]].y,y_max)   ) {

      vms_vector center,orthog,vector;
      APOINT point;
      mine->CalcCenter (center,(INT16)walls [i].segnum,(INT16)walls [i].sidenum);
      mine->CalcOrthoVector(orthog,(INT16)walls [i].segnum,(INT16)walls [i].sidenum);
      vector.x = center.x - orthog.x;
      vector.y = center.y - orthog.y;
      vector.z = center.z - orthog.z;
		m_matrix.SetPoint(&vector,&point);
	  for (j=0;j<4;j++) {
		m_pDC->MoveTo(point.x,point.y);
		m_pDC->LineTo(m_viewPoints [seg->verts [side_vert [(INT16)walls [i].sidenum] [j]]].x,
		   m_viewPoints [seg->verts [side_vert [(INT16)walls [i].sidenum] [j]]].y);
      }
      if (walls [i].trigger!= NO_TRIGGER) {
	APOINT arrowstart_point,arrowend_point,arrow1_point,arrow2_point;
	vms_vector fin;

	// calculate arrow points
	vector.x = center.x - 3*orthog.x;
	vector.y = center.y - 3*orthog.y;
	vector.z = center.z - 3*orthog.z;
	m_matrix.SetPoint(&vector,&arrowstart_point);
	vector.x = center.x + 3*orthog.x;
	vector.y = center.y + 3*orthog.y;
	vector.z = center.z + 3*orthog.z;
	m_matrix.SetPoint(&vector,&arrowend_point);

	// direction toward center of line 0 from center

	vector.x  = vertices [seg->verts [side_vert [(INT16)walls [i].sidenum] [0]]].x;
	vector.x += vertices [seg->verts [side_vert [(INT16)walls [i].sidenum] [1]]].x;
	vector.x /= 2;
	vector.y  = vertices [seg->verts [side_vert [(INT16)walls [i].sidenum] [0]]].y;
	vector.y += vertices [seg->verts [side_vert [(INT16)walls [i].sidenum] [1]]].y;
	vector.y /= 2;
	vector.z  = vertices [seg->verts [side_vert [(INT16)walls [i].sidenum] [0]]].z;
	vector.z += vertices [seg->verts [side_vert [(INT16)walls [i].sidenum] [1]]].z;
	vector.z /= 2;
	vector.x -= center.x;
	vector.y -= center.y;
	vector.z -= center.z;
	double length;
	length = my_sqrt(  (double)vector.x*(double)vector.x
			 + (double)vector.y*(double)vector.y
			 + (double)vector.z*(double)vector.z);
	length /= F1_0;
	if (length < 1.0) length = 1.0;
	vector.x = (long) ((double) vector.x / length);
	vector.y = (long) ((double) vector.y / length);
	vector.z = (long) ((double) vector.z / length);

	fin.x = center.x + 2*orthog.x + vector.x;
	fin.y = center.y + 2*orthog.y + vector.y;
	fin.z = center.z + 2*orthog.z + vector.z;
	m_matrix.SetPoint(&fin,&arrow1_point);

	fin.x = center.x + 2*orthog.x - vector.x;
	fin.y = center.y + 2*orthog.y - vector.y;
	fin.z = center.z + 2*orthog.z - vector.z;
	m_matrix.SetPoint(&fin,&arrow2_point);

	// draw arrow
	m_pDC->MoveTo(arrowstart_point.x,arrowstart_point.y);
	m_pDC->LineTo(arrowend_point.x,arrowend_point.y);
	m_pDC->LineTo(arrow1_point.x,arrow1_point.y);
	m_pDC->MoveTo(arrowend_point.x,arrowend_point.y);
	m_pDC->LineTo(arrow2_point.x,arrow2_point.y);
      }
    }
  }
}
//--------------------------------------------------------------------------
//			  draw_lights()
//--------------------------------------------------------------------------
void CMineView::DrawLights (CMine *mine) {
  INT16 i;

  if (!m_pDC) return;

#if 0
  // first show lights that blow up (delta lights)
  SelectObject(m_pDC, m_penLtGray);

  if (file_type != RDL_FILE) {
    DLIndex () = (dl_index *)GlobalLock(hDLIndex ());
    if (DLIndex ()) {
      DeltaLights () = (DeltaLights () *)GlobalLock(hDeltaLights ());
      if (DeltaLights ()) {
	for (i=0;i<GameInfo ().dl_indices.count;i++) {
	  sidenum = DLIndex () [i].sidenum;
	  segnum  = DLIndex () [i].segnum;
	  draw_octagon(m_pDC,sidenum,segnum);
	  if (segnum == current->segment && sidenum == current->side) {
	    POINT light_source;
	    light_source = segment_center_xy(sidenum,segnum);
	    for (int j=0;j<DLIndex () [i].count;j++) {
	      POINT light_dest;
	      int index = DLIndex () [i].index+j;
	      sidenum = DeltaLights () [index].sidenum;
		  segnum  = DeltaLights () [index].segnum;
	      segment *seg = Segments () [segnum];
	      light_dest = segment_center_xy(sidenum,segnum);
		  for (int k=0;k<4;k++)  {
		POINT corner;
		UINT8 l = DeltaLights () [index].vert_light [k];
		l = min(0x1f,l);
		l <<= 3;
		m_pen m_penLight = CreatePen(PS_SOLID, 1, RGB(l,l,255-l));
		SelectObject(m_pDC,m_penLight);
		corner.x = m_viewPoints [seg->verts [side_vert [sidenum] [k]]].x;
		corner.y = m_viewPoints [seg->verts [side_vert [sidenum] [k]]].y;
		corner.x = (corner.x + light_dest.x)>>1;
		corner.y = (corner.y + light_dest.y)>>1;
		MoveTo(m_pDC,light_source.x,light_source.y);
		LineTo(m_pDC,corner.x,corner.y);
		SelectObject(m_pDC, m_penLtGray);
		DeleteObject(m_penLight);
	      }
	    }
	  }
	}
	GlobalUnlock(hDeltaLights ());
	  }
      GlobalUnlock(hDLIndex ());
    }
  }
#endif
  // now show flickering lights
  m_pDC->SelectObject(m_penYellow);

  // find flickering light from
  for (i=0;i<mine->FlickerLightCount ();i++)
    DrawOctagon(mine, mine->FlickeringLights (i)->sidenum, mine->FlickeringLights (i)->segnum);
}

//------------------------------------------------------------------------
// DrawOctagon()
//------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void CMineView::DrawOctagon(CMine *mine, INT16 sidenum, INT16 segnum) {
	CDSegment *seg;
	INT16 j;
	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

	if (segnum >=0 && segnum <=mine->SegCount () && sidenum>=0 && sidenum<=5 ) {
	  POINT corners [4],center,line_centers [4],diamond [4],fortyfive [4];
	  seg = mine->Segments () + segnum;
	  for (j=0;j<4;j++) {
	    corners [j].x = m_viewPoints [seg->verts [side_vert [sidenum] [j]]].x;
	    corners [j].y = m_viewPoints [seg->verts [side_vert [sidenum] [j]]].y;
	  }
	  if (IN_RANGE(corners [0].x,x_max) && IN_RANGE(corners [0].y,y_max) &&
		   IN_RANGE(corners [1].x,x_max) && IN_RANGE(corners [1].y,y_max) &&
	      IN_RANGE(corners [2].x,x_max) && IN_RANGE(corners [2].y,y_max) &&
	      IN_RANGE(corners [3].x,x_max) && IN_RANGE(corners [3].y,y_max)   ) {

	    center.x = (corners [0].x + corners [1].x + corners [2].x + corners [3].x)>>2;
	    center.y = (corners [0].y + corners [1].y + corners [2].y + corners [3].y)>>2;
		for (j=0;j<4;j++) {
	      int k = (j+1) & 0x03;
			line_centers [j].x = (corners [j].x + corners [k].x) >> 1;
	      line_centers [j].y = (corners [j].y + corners [k].y) >> 1;
	      diamond [j].x = (line_centers [j].x + center.x) >> 1;
	      diamond [j].y = (line_centers [j].y + center.y) >> 1;
	      fortyfive [j].x = ((corners [j].x-center.x)*7)/20 + center.x;
	      fortyfive [j].y = ((corners [j].y-center.y)*7)/20 + center.y;
	    }
	    // draw octagon
	    m_pDC->MoveTo(diamond [3].x,diamond [3].y);
	    for (j=0;j<4;j++) {
	      m_pDC->LineTo(fortyfive [j].x,fortyfive [j].y);
		  m_pDC->LineTo(diamond [j].x,diamond [j].y);
	    }
	  }
	}
}

//----------------------------------------------------------------------------
// draw_spline()
//----------------------------------------------------------------------------

void CMineView::DrawSpline (CMine *mine) 
{
	int h, i, j;

//  SelectObject(hdc, hrgnAll);
m_pDC->SelectObject (m_penRed);
m_pDC->SelectObject ((HBRUSH)GetStockObject(NULL_BRUSH));
mine->CalcSpline ();
APOINT point;
m_matrix.SetPoint (&points[1],&point);
if (IN_RANGE(point.x,x_max) && IN_RANGE(point.y,y_max)){
	m_matrix.SetPoint (&points[0],&point);
	if (IN_RANGE(point.x,x_max) && IN_RANGE(point.y,y_max)){
		m_pDC->MoveTo (point.x,point.y);
		m_matrix.SetPoint (&points[1],&point);
		m_pDC->LineTo (point.x,point.y);
		m_pDC->Ellipse (point.x - 4,point.y - 4,point.x+4, point.y+4);
		}
	}
m_matrix.SetPoint (&points[2],&point);
if (IN_RANGE(point.x,x_max) && IN_RANGE(point.y,y_max)){
	m_matrix.SetPoint (&points[3],&point);
	if (IN_RANGE(point.x,x_max) && IN_RANGE(point.y,y_max)){
		m_pDC->MoveTo (point.x,point.y);
		m_matrix.SetPoint (&points[2],&point);
		m_pDC->LineTo (point.x,point.y);
		m_pDC->Ellipse (point.x - 4,point.y - 4,point.x+4, point.y+4);
		}
	}
m_pDC->SelectObject (m_penBlue);
j = MAX_VERTICES - 1;
vms_vector *verts = mine->Vertices (j);
for (h = n_splines * 4, i = 0; i < h; i++, j--, verts--)
	m_matrix.SetPoint (verts, m_viewPoints + j);
CDSegment *seg = mine->Segments (MAX_SEGMENTS - 1);
for (i = 0; i < n_splines; i++, seg--)
	DrawCubeQuick (seg);
}

//--------------------------------------------------------------------------
//			  DrawObject()
//
// Changed: 0=normal,1=gray,2=black
//        if (objnum == (GameInfo ().objects.count
//        then its a secret return point)
//--------------------------------------------------------------------------

void CMineView::DrawObject(CMine *mine,INT16 objnum,INT16 clear_it) 
{
	INT16 poly;
	CDObject *obj;
	vms_vector pt [MAX_POLY];
	APOINT poly_draw [MAX_POLY];
	APOINT object_shape [MAX_POLY] = {
		{ 0,  4, -4},
		{ 0,  0, -4},
		{ 0,  0,  4},
		{-2,  0,  2},
		{ 2,  0,  2},
		{ 0,  0,  4}
		};
	CDObject temp_obj;
	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

//  m_pDC->SelectObject(hrgnBackground);

if (objnum >=0 && objnum < mine->GameInfo ().objects.count) 
	obj = mine->Objects (objnum);
else {
	// secret return
	obj = &temp_obj;
	obj->type = -1;
	// mine->secret_orient = Objects () [0]->orient;
	obj->orient.rvec.x = -mine->SecretOrient ().rvec.x;
	obj->orient.rvec.y = -mine->SecretOrient ().rvec.y;
	obj->orient.rvec.z = -mine->SecretOrient ().rvec.z;
	obj->orient.uvec.x =  mine->SecretOrient ().fvec.x;
	obj->orient.uvec.y =  mine->SecretOrient ().fvec.y;
	obj->orient.uvec.z =  mine->SecretOrient ().fvec.z;
	obj->orient.fvec.x =  mine->SecretOrient ().uvec.x;
	obj->orient.fvec.y =  mine->SecretOrient ().uvec.y;
	obj->orient.fvec.z =  mine->SecretOrient ().uvec.z;
	// obj->orient =  mine->secret_orient;
	UINT16 segnum = (UINT16)mine->SecretCubeNum ();
	if (segnum >= mine->SegCount ())
		segnum = 0;
	mine->CalcSegCenter(obj->pos,segnum); // define obj->position
	}

switch (clear_it) {
	case 0: // normal
	case 1: // gray
		if (m_selectMode == OBJECT_MODE && objnum == mine->Current ()->object) 
			m_pDC->SelectObject(m_penRed); // RED
		else {
			switch(obj->type) {
				case OBJ_ROBOT    : /* an evil enemy */
				m_pDC->SelectObject(m_penMagenta);
				break;
			case OBJ_HOSTAGE  : /* a hostage you need to rescue */
				m_pDC->SelectObject(m_penBlue);
				break;
			case OBJ_PLAYER   : /* the player on the console */
				m_pDC->SelectObject(m_penCyan);
				break;
			case OBJ_WEAPON   : // exploding mine
				m_pDC->SelectObject(m_penDkGreen);
				break;
			case OBJ_POWERUP  : /* a powerup you can pick up */
				m_pDC->SelectObject(m_penOrange);
				break;
			case OBJ_CNTRLCEN : /* the control center */
				m_pDC->SelectObject(m_penLtGray);
				break;
			case OBJ_COOP     : /* a cooperative player object */
				m_pDC->SelectObject(m_penCyan);
				break;
			default:
				m_pDC->SelectObject(m_penGreen);
				}
			}
		break;
	case 2: // black
		m_pDC->SelectObject(GetStockObject(BLACK_PEN));
		break;
	}

// rotate object shape using object's orient matrix
// then translate object
for (poly = 0; poly < MAX_POLY; poly++) {
	pt [poly].x = (obj->orient.rvec.x * (FIX)object_shape [poly].x +
						obj->orient.uvec.x * (FIX)object_shape [poly].y +
						obj->orient.fvec.x * (FIX)object_shape [poly].z);
	pt [poly].y = (obj->orient.rvec.y * (FIX)object_shape [poly].x +
						obj->orient.uvec.y * (FIX)object_shape [poly].y +
						obj->orient.fvec.y * (FIX)object_shape [poly].z);
	pt [poly].z = (obj->orient.rvec.z * (FIX)object_shape [poly].x +
						obj->orient.uvec.z * (FIX)object_shape [poly].y +
						obj->orient.fvec.z * (FIX)object_shape [poly].z);
	pt [poly].x += obj->pos.x;
	pt [poly].y += obj->pos.y;
	pt [poly].z += obj->pos.z;
	m_matrix.SetPoint (pt + poly, poly_draw + poly);
	}

// figure out world coordinates

for (int i = 0; i < 6; i++)
	if (!(IN_RANGE (poly_draw [i].x, x_max) &&
			IN_RANGE (poly_draw [i].y, y_max)))
		return;

if ((file_type != RDL_FILE) &&
	 (objnum == mine->Current ()->object) &&
	 (obj->render_type == RT_POLYOBJ) &&
	 !SetupModel(mine,obj)) {
	if (!clear_it)
		m_pDC->SelectObject((HBRUSH)GetStockObject(BLACK_BRUSH));
	else {
		m_pDC->SelectObject(GetStockObject(BLACK_PEN));
		m_pDC->SelectObject((HBRUSH)GetStockObject(BLACK_BRUSH));
		}
	DrawModel();
	}
else {
	m_pDC->MoveTo (poly_draw [0].x,poly_draw [0].y);
	for (poly = 0; poly < 6; poly++)
		m_pDC->LineTo (poly_draw [poly].x, poly_draw [poly].y);
	if (objnum == mine->Current ()->object) {
		int dx,dy;
		for (dx = -1; dx < 2; dx++) {
			for (dy = -1; dy < 2; dy++) {
				m_pDC->MoveTo (poly_draw [0].x+dx,poly_draw [0].y+dy);
				for (poly = 0; poly < 6; poly++)
					m_pDC->LineTo (poly_draw [poly].x + dx, poly_draw [poly].y + dy);
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
//			  DrawObjects()
//--------------------------------------------------------------------------

bool CMineView::ViewObject (CDObject *obj)
{
switch(obj->type) {
	case OBJ_ROBOT:
		return ViewObject (eViewObjectsRobots);
	case OBJ_HOSTAGE:
		return ViewObject (eViewObjectsHostages);
	case OBJ_PLAYER:
	case OBJ_COOP:
		return ViewObject (eViewObjectsPlayers);
	case OBJ_WEAPON:
		return ViewObject (eViewObjectsWeapons);
	case OBJ_POWERUP:
		switch (powerup_types [obj->id]) {
			case POWERUP_WEAPON_MASK:
				return ViewObject (eViewObjectsWeapons);
			case POWERUP_POWERUP_MASK:
				return ViewObject (eViewObjectsPowerups);
			case POWERUP_KEY_MASK:
				return ViewObject (eViewObjectsKeys);
			default:
				return false;
			}
	case OBJ_CNTRLCEN:
		return ViewObject (eViewObjectsControlCenter);
	}
return false;
}

//--------------------------------------------------------------------------
//			  DrawObjects()
//--------------------------------------------------------------------------

void CMineView::DrawObjects (CMine *mine, INT16 clear_it) 
{
if (!ViewObject ())
	return;

int i, j;
if (file_type != RDL_FILE) {
	// see if there is a secret exit trigger
	for(i = 0; i < mine->GameInfo ().triggers.count; i++)
	if (mine->Triggers (i)->type == TT_SECRET_EXIT) {
		DrawObject (mine, (INT16)mine->GameInfo ().objects.count, 0);
		break; // only draw one secret exit
		}
	}

CDObject *obj = mine->Objects ();
for (i = mine->GameInfo ().objects.count, j = 0; i; i--, j++, obj++)
	if (ViewObject (obj))
		DrawObject (mine, j, 0);
}

//--------------------------------------------------------------------------
//			  draw_highlight()
//--------------------------------------------------------------------------

void CMineView::DrawHighlight(CMine *mine, INT16 clear_it) {
//	INT16 i;
//	RECT rect;

if (disable_drawing) 
	return;

if (mine->SegCount ()==0) 
	return;

// draw Objects ()
if (!clear_it) {
	DrawObjects (mine, clear_it);
//	if (/*!(preferences & PREFS_HIDE_MARKED_BLOCKS) ||*/ SelectMode (eSelectBlock))
		DrawMarkedCubes(mine, clear_it);
  }

// draw highlighted Segments () (other first, then current)
if (mine->Current () == &mine->Current1 ()) {
	if (mine->Current1 ().segment != mine->Current2 ().segment)
		DrawCube (mine, mine->Current2 ().segment,mine->Current2 ().side,mine->Current2 ().line,mine->Current2 ().point,clear_it);
	DrawCube (mine, mine->Current1 ().segment,mine->Current1 ().side,mine->Current1 ().line,mine->Current1 ().point,clear_it);
	}
else {
	if (mine->Current1 ().segment != mine->Current2 ().segment)
		DrawCube (mine, mine->Current1 ().segment,mine->Current1 ().side,mine->Current1 ().line,mine->Current1 ().point,clear_it);
	DrawCube (mine, mine->Current2 ().segment,mine->Current2 ().side,mine->Current2 ().line,mine->Current2 ().point,clear_it);
	}

// draw Walls ()
if (ViewFlag (eViewMineWalls))
	DrawWalls(mine);

// draw lights
if (ViewFlag (eViewMineLights))
	  DrawLights (mine);

// draw spline
if (mine->m_bSplineActive)
	DrawSpline (mine);

*message = '\0';
if (preferences & PREFS_SHOW_POINT_COORDINATES) {
   strcat(message,"  point (x,y,z): (");
   INT16 vertex = mine->Segments () [mine->Current ()->segment].verts [side_vert [mine->Current ()->side] [mine->Current ()->point]];
	char	szCoord [20];
	sprintf (szCoord, "%1.4f,%1.4f,%1.4f)",mine->Vertices (vertex)->x,mine->Vertices (vertex)->y,mine->Vertices (vertex)->z);
	strcat(message,szCoord);
	}
else {
   // calculate cube size (length between center point of opposing sides)
	strcat(message,"  cube size: ");
	vms_vector center1,center2;
   double length;
   mine->CalcCenter (center1,mine->Current ()->segment,0);
	mine->CalcCenter (center2,mine->Current ()->segment,2);
   length = mine->CalcLength(&center1,&center2) / F1_0;
	sprintf(message + strlen(message),"%.1f",(double)length);
	strcat(message," x ");
   mine->CalcCenter (center1,mine->Current ()->segment,1);
   mine->CalcCenter (center2,mine->Current ()->segment,3);
	length = mine->CalcLength(&center1,&center2) / F1_0;
   sprintf(message + strlen(message),"%.1f",(double)length);
	strcat(message," x ");
   mine->CalcCenter (center1,mine->Current ()->segment,4);
   mine->CalcCenter (center2,mine->Current ()->segment,5);
   length = mine->CalcLength(&center1,&center2) / F1_0;
	sprintf(message + strlen(message),"%.1f",(double)length);
	}
strcat(message,",  cube:");
itoa(mine->Current ()->segment,message + strlen(message),10);
strcat(message," side:");
itoa(mine->Current ()->side,message + strlen(message),10);
strcat(message," point:");
itoa(mine->Current ()->point,message + strlen(message),10);

strcat(message,",  textures:");
strcat(message," 1st:");
itoa(mine->CurrSide ()->tmap_num,message + strlen(message),10);
strcat(message," 2nd:");
itoa(mine->CurrSide ()->tmap_num2,message + strlen(message),10);

strcat(message,",  zoom:");
double zoom_factor = log (10 * m_sizex) / log (1.2);
if (zoom_factor > 0) 
	zoom_factor += 0.5;
else
	zoom_factor -= 0.5;
sprintf(message + strlen(message), "%1.2f", zoom_factor);
STATUSMSG (message);
}

/////////////////////////////////////////////////////////////////////////////
// CMineView printing

BOOL CMineView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CMineView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CMineView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CMineView diagnostics

#ifdef _DEBUG
void CMineView::AssertValid() const
{
	CView::AssertValid();
}

void CMineView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CDlcDoc* CMineView::GetDocument() // non-debug version is inline
{
	if (m_pDocument)
		ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDlcDoc)));
	return (CDlcDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMineView message handlers

BOOL CMineView::OnEraseBkgnd(CDC* pDC)
{
//	return CView::OnEraseBkgnd(pDC);
	return TRUE;
}

                        /*--------------------------*/

void CMineView::OnSize(UINT nType, int cx, int cy)
{
	CRect	rc;

GetClientRect(rc);
// define global screen variables (these must be redefined if window is sized)
left = rc.left;
top = rc.top;
right = rc.right;
bottom = rc.bottom;
x_center = (right-left)/2;
y_center = (bottom-top)/2;
//aspect_ratio = (y_center/7.0) / (x_center/10.0);
aspect_ratio = (double) rc.Height () / (double) rc.Width ();
x_max = 8*right;
y_max = 8*bottom;
//if (theApp.MainFrame () && ((m_viewWidth != cx) || (m_viewHeight != cy)))
//	theApp.MainFrame ()->ResetPaneMode ();
CView::OnSize (nType, cx, cy);
m_bUpdate = true;
}

                        /*--------------------------*/

static double zoomScales [2] = {1.2, 1.033};

int CMineView::ZoomFactor (int nSteps, double min, double max)
{
double zoom;
int i;

for (zoom = log(10*m_sizex), i = 0; i < nSteps; i++) {
	zoom /= log (1.2);
	if ((zoom < min) || (zoom > max))
		return i;
	}
return nSteps; //(int) ((zoom > 0) ? zoom + 0.5 : zoom - 0.5);
}

                        /*--------------------------*/

void CMineView::Zoom (int nSteps, double zoom)
{
for (; nSteps; nSteps--) {
	m_sizex *= zoom;
	m_sizey *= zoom;
	m_sizez *= zoom;
	m_matrix.Scale (1.0 / zoom);
	}
Refresh (false);
}

                        /*--------------------------*/

int CMineView::ZoomIn(int nSteps, bool bSlow)
{
if (nSteps = ZoomFactor (nSteps, -100, 25))
	Zoom (nSteps, zoomScales [bSlow]);
else
	INFOMSG("Already at maximum zoom")
/*
		ErrorMsg("Already at maximum zoom\n\n"
					"Hint: Try using the 'A' and 'Z' keys to\n"
					"move in forward and backwards.");
*/
return nSteps;
}

                        /*--------------------------*/

int CMineView::ZoomOut(int nSteps, bool bSlow)
{
if (nSteps = ZoomFactor (nSteps, -5, 100))
	Zoom (nSteps, 1.0 / zoomScales [bSlow]);
else
	INFOMSG("Already at minimum zoom")
return nSteps;
}

                        /*--------------------------*/

void CMineView::SetViewPoints (CRect *pRC, int* pIMinX, int *pIMinY)
{
if (!GetMine ())
	return;

	CRect		rc (LONG_MAX, LONG_MAX, -LONG_MAX, -LONG_MAX);
	long		i, x, y, iMinX, iMinY, minX = LONG_MAX, minY = LONG_MAX;

m_matrix.Calculate (m_movex, m_movey, m_movez);
InitViewDimensions ();
m_matrix.SetViewInfo (m_DepthPerception, m_viewWidth, m_viewHeight);
for (i = m_mine->VertCount (); --i; ) {
	m_matrix.SetPoint (m_mine->Vertices (i), m_viewPoints + i);
	//m_viewPoints [i].y = -m_viewPoints [i].y;
	x = m_viewPoints [i].x;
	y = m_viewPoints [i].y;
	if (rc.left > x)
		rc.left = x;
	if (rc.right < x)
		rc.right = x;
	if (rc.top > y)
		rc.top = y;
	if (rc.bottom < y)
		rc.bottom = y;
	if (minX > x) {
		minX = x;
		iMinX = i;
		}
	if (minY > y) {
		minY = y;
		iMinY = i;
		}
	}
x = rc.Width ();
y = rc.Height ();
if (pRC)
	*pRC = rc;
if (pIMinX)
	*pIMinX = iMinX;
if (pIMinY)
	*pIMinY = iMinY;
}

                        /*--------------------------*/

int CMineView::FitToView (void)
{
#if OGL_RENDERING
//GLFitToView ();
#else
//if (!GetMine ())
//	return;

	CRect			rc (LONG_MAX, LONG_MAX, -LONG_MAX, -LONG_MAX);
	double		zoomX, zoomY, zoom;
	int			dx, dy;

DelayRefresh (true);
//CenterMine ();
SetViewPoints (&rc);
m_movex = 0.0;
m_movey = 0.0;
m_movez = 0.0;
SetViewPoints (&rc);
CRect	crc;
GetClientRect (crc);
crc.InflateRect (-4, -4);
zoomX = (double) crc.Width () / (double) rc.Width ();
zoomY = (double) crc.Height () / (double) rc.Height ();
zoom = (zoomX < zoomY) ? zoomX : zoomY;
//if (zoom == 1)
//	return 0;
Zoom (1, zoom);
SetViewPoints (&rc);
dy = (crc.Height () - rc.Height ()) / 2;
while (rc.top - dy > 0) {
	Pan ('Y', -1);
	SetViewPoints (&rc);
	}
if (rc.top < dy)
	while (rc.top - dy < 0) {
		Pan ('Y', 1);
		SetViewPoints (&rc);
		}
else
	while (rc.bottom + dy > crc.bottom) {
		Pan ('Y', -1);
		SetViewPoints (&rc);
		}
dx = (crc.Width () - rc.Width ()) / 2;
if (rc.left < dx)
	while (rc.left - dx < 0) {
		Pan ('X', -1);
		SetViewPoints (&rc);
		}
else
	while (rc.right + dx > crc.right) {
		Pan ('X', +1);
		SetViewPoints (&rc);
		}
DelayRefresh (false);
Refresh ();
#endif
return 1;
}

                        /*--------------------------*/

void CMineView::Rotate(char direction, double angle)
{
	m_matrix.Rotate(direction, angle);
	Refresh (false);
}

                        /*--------------------------*/

void CMineView::Pan(char direction, INT32 value)
{
if (value) {
	INT32 index;
	switch(direction)
	{
		default:
		case 'X': 
			index = 1; 
			break;
		case 'Y': 
			index = 2; 
			break;
		case 'Z': 
			index = 3; 
			break;
	}
	m_movex -= value*m_matrix.IM [1] [index];  /* move view point */
	m_movey -= value*m_matrix.IM [2] [index];
	m_movez -= value*m_matrix.IM [3] [index];

	Refresh (false);
	}
}

                        /*--------------------------*/

void CMineView::AlignSide()
{
}
                        /*--------------------------*/

void CMineView::CenterMine()
{
//	CDlcDoc* pDoc = GetDocument();
//	ASSERT_VALID(pDoc);

	vms_vector *verts;
	if (!GetMine ())
		return;

	INT32 maxx = 0;
	INT32 minx = 0;
	INT32 maxy = 0;
	INT32 miny = 0;
	INT32 maxz = 0;
	INT32 minz = 0;

	INT32 i;
	verts = m_mine->Vertices ();
	for (i=0;i<m_mine->VertCount ();i++, verts++) {
		maxx = max(maxx, verts->x);
		minx = min(minx, verts->x);
		maxy = max(maxy, verts->y);
		miny = min(miny, verts->y);
		maxz = max(maxz, verts->z);
		minz = min(minz, verts->z);
	}
	INT16 max_x = (INT16)(maxx / F1_0);
	INT16 max_y = (INT16)(maxy / F1_0);
	INT16 max_z = (INT16)(maxz / F1_0);
	INT16 min_x = (INT16)(minx / F1_0);
	INT16 min_y = (INT16)(miny / F1_0);
	INT16 min_z = (INT16)(minz / F1_0);

	m_spinx = M_PI/4.f;
	m_spiny = M_PI/4.f;
	m_spinz = 0.0;
	m_movex = -(max_x + min_x)/2.f;
	m_movey = -(max_y + min_y)/2.f;
	m_movez = -(max_z + min_z)/2.f;
	int factor;
	int max_all = max(max(max_x-min_x,max_y-min_y),max_z-min_z)/20;
	if (max_all < 2)      factor = 14;
	else if (max_all < 4) factor = 10;
	else if (max_all < 8) factor = 8;
	else if (max_all < 12) factor = 5;
	else if (max_all < 16) factor = 3;
	else if (max_all < 32) factor = 2;
	else factor = 1;
	m_sizex = .1f * (double)pow(1.2,factor);
	m_sizey = m_sizex;
	m_sizez = m_sizex;
	m_matrix.Set(m_movex, m_movey, m_movez,
		m_sizex, m_sizey, m_sizez,
		m_spinx, m_spiny, m_spinz);

Refresh (false);
}

                        /*--------------------------*/

void CMineView::CenterCube()
{
if (!GetMine ())
	return;
	CDSegment& seg = m_mine->Segments () [m_Current->segment];
	vms_vector *vMine = m_mine->Vertices ();
	INT16 *vSeg = seg.verts;

	m_movex = -((double)vMine [seg.verts [0]].x
		 +(double)vMine [vSeg [1]].x
		 +(double)vMine [vSeg [2]].x
		 +(double)vMine [vSeg [3]].x
		 +(double)vMine [vSeg [4]].x
		 +(double)vMine [vSeg [5]].x
		 +(double)vMine [vSeg [6]].x
		 +(double)vMine [vSeg [7]].x)/(double)0x80000L;
	m_movey = -((double)vMine [vSeg [0]].y
		 +(double)vMine [vSeg [1]].y
		 +(double)vMine [vSeg [2]].y
		 +(double)vMine [vSeg [3]].y
		 +(double)vMine [vSeg [4]].y
		 +(double)vMine [vSeg [5]].y
		 +(double)vMine [vSeg [6]].y
		 +(double)vMine [vSeg [7]].y)/(double)0x80000L;
	m_movez = -((double)vMine [vSeg [0]].z
		 +(double)vMine [vSeg [1]].z
		 +(double)vMine [vSeg [2]].z
		 +(double)vMine [vSeg [3]].z
		 +(double)vMine [vSeg [4]].z
		 +(double)vMine [vSeg [5]].z
		 +(double)vMine [vSeg [6]].z
		 +(double)vMine [vSeg [7]].z)/(double)0x80000L;

Refresh (false);
}

                        /*--------------------------*/

void CMineView::CenterObject()
{
	CDlcDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;
	if (!pDoc->m_mine) return;
	CMine *mine = pDoc->m_mine;

	CDObject& obj = mine->Objects () [m_Current->object];
	m_movex = (INT16)(-(obj.pos.x)/0x10000L);
	m_movey = (INT16)(-(obj.pos.y)/0x10000L);
	m_movez = (INT16)(-(obj.pos.z)/0x10000L);

Refresh (false);
}

                        /*--------------------------*/

void CMineView::SetViewOption(eViewOptions option)
{
	m_viewOption = option;
Refresh ();
}

                        /*--------------------------*/

void CMineView::ToggleViewMine (eMineViewFlags flag)
{
	m_viewMineFlags ^= flag;
	EnableDeltaShading ((m_viewMineFlags & eViewMineDeltaLights) != 0, -1, -1);
	Refresh ();
}

                        /*--------------------------*/

void CMineView::ToggleViewObjects(eObjectViewFlags mask)
{
	m_viewObjectFlags ^= mask;
	Refresh ();
}

                        /*--------------------------*/

void CMineView::SetViewMineFlags(UINT32 mask)
{
	m_viewMineFlags = mask;
	Refresh ();
}

                        /*--------------------------*/

void CMineView::SetViewObjectFlags(UINT32 mask)
{
	m_viewObjectFlags = mask;
	Refresh ();
}

                        /*--------------------------*/

void CMineView::SetSelectMode(UINT32 mode)
{
GetMine ()->SetSelectMode ((INT16) mode);
theApp.MainFrame ()->UpdateSelectButtons ((eSelectModes) mode);
m_selectMode = mode; 
Refresh (false);
}

                        /*--------------------------*/

BOOL CMineView::OnSetCursor (CWnd* pWnd, UINT nHitTest, UINT message)
{
//if (m_bUpdateCursor) {
//	::SetCursor (AfxGetApp()->LoadStandardCursor (nIdCursors [m_mouseState]));
//	return TRUE;
//	}
return CView::OnSetCursor (pWnd, nHitTest, message);
}

                        /*--------------------------*/

BOOL CMineView::SetCursor (HCURSOR hCursor)
{
if (!hCursor) // || (hCursor == m_hCursor))
   return FALSE;
::SetClassLong (GetSafeHwnd (), GCL_HCURSOR, (int) (/*m_hCursor =*/ hCursor));
return TRUE;
}
                        
                        /*--------------------------*/

void CMineView::SetMouseState (int newMouseState)
{
if (newMouseState != m_mouseState) {
	m_lastMouseState = m_mouseState;
	m_mouseState = newMouseState;
	m_bUpdateCursor = true;
	SetCursor (m_hCursors [m_mouseState]);
	m_bUpdateCursor = false;
#ifdef _DEBUG
  	INFOMSG (szMouseStates [m_mouseState]);
#endif
	}
}

                        /*--------------------------*/

void CMineView::OnMouseMove(UINT nFlags, CPoint point)
{
	CPoint change = m_lastMousePos - point;

switch(m_mouseState) {
	case eMouseStateIdle:
	case eMouseStatePan:
	case eMouseStateRotate:
		// if Control Key down
		if (nFlags & MK_CONTROL) {
			// if Shift then rotate
			if (nFlags & MK_SHIFT) {
				SetMouseState (eMouseStateRotate);
				Rotate('Y', -(double) change.x / 200.0f);
				Rotate('X', (double) change.y / 200.0f);
				}
			else {// else move
				SetMouseState (eMouseStatePan);
				Pan('X', change.x);
				Pan('Y', -change.y);
				}
			}
		else if ((m_mouseState == eMouseStatePan) || (m_mouseState == eMouseStateRotate))
			SetMouseState (eMouseStateIdle);
		break;

	case eMouseStateButtonDown:
		if (nFlags & MK_CONTROL)
			SetMouseState (eMouseStateZoom);
		else {
			GetMine ();
			int x = m_viewPoints [m_mine->CurrVert ()].x;
			int y = m_viewPoints [m_mine->CurrVert ()].y;
			if (abs (m_clickPos.x - x) < 5 && abs (m_clickPos.y - y) < 5) {
				SetMouseState (eMouseStateInitDrag);
				UpdateDragPos ();
				}
			else {
				SetMouseState (eMouseStateRubberBand);
				UpdateRubberRect (point);
				}
			}
		break;

	case eMouseStateDrag:
		if (change.x || change.y)
			UpdateDragPos ();
		break;

	case eMouseStateZoom:
	case eMouseStateZoomIn:
	case eMouseStateZoomOut:
		if ((change.x > 0) || ((change.x == 0) && (change.y < 0))) {
			SetMouseState (eMouseStateZoomOut);
			ZoomOut (1, true);
			}
		else {
			SetMouseState (eMouseStateZoomIn);
			ZoomIn (1, true);
			}
		break;
		
	case eMouseStateRubberBand:
		UpdateRubberRect (point);
		break;
	}
CView::OnMouseMove(nFlags, point);
m_lastMousePos = point;
}

                        /*--------------------------*/

void CMineView::OnLButtonDown(UINT nFlags, CPoint point)
{
SetMouseState (eMouseStateButtonDown);
m_clickPos = point;
m_clickState = nFlags;
CView::OnLButtonDown(nFlags, point);
}

                        /*--------------------------*/

void CMineView::OnLButtonUp(UINT nFlags, CPoint point)
{
m_releasePos = point;
m_releaseState = nFlags;
if (m_mouseState == eMouseStateButtonDown)
	if (m_clickState & MK_CONTROL)
		ZoomIn ();
	else
		SelectCurrentSegment (1, m_clickPos.x, m_clickPos.y);
else if (m_mouseState == eMouseStateRubberBand) {
   ResetRubberRect ();
	MarkRubberBandedVertices ();
	}
else if (m_mouseState == eMouseStateDrag)
	FinishDrag ();
SetMouseState (eMouseStateIdle);
CView::OnLButtonUp (nFlags, point);
}

                        /*--------------------------*/

void CMineView::OnRButtonDown(UINT nFlags, CPoint point)
{
SetMouseState (eMouseStateButtonDown);
m_clickPos = point;
m_clickState = nFlags;
CView::OnRButtonDown(nFlags, point);
}

                        /*--------------------------*/

void CMineView::OnRButtonUp(UINT nFlags, CPoint point)
{
m_releasePos = point;
m_releaseState = nFlags;
if (m_mouseState == eMouseStateButtonDown)
	if (m_clickState & MK_CONTROL)
		ZoomOut ();
	else {
		SetMouseState (eMouseStateIdle);
		SelectCurrentObject (m_clickPos.x, m_clickPos.y);
		}
else if (m_mouseState == eMouseStateRubberBand)
   ResetRubberRect ();
SetMouseState (eMouseStateIdle);
CView::OnRButtonUp(nFlags, point);
}

                        /*--------------------------*/

void CMineView::Refresh (bool bAll)
{
	static bool bRefreshing = false;

if (!(bRefreshing || m_nDelayRefresh)) {
	bRefreshing = true;
	InvalidateRect (NULL, TRUE);
//	SetFocus ();
	if (bAll && (m_mouseState == eMouseStateIdle)) {
		theApp.ToolView ()->Refresh ();
		theApp.TextureView ()->Refresh ();
//		UpdateWindow ();
		}
	m_bUpdate = true;
	bRefreshing = false;
	}
}

                        /*--------------------------*/

void CMineView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
//m_bUpdate = true;
//InvalidateRect(NULL);
Refresh ();
}

                        /*--------------------------*/

void CMineView::MarkRubberBandedVertices (void)
{
if (!GetMine ())
	return;

	APOINT	*pa = m_viewPoints;
	int		x, y;

for (int i = 0; i < m_mine->VertCount (); i++, pa++) {
	x = pa->x;
	y = pa->y;
	if (BETWEEN(m_clickPos.x, x, m_releasePos.x) &&
		 BETWEEN(m_clickPos.y, y, m_releasePos.y)) {
		if (m_clickState & MK_SHIFT)
			*m_mine->VertStatus (i) &= ~MARKED_MASK;
		else
			*m_mine->VertStatus (i) |= MARKED_MASK;
		m_bUpdate = true;
		}
	}
if (m_bUpdate) {
	m_mine->UpdateMarkedCubes ();
	Refresh ();
	}
}

//==========================================================================
//==========================================================================
void CMineView::RefreshObject(INT16 old_object, INT16 new_object) 
{
if (!GetMine ())
	return;

m_mine->Current ()->object = new_object;
theApp.ToolView ()->Refresh ();
Refresh (false);
}

//--------------------------------------------------------------------------
//		       select_current_object()
//
//  ACTION - finds object pointed to by mouse then draws.
//
//--------------------------------------------------------------------------

void CMineView::SelectCurrentObject(long mouse_x, long mouse_y) 
{
if (!GetMine ())
	return;

CDObject *obj;
INT16 closest_object;
INT16 i;
double radius,closest_radius;
APOINT pt;
CDObject temp_obj;

// default to object 0 but set radius very large
closest_object = 0;
closest_radius = 1.0E30;

// if there is a secret exit, then enable it in search
int enable_secret = FALSE;
if (file_type != RDL_FILE)
	for(i=0;i<(INT16)m_mine->GameInfo ().triggers.count;i++)
		if (m_mine->Triggers () [i].type ==TT_SECRET_EXIT) {
			enable_secret = TRUE;
			break;
			}

for (i=0;i<=m_mine->GameInfo ().objects.count;i++) {
	BOOL drawable = FALSE;
	// define temp object type and position for secret object selection
	if (i == m_mine->GameInfo ().objects.count && file_type != RDL_FILE && enable_secret) {
		obj = &temp_obj;
		obj->type = OBJ_PLAYER;
		// define obj->position
		CalcSegmentCenter(obj->pos,(UINT16)m_mine->SecretCubeNum ());
		}
	else
		obj = m_mine->Objects (i);
#if 0
	switch(obj->type) {
		case OBJ_WEAPON:
			if (ViewObject (eViewObjectsPowerups | eViewObjectsWeapons)) {
				drawable = TRUE;
				}
		case OBJ_POWERUP:
			if (ViewObject (powerup_types [obj->id])) {
				drawable = TRUE;
				}
			break;
		default:
			if(ViewObject (1<<obj->type))
				drawable = TRUE;
		}
	if (drawable) 
#else
	if (ViewObject (obj))
#endif
		{
		// translate object's position to screen coordinates
		m_matrix.SetPoint(&obj->pos, &pt);
		// calculate radius^2 (don't bother to take square root)
		double dx = (double)pt.x - (double)mouse_x;
		double dy = (double)pt.y - (double)mouse_y;
		radius = dx * dx + dy * dy;
	// check to see if this object is closer than the closest so far
		if (radius < closest_radius) {
			closest_object = i;
			closest_radius = radius;
			}
		}
	}

// unhighlight current object and select next object
i = m_mine->Current ()->object;
RefreshObject(i, closest_object);
}

//--------------------------------------------------------------------------
//		       select_current_segment()
//
//  ACTION - finds segment pointed to by mouse then draws.  Segment must have
//         all points in screen region.
//
//  INPUT  - direction: must be a 1 or a -1
//
//--------------------------------------------------------------------------
void CMineView::SelectCurrentSegment(INT16 direction, long mouse_x, long mouse_y) 
{
if (!GetMine ())
	return;

  CDSegment *seg;
  CRect		rc;
//  extern INT16 mouse_x,mouse_y;
  INT8 flag1,flag2,flag3,flag4;
  INT16 cur_segment, next_segment;
  INT16 i;

/* find next segment which is within the cursor position */
GetClientRect (rc);
next_segment = cur_segment = m_mine->Current ()->segment;
do {
	wrap(&next_segment,direction,0,m_mine->SegCount ()-1); /* point to next segment */
	//  seg = calc_far_ptr(&Segments () [0],(INT32)((INT32)(next_segment) * (INT32)sizeof(segment)));
	seg = m_mine->Segments () + next_segment;
	if (ViewOption (eViewNearbyCubeLines) && !(seg->map_bitmask&0x1000))
		continue;
	flag1 = flag2 = flag3 = flag4 = 0;
	for (i=0;i<8;i++) {
		if ((m_viewPoints [seg->verts [i]].x <= rc.right) && (m_viewPoints [seg->verts [i]].x >= rc.left))
			if (mouse_x > m_viewPoints [seg->verts [i]].x)
				flag1 = 1;
			else
				flag2 = 1;
		if ((m_viewPoints [seg->verts [i]].y <= rc.bottom) && (m_viewPoints [seg->verts [i]].y >= rc.top))
			if (mouse_y > m_viewPoints [seg->verts [i]].y)
				flag3 = 1;
			else
				flag4 = 1;
		}
	if (flag1 && flag2 && flag3 && flag4) 
		break;
	}
while (next_segment != cur_segment);
m_mine->Current ()->segment = next_segment;
theApp.ToolView ()->Refresh ();
Refresh ();
}

//-------------------------------------------------------------------------
// calculate_segment_center()
//-------------------------------------------------------------------------

void CMineView::CalcSegmentCenter(vms_vector &pos,INT16 segnum) 
{
if (!GetMine ())
	return;

CDSegment *seg = m_mine->Segments () + segnum;
vms_vector *vMine = m_mine->Vertices ();
INT16 *vSeg = seg->verts;
pos.x  =
   (vMine [vSeg [0]].x
   +vMine [vSeg [1]].x
   +vMine [vSeg [2]].x
   +vMine [vSeg [3]].x
   +vMine [vSeg [4]].x
   +vMine [vSeg [5]].x
   +vMine [vSeg [6]].x
   +vMine [vSeg [7]].x)/8;
pos.y  =
   (vMine [vSeg [0]].y
   +vMine [vSeg [1]].y
   +vMine [vSeg [2]].y
   +vMine [vSeg [3]].y
   +vMine [vSeg [4]].y
   +vMine [vSeg [5]].y
   +vMine [vSeg [6]].y
   +vMine [vSeg [7]].y)/8;
pos.z  =
   (vMine [vSeg [0]].z
   +vMine [vSeg [1]].z
   +vMine [vSeg [2]].z
   +vMine [vSeg [3]].z
   +vMine [vSeg [4]].z
   +vMine [vSeg [5]].z
   +vMine [vSeg [6]].z
   +vMine [vSeg [7]].z)/8;
}

                        /*--------------------------*/
                        
BOOL CMineView::DrawRubberBox ()
{
//CPaintDC dc (this);
      
if (m_mouseState != eMouseStateRubberBand)
	return FALSE;
if (m_rubberRect.Width () || m_rubberRect.Height ()) {
      CPen     pen (PS_DOT, 1, RGB (0,0,0));
      CPen *   pOldPen;
      POINT    rubberPoly [5];
   
   m_pDC->SetROP2 (R2_XORPEN);
   pOldPen = m_pDC->SelectObject (&pen);
   rubberPoly [0].x = m_rubberRect.left + RUBBER_BORDER;
   rubberPoly [0].y = m_rubberRect.top + RUBBER_BORDER;
   rubberPoly [1].x = m_rubberRect.right - RUBBER_BORDER;
   rubberPoly [1].y = m_rubberRect.top + RUBBER_BORDER;
   rubberPoly [2].x = m_rubberRect.right - RUBBER_BORDER;
   rubberPoly [2].y = m_rubberRect.bottom - RUBBER_BORDER;
   rubberPoly [3].x = m_rubberRect.left + RUBBER_BORDER;
   rubberPoly [3].y = m_rubberRect.bottom - RUBBER_BORDER;
   rubberPoly [4] = rubberPoly [0];
   m_pDC->Polyline (rubberPoly, sizeof (rubberPoly) / sizeof (POINT));
   m_pDC->SetROP2 (R2_COPYPEN);
   m_pDC->SelectObject (pOldPen);
   }
return TRUE;
}                        
                        
                        /*--------------------------*/
                        
void CMineView::UpdateRubberRect (CPoint pt)
{
if (m_mouseState == eMouseStateZoom)
	return;
if (m_mouseState == eMouseStateDrag)
	return;
if (m_mouseState == eMouseStateButtonDown)
   SetCapture ();
else {
   InvalidateRect (&m_rubberRect, FALSE);
   UpdateWindow ();
   }
if (m_clickPos.x < pt.x) {
   m_rubberRect.left = m_clickPos.x - RUBBER_BORDER;
   m_rubberRect.right = pt.x + RUBBER_BORDER;
   }
else {
   m_rubberRect.right = m_clickPos.x + RUBBER_BORDER;
   m_rubberRect.left = pt.x - RUBBER_BORDER;
   }
if (m_clickPos.y < pt.y) {
   m_rubberRect.top = m_clickPos.y - RUBBER_BORDER;
   m_rubberRect.bottom = pt.y + RUBBER_BORDER;
   }
else {
   m_rubberRect.bottom = m_clickPos.y + RUBBER_BORDER;
   m_rubberRect.top = pt.y - RUBBER_BORDER;
   }
SetMouseState (eMouseStateRubberBand);
InvalidateRect (&m_rubberRect, FALSE);
UpdateWindow ();
}                        
                        
                        /*--------------------------*/
                        
void CMineView::ResetRubberRect ()
{
ReleaseCapture ();
InvalidateRect (&m_rubberRect, FALSE);
UpdateWindow ();
m_rubberRect.left = m_rubberRect.right =
m_rubberRect.top = m_rubberRect.bottom = 0;
}

                        /*--------------------------*/
                        
BOOL CMineView::UpdateDragPos ()
{
if (!GetMine ())
	return FALSE;
if ((m_mouseState != eMouseStateInitDrag) && (m_mouseState != eMouseStateDrag))
	return FALSE;

	INT16 nVert = side_vert [m_mine->Current ()->side] [m_mine->Current ()->point];
	INT16 v = m_mine->Segments () [m_mine->Current ()->segment].verts [nVert];
	INT16 x = m_viewPoints [v].x;
	INT16 y = m_viewPoints [v].y;

if (m_mouseState == eMouseStateInitDrag) {
	SetMouseState (eMouseStateDrag);
// SetCapture ();
	m_highlightPos.x = x;
	m_highlightPos.y = y;
	m_lastDragPos = m_highlightPos;
	}
HighlightDrag (m_mine, nVert, x, y);
	
//InvalidateRect (NULL, TRUE);
return TRUE;
}

                        /*--------------------------*/
                        
void CMineView::HighlightDrag (CMine *mine, INT16 nVert, long x, long y) 
{
m_pDC->SelectObject((HBRUSH) GetStockObject (NULL_BRUSH));
//m_pDC->SetROP2 (R2_NOT);
m_pDC->SetROP2 (R2_NOT);
m_pDC->Ellipse (x-4, y-4, x+4, y+4);

CRect	rc (x, y, x, y);
for (int i = 0; i < 3; i++) {
	m_pDC->MoveTo (x, y);
	INT16 nVert2 = connect_points [nVert] [i];
	INT16 x2 = m_viewPoints [mine->Segments () [mine->Current ()->segment].verts [nVert2]].x;
	INT16 y2 = m_viewPoints [mine->Segments () [mine->Current ()->segment].verts [nVert2]].y;
   m_pDC->LineTo (x2, y2);
	if (rc.left > x2)
		rc.left = x2;
	if (rc.right < x2)
		rc.right = x2;
	if (rc.top > y2)
		rc.top = y2;
	if (rc.bottom < y2)
		rc.bottom = y2;
	}
m_pDC->SetROP2 (R2_COPYPEN);
rc.InflateRect (4, 4);
InvalidateRect (rc, FALSE);
UpdateWindow ();
}

                        /*--------------------------*/
                        
BOOL CMineView::DrawDragPos (void)
{
if (!GetMine ())
	return FALSE;
if (m_mouseState != eMouseStateDrag)
	return FALSE;
if (m_lastMousePos == m_lastDragPos)
	return FALSE;

	INT16 nVert;
	INT16 x, y;
	int i;

nVert = side_vert [m_mine->Current ()->side] [m_mine->Current ()->point];

// unhighlight last point and lines drawing
HighlightDrag (m_mine, nVert, m_lastDragPos.x, m_lastDragPos.y);

// highlight the new position
HighlightDrag (m_mine, nVert, m_lastMousePos.x, m_lastMousePos.y);
m_lastDragPos = m_lastMousePos;

m_pDC->SetROP2 (R2_NOT);
for (i = 0; i < m_mine->VertCount (); i++) {
	x = m_viewPoints [i].x;
	y = m_viewPoints [i].y;
	if ((abs (x - m_lastMousePos.x) < 5) && (abs (y - m_lastMousePos.y) < 5)) {
		if ((x != m_highlightPos.x) || (y != m_highlightPos.y)) {
			if (m_highlightPos.x != -1)
				// erase last point
				m_pDC->Ellipse (m_highlightPos.x-8, m_highlightPos.y-8, m_highlightPos.x+8, m_highlightPos.y+8);
			// define and draw new point
			m_highlightPos.x = x;
			m_highlightPos.y = y;
			m_pDC->Ellipse (m_highlightPos.x-8, m_highlightPos.y-8, m_highlightPos.x+8, m_highlightPos.y+8);
			break;
			}
		}
	}
// if no point found near cursor
if ((i >= m_mine->VertCount ()) && (m_highlightPos.x != -1))
	// erase last point
	m_pDC->Ellipse (m_highlightPos.x-8, m_highlightPos.y-8, m_highlightPos.x+8, m_highlightPos.y+8);
m_pDC->SetROP2 (R2_COPYPEN);
// define and draw new point
m_highlightPos.x = -1;
m_highlightPos.y = -1;
return TRUE;
}

                        /*--------------------------*/
                        
void CMineView::FinishDrag (void)
{
//ReleaseCapture ();
if (!GetMine ())
	return;

	int		changes_made = 1;
	int		i, new_vert, count = 0;
	long		xPos,yPos;
	INT16		xPoint,yPoint;
	INT16		point1,vert1;
	INT16		point2,vert2;

xPos = m_releasePos.x;
yPos = m_releasePos.y;
point1 = side_vert [m_mine->Current ()->side] [m_mine->Current ()->point];
vert1 = m_mine->Segments () [m_mine->Current ()->segment].verts [point1];
// find point to merge with
for (i = 0; i < m_mine->VertCount (); i++) {
	xPoint = m_viewPoints [i].x;
	yPoint = m_viewPoints [i].y;
	if (abs(xPos - xPoint) < 5 && abs(yPos - yPoint)<5) {
		count++;
		new_vert = i;
		}
	}
// if too many matches found
if ((count > 1) && 
	 (QueryMsg("It is not clear which point you want to snap to."
				  "Do you want to attach these points anyway?") == IDYES))
	count = 1;
if (count == 1) {
// make sure new vert is not one of the current cube's verts
	for (i=0;i<8;i++) {
		if (i!=point1) {
			vert2 = m_mine->Segments () [m_mine->Current ()->segment].verts [i];
			if (new_vert==vert2) {
				ErrorMsg("Cannot drop point onto another corner of the current cube.");
				break;
				}
			}
		}
	if (i==8 && new_vert!=vert1) {
	// make sure the new line lengths are close enough
		for (i=0;i<3;i++) {
			point2 = connect_points [point1] [i];
			vert2 = m_mine->Segments () [m_mine->Current ()->segment].verts [point2];
			if (m_mine->CalcLength (m_mine->Vertices (new_vert),m_mine->Vertices (vert2)) >= 1000.0*(double)F1_0) {
				ErrorMsg("Cannot move this point so far away.");
				break;
				}
			}
		if (i==3) { //
			// replace origional vertex with new vertex
			m_mine->Segments () [m_mine->Current ()->segment].verts [point1] = new_vert;
			// all unused vertices
			m_mine->DeleteUnusedVertices();
			m_mine->FixChildren();
			m_mine->SetLinesToDraw();
			}
		}	
	}
else {
	// no vertex found, just drop point along screen axii
	APOINT apoint;
	apoint.x = (INT16) xPos;
	apoint.y = (INT16) yPos;
	apoint.z = m_viewPoints [vert1].z;
	m_matrix.UnsetPoint(m_mine->Vertices (vert1), &apoint);
	}
Refresh ();
}


//==========================================================================
// MENU - NextPoint
//==========================================================================
void CMineView::NextPoint(int dir) 
{
if (!GetMine ())
	return;

//if (!mine->SplineActive ())
//	DrawHighlight (mine, 1);
//if (m_selectMode==POINT_MODE)
wrap(&m_mine->Current ()->point,dir,0,4-1);
m_mine->Current ()->line = m_mine->Current ()->point;
Refresh ();
//SetSelectMode (POINT_MODE);
}

//==========================================================================
// MENU - PreviousPoint
//==========================================================================

void CMineView::PrevPoint()
{
NextPoint (-1);
}

//==========================================================================
// MENU - NextSide
//==========================================================================
void CMineView::NextSide (int dir) 
{
if (!GetMine ())
	return;

wrap(&m_mine->Current ()->side,dir,0,6-1);
Refresh ();
//SetSelectMode (SIDE_MODE);
}

//==========================================================================
// MENU - PreviousSide
//==========================================================================
void CMineView::PrevSide () 
{
NextSide (-1);
}

//==========================================================================
// MENU - NextSide2 (same except doesn't change mode)
//==========================================================================
void CMineView::NextSide2 (int dir)
{
if (!GetMine ())
	return;

wrap(&m_mine->Current ()->side,dir,0,6-1);
Refresh ();
}

void CMineView::PrevSide2 ()
{
NextSide2 (-1);
}

//==========================================================================
// MENU - NextLine
//==========================================================================

void CMineView::NextLine (int dir) 
{
if (!GetMine ())
	return;

wrap (&m_mine->Current ()->line,1,0,4-1);
m_mine->Current ()->point = m_mine->Current ()->line;
Refresh ();
//SetSelectMode (LINE_MODE);
}

//==========================================================================
// MENU - PreviousLine
//==========================================================================

void CMineView::PrevLine () 
{
NextLine (-1);
}

//==========================================================================
// MENU - NextCube
//==========================================================================

void CMineView::NextCube (int dir) 
{
if (!GetMine ())
	return;

if (m_mine->SegCount () <= 0)
	return;

if (0) {//!ViewOption (eViewPartialLines)) {
	DrawHighlight (m_mine, 1);
	//if (m_selectMode == CUBE_MODE)
		wrap (&m_mine->Current ()->segment,dir,0, m_mine->SegCount () - 1);
	Refresh ();
	//SetSelectMode (CUBE_MODE);
	DrawHighlight (m_mine, 0);
	}
else {
	//if (m_selectMode == CUBE_MODE)
		wrap (&m_mine->Current ()->segment, dir, 0, m_mine->SegCount () - 1);
	Refresh ();
	//SetSelectMode (CUBE_MODE);
	}
}

//==========================================================================
// MENU - PreviousCube
//==========================================================================

void CMineView::PrevCube () 
{
NextCube (-1);
}

//==========================================================================
// MENU - Forward_Cube
//
// ACTION - If child exists, this routine sets current_segment to child seg
//
// Changes - Smart side selection added (v0.8)
//         Smart side selection done before moving (instead of after) (v0.9)
//==========================================================================

void CMineView::ForwardCube (int dir) 
{
if (!GetMine ())
	return;

	CDSegment *seg,*childseg;
	INT16 child,sidenum;
	bool bFwd = (dir == 1);

DrawHighlight (m_mine, 1);
seg = m_mine->Segments () + m_mine->Current ()->segment;
child = seg->children [bFwd ? m_mine->Current ()->side : opp_side [m_mine->Current ()->side]];
if (child <= -1) {
	// first try to find a non backwards route
	for (sidenum=0;sidenum<6;sidenum++) {
		if (seg->children [sidenum] != m_lastSegment && seg->children [sidenum] > -1) {
			child = seg->children [sidenum];
			m_mine->Current ()->side =  bFwd ? sidenum : opp_side [sidenum];
			break;
			}
		}
	// then settle for any way out
	if (sidenum == 6) {
		for (sidenum=0;sidenum<6;sidenum++) {
			if (seg->children [sidenum] > -1) {
				child = seg->children [sidenum];
				m_mine->Current ()->side = bFwd ? sidenum : opp_side [sidenum];
				break;
				}
			}			
		}
	}
if (child > -1) {
	childseg = m_mine->Segments () + child;
// try to select side which is in same direction as current side
	for (sidenum=0;sidenum<6;sidenum++) {
		if (childseg->children [sidenum] == m_mine->Current ()->segment) {
			m_mine->Current ()->side =  bFwd ? opp_side [sidenum] : sidenum;
			break;
			}
		}
	m_lastSegment = m_mine->Current ()->segment;
	if (0) {//!ViewOption (eViewPartialLines)) {
		// DrawHighlight (m_mine, 1);
		m_mine->Current ()->segment = child;
		// DrawHighlight (m_mine, 0);
		} 
	else {
		m_mine->Current ()->segment = child;
		Refresh ();
		}
	}
DrawHighlight (m_mine, 0);
}

//==========================================================================
// MENU - Backwards_Cube
//==========================================================================

void CMineView::BackwardsCube () 
{
ForwardCube (-1);
}

//==========================================================================
// MENU - Other_Cube
//==========================================================================

void CMineView::SelectOtherCube() 
{
if (!GetMine ())
	return;

m_mine->Current () = (m_mine->Current () == &m_mine->Current1 ()) ? &m_mine->Current2 () : &m_mine->Current1 ();
Refresh ();
//theApp.ToolView ()->CubeTool ()->Refresh ();
}

bool CMineView::SelectOtherSide () 
{
if (!GetMine ())
	return false;

INT16 opp_segnum, opp_sidenum;

if (!m_mine->GetOppositeSide (opp_segnum, opp_sidenum))
	return false;

m_mine->Current ()->segment = opp_segnum;
m_mine->Current ()->side = opp_sidenum;
Refresh ();
//theApp.ToolView ()->CubeTool ()->Refresh ();
return true;
}

//==========================================================================
// MENU - NextObject
//==========================================================================

void CMineView::NextObject (int dir) 
{
if (!GetMine ())
	return;

  INT16 old_object = m_mine->Current ()->object;
  INT16 new_object = m_mine->Current ()->object;

//  DrawHighlight (m_mine, 1);
if (m_mine->GameInfo ().objects.count > 1) {
//	if (m_selectMode == OBJECT_MODE)
		wrap(&new_object,dir,0,(INT16)m_mine->GameInfo ().objects.count - 1) ;
	Refresh ();
	}
//SetSelectMode (OBJECT_MODE);
RefreshObject (old_object, new_object);
}

//==========================================================================
// MENU - PreviousObject
//==========================================================================

void CMineView::PrevObject() 
{
NextObject (-1);
}

//==========================================================================
// NextElement
//==========================================================================

void CMineView::NextCubeElement (int dir)
{
switch (m_selectMode) {
	case eSelectPoint:
		NextPoint (dir);
		break;
	case eSelectLine:
		NextLine (dir);
		break;
	default:
		NextSide (dir);
		break;
	}
}

                        /*--------------------------*/

void CMineView::PrevCubeElement ()
{
NextCubeElement (-1);
}

								/*---------------------------*/

BOOL CMineView::OnMouseWheel (UINT nFlags, short zDelta, CPoint pt)
{
	CRect	rc;

GetWindowRect (rc);
if ((pt.x < rc.left) || (pt.x >= rc.right) || (pt.y < rc.top) || (pt.y >= rc.bottom))
	return theApp.TextureView ()->OnMouseWheel (nFlags, zDelta, pt);
if (zDelta < 0)
	ZoomIn (-zDelta / WHEEL_DELTA);
else
	ZoomOut (zDelta / WHEEL_DELTA);
return 0;
}

                        /*--------------------------*/

void CMineView::OnSelectPrevTab ()
{
theApp.MainFrame ()->ShowTools ();
theApp.ToolView ()->PrevTab ();
}

                        /*--------------------------*/

void CMineView::OnSelectNextTab ()
{
theApp.MainFrame ()->ShowTools ();
theApp.ToolView ()->NextTab ();
}

                        /*--------------------------*/

#if OGL_RENDERING

// code partially from NeHe productions tutorials

                        /*--------------------------*/

	static float	uvMap [4][2] = {{0.0,0.0},{0.0,1.0},{1.0,1.0},{1.0,0.0}};
	static UINT8	rgbBuf [64*64*4];
	static GLuint	glHandles [910];
	static UINT8	*glPalette = NULL;
	static BOOL		glFitToView = FALSE;

BOOL CMineView::GLInit (GLvoid)
{
glShadeModel (GL_SMOOTH);
glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
glClearDepth (1.0f);
glEnable (GL_DEPTH_TEST);
glDepthFunc (GL_LEQUAL);
glEnable (GL_ALPHA_TEST);
glAlphaFunc (GL_GEQUAL, 0.5);	
glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
memset (glHandles, 0, sizeof (glHandles));		
return TRUE;
}

                        /*--------------------------*/

BOOL CMineView::GLInitPalette (GLvoid)
{
if (!glPalette) {
	HINSTANCE hInst = AfxGetInstanceHandle ();
	HRSRC hFind = FindResource (hInst, PaletteResource (), "RC_DATA");
	if (hFind) {
		HGLOBAL hGlobal = LoadResource (hInst, hFind);
		if (hGlobal)
			glPalette = (UINT8 *) LockResource (hGlobal);
		}
	}
return (glPalette != NULL);
}

                        /*--------------------------*/

GLvoid CMineView::GLFitToView (GLvoid)
{
CRect	rc;
SetViewPoints (&rc);

int zMin = LONG_MAX, zMax = LONG_MIN;
for (int i = m_mine->VertCount (); --i; ) {
	if (zMin > m_viewPoints [i].z)
		zMin = m_viewPoints [i].z;
	if (zMax < m_viewPoints [i].z)
		zMax = m_viewPoints [i].z;
	}
int rad = rc.Width ();
if (rad < rc.Height ())
	rad = rc.Height ();
if (rad < (zMax - zMin + 1))
	rad = (zMax - zMin + 1);
int xMid = (rc.left + rc.right) / 2;
int yMid = (rc.bottom + rc.top) / 2;
float left = (float) xMid - rad;
float right = (float) xMid + rad;
float bottom = (float) yMid - rad;
float top = (float) yMid + rad;
GetClientRect (rc);
float aspect = (float) rc.Width () / (float) rc.Height ();
if (aspect < 1.0) {
	bottom /= aspect;
	top /= aspect;
	}
else {
	left /= aspect;
	right /= aspect;
	}
glMatrixMode (GL_PROJECTION);
glLoadIdentity ();
glOrtho (left, right, bottom, top, 1.0, 1.0 + rad);
glMatrixMode (GL_MODELVIEW);
glLoadIdentity ();
gluLookAt (0.0, 0.0, 2.0 * rad, xMid, yMid, (zMin + zMax) / 2, 0.0, 1.0, 0.0);
glFitToView = TRUE;
}

                        /*--------------------------*/

BOOL CMineView::GLResizeScene (GLvoid) 
{
if (!GLCreateWindow ())
	return FALSE;

CRect rc;
	
GetClientRect (rc);
//ClientToScreen (rc);
if (!(rc.Width () && rc.Height ()))
	return FALSE;
glViewport (rc.left, rc.top, rc.Width (), rc.Height ());
if (glFitToView)
	glFitToView = FALSE;
else {
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	}
glMatrixMode (GL_PROJECTION);
glLoadIdentity ();
gluPerspective (90.0f, (GLfloat) rc.Width () / (GLfloat) rc.Height (), 0.1f, 1000000.0f);
glMatrixMode (GL_MODELVIEW);
glLoadIdentity ();
return TRUE;
}

                        /*--------------------------*/

void CMineView::GLCreateTexture (INT16 nTexture)
{
if (!GLInitPalette ())
	return;
INT16 iTexture = nTexture;// & ~0xC000; 
if (!glHandles [iTexture]) {
	CDTexture tx (bmBuf);
	DefineTexture (nTexture, 0, &tx, 0, 0);
	DrawAnimDirArrows (nTexture, &tx);
	// create RGBA bitmap from source bitmap
	int h, i, j;
	for (h = i = 0; i < 64*64; i++) {
		j = bmBuf [i];
		j *= 3;
		rgbBuf [h++] = glPalette [j++] << 2;
		rgbBuf [h++] = glPalette [j++] << 2;
		rgbBuf [h++] = glPalette [j++] << 2;
		rgbBuf [h++] = (bmBuf [i] >= 254) ? 0 : 255;
		}
	glGenTextures (1, glHandles + iTexture); 
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, glHandles [iTexture]); 
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbBuf);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}

                        /*--------------------------*/

void CMineView::GLRenderTexture (INT16 segnum, INT16 sidenum, INT16 nTexture)
{
	CDSegment *seg = m_mine->Segments (segnum);
	CDSide *side = seg->sides + sidenum;
	uvl *uvls;
	double l;
#if OGL_MAPPED
	APOINT *a;
#else
	vms_vector *verts = m_mine->Vertices ();
	vms_vector *v;
#endif
	static int rotOffs [4] = {0,3,2,1};
	int j = rotOffs [(nTexture & 0xC000) >> 14];

GLCreateTexture (nTexture);
glEnable (GL_TEXTURE_2D);
glBindTexture (GL_TEXTURE_2D, glHandles [nTexture/* & ~0xC000*/]); 
glBegin (GL_TRIANGLE_FAN);
for (int i = 0; i < 4; i++) {
	uvls = side->uvls + j;
	l = uvls->l / UV_FACTOR;
	glColor3d (l,l,l);
	//glTexCoord2f (uvMap [j][0], uvMap [j][1]); 
	glTexCoord2d (uvls->u / 2048.0, uvls->v / 2048.0); 
#if OGL_MAPPED
	a = m_viewPoints + seg->verts [side_vert [sidenum][i]];
	glVertex3f ((float) a->x, (float) a->y, (float) a->z);
#else
	v = verts + seg->verts [side_vert [sidenum][i]];
	glVertex3f (f2fl (v->x), f2fl (v->y), f2fl (v->z));
#endif
	j = (j + 1) % 4;
	}
glEnd ();
}

                        /*--------------------------*/

void CMineView::GLRenderFace (INT16 segnum, INT16 sidenum)
{
	CDSegment *seg = m_mine->Segments (segnum);
	CDSide *side = seg->sides + sidenum;
	vms_vector *verts = m_mine->Vertices ();
	UINT8 wallnum = seg->sides [sidenum].wall_num;

if (side->tmap_num < 0)
	return;
CDWall *pWall = (wallnum == 255) ? NULL : ((CDlcDoc*) GetDocument ())->m_mine->Walls (wallnum);
if ((seg->children [sidenum] > -1) &&
	 (!pWall || (pWall->type == WALL_OPEN) || ((pWall->type == WALL_CLOAKED) && !pWall->cloak_value)))
	return;
#if OGL_MAPPED
APOINT& p0 = m_viewPoints [seg->verts [side_vert [sidenum] [0]]];
APOINT& p1 = m_viewPoints [seg->verts [side_vert [sidenum] [1]]];
APOINT& p3 = m_viewPoints [seg->verts [side_vert [sidenum] [3]]];

vms_vector a,b;
a.x = p1.x - p0.x;
a.y = p1.y - p0.y;
b.x = p3.x - p0.x;
b.y = p3.y - p0.y;
if (a.x*b.y > a.y*b.x)
	return;
#else
vms_vector *p0 = verts + seg->verts [side_vert [sidenum] [0]];
vms_vector *p1 = verts + seg->verts [side_vert [sidenum] [1]];
vms_vector *p3 = verts + seg->verts [side_vert [sidenum] [3]];

vms_vector a,b;
a.x = p1->x - p0->x;
a.y = p1->y - p0->y;
b.x = p3->x - p0->x;
b.y = p3->y - p0->y;
if (a.x*b.y > a.y*b.x)
	return;
#endif
GLRenderTexture (segnum, sidenum, side->tmap_num);
if (side->tmap_num2)
	GLRenderTexture (segnum, sidenum, side->tmap_num2);
}

                        /*--------------------------*/

BOOL CMineView::GLRenderScene (GLvoid)	
{
if (!GLResizeScene ())
	return FALSE;
glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
glLoadIdentity ();
glScaled (m_sizex, m_sizey, m_sizez);
glTranslated (-m_movex, -m_movey, -m_movez - 1000);	
for (INT16 segnum = m_mine->SegCount (); segnum--; )
	for (INT16 sidenum = 0; sidenum < MAX_SIDES_PER_SEGMENT; sidenum++)
		GLRenderFace (segnum, sidenum);
return TRUE;
}

                        /*--------------------------*/

GLvoid CMineView::GLReset (GLvoid)
{
glDeleteTextures (910, glHandles);
memset (glHandles, 0, sizeof (glHandles));
glPalette = NULL;
}

                        /*--------------------------*/

GLvoid CMineView::GLKillWindow (GLvoid)
{
GLReset ();
if (IsWindow (m_hWnd)) {
	if (m_glRC)	{
		if (!wglMakeCurrent (NULL, NULL))
			ErrorMsg ("OpenGL: Release of DC and RC failed.");
		if (!wglDeleteContext (m_glRC))
			ErrorMsg ("OpenGL: Release of rendering context failed.");
		m_glRC = NULL;
		}
	if (m_glDC) {
		if (!ReleaseDC (m_glDC))
			//ErrorMsg ("OpenGL: Release of device context failed.")
			;
		m_glDC = NULL;	
		}
	}
}

                        /*--------------------------*/

BOOL CMineView::GLCreateWindow (CDC *pDC)
{
if (m_glDC)
	return TRUE;

	GLuint PixelFormat;

static PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),
	1,	
	PFD_DRAW_TO_WINDOW |	PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA,
	8,
	0, 0, 0, 0, 0, 0,
	0,	
	0,
	0,
	0, 0, 0, 0,
	16,
	0,
	0,
	PFD_MAIN_PLANE,
	0,
	0, 0, 0
	};

if (!(m_glDC = pDC ? pDC : GetDC ())) {
	GLKillWindow ();
	ErrorMsg ("OpenGL: Can't create device context.");
	return FALSE;
	}
if (!(PixelFormat = ChoosePixelFormat (m_glDC->m_hDC, &pfd))) {
	GLKillWindow ();
	sprintf (message, "OpenGL: Can't find a suitable pixel format. (%d)", GetLastError ());
	ErrorMsg (message);
	return FALSE;
	}
if(!SetPixelFormat (m_glDC->m_hDC, PixelFormat, &pfd)) {
		GLKillWindow();
		sprintf (message, "OpenGL: Can't set the pixel format (%d).", GetLastError ());
		ErrorMsg (message);
		return FALSE;
	}
if (!(m_glRC = wglCreateContext (m_glDC->m_hDC))) {
	GLKillWindow ();
	sprintf (message, "OpenGL: Can't create a rendering context (%d).", GetLastError ());
	ErrorMsg (message);
	return FALSE;
	}

if(!wglMakeCurrent (m_glDC->m_hDC, m_glRC)) {
	GLKillWindow ();
	sprintf (message, "OpenGL: Can't activate the rendering context (%d).", GetLastError ());
	ErrorMsg (message);
	return FALSE;
	}
if (!GLInit ())	{
	GLKillWindow ();
	ErrorMsg ("OpenGL: Initialization failed.");
	return FALSE;
	}
return TRUE;
}

                        /*--------------------------*/

#endif //OGL_RENDERING

//eof