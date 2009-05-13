// dlcDoc.cpp : implementation of the CDlcDoc class
//

#include <process.h>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "stdafx.h"
#include "dlc.h"

#include "dlcDoc.h"
#include "global.h"
#include "parser.h"
#include "io.h"
#include "file.h"
#include "light.h"
#include "textures.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class CSaveFileDlg : public CDialog {
	public:
		char	m_name [256];
		LPSTR	m_lpszName;
		int	m_type;
		int	*m_pType;

		CSaveFileDlg (CWnd *pParentWnd)
			: CDialog (IDD_SAVELEVEL, pParentWnd) {};
		afx_msg void OnNo ()
			{ EndDialog (IDNO); }
	
	DECLARE_MESSAGE_MAP ()
	};

BEGIN_MESSAGE_MAP (CSaveFileDlg, CDialog)
	ON_BN_CLICKED (IDNO, OnNo)
END_MESSAGE_MAP ()

/////////////////////////////////////////////////////////////////////////////
// CDlcDoc

IMPLEMENT_DYNCREATE(CDlcDoc, CDocument)

BEGIN_MESSAGE_MAP(CDlcDoc, CDocument)
	//{{AFX_MSG_MAP(CDlcDoc)
	ON_COMMAND(ID_OPENFILE, OnOpenFile)
	ON_COMMAND(ID_SAVEFILE, OnSaveFile)
	ON_COMMAND(ID_SAVEFILE_AS, OnSaveFileAs)
	ON_COMMAND(ID_RUN_LEVEL, OnRunLevel)
	ON_COMMAND(ID_INSERT_CUBE, OnInsertCube)
	ON_COMMAND(ID_INSERT_CUBE_REACTOR, OnInsertCubeReactor)
	ON_COMMAND(ID_INSERT_CUBE_ROBOTMAKER, OnInsertCubeRobotMaker)
	ON_COMMAND(ID_INSERT_CUBE_FUELCENTER, OnInsertCubeFuelCenter)
	ON_COMMAND(ID_INSERT_CUBE_REPAIRCENTER, OnInsertCubeRepairCenter)
	ON_COMMAND(ID_INSERT_CUBE_WATER, OnInsertCubeWater)
	ON_COMMAND(ID_INSERT_CUBE_LAVA, OnInsertCubeLava)
	ON_COMMAND(ID_INSERT_OBJECT_PLAYERCOPY, OnInsertObjectPlayerCopy)
	ON_COMMAND(ID_INSERT_OBJECT_PLAYER, OnInsertObjectPlayer)
	ON_COMMAND(ID_INSERT_OBJECT_ROBOT, OnInsertObjectRobot)
	ON_COMMAND(ID_INSERT_OBJECT_WEAPON, OnInsertObjectWeapon)
	ON_COMMAND(ID_INSERT_OBJECT_POWERUP, OnInsertObjectPowerup)
	ON_COMMAND(ID_INSERT_OBJECT_GUIDEBOT, OnInsertObjectGuideBot)
	ON_COMMAND(ID_INSERT_OBJECT_COOPPLAYER, OnInsertObjectCoopPlayer)
	ON_COMMAND(ID_INSERT_OBJECT_REACTOR, OnInsertObjectReactor)
	ON_COMMAND(ID_INSERT_DOOR_NORMAL, OnInsertDoorNormal)
	ON_COMMAND(ID_INSERT_DOOR_PRISON, OnInsertDoorPrison)
	ON_COMMAND(ID_INSERT_DOOR_SECRETEXIT, OnInsertDoorSecretExit)
	ON_COMMAND(ID_INSERT_DOOR_EXIT, OnInsertDoorExit)
	ON_COMMAND(ID_INSERT_DOOR_GUIDEBOTDOOR, OnInsertDoorGuideBot)
	ON_COMMAND(ID_INSERT_TRIGGER_OPENDOOR, OnInsertTriggerOpenDoor)
	ON_COMMAND(ID_INSERT_TRIGGER_ROBOTMAKER, OnInsertTriggerRobotMaker)
	ON_COMMAND(ID_INSERT_TRIGGER_SHIELDDRAIN, OnInsertTriggerShieldDrain)
	ON_COMMAND(ID_INSERT_TRIGGER_ENERGYDRAIN, OnInsertTriggerEnergyDrain)
	ON_COMMAND(ID_INSERT_TRIGGER_CONTROLPANEL, OnInsertTriggerControlPanel)
	ON_COMMAND(ID_INSERT_WALL_FUELCELLS, OnInsertWallFuelCells)
	ON_COMMAND(ID_INSERT_WALL_ILLUSION, OnInsertWallIllusion)
	ON_COMMAND(ID_INSERT_WALL_FORCEFIELD, OnInsertWallForceField)
	ON_COMMAND(ID_INSERT_WALL_FAN, OnInsertWallFan)
	ON_COMMAND(ID_INSERT_WALL_GRATE, OnInsertWallGrate)
	ON_COMMAND(ID_INSERT_WALL_WATERFALL, OnInsertWallWaterfall)
	ON_COMMAND(ID_INSERT_WALL_LAVAFALL, OnInsertWallLavafall)
	ON_COMMAND(ID_DELETE_CUBE, OnDeleteCube)
	ON_COMMAND(ID_DELETE_OBJECT, OnDeleteObject)
	ON_COMMAND(ID_DELETE_WALL, OnDeleteWall)
	ON_COMMAND(ID_DELETE_TRIGGER, OnDeleteTrigger)
	ON_COMMAND(ID_FILE_TEST, OnFileTest)
	ON_COMMAND(ID_EDIT_CUT, OnCutBlock)
	ON_COMMAND(ID_EDIT_COPY, OnCopyBlock)
	ON_COMMAND(ID_EDIT_QUICKCOPY, OnQuickCopyBlock)
	ON_COMMAND(ID_EDIT_PASTE, OnPasteBlock)
	ON_COMMAND(ID_EDIT_QUICKPASTE, OnQuickPasteBlock)
	ON_COMMAND(ID_EDIT_DELETEBLOCK, OnDeleteBlock)
	ON_COMMAND(ID_EDIT_COPYOTHERCUBESTEXTURES, OnCopyOtherCube)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CDlcDoc, CDocument)
	//{{AFX_DISPATCH_MAP(CAutoDoc)
	DISP_PROPERTY(CDlcDoc, "MemberLong1", Member1, VT_I4)
	DISP_FUNCTION(CDlcDoc, "TestLong", Test, VT_I4, VTS_NONE)
	DISP_FUNCTION(CDlcDoc, "SetMyText", SetMyText, VT_EMPTY, VTS_BSTR)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_IAuto to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {3F315844-67AC-11d2-AE2A-00C0F03014A5}
