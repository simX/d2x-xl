// dlcView.h : interface of the CMineView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DLCVIEW_H__C46B3F4C_5EFF_11D2_AE2A_00C0F03014A5__INCLUDED_)
#define AFX_DLCVIEW_H__C46B3F4C_5EFF_11D2_AE2A_00C0F03014A5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "afxcview.h"
#include "DlcDoc.h"
#include "Matrix.h"
#include "poly.h"


enum eViewObjectFlags
{
	eViewObjectsNone          = 0,
	eViewObjectsHostages      = (1<<0),
	eViewObjectsKeys          = (1<<1),
	eViewObjectsCoopPlayers   = (1<<2),
	eViewObjectsPowerups      = (1<<3),
	eViewObjectsRobots        = (1<<4),
	eViewObjectsWeapons       = (1<<5),
	eViewObjectsControlCenter = (1<<6),
	eViewObjectsAll           = 0x7f
};

enum eViewFlags
{
	eViewFlagHiRes            = (1<<0),
	eViewFlagLights           = (1<<1),
	eViewFlagShading          = (1<<2),
	eViewFlagWalls            = (1<<3),
	eViewFlagSpecial          = (1<<4)
};

enum eViewOptions
{
	eViewAllLines = 0,
	eViewHideLines,
	eViewNearbyCubeLines,
	eViewPartialLines,
	eViewPartialTextureMapped,
	eViewTextureMapped
};

enum eMouseStates
{
	eMouseStateIdle,
	eMouseStateRubberBand
};

                        /*--------------------------*/

class CToolView : public CWnd {
	public:
		DECLARE_DYNCREATE(CToolView)
		CToolView () {};
	DECLARE_MESSAGE_MAP()
};
                        
                        /*--------------------------*/
                        
class CMineView : public CView
{
protected: // create from serialization only
	CMineView();
	DECLARE_DYNCREATE(CMineView)

	CSplitterWnd	*m_pSplitter;
#if 1
	// drawing functions
	void	InitView(CDC* pViewDC);
	void	ClearView();
	void	DrawWireFrame(CMine *mine, bool bPartial);
	void	DrawTextureMappedCubes(CMine *mine);
	void	DrawCube(CDSegment& seg, bool bPartial);
	void	DrawCubeTextured(CDSegment& seg, UINT8* light_index);

	void	DrawCurrentCube(CDSegment& seg, bool bPartial);
	void	DrawLine(CDSegment& seg,INT16 vert1,INT16 vert2);

	void	DrawWalls (CMine *mine);
	void	DrawLights (CMine *mine);
	void	DrawOctagon(CMine *mine, INT16 sidenum, INT16 segnum);
	void	DrawObject(CMine &mine,INT16 objnum,INT16 clear_it);
	void	DrawObjects (CMine &mine, bool bPartial);

	int SetupModel(CMine *mine, CDObject *obj); // poly.c
	void SetModelPoints(int start, int end);
	void DrawModel();      // poly.c
	void InterpModelData(UINT8 *model_data); // poly.c
	void DrawPoly(POLY *p);
	int ReadModelData(FILE *file, CDObject *obj);

	// view control functions
	void	ZoomIn();
	void	ZoomOut();
	void	Rotate(char direction, double angle);
	void	Pan(char direction, INT32 amount);
	void	AlignSide();
	void	CenterMine();
	void	CenterCube();
	void	CenterObject();
	void	SetViewOption(eViewOptions option);
	void	ToggleViewFlag(eViewFlags flag);
	void	ToggleViewObjects(eViewObjectFlags mask);
	void	SetViewObjects(eViewObjectFlags mask);

	// member variables
	int			m_viewHeight;	// in pixels
	int			m_viewWidth;	// in pixels
	int			m_viewDepth;	// in bytes
	HBITMAP		m_DIB;
	void			*m_pvBits;
	CDC			m_DC;
	CDC			*m_pDC; // if all goes well, this is set to &m_DC
	bool			m_bUpdate;
	UINT32		m_viewObjects;
	UINT32		m_viewFlags;
	UINT32		m_viewOption;
	UINT32		m_SelectMode;

	CDSelection	*m_Current;

	CPen*			m_PenCyan;
	CPen*			m_PenRed;
	CPen*			m_PenGray;
	CPen*			m_PenGreen;
	CPen*			m_PenDkGreen;
	CPen*			m_PenDkCyan;
	CPen*			m_PenYellow;
	CPen*			m_PenOrange;
	CPen*			m_PenBlue;
	CPen*			m_PenLtGray;
	CPen*			m_PenMagenta;

	INT16			m_x0;
	INT16			m_y0;
	INT16			m_z0;
	double		m_movex;
	double		m_movey;
	double		m_movez;
	double		m_sizex;
	double		m_sizey;
	double		m_sizez;
	double		m_spinx;
	double		m_spiny;
	double		m_spinz;
	double		m_DepthPerception;
	CMatrix		m_Matrix;

	double		M[4][4];  /* 4x4 matrix used in coordinate transformation */
	double		IM[4][4]; /* inverse matrix of M[4][4] */
	double		depth_perception;
	APOINT		m_viewPoints[MAX_VERTICES3];

	int			m_MouseState;
	CPoint		m_LastMousePoint;
#endif
// Attributes
public:
	CDlcDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMineView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMineView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CMineView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in dlcView.cpp
inline CDlcDoc* CMineView::GetDocument()
   { return (CDlcDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLCVIEW_H__C46B3F4C_5EFF_11D2_AE2A_00C0F03014A5__INCLUDED_)
