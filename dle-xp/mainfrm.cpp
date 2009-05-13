
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "dlc.h"

#include "MainFrm.h"
#include "global.h"
#include "toolview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

double gRotateRate  = 3.1415927f / 32.0f;
INT32 gMoveRate   = 10;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_COMMAND_EX(ID_VIEW_TOOLBAR, OnBarCheck)
	ON_COMMAND(ID_TOOLS_EDITORTOOLBAR,OnEditorToolbar)
	ON_COMMAND(ID_FILE_INFORMATION, OnCheckMine)
	ON_COMMAND(ID_FILE_EXTBLKFMT, OnExtBlkFmt)
	ON_COMMAND(ID_FILE_CHECKMINE, OnCheckMine)
	ON_COMMAND(ID_FILE_CONVERT, OnConvert)
	ON_COMMAND(ID_EDIT_UNDO, OnUndo)
	ON_COMMAND(ID_EDIT_REDO, OnRedo)
	ON_COMMAND(ID_EDITGEO_FWD, OnEditGeoFwd)
	ON_COMMAND(ID_EDITGEO_UP, OnEditGeoUp)
	ON_COMMAND(ID_EDITGEO_BACK, OnEditGeoBack)
	ON_COMMAND(ID_EDITGEO_LEFT, OnEditGeoLeft)
	ON_COMMAND(ID_EDITGEO_GROW, OnEditGeoGrow)
	ON_COMMAND(ID_EDITGEO_RIGHT, OnEditGeoRight)
	ON_COMMAND(ID_EDITGEO_ROTLEFT, OnEditGeoRotLeft)
	ON_COMMAND(ID_EDITGEO_DOWN, OnEditGeoDown)
	ON_COMMAND(ID_EDITGEO_ROTRIGHT, OnEditGeoRotRight)
	ON_COMMAND(ID_EDITGEO_SHRINK, OnEditGeoShrink)
	ON_COMMAND(ID_VIEW_REDRAW, OnRedraw)
	ON_COMMAND(ID_EDIT_MARK, OnEditMark)
	ON_COMMAND(ID_EDIT_MARKALL, OnEditMarkAll)
	ON_COMMAND(ID_EDIT_UNMARKALL, OnEditUnmarkAll)
	ON_COMMAND(ID_EDIT_TEXTURE, OnEditTexture)
	ON_COMMAND(ID_EDIT_CUBE, OnEditCube)
	ON_COMMAND(ID_EDIT_WALL, OnEditWall)
	ON_COMMAND(ID_EDIT_OBJECT, OnEditObject)
	ON_COMMAND(ID_EDIT_LIGHT, OnEditLight)
	ON_COMMAND(ID_EDIT_PREFS, OnEditPrefs)
	ON_COMMAND(ID_VIEW_ZOOMIN, OnViewZoomin)
	ON_COMMAND(ID_VIEW_ZOOMOUT, OnViewZoomout)
	ON_COMMAND(ID_VIEW_FITTOVIEW, OnViewFitToView)
	ON_COMMAND(ID_VIEW_TOGGLEVIEWS, OnToggleViews)
	ON_COMMAND(ID_VIEW_TOGGLETEXPANE, OnToggleTexturePane)
	ON_COMMAND(ID_FILE_PREFERENCES, OnEditPrefs)
	ON_COMMAND(ID_FILE_EDITMISSIONFILE, OnEditMission)
	ON_COMMAND(ID_VIEW_ALIGNSIDEROTATION, OnViewAlignsiderotation)
	ON_COMMAND(ID_VIEW_ALLLINES, OnViewAlllines)
	ON_COMMAND(ID_VIEW_CENTERENTIREMINE, OnViewCenterentiremine)
	ON_COMMAND(ID_VIEW_CENTERONCURRENTCUBE, OnViewCenteroncurrentcube)
	ON_COMMAND(ID_VIEW_CENTERONCURRENTOBJECT, OnViewCenteroncurrentobject)
	ON_COMMAND(ID_VIEW_HIDELINES, OnViewHidelines)
	ON_COMMAND(ID_VIEW_LIGHTS, OnViewLights)
	ON_COMMAND(ID_VIEW_NEARBYCUBELINES, OnViewNearbycubelines)
	ON_COMMAND(ID_VIEW_OBJECTS_ALLOBJECTS, OnViewObjectsAllobjects)
	ON_COMMAND(ID_VIEW_OBJECTS_CONTROLCENTER, OnViewObjectsControlcenter)
	ON_COMMAND(ID_VIEW_OBJECTS_HOSTAGES, OnViewObjectsHostages)
	ON_COMMAND(ID_VIEW_OBJECTS_KEYS, OnViewObjectsKeys)
	ON_COMMAND(ID_VIEW_OBJECTS_NOOBJECTS, OnViewObjectsNoobjects)
	ON_COMMAND(ID_VIEW_OBJECTS_PLAYERS, OnViewObjectsPlayers)
	ON_COMMAND(ID_VIEW_OBJECTS_POWERUPS, OnViewObjectsPowerups)
	ON_COMMAND(ID_VIEW_OBJECTS_ROBOTS, OnViewObjectsRobots)
	ON_COMMAND(ID_VIEW_OBJECTS_WEAPONS, OnViewObjectsWeapons)
	ON_COMMAND(ID_VIEW_PAN_DOWN, OnViewPanDown)
	ON_COMMAND(ID_VIEW_PAN_IN, OnViewPanIn)
	ON_COMMAND(ID_VIEW_PAN_LEFT, OnViewPanLeft)
	ON_COMMAND(ID_VIEW_PAN_OUT, OnViewPanOut)
	ON_COMMAND(ID_VIEW_PAN_RIGHT, OnViewPanRight)
	ON_COMMAND(ID_VIEW_PAN_UP, OnViewPanUp)
	ON_COMMAND(ID_VIEW_PARTIALLINES, OnViewPartiallines)
	ON_COMMAND(ID_VIEW_ROTATE_CLOCKWISE, OnViewRotateClockwise)
	ON_COMMAND(ID_VIEW_ROTATE_COUNTERCLOCKWISE, OnViewRotateCounterclockwise)
	ON_COMMAND(ID_VIEW_ROTATE_LEFT, OnViewRotateHorizontallyleft)
	ON_COMMAND(ID_VIEW_ROTATE_RIGHT, OnViewRotateHorizontallyright)
	ON_COMMAND(ID_VIEW_ROTATE_DOWN, OnViewRotateVerticallydown)
	ON_COMMAND(ID_VIEW_ROTATE_UP, OnViewRotateVerticallyup)
	ON_COMMAND(ID_VIEW_SHADING, OnViewShading)
	ON_COMMAND(ID_VIEW_DELTALIGHTS, OnViewDeltaLights)
	ON_COMMAND(ID_VIEW_SPECIAL, OnViewSpecial)
	ON_COMMAND(ID_VIEW_TEXTUREMAPPED, OnViewTexturemapped)
	ON_COMMAND(ID_VIEW_WALLS, OnViewWalls)
	ON_COMMAND(ID_VIEW_ALLTEXTURES, OnViewUsedTextures)
	ON_COMMAND(ID_SEL_PREVTAB, OnSelectPrevTab)
	ON_COMMAND(ID_SEL_NEXTTAB, OnSelectNextTab)
	ON_COMMAND(ID_SEL_POINTMODE, OnSelectPointMode)
	ON_COMMAND(ID_SEL_LINEMODE, OnSelectLineMode)
	ON_COMMAND(ID_SEL_SIDEMODE, OnSelectSideMode)
	ON_COMMAND(ID_SEL_CUBEMODE, OnSelectCubeMode)
	ON_COMMAND(ID_SEL_OBJECTMODE, OnSelectObjectMode)
	ON_COMMAND(ID_SEL_BLOCKMODE, OnSelectBlockMode)
	ON_COMMAND(ID_JOIN_POINTS, OnJoinPoints)
	ON_COMMAND(ID_SPLIT_POINTS, OnSplitPoints)
	ON_COMMAND(ID_JOIN_LINES, OnJoinLines)
	ON_COMMAND(ID_SPLIT_LINES, OnSplitLines)
	ON_COMMAND(ID_JOIN_SIDES, OnJoinSides)
	ON_COMMAND(ID_SPLIT_SIDES, OnSplitSides)
	ON_COMMAND(ID_JOIN_CURRENTSIDE, OnJoinCurrentSide)
	ON_COMMAND(ID_SPLIT_CURRENTSIDE, OnSplitCurrentSide)
	ON_COMMAND(ID_INSMODE_NORMAL, OnInsModeNormal)
	ON_COMMAND(ID_INSMODE_EXTEND, OnInsModeExtend)
	ON_COMMAND(ID_INSMODE_MIRROR, OnInsModeMirror)
	ON_COMMAND(ID_TOGGLE_INSMODE, OnToggleInsMode)
	ON_COMMAND(ID_SEL_NEXTPOINT, OnSelNextPoint)
	ON_COMMAND(ID_SEL_PREVPOINT, OnSelPrevPoint)
	ON_COMMAND(ID_SEL_NEXTLINE, OnSelNextLine)
	ON_COMMAND(ID_SEL_PREVLINE, OnSelPrevLine)
	ON_COMMAND(ID_SEL_NEXTSIDE, OnSelNextSide)
	ON_COMMAND(ID_SEL_PREVSIDE, OnSelPrevSide)
	ON_COMMAND(ID_SEL_NEXTCUBE, OnSelNextCube)
	ON_COMMAND(ID_SEL_PREVCUBE, OnSelPrevCube)
	ON_COMMAND(ID_SEL_NEXTOBJECT, OnSelNextObject)
	ON_COMMAND(ID_SEL_PREVOBJECT, OnSelPrevObject)
	ON_COMMAND(ID_SEL_FWDCUBE, OnSelFwdCube)
	ON_COMMAND(ID_SEL_BACKCUBE, OnSelBackCube)
	ON_COMMAND(ID_SEL_OTHERCUBE, OnSelOtherCube)
	ON_COMMAND(ID_SEL_OTHERSIDE, OnSelOtherSide)
	ON_COMMAND(ID_SEL_NEXTCUBEELEM, OnSelNextCubeElem)
	ON_COMMAND(ID_SEL_PREVCUBEELEM, OnSelPrevCubeElem)
	ON_COMMAND(ID_TOOLS_TEXTUREEDIT, OnEditTexture)
	ON_COMMAND(ID_TOOLS_POINTEDIT, OnEditCube)
	ON_COMMAND(ID_TOOLS_CUBEEDIT, OnEditCube)
	ON_COMMAND(ID_TOOLS_OBJECTEDIT, OnEditObject)
	ON_COMMAND(ID_TOOLS_EFFECTEDIT, OnEditEffect)
	ON_COMMAND(ID_TOOLS_WALLEDIT, OnEditWall)
	ON_COMMAND(ID_TOOLS_TRIGGEREDIT, OnEditTrigger)
	ON_COMMAND(ID_TOOLS_LIGHTADJUSTMENT, OnEditLight)
	ON_COMMAND(ID_TOOLS_ALIGNTEXTURE, OnEditTexture)
	ON_COMMAND(ID_TOOLS_REACTORTRIGGERS, OnEditReactor)
	ON_COMMAND(ID_TOOLS_CURVEGENERATOR, OnTunnelGenerator)
	ON_COMMAND(ID_TOOLS_TXTFILTER, OnTxtFilters)
	ON_COMMAND(ID_SPLINE_INCREASE, OnIncSpline)
	ON_COMMAND(ID_SPLINE_DECREASE, OnDecSpline)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOGGLEVIEWS, OnUpdateToggleViews)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOGGLETEXPANE, OnUpdateToggleTexPane)
	ON_UPDATE_COMMAND_UI(ID_FILE_EXTBLKFMT, OnUpdateExtBlkFmt)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_EDITORTOOLBAR,OnUpdateEditorToolbar)
	ON_UPDATE_COMMAND_UI(ID_INSMODE_NORMAL, OnUpdateInsModeNormal)
	ON_UPDATE_COMMAND_UI(ID_INSMODE_EXTEND, OnUpdateInsModeExtend)
	ON_UPDATE_COMMAND_UI(ID_INSMODE_MIRROR, OnUpdateInsModeMirror)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ALLTEXTURES, OnUpdateViewUsedTextures)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WALLS, OnUpdateViewWalls)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SPECIAL, OnUpdateViewSpecial)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHADING, OnUpdateViewShading)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DELTALIGHTS, OnUpdateViewDeltaLights)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PARTIALLINES, OnUpdateViewPartiallines)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECTS_WEAPONS, OnUpdateViewObjectsWeapons)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECTS_ROBOTS, OnUpdateViewObjectsRobots)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECTS_POWERUPS, OnUpdateViewObjectsPowerups)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECTS_PLAYERS, OnUpdateViewObjectsPlayers)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECTS_NOOBJECTS, OnUpdateViewObjectsNoobjects)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECTS_KEYS, OnUpdateViewObjectsKeys)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECTS_HOSTAGES, OnUpdateViewObjectsHostages)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECTS_CONTROLCENTER, OnUpdateViewObjectsControlcenter)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECTS_ALLOBJECTS, OnUpdateViewObjectsAllobjects)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NEARBYCUBELINES, OnUpdateViewNearbycubelines)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LIGHTS, OnUpdateViewLights)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HIDELINES, OnUpdateViewHidelines)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ALLLINES, OnUpdateViewAlllines)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXTUREMAPPED, OnUpdateViewTexturemapped)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR
