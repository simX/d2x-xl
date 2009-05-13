// dlcView.cpp : implementation of the CMineView class
//

#include "stdafx.h"
#include "dlc.h"

#include "dlcDoc.h"
#include "dlcView.h"

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

#define TEXTURE_SCALE	2

static UINT8 test_bitmap[64*64];

/////////////////////////////////////////////////////////////////////////////
// CMineView

IMPLEMENT_DYNCREATE(CToolView, CWnd)
BEGIN_MESSAGE_MAP(CToolView, CWnd)
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CMineView, CView)

BEGIN_MESSAGE_MAP(CMineView, CView)
	//{{AFX_MSG_MAP(CMineView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

BOOL CMineView::PreCreateWindow(CREATESTRUCT& cs)
{
return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMineView construction/destruction

CMineView::CMineView()
{
	m_viewWidth = m_viewHeight = m_viewDepth = 0;	// force OnDraw to initialize these
	m_bUpdate = true;
	m_viewObjects = eViewObjectsAll;
	m_viewFlags = eViewMineHiRes | eViewMineLights | eViewMineShading | eViewMineWalls;
	m_viewOption = eViewAllLines;

	m_PenCyan    = new CPen(PS_SOLID, 1, RGB(  0,255,255));
	m_PenRed     = new CPen(PS_SOLID, 1, RGB(255,  0,  0));
	m_PenGray    = new CPen(PS_SOLID, 1, RGB(128,128,128));
	m_PenLtGray  = new CPen(PS_SOLID, 1, RGB(160,160,160));
	m_PenGreen   = new CPen(PS_SOLID, 1, RGB(  0,255,  0));
	m_PenDkGreen = new CPen(PS_SOLID, 1, RGB(  0,128,  0));
	m_PenDkCyan  = new CPen(PS_SOLID, 1, RGB(  0,128,128));
	m_PenBlue	 = new CPen(PS_SOLID, 1, RGB(  0,  0,255));
	m_PenYellow  = new CPen(PS_SOLID, 1, RGB(255,196,  0));
	m_PenOrange  = new CPen(PS_SOLID, 1, RGB(255,128,  0));
	m_PenMagenta = new CPen(PS_SOLID, 1, RGB(255,  0,255));

	m_MouseState  = eMouseStateIdle;

	m_selectMode = SIDE_MODE;

#ifndef M_PI
#define M_PI 3.1415926f
#endif
	m_x0 = 0;
	m_y0 = 0;
	m_z0 = 0;
	m_spinx = M_PI/4.f;
	m_spiny = M_PI/4.f;
	m_spinz = 0.0;
	m_movex = 0.0f;
	m_movey = 0.0f;
	m_movez = 0.0f;
	m_sizex = 2.0f;
	m_sizey = 2.0f;
	m_sizez = 2.0f;
	m_DepthPerception = 100.0f;

	// calculate transformation m_matrix based on move, size, and spin
	m_matrix.Set(m_movex, m_movey, m_movez,
		m_sizex, m_sizey, m_sizez,
		m_spinx, m_spiny, m_spinz);


	for (int i=0; i<64*64; i++)
		test_bitmap[i] = (UINT8)i;
}

CMineView::~CMineView()
{
	delete		m_PenCyan;
	delete		m_PenRed;
	delete		m_PenGray;
	delete		m_PenGreen;
	delete		m_PenDkGreen;
	delete		m_PenDkCyan;
	delete		m_PenLtGray;
	delete		m_PenYellow;
	delete		m_PenOrange;
	delete		m_PenMagenta;
}

/////////////////////////////////////////////////////////////////////////////
// CMineView drawing

void CMineView::OnDraw(CDC* pViewDC)
{
	CDlcDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;
	if (!pDoc->m_mine) return;
	CMine *mine = *pDoc->m_mine;
	bool bPartial = false;

	// Initialize/Reinitialize the View's device context
	// and other handy variables
	InitView(pViewDC);

	if (m_bUpdate)
	{
		// Clear the View
		ClearView();

		// Calculate m_matrix M based on IM and "move x,y,z"
		m_matrix.Calculate(m_movex, m_movey, m_movez);

		// Set view m_matrix misc. information
		m_matrix.SetViewInfo(m_DepthPerception, m_viewWidth, m_viewHeight);

		// Transform points
		UINT32 i;
		for (i=0;i<mine->numVertices;i++) {
			m_matrix.SetPoint(mine->vertices[i], m_viewPoints[i]);
		}

		// make a local copy the mine's selection
		m_Current = mine->current;

		// draw the level
		switch(m_viewOption)
		{
			case eViewAllLines:
				DrawWireFrame(mine, false);
			break;

			case eViewHideLines:
				DrawWireFrame(mine, false); // temporary
			break;

			case eViewNearbyCubeLines:
				DrawWireFrame(mine, false); // temporary
			break;

			case eViewPartialLines:
				DrawWireFrame(mine, bPartial = true);
			break;

			case eViewPartialTextureMapped:
				DrawTextureMappedCubes(mine);
				DrawWireFrame(mine, bPartial = true); // temporary
			break;

			case eViewTextureMapped:
				DrawTextureMappedCubes(mine);
			break;
		}
	}
	if (m_viewFlags & eViewMineWalls)
		DrawWalls (mine);
	if (m_viewFlags & eViewMineLights)
		DrawLights (mine);
	if (m_viewObjects)
		DrawObjects (mine, bPartial);
	// if we are using our own DC, then copy it to the display
	if (m_DIB) {
		pViewDC->BitBlt(0,0,m_viewWidth, m_viewHeight, &m_DC, 0, 0, SRCCOPY);
	}
	m_bUpdate = false;
}

//----------------------------------------------------------------------------
// InitView()
//----------------------------------------------------------------------------
void CMineView::InitView(CDC* pViewDC)
{
	// if all else fails, then return the original device context
	m_pDC = pViewDC;

	// if view size is new, then reset dib and delete the current device context
	CRect Rect;
	GetClientRect(Rect);

//	int depth = m_pDC->GetDeviceCaps(BITSPIXEL) / 8;
	int depth = 1; // force 8-bit DIB

	int width = (Rect.Width() + 3) & ~3; // long word align
	int height = Rect.Height();

	if (width != m_viewWidth || height != m_viewHeight || depth != m_viewDepth) {
		m_bUpdate = true;

		if (m_DIB) {
			DeleteObject(m_DIB);
			m_DIB = 0;
		}
		if (m_DC.m_hDC) {
			m_DC.DeleteDC();
		}
	}
	m_viewWidth  = width;
	m_viewHeight = height;
	m_viewDepth  = depth;


	if (!m_DIB) {
		if (!m_DC.m_hDC) {
			m_DC.CreateCompatibleDC(pViewDC);
		}
		if (m_DC.m_hDC) {
			typedef struct {
				BITMAPINFOHEADER    bmiHeader;
				RGBQUAD             bmiColors[256];
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
			HRSRC hFind     = FindResource( hInst, MAKEINTRESOURCE(IDR_GROUPA_256), "RC_DATA");
			ASSERT(hFind);
			if (hFind)
			{
				HGLOBAL hGlobal = LoadResource( hInst, hFind);
				ASSERT(hGlobal);
				if (hGlobal)
				{
					UINT8* palette = (UINT8 *) LockResource(hGlobal);
					for (int i = 0; i < 256; ++i)
					{
						mybmi.bmiColors[i].rgbRed   = palette[i*3+0]<<2;
						mybmi.bmiColors[i].rgbGreen = palette[i*3+1]<<2;
						mybmi.bmiColors[i].rgbBlue  = palette[i*3+2]<<2;
						mybmi.bmiColors[i].rgbReserved = 0;
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
	INT32 segnum;
	m_pDC->SelectObject(m_PenGray);
	for (segnum=0;segnum<mine->numSegments;segnum++)
	{
		CDSegment& seg = mine->Segments ()[segnum];

		DrawCube(seg, bPartial);

		if (segnum == m_Current->segment)
		{
			DrawCurrentCube(seg, bPartial);
			m_pDC->SelectObject(m_PenGray);
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

void QSortCubes (pSegZOrder pszo, INT16 left, INT16 right)
{
	INT32		zMedian = pszo [(left + right) / 2].zMax;
	tSegZOrder	h;
	INT16	l = left, r = right;
do {
	while (pszo [l].zMax > zMedian)
		l++;
	while (pszo [r].zMax < zMedian)
		r--;
	if (l <= r) {
		if (l < r) {
			h = pszo [l];
			pszo [l] = pszo [r];
			pszo [r] = h;
			}
		l++;
		r--;
		}
	}
while (l < r);
if (l < right)
	QSortCubes (pszo, l, right);
if (left < r)
	QSortCubes (pszo, left, r);
}

void CMineView::DrawTextureMappedCubes(CMine *mine)
{
	UINT32 	segnum;
	INT16	 	iVertex;
	INT32	 	z, zMax;
	UINT8		*light_index;

	// Get shading table data
	if (m_viewMineFlags & eViewMineShading && (light_index = PalettePtr ()))
		light_index += 256*5; // skip 3-byte palette + 1st 2 light tables

	// Draw Segments ()
	pSegZOrder pszo = new tSegZOrder [mine->numSegments];
	for (segnum=0; segnum < mine->numSegments; segnum++)
	{
		pszo [segnum].iSeg = segnum;
		for (iVertex = 0, zMax = LONG_MIN; iVertex < MAX_VERTICES_PER_SEGMENT; iVertex++)
			if (zMax < (z = m_viewPoints [mine->Segments (segnum)->verts [iVertex]].z))
				zMax = z;
		pszo [segnum].zMax = zMax;
	}
	QSortCubes (pszo, 0, mine->numSegments - 1);
	for (segnum=0; segnum < mine->numSegments; segnum++)
 		DrawCubeTextured (mine->Segments (pszo [segnum].iSeg), light_index);
	delete [] pszo;
}

//--------------------------------------------------------------------------
// DrawCube()
//--------------------------------------------------------------------------
#define IN_RANGE(value,absolute_range) (-absolute_range <= value && value <= absolute_range)

void CMineView::DrawCube(CDSegment& seg, bool bPartial)
{

	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

	if (IN_RANGE(m_viewPoints[seg.verts[0]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[0]].y,y_max) &&
		IN_RANGE(m_viewPoints[seg.verts[1]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[1]].y,y_max) &&
		IN_RANGE(m_viewPoints[seg.verts[2]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[2]].y,y_max) &&
		IN_RANGE(m_viewPoints[seg.verts[3]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[3]].y,y_max) &&
		IN_RANGE(m_viewPoints[seg.verts[4]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[4]].y,y_max) &&
		IN_RANGE(m_viewPoints[seg.verts[5]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[5]].y,y_max) &&
		IN_RANGE(m_viewPoints[seg.verts[6]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[6]].y,y_max) &&
		IN_RANGE(m_viewPoints[seg.verts[7]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[7]].y,y_max)   )
	{
		if (bPartial)
		{
			UINT32 sidenum;
			for (sidenum=0; sidenum<6; sidenum++)
			{
				APOINT& p0 = m_viewPoints[seg.verts[side_vert[sidenum][0]]];
				APOINT& p1 = m_viewPoints[seg.verts[side_vert[sidenum][1]]];
				APOINT& p2 = m_viewPoints[seg.verts[side_vert[sidenum][2]]];
				APOINT& p3 = m_viewPoints[seg.verts[side_vert[sidenum][3]]];

				vms_vector a,b;
				a.x = p1.x - p0.x;
				a.y = p1.y - p0.y;
				b.x = p3.x - p0.x;
				b.y = p3.y - p0.y;

				if (a.x*b.y < a.y*b.x)
					m_pDC->SelectObject((HPEN)GetStockObject(WHITE_PEN));
				else
					m_pDC->SelectObject(m_PenGray);
				m_pDC->MoveTo(p0.x, p0.y);
				m_pDC->LineTo(p1.x, p1.y);
				m_pDC->LineTo(p2.x, p2.y);
				m_pDC->LineTo(p3.x, p3.y);
				m_pDC->LineTo(p0.x, p0.y);
			}
		}
		else
		{
			m_pDC->MoveTo(m_viewPoints[seg.verts[0]].x, m_viewPoints[seg.verts[0]].y);
			m_pDC->LineTo(m_viewPoints[seg.verts[1]].x, m_viewPoints[seg.verts[1]].y);
			m_pDC->LineTo(m_viewPoints[seg.verts[2]].x, m_viewPoints[seg.verts[2]].y);
			m_pDC->LineTo(m_viewPoints[seg.verts[3]].x, m_viewPoints[seg.verts[3]].y);

			m_pDC->MoveTo(m_viewPoints[seg.verts[0]].x, m_viewPoints[seg.verts[0]].y);
			m_pDC->LineTo(m_viewPoints[seg.verts[3]].x, m_viewPoints[seg.verts[3]].y);
			m_pDC->LineTo(m_viewPoints[seg.verts[7]].x,  m_viewPoints[seg.verts[7]].y);

			m_pDC->MoveTo(m_viewPoints[seg.verts[0]].x, m_viewPoints[seg.verts[0]].y);
			m_pDC->LineTo(m_viewPoints[seg.verts[4]].x, m_viewPoints[seg.verts[4]].y);
			m_pDC->LineTo(m_viewPoints[seg.verts[5]].x, m_viewPoints[seg.verts[5]].y);
			m_pDC->LineTo(m_viewPoints[seg.verts[6]].x, m_viewPoints[seg.verts[6]].y);
			m_pDC->LineTo(m_viewPoints[seg.verts[7]].x, m_viewPoints[seg.verts[7]].y);

			m_pDC->MoveTo(m_viewPoints[seg.verts[4]].x, m_viewPoints[seg.verts[4]].y);
			m_pDC->LineTo(m_viewPoints[seg.verts[7]].x, m_viewPoints[seg.verts[7]].y);

			m_pDC->MoveTo(m_viewPoints[seg.verts[2]].x, m_viewPoints[seg.verts[2]].y);
			m_pDC->LineTo(m_viewPoints[seg.verts[6]].x, m_viewPoints[seg.verts[6]].y);

			m_pDC->MoveTo(m_viewPoints[seg.verts[1]].x, m_viewPoints[seg.verts[1]].y);
			m_pDC->LineTo(m_viewPoints[seg.verts[5]].x, m_viewPoints[seg.verts[5]].y);
		}
	}
}


//--------------------------------------------------------------------------
// DrawCubeTextured()
//--------------------------------------------------------------------------
void CMineView::DrawCubeTextured(CDSegment& seg, UINT8* light_index) {

	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

	if (IN_RANGE(m_viewPoints[seg.verts[0]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[0]].y,y_max) &&
		IN_RANGE(m_viewPoints[seg.verts[1]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[1]].y,y_max) &&
		IN_RANGE(m_viewPoints[seg.verts[2]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[2]].y,y_max) &&
		IN_RANGE(m_viewPoints[seg.verts[3]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[3]].y,y_max) &&
		IN_RANGE(m_viewPoints[seg.verts[4]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[4]].y,y_max) &&
		IN_RANGE(m_viewPoints[seg.verts[5]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[5]].y,y_max) &&
		IN_RANGE(m_viewPoints[seg.verts[6]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[6]].y,y_max) &&
		IN_RANGE(m_viewPoints[seg.verts[7]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[7]].y,y_max)   )
	{

		int resolution = 0;
		UINT8 *bitmapBuffer = test_bitmap;
		UINT8 *pScrnMem = (UINT8 *)m_pvBits;
		APOINT* scrn = m_viewPoints;
		UINT16 width = m_viewWidth;
		UINT16 height = m_viewHeight;
		UINT16 rowOffset = (m_viewWidth + 3) & ~3;
		UINT16 sidenum = 5;
		CDWall *pWall;
		UINT16 wallnum = NO_WALL;
		for (sidenum=0; sidenum<6; sidenum++)
		{
//			if (seg.wall_bitmask & (1 << sidenum))
//				_asm int 3;
			pWall = ((wallnum = seg.sides [sidenum].wall_num) == NO_WALL) ? NULL : ((CDlcDoc*) GetDocument ())->m_mine->Walls () + wallnum;
			if ((seg.children[sidenum] == -1) ||
				(pWall && (pWall->type != WALL_OPEN) && ((pWall->type != WALL_CLOAKED) || pWall->cloak_value))
				)
			{
				APOINT& p0 = m_viewPoints[seg.verts[side_vert[sidenum][0]]];
				APOINT& p1 = m_viewPoints[seg.verts[side_vert[sidenum][1]]];
				APOINT& p3 = m_viewPoints[seg.verts[side_vert[sidenum][3]]];

				vms_vector a,b;
				a.x = p1.x - p0.x;
				a.y = p1.y - p0.y;
				b.x = p3.x - p0.x;
				b.y = p3.y - p0.y;
				if (a.x*b.y > a.y*b.x) {
					CDTexture tx (bmBuf);
					INT16 texture1 = seg.sides[sidenum].tmap_num;
					INT16 texture2 = seg.sides[sidenum].tmap_num2;
					if (!DefineTexture(texture1,texture2, &tx,0,0))
						TextureMap (resolution, seg, sidenum, tx.data, tx.width, tx.height, 
										light_index, pScrnMem, scrn, width, height, rowOffset);
				}
			}
		}
	}
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

void CMineView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	m_bUpdate = true;
}


void CMineView::ZoomIn()
{
	int zoom_factor;
	double zoom = log(10*m_sizex) / log(1.2);
	if (zoom > 0) zoom_factor = (int)(zoom + 0.5);
	else          zoom_factor = (int)(zoom - 0.5);
	if (zoom_factor < 25) {
		m_sizex*=1.2f;
		m_sizey*=1.2f;
		m_sizez*=1.2f;
		m_matrix.Scale(1.f/1.2f);
	} else {
		ErrorMsg("Already at maximum zoom\n\n"
			"Hint: Try using the 'A' and 'Z' keys to\n"
			"move in forward and backwards.");
	}
	m_bUpdate = TRUE;
	InvalidateRect(NULL, FALSE);
}

void CMineView::ZoomOut()
{

	int zoom_factor;
	double zoom = log(10*m_sizex) / log(1.2);
	if (zoom > 0) zoom_factor = (int)(zoom + 0.5);
	else          zoom_factor = (int)(zoom - 0.5);
	if (zoom_factor > -5) {
		m_sizex/=1.2f;
		m_sizey/=1.2f;
		m_sizez/=1.2f;
		m_matrix.Scale(1.2f);
	}
	m_bUpdate = TRUE;
	InvalidateRect(NULL, FALSE);
}

void CMineView::Rotate(char direction, double angle)
{
	m_matrix.Rotate(direction, angle);
	m_bUpdate = TRUE;
	InvalidateRect(NULL, FALSE);
}

void CMineView::Pan(char direction, INT32 value)
{
	INT32 index;
	switch(direction)
	{
		default:
		case 'X': index = 1; break;
		case 'Y': index = 2; break;
		case 'Z': index = 3; break;
	}

	m_movex -= value*m_matrix.IM[1][index];  /* move view point */
	m_movey -= value*m_matrix.IM[2][index];
	m_movez -= value*m_matrix.IM[3][index];

	m_bUpdate = TRUE;
	InvalidateRect(NULL, FALSE);
}

void CMineView::AlignSide()
{
}

void CMineView::CenterMine()
{
	CDlcDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;
	if (!pDoc->m_mine) return;
	CMine *mine = *pDoc->m_mine;

	INT32 maxx = 0;
	INT32 minx = 0;
	INT32 maxy = 0;
	INT32 miny = 0;
	INT32 maxz = 0;
	INT32 minz = 0;

	INT32 i;
	for (i=0;i<mine->numVertices;i++) {
		maxx = max(maxx, mine->vertices[i].x);
		minx = min(minx, mine->vertices[i].x);
		maxy = max(maxy, mine->vertices[i].y);
		miny = min(miny, mine->vertices[i].y);
		maxz = max(maxz, mine->vertices[i].z);
		minz = min(minz, mine->vertices[i].z);
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

	m_bUpdate = TRUE;
	InvalidateRect(NULL, FALSE);
}

void CMineView::CenterCube()
{
	CDlcDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;
	if (!pDoc->m_mine) return;
	CMine *mine = *pDoc->m_mine;

	CDSegment& seg = mine->Segments ()[m_Current->segment];

	m_movex = -((double)mine->vertices[seg.verts[0]].x
		 +(double)mine->vertices[seg.verts[1]].x
		 +(double)mine->vertices[seg.verts[2]].x
		 +(double)mine->vertices[seg.verts[3]].x
		 +(double)mine->vertices[seg.verts[4]].x
		 +(double)mine->vertices[seg.verts[5]].x
		 +(double)mine->vertices[seg.verts[6]].x
		 +(double)mine->vertices[seg.verts[7]].x)/(double)0x80000L;
	m_movey = -((double)mine->vertices[seg.verts[0]].y
		 +(double)mine->vertices[seg.verts[1]].y
		 +(double)mine->vertices[seg.verts[2]].y
		 +(double)mine->vertices[seg.verts[3]].y
		 +(double)mine->vertices[seg.verts[4]].y
		 +(double)mine->vertices[seg.verts[5]].y
		 +(double)mine->vertices[seg.verts[6]].y
		 +(double)mine->vertices[seg.verts[7]].y)/(double)0x80000L;
	m_movez = -((double)mine->vertices[seg.verts[0]].z
		 +(double)mine->vertices[seg.verts[1]].z
		 +(double)mine->vertices[seg.verts[2]].z
		 +(double)mine->vertices[seg.verts[3]].z
		 +(double)mine->vertices[seg.verts[4]].z
		 +(double)mine->vertices[seg.verts[5]].z
		 +(double)mine->vertices[seg.verts[6]].z
		 +(double)mine->vertices[seg.verts[7]].z)/(double)0x80000L;

	m_bUpdate = TRUE;
	InvalidateRect(NULL, FALSE);
}

void CMineView::CenterObject()
{
	CDlcDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;
	if (!pDoc->m_mine) return;
	CMine *mine = *pDoc->m_mine;

	CDObject& obj = mine->Objects ()[m_Current->object];
	m_movex = (INT16)(-(obj.pos.x)/0x10000L);
	m_movey = (INT16)(-(obj.pos.y)/0x10000L);
	m_movez = (INT16)(-(obj.pos.z)/0x10000L);

	m_bUpdate = TRUE;
	InvalidateRect(NULL, FALSE);
}

void CMineView::SetViewOption(eViewOptions option)
{
	m_viewOption = option;
	m_bUpdate = TRUE;
	InvalidateRect(NULL, FALSE);
}

void CMineView::ToggleViewMine(eViewMines flag)
{
	m_viewFlags ^= flag;
	m_bUpdate = TRUE;
	InvalidateRect(NULL, FALSE);
}

void CMineView::ToggleViewObjects(eViewObjectFlags mask)
{
	m_viewObjects ^= mask;
	m_bUpdate = TRUE;
	InvalidateRect(NULL, FALSE);
}

void CMineView::SetViewObjects(eViewObjectFlags mask)
{
	m_viewObjects = mask;
	m_bUpdate = TRUE;
	InvalidateRect(NULL, FALSE);
}

void CMineView::OnMouseMove(UINT nFlags, CPoint point)
{
	CPoint change = m_LastMousePoint - point;

	switch(m_MouseState)
	{
		case eMouseStateIdle:
			// if Control Key down
			if (nFlags & MK_CONTROL)
			{
				// if Shift then rotate
				if (nFlags & MK_SHIFT)
				{
					Rotate('Y', (double)change.x / 200.0f);
					Rotate('X', (double)change.y / 200.0f);
				}
				else
				// else move
				{
					Pan('X', change.x);
					Pan('Y', -change.y);
				}
			}
		break;

		case eMouseStateRubberBand:
		break;

	}
	CView::OnMouseMove(nFlags, point);

	m_LastMousePoint = point;
}

void CMineView::OnLButtonUp(UINT nFlags, CPoint point)
{

	CView::OnLButtonUp(nFlags, point);
}

void CMineView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CView::OnLButtonDown(nFlags, point);
}

void CMineView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	m_bUpdate = true;
	InvalidateRect(NULL);
}


//--------------------------------------------------------------------------
// DrawCurrentCube()
//--------------------------------------------------------------------------
void CMineView::DrawCurrentCube(CDSegment& seg, bool bPartial)
{
	INT16 sidenum = m_Current->side;
	INT16 linenum = m_Current->point;
	INT16 pointnum = m_Current->point;

	if (seg.wall_bitmask & MARKED_MASK) {
		m_pDC->SelectObject(m_PenCyan);
	}
	else
	{
		if (m_selectMode == CUBE_MODE)
		{
			m_pDC->SelectObject(m_PenRed);
		} else {
			m_pDC->SelectObject(GetStockObject(WHITE_PEN));
		}
	}

	// draw current side
	// must draw in same order as segment to avoid leftover pixels on screen
	if (m_selectMode == SIDE_MODE)
		m_pDC->SelectObject(m_PenRed);
	else
		m_pDC->SelectObject(m_PenGreen);

// Select this pen if this is the "other current" cube
//	m_pDC->SelectObject(m_PenDkGreen);

	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

	if (IN_RANGE(m_viewPoints[seg.verts[side_vert[sidenum][0]]].x,x_max) &&
		 IN_RANGE(m_viewPoints[seg.verts[side_vert[sidenum][0]]].y,y_max) &&
		 IN_RANGE(m_viewPoints[seg.verts[side_vert[sidenum][1]]].x,x_max) &&
		 IN_RANGE(m_viewPoints[seg.verts[side_vert[sidenum][1]]].y,y_max) &&
		 IN_RANGE(m_viewPoints[seg.verts[side_vert[sidenum][2]]].x,x_max) &&
		 IN_RANGE(m_viewPoints[seg.verts[side_vert[sidenum][2]]].y,y_max) &&
		 IN_RANGE(m_viewPoints[seg.verts[side_vert[sidenum][3]]].x,x_max) &&
		 IN_RANGE(m_viewPoints[seg.verts[side_vert[sidenum][3]]].y,y_max)    ) {

		DrawLine(seg, side_vert[sidenum][0], side_vert[sidenum][1]);
		DrawLine(seg, side_vert[sidenum][1], side_vert[sidenum][2]);
		DrawLine(seg, side_vert[sidenum][2], side_vert[sidenum][3]);
		DrawLine(seg, side_vert[sidenum][3], side_vert[sidenum][0]);
	}

	// draw current line
	// must draw in same order as segment to avoid leftover pixels on screen
	if (m_selectMode == LINE_MODE) { // && edit_mode != EDIT_OFF) {
		m_pDC->SelectObject(m_PenRed);  // RED
	} else {
		m_pDC->SelectObject(m_PenCyan);  // BLUE/CYAN
	}

	if (IN_RANGE(m_viewPoints[seg.verts[line_vert[side_line[sidenum][linenum]][0]]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[line_vert[side_line[sidenum][linenum]][0]]].y,y_max) &&
		IN_RANGE(m_viewPoints[seg.verts[line_vert[side_line[sidenum][linenum]][1]]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[line_vert[side_line[sidenum][linenum]][1]]].y,y_max)   ) {

		DrawLine(seg, line_vert[side_line[sidenum][linenum]][0],
			line_vert[side_line[sidenum][linenum]][1]);
	}

	// draw a circle around the current point
	m_pDC->SelectObject((HBRUSH)GetStockObject(NULL_BRUSH));
	if (m_selectMode == POINT_MODE) { //  && edit_mode != EDIT_OFF) {
		m_pDC->SelectObject(m_PenRed); // RED
	} else {
		m_pDC->SelectObject(m_PenCyan); // CYAN
	}

	if (IN_RANGE(m_viewPoints[seg.verts[side_vert[sidenum][pointnum]]].x,x_max) &&
		IN_RANGE(m_viewPoints[seg.verts[side_vert[sidenum][pointnum]]].y,y_max)     ) {

		m_pDC->Ellipse(m_viewPoints[seg.verts[side_vert[sidenum][pointnum]]].x - 4,
			m_viewPoints[seg.verts[side_vert[sidenum][pointnum]]].y - 4,
			m_viewPoints[seg.verts[side_vert[sidenum][pointnum]]].x + 4,
			m_viewPoints[seg.verts[side_vert[sidenum][pointnum]]].y + 4);
	}
}

//--------------------------------------------------------------------------
// DrawLine ()
//
// Action - draws a line starting with lowest vert
//--------------------------------------------------------------------------
void CMineView::DrawLine(CDSegment& seg,INT16 vert1,INT16 vert2) {
	if (vert2 > vert1) {
		m_pDC->MoveTo(m_viewPoints[seg.verts[vert1]].x,m_viewPoints[seg.verts[vert1]].y);
		m_pDC->LineTo(m_viewPoints[seg.verts[vert2]].x,m_viewPoints[seg.verts[vert2]].y);
	} else {
		m_pDC->MoveTo(m_viewPoints[seg.verts[vert2]].x,m_viewPoints[seg.verts[vert2]].y);
		m_pDC->LineTo(m_viewPoints[seg.verts[vert1]].x,m_viewPoints[seg.verts[vert1]].y);
	}
}
//--------------------------------------------------------------------------
// DrawWalls()
//--------------------------------------------------------------------------
void CMineView::DrawWalls(CMine *mine) {
	CDWall	*Walls () = mine->Walls ();
	CDSegment *Segments () = mine->Segments ();
	vms_vector	*vertices = mine->vertices;
	CDSegment	*seg;
	INT16 i,j;
	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;
	APOINT* scrn = m_viewPoints;

  for (i=0;i<mine->GameInfo ().walls_howmany;i++) {
	seg = Segments () + (int)Walls ()[i].segnum;
	switch(Walls ()[i].type) {
      case WALL_NORMAL    :
	m_pDC->SelectObject(m_PenLtGray);
	break;
      case WALL_BLASTABLE :
	m_pDC->SelectObject(m_PenLtGray);
	break;
	  case WALL_DOOR      :
	switch(Walls ()[i].keys) {
	  case KEY_NONE :
	    m_pDC->SelectObject(m_PenLtGray);
	    break;
	  case KEY_BLUE :
	    m_pDC->SelectObject(m_PenBlue);
		break;
	  case KEY_RED  :
	    m_pDC->SelectObject(m_PenRed);
	    break;
	  case KEY_GOLD :
	    m_pDC->SelectObject(m_PenYellow);
	    break;
	  default:
	 m_pDC->SelectObject(m_PenGray);
	}
	break;
      case WALL_ILLUSION  :
	m_pDC->SelectObject(m_PenLtGray);
	break;
      case WALL_OPEN      :
	m_pDC->SelectObject(m_PenLtGray);
	break;
	  case WALL_CLOSED    :
	m_pDC->SelectObject(m_PenLtGray);
	break;
      default:
	m_pDC->SelectObject(m_PenLtGray);
    }
	if (IN_RANGE(scrn[seg->verts[side_vert[(INT16)Walls ()[i].sidenum][0]]].x,x_max) &&
		IN_RANGE(scrn[seg->verts[side_vert[(INT16)Walls ()[i].sidenum][0]]].y,y_max) &&
		IN_RANGE(scrn[seg->verts[side_vert[(INT16)Walls ()[i].sidenum][1]]].x,x_max) &&
		IN_RANGE(scrn[seg->verts[side_vert[(INT16)Walls ()[i].sidenum][1]]].y,y_max) &&
		IN_RANGE(scrn[seg->verts[side_vert[(INT16)Walls ()[i].sidenum][2]]].x,x_max) &&
		IN_RANGE(scrn[seg->verts[side_vert[(INT16)Walls ()[i].sidenum][2]]].y,y_max) &&
		IN_RANGE(scrn[seg->verts[side_vert[(INT16)Walls ()[i].sidenum][3]]].x,x_max) &&
		IN_RANGE(scrn[seg->verts[side_vert[(INT16)Walls ()[i].sidenum][3]]].y,y_max)   ) {

      vms_vector center,orthog,vector;
      APOINT point;
      CalcCenter(mine, center,(INT16)Walls ()[i].segnum,(INT16)Walls ()[i].sidenum);
      CalcOrthoVector(mine, orthog,(INT16)Walls ()[i].segnum,(INT16)Walls ()[i].sidenum);
      vector.x = center.x - orthog.x;
      vector.y = center.y - orthog.y;
      vector.z = center.z - orthog.z;
		m_matrix.SetPoint(vector,point);
	  for (j=0;j<4;j++) {
		m_pDC->MoveTo(point.x,point.y);
		m_pDC->LineTo(scrn[seg->verts[side_vert[(INT16)Walls ()[i].sidenum][j]]].x,
		   scrn[seg->verts[side_vert[(INT16)Walls ()[i].sidenum][j]]].y);
      }
      if (Walls ()[i].trigger!= -1) {
	APOINT arrowstart_point,arrowend_point,arrow1_point,arrow2_point;
	vms_vector fin;

	// calculate arrow points
	vector.x = center.x - 3*orthog.x;
	vector.y = center.y - 3*orthog.y;
	vector.z = center.z - 3*orthog.z;
	m_matrix.SetPoint(vector,arrowstart_point);
	vector.x = center.x + 3*orthog.x;
	vector.y = center.y + 3*orthog.y;
	vector.z = center.z + 3*orthog.z;
	m_matrix.SetPoint(vector,arrowend_point);

	// direction toward center of line 0 from center

	vector.x  = vertices[seg->verts[side_vert[(INT16)Walls ()[i].sidenum][0]]].x;
	vector.x += vertices[seg->verts[side_vert[(INT16)Walls ()[i].sidenum][1]]].x;
	vector.x /= 2;
	vector.y  = vertices[seg->verts[side_vert[(INT16)Walls ()[i].sidenum][0]]].y;
	vector.y += vertices[seg->verts[side_vert[(INT16)Walls ()[i].sidenum][1]]].y;
	vector.y /= 2;
	vector.z  = vertices[seg->verts[side_vert[(INT16)Walls ()[i].sidenum][0]]].z;
	vector.z += vertices[seg->verts[side_vert[(INT16)Walls ()[i].sidenum][1]]].z;
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
	vector.x /= length;
	vector.y /= length;
	vector.z /= length;

	fin.x = center.x + 2*orthog.x + vector.x;
	fin.y = center.y + 2*orthog.y + vector.y;
	fin.z = center.z + 2*orthog.z + vector.z;
	m_matrix.SetPoint(fin,arrow1_point);

	fin.x = center.x + 2*orthog.x - vector.x;
	fin.y = center.y + 2*orthog.y - vector.y;
	fin.z = center.z + 2*orthog.z - vector.z;
	m_matrix.SetPoint(fin,arrow2_point);

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
  SelectObject(m_pDC, hPenLtGray);

  if (file_type != RDL_FILE) {
    dl_indices = (dl_index *)GlobalLock(hdl_indices);
    if (dl_indices) {
      delta_lights = (delta_light *)GlobalLock(hdelta_lights);
      if (delta_lights) {
	for (i=0;i<GameInfo ().dl_indices_howmany;i++) {
	  sidenum = dl_indices[i].sidenum;
	  segnum  = dl_indices[i].segnum;
	  draw_octagon(m_pDC,sidenum,segnum);
	  if (segnum == current->segment && sidenum == current->side) {
	    POINT light_source;
	    light_source = segment_center_xy(sidenum,segnum);
	    for (int j=0;j<dl_indices[i].count;j++) {
	      POINT light_dest;
	      int index = dl_indices[i].index+j;
	      sidenum = delta_lights[index].sidenum;
		  segnum  = delta_lights[index].segnum;
	      segment *seg = Segments ()[segnum];
	      light_dest = segment_center_xy(sidenum,segnum);
		  for (int k=0;k<4;k++)  {
		POINT corner;
		UINT8 l = delta_lights[index].vert_light[k];
		l = min(0x1f,l);
		l <<= 3;
		HPEN hPenLight = CreatePen(PS_SOLID, 1, RGB(l,l,255-l));
		SelectObject(m_pDC,hPenLight);
		corner.x = scrn[seg->verts[side_vert[sidenum][k]]].x;
		corner.y = scrn[seg->verts[side_vert[sidenum][k]]].y;
		corner.x = (corner.x + light_dest.x)>>1;
		corner.y = (corner.y + light_dest.y)>>1;
		MoveTo(m_pDC,light_source.x,light_source.y);
		LineTo(m_pDC,corner.x,corner.y);
		SelectObject(m_pDC, hPenLtGray);
		DeleteObject(hPenLight);
	      }
	    }
	  }
	}
	GlobalUnlock(hdelta_lights);
	  }
      GlobalUnlock(hdl_indices);
    }
  }
#endif
  // now show flickering lights
  m_pDC->SelectObject(m_PenYellow);

  // find flickering light from
  for (i=0;i<mine->N_flickering_lights;i++)
    DrawOctagon(mine, mine->flickering_lights[i].sidenum, mine->flickering_lights[i].segnum);
}

//------------------------------------------------------------------------
// DrawOctagon()
//------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void CMineView::DrawOctagon(CMine *mine, INT16 sidenum, INT16 segnum) {
	CDSegment *seg;
	INT16 j;
	APOINT *scrn = m_viewPoints;
	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

	if (segnum >=0 && segnum <=mine->numSegments && sidenum>=0 && sidenum<=5 ) {
	  POINT corners[4],center,line_centers[4],diamond[4],fortyfive[4];
	  seg = mine->Segments () + segnum;
	  for (j=0;j<4;j++) {
	    corners[j].x = scrn[seg->verts[side_vert[sidenum][j]]].x;
	    corners[j].y = scrn[seg->verts[side_vert[sidenum][j]]].y;
	  }
	  if (IN_RANGE(corners[0].x,x_max) && IN_RANGE(corners[0].y,y_max) &&
		   IN_RANGE(corners[1].x,x_max) && IN_RANGE(corners[1].y,y_max) &&
	      IN_RANGE(corners[2].x,x_max) && IN_RANGE(corners[2].y,y_max) &&
	      IN_RANGE(corners[3].x,x_max) && IN_RANGE(corners[3].y,y_max)   ) {

	    center.x = (corners[0].x + corners[1].x + corners[2].x + corners[3].x)>>2;
	    center.y = (corners[0].y + corners[1].y + corners[2].y + corners[3].y)>>2;
		for (j=0;j<4;j++) {
	      int k = (j+1) & 0x03;
			line_centers[j].x = (corners[j].x + corners[k].x) >> 1;
	      line_centers[j].y = (corners[j].y + corners[k].y) >> 1;
	      diamond[j].x = (line_centers[j].x + center.x) >> 1;
	      diamond[j].y = (line_centers[j].y + center.y) >> 1;
	      fortyfive[j].x = ((corners[j].x-center.x)*7)/20 + center.x;
	      fortyfive[j].y = ((corners[j].y-center.y)*7)/20 + center.y;
	    }
	    // draw octagon
	    m_pDC->MoveTo(diamond[3].x,diamond[3].y);
	    for (j=0;j<4;j++) {
	      m_pDC->LineTo(fortyfive[j].x,fortyfive[j].y);
		  m_pDC->LineTo(diamond[j].x,diamond[j].y);
	    }
	  }
	}
}

//--------------------------------------------------------------------------
//			  DrawObject()
//
// Changed: 0=normal,1=gray,2=black
//          if (objnum == (GameInfo ().object_howmany
//          then its a secret return point)
//--------------------------------------------------------------------------
void CMineView::DrawObject(CMine &mine,INT16 objnum,INT16 clear_it) {
  INT16 poly;
  CDObject *obj;
  vms_vector pt[MAX_POLY];
  APOINT poly_draw[MAX_POLY];
  APOINT object_shape[MAX_POLY] = {
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

  if (objnum >=0 && objnum < mine->GameInfo ().object_howmany) {
    obj = mine->Objects () + objnum;
  } else {
    // secret return
    obj = &temp_obj;
    obj->type = -1;

//    mine->secret_orient = Objects ()[0]->orient;

   obj->orient.rvec.x = -mine->m_secret_orient.rvec.x;
   obj->orient.rvec.y = -mine->m_secret_orient.rvec.y;
   obj->orient.rvec.z = -mine->m_secret_orient.rvec.z;
   obj->orient.uvec.x =  mine->m_secret_orient.fvec.x;
	obj->orient.uvec.y =  mine->m_secret_orient.fvec.y;
   obj->orient.uvec.z =  mine->m_secret_orient.fvec.z;
   obj->orient.fvec.x =  mine->m_secret_orient.uvec.x;
   obj->orient.fvec.y =  mine->m_secret_orient.uvec.y;
   obj->orient.fvec.z =  mine->m_secret_orient.uvec.z;

//    obj->orient =  mine->secret_orient;
	UINT16 segnum = (UINT16)mine->m_secret_cubenum;
	if (segnum > mine->numSegments) {
	  segnum = 0;
	}
	mine->CalcSegCenter(obj->pos,segnum); // define obj->position
  }

  switch (clear_it) {
	case 0: // normal
	case 1: // gray
//	  if (objnum==current->object) {
		if (m_selectMode == OBJECT_MODE
			&& objnum==mine->current->object) { // && edit_mode != EDIT_OFF) {
		  m_pDC->SelectObject(m_PenRed); // RED
		} else {
		  switch(obj->type) {
			case OBJ_ROBOT    : /* an evil enemy */
			  m_pDC->SelectObject(m_PenMagenta);
			  break;
			case OBJ_HOSTAGE  : /* a hostage you need to rescue */
			 m_pDC->SelectObject(m_PenBlue);
			 break;
			case OBJ_PLAYER   : /* the player on the console */
			  m_pDC->SelectObject(m_PenCyan);
			  break;
			case OBJ_WEAPON   : // exploding mine
			  m_pDC->SelectObject(m_PenDkGreen);
			  break;
			case OBJ_POWERUP  : /* a powerup you can pick up */
			  m_pDC->SelectObject(m_PenOrange);
			  break;
			case OBJ_CNTRLCEN : /* the control center */
			  m_pDC->SelectObject(m_PenLtGray);
			  break;
			case OBJ_COOP     : /* a cooperative player object */
			  m_pDC->SelectObject(m_PenCyan);
			  break;
			default:
			  m_pDC->SelectObject(m_PenGreen);
		//	      m_pDC->SelectObject(m_PenLtGray);
		  }
		}
//	  } else {
//		if (show_lines & SHOW_FILLED_POLYGONS) {
//		  m_pDC->SelectObject(hPenGray);
//		} else {
//		  m_pDC->SelectObject(hPenLtGray);
//		}
//	  }
	  break;
//	case 1: // gray
//	  if (show_lines & SHOW_FILLED_POLYGONS) {
//		m_pDC->SelectObject(hPenGray);
//	  } else {
//		m_pDC->SelectObject(hPenLtGray);
//	  }
//	  break;
	case 2: // black
	  m_pDC->SelectObject(GetStockObject(BLACK_PEN));
	  break;
  }

// rotate object shape using object's orient matrix
  for (poly=0;poly<MAX_POLY;poly++) {
	pt[poly].x = (  obj->orient.rvec.x *  (FIX)object_shape[poly].x
		  + obj->orient.uvec.x *  (FIX)object_shape[poly].y
		  + obj->orient.fvec.x *  (FIX)object_shape[poly].z);
	pt[poly].y = (  obj->orient.rvec.y *  (FIX)object_shape[poly].x
		  + obj->orient.uvec.y *  (FIX)object_shape[poly].y
		  + obj->orient.fvec.y *  (FIX)object_shape[poly].z);
	pt[poly].z = (  obj->orient.rvec.z *  (FIX)object_shape[poly].x
		  + obj->orient.uvec.z *  (FIX)object_shape[poly].y
		  + obj->orient.fvec.z *  (FIX)object_shape[poly].z);
  }

  // translate object
  for (poly=0;poly<MAX_POLY;poly++) {
	pt[poly].x += obj->pos.x;
	pt[poly].y += obj->pos.y;
	pt[poly].z += obj->pos.z;
	m_matrix.SetPoint(pt[poly],poly_draw[poly]);
  }

  // figure out world coordinates

  if (IN_RANGE(poly_draw[0].x,x_max) &&
	  IN_RANGE(poly_draw[0].y,y_max) &&
	  IN_RANGE(poly_draw[1].x,x_max) &&
	  IN_RANGE(poly_draw[1].y,y_max) &&
	  IN_RANGE(poly_draw[2].x,x_max) &&
	  IN_RANGE(poly_draw[2].y,y_max) &&
	  IN_RANGE(poly_draw[3].x,x_max) &&
	  IN_RANGE(poly_draw[3].y,y_max) &&
	  IN_RANGE(poly_draw[4].x,x_max) &&
	  IN_RANGE(poly_draw[4].y,y_max) &&
	  IN_RANGE(poly_draw[5].x,x_max) &&
	  IN_RANGE(poly_draw[5].y,y_max)    ) {

	if (file_type != RDL_FILE
		&& objnum == mine->current->object
		&& obj->render_type == RT_POLYOBJ
		&& SetupModel(mine,obj)==0) {
	  if (!clear_it) {
//		m_pDC->SelectObject(hPenLtGray);
//		m_pDC->SelectObject((HBRUSH)GetStockObject(GRAY_BRUSH));
		m_pDC->SelectObject((HBRUSH)GetStockObject(BLACK_BRUSH));
	  } else {
		m_pDC->SelectObject(GetStockObject(BLACK_PEN));
		m_pDC->SelectObject((HBRUSH)GetStockObject(BLACK_BRUSH));
	  }
	  DrawModel();
	} else {
	  m_pDC->MoveTo(poly_draw[0].x,poly_draw[0].y);
	  for (poly=0;poly<6;poly++) {
		m_pDC->LineTo(poly_draw[poly].x,poly_draw[poly].y);
	  }
	  if (objnum==mine->current->object) {
		int dx,dy;
		for (dx=-1;dx<2;dx++) {
		  for (dy=-1;dy<2;dy++) {
			m_pDC->MoveTo(poly_draw[0].x+dx,poly_draw[0].y+dy);
			for (poly=0;poly<6;poly++) {
			  m_pDC->LineTo(poly_draw[poly].x+dx,poly_draw[poly].y+dy);
			}
		  }
		}
	  }
	}
  }
}
//--------------------------------------------------------------------------
//			  DrawObjects()
//--------------------------------------------------------------------------

void CMineView::DrawObjects (CMine &mine, bool bPartial) {
	INT16 i;

//if (!clear_it)
	{
	if (!((show_lines & SHOW_FILLED_POLYGONS) && bPartial))
		if (file_type != RDL_FILE)
			// see if there is a secret exit trigger
			for(i=0;i<(INT16)mine->GameInfo ().triggers_howmany;i++)
				if (mine->Triggers ()[i].type ==TT_SECRET_EXIT) {
					DrawObject(mine,(INT16)mine->GameInfo ().object_howmany,0);
					break; // only draw one secret exit
				}
	for (i=0;i<mine->GameInfo ().object_howmany;i++)
		switch(mine->Objects ()[i].type) {
			case OBJ_WEAPON:
				if (m_viewObjects & eViewObjectsWeapons)
					DrawObject(mine,i,0);
			case OBJ_POWERUP:
				if (m_viewObjects & powerup_types[mine->Objects ()[i].id])
					DrawObject(mine,i,0);
				break;
			default:
				if(m_viewObjects & (1<<mine->Objects ()[i].type))
					DrawObject(mine,i,0);
			}
  }
}