static const IID IID_Idlc =
{ 0x3f315844, 0x67ac, 0x11d2, { 0xae, 0x2a, 0x0, 0xc0, 0xf0, 0x30, 0x14, 0xa5 } };

BEGIN_INTERFACE_MAP(CDlcDoc, CDocument)
	INTERFACE_PART(CDlcDoc, IID_Idlc, Dispatch)
END_INTERFACE_MAP()

                        /*--------------------------*/

class CNewFileDlg : public CDialog {
	public:
		char	m_name [256];
		LPSTR	m_lpszName;
		int	m_type;
		int	*m_pType;

		CNewFileDlg (CWnd *pParentWnd, LPSTR lpszName, int *pType)
			: CDialog (IDD_NEWLEVEL, pParentWnd) {
			strcpy (m_name, m_lpszName = lpszName);
			m_type = *(m_pType = pType);
			}
      virtual BOOL OnInitDialog () {
			CDialog::OnInitDialog ();
			if (!theApp.GetMine ()->m_bVertigo)
				GetDlgItem (IDC_D2VLEVEL)->EnableWindow (FALSE);
			return TRUE;
			}
		virtual void DoDataExchange (CDataExchange * pDX) { 
			DDX_Text (pDX, IDC_LEVELNAME, m_name, sizeof (m_name)); 
			DDX_Radio (pDX, IDC_D1LEVEL, m_type);
			}
		void OnOK (void) {
			UpdateData (TRUE);
			strcpy (m_lpszName, m_name); 
			*m_pType = m_type;
			EndDialog (IDOK);
			}
	};

/////////////////////////////////////////////////////////////////////////////
// CDlcDoc construction/destruction

