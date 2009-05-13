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
#include "textures.h"
#include "io.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE [] = __FILE__;
#endif

#define UV_FACTOR ((double)640.0/(double)0x10000L)

/////////////////////////////////////////////////////////////////////////////
// CToolView

static int rotMasks [4] = {0x0000, 0xC000, 0x8000, 0x4000};

                        /*--------------------------*/

int round_int (int value, int round) 
{
if (value >= 0)
	value += round/2;
else
	value -= round/2;
return (value / round) * round;
}

                        /*--------------------------*/

void CTextureTool::UpdateAlignWnd (void)
{
RefreshAlignWnd ();
m_alignWnd.InvalidateRect (NULL);
m_alignWnd.UpdateWindow ();
theApp.MineView ()->Refresh (false);
}

                        /*--------------------------*/

void CTextureTool::RefreshAlignWnd () 
{
	int			x, y, i, uv;
	CDSegment	*seg,
					*childSeg;
	CDSide		*side;
	int			nSide,
					nLine;
	CPen			hPenAxis, 
					hPenGrid;
	CPen			hPenCurrentPoint, 
					hPenCurrentLine,
					hPenCurrentSide;
	CDC			*pDC;
	CPoint		offset;
	CRgn			hRgn;
// each side has 4 children (ordered by side's line number)
	static int side_child[6][4] = {
		{4,3,5,1},//{5,1,4,3},
		{2,4,0,5},//{5,0,4,2},
		{5,3,4,1},//{5,3,4,1},
		{0,4,2,5},//{5,0,4,2},
		{2,3,0,1},//{2,3,0,1},
		{0,3,2,1} //{2,3,0,1}
		};


if (!GetMine ())
	return;
// read scroll bar
offset.x = (int)(m_zoom * (double) HScrollAlign ()->GetScrollPos ());
offset.y = (int)(m_zoom * (double) VScrollAlign ()->GetScrollPos ());
UpdateData (TRUE);
/*
RefreshX();
RefreshY();
RefreshAngle();
RefreshChecks();
*/
// setup drawing area
POINT	minRect, maxRect;

minRect.x = 12;
minRect.y = 10;
maxRect.x = minRect.x + 166;
maxRect.y = minRect.y + 166;
m_centerPt.x = minRect.x + 166 / 2;
m_centerPt.y = minRect.y + 166 / 2;

seg = m_mine->CurrSeg ();
side = m_mine->CurrSide ();
nSide = m_mine->Current ()->side;
nLine = m_mine->Current ()->line;

// get device context handle
pDC = m_alignWnd.GetDC ();

// create brush, pen, and region handles
hPenAxis.CreatePen (PS_DOT, 1, RGB (192,192,192));
hPenGrid.CreatePen (PS_DOT, 1, RGB (128,128,128));
UINT32 select_mode = theApp.MineView ()->GetSelectMode ();
hPenCurrentPoint.CreatePen (PS_SOLID, 1, (select_mode == POINT_MODE) ? RGB (255,0,0) : RGB(255,196,0)); // red
hPenCurrentLine.CreatePen (PS_SOLID, 1, (select_mode == LINE_MODE) ? RGB (255,0,0) : RGB(255,196,0)); // red
hPenCurrentSide.CreatePen (PS_SOLID, 1, (select_mode == LINE_MODE) ? RGB (255,0,0) : RGB(0,255,0)); // red
CRect rc;
m_alignWnd.GetClientRect (rc);
minRect.x = rc.left;
minRect.y = rc.top;
maxRect.x = rc.right;
maxRect.y = rc.bottom;
m_centerPt.x = rc.Width () / 2;
m_centerPt.y = rc.Height () / 2;
hRgn.CreateRectRgn (minRect.x, minRect.y, maxRect.x, maxRect.y);

// clear texture region
i = pDC->SelectObject (&hRgn);
CBrush	brBlack (RGB (0,0,0));
brBlack.GetSafeHandle ();
hRgn.GetSafeHandle ();
pDC->FillRgn (&hRgn, &brBlack);

// draw grid
pDC->SetBkMode (TRANSPARENT);
y=16;
for (x= -32 * y; x < 32 * y; x += 32) {
	pDC->SelectObject((x==0) ? hPenAxis : hPenGrid);
	pDC->MoveTo ((int) (offset.x+m_centerPt.x+m_zoom*x   ), (int) (offset.y+m_centerPt.y-m_zoom*32*y));
	pDC->LineTo ((int) (offset.x+m_centerPt.x+m_zoom*x   ), (int) (offset.y+m_centerPt.y+m_zoom*32*y));
	pDC->MoveTo ((int) (offset.x+m_centerPt.x-m_zoom*32*y), (int) (offset.y+m_centerPt.y+m_zoom*x   ));
	pDC->LineTo ((int) (offset.x+m_centerPt.x+m_zoom*32*y), (int) (offset.y+m_centerPt.y+m_zoom*x   ));
	}	

if (m_mine->IsWall ()) {
	// define array of screen points for (u,v) coordinates
	for (i = 0; i < 4; i++) {
		x = offset.x + m_centerPt.x + (int)(m_zoom*(double)side->uvls[i].u/64.0);
		y = offset.y + m_centerPt.y + (int)(m_zoom*(double)side->uvls[i].v/64.0);
		m_apts [i].x = x;
		m_apts [i].y = y;
		if (i==0) {
			m_minPt.x = m_maxPt.x = x;
			m_minPt.y = m_maxPt.y = y;
			}
		else {
			m_minPt.x = min(m_minPt.x,x);
			m_maxPt.x = max(m_maxPt.x,x);
			m_minPt.y = min(m_minPt.y,y);
			m_maxPt.y = max(m_maxPt.y,y);
			}
		}
	m_minPt.x = max(m_minPt.x,minRect.x);
	m_maxPt.x = min(m_maxPt.x,maxRect.x);
	m_minPt.y = max(m_minPt.y,minRect.y);
	m_maxPt.y = min(m_maxPt.y,maxRect.y);

	if (m_bShowChildren) {
		int sidenum,childnum,linenum;
		int point0,point1,vert0,vert1;
		int childs_side,childs_line;
		int childs_point0,childs_point1,childs_vert0,childs_vert1;
		int x0,y0;
		POINT child_pts[4];

		// draw all sides (u,v)
		pDC->SelectObject (hPenGrid);
		for (linenum=0;linenum<4;linenum++) {
			// find vert numbers for the line's two end points
			point0 = line_vert[side_line[nSide][linenum]][0];
			point1 = line_vert[side_line[nSide][linenum]][1];
			vert0  = seg->verts[point0];
			vert1  = seg->verts[point1];

			// check child for this line 
			sidenum = side_child[nSide][linenum];
			childnum = seg->children[sidenum];
			childSeg = m_mine->Segments () + childnum;
			if (childnum > -1) {

				// figure out which side of child shares two points w/ current->side
				for (childs_side=0;childs_side<6;childs_side++) {
					// ignore children of different textures (or no texture)
					CDSide *childSide = childSeg->sides + childs_side;
					if (m_mine->IsWall (childnum, childs_side) &&
						 (childSide->nBaseTex == side->nBaseTex)) {
						for (childs_line=0;childs_line<4;childs_line++) {
							// find vert numbers for the line's two end points
							childs_point0 = line_vert[side_line[childs_side][childs_line]][0];
							childs_point1 = line_vert[side_line[childs_side][childs_line]][1];
							childs_vert0  = childSeg->verts[childs_point0];
							childs_vert1  = childSeg->verts[childs_point1];
							// if both points of line == either point of parent
							if ((childs_vert0 == vert0 && childs_vert1 == vert1) ||
								 (childs_vert0 == vert1 && childs_vert1 == vert0)) {

								// now we know the child's side & line which touches the parent
								// so, we need to translate the child's points by even increments
								// ..of the texture size in order to make it line up on the screen
								// start by copying points into an array
								for (i = 0; i < 4; i++) {
									x = offset.x + m_centerPt.x + (int)(m_zoom*(double)childSide->uvls[i].u/64.0);
									y = offset.y + m_centerPt.y + (int)(m_zoom*(double)childSide->uvls[i].v/64.0);
									child_pts[i].x = x;
									child_pts[i].y = y;
									}
								// now, calculate offset
								uv = (childs_line+1)&3;
								x0 = child_pts[uv].x - m_apts [linenum].x;
								y0 = child_pts[uv].y - m_apts [linenum].y;
								x0 = round_int(x0,(int)(32.0*m_zoom));
								y0 = round_int(y0,(int)(32.0*m_zoom));
								// translate child points
								for (i=0;i<4;i++) {
									child_pts[i].x -= x0;
									child_pts[i].y -= y0;
									}
								// draw child (u,v)
								pDC->SelectObject (hPenCurrentPoint); // color = cyan
								pDC->MoveTo (child_pts[3].x,child_pts[3].y);
								for (i = 0; i < 4; i++) {
									pDC->LineTo (child_pts[i].x,child_pts[i].y);
									}
								}
							}
						}
					}
				}
			}
		}

	// highlight current point
	pDC->SelectObject ((HBRUSH)GetStockObject(NULL_BRUSH));
	pDC->SelectObject (hPenCurrentPoint);
	x = m_apts [m_mine->Current ()->point].x;
	y = m_apts [m_mine->Current ()->point].y;
	pDC->Ellipse((int) (x-4*m_zoom), (int) (y-4*m_zoom), 
					 (int) (x+4*m_zoom), (int) (y+4*m_zoom));
	// fill in texture
	DrawAlignment (pDC);
	pDC->SelectObject (hRgn);
	// draw uvl
	pDC->SelectObject (hPenCurrentSide);
	pDC->MoveTo (m_apts [3].x,m_apts [3].y);
	for (i=0;i<4;i++)
		pDC->LineTo (m_apts [i].x,m_apts [i].y);
	// highlight current line
	pDC->SelectObject(hPenCurrentLine);
	pDC->MoveTo (m_apts [nLine].x, m_apts [nLine].y);
	pDC->LineTo (m_apts [(nLine+1)&3].x, m_apts [(nLine+1)&3].y);
	}

// release dc
m_alignWnd.ReleaseDC (pDC);
// delete Objects ()
DeleteObject(hRgn);
DeleteObject(hPenCurrentSide);
DeleteObject(hPenCurrentLine);
DeleteObject(hPenCurrentPoint);
DeleteObject(hPenAxis);
DeleteObject(hPenGrid);
}

                        /*--------------------------*/

