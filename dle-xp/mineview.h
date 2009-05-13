// dlcView.h : interface of the CMineView class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __mineview_h
#define __mineview_h

#ifdef NDEBUG
# define OGL_RENDERING 0
#else
# define OGL_RENDERING 0
#endif

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "afxcview.h"
#include "DlcDoc.h"
#include "Matrix.h"
#include "poly.h"
#include "textures.h"

#if OGL_RENDERING
# include <gl\gl.h>
# include <gl\glu.h>
# include <gl\glaux.h>
#endif

enum eObjectViewFlags {
	eViewObjectsNone          = 0,
	eViewObjectsRobots        = (1<<0),
	eViewObjectsPlayers		  = (1<<1),
	eViewObjectsWeapons       = (1<<2),
	eViewObjectsPowerups      = (1<<3),
	eViewObjectsKeys          = (1<<4),
	eViewObjectsHostages      = (1<<5),
	eViewObjectsControlCenter = (1<<6),
	eViewObjectsAll           = 0x7f
	};

enum eMineViewFlags {
	eViewMineWalls            = (1<<0),
	eViewMineSpecial          = (1<<1),
	eViewMineLights           = (1<<2),
	eViewMineShading          = (1<<3),
	eViewMineDeltaLights		  = (1<<4),
	eViewMineUsedTextures	  = (1<<5),
	eViewMineSkyBox			  = (1<<6)
	};

enum eViewOptions {
	eViewAllLines = 0,
	eViewHideLines,
	eViewNearbyCubeLines,
	eViewPartialLines,
	eViewTextureMapped
	};

enum eSelectModes {
	eSelectPoint	= POINT_MODE,
	eSelectLine		= LINE_MODE,
	eSelectSide		= SIDE_MODE,
	eSelectCube		= CUBE_MODE,
	eSelectObject	= OBJECT_MODE,
	eSelectBlock	= BLOCK_MODE
	};

enum eMouseStates
{
	eMouseStateIdle,
	eMouseStateButtonDown,
	eMouseStateSelect,
	eMouseStatePan,
	eMouseStateRotate,
	eMouseStateZoom,
	eMouseStateZoomIn,
	eMouseStateZoomOut,
	eMouseStateInitDrag,
	eMouseStateDrag,
	eMouseStateRubberBand,
	eMouseStateCount	//must always be last tag
};

                        /*--------------------------*/
                        
class CMineView : public CView
{
protected: // create from serialization only
	CMineView();
	DECLARE_DYNCREATE(CMineView)

	CSplitterWnd	*m_pSplitter;
	// member variables
	CMine			*m_mine;
	int			m_viewHeight;	// in pixels
	int			m_viewWidth;	// in pixels
	int			m_viewDepth;	// in bytes
	HBITMAP		m_DIB;
	void			*m_pvBits;
	CDC			m_DC;
	CDC			*m_pDC; // if all goes well, this is set to &m_DC
	bool			m_bUpdate;
	bool			m_bUpdateCursor;
	bool			m_bDelayRefresh;
	int			m_nDelayRefresh;
	UINT32		m_viewObjectFlags;
	UINT32		m_viewMineFlags;
	UINT32		m_viewOption;
	UINT32		m_selectMode;
	HCURSOR		m_hCursors [eMouseStateCount];

	CDSelection	*m_Current;

	CPen*			m_penCyan;
	CPen*			m_penRed;
	CPen*			m_penMedRed;
	CPen*			m_penGray;
	CPen*			m_penGreen;
	CPen*			m_penDkGreen;
	CPen*			m_penDkCyan;
	CPen*			m_penYellow;
	CPen*			m_penOrange;
	CPen*			m_penBlue;
	CPen*			m_penMedBlue;
	CPen*			m_penLtBlue;
	CPen*			m_penLtGray;
	CPen*			m_penMagenta;
	CPen*			m_penHiCyan;
	CPen*			m_penHiRed;
	CPen*			m_penHiGray;
	CPen*			m_penHiGreen;
	CPen*			m_penHiDkGreen;
	CPen*			m_penHiDkCyan;
	CPen*			m_penHiYellow;
	CPen*			m_penHiOrange;
	CPen*			m_penHiBlue;
	CPen*			m_penHiLtGray;
	CPen*			m_penHiMagenta;

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
	double		m_depthPerception;
	CMatrix		m_matrix;

	double		M[4][4];  /* 4x4 matrix used in coordinate transformation */
	double		IM[4][4]; /* inverse matrix of M[4][4] */
//	double		depth_perception;
	APOINT		m_viewPoints [MAX_VERTICES3];
	APOINT		m_minViewPoint;
	APOINT		m_maxViewPoint;
	APOINT		m_minVPIdx;
	APOINT		m_maxVPIdx;