/*
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
*/
	};

#define TOOLBAR_STYLE   WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_GRIPPER | CBRS_SIZE_DYNAMIC
#define DOCKING_STYLE	CBRS_ALIGN_ANY //| CBRS_FLOAT_MULTI

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CExtToolBar, CToolBar)
	ON_WM_LBUTTONDOWN ()
	ON_WM_LBUTTONUP ()
	ON_WM_MOUSEMOVE ()
	ON_WM_TIMER ()
//	ON_WM_SETFOCUS ()
//	ON_WM_KILLFOCUS ()
//	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
//	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
END_MESSAGE_MAP()

								/*---------------------------*/

CExtToolBar::CExtToolBar ()
{
m_nId = -1;
m_nState = WM_LBUTTONUP;
m_nPos = -1;
m_nTimer = -1;
}

								/*---------------------------*/

void CExtToolBar::Notify (UINT nMsg)
{
theApp.MainFrame ()->SendMessage (WM_COMMAND, nMsg, NULL);
}

								/*---------------------------*/

void CExtToolBar::OnLButtonDown (UINT nFlags, CPoint point)
{
   CRect rc;
   int   h, i;

GetWindowRect (rc);
for (i = 0, h = GetToolBarCtrl ().GetButtonCount (); i < h; i++) {
   GetItemRect (i, rc);
	if ((point.x >= rc.left) && (point.x < rc.right) && (point.y >= rc.top) && (point.y < rc.bottom)) {
		if ((i >= 11) && (i <= 23) && (i != 13)) {
			m_nId = i;
			m_nTimer = SetTimer (1, m_nTimerDelay = 250U, NULL);
			}
		break;
		}
	}
CToolBar::OnLButtonDown (nFlags, point);
}

								/*---------------------------*/

void CExtToolBar::OnLButtonUp (UINT nFlags, CPoint point)
{
if (m_nId >= 0) {
	m_nId = -1;
	if (m_nTimer >= 0) {
		KillTimer (m_nTimer);
		m_nTimer = -1;
		}
	}
CToolBar::OnLButtonUp (nFlags, point);
}

								/*---------------------------*/