void CTextureTool::DrawAlignment (CDC *pDC)
{
if (!m_bShowTexture)
	return;
if (!GetMine ())
	return;

	CPalette		*oldPalette;
	CRgn			hRgn;
	int			h, i, j, x, y;
	POINT			offset;
	CDSide		*side = m_mine->CurrSide ();
	CDTexture	tx (bmBuf);
	UINT16		scale;

// read scroll bar
offset.x = (int) (m_zoom * (double) HScrollAlign ()->GetScrollPos ());
offset.y = (int) (m_zoom * (double) VScrollAlign ()->GetScrollPos ());

// set up logical palette
oldPalette = pDC->SelectPalette(thePalette, FALSE);
pDC->RealizePalette();
memset(tx.m_pDataBM, 0, sizeof(bmBuf));
if (DefineTexture (side->nBaseTex, side->nOvlTex, &tx, 0, 0)) {
	DEBUGMSG (" Texture tool: Texture not found (DefineTexture failed)");
	return;
	}
hRgn.CreatePolygonRgn (m_apts, sizeof (m_apts) / sizeof(POINT), ALTERNATE);
pDC->SelectObject (&hRgn);
scale = min (tx.m_width, tx.m_height) / 64;
for (x = m_minPt.x; x < m_maxPt.x; x++) {
	for (y = m_minPt.y; y < m_maxPt.y; y++) {
		i=((int)(((((x-(m_centerPt.x+offset.x))+128)*2)/m_zoom))&63)*scale;
		j=((int)(((((y-(m_centerPt.y+offset.y))+128)*2)/m_zoom))&63)*scale;
		pDC->SetPixel(x, y, h=PALETTEINDEX(tx.m_pDataBM[(tx.m_width-j)*tx.m_width+i]));
		}
	}
DeleteObject(hRgn);
// restort to origional palette
pDC->SelectPalette (oldPalette, FALSE);
}


                        /*--------------------------*/

