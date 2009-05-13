// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <math.h>
#include <io.h>
#include <string.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "dlc.h"
#include "mine.h"
#include "global.h"
#include "robot.h"
#include "textures.h"
#include "robot.h"
#include "dlcres.h"
#include "io.h"
#include "file.h"
#include "light.h"
#include "palette.h"

// local globals
char *VERTIGO_HINT =
	"Vertigo levels allow you to use 12 robots in addition\n"
	"to the 66 standard Descent II robots.  But, these robots will\n"
	"not appear in the game unless you play the \"Descent 2: Vertigo\"\n"
	"mission after you start Descent II and before you play your level.\n";


char cut_paste_filename [256] = "";

// local prototypes
int read_block_file(char *name,int option);
void strip_extension(char *str);

void DeleteSubFile (FILE *file, long size, long offset, int num_entries, int delete_index);
void ImportSubFile ();
bool ExportSubFile (const char *pszSrc, const char *pszDest, long offset, long size);
void RenameSubFile ();
void DeleteLevelSubFiles(FILE *file,char *name);

                         /*--------------------------*/

static UINT8 dataBuf [65536];

                         /*--------------------------*/

int AddFileData (CListBox *plb, int index, int size, int offset, int fileno)
{
if (index == -1)
	index = plb->GetCurSel ();
if (index == -1)
	return -1;
tHogFileData *pfd = new tHogFileData;
if (!pfd)
	return -1;
pfd->m_size = size;
pfd->m_offs = offset;
pfd->m_fileno = fileno;
plb->SetItemDataPtr (index, (void *) pfd);
return index;
}

                         /*--------------------------*/

void ClearFileList (CListBox *plb)
{
	int h = plb->GetCount ();
	tHogFileData *pfd;

for (int i = 0; i < h; i++)
	if (pfd = (tHogFileData *) plb->GetItemDataPtr (i))
		delete pfd;
plb->ResetContent ();
}

                         /*--------------------------*/

CInputDialog::CInputDialog (CWnd *pParentWnd, LPSTR pszTitle, LPSTR pszPrompt, LPSTR pszBuf, size_t nBufSize)
	: CDialog (IDD_INPDLG, pParentWnd)
{
m_pszTitle = pszTitle;
m_pszPrompt = pszPrompt;
m_pszBuf = pszBuf;
m_nBufSize = nBufSize;
}

                         /*--------------------------*/

BOOL CInputDialog::OnInitDialog (void)
{
CDialog::OnInitDialog ();
SetWindowText (m_pszTitle);
return TRUE;
}

                         /*--------------------------*/

void CInputDialog::DoDataExchange (CDataExchange * pDX)
{
DDX_Text (pDX, IDC_INPDLG_PROMPT, m_pszPrompt, strlen (m_pszPrompt));
DDX_Text (pDX, IDC_INPDLG_BUF, m_pszBuf, m_nBufSize);
}

                         /*--------------------------*/

void CInputDialog::OnOK (void)
{
UpdateData (TRUE);
CDialog::OnOK ();
}

                         /*--------------------------*/

BEGIN_MESSAGE_MAP (CHogManager, CDialog)
	ON_BN_CLICKED (IDC_HOG_RENAME, OnRename)
	ON_BN_CLICKED (IDC_HOG_DELETE, OnDelete)
	ON_BN_CLICKED (IDC_HOG_IMPORT, OnImport)
	ON_BN_CLICKED (IDC_HOG_EXPORT, OnExport)
	ON_BN_CLICKED (IDC_HOG_FILTER, OnFilter)
	ON_LBN_SELCHANGE (IDC_HOG_FILES, OnSetFile)
	ON_LBN_DBLCLK (IDC_HOG_FILES, OnOK)
END_MESSAGE_MAP ()

                        /*--------------------------*/

bool BrowseForFile (BOOL bOpen, LPSTR pszDefExt, LPSTR pszFile, LPSTR pszFilter, DWORD nFlags, CWnd *pParentWnd)
{
   int         nResult;
   char		   pn [256];

if (*pszFile)
	strcpy (pn, pszFile);
else if (*pszDefExt)
	sprintf (pn, "*.%s", pszDefExt);
else
	strcpy (pn, "*.*");
CFileDialog d (bOpen, pszDefExt, pn, nFlags, pszFilter, pParentWnd);
d.m_ofn.hInstance = AfxGetInstanceHandle ();
d.m_ofn.lpstrInitialDir = pn;
if ((nResult = d.DoModal ()) != IDOK)
	return false;
strcpy (pszFile, d.m_ofn.lpstrFile);
return true;
}

                         /*--------------------------*/

CHogManager::CHogManager (CWnd *pParentWnd, LPSTR pszFile, LPSTR pszSubFile)
	: CDialog (IDD_HOGMANAGER, pParentWnd) 
{
m_bInited = false;
m_pszFile = pszFile;
m_pszSubFile = pszSubFile;
m_bShowAll = false;
}

                         /*--------------------------*/

void CHogManager::EndDialog (int nResult)
{
if (m_bInited)
	ClearFileList ();
CDialog::EndDialog (nResult);
}

                         /*--------------------------*/

void CHogManager::Reset (void)
{
/*
m_fileData.m_nFiles = 0;
for (int i = 0; i < MAX_HOGFILES - 1; i++)
	m_fileData.m_size [i] = i + 1;
m_fileData.m_size [i] = -1;
m_fileData.m_nFreeList = 0;
*/
}

                         /*--------------------------*/

BOOL CHogManager::OnInitDialog (void)
{
CDialog::OnInitDialog ();
if (ReadHogData ()) {
	m_bInited = true;
	return TRUE;
	}
EndDialog (0);
return FALSE;
}

                         /*--------------------------*/

void CHogManager::DoDataExchange (CDataExchange * pDX)
{
	long size, offset;

GetFileData (-1, &size, &offset);
DDX_Text (pDX, IDC_HOG_SIZE, size);
DDX_Text (pDX, IDC_HOG_OFFSET, offset);
DDX_Check (pDX, IDC_HOG_FILTER, m_bShowAll);
}

                         /*--------------------------*/

void CHogManager::OnCancel (void)
{
CDialog::OnCancel ();
}

                         /*--------------------------*/

void CHogManager::OnFilter (void)
{
m_bShowAll = ((CButton *) GetDlgItem (IDC_HOG_FILTER))->GetCheck ();
if (!ReadHogData ())
	EndDialog (0);
}

                         /*--------------------------*/

void CHogManager::OnSetFile (void)
{
UpdateData (FALSE);
}

                         /*--------------------------*/

void CHogManager::ClearFileList (void)
{
::ClearFileList (LBFiles ());
}

                         /*--------------------------*/

int CHogManager::DeleteFile (int index)
{
if (index == -1)
	index = LBFiles ()->GetCurSel ();
if (index == -1)
	return -1;
tHogFileData *pfd = (tHogFileData *) LBFiles ()->GetItemDataPtr (index);
if (pfd)
	delete pfd;
LBFiles ()->DeleteString (index);
return 0;
}

                         /*--------------------------*/