afx_msg void CExtToolBar::OnMouseMove (UINT nFlags, CPoint point)
{
if (nFlags & MK_LBUTTON) {
	CRect	rc;

	GetItemRect (m_nId, rc);
	if ((point.x < 0) || (point.y < 0) || (point.x >= rc.right) || (point.y >= rc.bottom)) {
		if (m_nPos == 1) {
			m_nPos = 0;
			if (m_nTimer >= 0) {
				KillTimer (m_nTimer);
				m_nTimer = -1;
				}
			}
		}
	else {
		if (m_nPos != 1) {
			m_nPos = 1;
			m_nTimer = SetTimer (1, m_nTimerDelay = 250U, NULL);
			}
		}
	}
CToolBar::OnMouseMove (nFlags, point);
}

								/*---------------------------*/

int CExtToolBar::Width ()
{
   CRect rc;
   int   h, i, dx;

GetWindowRect (rc);
dx = 0;
for (i = 0, h = GetToolBarCtrl ().GetButtonCount (); i < h; i++) {
   GetItemRect (i, rc);
   dx += rc.Width ();
//	if (GetButtonStyle (i) != TBBS_SEPARATOR)
//      ++dx;
   }
return dx;// - 4;
}

                        /*--------------------------*/

void CExtToolBar::OnTimer (UINT nIdEvent)
{
if (nIdEvent == 1) {
	switch (m_nId) {
		case 11:
			Notify (ID_VIEW_ZOOMIN);
			break;
		case 12:
			Notify (ID_VIEW_ZOOMOUT);
			break;
		case 14:
			Notify (ID_VIEW_PAN_LEFT);
			break;
		case 15:
			Notify (ID_VIEW_PAN_RIGHT);
			break;
		case 16:
			Notify (ID_VIEW_PAN_UP);
			break;
		case 17:
			Notify (ID_VIEW_PAN_DOWN);
			break;
		case 18:
			Notify (ID_VIEW_ROTATE_RIGHT);
			break;
		case 19:
			Notify (ID_VIEW_ROTATE_LEFT);
			break;
		case 20:
			Notify (ID_VIEW_ROTATE_DOWN);
			break;
		case 21:
			Notify (ID_VIEW_ROTATE_UP);
			break;
		case 22:
			Notify (ID_VIEW_ROTATE_CLOCKWISE);
			break;
		case 23:
			Notify (ID_VIEW_ROTATE_COUNTERCLOCKWISE);
			break;
		default:
			break;
		}
	}
UINT i = (m_nTimerDelay * 9) / 10;
if (i >= 25) {
	KillTimer (m_nTimer);
	m_nTimer = SetTimer (1, m_nTimerDelay = i, NULL);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
m_mineView = NULL;
m_textureView = NULL;
m_toolView = NULL;
m_paneMode = 0;
m_bShowCtrlBar = 1;
m_bRecalcBarLayout = 0;
m_texPaneWidth = -1;
m_toolMode = 1;
m_textureMode = 1;
m_mineZoom = 0;
nLayout = GetPrivateProfileInt ("DLE-XP", "Layout", 0, "dle-xp.ini");
#if EDITBAR
m_bEditorTB = 1;
#else
m_bEditorTB = 0;
m_pEditTool = NULL;
#endif

}

CMainFrame::~CMainFrame()
{
}

void CMainFrame::OnClose ()
{
#if EDITBAR == 0
if (m_bEditorTB)
	OnEditorToolbar ();
#endif
theApp.SaveLayout ();
ToolView ()->PrefsDlg ()->SaveAppSettings ();
CFrameWnd::OnClose ();
}


void CMainFrame::OnSize (UINT nType, int cx, int cy)
{
CFrameWnd::OnSize (nType, cx, cy);
if (ToolView ())
	ToolView ()->RecalcLayout (m_toolMode, m_textureMode);
RecalcLayout (m_toolMode, m_textureMode);
}


int CMainFrame::CreateToolBars (bool bToolBar, bool bEditBar)
{
EnableDocking (DOCKING_STYLE);
if (bToolBar && !IsWindow (m_toolBar.m_hWnd)) {
	if (!m_toolBar.CreateEx (this, TBSTYLE_FLAT, TOOLBAR_STYLE) || 
		 !m_toolBar.LoadToolBar(IDR_MAIN_TOOLBAR)) {
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
		}
	m_toolBar.EnableDocking (DOCKING_STYLE);
	DockControlBar (&m_toolBar);
	}
#if EDITBAR
if (bEditBar && !IsWindow (m_editBar.m_hWnd)) {
	if (!m_editBar.CreateEx (this, TBSTYLE_FLAT, TOOLBAR_STYLE) ||
		 !m_editBar.LoadToolBar(IDR_EDIT_TOOLBAR)) {
		TRACE0("Failed to create edit toolbar\n");
		return -1;      // fail to create
		}
	m_editBar.EnableDocking (DOCKING_STYLE);
	//DockControlBar (&m_editBar);
	}
#endif
return 0;
}


void CMainFrame::DockToolBars (bool bToolBar, bool bEditBar)
{
if (bToolBar) {
	m_toolBar.EnableDocking (DOCKING_STYLE);
	DockControlBar (&m_toolBar);
	}
#if EDITBAR
if (bEditBar) {
	m_editBar.EnableDocking (DOCKING_STYLE);
	DockControlBar (&m_editBar);
	}
#endif
}

int CMainFrame::CreateStatusBar ()
{
	UINT nId, nStyle;
	int cxWidth;

if (!m_statusBar.Create (this) || 
	 !m_statusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT))) {
	TRACE0("Failed to create status bar\n");
	return -1;      // fail to create
	}
m_statusBar.GetPaneInfo (0, nId, nStyle, cxWidth);
m_statusBar.SetPaneInfo (0, nId, SBPS_NORMAL, 50);
m_statusBar.GetPaneInfo (1, nId, nStyle, cxWidth);
m_statusBar.SetPaneInfo (1, nId, SBPS_NORMAL, 600);
m_statusBar.GetPaneInfo (2, nId, nStyle, cxWidth);
m_statusBar.SetPaneInfo (2, nId, SBPS_STRETCH | SBPS_NORMAL, cxWidth);
m_statusBar.GetPaneInfo (3, nId, nStyle, cxWidth);
m_statusBar.SetPaneInfo (3, nId, SBPS_NORMAL, 75);
m_statusBar.GetPaneInfo (4, nId, nStyle, cxWidth);
m_statusBar.SetPaneInfo (4, nId, SBPS_NORMAL, 75);
return 0;
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
	return -1;
if (CreateToolBars ())
	return -1;
if (CreateStatusBar ())
	return -1;
UpdateInsModeButtons (add_segment_mode);
return 0;
}


BOOL CMainFrame::OnBarCheck(UINT nID)
{
BOOL bCheck = CFrameWnd::OnBarCheck (nID);
CControlBar* pBar = GetControlBar (nID);
m_bShowCtrlBar = pBar && ((pBar->GetStyle () & WS_VISIBLE) != 0);
#if EDITBAR
m_editBar.ShowWindow ((m_bShowCtrlBar && m_bEditorTB) ? SW_SHOW : SW_HIDE);
#endif
if (m_bShowCtrlBar) {
	if (m_bRecalcBarLayout)
	RecalcLayout (1);
	if (!CreateToolBars ())
		FixToolBars ();
	}
else {
	CRect rc;
	m_splitter2.GetPane (1,0)->GetWindowRect (rc);
	m_bRecalcBarLayout = (theApp.ToolSize ().cy == rc.Height () + 10);
	}
return bCheck;
}

void CMainFrame::FixToolBars ()
{
m_toolBar.ShowWindow (SW_SHOW);
#if EDITBAR
	CRect	rc1, rc2;
	CPoint p;

m_toolBar.GetWindowRect (rc1);
GetClientRect (rc2);
rc2.left = rc2.right - rc1.Height ();
p.x = rc2.left;
p.y = rc2.top;
m_editBar.ShowWindow (SW_SHOW);
FloatControlBar (&m_editBar, p, (UINT) AFX_IDW_DOCKBAR_RIGHT);
#endif
}



