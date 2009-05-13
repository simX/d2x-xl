// dlc.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "dlc.h"

#include "MainFrm.h"
#include "dlcDoc.h"
#include "mineview.h"
#include <initguid.h>
#include "Dlc_i.c"
#include "ComMine.h"
#include "ComCube.h"
#include "ComObj.h"
#include "global.h"
#include "io.h"

int nLayout = 1;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	int m_nTimer;
	int m_nTimeout;

	CAboutDlg(int m_nTimeout = 0);

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   virtual BOOL OnInitDialog ();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	afx_msg void OnLButtonDown (UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown (UINT nFlags, CPoint point);
	afx_msg void OnTimer (UINT nIdEvent);
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg(int nTimeout) : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_nTimer = -1;
	m_nTimeout = nTimeout;
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
		ON_WM_LBUTTONDOWN ()
		ON_WM_RBUTTONDOWN ()
		ON_WM_TIMER ()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CDlcApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

BOOL CAboutDlg::OnInitDialog ()
{
CDialog::OnInitDialog ();
if (m_nTimeout)
	m_nTimer = SetTimer (4, (UINT) 100, NULL);
return TRUE;
}

void CAboutDlg::OnLButtonDown (UINT nFlags, CPoint point)
{
EndDialog (1);
}

void CAboutDlg::OnRButtonDown (UINT nFlags, CPoint point)
{
EndDialog (1);
}

void CAboutDlg::OnTimer (UINT nIdEvent)
{
if (m_nTimer == (int) nIdEvent) {
	m_nTimeout--;
	if (m_nTimeout <= 0) {
		KillTimer (m_nTimer);
		m_nTimer = -1;
		EndDialog (1);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDlcApp

BEGIN_MESSAGE_MAP(CDlcApp, CWinApp)
	//{{AFX_MSG_MAP(CDlcApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlcApp construction

CDlcApp::CDlcApp()
{
m_pDlcDoc = NULL;
m_bSplashScreen = 1;
m_bMaximized = false;
ResetUndoBuffer ();
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDlcApp object

CDlcApp theApp;


// {3F315842-67AC-11d2-AE2A-00C0F03014A5}
static const CLSID clsid  = 
{ 0x3f315842, 0x67ac, 0x11d2, { 0xae, 0x2a, 0x0, 0xc0, 0xf0, 0x30, 0x14, 0xa5 } };

/////////////////////////////////////////////////////////////////////////////
// CDlcApp initialization

BOOL CDlcApp::InitInstance()
{
	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	if (!InitATL())
		return FALSE;

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings(10);  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	m_pDlcDoc = new CDlcDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CDlcDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CMineView));
	AddDocTemplate(m_pDlcDoc);

	// Connect the COleTemplateServer to the document template.
	//  The COleTemplateServer creates new documents on behalf
	//  of requesting OLE containers by using information
	//  specified in the document template.
	m_server.ConnectTemplate(clsid, m_pDlcDoc, TRUE);
		// Note: SDI applications register server Objects () only if /Embedding
		//   or /Automation is present on the command line.

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

    _Module.UpdateRegistryFromResource(IDR_DLC, TRUE);
	_Module.RegisterServer(TRUE);

	// Check to see if launched as OLE server
	if (cmdInfo.m_bRunEmbedded || cmdInfo.m_bRunAutomated)
	{
		// Register all OLE server (factories) as running.  This enables the
		//  OLE libraries to create Objects () from other applications.
		COleTemplateServer::RegisterAll();
		_Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE);


		// Application was run with /Embedding or /Automation.  Don't show the
		//  main window in this case.
		return TRUE;
	}

	// When a server application is launched stand-alone, it is a good idea
	//  to update the system registry in case it has been damaged.
	m_server.UpdateRegistry(OAT_DISPATCH_OBJECT);
	COleObjectFactory::UpdateRegistryAll();

	// Dispatch commands specified on the command line
	cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
	if (!ProcessShellCommand (cmdInfo))
		return FALSE;

	memset (pTextures, 0, sizeof (pTextures));
	TextureView ()->Setup ();
//	ToolView ()->Setup ();
	// The one and only window has been initialized, so show and update it.
	MainFrame ()->SetSelectMode (eSelectSide);
	MainFrame ()->ShowWindow (SW_SHOW);
	MainFrame ()->GetWindowText (m_szCaption, sizeof (m_szCaption));
	MainFrame ()->FixToolBars ();
	LoadLayout ();
	MainFrame ()->ToolView ()->RecalcLayout (MainFrame ()->m_toolMode, MainFrame ()->m_textureMode);
	MainFrame ()->ToolView ()->PrefsDlg ()->SetAppSettings ();
	MainFrame ()->UpdateWindow ();
	if (m_bSplashScreen || DEMO) {
		CAboutDlg aboutDlg (100);
		aboutDlg.DoModal ();
		}
	if (*cmdInfo.m_strFileName)
		GetDocument ()->OpenFile (false, cmdInfo.m_strFileName.GetBuffer (256), NULL /*"*"*/);
	ToolView ()->PrefsDlg ()->SetAppSettings (true);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlcApp commands



/////////////////////////////////////////////////////////////////////////////
// CDlcApp message handlers
	
CDLCModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_ComMine, CComMine)
	OBJECT_ENTRY(CLSID_ComCube, CComCube)
	OBJECT_ENTRY(CLSID_ComObj, CComObj)
END_OBJECT_MAP()

                        /*--------------------------*/

LONG CDLCModule::Unlock()
{
	AfxOleUnlockApp();
	return 0;
}

                        /*--------------------------*/

LONG CDLCModule::Lock()
{
	AfxOleLockApp();
	return 1;
}

                        /*--------------------------*/

int CDlcApp::ExitInstance()
{
if (m_bATLInited) {
	_Module.RevokeClassObjects();
	_Module.Term();
	}
return CWinApp::ExitInstance();
}

                        /*--------------------------*/

CDocument* CDlcApp::OpenDocumentFile (LPCTSTR lpszFileName)
{
	int			nAction = IDOK;

	ASSERT(m_pDocManager != NULL);
//	GetDocument ()->SetPathName ("(new document)");
	return CWinApp::OpenDocumentFile (lpszFileName);
}

                        /*--------------------------*/

BOOL CDlcApp::InitATL()
{
	m_bATLInited = TRUE;

	_Module.Init(ObjectMap, AfxGetInstanceHandle());
	_Module.dwThreadID = GetCurrentThreadId();

	return TRUE;

}

                        /*--------------------------*/

bool CDlcApp::SetModified (BOOL bModified) 
{
GetDocument ()->SetModifiedFlag (bModified);
if (bModified) {
	m_nModified++;
	return UpdateUndoBuffer ();
	}
else
	m_nModified = 0;
return false;
}  

                        /*--------------------------*/

void CDlcApp::ResetModified (bool bRevertUndo) 
{
if (m_nModified) {
	if (!--m_nModified)
		SetModified (FALSE);
	UnlockUndo ();
	if (bRevertUndo)
		RevertUndoBuffer ();
	}
}

                        /*--------------------------*/

void CDlcApp::ResetUndoBuffer ()
{
m_undoList.Reset ();
}

                        /*--------------------------*/

void CDlcApp::DelayUndo (bool bDelay)
{
m_undoList.Delay (bDelay);
}

                        /*--------------------------*/

int CDlcApp::UndoCount ()
{
return m_undoList.UndoCount ();
}

                        /*--------------------------*/

bool CDlcApp::UpdateUndoBuffer (bool bForce)
{
return m_undoList.Update (bForce);
}

                        /*--------------------------*/

bool CDlcApp::RevertUndoBuffer ()
{
return m_undoList.Revert ();
}

                        /*--------------------------*/

int CDlcApp::EnableUndo (int bEnable)
{
return m_undoList.Enable (bEnable);
}

                        /*--------------------------*/

bool CDlcApp::Undo ()
{
bool bUndo = m_undoList.Undo ();
if (bUndo)
	MineView ()->Refresh ();
return bUndo;
}

                        /*--------------------------*/

bool CDlcApp::Redo ()
{
bool bRedo = m_undoList.Redo ();
if (bRedo)
	MineView ()->Refresh ();
return bRedo;
}

                        /*--------------------------*/

void CDlcApp::WritePrivateProfileInt (LPSTR szKey, int nValue)
{
	char	szValue [20];

sprintf (szValue, "%d", nValue);
WritePrivateProfileString ("DLE-XP", szKey, szValue, "dle-xp.ini");
}

                        /*--------------------------*/

void CDlcApp::SaveLayout ()
{
	CRect	rc;

MainFrame ()->GetWindowRect (rc);
if (rc.left < 0)
	rc.left = 0;
if (rc.top < 0)
	rc.top = 0;
WritePrivateProfileInt ("xWin", rc.left);
WritePrivateProfileInt ("yWin", rc.top);
WritePrivateProfileInt ("cxWin", rc.Width ());
WritePrivateProfileInt ("cyWin", rc.Height ());
//MainFrame ()->ScreenToClient (rc);
MainFrame ()->m_toolBar.GetWindowRect (rc);
WritePrivateProfileInt ("xMainTB", rc.left);
WritePrivateProfileInt ("yMainTB", rc.top);
WritePrivateProfileInt ("cxMainTB", rc.Width ());
WritePrivateProfileInt ("cyMainTB", rc.Height ());
#if EDITBAR
MainFrame ()->m_editBar.GetWindowRect (rc);
WritePrivateProfileInt ("xEditTB", rc.left);
WritePrivateProfileInt ("yEditTB", rc.top);
WritePrivateProfileInt ("cxEditTB", rc.Width ());
WritePrivateProfileInt ("cyEditTB", rc.Height ());
#endif
WritePrivateProfileInt ("ToolMode", MainFrame ()->m_toolMode);
WritePrivateProfileInt ("TextureMode", MainFrame ()->m_textureMode);
WritePrivateProfileInt ("AutoFixBugs", ToolView ()->DiagTool ()->m_bAutoFixBugs);
WritePrivateProfileInt ("SplashScreen", m_bSplashScreen);
}

                        /*--------------------------*/

void CDlcApp::LoadLayout ()
{
	CRect	rc, tbrc;
	UINT h = AFX_IDW_DOCKBAR_TOP;

MainFrame ()->m_toolMode = GetPrivateProfileInt ("DLE-XP", "ToolMode", 1, "dle-xp.ini");
MainFrame ()->m_textureMode = GetPrivateProfileInt ("DLE-XP", "TextureMode", 1, "dle-xp.ini");
MainFrame ()->m_paneMode = ((MainFrame ()->m_toolMode == 2) && (MainFrame ()->m_textureMode == 2)) ? 2 : 0;
ToolView ()->DiagTool ()->m_bAutoFixBugs = GetPrivateProfileInt ("DLE-XP", "AutoFixBugs", 1, "dle-xp.ini");
rc.left = GetPrivateProfileInt ("DLE-XP", "xWin", 0, "dle-xp.ini");
rc.top = GetPrivateProfileInt ("DLE-XP", "yWin", 0, "dle-xp.ini");
rc.right = rc.left + GetPrivateProfileInt ("DLE-XP", "cxWin", 0, "dle-xp.ini");
rc.bottom = rc.top + GetPrivateProfileInt ("DLE-XP", "cyWin", 0, "dle-xp.ini");
if ((rc.left >= rc.right) || (rc.top >= rc.bottom) || 
	 (rc.bottom < 0) || (rc.right < 0) ||
	 (rc.left >= GetSystemMetrics (SM_CXSCREEN)) || (rc.top >= GetSystemMetrics (SM_CYSCREEN))) {
	rc.left = rc.top = 0;
	rc.right = GetSystemMetrics (SM_CXSCREEN);
	rc.bottom = GetSystemMetrics (SM_CYSCREEN);
	}
if (!rc.left && !rc.top && 
	 (rc.right >= GetSystemMetrics (SM_CXSCREEN)) && 
	 (rc.bottom >= GetSystemMetrics (SM_CYSCREEN)))
 	MainFrame ()->ShowWindow (SW_SHOWMAXIMIZED);
else
	MainFrame ()->MoveWindow (&rc, TRUE);

#if 0
tbrc.left = GetPrivateProfileInt ("DLE-XP", "xMainTB", 0, "dle-xp.ini");
if (tbrc.left < 0)
	tbrc.left = 0;
tbrc.top = GetPrivateProfileInt ("DLE-XP", "yMainTB", 0, "dle-xp.ini");
tbrc.right = tbrc.left + GetPrivateProfileInt ("DLE-XP", "cxMainTB", 0, "dle-xp.ini");
tbrc.bottom = tbrc.top + GetPrivateProfileInt ("DLE-XP", "cyMainTB", 0, "dle-xp.ini");
	if (tbrc.Width () > tbrc.Height ())	//horizontal
	if (tbrc.bottom >= rc.bottom - GetSystemMetrics (SM_CYFRAME))
		h = AFX_IDW_DOCKBAR_BOTTOM;
	else
		h = AFX_IDW_DOCKBAR_TOP;
else //vertical
	if (tbrc.right >= rc.right - GetSystemMetrics (SM_CXFRAME))
		h = AFX_IDW_DOCKBAR_RIGHT;
	else
		h = AFX_IDW_DOCKBAR_LEFT;
if (tbrc.Width () && tbrc.Height ())
	MainFrame ()->DockControlBar (&MainFrame ()->m_toolBar, (UINT) h, &tbrc);
#endif

tbrc.left = GetPrivateProfileInt ("DLE-XP", "xEditTB", 0, "dle-xp.ini");
if (tbrc.left < 0)
	tbrc.left = 0;
tbrc.top = GetPrivateProfileInt ("DLE-XP", "yEditTB", 0, "dle-xp.ini");
#if EDITBAR
tbrc.right = tbrc.left + GetPrivateProfileInt ("DLE-XP", "cxEditTB", 0, "dle-xp.ini");
tbrc.bottom = tbrc.top + GetPrivateProfileInt ("DLE-XP", "cyEditTB", 0, "dle-xp.ini");
if (tbrc.Width () > tbrc.Height ())	//horizontal
	if (tbrc.bottom >= rc.bottom - GetSystemMetrics (SM_CYFRAME))
		h = AFX_IDW_DOCKBAR_BOTTOM;
	else if (!tbrc.top)
		h = AFX_IDW_DOCKBAR_TOP;
else //vertical
	if (tbrc.right >= rc.right - GetSystemMetrics (SM_CXFRAME))
		h = AFX_IDW_DOCKBAR_RIGHT;
	else if (!tbrc.left)
		h = AFX_IDW_DOCKBAR_LEFT;
if (tbrc.Width () && tbrc.Height ()) {
	CPoint p;
	p.x = tbrc.left;
	p.y = tbrc.top;
	MainFrame ()->FloatControlBar (&MainFrame ()->m_editBar, p, (UINT) h);
	}
#endif
m_bSplashScreen = GetPrivateProfileInt ("DLE-XP", "SplashScreen", 1, "dle-xp.ini");
}

                        /*--------------------------*/

CUndoList::CUndoList (int maxSize)
{
m_head = m_tail = m_current = NULL;
m_size = 0;
m_enabled = 1;
m_maxSize = maxSize;
}

                        /*--------------------------*/

CUndoList::~CUndoList ()
{
Reset ();
}

                        /*--------------------------*/

void CUndoList::Reset ()
{
m_current = m_head;
Truncate ();
}

                        /*--------------------------*/

int CUndoList::Enable (int bEnable)
{
	int b = m_enabled;

m_enabled = bEnable;
return b;
}

                        /*--------------------------*/

void CUndoList::Truncate ()
{
	tUndoBuffer	*p;

if (!m_current)
	return;
m_tail = m_current->prevBuf;
while (p = m_current) {
	m_current = m_current->nextBuf;
	delete p;
	m_size--;
	}
if (m_current = m_tail)
	m_tail->nextBuf = NULL;
else
	m_head = NULL;
}

                        /*--------------------------*/

bool CUndoList::Update (bool bForce)
{
	CMine *mine = theApp.GetMine ();
	tUndoBuffer	*p;

if (!m_enabled || m_delay)
	return false;
if (!bForce && m_head &&
	 !memcmp (&m_current->undoBuffer, &mine->MineData (), sizeof (struct tMineData)))
	return true;
if (m_current != m_tail) {
	if (m_current)
		m_current = m_current->nextBuf;
	else
		m_current = m_head;
	Truncate ();
	}
if ((m_size < m_maxSize) && (p = new tUndoBuffer)) {
	m_size++;
	p->prevBuf = m_tail;
	if (m_tail)
		m_tail->nextBuf = p;
	else
		m_head = p;
	m_tail = p;
	}
else if (m_head) {
	if (m_head != m_tail) {
		m_head->prevBuf = m_tail;
		m_tail->nextBuf = m_head;
		m_tail = m_head;
		m_head = m_head->nextBuf;
		m_head->prevBuf = NULL;
		}
	}
else
	return false;
m_tail->nextBuf = NULL;
memcpy (&m_tail->undoBuffer, &mine->MineData (), sizeof (struct tMineData));
m_current = m_tail;
return true;
}

                        /*--------------------------*/

int CUndoList::SetMaxSize (int maxSize)
{
if (maxSize < 1)
	maxSize = 0;
else if (maxSize > MAX_UNDOS)
	maxSize = MAX_UNDOS;
Enable (maxSize > 0);
while (m_size > maxSize)
	Truncate ();
return m_maxSize = maxSize;
}

                        /*--------------------------*/

bool CUndoList::Undo ()
{
if (!m_enabled)
	return false;
if (m_current == m_tail)
	Update ();
if (!m_current)
	return false;
if (m_current != m_head)
	m_current = m_current->prevBuf;
memcpy (&theApp.GetMine ()->MineData (), &m_current->undoBuffer, sizeof (struct tMineData));
return true;
}

                        /*--------------------------*/

bool CUndoList::Redo ()
{
if (!m_enabled)
	return false;
if (m_current == m_tail)
	return false;
if (m_current)
	m_current = m_current->nextBuf;
else
	m_current = m_head;
memcpy (&theApp.GetMine ()->MineData (), &m_current->undoBuffer, sizeof (struct tMineData));
if (m_current == m_tail)
	Truncate ();
return true;
}

                        /*--------------------------*/

bool CUndoList::Revert ()
{
if (!m_enabled || m_delay || !m_head)
	return false;
m_current = m_tail;
Truncate ();
return true;
}

                        /*--------------------------*/

void CUndoList::Delay (bool bDelay)
{
if (bDelay)
	m_delay++;
else if (m_delay)
	m_delay--;
}

                        /*--------------------------*/

int CUndoList::UndoCount ()
{
	tUndoBuffer	*p;
	int			i;

if (!m_enabled)
	return 0;
if (!(p = m_head))
	return 0;
for (i = 1; p != m_current; p = p->nextBuf)
	i++;
return i;
}

                        /*--------------------------*/

//eof dlc.cpp