void CTextureTool::OnAlignX ()
{
if (!GetMine ())
	return;
UpdateData (TRUE);

	int i,	delta;
	CDSide	*side = m_mine->CurrSide ();

if (delta = (int) (side->uvls [m_mine->Current ()->point].u - m_alignX / UV_FACTOR)) {
	UpdateData (TRUE);
	theApp.SetModified (TRUE);
	switch (theApp.MineView ()->GetSelectMode ()) {
		case POINT_MODE:
			side->uvls[m_mine->Current ()->point].u -= delta;
			break;
		case LINE_MODE:
			side->uvls[m_mine->Current ()->line].u -= delta;
			side->uvls[(m_mine->Current ()->line+1)&3].u -= delta;
			break;
		default:
			for (i = 0; i < 4; i++)
				side->uvls[i].u -= delta;
		}  
	UpdateAlignWnd ();
	}
}

                        /*--------------------------*/

void CTextureTool::OnAlignY ()
{
if (!GetMine ())
	return;
UpdateData (TRUE);

	int i, delta;
	CDSide	*side = m_mine->CurrSide ();

if (delta = (int) (side->uvls [m_mine->Current ()->point].v - m_alignY / UV_FACTOR)) {
	UpdateData (TRUE);
	theApp.SetModified (TRUE);
	switch (theApp.MineView ()->GetSelectMode ()) {
		case POINT_MODE:
			side->uvls[m_mine->Current ()->point].v -= delta;
			break;
		case LINE_MODE:
			side->uvls[m_mine->Current ()->line].v -= delta;
			side->uvls[(m_mine->Current ()->line+1)&3].v -= delta;
			break;
		default:
			for (i = 0; i < 4; i++)
				side->uvls[i].v -= delta;
		}  
	UpdateAlignWnd ();
	}
}

                        /*--------------------------*/