CMineView* CMainFrame::GetMineView() {
	CView* view = NULL;
	CDocument* doc = GetActiveDocument();
	if (doc)
	{
		POSITION pos = doc->GetFirstViewPosition();
		while (pos != NULL)
		{
			view = (CMineView *)doc->GetNextView(pos);
			if (view->IsKindOf(RUNTIME_CLASS(CMineView))) break;
		}
		ASSERT(view->IsKindOf(RUNTIME_CLASS(CMineView)));
	}
	return (CMineView *)view;
}


CTextureView* CMainFrame::GetTextureView() {
return TextureView ();
}


CToolView* CMainFrame::GetToolView () 
{
return ToolView ();
}


BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

return CFrameWnd::PreCreateWindow(cs);
}

                        /*--------------------------*/

#define CX_TOOLS		354
#define CY_TOOLS		150
#define CX_TEXTURES	170
#define CY_TEXTURES	155

BOOL CMainFrame::OnCreateClient (LPCREATESTRUCT lpcs, CCreateContext * pContext)
{
	struct	CCreateContext context = *pContext;
	CRect		rc;

if (!m_splitter1.CreateStatic (this, 1, 2, WS_CHILD | WS_VISIBLE | WS_BORDER))
	return FALSE;
GetClientRect (rc);
rc.InflateRect (-2, -2);
if (nLayout) {
	context.m_pNewViewClass = RUNTIME_CLASS (CToolView);
	m_splitter1.CreateView (0, 0, RUNTIME_CLASS (CToolView), CSize (CX_TOOLS, rc.Height ()), &context);
	m_splitter1.SetRowInfo (0, rc.Height (), 16);
	m_splitter1.SetColumnInfo (0, CX_TOOLS, 16);
	m_splitter1.SetColumnInfo (1, rc.Width () - CX_TOOLS, 16);
	m_splitter2.CreateStatic (&m_splitter1, 2, 1, WS_CHILD | WS_VISIBLE | WS_BORDER, m_splitter1.IdFromRowCol(0, 1));
	context.m_pNewViewClass = RUNTIME_CLASS (CMineView);
	m_splitter2.CreateView (0, 0, RUNTIME_CLASS (CMineView), CSize (rc.Width (), rc.Height ()), &context);
	context.m_pNewViewClass = RUNTIME_CLASS (CTextureView);
	m_splitter2.CreateView (1, 0, RUNTIME_CLASS (CTextureView), CSize (rc.Width (), 0), &context);
	m_splitter2.SetRowInfo (0, rc.Height () - CY_TEXTURES, 16);
	m_splitter2.SetRowInfo (1, CY_TEXTURES, 16);
	m_splitter2.SetColumnInfo (0, rc.Width (), 0);
	SetActiveView ((CView*) m_splitter2.GetPane (0,0)); 
	m_toolView = (CToolView *) m_splitter1.GetPane (0,0);
	m_mineView = (CMineView *) m_splitter2.GetPane (0,0);
	m_textureView = (CTextureView *) m_splitter2.GetPane (1,0);
	}
else {
	context.m_pNewViewClass = RUNTIME_CLASS (CTextureView);
	m_splitter1.CreateView (0, 0, RUNTIME_CLASS (CTextureView), CSize (CX_TEXTURES, rc.Height ()), &context);
	m_splitter1.SetRowInfo (0, rc.Height (), 16);
	m_splitter1.SetColumnInfo (0, CX_TEXTURES, 16);
	m_splitter1.SetColumnInfo (1, rc.Width () - CX_TEXTURES, 16);
	m_splitter2.CreateStatic (&m_splitter1, 2, 1, WS_CHILD | WS_VISIBLE | WS_BORDER, m_splitter1.IdFromRowCol(0, 1));
	context.m_pNewViewClass = RUNTIME_CLASS (CMineView);
	m_splitter2.CreateView (0, 0, RUNTIME_CLASS (CMineView), CSize (rc.Width (), rc.Height ()), &context);
	context.m_pNewViewClass = RUNTIME_CLASS (CToolView);
	m_splitter2.CreateView (1, 0, RUNTIME_CLASS (CToolView), CSize (rc.Width (), 0), &context);
	m_splitter2.SetRowInfo (0, rc.Height () - CY_TOOLS, 16);
	m_splitter2.SetRowInfo (1, CY_TOOLS, 0);
	m_splitter2.SetColumnInfo (0, rc.Width (), 16);
	SetActiveView ((CView*)m_splitter2.GetPane(0,0)); 
	m_textureView = (CTextureView *) m_splitter1.GetPane (0,0);
	m_mineView = (CMineView *) m_splitter2.GetPane (0,0);
	m_toolView = (CToolView *) m_splitter2.GetPane (1,0);
	}
return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::ResetMineZoom ()
{
if (m_mineZoom) {
	MineView ()->ZoomOut (m_mineZoom);
	m_mineZoom = 0;
	}
}

void CMainFrame::ResetPaneMode ()
{
if (m_paneMode) {
	m_paneMode = 0;
	ResetMineZoom ();
	m_toolBar.SetButtonInfo (44, ID_VIEW_TOGGLEVIEWS, TBBS_BUTTON, 38);
	}
}

void CMainFrame::ShowTools ()
{
RecalcLayout (1,1);
if (m_paneMode == 2)
	ResetMineZoom ();
m_paneMode = 0;
}

void CMainFrame::OnEditMark() 
{
GetMine ()->Mark();
}

void CMainFrame::OnEditMarkAll() 
{
GetMine ()->MarkAll();
}

void CMainFrame::OnEditUnmarkAll() 
{
GetMine ()->UnmarkAll();
}

void CMainFrame::OnEditTexture() 
{
ToolView ()->EditTexture();
}

void CMainFrame::OnEditWall() 
{
ShowTools ();
ToolView ()->EditWall();
}

void CMainFrame::OnEditCube() 
{
ShowTools ();
ToolView ()->EditCube();
}

void CMainFrame::OnEditTrigger() 
{
ShowTools ();
ToolView ()->EditTrigger();
}

void CMainFrame::OnEditObject() 
{
ShowTools ();
ToolView ()->EditObject();
}

void CMainFrame::OnEditEffect() 
{
ShowTools ();
ToolView ()->EditEffect();
}

void CMainFrame::OnEditLight() 
{
ShowTools ();
ToolView ()->EditLight();
ToolView ()->LightTool ()->OnOK ();
}

void CMainFrame::OnEditMission () 
{
ShowTools ();
ToolView()->EditMission ();
}

void CMainFrame::OnEditPrefs () 
{
ShowTools ();
ToolView ()->EditPrefs ();
}

void CMainFrame::OnEditReactor () 
{
ShowTools ();
ToolView ()->EditReactor ();
}

void CMainFrame::OnTunnelGenerator () 
{
ShowTools ();
GetMine ()->TunnelGenerator ();
}

void CMainFrame::OnTxtFilters () 
{
ShowTools ();
ToolView ()->TxtFilter ();
}

void CMainFrame::OnIncSpline () 
{
GetMine ()->IncreaseSpline ();
}

void CMainFrame::OnDecSpline () 
{
GetMine ()->DecreaseSpline ();
}

void CMainFrame::OnRedraw() 
{
MineView ()->Refresh ();
}

void CMainFrame::OnToggleViews() 
{
if (m_paneMode == 2) {
	RecalcLayout (1,1);
	m_paneMode = 0;
	m_toolBar.SetButtonInfo (44, ID_VIEW_TOGGLEVIEWS, TBBS_BUTTON, 38);
	ResetMineZoom ();
	}
else {
	RecalcLayout (2,2);
	m_paneMode = 2;
	m_toolBar.SetButtonInfo (44, ID_VIEW_TOGGLEVIEWS, TBBS_BUTTON, 39);
	m_mineZoom = MineView ()->ZoomIn (2);
	}
}

void CMainFrame::OnToggleTexturePane() 
{
RecalcLayout (m_toolMode, (m_textureMode == 1) ? 2 : 1);
}

void CMainFrame::OnViewZoomin() 
{
	GetMineView()->ZoomIn();
}

void CMainFrame::OnViewZoomout() 
{
	GetMineView()->ZoomOut();
}

void CMainFrame::OnViewFitToView () 
{
	GetMineView()->FitToView ();
}

void CMainFrame::OnViewAlignsiderotation() 
{
	GetMineView()->AlignSide();	
}

void CMainFrame::OnViewCenterentiremine() 
{
	GetMineView()->CenterMine();	
}

void CMainFrame::OnViewCenteroncurrentcube() 
{
	GetMineView()->CenterCube();	
}

void CMainFrame::OnViewCenteroncurrentobject() 
{
	GetMineView()->CenterObject();
}

//////// drawing options ///////////////

void CMainFrame::OnViewAlllines() 
{
	GetMineView()->SetViewOption(eViewAllLines);
}
void CMainFrame::OnViewHidelines() 
{
	GetMineView()->SetViewOption(eViewHideLines);
}
void CMainFrame::OnViewNearbycubelines() 
{
	GetMineView()->SetViewOption(eViewNearbyCubeLines);
}
void CMainFrame::OnViewPartiallines() 
{
	GetMineView()->SetViewOption(eViewPartialLines);
}
void CMainFrame::OnViewTexturemapped() 
{
	GetMineView()->SetViewOption(eViewTextureMapped);
}

////////////// view flags //////////////

void CMainFrame::OnViewLights() 
{
	GetMineView()->ToggleViewMine(eViewMineLights);
}
void CMainFrame::OnViewShading() 
{
	GetMineView()->ToggleViewMine(eViewMineShading);
}
void CMainFrame::OnViewDeltaLights() 
{
	GetMineView()->ToggleViewMine(eViewMineDeltaLights);
}
void CMainFrame::OnViewWalls() 
{
	GetMineView()->ToggleViewMine(eViewMineWalls);
}
void CMainFrame::OnViewSpecial() 
{
	GetMineView()->ToggleViewMine(eViewMineSpecial);
}
void CMainFrame::OnViewUsedTextures() 
{
	GetTextureView ()->ToggleViewFlag(eViewMineUsedTextures);
	GetToolView ()->PrefsDlg ()->Refresh ();
}

void CMainFrame::OnJoinPoints ()
{
GetMine ()->JoinPoints ();
}

void CMainFrame::OnJoinLines ()
{
GetMine ()->JoinLines ();
}

void CMainFrame::OnJoinSides ()
{
GetMine ()->JoinSegments ();
}

void CMainFrame::OnJoinCurrentSide ()
{
GetMine ()->JoinSegments (1);
}

void CMainFrame::OnSplitPoints ()
{
GetMine ()->SplitPoints ();
}

void CMainFrame::OnSplitLines ()
{
GetMine ()->SplitLines ();
}

void CMainFrame::OnSplitSides ()
{
GetMine ()->SplitSegments ();
}

void CMainFrame::OnSplitCurrentSide ()
{
GetMine ()->SplitSegments (1);
}

void CMainFrame::UpdateInsModeButtons (INT16 mode)
{
	static char *szInsMode [] = {" insert: normal", " insert: extend", " insert: mirror"};

for (int i = 0; i <= ID_INSMODE_MIRROR - ID_INSMODE_NORMAL; i++)
	m_toolBar.GetToolBarCtrl ().CheckButton (ID_INSMODE_NORMAL + i, i == mode);
InsModeMsg (szInsMode [mode]);
}

void CMainFrame::SetInsertMode (INT16 mode)
{
add_segment_mode = mode;
UpdateInsModeButtons (mode);
}

void CMainFrame::OnInsModeNormal ()
{
SetInsertMode (ORTHOGONAL);
}

void CMainFrame::OnInsModeExtend ()
{
SetInsertMode (EXTEND);
}

void CMainFrame::OnInsModeMirror ()
{
SetInsertMode (MIRROR);
}

void CMainFrame::OnToggleInsMode ()
{
add_segment_mode = (add_segment_mode + 1) % 3;
UpdateInsModeButtons (add_segment_mode);
}

void CMainFrame::OnEditorToolbar ()
{
m_bEditorTB = !m_bEditorTB;
if (m_bEditorTB) {
	if (m_pEditTool = new CEditTool ()) {
		m_pEditTool->Create (IDD_EDITTOOL, MineView ());
		CRect rc0, rc1, rc2;
		MineView ()->GetClientRect (rc1);
		MineView ()->ClientToScreen (rc1);
		m_pEditTool->GetWindowRect (rc2);
		rc0.left = GetPrivateProfileInt ("DLE-XP", "xEditTB", 0, "dle-xp.ini");
		rc0.top = GetPrivateProfileInt ("DLE-XP", "yEditTB", 0, "dle-xp.ini");
		if (rc0.left < 0)
			rc0.left = 0;
		else if (rc0.left > rc1.right)
			rc0.left = rc1.right - rc2.Width ();
		if (rc0.top < 0)
			rc0.top = 0;
		else if (rc0.top > rc1.bottom)
			rc0.top = rc1.bottom - rc2.Height ();
		rc0.right = rc0.left + rc2.Width ();
		rc0.bottom = rc0.top + rc2.Height ();
		//MineView ()->ClientToScreen (rc0);
		m_pEditTool->MoveWindow (rc0, TRUE);
		m_pEditTool->ShowWindow (SW_SHOW);
		}
	else
		m_bEditorTB = NULL;
	}
else {
	CRect rc;
	m_pEditTool->GetWindowRect (rc);
	theApp.WritePrivateProfileInt ("xEditTB", rc.left);
	theApp.WritePrivateProfileInt ("yEditTB", rc.top);
	m_pEditTool->DestroyWindow ();
	delete m_pEditTool;
	m_pEditTool = NULL;
	}
#if EDITBAR
if (m_bShowCtrlBar && m_bEditorTB)
	CreateToolBars ();
CWnd *pWnd = m_editBar.GetParent ();
m_editBar.GetParent ()->ShowWindow ((m_bShowCtrlBar && m_bEditorTB) ? SW_SHOW : SW_HIDE);
m_editBar.ShowWindow ((m_bShowCtrlBar && m_bEditorTB) ? SW_SHOW : SW_HIDE);
FixToolBars ();
#endif
}

void CMainFrame::OnExtBlkFmt ()
{
bExtBlkFmt = !bExtBlkFmt;
}

void CMainFrame::OnUpdateToggleViews (CCmdUI* pCmdUI)
{
pCmdUI->SetCheck(m_paneMode == 2);
}

void CMainFrame::OnUpdateToggleTexPane (CCmdUI* pCmdUI)
{
pCmdUI->SetCheck(m_textureMode == 1);
}

void CMainFrame::OnUpdateExtBlkFmt (CCmdUI* pCmdUI)
{
pCmdUI->SetCheck(bExtBlkFmt);
}

void CMainFrame::OnUpdateEditorToolbar (CCmdUI* pCmdUI)
{
pCmdUI->SetCheck(m_bEditorTB);
}

void CMainFrame::OnUpdateInsModeNormal (CCmdUI* pCmdUI)
{
pCmdUI->SetCheck(add_segment_mode == ORTHOGONAL);
}

void CMainFrame::OnUpdateInsModeExtend (CCmdUI* pCmdUI)
{
pCmdUI->SetCheck(add_segment_mode == EXTEND);
}

void CMainFrame::OnUpdateInsModeMirror (CCmdUI* pCmdUI)
{
pCmdUI->SetCheck(add_segment_mode == MIRROR);
}

void CMainFrame::UpdateSelectButtons (eSelectModes mode)
{
	static char *szSelMode [] = {" select: point", " select: line", " select: side", " select: cube", " select: object", " select: block"};

for (int i = 0; i <= ID_SEL_BLOCKMODE - ID_SEL_POINTMODE; i++)
	m_toolBar.GetToolBarCtrl ().CheckButton (ID_SEL_POINTMODE + i, i == mode);
SelModeMsg (szSelMode [mode]);
}

void CMainFrame::SetSelectMode (eSelectModes mode)
{
UpdateSelectButtons (mode);
MineView ()->SetSelectMode (mode);
}

void CMainFrame::OnSelectPrevTab ()
{
ShowTools ();
ToolView ()->PrevTab ();
}

void CMainFrame::OnSelectNextTab ()
{
ShowTools ();
ToolView ()->NextTab ();
}

void CMainFrame::OnSelectPointMode ()
{
SetSelectMode (eSelectPoint);
}

void CMainFrame::OnSelectLineMode ()
{
SetSelectMode (eSelectLine);
}

void CMainFrame::OnSelectSideMode ()
{
SetSelectMode (eSelectSide);
}

void CMainFrame::OnSelectCubeMode ()
{
SetSelectMode (eSelectCube);
}

void CMainFrame::OnSelectObjectMode ()
{
SetSelectMode (eSelectObject);
}

void CMainFrame::OnSelectBlockMode ()
{
SetSelectMode (eSelectBlock);
}


///////////// view Objects () //////////////
void CMainFrame::OnViewObjectsAllobjects() 
{
	GetMineView()->SetViewObjectFlags(eViewObjectsAll);
}
void CMainFrame::OnViewObjectsHostages() 
{
	GetMineView()->ToggleViewObjects(eViewObjectsHostages);
}
void CMainFrame::OnViewObjectsKeys() 
{
	GetMineView()->ToggleViewObjects(eViewObjectsKeys);
}
void CMainFrame::OnViewObjectsNoobjects() 
{
	GetMineView()->SetViewObjectFlags(eViewObjectsNone);
}
void CMainFrame::OnViewObjectsPlayers() 
{
	GetMineView()->ToggleViewObjects(eViewObjectsPlayers);
}
void CMainFrame::OnViewObjectsPowerups() 
{
	GetMineView()->ToggleViewObjects(eViewObjectsPowerups);
}
void CMainFrame::OnViewObjectsRobots() 
{
	GetMineView()->ToggleViewObjects(eViewObjectsRobots);
}
void CMainFrame::OnViewObjectsWeapons() 
{
	GetMineView()->ToggleViewObjects(eViewObjectsWeapons);
}

void CMainFrame::OnViewObjectsControlcenter() 
{
	GetMineView()->ToggleViewObjects(eViewObjectsControlCenter);
}

//////////////////// view orientation /////////////////
void CMainFrame::OnViewPanUp() 
{
	CMineView* view = GetMineView();
	if (view->m_viewOption != eViewNearbyCubeLines)
		view->Pan('Y', -gMoveRate);
}

void CMainFrame::OnViewPanIn() 
{
	CMineView* view = GetMineView();
	if (view->m_viewOption != eViewNearbyCubeLines)
		view->Pan('Z', 4*gMoveRate);
}

void CMainFrame::OnViewPanRight() 
{
	CMineView* view = GetMineView();
	if (view->m_viewOption != eViewNearbyCubeLines)
		view->Pan('X', -gMoveRate);
}

void CMainFrame::OnViewPanOut() 
{
	CMineView* view = GetMineView();
	if (view->m_viewOption != eViewNearbyCubeLines)
		view->Pan('Z', -4*gMoveRate);
}

void CMainFrame::OnViewPanLeft() 
{
	CMineView* view = GetMineView();
	if (view->m_viewOption != eViewNearbyCubeLines)
		view->Pan('X',  gMoveRate);
}

void CMainFrame::OnViewPanDown() 
{
	CMineView* view = GetMineView();
	if (view->m_viewOption != eViewNearbyCubeLines)
		view->Pan('Y',  gMoveRate);
}

void CMainFrame::OnViewRotateCounterclockwise() 
{
	GetMineView()->Rotate('Z', gRotateRate);
}

void CMainFrame::OnViewRotateClockwise() 
{
	GetMineView()->Rotate('Z', -gRotateRate);
}

void CMainFrame::OnViewRotateHorizontallyleft() 
{
	GetMineView()->Rotate('Y',-gRotateRate);
}

void CMainFrame::OnViewRotateHorizontallyright() 
{
	GetMineView()->Rotate('Y', gRotateRate);
}

void CMainFrame::OnViewRotateVerticallydown() 
{
	GetMineView()->Rotate('X', gRotateRate);
}

void CMainFrame::OnViewRotateVerticallyup() 
{
	GetMineView()->Rotate('X', -gRotateRate);
}

/////////////// view Objects () /////////////////
void CMainFrame::OnUpdateViewObjectsHostages(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewObjectFlags & eViewObjectsHostages ? 1 : 0);
	//ToolView ()->Refresh ();
}
void CMainFrame::OnUpdateViewObjectsKeys(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewObjectFlags & eViewObjectsKeys ? 1 : 0);
	//ToolView ()->Refresh ();
}
void CMainFrame::OnUpdateViewObjectsPlayers(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewObjectFlags & eViewObjectsPlayers ? 1 : 0);
	//ToolView ()->Refresh ();
}
void CMainFrame::OnUpdateViewObjectsPowerups(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewObjectFlags & eViewObjectsPowerups ? 1 : 0);
	//ToolView ()->Refresh ();
}
void CMainFrame::OnUpdateViewObjectsRobots(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewObjectFlags & eViewObjectsRobots ? 1 : 0);
	//ToolView ()->Refresh ();
}
void CMainFrame::OnUpdateViewObjectsWeapons(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewObjectFlags & eViewObjectsWeapons ? 1 : 0);
	//ToolView ()->Refresh ();
}
void CMainFrame::OnUpdateViewObjectsNoobjects(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewObjectFlags == 0 ? 1 : 0);
	//ToolView ()->Refresh ();
}
void CMainFrame::OnUpdateViewObjectsControlcenter(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewObjectFlags & eViewObjectsControlCenter ? 1 : 0);
	//ToolView ()->Refresh ();
}
void CMainFrame::OnUpdateViewObjectsAllobjects(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewObjectFlags == eViewObjectsAll ? 1 : 0);
	//ToolView ()->Refresh ();
}