int CHogManager::AddFile (LPSTR pszName, long size, long offset, int fileno)
{
	int index = LBFiles ()->AddString (strlwr (pszName));

if (0 > AddFileData (index, size, offset, fileno))
	return -1;
LBFiles ()->SetCurSel (index);
return 0;
}

                         /*--------------------------*/

int CHogManager::GetFileData (int index, long *size, long *offset)
{
if (index == -1)
	index = LBFiles ()->GetCurSel ();
if (index == -1)
	return -1;
tHogFileData *pfd = (tHogFileData *) LBFiles ()->GetItemDataPtr (index);
if (!pfd)
	return -1;
if (size)
	*size = pfd->m_size;
if (offset)
	*offset = pfd->m_offs;
return pfd->m_fileno;
}

                         /*--------------------------*/

int CHogManager::AddFileData (int index, long size, long offset, int fileno)
{
return ::AddFileData (LBFiles (), index, size, offset, fileno);
}

                         /*--------------------------*/

int CHogManager::FindFilename (LPSTR pszName)
{
	CListBox	*plb = LBFiles ();
	char szName [256];

for (int h = plb->GetCount (), i = 0; i < h; i++) {
	plb->GetText (i, szName);
	if (!strcmpi (szName, pszName))
		return i;
	}
return -1;
}

//------------------------------------------------------------------------
// CHogManager - read hog data
//------------------------------------------------------------------------

bool CHogManager::ReadHogData () 
{
ClearFileList ();
Reset ();
if (!::ReadHogData (m_pszFile, LBFiles (), m_bShowAll == 1, false))
	return false;
UpdateData (FALSE);
return true;
}

//------------------------------------------------------------------------
// CHogManager - load level
//------------------------------------------------------------------------

bool CHogManager::LoadLevel (LPSTR pszFile, LPSTR pszSubFile) 
{
	FILE*		fTmp = NULL, *fSrc = NULL;
	long		size,offset;
	char		szTmp[256];
	int		chunk;
	char*		pszExt;
	int		index = -1;
	bool		funcRes = false;
	CMine*	mine = theApp.GetMine ();

if (!pszFile)
	pszFile = m_pszFile;
if (pszSubFile) {
	if (!FindFileData (pszFile, pszSubFile, &size, &offset))
		return false;
	strcpy (m_pszSubFile, pszSubFile);
	}
else if (0 > (index = GetFileData (-1, &size, &offset)))
	goto errorExit;
FreeTextureHandles ();


FSplit (pszFile, szTmp, NULL, NULL);
strcat (szTmp, "dle_temp.rdl");
fTmp = fopen (szTmp, "wb");
fSrc = fopen (pszFile, "rb");
// copy level to a temporary fTmp
if (!(fTmp && fSrc)) {
	ErrorMsg ("Unable to create temporary DLE-XP work file.");
	goto errorExit;
	}
// set subfile name
fseek (fSrc, sizeof (struct level_header) + offset, SEEK_SET);
size_t fPos = ftell (fSrc);
if (mine->LoadMineSigAndType (fSrc))
	goto errorExit;
fseek (fSrc, fPos, SEEK_SET);
while (size > 0) {
	chunk = (size > sizeof (dataBuf)) ? sizeof (dataBuf) : size;
	fread (dataBuf, 1, chunk, fSrc);
	if (!chunk)
		break;
	fwrite(dataBuf, 1, chunk, fTmp);
	size -= chunk;
	}

// read custom palette if one exists
strcpy (message, m_pszSubFile);
pszExt = strrchr (message, '.');
if (pszExt) {
	sprintf (pszExt,".pal");
	if (pszSubFile)
		FindFileData (pszFile, message, &size, &offset);
	else {
		index = FindFilename (message);
		if (index < 0)
			size = offset = -1;
		else
			GetFileData (index, &size, &offset);
		}
	if ((size > 0) || (offset >= 0)) {
		fseek (fSrc, sizeof (struct level_header) + offset, SEEK_SET);
		int h = ftell (fSrc);
		ReadCustomPalette (fSrc, size);
		h = ftell (fSrc) - h;
		}
	}
// read custom light values if a lightmap file exists
strcpy (message, m_pszSubFile);
pszExt = strrchr (message, '.');
if (pszExt) {
	sprintf (pszExt, ".lgt");
	if (pszSubFile)
		FindFileData (pszFile, message, &size, &offset);
	else {
		index = FindFilename (message);
		if (index < 0)
			size = offset = -1;
		else
			GetFileData (index, &size, &offset);
		}
	if ((size < 0) || (offset < 0))
		CreateLightMap ();
	else {
		fseek (fSrc, sizeof (struct level_header) + offset, SEEK_SET);
		int h = ftell (fSrc);
		ReadLightMap (fSrc, size);
		h = ftell (fSrc) - h;
		}
	}

//memset (mine->TexColors (), 0, sizeof (mine->MineData ().texColors));
sprintf (pszExt, ".clr");
if (pszSubFile)
	FindFileData (pszFile, message, &size, &offset);
else {
	index = FindFilename (message);
	if (index < 0)
		size = offset = -1;
	else
		GetFileData (index, &size, &offset);
	}
if ((size >= 0) && (offset >= 0)) {
	fseek (fSrc, sizeof (struct level_header) + offset, SEEK_SET);
	int h = ftell (fSrc);
	mine->ReadColorMap (fSrc);
	h = ftell (fSrc) - h;
	}
// read custom textures if a pog file exists
strcpy (message, m_pszSubFile);
pszExt = strrchr (message, '.');
if (pszExt) {
	sprintf (pszExt, ".pog");
	if (pszSubFile)
		FindFileData (pszFile, message, &size, &offset);
	else {
		index = FindFilename (message);
		if (index < 0)
			size = offset = -1;
		else
			GetFileData (index, &size, &offset);
		}
	if ((size > 0) && (offset >= 0)) {
		fseek(fSrc, sizeof (struct level_header) + offset, SEEK_SET);
		int h = ftell (fSrc);
		ReadPog (fSrc, size);
		h = ftell (fSrc) - h;
		}
	}
// read custom robot info if hxm fTmp exists
strcpy (message, m_pszSubFile);
pszExt = strrchr (message, '.');
if (pszExt) {
	sprintf (pszExt, ".hxm");
	if (pszSubFile)
		FindFileData (pszFile, message, &size, &offset);
	else {
		index = FindFilename (message);
		if (index < 0)
			size = offset = -1;
		else
			GetFileData (index, &size, &offset);
		}
	if ((size >= 0) && (offset >= 0)) {
		fseek (fSrc, sizeof (struct level_header) + offset, SEEK_SET);
		theApp.GetMine ()->ReadHxmFile (fSrc, size);
		CMine *mine = theApp.GetMine ();
		for (int i = 0, count = 0; i < (int) N_robot_types;i++)
			if (mine->RobotInfo (i)->pad [0])
				count++;
		sprintf (message," Hog manager: %d custom robots read", count);
		DEBUGMSG (message);
		}
	}
funcRes = true;

errorExit:

if (fTmp) 
	fclose (fTmp);
if (fSrc) 
	fclose (fSrc);
return funcRes;
}