CDlcDoc::CDlcDoc()
{
m_mine = new CMine;
m_bInitDocument = true;
*m_szFile = '\0';
*m_szSubFile = '\0';
memset (&missionData, 0, sizeof (missionData));
}

CDlcDoc::~CDlcDoc()
{
	delete m_mine;
}

/////////////////////////////////////////////////////////////////////////////
// CDlcDoc serialization

void CDlcDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDlcDoc diagnostics

#ifdef _DEBUG
void CDlcDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDlcDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDlcDoc commands

BOOL CDlcDoc::OnNewDocument()
{
if (!CDocument::OnNewDocument())
	return FALSE;
if (!m_mine) 
	return FALSE;
*m_szFile = '\0';
*m_szSubFile = '\0';
if (m_bInitDocument) {
	m_bInitDocument = false;
	m_mine->Load ();
	}
else
	CreateNewLevel ();
return TRUE;
}

                        /*--------------------------*/

void CDlcDoc::CreateNewLevel ()
{
char	new_level_name [256];
int	new_level_type = 1;
strcpy(new_level_name,"(untitled)");

CNewFileDlg	d (theApp.MainFrame (), new_level_name, &new_level_type);
if (d.DoModal () == IDOK) {
	m_mine->Default ();
	theApp.MineView ()->Reset ();
	theApp.TextureView ()->Reset ();
	theApp.ToolView ()->Reset ();
//		InitRobotData();

	*m_szFile = '\0';
	file_type = new_level_type;
	switch (new_level_type) {
		case 0:
			file_type = 0;
			break;
		case 1:
		case 2:
		case 3:
			file_type = 1;
			break;
		}
	m_mine->Load ();
	switch (new_level_type) {
		case 0:
			level_version = 1;
			break;
		case 1:
			level_version = 7;
			break;
		case 2:
			level_version = 8;
			break;
		case 3:
			level_version = LEVEL_VERSION;
			m_mine->ConvertWallNum (MAX_WALLS2, MAX_WALLS3
				);
		}
#if 0
	if (strcmpi (strlwr (new_level_name), "(untitled)")) {
		strcpy (m_szSubFile, new_level_name);
#if 0
		int l = strlen (new_level_name);
		if ((l < 4) || strcmpi (new_level_name + l - 4, file_type ? ".rl2" : ".rdl"))
			strcat (new_level_name, (file_type ? ".rl2" : ".rdl"));
#endif
	}
	else
#endif
		*m_szSubFile = '\0';
	strcpy (m_mine->LevelName (), new_level_name);
	m_mine->Reset ();
	m_mine->SetLinesToDraw ();
	theApp.MineView ()->ResetView (true);
	memset (&missionData, 0, sizeof (missionData));
	CreateLightMap ();
	theApp.TextureView ()->Setup ();
	theApp.ToolView ()->TextureTool ()->LoadTextureListBoxes ();
	theApp.ToolView ()->MissionTool ()->Refresh ();
	}
}

                        /*--------------------------*/

bool CDlcDoc::BrowseForFile (LPSTR pszFile, BOOL bOpen)
{
return ::BrowseForFile (bOpen, "hog;*.rl2;*.rdl", pszFile, 
								"Descent level (*.hog;*.rl2;*.rdl)|*.hog;*.rl2;*.rdl"
								"|Descent hog file (*.hog)|*.hog|"
								"|Descent 2 level (*.rl2)|*.rl2|"
								"Descent level (*.rdl)|*.rdl||", 
								0, theApp.MainFrame ());
}

                        /*--------------------------*/

void CDlcDoc::UpdateCaption ()
{
SetPathName (m_szFile);
}

                        /*--------------------------*/

bool CDlcDoc::SaveIfModified (void)
{
	int	nAction;

if (!IsModified ())
	return true;

CSaveFileDlg d (theApp.MainFrame ());

nAction = d.DoModal (); //AfxMessageBox ("\nThe mine has been modified.\n\nClick 'Yes' to load another mine and loose all changes,\n'No' to save changes before loading another mine,\nor 'Cancel' to keep this mine and return without saving.", MB_YESNOCANCEL | MB_ICONEXCLAMATION);
if (nAction == IDCANCEL)
	return false;
theApp.SetModified (FALSE);
// buggy for new mine; fix required
if (nAction == IDNO) 
	SaveFile (*GetPathName () == '\0');
return true;
}

                        /*--------------------------*/