////////////////// view flags ///////////////////
void CMainFrame::OnUpdateViewLights(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewMineFlags & eViewMineLights ? 1 : 0);
	//ToolView ()->Refresh ();
}
void CMainFrame::OnUpdateViewShading(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewMineFlags & eViewMineShading ? 1 : 0);
	//ToolView ()->Refresh ();
}
void CMainFrame::OnUpdateViewDeltaLights(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewMineFlags & eViewMineDeltaLights ? 1 : 0);
	//ToolView ()->Refresh ();
}
void CMainFrame::OnUpdateViewWalls(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewMineFlags & eViewMineWalls ? 1 : 0);
	//ToolView ()->Refresh ();
}

void CMainFrame::OnUpdateViewSpecial(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewMineFlags & eViewMineSpecial ? 1 : 0);
	//ToolView ()->Refresh ();
}

void CMainFrame::OnUpdateViewUsedTextures(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetTextureView()->m_viewFlags & eViewMineUsedTextures ? 1 : 0);
	//ToolView ()->Refresh ();
}



////////// view option menu checks /////////////
void CMainFrame::OnUpdateViewAlllines(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewOption == eViewAllLines ? 1 : 0);
}
void CMainFrame::OnUpdateViewHidelines(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewOption == eViewHideLines ? 1 : 0);
}
void CMainFrame::OnUpdateViewNearbycubelines(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewOption == eViewNearbyCubeLines ? 1 : 0);
}
void CMainFrame::OnUpdateViewPartiallines(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewOption == eViewPartialLines ? 1 : 0);
}