void CTextureTool::OnAlignRot ()
{
if (!GetMine ())
	return;
UpdateData (TRUE);
  
	double delta,dx,dy,angle;
	CDSide	*side = m_mine->CurrSide ();

dx = side->uvls[1].u - side->uvls[0].u;
dy = side->uvls[1].v - side->uvls[0].v;
angle = (dx || dy) ? atan3 (dy,dx) - M_PI_2 : 0;
delta = angle - m_alignAngle * PI / 180.0;
RotateUV (delta, FALSE);
}

                        /*--------------------------*/

void CTextureTool::RefreshAlignment ()
{
if (!GetMine ())
	return;
CDSide * side = m_mine->CurrSide ();

m_alignX = (double) side->uvls [m_mine->Current ()->point].u * UV_FACTOR;
m_alignY = (double) side->uvls [m_mine->Current ()->point].v * UV_FACTOR;

double dx = side->uvls [1].u - side->uvls [0].u;
double dy = side->uvls [1].v - side->uvls [0].v;
m_alignAngle = ((dx || dy) ? atan3 (dy,dx) - M_PI_2 : 0) * 180.0 / M_PI;
if (m_alignAngle < 0)
	m_alignAngle += 360.0;
else if (m_alignAngle > 360)
	m_alignAngle -= 360.0;
int h = side->nOvlTex & 0xC000;
for (m_alignRot2nd = 0; m_alignRot2nd < 4; m_alignRot2nd++)
	if (rotMasks [m_alignRot2nd] == h)
		break;
}

                        /*--------------------------*/

void CTextureTool::RotateUV (double angle, bool bUpdate)
{
if (!GetMine ())
	return;

	int i;
	double	x,y,a,radius;
	CDSide	*side = m_mine->CurrSide ();

UpdateData (TRUE);
theApp.SetModified (TRUE);
for (i = 0; i < 4; i++) {
	// convert to polar coordinates
	x = side->uvls[i].u;
	y = side->uvls[i].v;
	if (x || y) {
		radius = sqrt(x*x + y*y);
		a = atan3 (y,x) - angle;			// add rotation
		// convert back to rectangular coordinates
		x = radius * cos(a);
		y = radius * sin(a);
		side->uvls[i].u = (INT16) x;
		side->uvls[i].v = (INT16) y;
		}
	}
if (bUpdate)
	m_alignAngle -= angle * 180.0 / PI;
UpdateData (FALSE);
UpdateAlignWnd ();
}

                        /*--------------------------*/

void CTextureTool::HFlip (void)
{
if (!GetMine ())
	return;

	CDSide	*side = m_mine->CurrSide ();
	INT16		h, i, l;

UpdateData (TRUE);
theApp.SetModified (TRUE);
switch (theApp.MineView ()->GetSelectMode ()) {
	case POINT_MODE:
		break;
	case LINE_MODE:
		l = m_mine->Current ()->line;
		h = side->uvls [l].u;
		side->uvls [l].u = side->uvls [(l + 1) & 3].u;
		side->uvls [(l + 1) & 3].u = h;
		break;
	default:
		for (i = 0; i < 2; i++) {
			h = side->uvls[i].u;
			side->uvls[i].u = side->uvls[i + 2].u;
			side->uvls[i + 2].u = h;
			}
	}
UpdateData (FALSE);
theApp.SetModified (TRUE);
UpdateAlignWnd ();
}

                        /*--------------------------*/