	int			m_mouseState;
	int			m_lastMouseState;
	CPoint		m_lastMousePos;
	CPoint		m_clickPos, 
					m_releasePos,
					m_lastDragPos,
					m_highlightPos;
	UINT			m_clickState,
					m_releaseState;
	INT16			m_lastSegment;
	CRect			m_rubberRect;
	UINT			m_lightTimer;
	UINT			m_selectTimer;
	int			m_nFrameRate;
	int			m_bShowLightSource;
	bool			m_bHScroll,
					m_bVScroll;
	int			m_xScrollRange,
					m_yScrollRange;
	int			m_xScrollCenter,
					m_yScrollCenter;
	int			m_xRenderOffs,
					m_yRenderOffs;
	int			m_nViewDist;
	int			m_nMineCenter;

#if OGL_RENDERING
	HGLRC           m_glRC; // Permanent Rendering Context
	CDC             *m_glDC; // Private GDI Device Context
#endif
// Attributes
public:
	CDlcDoc* GetDocument();
	inline CMine *GetMine () 
		{ return m_mine = GetDocument () ? GetDocument ()->m_mine : NULL; }

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
	void Reset ();
	virtual ~CMineView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	inline void SetViewDist (int nViewDist) {
		if (m_nViewDist != nViewDist) {
			m_nViewDist = nViewDist;
			Refresh ();
			}
		}
	inline int ViewDist (void) {
		return (m_nViewDist <= 10) ? m_nViewDist : 
		(m_nViewDist < 20) ? 10 + 2 * (m_nViewDist - 10) : 30 + 3 * (m_nViewDist - 20);
		}
	inline bool Visible (CDSegment *segP) {
		if ((segP->special == SEGMENT_IS_SKYBOX) && !ViewFlag (eViewMineSkyBox))
			return false;
		if (!m_nViewDist)
			return true;
		return (segP->seg_number >= 0) && (segP->seg_number <= ViewDist ()); 
		}
	void DrawMineCenter (CDC *pViewDC);
	bool VertexVisible (int v);
	void SetViewPoints (CRect *pRC = NULL, bool bSetViewInfo = true);
	void ShiftViewPoints ();
	// drawing functions
	void	InitView(CDC* pViewDC);
	bool	UpdateScrollBars (void);
	void	ClearView();
	void	ResetView (bool bRefresh = false);
	bool	InitViewDimensions (void);
	void	DrawWireFrame(CMine *mine, bool bPartial);
	void	DrawTextureMappedCubes(CMine *mine);
	void	DrawMarkedCubes (CMine *mine, INT16 clear_it = 0);
	void	DrawCube(CDSegment *seg, bool bPartial);
	void	DrawCube (CMine *mine, INT16 segnum,INT16 sidenum, INT16 linenum, INT16 pointnum, INT16 clear_it = 0);
	void	DrawCubePartial (CDSegment *seg);
	void	DrawCubeQuick(CDSegment *seg, bool bPartial = false);
	void	DrawCubeTextured(CDSegment *seg, UINT8* light_index);
	void	DrawCubePoints (CDSegment *seg);

	void	DrawCurrentCube(CDSegment *seg, bool bPartial);
	void	DrawLine(CDSegment *seg,INT16 vert1,INT16 vert2);
	void	DrawLine (CDTexture *pTx, POINT pt0, POINT pt1, UINT8 color);
	void	DrawAnimDirArrows (INT16 texture1, CDTexture *pTx);

	void	DrawWalls (CMine *mine);
	void	DrawLights (CMine *mine);
	void	DrawOctagon(CMine *mine, INT16 sidenum, INT16 segnum);
	void	DrawObject (CMine *mine,INT16 objnum,INT16 clear_it = 0);
	void	DrawObjects (CMine *mine, INT16 clear_it = 0);
	void	DrawHighlight (CMine *mine, INT16 clear_it = 0);
	void  DrawSpline (CMine *mine);

	int	SetupModel(CMine *mine, CDObject *obj); // poly.c
	void	SetModelPoints(int start, int end);
	void	DrawModel();      // poly.c
	void	InterpModelData(UINT8 *model_data); // poly.c
	void	DrawPoly(POLY *p);
	int	ReadModelData(FILE *file, CDObject *obj);

	// view control functions
	void	Zoom(int nSteps, double zoom);
	int	ZoomFactor (int nSteps, double min, double max);
	int	ZoomIn(int nSteps = 1, bool bSlow = false);
	int	ZoomOut(int nSteps = 1, bool bSlow = false);
	int	FitToView (void);
	void	Rotate(char direction, double angle);
	void	Pan(char direction, INT32 amount);
	void	AlignSide();
	void	CenterMine();
	void	CenterCube();
	void	CenterObject();
	void	SetViewOption(eViewOptions option);
	void	ToggleViewMine(eMineViewFlags flag);
	void	ToggleViewObjects(eObjectViewFlags mask);
	void	SetViewMineFlags(UINT32 mask);
	void	SetViewObjectFlags(UINT32 mask);
	void	SetSelectMode(UINT32 mode);
	void CalcSegDist (CMine *mine);
	bool	InRange (INT16 *pv, INT16 i);