//------------------------------------------------------------------------
// CHogManager::Ok()
//
// Saves file to a temporary file called dle_temp.rdl so editor can load it
//------------------------------------------------------------------------

void CHogManager::OnOK () 
{
LBFiles ()->GetText (LBFiles ()->GetCurSel (), m_pszSubFile);
char *pszExt = strrchr ((char *) m_pszSubFile, '.');
if (pszExt && strcmpi (pszExt,".rdl") && strcmpi (pszExt,".rl2")) {
#if 1
	ErrorMsg ("DLE-XP cannot process this file. To change the file,\n\n"
				 "export it and process it with the appropriate application.\n"
				 "To incorporate the changes in the HOG file,\n"
				 "import the modified file back into the HOG file.");
	return;
	}
#else //0
	if (QueryMsg ("Would you like to view this file\n"
					  "from an associated windows program?\n\n"
					  "Note: Any changes to this file will affect\n"
					  "the exported file and will have to be imported\n"
					  "back into the HOG file when finished.") == IDOK) {
		SizeListBox->GetString (message, index);
		long size = atol(message);
		OffsetListBox->GetString(message,index);
		long offset = atol(message) + sizeof(level_header);

		// Set all structure members to zero.
		OPENFILENAME ofn;
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = HWindow;
		ofn.lpstrFilter = "All Files\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrDefExt = pszExt;
		strcpy(szTmp, m_pszSubFile);
		ofn.lpstrFile= szTmp;
		ofn.nMaxFile = sizeof(szTmp);
		ofn.Flags =   OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
		if (GetSaveFileName(&ofn)) {
		Export (m_pszName, szTmp, offset, size);
		HINSTANCE hinst = FindExecutable(szTmp, starting_directory, message);
		if (hinst > (HINSTANCE)32)
			ShellExecute(HWindow, 0, message, szTmp, starting_directory, SW_SHOWNORMAL);
		else {
			ErrorMsg ("No application is \"associated\" with this extention.\n\n"
						 "Hint: You can use File Manager to associate this fTmp's\n"
						 "extension with an application. File Manager can be found\n"
						 "in your windows directory under the szName WINFILE.EXE");
				}
			}
		}
	return;
	}
#endif //1

LoadLevel ();
CDialog::OnOK ();
}

//------------------------------------------------------------------------
// CHogManager::RenameMsg()
//------------------------------------------------------------------------

void CHogManager::OnRename ()
{
	char buf[20];
	int index = LBFiles ()->GetCurSel ();

if (index < 0)
	return;
LBFiles ()->GetText (index, buf);
CInputDialog dlg (this, "Rename file", "Enter new name:",(char *) buf, sizeof (buf));
if (dlg.DoModal () != IDOK)
	return;
if (FindFilename (buf) >= 0) {
	ErrorMsg ("A file with that name already exists\nin the HOG file.");
	return;
	}
FILE *hogfile;
hogfile = fopen (m_pszFile,"r+b"); // add it to the end
if (!hogfile) {
	ErrorMsg("Could not open HOG file.");
	return;
	}
long size, offset;
level_header lh;
int fileno = GetFileData (index, &size, &offset);
fseek (hogfile, offset, SEEK_SET);
if (!fread (&lh, sizeof (lh), 1, hogfile))
	ErrorMsg("Cannot read from HOG file");
else {
	memset(lh.name, 0, sizeof (lh.name));
	buf[12] = NULL;
	strlwr(buf);
	strncpy(lh.name,buf,12);
	fseek(hogfile,offset,SEEK_SET);
	if (!fwrite(&lh,sizeof(lh),1,hogfile))
		ErrorMsg ("Cannot write to HOG file");
	else {
		// update list box
		DeleteFile ();
		AddFile (lh.name, size, offset, fileno);
/*
		index = LBFiles ()->GetCurSel ();
		LBFiles ()->DeleteString (index);
		int i = LBFiles ()->AddString (lh.name);
		LBFiles ()->SetItemData (i, index);
		LBFiles ()->SetCurSel (index);
*/
		}
	}
fclose(hogfile);
}

//------------------------------------------------------------------------
// CHogManager - ImportMsg
//
// Adds file to the end of the list
//------------------------------------------------------------------------