void CTextureTool::VFlip (void)
{
if (!GetMine ())
	return;

	CDSide	*side = m_mine->CurrSide ();
	INT16		h, i, l;

UpdateData (TRUE);
theApp.SetModified (TRUE);
switch (theApp.MineView ()->GetSelectMode ()) {
	case POINT_MODE:
		break;
	case LINE_MODE:
		l = m_mine->Current ()->line;
		h = side->uvls [l].v;
		side->uvls [l].v = side->uvls [(l + 1) & 3].v;
		side->uvls [(l + 1) & 3].v = h;
		break;
	default:
		for (i = 0; i < 2; i++) {
			h = side->uvls[i].v;
			side->uvls[i].v = side->uvls[i + 2].v;
			side->uvls[i + 2].v = h;
			}
	}
UpdateData (FALSE);
theApp.SetModified (TRUE);
UpdateAlignWnd ();
}

                        /*--------------------------*/

void CTextureTool::HAlign (int dir)
{
if (!GetMine ())
	return;

	int		i;
	CDSide	*side = m_mine->CurrSide ();
	double	delta = ((double) move_rate / 0x10000L) * (0x0800 / 8) / m_zoom * dir;

UpdateData (TRUE);
theApp.SetModified (TRUE);
switch (theApp.MineView ()->GetSelectMode ()) {
	case POINT_MODE:
		side->uvls[m_mine->Current ()->point].u += (INT16) delta;
		break;
	case LINE_MODE:
		side->uvls[m_mine->Current ()->line].u += (INT16) delta;
		side->uvls[(m_mine->Current ()->line+1)&3].u += (INT16) delta;
		break;
	default:
		for (i=0;i<4;i++)
			side->uvls[i].u += (INT16) delta;
	}
m_alignX = (double) side->uvls [m_mine->Current ()->point].u * UV_FACTOR;
UpdateData (FALSE);
theApp.SetModified (TRUE);
UpdateAlignWnd ();
}

                        /*--------------------------*/

void CTextureTool::VAlign (int dir)
{
if (!GetMine ())
	return;

	int		i;
	CDSide	*side = m_mine->CurrSide ();
	double	delta = ((double) move_rate / 0x10000L) * (0x0800 / 8) / m_zoom * dir;

UpdateData (TRUE);
theApp.SetModified (TRUE);
switch (theApp.MineView ()->GetSelectMode ()) {
	case POINT_MODE:
		side->uvls[m_mine->Current ()->point].v += (INT16) delta;
		break;
	case LINE_MODE:
		side->uvls[m_mine->Current ()->line].v += (INT16) delta;
		side->uvls[(m_mine->Current ()->line+1)&3].v += (INT16) delta;
		break;
	default:
		for (i=0;i<4;i++)
			side->uvls[i].v += (INT16) delta;
	}
m_alignY = (double)side->uvls[m_mine->Current ()->point].v * UV_FACTOR;
UpdateData (FALSE);
theApp.SetModified (TRUE);
UpdateAlignWnd ();
}

                        /*--------------------------*/

void CTextureTool::OnHFlip (void)
{
HFlip ();
}

                        /*--------------------------*/

void CTextureTool::OnVFlip (void)
{
VFlip ();
}

                        /*--------------------------*/

void CTextureTool::OnAlignLeft (void)
{
HAlign (-1);
}

                        /*--------------------------*/

void CTextureTool::OnAlignRight (void)
{
HAlign (1);
}

                        /*--------------------------*/

void CTextureTool::OnAlignUp (void)
{
VAlign (-1);
}

                        /*--------------------------*/

void CTextureTool::OnAlignDown (void)
{
VAlign (1);
}

                        /*--------------------------*/

void CTextureTool::OnAlignRotLeft (void)
{
RotateUV (angle_rate);
}

                        /*--------------------------*/

void CTextureTool::OnAlignRotRight (void)
{
RotateUV (-angle_rate);
}

                        /*--------------------------*/

void CTextureTool::OnHShrink ()
{
if (!GetMine ())
	return;

	int		i = m_mine->Current ()->point;
	CDSide	*side = m_mine->CurrSide ();
	double	delta = ((double) move_rate / 0x10000L) * (0x0800 / 8) / m_zoom ;

UpdateData (TRUE);
theApp.SetModified (TRUE);
side->uvls [0].u -= (INT16) delta;
side->uvls[1].u -= (INT16) delta;
side->uvls [2].u += (INT16) delta;
side->uvls[3].u += (INT16) delta;
UpdateAlignWnd ();
}

                        /*--------------------------*/

