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

#define UV_FACTOR ((double)640.0/(double)0x10000L)
#define M_PI_2 (PI / 2)

/////////////////////////////////////////////////////////////////////////////
// CToolView

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
}

                        /*--------------------------*/

void CTextureTool::RefreshAlignWnd () 
{
	CMine			*mine;
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


if (!(mine = theApp.GetMine ()))
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

seg = mine->CurrSeg ();
side = mine->CurrSide ();
nSide = mine->current->side;
nLine = mine->current->line;

// get device context handle
pDC = m_alignWnd.GetDC ();

// create brush, pen, and region handles
hPenAxis.CreatePen (PS_DOT, 1, RGB (192,192,192));
hPenGrid.CreatePen (PS_DOT, 1, RGB (128,128,128));
UINT32 select_mode = theApp.MineView ()->GetSelectMode ();
hPenCurrentPoint.CreatePen (PS_SOLID, 1, (select_mode == POINT_MODE) ? RGB (255,0,0) : RGB(255,196,0)); // red
hPenCurrentLine.CreatePen (PS_SOLID, 1, (select_mode == LINE_MODE) ? RGB (255,0,0) : RGB(255,196,0)); // red
hPenCurrentSide.CreatePen (PS_SOLID, 1, (select_mode == LINE_MODE) ? RGB (255,0,0) : RGB(0,255,,0)); // red
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

if (mine->IsWall ()) {
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
			childSeg = mine->Segments () + childnum;
			if (childnum > -1) {

				// figure out which side of child shares two points w/ current->side
				for (childs_side=0;childs_side<6;childs_side++) {
					// ignore children of different textures (or no texture)
					CDSide *childSide = childSeg->sides + childs_side;
					if (mine->IsWall (childnum, childs_side) &&
						 (childSide->tmap_num == side->tmap_num)) {
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
	x = m_apts [mine->current->point].x;
	y = m_apts [mine->current->point].y;
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

	CPalette	*oldPalette;
	CRgn		hRgn;
	int		h, i, j, x, y;
	//UINT8		bitmap_array [1024*1204];
	POINT		offset;
	CMine		*mine = theApp.GetMine ();
	CDSide	*side = mine->CurrSide ();

// read scroll bar
offset.x = (int) (m_zoom * (double) HScrollAlign ()->GetScrollPos ());
offset.y = (int) (m_zoom * (double) VScrollAlign ()->GetScrollPos ());

// set up logical palette
oldPalette = pDC->SelectPalette(thePalette, FALSE);
pDC->RealizePalette();
memset(bmBuf,0,sizeof(bmBuf));
if (DefineTexture (side->tmap_num, side->tmap_num2, bmBuf, 0, 0))
	DEBUGMSG (" Texture tool: texture not found (DefineTexture failed)");
hRgn.CreatePolygonRgn (m_apts, sizeof (m_apts) / sizeof(POINT), ALTERNATE);
pDC->SelectObject (&hRgn);
for (x = m_minPt.x; x< m_maxPt.x; x++) {
	for (y = m_minPt.y; y < m_maxPt.y; y++) {
		i=((int)((((x-(m_centerPt.x+offset.x))+128)*2)/m_zoom))&63;
		j=((int)((((y-(m_centerPt.y+offset.y))+128)*2)/m_zoom))&63;
		pDC->SetPixel(x, y, h=PALETTEINDEX(bmBuf[(63-j)*64+i]));
		}
	}
DeleteObject(hRgn);
// restort to origional palette
pDC->SelectPalette (oldPalette, FALSE);
}


                        /*--------------------------*/

void CTextureTool::OnAlignX ()
{
	CMine	*mine;

if (!(mine = theApp.GetMine ()))
	return;
UpdateData (TRUE);

	int i,	delta;
	CDSide	*side = mine->CurrSide ();

if (delta = (int) (side->uvls [mine->current->point].u - m_alignX / UV_FACTOR)) {
	switch (theApp.MineView ()->GetSelectMode ()) {
		case POINT_MODE:
			side->uvls[mine->current->point].u -= delta;
			break;
		case LINE_MODE:
			side->uvls[mine->current->line].u -= delta;
			side->uvls[(mine->current->line+1)&3].u -= delta;
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
	CMine	*mine;

if (!(mine = theApp.GetMine ()))
	return;
UpdateData (TRUE);

	int i, delta;
	CDSide	*side = mine->CurrSide ();

if (delta = (int) (side->uvls [mine->current->point].v - m_alignY / UV_FACTOR)) {
	switch (theApp.MineView ()->GetSelectMode ()) {
		case POINT_MODE:
			side->uvls[mine->current->point].v -= delta;
			break;
		case LINE_MODE:
			side->uvls[mine->current->line].v -= delta;
			side->uvls[(mine->current->line+1)&3].v -= delta;
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
	CMine	*mine;

if (!(mine = theApp.GetMine ()))
	return;
UpdateData (TRUE);
  
	double delta,dx,dy,angle;
	CDSide	*side = mine->CurrSide ();

dx = side->uvls[1].u - side->uvls[0].u;
dy = side->uvls[1].v - side->uvls[0].v;
angle = (dx || dy) ? atan3 (dy,dx) - M_PI_2 : 0;
delta = angle - m_alignAngle * PI / 180.0;
RotateUV (delta, FALSE);
}

                        /*--------------------------*/

void CTextureTool::RotateUV (double angle, bool bUpdate)
{
	int i;
	double	x,y,a,radius;
	CDSide	*side = theApp.GetMine ()->CurrSide ();

for (i=0;i<4;i++) {
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

void CTextureTool::HAlign (int dir)
{
	CMine		*mine = theApp.GetMine ();
	int		i;
	CDSide	*side = theApp.GetMine ()->CurrSide ();
	double	delta = ((double) move_rate / 0x10000L) * (0x0800 / 8) / m_zoom * dir;

switch (theApp.MineView ()->GetSelectMode ()) {
	case POINT_MODE:
		side->uvls[mine->current->point].u += (INT16) delta;
		break;
	case LINE_MODE:
		side->uvls[mine->current->line].u += (INT16) delta;
		side->uvls[(mine->current->line+1)&3].u += (INT16) delta;
		break;
	default:
		for (i=0;i<4;i++)
			side->uvls[i].u += (INT16) delta;
	}
m_alignX = (double)side->uvls[mine->current->point].u * UV_FACTOR;
UpdateData (FALSE);
UpdateAlignWnd ();
}

                        /*--------------------------*/

void CTextureTool::VAlign (int dir)
{
	CMine		*mine = theApp.GetMine ();
	int		i;
	CDSide	*side = theApp.GetMine ()->CurrSide ();
	double	delta = ((double) move_rate / 0x10000L) * (0x0800 / 8) / m_zoom * dir;

switch (theApp.MineView ()->GetSelectMode ()) {
	case POINT_MODE:
		side->uvls[mine->current->point].v += (INT16) delta;
		break;
	case LINE_MODE:
		side->uvls[mine->current->line].v += (INT16) delta;
		side->uvls[(mine->current->line+1)&3].v += (INT16) delta;
		break;
	default:
		for (i=0;i<4;i++)
			side->uvls[i].v += (INT16) delta;
	}
m_alignY = (double)side->uvls[mine->current->point].v * UV_FACTOR;
UpdateData (FALSE);
UpdateAlignWnd ();
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
	CMine		*mine = theApp.GetMine ();
	int		i = mine->current->point;
	CDSide	*side = theApp.GetMine ()->CurrSide ();
	double	delta = ((double) move_rate / 0x10000L) * (0x0800 / 8) / m_zoom ;


side->uvls [0].u -= (INT16) delta;
side->uvls[1].u -= (INT16) delta;
side->uvls [2].u += (INT16) delta;
side->uvls[3].u += (INT16) delta;
UpdateAlignWnd ();
}

                        /*--------------------------*/

void CTextureTool::OnVShrink ()
{
	CMine		*mine = theApp.GetMine ();
	int		i = mine->current->point;
	CDSide	*side = theApp.GetMine ()->CurrSide ();
	double	delta = ((double) move_rate / 0x10000L) * (0x0800 / 8) / m_zoom;


side->uvls [0].v += (INT16) delta;
side->uvls[3].v += (INT16) delta;
side->uvls [1].v -= (INT16) delta;
side->uvls[2].v -= (INT16) delta;
UpdateAlignWnd ();
}

                        /*--------------------------*/

void CTextureTool::OnAlignReset ()
{
	CMine	*mine = theApp.GetMine ();

mine->SetUV (mine->current->segment, mine->current->side, 0, 0, 0);
m_alignX = 0;
m_alignY = 0;
m_alignAngle = 0;
Rot2nd (0);
UpdateData (FALSE);
UpdateAlignWnd ();
}

                        /*--------------------------*/

void CTextureTool::OnAlignResetMarked ()
{	
	CMine	*mine = theApp.GetMine ();
	CDSegment *seg;
	INT16 segnum, sidenum;

for (segnum = 0, seg = mine->Segments (); segnum < mine->numSegments; segnum++) {
	if  (seg->wall_bitmask & MARKED_MASK) {
		for (sidenum = 0; sidenum < 6; sidenum++) {
			if (seg->children[sidenum] == -1) {
				seg->sides [sidenum].tmap_num2 &=(~0xc000); // rotate 0
				mine->SetUV (segnum,sidenum,0,0,0);
				}
			}
		}
	}
}

                        /*--------------------------*/

void CTextureTool::OnAlignStretch2Fit ()
{
CDSide	*side = theApp.GetMine ()->CurrSide ();

for (int i = 0; i < 4; i++) {
	side->uvls[i].u = default_uvls[i].u;
   side->uvls[i].v = default_uvls[i].v;
	}
UpdateAlignWnd ();
}

                        /*--------------------------*/

void CTextureTool::OnAlignChildAlign ()
{
	// set all segment sides as not aligned yet
	CMine	*mine = theApp.GetMine ();
	CDSegment *seg;
	INT16 segnum;

for (segnum = 0, seg = mine->Segments (); segnum < mine->numSegments; segnum++)
    seg->seg_number = 0; // all six sides not aligned yet
// mark current side as aligned
mine->CurrSeg ()->seg_number = 1;

// call recursive function which aligns one at a time
AlignChildTextures (mine->current->segment, mine->current->side, 100);
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
	CMine	*mine = theApp.GetMine ();
	int child_segnum;
	int child_sidenum;
	int i;
	CDSegment	*seg, *childSeg;

if (nDepth > 0) {
	seg = mine->Segments () + segnum;
	for (i=0;i<4;i++) {
		child_segnum = seg->children[side_child[sidenum][i]];
		if ((child_segnum < 0) || (child_segnum >= mine->numSegments))
			continue;
		childSeg = mine->Segments () + child_segnum;
		if (childSeg->seg_number != 0)
			continue;
			child_sidenum = mine->AlignTextures (segnum, sidenum, child_segnum);
		if (child_sidenum == -1)
			continue;
		childSeg->seg_number |= (1 << child_sidenum);
		AlignChildTextures (child_segnum, child_sidenum, --nDepth);
		}
	}
}

                        /*--------------------------*/

void CTextureTool::OnZoomIn ()
{
if (m_zoom < 4.0) {
	m_zoom *= 2.0;
	UpdateAlignWnd ();
	}
}

                        /*--------------------------*/

void CTextureTool::OnZoomOut ()
{
if (m_zoom > 1.0/4.0) {
	m_zoom /= 2.0;
	UpdateAlignWnd ();
	}
}

                        /*--------------------------*/

void CTextureTool::Rot2nd (int iAngle)
{
	CDSide *side = theApp.GetMine ()->CurrSide ();
	static int rotMasks [4] = {0x0000, 0xC000, 0x8000, 0x4000};
 
if (side->tmap_num2 & 0x1fff) {
	side->tmap_num2 &= (~0xc000);
   side->tmap_num2 |= rotMasks [iAngle];
	m_alignRot2nd = iAngle;
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