BOOL CDlcDoc::OpenFile (bool bBrowseForFile, LPSTR pszFile, LPSTR pszSubFile) 
{
	int err = 0;
	char szFile [256], szSubFile [256];

if (!m_mine) 
	return FALSE;
if (enable_delta_shading)
	theApp.ToolView ()->LightTool ()->OnShowDelta ();
if (!SaveIfModified ())
	return FALSE;
if (bBrowseForFile && !BrowseForFile (m_szFile, TRUE))
	return FALSE;
if (theApp.ToolView ())
	theApp.ToolView ()->DiagTool ()->Reset ();
if (!pszFile)
	pszFile = m_szFile;
if (!pszSubFile)
	pszSubFile = m_szSubFile;
strlwr (pszFile);
strcpy (szFile, pszFile);
strcpy (szSubFile, pszSubFile);
CreateLightMap ();
if (strstr (pszFile, ".hog")) {
	CHogManager	hm (theApp.MainFrame (), szFile, szSubFile);
	if (pszSubFile != m_szSubFile) {
		if (!hm.LoadLevel (szFile, szSubFile))
			return FALSE;
		}
	else {
		if (hm.DoModal () != IDOK)
			return FALSE;
		}
	if (pszFile != m_szFile)
		strcpy (m_szFile, szFile);
	strcpy (m_szSubFile, szSubFile);
	FSplit (pszFile, starting_directory, NULL, NULL);
	sprintf (m_szTmpFile, "%sdle_temp.rdl", starting_directory);
	err = m_mine->Load (m_szTmpFile, true);
	memset (&missionData, 0, sizeof (missionData));
	ReadMissionFile (m_szFile);
	}
else {
		char szExt [256];

	err = m_mine->Load (pszFile);
	FSplit (pszFile, NULL, pszSubFile, szExt);
	strcat (pszSubFile, szExt);
	}
m_mine->Reset ();
theApp.TextureView ()->Setup ();
theApp.MineView ()->DelayRefresh (true);
theApp.MineView ()->Reset ();
theApp.MineView ()->FitToView ();
//theApp.TextureView ()->Refresh ();
//theApp.ToolView ()->MissionTool ()->Refresh ();
theApp.MineView ()->DelayRefresh (false);
theApp.MineView ()->ResetView (true);
theApp.MainFrame ()->UpdateSelectButtons ((enum eSelectModes) theApp.MineView ()->GetSelectMode ());
//UpdateAllViews (NULL);
if (!err) {
	UpdateCaption ();
	AfxGetApp ()->AddToRecentFileList (m_szFile);
	}
CountCustomTextures ();
theApp.ToolView ()->TextureTool ()->LoadTextureListBoxes ();
return (err == 0);
}

                        /*--------------------------*/

//BOOL CDlcDoc::OnSaveDocument (LPCTSTR lpszPathName) 
bool CDlcDoc::SaveFile (bool bSaveAs) 
{
#if DEMO
ErrorMsg ("You cannot save a mine in the demo.");
return false;
#else
	int err = 0;

if (!m_mine)
	return false;
CountCustomTextures ();
if (enable_delta_shading)
	theApp.ToolView ()->LightTool ()->OnShowDelta ();
if (!*m_szFile) {
	char	szMissions [256];
	FSplit ((file_type == RDL_FILE) ? descent_path : levels_path, szMissions, NULL, NULL);
//	strcpy (m_szFile, (file_type == RDL_FILE) ? "new.rdl" : "new.rl2");
	sprintf (m_szFile, "%s%s.hog", szMissions, *m_szSubFile ? m_szSubFile : "new");
	}
if (bSaveAs && !BrowseForFile (m_szFile, FALSE))
	return false;
if (strstr (m_szFile, ".hog"))
	err = SaveToHog (m_szFile, m_szSubFile, bSaveAs);
else
	err = m_mine->Save (m_szFile);
SetModifiedFlag (err != 0);
if (!err) {
	UpdateCaption ();
	AfxGetApp ()->AddToRecentFileList (m_szFile);
	}
return (err == 0);
#endif
//	return CDocument::OnSaveDocument(lpszPathName);
}

                        /*--------------------------*/