void CTextureTool::OnVShrink ()
{
if (!GetMine ())
	return;

	int		i = m_mine->Current ()->point;
	CDSide	*side = m_mine->CurrSide ();
	double	delta = ((double) move_rate / 0x10000L) * (0x0800 / 8) / m_zoom;

UpdateData (TRUE);
theApp.SetModified (TRUE);
side->uvls [0].v += (INT16) delta;
side->uvls[3].v += (INT16) delta;
side->uvls [1].v -= (INT16) delta;
side->uvls[2].v -= (INT16) delta;
UpdateAlignWnd ();
}

                        /*--------------------------*/

void CTextureTool::OnAlignReset ()
{
if (!GetMine ())
	return;
UpdateData (TRUE);
theApp.SetModified (TRUE);
theApp.LockUndo ();
m_mine->SetUV (m_mine->Current ()->segment, m_mine->Current ()->side, 0, 0, 0);
m_alignX = 0;
m_alignY = 0;
m_alignAngle = 0;
Rot2nd (0);
UpdateData (FALSE);
theApp.UnlockUndo ();
UpdateAlignWnd ();
}

                        /*--------------------------*/

void CTextureTool::OnAlignResetMarked ()
{	
if (!GetMine ())
	return;

	CDSegment *seg;
	INT16 segnum, sidenum, nWalls = m_mine->GameInfo ().walls.count;
	BOOL bModified = FALSE;

UpdateData (TRUE);
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
for (segnum = 0, seg = m_mine->Segments (); segnum < m_mine->SegCount (); segnum++, seg++) {
	for (sidenum = 0; sidenum < 6; sidenum++) {
		if (m_mine->SideIsMarked (segnum, sidenum)) {
			if ((seg->children [sidenum] == -1) || 
				 (seg->sides [sidenum].nWall < nWalls)) {
				seg->sides [sidenum].nOvlTex &= 0x3fff; // rotate 0
				m_mine->SetUV (segnum,sidenum,0,0,0);
				bModified = TRUE;
				}
			}
		}
	}
if (bModified)
	theApp.UnlockUndo ();
else
	theApp.ResetModified (bUndo);
theApp.MineView ()->Refresh (false);
UpdateAlignWnd ();
}

                        /*--------------------------*/

void CTextureTool::OnAlignStretch2Fit ()
{
	CDSide		*side = m_mine->CurrSide ();
	UINT32		scale = 1; //pTextures [file_type][side->nBaseTex].Scale (side->nBaseTex);
	CDSegment	*seg;
	INT16			segnum, sidenum;
	int			i;

UpdateData (TRUE);
theApp.SetModified (TRUE);
if (!m_mine->GotMarkedSides ()) {
	for (i = 0; i < 4; i++) {
		side->uvls [i].u = default_uvls [i].u / scale;
		side->uvls [i].v = default_uvls [i].v / scale;
		}
	}
else {
	theApp.LockUndo ();
	for (segnum = 0, seg = m_mine->Segments (); segnum < m_mine->SegCount (); segnum++, seg++) {
		for (sidenum = 0, side = seg->sides; sidenum < 6; sidenum++, side++) {
			if (m_mine->SideIsMarked (segnum, sidenum)) {
				for (i = 0; i < 4; i++) {
					side->uvls [i].u = default_uvls [i].u / scale;
					side->uvls [i].v = default_uvls [i].v / scale;
					}
				}
			}
		}
	theApp.UnlockUndo ();
	}
theApp.MineView ()->Refresh (false);
UpdateAlignWnd ();
}

                        /*--------------------------*/

void CTextureTool::AlignChildren (INT16 segnum, INT16 sidenum, bool bStart)
{
// set all segment sides as not aligned yet
if (!GetMine ())
	return;
if (bStart) {
	CDSegment *seg = m_mine->Segments ();
	int i;
	for (i = m_mine->SegCount (); i; i--, seg++)
		 seg->seg_number = 0; // all six sides not aligned yet
	}
// mark current side as aligned
m_mine->Segments (segnum)->seg_number = 1;
// call recursive function which aligns one at a time
AlignChildTextures (segnum, sidenum, MAX_SEGMENTS);
}

                        /*--------------------------*/