	void NextPoint (int dir = 1);
	void PrevPoint ();
	void NextLine (int dir = 1);
	void PrevLine ();
	void NextSide (int dir = 1);
	void PrevSide ();
	void NextSide2 (int dir = 1);
	void PrevSide2 ();
	void NextCube (int dir = 1);
	void PrevCube ();
	void ForwardCube (int dir = 1);
	void BackwardsCube ();
	void SelectOtherCube ();
	bool SelectOtherSide ();
	void NextObject (int dir = 1);
	void PrevObject ();
	void NextCubeElement (int dir = 1);
	void PrevCubeElement ();
	void HiliteTarget (CMine *mine);

	void Refresh (bool bAll = true);
	void EnableDeltaShading (int bEnable, int nFrameRate, int bShowLightSource);
	void AdvanceLightTick (void);
	bool SetLightStatus (void);
	void Invalidate (BOOL bErase);
	void InvalidateRect (LPCRECT lpRect, BOOL bErase);

	bool ViewObject (CDObject *obj);
	inline bool ViewObject (UINT32 flag = 0)
		{ return flag ? ((m_viewObjectFlags & flag) != 0) : (m_viewObjectFlags != 0); }
	inline bool ViewFlag (UINT32 flag = 0)
		{ return flag ? (m_viewMineFlags & flag) != 0 : (m_viewMineFlags != 0); }
	inline bool ViewOption (UINT32 option)
		{ return m_viewOption == option; }
	inline bool SelectMode (UINT32 mode)
		{ return m_selectMode == mode; }
	inline UINT32 GetMineViewFlags ()
		{ return m_viewMineFlags; }
	inline UINT32 GetObjectViewFlags ()
		{ return m_viewObjectFlags; }
	inline UINT32 GetViewOptions ()
		{ return m_viewOption; }
	inline UINT32 GetSelectMode ()
		{ return m_selectMode; }
	inline int *MineCenter (void)
		{ return &m_nMineCenter; }
	inline double &DepthPerception (void)
		{ return m_depthPerception; }
	inline void DelayRefresh (bool bDelay) {
		if (bDelay)
			m_nDelayRefresh++;
		else if (m_nDelayRefresh)
			m_nDelayRefresh--;
		}

	void SetMouseState (int newMouseState);
	BOOL SetCursor (HCURSOR hCursor);
//	void UpdateCursor (void);

	inline void wrap (INT16 *x, INT16 delta,INT16 min,INT16 max) {
		*x += delta;
		if (*x > max)
			*x = min;
		else if (*x < min)
			*x = max;
		}

	void SelectCurrentSegment(INT16 direction,long mouse_x, long mouse_y);
	void SelectCurrentObject(long mouse_x, long mouse_y);
	void CalcSegmentCenter(vms_vector &pos,INT16 segnum);
	void RefreshObject(INT16 old_object, INT16 new_object);
	void MarkRubberBandedVertices (void);
	BOOL DrawRubberBox ();
	void UpdateRubberRect (CPoint pt);
	void ResetRubberRect ();
	BOOL UpdateDragPos ();
	void HighlightDrag (CMine *mine, INT16 nVert, long x, long y);
	BOOL CMineView::DrawDragPos (void);
	void FinishDrag (void);

	BOOL SetWindowPos(const CWnd *pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags);

#if OGL_RENDERING
	BOOL GLInit (GLvoid);
	BOOL GLInitPalette (GLvoid);
	GLvoid GLReset (GLvoid);
	GLvoid GLFitToView (GLvoid);
	BOOL GLResizeScene (GLvoid);
	BOOL GLRenderScene (GLvoid);
	void GLRenderFace (INT16 segnum, INT16 sidenum);
	void GLRenderTexture (INT16 segnum, INT16 sidenum, INT16 nTexture);
	void GLCreateTexture (INT16 nTexture);
	GLvoid GLKillWindow (GLvoid);
	BOOL GLCreateWindow (CDC *pDC = NULL);
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CMineView)
	afx_msg void OnDestroy ();
	afx_msg void OnTimer (UINT nIdEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnPaint ();
	afx_msg BOOL OnMouseWheel (UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSelectPrevTab ();
	afx_msg void OnSelectNextTab ();
	afx_msg void OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);
	afx_msg void OnVScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);
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

#endif //__mineview_h