BOOL CDlcDoc::OnOpenDocument (LPCTSTR lpszPathName) 
{
strcpy (m_szFile, lpszPathName);
return OpenFile (false);
}

                        /*--------------------------*/

BOOL CDlcDoc::OnSaveDocument (LPCTSTR lpszPathName) 
{
strcpy (m_szFile, lpszPathName);
return SaveFile (false);
}

                        /*--------------------------*/

void CDlcDoc::OnOpenFile () 
{
OpenFile ();
}

                        /*--------------------------*/

void CDlcDoc::OnSaveFile () 
{
SaveFile (false);
}

                        /*--------------------------*/

void CDlcDoc::OnSaveFileAs () 
{
SaveFile (true);
}

                        /*--------------------------*/

void CDlcDoc::OnRunLevel () 
{
SaveFile (false);
char *h, *p = strstr (m_szFile, "missions\\");
if (p) {
	char	szProg [255], szHogFile [255], szMission [255];

	strcpy (szProg, descent2_path);
	if (h = strstr (szProg, "data"))
		*h = '\0';
	for (int i = strlen (szProg); i && szProg [i - 1] != '\\'; i--)
		;
	szProg [i] = '\0';
	_chdir (szProg);
#ifdef _DEBUG
	strcat (szProg, "d2x-xl-dbg.exe");
#else
	strcat (szProg, "d2x-xl.exe");
#endif
	sprintf (szHogFile, "\"%s\"", p + strlen ("missions\\"));
	sprintf (szMission, "\"%s\"", m_szSubFile);
	i = _spawnl (_P_WAIT, szProg, szProg, 
					 *player_profile ? "-player" : "", player_profile, 
					 "-auto_hogfile", szHogFile, 
					 "-auto_mission", szMission, 
					 NULL);
	if (i < 0)
		i = errno;
	}
}

                        /*--------------------------*/

void CDlcDoc::OnInsertCube() 
{
if (m_mine) m_mine->AddSegment ();
}

void CDlcDoc::OnDeleteCube() 
{
if (m_mine) m_mine->DeleteSegment();
}

void CDlcDoc::OnInsertCubeReactor ()
{
if (m_mine) m_mine->AddReactor ();
}

void CDlcDoc::OnInsertCubeRobotMaker ()
{
if (m_mine) m_mine->AddRobotMaker ();
}

void CDlcDoc::OnInsertCubeFuelCenter ()
{
if (m_mine) m_mine->AddFuelCenter ();
}

void CDlcDoc::OnInsertCubeRepairCenter ()
{
if (m_mine) m_mine->AddFuelCenter (-1, SEGMENT_IS_REPAIRCEN);
}

void CDlcDoc::OnInsertCubeWater ()
{
if (m_mine) m_mine->AddWaterCube ();
}

void CDlcDoc::OnInsertCubeLava ()
{
if (m_mine) m_mine->AddLavaCube ();
}

void CDlcDoc::OnInsertDoorNormal ()
{
if (m_mine) m_mine->AddAutoDoor ();
}

void CDlcDoc::OnInsertDoorPrison ()
{
if (m_mine) m_mine->AddPrisonDoor ();
}

void CDlcDoc::OnInsertDoorGuideBot ()
{
if (m_mine) m_mine->AddGuideBotDoor ();
}

void CDlcDoc::OnInsertDoorExit ()
{
if (m_mine) m_mine->AddNormalExit ();
}

void CDlcDoc::OnInsertDoorSecretExit ()
{
if (m_mine) m_mine->AddSecretExit ();
}

void CDlcDoc::OnInsertTriggerOpenDoor ()
{
if (m_mine) m_mine->AddOpenDoorTrigger ();
}

void CDlcDoc::OnInsertTriggerRobotMaker ()
{
if (m_mine) m_mine->AddRobotMakerTrigger ();
}

void CDlcDoc::OnInsertTriggerShieldDrain ()
{
if (m_mine) m_mine->AddShieldTrigger ();
}

void CDlcDoc::OnInsertTriggerEnergyDrain ()
{
if (m_mine) m_mine->AddEnergyTrigger ();
}

void CDlcDoc::OnInsertTriggerControlPanel ()
{
if (m_mine) m_mine->AddUnlockTrigger ();
}

void CDlcDoc::OnInsertWallFuelCells ()
{
if (m_mine) m_mine->AddFuelCell ();
}