void CMainFrame::OnUpdateViewTexturemapped(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetMineView()->m_viewOption == eViewTextureMapped ? 1 : 0);
}

                        /*--------------------------*/

void CMainFrame::OnEditGeoFwd ()
{
GetMine ()->EditGeoFwd ();
MineView ()->Refresh ();
}

void CMainFrame::OnEditGeoUp ()
{
GetMine ()->EditGeoUp ();
MineView ()->Refresh ();
}

void CMainFrame::OnEditGeoBack ()
{
GetMine ()->EditGeoBack ();
MineView ()->Refresh ();
}

void CMainFrame::OnEditGeoRotLeft ()
{
GetMine ()->EditGeoRotLeft ();
MineView ()->Refresh ();
}

void CMainFrame::OnEditGeoGrow ()
{
GetMine ()->EditGeoGrow ();
MineView ()->Refresh ();
}

void CMainFrame::OnEditGeoRotRight ()
{
GetMine ()->EditGeoRotRight ();
MineView ()->Refresh ();
}

void CMainFrame::OnEditGeoLeft ()
{
GetMine ()->EditGeoLeft ();
MineView ()->Refresh ();
}

void CMainFrame::OnEditGeoDown ()
{
GetMine ()->EditGeoDown ();
MineView ()->Refresh ();
}