void CHogManager::OnImport () 
{
	long offset;

	char szFile[256] = "\0"; // buffer for file name
	level_header lh;
//  DWORD index;

//  // make sure there is an item selected
//  index = LBFiles ()->GetCurSel ();
//  if (index < 0) {
//    ErrorMsg("You must select an item first.");
//  } else {
	// Set all structure members to zero.
#if 0
if (!BrowseForFile (TRUE, (file_type == RDL_FILE) ? "rdl" : "rl2", szFile, 
						  "Descent Level|*.rdl|"
						  "Descent 2 Level|*.rl2|"
						  "Texture file|*.pog|"
						  "Robot file|*.hxm|"
						  "Lightmap file|*.lgt|"
						  "Color file|*.clr|"
						  "Palette file|*.pal|"
						  "All Files|*.*||",
						  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
						  theApp.MainFrame ())
	return;
#else
	OPENFILENAME ofn;
memset(&ofn, 0, sizeof (OPENFILENAME));
ofn.lStructSize = sizeof (OPENFILENAME);
ofn.hwndOwner = GetSafeHwnd ();
ofn.lpstrFilter = "Descent Level\0*.rdl\0"
						"Descent 2 Level\0*.rl2\0"
						"Texture file\0*.pog\0"
						"Robot file\0*.hxm\0"
						"Lightmap file\0*.lgt\0"
						"Color file\0*.clr\0"
						"Palette file\0*.pal\0"
						"All Files\0*.*\0";
if (file_type == RDL_FILE) {
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = "rdl";
	}
else {
	ofn.nFilterIndex = 2;
	ofn.lpstrDefExt = "rl2";
	}
ofn.lpstrFile= szFile;
ofn.nMaxFile = sizeof(szFile);
ofn.lpstrFileTitle = lh.name;
ofn.nMaxFileTitle = sizeof(lh.name);
ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
if (!GetOpenFileName (&ofn))
	return;
#endif

FILE *fDest = fopen (m_pszFile, "ab"); // add it to the end
if (!fDest) {
	ErrorMsg ("Could not open destination HOG file for import.");
	return;
	}
FILE *fSrc = fopen (szFile, "rb");
if (!fSrc) {
	ErrorMsg ("Could not open source file for import.");
	fclose (fDest);
	return;
	}
fseek (fDest, 0, SEEK_END);
offset = ftell (fDest);
// write header
lh.size = filelength (fileno (fSrc));
strlwr (lh.name);
fwrite (&lh, sizeof (level_header), 1, fDest);

	// write data (from source to HOG)
while(!feof(fSrc)) {
	int n_bytes = fread (dataBuf, 1, sizeof (dataBuf), fSrc);
	if (n_bytes <= 0)
		break;
	fwrite (dataBuf, 1, n_bytes, fDest);
	}
fclose(fSrc);
fclose(fDest);
// update list boxes
AddFile (lh.name, lh.size, offset, LBFiles ()->GetCount ());
/*
int index = LBFiles ()->AddString (lh.name);
AddFileData (index, lh.size, offset);
LBFiles ()->SetCurSel (index);
*/
}

//------------------------------------------------------------------------
// CHogManager - ExportMsg
//
// Exports selected item to a file
//------------------------------------------------------------------------

void CHogManager::OnExport () 
{
	char szFile[256] = "\0"; // buffer for file name
	DWORD index;
	level_header lh;
	long size, offset;

// make sure there is an item selected
index = LBFiles ()->GetCurSel ();
if (index < 0) {
	ErrorMsg("Please select a file to export.");
	return;
	}
// get item name, size, and offset
LBFiles ()->GetText (index, lh.name);
GetFileData (index, &size, &offset);
lh.size = size;
offset += sizeof(level_header);
strcpy (szFile, lh.name);
#if 1
if (!BrowseForFile (FALSE, "", szFile, "All Files|*.*||",
						  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT,
						  theApp.MainFrame ()))
	return;
ExportSubFile (m_pszFile, szFile, offset, lh.size);
#else //0
// Set al structure members to zero.
	OPENFILENAME ofn;
memset(&ofn, 0, sizeof(OPENFILENAME));
ofn.lStructSize = sizeof(OPENFILENAME);
ofn.hwndOwner = GetSafeHwnd ();
ofn.lpstrFilter = "All Files\0*.*\0";
ofn.nFilterIndex = 1;
ofn.lpstrDefExt = "";
strcpy(szFile,lh.name);
ofn.lpstrFile= szFile;
ofn.nMaxFile = sizeof(szFile);
ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
if (GetSaveFileName (&ofn))
	ExportSubFile (m_pszFile, ofn.lpstrFile, offset, lh.size);
#endif
}

//------------------------------------------------------------------------
// CHogManager - DeleteMsg
//
// Deletes an item from a HOG file using the following steps:
//
// 1) Creates a new HOG file which does not contain the file selected
// 2) Deletes original HOG file
// 3) Renames new file to original file's name
//
//------------------------------------------------------------------------

void CHogManager::OnDelete () 
{
	int delete_index;
	long size;
	long offset;
	int fileno;
	FILE *hogFile = 0;
	CListBox * plb = LBFiles ();

// make sure there is an item selected
delete_index = plb->GetCurSel ();
if (delete_index < 0) {
	ErrorMsg ("Please choose a file to delete.");
	return;
	}
#if 1//ndef _DEBUG
if (!bExpertMode) {
	strcpy (message, "Are you sure you want to delete '");
	plb->GetText (delete_index, message + strlen(message));
	strcat(message,"'?\n(This operation will change the HOG hogFile immediately)");
	if (QueryMsg (message) != IDYES)
		return;
	}
#endif
fileno = GetFileData (delete_index, &size, &offset);
int nFiles = plb->GetCount ();
// open hog hogFile for modification
hogFile = fopen (m_pszFile,"r+b");
if (!hogFile) {
	ErrorMsg("Could not open hog hogFile.");
	return;
	}
DeleteSubFile (hogFile, size + sizeof (struct level_header), offset, nFiles, fileno);
fclose (hogFile);
ReadHogData ();
LBFiles ()->SetCurSel ((delete_index < nFiles - 1) ? delete_index : nFiles - 1);
}

//------------------------------------------------------------------------
// CHogManager - read hog data
//------------------------------------------------------------------------

bool ReadHogData (LPSTR pszFile, CListBox *plb, bool bAllFiles, bool bOnlyLevels, bool bGetFileData) 
{
	struct level_header *level;
	char data [256];
	long position;
	UINT8 index;
	FILE *hog_file;
	int nFiles;

ClearFileList (plb);
hog_file = fopen(pszFile,"rb");
if (!hog_file) {
	sprintf(message,"Unable to open HOG file (%s)",pszFile);
	ErrorMsg(message);
	return false;
	}
fread(data,3,1,hog_file); // verify signature "DHF"
if (data[0] != 'D' || data[1] != 'H' || data[2] != 'F') {
	ErrorMsg("This is not a Descent HOG file");
	return false;
	}
position = 3;
nFiles = 0;
while (!feof (hog_file)) {
	fseek (hog_file, position, SEEK_SET);
	if (fread (data, sizeof (struct level_header), 1, hog_file) != 1) 
		break;
	level = (struct level_header *) data;
	if (level->size < 0) {
		ErrorMsg ("Error reading HOG file");
		fclose (hog_file);
		return false;
		}
	level->name [sizeof (level->name) - 1] = 0; // null terminate in case its bad
	strlwr (level->name);
	if (bAllFiles 
		 || strstr (level->name, ".rdl") || strstr (level->name, ".rl2")
		 || (!bOnlyLevels && (strstr (level->name, ".pog") || 
									 strstr (level->name, ".hxm") || 
									 strstr (level->name, ".lgt") || 
									 strstr (level->name, ".clr") || 
									 strstr (level->name, ".pal")))) {
		int i = plb->AddString (strlwr (level->name));
		if (bGetFileData && (0 > AddFileData (plb, i, level->size, position, nFiles))) {
			ErrorMsg ("Too many files in HOG file.");
			fclose (hog_file);
			return false;
			}
		plb->SetCurSel (i);
		nFiles++;
		}
	position += sizeof (struct level_header) + level->size;
	}
fclose (hog_file);
// select first level file
for (index = 0; index < nFiles; index++) {
	message [0] = NULL;
	plb->GetText (index, message);
	strlwr (message);
	if (strstr(message, ".rdl")) 
		break;
	if (strstr(message, ".rl2")) 
		break;
	}
if (index == nFiles)
	index = 0;
plb->SetCurSel (index);
return true;
}

//------------------------------------------------------------------------
// CHogManager - read hog data
//------------------------------------------------------------------------

bool FindFileData (LPSTR pszFile, LPSTR pszSubFile, long *nSize, long *nPos, BOOL bVerbose) 
{
	struct level_header *level;
	char data [256];
	long position;
	FILE *hog_file;
	int nFiles;

*nSize = -1;
*nPos = -1;
hog_file = fopen(pszFile,"rb");
if (!hog_file) {
	if (bVerbose) {
		sprintf(message,"Unable to open HOG file (%s)",pszFile);
		ErrorMsg(message);
		}
	return false;
	}
fread(data,3,1,hog_file); // verify signature "DHF"
if (data[0] != 'D' || data[1] != 'H' || data[2] != 'F') {
	if (bVerbose)
		ErrorMsg("This is not a Descent HOG file");
	return false;
	}
position = 3;
nFiles = 0;
while (!feof (hog_file)) {
	fseek (hog_file, position, SEEK_SET);
	if (fread (data, sizeof (struct level_header), 1, hog_file) != 1) 
		break;
	level = (struct level_header *) data;
	if (level->size > 10000000L || level->size < 0) {
		if (bVerbose)
			ErrorMsg ("Error reading HOG file");
		fclose (hog_file);
		return false;
		}
	level->name [sizeof (level->name) - 1] = 0; // null terminate in case its bad
	strlwr (level->name);
	if (!strcmp (pszSubFile, "*"))
		strcpy (pszSubFile, level->name);
	if (!strcmpi (level->name, pszSubFile)) {
		*nSize = level->size;
		*nPos = position;
		fclose (hog_file);
		return true;
		}
	position += sizeof (struct level_header) + level->size;
	}
fclose (hog_file);
return false;
}

//------------------------------------------------------------------------
// extract_from_hog()
//------------------------------------------------------------------------

bool ExportSubFile (const char *pszSrc, const char *pszDest, long offset, long size) 
{
FILE *fSrc = fopen(pszSrc,"rb");
if (!fSrc) {
	ErrorMsg("Could not open HOG file.");
	return false;
	}
FILE *fDest = fopen (pszDest,"wb");
if (!fDest) {
	ErrorMsg("Could not create export file.");
	fclose (fSrc);
	return false;
	}
// seek to item's offset in HOG file
fseek(fSrc,offset,SEEK_SET);
// create file (from HOG to file)
while(size > 0) {
	int n_bytes,n;
	n = (size > sizeof (dataBuf)) ? sizeof (dataBuf) : size;
	n_bytes = fread (dataBuf, 1, n, fSrc);
	fwrite (dataBuf, 1, n_bytes, fDest);
	size -= n_bytes;
	if (n_bytes != n)
		break;
	}
fclose(fDest);
fclose(fSrc);
return (size == 0);
}


//----------------------------------------------------------------------------
// delete_sub_file()
//----------------------------------------------------------------------------

void DeleteSubFile (FILE *file, long size, long offset, int num_entries, int delete_index) 
{
int n_bytes;
// as long as we are not deleting the last item
if (delete_index < num_entries - 1) {
	// get size of chunk to remove from the file, then move everything
	// down by that amount.
	do {
		fseek (file, offset + size, SEEK_SET);
		n_bytes = fread (dataBuf, 1, sizeof (dataBuf), file);
		if (n_bytes <= 0)
			break;
		fseek (file, offset, SEEK_SET);
		fwrite (dataBuf, 1, n_bytes, file);
		offset += n_bytes;
		} while (n_bytes > 0);
	}
// set the new size of the file
chsize (fileno (file), offset);
}

//--------------------------------------------------------------------------
// strip_extension
//--------------------------------------------------------------------------

void strip_extension(char *str) 
{
char *ext = strrchr(str,'.');
if (ext)
	*ext = '\0';
}

//--------------------------------------------------------------------------
// DeleteLevelSubFiles()
//
// deletes sub-files with same base name from hog
//--------------------------------------------------------------------------

#define MAX_REGNUM 6

void DeleteLevelSubFiles (FILE *file, char *base) 
{
struct region {
	int index;
	int offset;
	int size;
	int files;
	};
	int regnum = 0;
	int delete_index = 0;
	int num_entries = 0;
	region reg [MAX_REGNUM] = {{-1,0,0,0},{-1,0,0,0},{-1,0,0,0},{-1,0,0,0},{-1,0,0,0},{-1,0,0,0}};

// figure out regions of the file to delete (3 regions max)
long offset = 3;
long size;
delete_index = -1;
while(!feof (file)) {
	level_header lh = {"",0};
	fseek (file, offset, SEEK_SET); // skip "HOG"
	if (!fread (&lh, sizeof (lh), 1, file)) 
		break;
	size = lh.size + sizeof (lh);
	if (regnum < MAX_REGNUM) {
		strlwr (lh.name);
		LPSTR ext = strrchr (lh.name, '.');
		if (!ext)
			goto nextSubFile;
		if (strcmpi (ext, ".rdl") &&
		    strcmpi (ext, ".rl2") &&
		    strcmpi (ext, ".hxm") &&
		    strcmpi (ext, ".pog") &&
		    strcmpi (ext, ".pal") &&
		    strcmpi (ext, ".lgt") &&
		    strcmpi (ext, ".clr"))
			goto nextSubFile;
		*ext = '\0';
		if (strcmpi (base, lh.name))
			goto nextSubFile;
		// try to merge this with the last region
		if ((regnum > 0) && (delete_index == num_entries - 1)) {
			reg[regnum-1].size += size;
			reg[regnum-1].files++;
			num_entries--; // pretend that there is one less entry
			}
		else {
			reg[regnum].index = num_entries;
			reg[regnum].offset = offset;
			reg[regnum].size = size;
			reg[regnum].files++;
			regnum++;
			}
		delete_index = num_entries;
		}
nextSubFile:
	offset += size;
	num_entries++;
	}

// now delete matching regions
while (regnum > 0) {
	regnum--;
	DeleteSubFile (file, reg [regnum].size, reg [regnum].offset, num_entries, reg [regnum].index);
	num_entries -= reg [regnum].files;
	}
fclose (file);
}

#undef MAX_REGNUM

//==========================================================================
// write_sub_file()
//==========================================================================

int WriteSubFile (FILE *fDest, char *szSrc, char *szLevel) 
{
	FILE *fSrc;
	long n_bytes;
	level_header lh;

fSrc = fopen (szSrc,"rb");
if (!fSrc) {
	sprintf(message,"Unable to open temporary file:\n%s",szSrc);
	ErrorMsg(message);
	return -1;
	}
// write szLevel (13 chars, null filled)
memset (&lh, 0, sizeof (lh));
strncpy (lh.name, szLevel, 12);
strlwr(lh.name);
// calculate then write size
fseek (fSrc, 0, SEEK_END);
lh.size = ftell (fSrc);
//fclose (fSrc);
//fSrc = fopen (szSrc,"rb");
fseek (fSrc,0,SEEK_SET);
fwrite (&lh, sizeof (lh), 1, fDest);
// write data
while(!feof (fSrc)) {
	n_bytes = fread (dataBuf,1,sizeof(dataBuf),fSrc);
	if (n_bytes > 0)
		fwrite(dataBuf,1,n_bytes,fDest);
	}
fclose(fSrc);
return lh.size + sizeof (lh);
}

//==========================================================================
// make_hog()
//
// Action - makes a HOG file which includes three files
//             1. the rdl file to include (only one)
//             2. a briefing file (called brief.txb)
//             3. an ending sequence (same name as hog w/ .txb extension)
//          also makes a mission file for this HOG file
//
// Changes - now saves rl2 files
//==========================================================================

int MakeHog (char *rdlFilename, char *hogFilename, char*szSubFile, bool bSaveAs) 
{
	FILE *hogfile, *fTmp;
	char *name_start,*name_end, *ext_start;
	char base_name[13];
	char filename[256];
	char szTmp[256], szBase [13];
	int custom_robots = 0;
	int custom_textures = 0;
	CMine *mine = theApp.GetMine ();

// create HOG file which contains szTmp.rdl, szTmp.txb, and dlebrief.txb");
strcpy(filename,hogFilename);
hogfile = fopen(filename,"wb");
if (!hogfile) {
	sprintf(message,"Unable to create HOG file:\n%s",filename);
	ErrorMsg(message);
	return 1;
	}
// write file type
fwrite("DHF",1,3,hogfile); // starts with Descent Hog File
// get base szTmp w/o extension and w/o path
name_start = strrchr(hogFilename,'\\');
if (name_start==NULL)
	name_start = hogFilename;
else
	name_start++; // move to just pass the backslash
strncpy(base_name,name_start,12);
base_name[12] = NULL; // make sure it is null terminated
name_end = strrchr((char *)base_name,'.');
if (!name_end)
	name_end = base_name + strlen((char *)base_name);
for (;name_end < base_name + 12; name_end++)
	*name_end = '\0';
// write rdl file
if (*szSubFile) {
	for (ext_start = szSubFile; *ext_start && (*ext_start != '.'); ext_start++)
		;
	strncpy (base_name, szSubFile, ext_start - szSubFile);
	base_name [ext_start - szSubFile] = '\0';
	}
if (file_type == RDL_FILE)
	sprintf(szTmp,"%s.RDL",base_name);
else
	sprintf(szTmp,"%s.RL2",base_name);
WriteSubFile (hogfile, rdlFilename, szTmp);
unlink (szTmp);
#if 0
// write palette file into hog (if D2)
if (file_type == RL2_FILE) {
	if (strcmp(palette_resource(),"GROUPA_256") == 0) {
		char *start_name = strrchr(descent2_path,'\\');
		if (!start_name) {
			start_name = descent2_path; // point to 1st char if no slash found
			}
		else {
			start_name++;               // point to character after slash
			}
		strncpy(szTmp,start_name,12);
		szTmp[13] = NULL;  // null terminate just in case
		// replace extension with *.256
		if (strlen(szTmp) > 4) {
			strcpy(szTmp + strlen(szTmp) - 4,".256");
			}
		else {
			strcpy(szTmp,"GROUPA.256");
			}
		strupr(szTmp);
		sprintf(message,"%s\\groupa.256",starting_directory);
		write_sub_file(hogfile,message,szTmp);
		}
	}
#endif

if (mine->HasCustomLightMap ()) {
	FSplit (hogFilename, szTmp, NULL, NULL);
	strcat (szTmp, "dle_temp.lgt");
	fTmp = fopen (szTmp,"wb");
	if (fTmp) {
		if (!WriteLightMap (fTmp)) {
			fclose (fTmp);
			sprintf (szBase, "%s.lgt", base_name);
			WriteSubFile (hogfile, szTmp, szBase);
			}
		else
			fclose (fTmp);
		unlink (szTmp);
		}
	}
if (mine->HasCustomLightColors ()) {
	FSplit (hogFilename, szTmp, NULL, NULL);
	strcat (szTmp, "dle_temp.clr");
	fTmp = fopen (szTmp, "wb");
	if (fTmp) {
		if (!mine->WriteColorMap (fTmp)) {
			fclose (fTmp);
			sprintf (szBase, "%s.clr", base_name);
			WriteSubFile (hogfile, szTmp, szBase);
			}
		else
			fclose (fTmp);
		unlink (szTmp);
		}
	}
	
if (HasCustomPalette ()) {
	FSplit (hogFilename, szTmp, NULL, NULL);
	strcat (szTmp, "dle_temp.pal");
	fTmp = fopen (szTmp,"wb");
	if (fTmp) {
		if (!WriteCustomPalette (fTmp)) {
			fclose (fTmp);
			sprintf (szBase, "%s.pal", base_name);
			WriteSubFile (hogfile, szTmp, szBase);
			}
		else
			fclose (fTmp);
		unlink (szTmp);
		}
	}

// if textures have changed, ask if user wants to create a pog file
if (HasCustomTextures ()) {
	if (bExpertMode ||
		 QueryMsg("This level contains custom textures.\n"
					 "Would you like save these textures into the HOG file?\n\n"
					 "Note: You must use version 1.2 or higher of Descent2 to see\n"
					 "the textures when you play the game.") == IDYES) {
		FSplit (hogFilename, szTmp, NULL, NULL);
		strcat (szTmp, "dle_temp.pog");
		fTmp = fopen (szTmp,"wb");
		if (fTmp) {
			if (!CreatePog (fTmp)) {
				fclose (fTmp);
				sprintf (szBase, "%s.pog", base_name);
				WriteSubFile (hogfile, szTmp, szBase);
				custom_textures = 1;
				}
			else
				fclose (fTmp);
			unlink (szTmp);
			}
		}
	}
// if robot info has changed, ask if user wants to create a hxm file
if (theApp.GetMine ()->HasCustomRobots ()) {
	if (bExpertMode ||
		 QueryMsg ("This level contains custom robot settings.\n"
					  "Would you like save these changes into the HOG file?\n\n"
					  "Note: You must use version 1.2 or higher of Descent2 for\n"
					  "the changes to take effect.") == IDYES) {
		FSplit (hogFilename, szTmp, NULL, NULL);
		strcat (szTmp, "dle_temp.hxm");
		fTmp = fopen (szTmp, "wb");
		if (fTmp) {
			if (!theApp.GetMine ()->WriteHxmFile (fTmp)) {
				sprintf (szBase, "%s.hxm", base_name);
				WriteSubFile (hogfile, szTmp, szBase);
				custom_robots = 1;
				}
			unlink (szTmp);
			}
		}
	}
fclose (hogfile);
MakeMissionFile (hogFilename, szSubFile, custom_textures, custom_robots, bSaveAs);
return 0;
}

//==========================================================================
// MENU - Save
//==========================================================================

int SaveToHog (LPSTR szHogFile, LPSTR szSubFile, bool bSaveAs) 
{
	FILE	*fTmp;
	char szTmp [256], subName [256];
	char *psz;
	CMine *mine = theApp.GetMine ();

psz = strstr (strlwr (szHogFile), "new.");
if (!*szSubFile || psz) { 
	CInputDialog dlg (theApp.MainFrame (), "Name mine", "Enter file name:", szSubFile, 9);
	if (dlg.DoModal () != IDOK)
		return 1;
	LPSTR ext = strrchr (szSubFile, '.');
	if (ext)
		*ext = '\0';
	psz = strrchr (szSubFile, '.');
	if (psz)
		*psz = '\0';
	psz = strstr (strlwr (szHogFile), "new.");
	if (psz) {
		strcpy (psz, szSubFile);
		strcat (szHogFile, ".hog");
		}
	strcat (szSubFile, (file_type == RDL_FILE) ? ".rdl" : ".rl2");
	}
// if this HOG file only contains one rdl/rl2 file total and
// it has the same name as the current level, and it has
// no other files (besides hxm or pog files), then
// allow quick save
FILE *file;
// See if another level with the same name exists
// and see if there are any other files here (ignore hxm and pog files)
int bOtherFilesFound = 0;
int bIdenticalLevelFound = 0;
file = fopen (szHogFile, "rb");
if (!file) {
	FSplit (szHogFile, szTmp, NULL, NULL);
	strcat (szTmp, "dle_temp.rdl");
	theApp.GetMine ()->Save (szTmp);
	return MakeHog (szTmp, szHogFile, szSubFile, true);
	}
fseek(file,3,SEEK_SET); // skip "HOG"
while(!feof(file)) {
	level_header lh = {"",0};
	if (!fread (&lh, sizeof (lh), 1, file)) 
		break;
	strlwr (lh.name);
	lh.name [sizeof (lh.name) - 1] = NULL; // null terminate
	if (!strstr ((char *) lh.name, ".hxm") &&     // not a custom robot file
		 !strstr ((char *) lh.name, ".pog") &&     // not a custom texture file
		 !strstr ((char *) lh.name, ".lgt") &&     // not an texture brightness table file
		 !strstr ((char *) lh.name, ".clr") &&     // not an texture color table file
		 !strstr ((char *) lh.name, ".pal") &&     // not a palette file
		 strcmpi((char *) lh.name, szSubFile)) // not the same level
		bOtherFilesFound = 1;
	if (!strcmpi ((char *) lh.name, szSubFile)) // same level
		bIdenticalLevelFound = 1;
	fseek(file, lh.size, SEEK_CUR);
	}
fclose(file);

// if no other files found
// then simply do a quick save
int bQuickSave = 0;
if (!bOtherFilesFound)
	bQuickSave = 1;
else if (bIdenticalLevelFound) {
	if (QueryMsg ("Overwrite old level with same name?") != IDYES)
		return 1;
	}
else {
	// otherwise, if save file was not found,
	// then ask user if they want to append to the HOG file
	if (QueryMsg ("Would you like to add the level to the end\n"
					  "of this HOG file?\n\n"
					  "(Press OK to append this level to the HOG file\n"
					  "or Cancel to overwrite the entire HOG file") == IDYES) {
		if (!bExpertMode)
			ErrorMsg ("Don't forget to add this level's name to the mission file.\n");
		}
	else
		bQuickSave = 1;
	}
if (bQuickSave) {
	FSplit (szHogFile, szTmp, NULL, NULL);
	strcat (szTmp, "dle_temp.rdl");
	theApp.GetMine ()->Save (szTmp);
	return MakeHog (szTmp, szHogFile, szSubFile, bSaveAs);
//	MySetCaption (szHogFile);
	}
char base [256];
// determine base name
FSplit (szSubFile, NULL, base, NULL);
base[8] = NULL;
strlwr (base);
strip_extension (base);
file = fopen(szHogFile,"r+b"); // reopen file
if (!file) {
	ErrorMsg("Destination HOG file not found/accessible.");
	return 1;
	}
DeleteLevelSubFiles (file, base);
fclose (file);
// now append sub-files to the end of the HOG file
file = fopen(szHogFile,"ab");
if (!file) {
	ErrorMsg("Could not open destination HOG file for save.");
	return 1;
	}
fseek(file,0,SEEK_END);
FSplit (szHogFile, szTmp, NULL, NULL);
strcat (szTmp, "dle_temp.rdl");
theApp.GetMine ()->Save (szTmp, true);
WriteSubFile (file, szTmp, szSubFile);

if (mine->HasCustomLightMap ()) {
	FSplit (szHogFile, szTmp, NULL, NULL);
	strcat (szTmp, "dle_temp.lgt");
	fTmp = fopen (szTmp, "wb");
	bool bOk = false;
	if (fTmp) {
		if (!WriteLightMap (fTmp)) {
			fclose (fTmp);
			sprintf (subName, "%s.lgt", base);
			WriteSubFile (file, szTmp, subName);
			bOk = true;
			}
		else
			fclose (fTmp);
		unlink (szTmp);
		}
	if (!bOk)
		ErrorMsg ("Error writing custom light map.");
	}
if (mine->HasCustomLightColors ()) {
	FSplit (szHogFile, szTmp, NULL, NULL);
	strcat (szTmp, "dle_temp.clr");
	fTmp = fopen (szTmp, "wb");
	if (fTmp) {
		if (!mine->WriteColorMap (fTmp)) {
			fclose (fTmp);
			sprintf (subName, "%s.clr", base);
			WriteSubFile (file, szTmp, subName);
			}
		else
			fclose (fTmp);
		unlink (szTmp);
		}
	}

if (HasCustomTextures ()) {
	FSplit (szHogFile, szTmp, NULL, NULL);
	strcat (szTmp, "dle_temp.hxm");
	fTmp = fopen (szTmp, "wb");
	bool bOk = false;
	if (fTmp) {
		if (!CreatePog (fTmp)) {
			sprintf (subName, "%s.pog", base);
			WriteSubFile (file, szTmp, subName);
			bOk = true;
			}
		fclose (fTmp);
		unlink (szTmp);
		}
	if (!bOk)
		ErrorMsg ("Error writing custom textures.");
	}

if (theApp.GetMine ()->HasCustomRobots ()) {
	FSplit (szHogFile, szTmp, NULL, NULL);
	strcat (szTmp, "dle_temp.hxm");
	fTmp = fopen (szTmp, "wb");
	bool bOk = false;
	if (fTmp) {
		if (!theApp.GetMine ()->WriteHxmFile (fTmp)) {
			sprintf (subName, "%s.hxm", base);
			WriteSubFile (file, szTmp, subName);
			bOk = true;
			}
		unlink (szTmp);
		}
	if (!bOk)
		ErrorMsg ("Error writing custom robots.");
	}
fclose(file);
return 0;
}

//==========================================================================
// write_mission_file()
//==========================================================================

static LPSTR szMissionName [] = {"name", "zname", "d2x-name", NULL};
static LPSTR szMissionInfo [] = {"editor", "build_time", "date", "revision", "author", "email", "web_site", "briefing", NULL};
static LPSTR szMissionType [] = {"type", NULL};
static LPSTR szMissionTypes [] = {"anarchy", "normal", NULL};
static LPSTR szMissionFlags [] = {"normal", "anarchy", "robo_anarchy", "coop", "capture_flag", "hoard", NULL};
static LPSTR szCustomFlags [] = {"custom_textures", "custom_robots", "custom_music", NULL};
static LPSTR szAuthorFlags [] = {"multi_author", "want_feedback", NULL};
static LPSTR szNumLevels [] = {"num_levels", NULL};
static LPSTR szNumSecrets [] = {"num_secrets", NULL};
static LPSTR szBool [] = {"no", "yes", NULL};

static LPSTR *szTags [] = {szMissionName, szMissionInfo, szMissionType, szMissionFlags, szCustomFlags, szAuthorFlags, szNumLevels, szNumSecrets};

                         /*--------------------------*/

int atob (LPSTR psz)
{
strlwr (psz);
for (int i = 0; i < 2; i++)
	if (!strcmp (psz, szBool [i]))
		return i;
return 0;
}

                         /*--------------------------*/

int ReadMissionFile (char *pszFile) 
{
	FILE	*fMsn;
	char  szMsn [256];
	LPSTR	psz, *ppsz;
	char	szTag [20], szValue [80], szBuf [100];
	int	i, j, l;

strcpy (szMsn, pszFile);
char *pExt = strrchr (szMsn, '.');
if (pExt)
	*pExt = '\0';
strcat (szMsn, (file_type == RDL_FILE) ? ".msn" : ".mn2");
if (!(fMsn = fopen (szMsn, "rt"))) {
	DEBUGMSG (" Hog manager: Mission file not found.");
	return -1;
	}
memset (missionData.comment, 0, sizeof (missionData.comment));
l = 0;
while (fgets (szBuf, sizeof (szBuf), fMsn)) {
	if ((psz = strstr (szBuf, "\r\n")) || (psz = strchr (szBuf, '\n')))	//replace cr/lf
		*psz = '\0';
	if (*szBuf == ';') {	//comment
		if (l) {
			strncpy (missionData.comment + l, "\r\n", sizeof (missionData.comment) - l);
			l += 2;
			}
		strncpy (missionData.comment + l, szBuf + 1, sizeof (missionData.comment) - l);
		l += strlen (szBuf + 1);
		continue;
		}
	else if (!(psz = strchr (szBuf, '=')))	// otherwise need <tag> '=' <value> format
		continue;
	for (i = -1; psz + i > szBuf; i--)	// remove blanks around '='
		if (psz [i] != ' ') {
			psz [++i] = '\0';
			strncpy (szTag, szBuf, sizeof (szTag));
			szTag [sizeof (szTag) - 1] = '\0';
			break;
			}
	for (i = 1; psz [i]; i++)
		if (psz [i] != ' ') {
			strncpy (szValue, psz + i, sizeof (szValue));
			szValue [sizeof (szValue) - 1] = '\0';
			break;
			}
	if (!(*szTag && *szValue))	// mustn't be empty
		continue;
	strlwr (szTag);	// find valid tag
	for (i = 0, j = -1; i < 8; i++)
		for (ppsz = szTags [i]; *ppsz; ppsz++)
			if (!strcmp (*ppsz, szTag)) {
				j = ppsz - szTags [i];
				goto tagFound;
				}
	continue;
tagFound:
	switch (i) {
		case 0:
			strcpy (missionData.missionName, szValue);
			break;
		case 1:
			strcpy (missionData.missionInfo [j], szValue);
			break;
		case 2:
			strlwr (szValue);
			for (j = 0; j < 2; j++)
				if (!strcmp (szValue, szMissionTypes [j]))
					missionData.missionType = j;
			break;
		case 3:
			missionData.missionFlags [j] = atob (szValue);
			break;
		case 4:
			missionData.customFlags [j] = atob (szValue);
			break;
		case 5:
			missionData.authorFlags [j] = atob (szValue);
			break;
		case 6:
			missionData.numLevels = atol (szValue);
			for (i = 0; i < missionData.numLevels; i++) {
				fgets (missionData.levelList [i], sizeof (missionData.levelList [i]), fMsn);
				for (j = strlen (missionData.levelList [i]); --j; )
					if ((missionData.levelList [i][j] != '\r') &&
						 (missionData.levelList [i][j] != '\n'))
						break;
				missionData.levelList [i][j+1] = '\0';
				strlwr (missionData.levelList [i]);
				}
			break;
		case 7:
			missionData.numSecrets = atol (szValue);
			for (i = 0, j = missionData.numLevels; i < missionData.numSecrets; i++, j++) {
				fscanf (fMsn, "%s", missionData.levelList [j]);
				strlwr (missionData.levelList [i]);
				}
			break;
		default:
			continue;
		}
	}
fclose (fMsn);
return 0;
}

                         /*--------------------------*/

int WriteMissionFile (char *pszFile, bool bSaveAs) 
{
	FILE	*fMsn;
	char  szMsn [256];
	int	i, j;

strcpy (szMsn, pszFile);
char *pExt = strrchr (szMsn, '.');
if (pExt)
	*pExt = '\0';
strcat (szMsn, (file_type == RDL_FILE) ? ".msn" : ".mn2");
if (bSaveAs && (fMsn = fopen (szMsn, "rt"))) {
	if (AfxMessageBox ("A mission file with that name already exists.\nOverwrite mission file?", MB_YESNO) != IDYES)
		return -1;
	fclose (fMsn);
	}
// create mission file
if (!(fMsn = fopen (szMsn, "wt")))
	return -1;
if (level_version >= 9)
	fprintf (fMsn, "d2x-name = %s\n", missionData.missionName);
else if (level_version >= 8)
	fprintf (fMsn, "zname = %s\n", missionData.missionName);
else
	fprintf (fMsn, "name = %s\n", missionData.missionName);
fprintf (fMsn, "type = %s\n", szMissionTypes [missionData.missionType]);
fprintf (fMsn, "num_levels = %d\n", missionData.numLevels);
for (i = 0; i < missionData.numLevels; i++)
	fprintf (fMsn, "%s\n", missionData.levelList [i]);
if (missionData.numSecrets) {
	fprintf (fMsn, "num_secrets = %d\n", missionData.numSecrets);
	for (j = 0; j < missionData.numSecrets; i++, j++)
		fprintf (fMsn, "%s\n", missionData.levelList [i]);
	}
for (i = 0; i < 8; i++)
	if (*missionData.missionInfo [i])
		fprintf (fMsn, "%s = %s\n", szMissionInfo [i], missionData.missionInfo [i]);
for (i = 0; i < 3; i++)
	fprintf (fMsn, "%s = %s\n", szCustomFlags [i], szBool [missionData.customFlags [i]]);
for (i = 0; i < 6; i++)
	fprintf (fMsn, "%s = %s\n", szMissionFlags [i], szBool [missionData.missionFlags [i]]);
for (i = 0; i < 2; i++)
	fprintf (fMsn, "%s = %s\n", szAuthorFlags [i], szBool [missionData.authorFlags [i]]);
if (*missionData.comment) {
	char *pi, *pj;
	for (pi = pj = missionData.comment; ; pj++)
		if (!*pj) {
			fprintf (fMsn, ";%s\n", pi);
			break;
			}
		else if ((pj [0] == '\r') && (pj [1] == '\n')) {
			*pj = '\0';
			fprintf (fMsn, ";%s\n", pi);
			*pj++ = '\r';
			pi = pj + 1;
			}
	}
fclose (fMsn);
return 0;
}

//==========================================================================
// make_mission_file()
//==========================================================================

int MakeMissionFile (char *pszFile, char *pszSubFile, int bCustomTextures, int bCustomRobots, bool bSaveAs) 
{
	char	szBaseName [13];
	char	szTime [20];

//memset (&missionData, 0, sizeof (missionData));
FSplit (pszSubFile, NULL, szBaseName, NULL);
if (!*missionData.missionName)
	strcpy (missionData.missionName, szBaseName);
if (bSaveAs || !*missionData.missionName)
	do {
		CInputDialog dlg (theApp.MainFrame (), "Mission title", "Enter mission title:", missionData.missionName, sizeof (missionData.missionName));
		if (dlg.DoModal () != IDOK)
			return -1;
	} while (!*missionData.missionName);
missionData.missionType = 1;
missionData.numLevels = 1;
strcpy (missionData.levelList [0], pszSubFile);
if (!strchr (pszSubFile, '.'))
	strcat (missionData.levelList [0], file_type ? ".rl2" : ".rdl");
missionData.numSecrets = 0;
strcpy (missionData.missionInfo [0], "DLE-XP");
strcpy (missionData.missionInfo [2], DateStr (szTime, true));
if (bSaveAs)
	strcpy (missionData.missionInfo [3], "1.0");
missionData.customFlags [0] = bCustomTextures;
missionData.customFlags [1] = bCustomRobots;
return WriteMissionFile (pszFile, bSaveAs);
}

// eof file.cpp