void CDlcDoc::OnInsertWallIllusion ()
{
if (m_mine) m_mine->AddIllusionaryWall ();
}

void CDlcDoc::OnInsertWallForceField ()
{
if (m_mine) m_mine->AddForceField ();
}

void CDlcDoc::OnInsertWallFan ()
{
if (m_mine) m_mine->AddFan ();
}

void CDlcDoc::OnInsertWallGrate ()
{
if (m_mine) m_mine->AddGrate ();
}

void CDlcDoc::OnInsertWallWaterfall ()
{
if (m_mine) m_mine->AddWaterFall ();
}

void CDlcDoc::OnInsertWallLavafall ()
{
if (m_mine) m_mine->AddLavaFall ();
}

void CDlcDoc::OnInsertObjectPlayer ()
{
if (m_mine) m_mine->CopyObject (OBJ_PLAYER);
}

void CDlcDoc::OnInsertObjectCoopPlayer ()
{
if (m_mine) m_mine->CopyObject (OBJ_COOP);
}

void CDlcDoc::OnInsertObjectPlayerCopy ()
{
if (m_mine) 
	m_mine->CopyObject (OBJ_NONE);
}

void CDlcDoc::OnInsertObjectRobot ()
{
if (m_mine && m_mine->CopyObject (OBJ_ROBOT)) {
	m_mine->CurrObj ()->id = 3; // class 1 drone
	m_mine->SetObjectData (m_mine->CurrObj ()->type);
	}
}

void CDlcDoc::OnInsertObjectWeapon ()
{
if (m_mine && m_mine->CopyObject (OBJ_WEAPON)) {
	m_mine->CurrObj ()->id = 3; // laser
	m_mine->SetObjectData (m_mine->CurrObj ()->type);
	}
}

void CDlcDoc::OnInsertObjectPowerup ()
{
if (m_mine && m_mine->CopyObject (OBJ_POWERUP)) {
	m_mine->CurrObj ()->id = 1; // energy boost
	m_mine->SetObjectData (m_mine->CurrObj ()->type);
	}
}

void CDlcDoc::OnInsertObjectGuideBot ()
{
if (m_mine && m_mine->CopyObject (OBJ_ROBOT)) {
	m_mine->CurrObj ()->id = 33; // guide bot
	m_mine->SetObjectData (m_mine->CurrObj ()->type);
	}
}

void CDlcDoc::OnInsertObjectReactor ()
{
if (m_mine && m_mine->CopyObject (OBJ_CNTRLCEN)) {
	m_mine->CurrObj ()->id = 2; // standard reactor
	m_mine->SetObjectData (m_mine->CurrObj ()->type);
	}
}

void CDlcDoc::OnDeleteObject ()
{
if (m_mine && (QueryMsg ("Are you sure you want to delete this object?") == IDYES))
	m_mine->DeleteObject ();
}

void CDlcDoc::OnDeleteWall ()
{
if (m_mine) m_mine->DeleteWall ();
}

void CDlcDoc::OnDeleteTrigger ()
{
if (m_mine) m_mine->DeleteTrigger ();
}

void CDlcDoc::OnCutBlock ()
{
if (m_mine) m_mine->CutBlock ();
}

void CDlcDoc::OnCopyBlock ()
{
if (m_mine) m_mine->CopyBlock ();
}

void CDlcDoc::OnQuickCopyBlock ()
{
if (m_mine) m_mine->CopyBlock ("dle_temp.blx");
}

void CDlcDoc::OnPasteBlock ()
{
if (m_mine) m_mine->PasteBlock ();
}

void CDlcDoc::OnQuickPasteBlock ()
{
if (m_mine) m_mine->QuickPasteBlock ();
}

void CDlcDoc::OnDeleteBlock ()
{
if (m_mine) m_mine->DeleteBlock ();
}

void CDlcDoc::OnCopyOtherCube ()
{
if (m_mine) m_mine->CopyOtherCube ();
}


long CDlcDoc::Test() 
{
	// TODO: Add your dispatch handler code here

	return 0;
}

void CDlcDoc::SetMyText(LPCTSTR string) 
{
//	 m_Text.Format(string);
}

void CDlcDoc::OnFileTest() 
{
	CParser parser(m_mine);
	
	parser.RunScript();
	
}