void CMainFrame::OnEditGeoRight ()
{
GetMine ()->EditGeoRight ();
MineView ()->Refresh ();
}

void CMainFrame::OnEditGeoShrink ()
{
GetMine ()->EditGeoShrink ();
MineView ()->Refresh ();
}

                        /*--------------------------*/

void CMainFrame::OnSelNextCubeElem ()
{
MineView ()->NextCubeElement ();
}

void CMainFrame::OnSelPrevCubeElem ()
{
MineView ()->PrevCubeElement ();
}

void CMainFrame::OnSelNextPoint ()
{
if (MineView ()->SelectMode (POINT_MODE))
	MineView ()->NextPoint ();
else
	MineView ()->SetSelectMode (POINT_MODE);
}

void CMainFrame::OnSelPrevPoint ()
{
if (MineView ()->SelectMode (POINT_MODE))
	MineView ()->PrevPoint ();
else
	MineView ()->SetSelectMode (POINT_MODE);
}

void CMainFrame::OnSelNextLine ()
{
if (MineView ()->SelectMode (LINE_MODE))
	MineView ()->NextLine ();
else
	MineView ()->SetSelectMode (LINE_MODE);
}

void CMainFrame::OnSelPrevLine ()
{
if (MineView ()->SelectMode (LINE_MODE))
	MineView ()->PrevLine ();
else
	MineView ()->SetSelectMode (LINE_MODE);
}

void CMainFrame::OnSelNextSide ()
{
if (MineView ()->SelectMode (SIDE_MODE))
	MineView ()->NextSide ();
else
	MineView ()->SetSelectMode (SIDE_MODE);
}

void CMainFrame::OnSelPrevSide ()
{
if (MineView ()->SelectMode (SIDE_MODE))
	MineView ()->PrevSide ();
else
	MineView ()->SetSelectMode (SIDE_MODE);
}

void CMainFrame::OnSelNextCube ()
{
if (MineView ()->SelectMode (CUBE_MODE))
	MineView ()->NextCube ();
else
	MineView ()->SetSelectMode (CUBE_MODE);
}

void CMainFrame::OnSelPrevCube ()
{
if (MineView ()->SelectMode (CUBE_MODE))
	MineView ()->PrevCube ();
else
	MineView ()->SetSelectMode (CUBE_MODE);
}

void CMainFrame::OnSelNextObject ()
{
if (MineView ()->SelectMode (OBJECT_MODE))
	MineView ()->NextObject ();
else
	MineView ()->SetSelectMode (OBJECT_MODE);
}

void CMainFrame::OnSelPrevObject ()
{
if (MineView ()->SelectMode (OBJECT_MODE))
	MineView ()->PrevObject ();
else
	MineView ()->SetSelectMode (OBJECT_MODE);
}

void CMainFrame::OnSelFwdCube ()
{
MineView ()->ForwardCube ();
}

void CMainFrame::OnSelBackCube ()
{
MineView ()->BackwardsCube ();
}

void CMainFrame::OnSelOtherCube ()
{
MineView ()->SelectOtherCube ();
}

void CMainFrame::OnSelOtherSide () 
{
MineView ()->SelectOtherSide ();
}

void CMainFrame::OnUndo () 
{
if (theApp.Undo ())
	MineView ()->Refresh ();
}

void CMainFrame::OnRedo () 
{
if (theApp.Redo ())
	MineView ()->Refresh ();
}

void CMainFrame::OnCheckMine () 
{
ShowTools ();
ToolView ()->EditDiag ();
ToolView ()->DiagTool ()->OnCheckMine ();
}


void CMainFrame::OnConvert () 
{
	CConvertDlg	d;

if (file_type == RDL_FILE) {
	if (d.DoModal () == IDOK) {
		MineView ()->Refresh ();
		if (bExpertMode)
			INFOMSG (" Mine converted to a Descent 2 level")
		else
			ErrorMsg ("Mine converted to a Descent 2 level.");
		GetMine ()->ConvertWallNum (MAX_WALLS1 + 1, MAX_WALLS2 + 1);
		}
	}
else {
	if (level_version < 8) {
		level_version = 8;
		if (bExpertMode)
			INFOMSG (" Mine converted to a Vertigo level")
		else
			ErrorMsg ("Mine converted to a Vertigo level.");
		}
	else if (level_version < LEVEL_VERSION) {
		level_version = LEVEL_VERSION;
		if (bExpertMode)
			INFOMSG (" Mine converted to a D2X-XL level")
		else
			ErrorMsg ("Mine converted to a D2X-XL level.");
		GetMine ()->ConvertWallNum (MAX_WALLS2 + 1, MAX_WALLS3 + 1);
		GetMine ()->NumObjTriggers () = 0;
		memset (GetMine ()->ObjTriggerList (), 0xff, MAX_OBJ_TRIGGERS * sizeof (CDObjTriggerList));
		memset (GetMine ()->ObjTriggerRoot (), 0xff, MAX_OBJECTS2 * sizeof (short));
		}
	else {
		level_version = 7;
		if (bExpertMode)
			INFOMSG (" Mine converted to a non-Vertigo level")
		else
			ErrorMsg ("Mine converted to a non-Vertigo level.\n\nWarning - You should now remove all Vertigo and D2X-XL objects.");
		GetMine ()->ConvertWallNum (MAX_WALLS3 + 1, MAX_WALLS2 + 1);
		}
	}
}


                        /*--------------------------*/

void CMainFrame::RecalcLayout (int nToolMode, int nTextureMode)
{
	CRect	rc, rc0, rc1;

if (!(IsWindow (m_splitter1) && IsWindow (m_splitter2)))
	return;
m_splitter2.RecalcLayout (nToolMode, nTextureMode);
if (nLayout) {
	if (nTextureMode)
		m_textureMode = nTextureMode;
	if (nToolMode) {
		m_splitter1.GetClientRect (rc);
		MinePane ()->GetWindowRect (rc0);
		ToolPane ()->GetWindowRect (rc1);
		if (rc.Width () - rc1.Width () < rc0.Width ())
			m_splitter1.SetColumnInfo (0, rc.Width () - rc1.Width () - 9 - 2 * GetSystemMetrics (SM_CYBORDER), 0);
		else if ((nToolMode == 1) || (rc1.Width () > theApp.ToolSize ().cx)) {
			m_splitter1.SetColumnInfo (1, rc.Width () - theApp.ToolSize ().cx, 0);
			m_splitter1.SetColumnInfo (0, theApp.ToolSize ().cx, 0);
			}
		else if (nToolMode == 2) {
			m_splitter1.SetColumnInfo (0, 0, 0);
			m_splitter1.SetColumnInfo (1, rc.Width () - 9, 0);
			}
		m_splitter1.RecalcLayout ();
		m_toolMode = nToolMode;
		}
	}
else {
	if (nToolMode)
		m_toolMode = nToolMode;
	if (nTextureMode) {
			int	nWidth = (nTextureMode == 1) ? (m_texPaneWidth < 0) ? CX_TEXTURES : m_texPaneWidth : 0;

		if (nTextureMode == 2) {	//hide
			TexturePane ()->GetWindowRect (rc);
			m_texPaneWidth = rc.Width ();
			}
		m_splitter1.GetClientRect (rc);
		m_splitter1.SetColumnInfo (0, nWidth, 0);
		m_splitter1.SetColumnInfo (1, rc.Width () - nWidth - /*nWidth **/ 9, 0);
		m_splitter1.RecalcLayout ();
		m_textureMode = nTextureMode;
		}
	}
CFrameWnd::RecalcLayout ();
}

                        /*--------------------------*/