void CTextureTool::OnAlignAll (void)
{
// set all segment sides as not aligned yet
if (!GetMine ())
	return;

	CDSegment	*currSeg = m_mine->CurrSeg (),
					*seg = m_mine->Segments ();
	CDSide		*side = m_mine->CurrSide (),
					*childSide;
	INT16			segnum, 
					sidenum = m_mine->Current ()->side,
					linenum = 3;
	double		sangle, cangle, angle, length; 

UpdateData (TRUE);
theApp.SetModified (TRUE);
theApp.LockUndo ();
bool bAll = !m_mine->GotMarkedSegments ();
for (segnum = 0, seg = m_mine->Segments (); segnum < m_mine->SegCount (); segnum++, seg++)
	 seg->seg_number = 0;
for (segnum = 0, seg = m_mine->Segments (); segnum < m_mine->SegCount (); segnum++, seg++) {
	if (seg->seg_number)
		continue;
	childSide = seg->sides + sidenum;
	if (m_bUse1st && (side->nBaseTex != childSide->nBaseTex))
		continue;
	if (m_bUse2nd && (side->nOvlTex != childSide->nOvlTex))
		continue;
	if (!(bAll || m_mine->SideIsMarked (segnum, sidenum)))
		continue;
	if (segnum != m_mine->Current ()->segment) {
		m_mine->SetUV (segnum, sidenum, 0, 0, 0);
		sangle = atan3 (side->uvls [(linenum + 1) & 3].v - side->uvls [linenum].v, 
							 side->uvls [(linenum + 1) & 3].u - side->uvls [linenum].u); 
		cangle = atan3 (childSide->uvls [linenum].v - childSide->uvls [(linenum + 1) & 3].v, 
							 childSide->uvls [linenum].u - childSide->uvls [(linenum + 1) & 3].u); 
		// now rotate childs (u, v) coords around child_point1 (cangle - sangle)
		int i;
		for (i = 0; i < 4; i++) {
			angle = atan3 (childSide->uvls [i].v, childSide->uvls [i].u); 
			length = sqrt ((double)childSide->uvls [i].u * (double) childSide->uvls [i].u +
								(double)childSide->uvls [i].v * (double) childSide->uvls [i].v); 
			angle -= (cangle - sangle); 
			childSide->uvls [i].u = (INT16) (length * cos (angle)); 
			childSide->uvls [i].v = (INT16) (length * sin (angle)); 
			}
		}
	AlignChildren (segnum, sidenum, false);
	}
theApp.UnlockUndo ();
UpdateAlignWnd ();
}

                        /*--------------------------*/

void CTextureTool::OnAlignChildren ()
{
// set all segment sides as not aligned yet
if (!GetMine ())
	return;
UpdateData (TRUE);
theApp.SetModified (TRUE);
theApp.LockUndo ();
if (!m_mine->GotMarkedSegments ())
	// call recursive function which aligns one at a time
	AlignChildren (m_mine->Current ()->segment, m_mine->Current ()->side, true);
else {	// use all marked sides as alignment source
	int segnum, sidenum;
	for (segnum = 0; segnum < m_mine->SegCount (); segnum++)
		for (sidenum = 0; sidenum < 6; sidenum++)
			if (m_mine->SideIsMarked (segnum, sidenum)) 
				AlignChildren (segnum, sidenum, true);
	}
theApp.UnlockUndo ();
UpdateAlignWnd ();
}

                        /*--------------------------*/

static const int side_child[6][4] = {
  {4,3,5,1},
  {2,4,0,5},
  {5,3,4,1},
  {0,4,2,5},
  {2,3,0,1},
  {0,3,2,1}
};