void CDlcSplitterWnd::RecalcLayout (int nToolMode, int nTextureMode)
{
if (theApp.MainFrame () && theApp.ToolView ()) {
	CRect	rc, rc0, rc1;
	GetClientRect (rc);
	GetPane (0,0)->GetWindowRect (rc0);
	GetPane (1,0)->GetWindowRect (rc1);
	if (nLayout) {
		if (nTextureMode) {
				int	nHeight = (nTextureMode == 1) ? (m_texPaneHeight < 0) ? CY_TEXTURES : m_texPaneHeight : 0;

			if (nTextureMode == 2)	//hide
				m_texPaneHeight = rc1.Height ();
			SetRowInfo (0, rc.Height () - nHeight - m_cySplitter - 2 * GetSystemMetrics (SM_CYBORDER), 0);
			SetRowInfo (1, nHeight, 0);
			}
		}
	else {
		if (rc.Height () - rc1.Height () < rc0.Height ())
			SetRowInfo (0, rc.Height () - rc1.Height () - m_cySplitter - 2 * GetSystemMetrics (SM_CYBORDER), 0);
		else if ((nToolMode == 1) || (rc1.Height () > theApp.ToolSize ().cy)) {
			SetRowInfo (0, rc.Height () - theApp.ToolSize ().cy, 0);
			SetRowInfo (1, theApp.ToolSize ().cy, 0);
			}
		else if (nToolMode == 2) {
			SetRowInfo (0, rc.Height (), 0);
			SetRowInfo (1, 0, 0);
			}
		}
	}
CSplitterWnd::RecalcLayout ();
}

                        /*--------------------------*/

void CMainFrame::DebugMsg (const char *pszMsg)
{
m_statusBar.SetPaneText (2, pszMsg); 
if (!(theApp.ToolView () && theApp.ToolView ()->DiagTool ()))
	return;
if (!theApp.ToolView ()->DiagTool ()->Inited ()) {
	int i = theApp.ToolView ()->m_pTools->GetActiveIndex ();
	theApp.ToolView ()->EditDiag ();
	theApp.ToolView ()->SetActive (i);
	}
if (!theApp.ToolView ()->DiagTool ()->Inited ())
	return;
theApp.ToolView ()->DiagTool ()->AddMessage (pszMsg, 100);
}

                        /*--------------------------*/

bool CMainFrame::InitProgress (int nMax)
{
CRect rc;
m_statusBar.GetItemRect (2, &rc);
if (!m_progress.Create (WS_CHILD | WS_VISIBLE, rc, &m_statusBar, 1))
	return false;
m_progress.SetRange32 (0, nMax);
m_progress.SetStep (1);
return true;
}

                        /*--------------------------*/

#if EDITBAR == 0

BEGIN_MESSAGE_MAP(CEditTool, CDialog)
	ON_WM_TIMER ()
	ON_BN_CLICKED (IDC_EDITGEO0, OnEditGeo0)
	ON_BN_CLICKED (IDC_EDITGEO1, OnEditGeo1)
	ON_BN_CLICKED (IDC_EDITGEO2, OnEditGeo2)
	ON_BN_CLICKED (IDC_EDITGEO3, OnEditGeo3)
	ON_BN_CLICKED (IDC_EDITGEO4, OnEditGeo4)
	ON_BN_CLICKED (IDC_EDITGEO5, OnEditGeo5)
	ON_BN_CLICKED (IDC_EDITGEO6, OnEditGeo6)
	ON_BN_CLICKED (IDC_EDITGEO7, OnEditGeo7)
	ON_BN_CLICKED (IDC_EDITGEO8, OnEditGeo8)
	ON_BN_CLICKED (IDC_EDITGEO9, OnEditGeo9)
END_MESSAGE_MAP()

                        /*--------------------------*/

CEditTool::CEditTool ()
{
m_nTimer = -1;
m_nEditFunc = -1;
}

                        /*--------------------------*/

BOOL CEditTool::OnInitDialog ()
{
CDialog::OnInitDialog ();
for (int i = 0; i < 10; i++)
	m_btns [i].AutoLoad (IDC_EDITGEO7 + i, this);
return TRUE;
}

                        /*--------------------------*/

void CEditTool::OnEditGeo0 () { theApp.MainFrame ()->OnEditGeoShrink (); }
void CEditTool::OnEditGeo1 () { theApp.MainFrame ()->OnEditGeoRotLeft (); }
void CEditTool::OnEditGeo2 () { theApp.MainFrame ()->OnEditGeoDown (); }
void CEditTool::OnEditGeo3 () { theApp.MainFrame ()->OnEditGeoRotRight (); }
void CEditTool::OnEditGeo4 () { theApp.MainFrame ()->OnEditGeoLeft (); }
void CEditTool::OnEditGeo5 () { theApp.MainFrame ()->OnEditGeoGrow (); }
void CEditTool::OnEditGeo6 () { theApp.MainFrame ()->OnEditGeoRight (); }
void CEditTool::OnEditGeo7 () { theApp.MainFrame ()->OnEditGeoBack (); }
void CEditTool::OnEditGeo8 () { theApp.MainFrame ()->OnEditGeoUp (); }
void CEditTool::OnEditGeo9 () { theApp.MainFrame ()->OnEditGeoFwd (); }

                        /*--------------------------*/

void CEditTool::OnTimer (UINT nIdEvent)
{
switch (m_nEditFunc) {
	case IDC_EDITGEO0:
		OnEditGeo0 ();
		break;
	case IDC_EDITGEO1:
		OnEditGeo1 ();
		break;
	case IDC_EDITGEO2:
		OnEditGeo2 ();
		break;
	case IDC_EDITGEO3:
		OnEditGeo3 ();
		break;
	case IDC_EDITGEO4:
		OnEditGeo4 ();
		break;
	case IDC_EDITGEO5:
		OnEditGeo5 ();
		break;
	case IDC_EDITGEO6:
		OnEditGeo6 ();
		break;
	case IDC_EDITGEO7:
		OnEditGeo7 ();
		break;
	case IDC_EDITGEO8:
		OnEditGeo8 ();
		break;
	case IDC_EDITGEO9:
		OnEditGeo9 ();
		break;
	default:
		break;
	}
UINT i = (m_nTimerDelay * 9) / 10;
if (i >= 25) {
	KillTimer (m_nTimer);
	m_nTimer = SetTimer (1, m_nTimerDelay = i, NULL);
	}
}

                        /*--------------------------*/

BOOL CEditTool::OnNotify (WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	LPNMHDR	nmHdr = (LPNMHDR) lParam;
	int		nMsg = nmHdr->code;

if ((wParam < IDC_EDITGEO7) || (wParam > IDC_EDITGEO0))
	return CWnd::OnNotify (wParam, lParam, pResult);
if (((LPNMHDR) lParam)->code == WM_LBUTTONDOWN) {
	m_nEditFunc = wParam;
	m_nTimer = SetTimer (1, 250U, NULL);
	m_nTimerDelay = 250;
	}
else {
	m_nEditFunc = -1;
	if (m_nTimer >= 0) {
		KillTimer (m_nTimer);
		m_nTimer = -1;
		}
	}
*pResult = 0;
return TRUE;
}
		
#endif

                        /*--------------------------*/

//eof mainfrm.cpp