void CTextureTool::AlignChildTextures (int segnum, int sidenum, int nDepth)  
{
if (!GetMine ())
	return;

	CDSegment	*seg, *childSeg;
	CDSide		*side, *childSide; 
	int			child_segnum;
	int			child_sidenum;
	int			nLine, h;
	INT16			nBaseTex;
	char			bAlignedSides = 0;

if (nDepth <= 0)
	return;
if ((segnum < 0) || (segnum >= m_mine->SegCount ()))
	return;
seg = m_mine->Segments (segnum);
if (seg->seg_number < 0)
	return;

	INT16			*childList = new INT16 [m_mine->SegCount ()];
	INT16			pos = 0, tos = 0;

if (!childList)
	return;

childList [tos++] = segnum;
seg->seg_number = sidenum;

if (m_bIgnorePlane) {
	side = seg->sides + sidenum;
	nBaseTex = side->nBaseTex;
	bAlignedSides = 1 << sidenum;
	h = m_mine->AlignTextures (segnum, sidenum, segnum, m_bUse1st, m_bUse2nd, bAlignedSides);
	for (nLine = 0; nLine < 4; nLine++) {
		child_sidenum = side_child[sidenum][nLine];
		if (!(bAlignedSides & (1 << child_sidenum))) {
			bAlignedSides |= (1 << child_sidenum);
			childSide = seg->sides + child_sidenum;
			if (childSide->nBaseTex == nBaseTex)
				m_mine->AlignTextures (segnum, child_sidenum, segnum, m_bUse1st, m_bUse2nd, bAlignedSides);
			}
		}
	if (h >= 0) {
		for (child_sidenum = 0, childSide = seg->sides; child_sidenum < 6; child_sidenum++, childSide++) {
			if (childSide->nBaseTex == nBaseTex) {
				for (nLine = 0; nLine < 4; nLine++) {
					child_segnum = seg->children [side_child[child_sidenum][nLine]];
					if ((child_segnum < 0) || (child_segnum >= m_mine->SegCount ()))
						continue;
					childSeg = m_mine->Segments (child_segnum);
					if (childSeg->seg_number != 0)
						continue;
					h = m_mine->AlignTextures (segnum, child_sidenum, child_segnum, m_bUse1st, m_bUse2nd, 0);
					childSeg->seg_number = h + 1;
					}
				}
			}
		}
	seg->seg_number = -1;
	--nDepth;
	for (sidenum = 0, childSide = seg->sides; sidenum < 6; sidenum++, childSide++) {
//			if (childSide->nBaseTex != side->nBaseTex)
//				continue;
		for (nLine = 0; nLine < 4; nLine++) {
			child_segnum = seg->children [side_child[sidenum][nLine]];
			if ((child_segnum < 0) || (child_segnum >= m_mine->SegCount ()))
				continue;
			childSeg = m_mine->Segments (child_segnum);
			child_sidenum = childSeg->seg_number - 1;
			if (child_sidenum < 0)
				continue;
			AlignChildTextures (child_segnum, child_sidenum, nDepth);
			}
		}
	}
else {
	while (pos < tos) {
		segnum = childList [pos++];
		seg = m_mine->Segments (segnum);
		sidenum = seg->seg_number;
		seg->seg_number = -1;
		for (nLine = 0; nLine < 4; nLine++) {
			if (sidenum < 0)
				continue;
			child_segnum = seg->children [side_child[sidenum][nLine]];
			if ((child_segnum < 0) || (child_segnum >= m_mine->SegCount ()))
				continue;
			childSeg = m_mine->Segments (child_segnum);
			if (childSeg->seg_number)
				continue;
			childSeg->seg_number = m_mine->AlignTextures (segnum, sidenum, child_segnum, m_bUse1st, m_bUse2nd, 0);
			if (childSeg->seg_number >= 0)
				childList [tos++] = child_segnum;
			}
/*
		for (nLine = 0; nLine < 4; nLine++) {
			child_segnum = seg->children [side_child[sidenum][nLine]];
			if ((child_segnum < 0) || (child_segnum >= m_mine->SegCount ()))
				continue;
			childSeg = m_mine->Segments (child_segnum);
			child_sidenum = childSeg->seg_number;
			if (child_sidenum < 0)
				continue;
			AlignChildTextures (child_segnum, child_sidenum, --nDepth);
			}
*/
		}
	}
delete childList;
}

                        /*--------------------------*/

void CTextureTool::OnZoomIn ()
{
if (m_zoom < 16.0) {
	m_zoom *= 2.0;
	UpdateAlignWnd ();
	}
}

                        /*--------------------------*/

void CTextureTool::OnZoomOut ()
{
if (m_zoom > 1.0/16.0) {
	m_zoom /= 2.0;
	UpdateAlignWnd ();
	}
}

                        /*--------------------------*/

void CTextureTool::Rot2nd (int iAngle)
{
	CDSide *side = m_mine->CurrSide ();
 
if ((side->nOvlTex & 0x1fff) && ((side->nOvlTex & 0xc000) != rotMasks [iAngle])) {
	theApp.SetModified (TRUE);
	side->nOvlTex &= ~0xc000;
   side->nOvlTex |= rotMasks [iAngle];
	m_alignRot2nd = iAngle;
	UpdateData (FALSE);
	UpdateAlignWnd ();
	}
}

                        /*--------------------------*/

void CTextureTool::OnRot2nd0 ()
{
Rot2nd (0);
}

                        /*--------------------------*/

void CTextureTool::OnRot2nd90 ()
{
Rot2nd (1);
}

                        /*--------------------------*/

void CTextureTool::OnRot2nd180 ()
{
Rot2nd (2);
}

                        /*--------------------------*/

void CTextureTool::OnRot2nd270 ()
{
Rot2nd (3);
}

                        /*--------------------------*/
		
//eof aligndlg.cpp