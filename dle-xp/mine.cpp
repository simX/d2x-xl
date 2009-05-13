// Copyright (c) 1998 Bryan Aamot, Brainware

#include "stdafx.h"
#include "dlcres.h"

#include < math.h>
#include "define.h"
#include "types.h"
#include "global.h"
#include "mine.h"
#include "matrix.h"
#include "io.h"
#include "textures.h"
#include "palette.h"
#include "dlc.h"
#include "robot.h"
#include "file.h"
#include "light.h"

#ifdef ALLOCATE_POLYMODELS
#undef ALLOCATE_POLYMODELS
#endif
#define ALLOCATE_POLYMODELS 0

#define ENABLE_TEXT_DUMP 0

//==========================================================================
// CMine - CMine
//==========================================================================

CMine::CMine()
{
	VertCount () = 0;
	SegCount () = 0;
	dlcLogPalette = 0;
	thePalette = NULL;
	FlickerLightCount () = 0;
	Current () = &Current1 ();
	*m_szBlockFile = '\0';
	GameInfo ().objects.count = 0;
	GameInfo ().walls.count = 0;
	GameInfo ().doors.count = 0;
	GameInfo ().triggers.count = 0;
	GameInfo ().control.count = 0;
	GameInfo ().botgen.count = 0;
	GameInfo ().equipgen.count = 0;
	GameInfo ().dl_indices.count = 0;
	GameInfo ().delta_lights.count = 0;
	m_nNoLightDeltas = 2;
	m_lightRenderDepth = MAX_LIGHT_DEPTH;
	m_deltaLightRenderDepth = MAX_LIGHT_DEPTH;
	memset (RobotInfo (), 0, sizeof (ROBOT_INFO));
//	LoadPalette ();
	m_bSortObjects = TRUE;
	m_bVertigo = false;
	m_pHxmExtraData = NULL;
	m_nHxmExtraDataSize = 0;
	m_bUseTexColors = false;
	LoadDefaultLightAndColor ();
	
//	strcpy (descent2_path, "d:\\games\\descent\\d2\\");
}

//==========================================================================
// CMine - ~CMine
//==========================================================================

CMine::~CMine()
{
Default ();
}

//==========================================================================
// CMine - init_data()
//==========================================================================

void CMine::Reset ()
{
/*
  INT16 i;
	FIX maxx, maxy, maxz, minx, miny, minz;

	// set max and min points
	maxx = minx = maxy = miny = maxz = minz = 0;
	for (i = 0; i < VertCount (); i++) {
		maxx = max(maxx, vertices [i].x);
		minx = min(minx, vertices [i].x);
		maxy = max(maxy, vertices [i].y);
		miny = min(miny, vertices [i].y);
		maxz = max(maxz, vertices [i].z);
		minz = min(minz, vertices [i].z);
	}
	max_x = (UINT16)(maxx / F1_0);
	max_y = (UINT16)(maxy / F1_0);
	max_z = (UINT16)(maxz / F1_0);
	min_x = (UINT16)(minx / F1_0);
	min_y = (UINT16)(miny / F1_0);
	min_z = (UINT16)(minz / F1_0);
#if 0
	sprintf(message, "max(%d, %d, %d), min(%d, %d, %d)", max_x, max_y, max_z, min_x, min_y, min_z);
	DEBUGMSG(message);
#endif
	gx0 = 0;
	gy0 = 0;
	gz0 = 0;
	spinx = M_PI/4.f;
	spiny = M_PI/4.f;
	spinz = 0.0;
	movex = - (max_x + min_x)/2.f;
	movey = - (max_y + min_y)/2.f;
	movez = - (max_z + min_z)/2.f;
	int factor;
	int max_all;
	max_all = max(max(max_x - min_x, max_y - min_y), max_z - min_z)/20;
	if (max_all < 2)      factor = 14;
	else if (max_all < 4) factor = 10;
	else if (max_all < 8) factor = 8;
	else if (max_all < 12) factor = 5;
	else if (max_all < 16) factor = 3;
	else if (max_all < 32) factor = 2;
	else factor = 1;
	sizex = .1f * (double)pow(1.2, factor);
	sizey = sizex;
	sizez = sizex;
*/
	Current1 ().segment = DEFAULT_SEGMENT;
	Current1 ().point = DEFAULT_POINT;
	Current1 ().line = DEFAULT_LINE;
	Current1 ().side = DEFAULT_SIDE;
	Current1 ().object = DEFAULT_OBJECT;
	Current2 ().segment = DEFAULT_SEGMENT;
	Current2 ().point = DEFAULT_POINT;
	Current2 ().line = DEFAULT_LINE;
	Current2 ().side = DEFAULT_SIDE;
	Current2 ().object = DEFAULT_OBJECT;
	theApp.ResetUndoBuffer ();
}

void CMine::ConvertWallNum (UINT16 wNumOld, UINT16 wNumNew)
{
CDSegment *segP = Segments ();
CDSide *sideP;
int i, j;

for (i = SegCount (); i; i--, segP++)
	for (j = 0, sideP = segP->sides; j < 6; j++, sideP++)
		if (sideP->nWall >= wNumOld)
			sideP->nWall = wNumNew;
}


//==========================================================================
// CMine - load()
//
// ACTION -  loads a level (.RDL) file from disk
//
//==========================================================================

INT16 CMine::Load (const char *filename_passed, bool bLoadFromHog)
{
char filename [128];
INT16 check_err;
bool bNewMine = false;

// first disable curve generator
m_bSplineActive = FALSE;

memset (LightColors (), 0, sizeof (MineData ().lightColors));
memset (VertexColors (), 0, sizeof (MineData ().sideColors));
// if no file passed, define a new level w/ 1 object
FreeCustomPalette ();
if (filename_passed && *filename_passed)
	strcpy(filename, filename_passed);
else if (!CreateNewLevel ()) {
	CreateLightMap ();
	FSplit ((file_type== RDL_FILE) ? descent_path : levels_path, starting_directory, NULL, NULL);
	sprintf(filename, (file_type== RDL_FILE) ? "%sNEW.RDL" : "%sNEW.RL2", starting_directory);
	bLoadFromHog = false;
	bNewMine = true;
	}

disable_drawing = TRUE;
if (!bLoadFromHog)
	FreeTextureHandles ();
LoadMine (filename, bLoadFromHog, bNewMine);
if (!bNewMine && (level_version >= 9) && (level_version < LEVEL_VERSION)) {
	if (level_version < 15) {
		ConvertWallNum (MAX_WALLS2 + 1, MAX_WALLS3 + 1);
		NumObjTriggers () = 0;
		memset (ObjTriggerList (), 0xff, MAX_OBJ_TRIGGERS * sizeof (CDObjTriggerList));
		memset (ObjTriggerRoot (), 0xff, MAX_OBJECTS2 * sizeof (short));
		}
	level_version = LEVEL_VERSION;
	}
//CalcDeltaLightData ();
check_err = FixIndexValues();
if (check_err != 0) {
	sprintf(message, "File contains corrupted data. Would you like to load anyway? Error Code %#04x", check_err);
	if (QueryMsg(message) != IDYES) {
		if (!CreateNewLevel ()) {
			FSplit ((file_type== RDL_FILE) ? descent_path : levels_path, starting_directory, NULL, NULL);
			sprintf(filename, (file_type== RDL_FILE) ? "%sNEW.RDL" : "%sNEW.RL2", starting_directory);
			bLoadFromHog = false;
			bNewMine = true;
			}
		disable_drawing = TRUE;
		FreeTextureHandles ();
		LoadMine(filename, bLoadFromHog, bNewMine);
		disable_drawing = FALSE;
		return 1;
		}
	}
disable_drawing = FALSE;
return 0;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

INT16 CMine::LoadPalette (void)
{
HINSTANCE hInst = AfxGetInstanceHandle();
// set palette
// make global palette
UINT8 *palette = PalettePtr ();
ASSERT(palette);
if (!palette)
	return 1;
// redefine logical palette entries if memory for it is allocated
dlcLogPalette = (LPLOGPALETTE) malloc (sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 256);
if (!dlcLogPalette) {
	FreePaletteResource ();
	return 1;
	}
dlcLogPalette->palVersion = 0x300;
dlcLogPalette->palNumEntries = 256;
for (int i = 0; i < 256; ++i) {
	dlcLogPalette->palPalEntry [i].peRed = palette [i*3 + 0] << 2;
	dlcLogPalette->palPalEntry [i].peGreen = palette [i*3 + 1] << 2;
	dlcLogPalette->palPalEntry [i].peBlue = palette [i*3 + 2] << 2;
	dlcLogPalette->palPalEntry [i].peFlags = PC_RESERVED;
}
// now recreate the global Palette
if (thePalette)
	delete thePalette;
thePalette = new CPalette ();
thePalette->CreatePalette (dlcLogPalette);
FreePaletteResource ();
return 0;
}

// ------------------------------------------------------------------------

INT16 CMine::LoadMineSigAndType (FILE* fp)
{
INT32 sig = read_INT32 (fp);
if (sig != 'P'*0x1000000L + 'L'*0x10000L + 'V'*0x100 + 'L') {
	ErrorMsg ("Signature value incorrect.");
	fclose (fp);
	return 1;
	}

// read version
level_version = read_INT32 (fp);
if (level_version == 1) {
	file_type = RDL_FILE;
	texture_resource = D1_TEXTURE_STRING_TABLE;
	}
else if ((level_version >= 6L) && (level_version <= 18L)) {
	file_type = RL2_FILE;
	texture_resource = D2_TEXTURE_STRING_TABLE;
	}
else {
	sprintf(message, "Version %d unknown. Cannot load this level.", level_version);
	ErrorMsg (message);
	fclose (fp);
	return 1;
	}
return 0;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

INT16 CMine::LoadMine (char *filename, bool bLoadFromHog, bool bNewMine)
{
	HINSTANCE hInst = AfxGetInstanceHandle();
	UINT8* palette = 0;

	FILE* loadFile = 0;
	INT32 sig = 0;
	INT32 minedata_offset = 0;
	INT32 gamedata_offset = 0;
	INT32 mine_err, game_err = 0;
	int return_code = 0;
	char palette_name [16];
	INT16 nLights = 0;

changes_made = 0;
loadFile = fopen (filename, "rb");

if (!loadFile) {
	sprintf(message, "Error %d: Can't open file <%s>.", GetLastError (), filename);
	ErrorMsg(message);
	return -1;
	}
	//  strcpy(gamesave_current_filename, filename);
if (LoadMineSigAndType (loadFile))
	return -1;
// read mine data offset
minedata_offset = read_INT32 (loadFile);
// read game data offset
gamedata_offset = read_INT32 (loadFile);

// don't bother reading  hostagetext_offset since
// for Descent 1 files since we dont use it anyway
// hostagetext_offset = read_INT32(loadFile);

if (file_type != RDL_FILE) {
	if (level_version >= 8) {
		read_INT16(loadFile);
		read_INT16(loadFile);
		read_INT16(loadFile);
		read_INT8(loadFile);
		}
	}

// read palette name *.256
	if (file_type != RDL_FILE) {
		// read palette file name
		for (int i = 0; i < 15; i++) {
			palette_name [i] = fgetc(loadFile);
			if (palette_name [i]== 0x0a) {
				palette_name [i] = NULL;
				break;
				}
			}
		// replace extension with .pig
		if (i >= 4) {
			palette_name [strlen((char *)palette_name) - 4] = NULL;
			strcat(palette_name, ".PIG");
			}
		// try to find new pig file in same directory as Current () pig file
		// 1) cut off old name
	if (!bNewMine) {
		if (descent2_path [0] != NULL) {
			char *path = strrchr(descent2_path, '\\');
			if (!path) {
				descent2_path [0] = NULL;
				} 
			else {
				path++;  // leave slash
				*path = NULL;
				}
			// paste on new *.pig name
			strcat(descent2_path, palette_name);
			strlwr(descent2_path);
			}
		}
	}
#if 1
if (return_code = LoadPalette ())
	goto load_end;
#else
// set palette
palette = PalettePtr ();
ASSERT(palette);
if (!palette) 
	goto load_end;

// redefine logical palette entries if memory for it is allocated
dlcLogPalette = (LPLOGPALETTE) malloc (sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 256);
if (dlcLogPalette) {
	dlcLogPalette->palVersion = 0x300;
	dlcLogPalette->palNumEntries = 256;
#if 0
	for (int i = 0; i < 256; ++i) {
		dlcLogPalette->palPalEntry [i].peRed = *palette++;
		dlcLogPalette->palPalEntry [i].peGreen = *palette++;
		dlcLogPalette->palPalEntry [i].peBlue = *palette++;
		dlcLogPalette->palPalEntry [i].peFlags = PC_RESERVED; palette++;
		}
#else
	for (int i = 0; i < 256;++i) {
		dlcLogPalette->palPalEntry [i].peRed = palette [i*3 + 0] << 2;
		dlcLogPalette->palPalEntry [i].peGreen = palette [i*3 + 1] << 2;
		dlcLogPalette->palPalEntry [i].peBlue = palette [i*3 + 2] << 2;
		dlcLogPalette->palPalEntry [i].peFlags = PC_RESERVED;
		}
#endif
	// now recreate the global Palette
	if (thePalette) {
		delete thePalette;
		thePalette = new CPalette ();
		thePalette->CreatePalette (dlcLogPalette);
//			::DeleteObject(ThePalette);
//			ThePalette = CreatePalette(dlcLogPalette);
		}
	}
FreeResource(hGlobal);
#endif //0

// read descent 2 reactor information
nLights = 0;
if (file_type != RDL_FILE) {
	ReactorTime () = read_INT32(loadFile); // base control center explosion time
	ReactorStrength () = read_INT32(loadFile); // reactor strength

#if 0
	sprintf(message, "Reactor time=%ld, Reactor strength=%ld, Secret Cube #=%ld",
		ReactorTime (), ReactorStrength (), SecretCubeNum ());
	INFOMSG(message);
#endif

	if (level_version > 6) {
		nLights = (INT16)read_INT32(loadFile);
		if (nLights > 0 && FlickerLightCount () <= MAX_FLICKERING_LIGHTS) {
			fread(FlickeringLights (), sizeof(FLICKERING_LIGHT), nLights, loadFile);
			} 
		else {
			if (nLights != 0) {
				ErrorMsg("Error reading flickering lights");
				nLights = 0;
				}
			}
		}
	// NOTE: d2 v1.1 has two levels at version 7 (b2 and f4),
	//       both have 0 flickering lights
	//    sprintf(message, "%d flickering lights", nflicks);
	//    DEBUGMSG(message);

	// read secret cube number
	SecretCubeNum () = read_INT32(loadFile);
	// read secret cube orientation?
	read_matrix(&SecretOrient (), loadFile);
	}

disable_drawing = TRUE;

fseek(loadFile, minedata_offset, SEEK_SET);
mine_err = LoadMineDataCompiled(loadFile, bNewMine);
FlickerLightCount () = nLights;

if (mine_err != 0) {
	ErrorMsg("Error loading mine data");
	fclose(loadFile);
	return(2);
}

fseek(loadFile, gamedata_offset, SEEK_SET);
game_err = LoadGameData(loadFile, bNewMine);

if (game_err != 0) {
	ErrorMsg("Error loading game data");
	// reset "howmany"
	GameInfo ().objects.count = 0;
	GameInfo ().walls.count = 0;
	GameInfo ().doors.count = 0;
	GameInfo ().triggers.count = 0;
	GameInfo ().control.count = 0;
	GameInfo ().botgen.count = 0;
	GameInfo ().equipgen.count = 0;
	GameInfo ().dl_indices.count = 0;
	GameInfo ().delta_lights.count = 0;
	fclose(loadFile);
	return(3);
	}

goto load_pog;

load_end:
	return_code = 0;
load_pog:

	fclose(loadFile);
if (!bLoadFromHog && (file_type != RDL_FILE)) {
	strcpy (strstr (filename, "."), ".pog");
	loadFile = fopen(filename, "rb");
	if (loadFile) {
		ReadPog (loadFile);
		fclose (loadFile);
		}
	ReadHamFile ();
#if 0
	char szHamFile [256];
	FSplit (descent2_path, szHamFile, NULL, NULL);
	strcat (szHamFile, "hoard.ham");
	ReadHamFile (szHamFile, EXTENDED_HAM);
#endif
#if 1
	if (file_type == RL2_FILE) {
		char szHogFile [256], szHamFile [256], *p;
		long nSize, nPos;

		FSplit (descent2_path, szHogFile, NULL, NULL);
		if (p = strstr (szHogFile, "data"))
			*p = '\0';
		strcat (szHogFile, "missions\\d2x.hog");
		if (FindFileData (szHogFile, "d2x.ham", &nSize, &nPos, FALSE)) {
			FSplit (descent2_path, szHamFile, NULL, NULL);
			if (p = strstr (szHamFile, "data"))
				*p = '\0';
			strcat (szHamFile, "missions\\d2x.ham");
			if (ExportSubFile (szHogFile, szHamFile, nPos + sizeof (struct level_header), nSize)) {
				m_bVertigo = ReadHamFile (szHamFile, EXTENDED_HAM) == 0;
				unlink (szHamFile);
				}
			}
		}
#endif
	strcpy (strstr (filename, "."), ".hxm");
	loadFile = fopen(filename, "rb");
	if (loadFile) {
		ReadHxmFile (loadFile, -1);
		fclose (loadFile);
		}
	}
SortObjects ();
return return_code;
}

// ------------------------------------------------------------------------

UINT8 *CMine::LoadDataResource (LPCTSTR pszRes, HGLOBAL& hGlobal, UINT32& nResSize)
{
HINSTANCE hInst = AfxGetInstanceHandle ();
HRSRC hRes = FindResource (hInst, pszRes, "RC_DATA");
if (!hRes)
	return NULL;
if (!(hGlobal = LoadResource (hInst, hRes)))
	return NULL;
nResSize = SizeofResource (hInst, hRes);
return (UINT8 *) LockResource (hGlobal);
}

// ------------------------------------------------------------------------

BOOL CMine::HasCustomLightMap (void)
{
HGLOBAL hGlobal = 0;
UINT32 nSize = 0;
UINT8 *dataP = LoadDataResource (MAKEINTRESOURCE ((file_type == RDL_FILE) ? IDR_LIGHT_D1 : IDR_LIGHT_D2), hGlobal, nSize);
if (!dataP)
	return 0;
BOOL bCustom = memcmp (lightMap, dataP, sizeof (lightMap)) != 0;
FreeResource (hGlobal);
return bCustom;
}

// ------------------------------------------------------------------------

BOOL CMine::HasCustomLightColors (void)
{
HGLOBAL hGlobal = 0;
UINT32 nSize = 0;
UINT8 *dataP = LoadDataResource (MAKEINTRESOURCE ((file_type == RDL_FILE) ? IDR_COLOR_D1 : IDR_COLOR_D2), hGlobal, nSize);
if (!dataP)
	return 0;
BOOL bCustom = memcmp (MineData ().texColors, dataP, sizeof (MineData ().texColors)) != 0;
FreeResource (hGlobal);
return bCustom;
}

// ------------------------------------------------------------------------

INT16 CMine::LoadDefaultLightAndColor (void)
{
HGLOBAL hGlobal = 0;
UINT32 nSize = 0;
UINT8 *dataP = LoadDataResource (MAKEINTRESOURCE ((file_type == RDL_FILE) ? IDR_COLOR_D1 : IDR_COLOR_D2), hGlobal, nSize);
if (!dataP)
	return 0;
int i = nSize / (3 * sizeof (int) + sizeof (UINT8));
if (i > sizeof (MineData ().texColors) / sizeof (MineData ().texColors [0]))
	i = sizeof (MineData ().texColors) / sizeof (MineData ().texColors [0]);
for (CDColor *pc = MineData ().texColors; i; i--, pc++) {
	pc->index = *dataP++;
	pc->color.r = (double) *((int *) dataP) / (double) 0x7fffffff;
	dataP += sizeof (int);
	pc->color.g = (double) *((int *) dataP) / (double) 0x7fffffff;
	dataP += sizeof (int);
	pc->color.b = (double) *((int *) dataP) / (double) 0x7fffffff;
	dataP += sizeof (int);
	}
FreeResource (hGlobal);
dataP = LoadDataResource (MAKEINTRESOURCE ((file_type == RDL_FILE) ? IDR_LIGHT_D1 : IDR_LIGHT_D2), hGlobal, nSize);
if (!dataP)
	return 0;
memcpy (lightMap, dataP, min (nSize, sizeof (lightMap)));
FreeResource (hGlobal);
return 1;
}

// ------------------------------------------------------------------------
// Create()
//
// Action: makes a new level from the resource file
// ------------------------------------------------------------------------

INT16 CMine::CreateNewLevel ()
{
HGLOBAL hGlobal;
UINT32 nResSize;
UINT8 *data = LoadDataResource (MAKEINTRESOURCE ((file_type == RDL_FILE) ? IDR_NEW_RDL : IDR_NEW_RL2), hGlobal, nResSize);
if (!data)
	return 0;
// copy data to a file

FSplit ((file_type== RDL_FILE) ? descent_path : levels_path, starting_directory, NULL, NULL);
sprintf(message, (file_type== RDL_FILE) ? "%sNEW.RDL" : "%sNEW.RL2", starting_directory);
memcpy (RobotInfo (), DefRobotInfo (), sizeof(ROBOT_INFO) * N_robot_types);
texture_resource = (file_type == RDL_FILE) ? D1_TEXTURE_STRING_TABLE : D2_TEXTURE_STRING_TABLE;
FILE *file = fopen(message, "wb");
if (file) {
	size_t nBytes = fwrite(data, sizeof(UINT8), (UINT16)nResSize, file);
	fclose(file);
	FreeResource (hGlobal);
	if (nBytes != nResSize)
		return 1;
	NumObjTriggers () = 0;
	memset (ObjTriggerList (), 0xff, MAX_OBJ_TRIGGERS * sizeof (CDObjTriggerList));
	memset (ObjTriggerRoot (), 0xff, MAX_OBJECTS2 * sizeof (short));
	return 0;
	} 
else {
	FreeResource (hGlobal);
	return 2;
	}
}

// ------------------------------------------------------------------------
// FixIndexValues()
//
// Action - This routine attempts to fix array index values to prevent
//          the editor from crashing when the level is drawn.
//
// Returns - 0 if no error is detected
// ------------------------------------------------------------------------
INT16 CMine::FixIndexValues()
{
	INT16 	segnum, sidenum, vertnum;
	UINT16	wallnum;
	INT16 	check_err;

	check_err = 0;
	CDSegment *seg = Segments ();
	for(segnum = 0; segnum < SegCount (); segnum++, seg++) {
		for(sidenum = 0; sidenum < MAX_SIDES_PER_SEGMENT; sidenum++) {
			// check wall numbers
			CDSide& side = seg->sides [sidenum];
			if (side.nWall >= GameInfo ().walls.count &&
				side.nWall != NO_WALL) {
				side.nWall = NO_WALL;
				check_err |= (1 << 0);
			}
			// check children
			if (seg->children [sidenum] < - 2 || seg->children [sidenum] >(INT16)SegCount ()) {
				seg->children [sidenum] =-1;
				check_err |= (1 << 1);
			}
		}
		// check verts
		for(vertnum = 0; vertnum < MAX_VERTICES_PER_SEGMENT; vertnum++) {
			if (seg->verts [vertnum] < 0 || seg->verts [vertnum] >= VertCount ()) {
				seg->verts [vertnum] = 0;  // this will cause a bad looking picture
				check_err |= (1 << 2);      // but it will prevent a crash
			}
		}
	}
	CDWall *wall = Walls ();
	for (wallnum = 0; wallnum < GameInfo ().walls.count; wallnum++, wall++) {
		// check segnum
		if (wall->segnum < 0 || wall->segnum > SegCount ()) {
			wall->segnum = 0;
			check_err |= (1 << 3);
		}
		// check sidenum
		if (wall->sidenum < 0 || wall->sidenum > 5) {
			wall->sidenum = 0;
			check_err |= (1 << 4);
		}
	}
	return check_err;
}

// ------------------------------------------------------------------------
// CMine::Default()
// ------------------------------------------------------------------------
void CMine::Default()
{

	ClearMineData();

	if (m_pHxmExtraData) {
		free (m_pHxmExtraData);
		m_pHxmExtraData = NULL;
		m_nHxmExtraDataSize = 0;
		}

	CDSegment& seg = *Segments ();
	vms_vector *vert = Vertices ();

	seg.sides [0].nWall = NO_WALL;
	seg.sides [0].nBaseTex = 0;
	seg.sides [0].nOvlTex = 0;
	seg.sides [0].uvls [0].u = 0;
	seg.sides [0].uvls [0].v = 0;
	seg.sides [0].uvls [0].l = 0x8000U;
	seg.sides [0].uvls [1].u = 0;
	seg.sides [0].uvls [1].v = 2047;
	seg.sides [0].uvls [1].l = 511;
	seg.sides [0].uvls [2].u = - 2048;
	seg.sides [0].uvls [2].v = 2047;
	seg.sides [0].uvls [2].l = 3833;
	seg.sides [0].uvls [3].u = - 2048;
	seg.sides [0].uvls [3].v = 0;
	seg.sides [0].uvls [3].l = 0x8000U;

	seg.sides [1].nWall = NO_WALL;
	seg.sides [1].nBaseTex = 263;
	seg.sides [1].nOvlTex = 264;
	seg.sides [1].uvls [0].u = 0;
	seg.sides [1].uvls [0].v = 0;
	seg.sides [1].uvls [0].l = 0x8000U;
	seg.sides [1].uvls [1].u = 0;
	seg.sides [1].uvls [1].v = 2047;
	seg.sides [1].uvls [1].l = 0x8000U;
	seg.sides [1].uvls [2].u = - 2048;
	seg.sides [1].uvls [2].v = 2047;
	seg.sides [1].uvls [2].l = 0x8000U;
	seg.sides [1].uvls [3].u = - 2048;
	seg.sides [1].uvls [3].v = 0;
	seg.sides [1].uvls [3].l = 0x8000U;

	seg.sides [2].nWall = NO_WALL;
	seg.sides [2].nBaseTex = 0;
	seg.sides [2].nOvlTex = 0;
	seg.sides [2].uvls [0].u = 0;
	seg.sides [2].uvls [0].v = 0;
	seg.sides [2].uvls [0].l = 0x8000U;
	seg.sides [2].uvls [1].u = 0;
	seg.sides [2].uvls [1].v = 2047;
	seg.sides [2].uvls [1].l = 3836;
	seg.sides [2].uvls [2].u = - 2048;
	seg.sides [2].uvls [2].v = 2047;
	seg.sides [2].uvls [2].l = 5126;
	seg.sides [2].uvls [3].u = - 2048;
	seg.sides [2].uvls [3].v = 0;
	seg.sides [2].uvls [3].l = 0x8000U;

	seg.sides [3].nWall = NO_WALL;
	seg.sides [3].nBaseTex = 270;
	seg.sides [3].nOvlTex = 0;
	seg.sides [3].uvls [0].u = 0;
	seg.sides [3].uvls [0].v = 0;
	seg.sides [3].uvls [0].l = 11678;
	seg.sides [3].uvls [1].u = 0;
	seg.sides [3].uvls [1].v = 2047;
	seg.sides [3].uvls [1].l = 12001;
	seg.sides [3].uvls [2].u = - 2048;
	seg.sides [3].uvls [2].v = 2047;
	seg.sides [3].uvls [2].l = 12001;
	seg.sides [3].uvls [3].u = - 2048;
	seg.sides [3].uvls [3].v = 0;
	seg.sides [3].uvls [3].l = 11678;

	seg.sides [4].nWall = NO_WALL;
	seg.sides [4].nBaseTex = 0;
	seg.sides [4].nOvlTex = 0;
	seg.sides [4].uvls [0].u = 0;
	seg.sides [4].uvls [0].v = 0;
	seg.sides [4].uvls [0].l = 0x8000U;
	seg.sides [4].uvls [1].u = 0;
	seg.sides [4].uvls [1].v = 2043;
	seg.sides [4].uvls [1].l = 0x8000U;
	seg.sides [4].uvls [2].u = - 2044;
	seg.sides [4].uvls [2].v = 2045;
	seg.sides [4].uvls [2].l = 0x8000U;
	seg.sides [4].uvls [3].u = - 2043;
	seg.sides [4].uvls [3].v = 0;
	seg.sides [4].uvls [3].l = 0x8000U;

	seg.sides [5].nWall = NO_WALL;
	seg.sides [5].nBaseTex = 0;
	seg.sides [5].nOvlTex = 0;
	seg.sides [5].uvls [0].u = 0;
	seg.sides [5].uvls [0].v = 0;
	seg.sides [5].uvls [0].l = 24576;
	seg.sides [5].uvls [1].u = 0;
	seg.sides [5].uvls [1].v = 2048;
	seg.sides [5].uvls [1].l = 24576;
	seg.sides [5].uvls [2].u = - 2048;
	seg.sides [5].uvls [2].v = 2048;
	seg.sides [5].uvls [2].l = 24576;
	seg.sides [5].uvls [3].u = - 2048;
	seg.sides [5].uvls [3].v = 0;
	seg.sides [5].uvls [3].l = 24576;

	seg.children [0] =-1;
	seg.children [1] =-1;
	seg.children [2] =-1;
	seg.children [3] =-1;
	seg.children [4] =-1;
	seg.children [5] =-1;

	seg.verts [0] = 0;
	seg.verts [1] = 1;
	seg.verts [2] = 2;
	seg.verts [3] = 3;
	seg.verts [4] = 4;
	seg.verts [5] = 5;
	seg.verts [6] = 6;
	seg.verts [7] = 7;

	seg.special = 0;
	seg.matcen_num =-1;
	seg.value =-1;
	seg.s2_flags = 0;
	seg.static_light = 263152L;
	seg.child_bitmask = 0;
	seg.wall_bitmask = 0;
	seg.seg_number = 0;
	seg.map_bitmask = 0;

	vert [0].x = 10*F1_0;
	vert [0].y = 10*F1_0;
	vert [0].z = - 10*F1_0;
	vert [1].x = 10*F1_0;
	vert [1].y = - 10*F1_0;
	vert [1].z = - 10*F1_0;
	vert [2].x = - 10*F1_0;
	vert [2].y = - 10*F1_0;
	vert [2].z = - 10*F1_0;
	vert [3].x = - 10*F1_0;
	vert [3].y = 10*F1_0;
	vert [3].z = - 10*F1_0;
	vert [4].x = 10*F1_0;
	vert [4].y = 10*F1_0;
	vert [4].z = 10*F1_0;
	vert [5].x = 10*F1_0;
	vert [5].y = - 10*F1_0;
	vert [5].z = 10*F1_0;
	vert [6].x = - 10*F1_0;
	vert [6].y = - 10*F1_0;
	vert [6].z = 10*F1_0;
	vert [7].x = - 10*F1_0;
	vert [7].y = 10*F1_0;
	vert [7].z = 10*F1_0;

	SegCount () = 1;
	VertCount () = 8;
}

// ------------------------------------------------------------------------
// ClearMineData()
// ------------------------------------------------------------------------
void CMine::ClearMineData() {
	INT16 i;

	// initialize Segments ()
	CDSegment *seg = Segments ();
	for (i = 0; i < MAX_SEGMENTS; i++, seg++)
		seg->wall_bitmask &= ~MARKED_MASK;
	SegCount () = 0;

	// initialize vertices
	for (i = 0; i < MAX_VERTICES; i++) {
		*VertStatus (i) &= ~MARKED_MASK;
	}
	VertCount () = 0;

	FlickerLightCount () = 0;

	// reset "howmany"
	GameInfo ().objects.count = 0;
	GameInfo ().walls.count = 0;
	GameInfo ().doors.count = 0;
	GameInfo ().triggers.count = 0;
	GameInfo ().control.count = 0;
	GameInfo ().botgen.count = 0;
	GameInfo ().equipgen.count = 0;
	GameInfo ().dl_indices.count = 0;
	GameInfo ().delta_lights.count = 0;
}

// ------------------------------------------------------------------------

void CMine::ReadColor (CDColor *pc, FILE *loadFile)
{
	int	c;

fread (&pc->index, sizeof (pc->index), 1, loadFile);
fread (&c, sizeof (c), 1, loadFile);
pc->color.r = (double) c / (double) 0x7fffffff;
fread (&c, sizeof (c), 1, loadFile);
pc->color.g = (double) c / (double) 0x7fffffff;
fread (&c, sizeof (c), 1, loadFile);
pc->color.b = (double) c / (double) 0x7fffffff;
}

// ------------------------------------------------------------------------

void CMine::LoadColors (CDColor *pc, int nColors, int nFirstVersion, int nNewVersion, FILE *fp)
{
if (level_version > nFirstVersion)
	if (level_version < nNewVersion)
		fread (pc, sizeof (CDColor), nColors, fp);
	else
		for (; nColors; nColors--, pc++)
			ReadColor (pc, fp);
}

// ------------------------------------------------------------------------
// LoadMineDataCompiled()
//
// ACTION - Reads a mine data portion of RDL file.
// ------------------------------------------------------------------------
INT16 CMine::LoadMineDataCompiled(FILE *loadFile, bool bNewMine)
{
	INT16    i, segnum, sidenum; /** was INT32 */
	UINT8    version;
	UINT16   temp_UINT16;
	UINT8    bit_mask;
	UINT16   n_vertices;
	UINT16   n_segments;

	// read version (1 byte)
	fread(&version, sizeof(UINT8), 1, loadFile);
	//  if(version!= COMPILED_MINE_VERSION){
	//    sprintf(message, "Version incorrect (%d)\n", version);
	//    ErrorMsg(message);
	//  }

	// read number of vertices (2 bytes)
	fread(&temp_UINT16, sizeof(UINT16), 1, loadFile);
	n_vertices = temp_UINT16;
	if (n_vertices > MAX_VERTICES3) {
		sprintf(message, "Too many vertices (%d)", n_vertices);
		ErrorMsg (message);
		return(1);
		}
	if (((file_type == RDL_FILE) && (n_vertices > MAX_VERTICES1)) ||
		 ((file_type != RDL_FILE) && (level_version < 9) && (n_vertices > MAX_VERTICES2)))
		ErrorMsg ("Warning: Too many vertices for this level version");

	// read number of Segments () (2 bytes)
	fread(&temp_UINT16, sizeof(UINT16), 1, loadFile);
	n_segments = temp_UINT16;
	if (n_segments > MAX_SEGMENTS3) {
		sprintf (message, "Too many Segments (%d)", n_segments);
		ErrorMsg (message);
		return(2);
	}
	if (((file_type == RDL_FILE) && (n_segments > MAX_SEGMENTS1)) ||
		 ((file_type != RDL_FILE) && (level_version < 9) && (n_segments > MAX_SEGMENTS2)))
		ErrorMsg ("Warning: Too many Segments for this level version");

	// if we are happy with the number of verts and Segments (), then proceed...
	ClearMineData();
	VertCount () = n_vertices;
	SegCount () = n_segments;

	// read all vertices
	fread(Vertices (), sizeof(vms_vector), VertCount (), loadFile);
	if (n_vertices != VertCount ()) {
		fseek(loadFile, sizeof(vms_vector)*(n_vertices - VertCount ()), SEEK_CUR);
	}

	// unmark all vertices while we are here...
	for (i = 0; i < VertCount (); i++) {
		*VertStatus (i) &= ~MARKED_MASK;
	}

	// read segment information
	for (segnum = 0; segnum < SegCount (); segnum++)   {
		INT16   bit; /** was INT32 */
		CDSegment *seg = Segments (segnum);

		if (level_version >= 9) {
			fread(&seg->owner, sizeof(UINT8), 1, loadFile);
			fread(&seg->group, sizeof(INT8), 1, loadFile);
			}
		else {
			seg->owner = -1;
			seg->group = -1;
			}
		// read in child mask (1 byte)
		fread(&bit_mask, sizeof(UINT8), 1, loadFile);
		seg->child_bitmask = bit_mask;

		// read 0 to 6 children (0 to 12 bytes)
		for (bit = 0; bit < MAX_SIDES_PER_SEGMENT; bit++) {
			if (bit_mask & (1 << bit)) {
				fread(&seg->children [bit], sizeof(INT16), 1, loadFile);
			} else {
				seg->children [bit] =-1;
			}
		}

		// read vertex numbers (16 bytes)
		fread(seg->verts, sizeof(INT16), MAX_VERTICES_PER_SEGMENT, loadFile);

		if (file_type == RDL_FILE) {
			// read special info (0 to 4 bytes)
			if (bit_mask & (1 << MAX_SIDES_PER_SEGMENT)) {
				fread(&seg->special, sizeof(UINT8), 1, loadFile);
				fread(&seg->matcen_num, sizeof(INT8), 1, loadFile);
				fread(&seg->value, sizeof(INT8), 1, loadFile);
				fread(&seg->s2_flags, sizeof(UINT8), 1, loadFile);
			} else {
				seg->owner = -1;
				seg->group = -1;
				seg->special = 0;
				seg->matcen_num =-1;
				seg->value = 0;
			}
			seg->s2_flags = 0;  // d1 doesn't use this number, so zero it

			// read static light (2 bytes)
			fread(&temp_UINT16, sizeof(temp_UINT16), 1, loadFile);
			seg->static_light = ((FIX)temp_UINT16) << 4;
		}

		// read the wall bit mask
		fread(&bit_mask, sizeof(UINT8), 1, loadFile);

		// read in wall numbers (0 to 6 bytes)
		for (sidenum = 0; sidenum < MAX_SIDES_PER_SEGMENT; sidenum++) {
			if (bit_mask & (1 << sidenum)) {
				if (level_version < 13) {
					UINT8	nWall;
					fread(&nWall, sizeof(UINT8), 1, loadFile);
					seg->sides [sidenum].nWall = nWall;
					}
				else {
					UINT16	nWall;
					fread(&nWall, sizeof(UINT16), 1, loadFile);
					seg->sides [sidenum].nWall = nWall;
					}
				} 
			else {
				seg->sides [sidenum].nWall = NO_WALL;
			}
		}

		// read in textures and uvls (0 to 60 bytes)
		for (sidenum = 0; sidenum < MAX_SIDES_PER_SEGMENT; sidenum++)   {
			if ((seg->children [sidenum]==-1) || (bit_mask & (1 << sidenum))) {
				//  read in texture 1 number
				fread(&temp_UINT16, sizeof(UINT16), 1, loadFile);
				seg->sides [sidenum].nBaseTex = temp_UINT16 & 0x7fff;
				//   read in texture 2 number
				if (!(temp_UINT16 & 0x8000)) {
					seg->sides [sidenum].nOvlTex = 0;
				} else {
					fread(&temp_UINT16, sizeof(UINT16), 1, loadFile);
					seg->sides [sidenum].nOvlTex = temp_UINT16;
					temp_UINT16 &= 0x1fff;
					if ((temp_UINT16 == 0) ||(temp_UINT16 >= MAX_TEXTURES))
						seg->sides [sidenum].nOvlTex = 0;
				}

				//   read uvl numbers
				for (i = 0; i < 4; i++)   {
					fread(&seg->sides [sidenum].uvls [i].u, sizeof(INT16), 1, loadFile);
					fread(&seg->sides [sidenum].uvls [i].v, sizeof(INT16), 1, loadFile);
					fread(&seg->sides [sidenum].uvls [i].l, sizeof(INT16), 1, loadFile);
				}
			} else {
				seg->sides [sidenum].nBaseTex = 0;
				seg->sides [sidenum].nOvlTex = 0;
				for (i = 0; i < 4; i++)   {
					seg->sides [sidenum].uvls [i].u = 0;
					seg->sides [sidenum].uvls [i].v = 0;
					seg->sides [sidenum].uvls [i].l = 0;
				}
			}
		}
	}

	if (file_type != RDL_FILE) {
		CDSegment *seg = Segments ();
		for (segnum = 0; segnum < SegCount (); segnum++, seg++) {
			// read special info (8 bytes)
			fread(&seg->special, sizeof(UINT8), 1, loadFile);
			fread(&seg->matcen_num, sizeof(INT8), 1, loadFile);
			fread(&seg->value, sizeof(INT8), 1, loadFile);
			fread(&seg->s2_flags, sizeof(UINT8), 1, loadFile);
			fread(&seg->static_light, sizeof(FIX), 1, loadFile);
			if ((seg->special == SEGMENT_IS_ROBOTMAKER) && (seg->matcen_num == -1)) {
				seg->special = SEGMENT_IS_NOTHING;
				seg->value = 0;
				seg->child_bitmask &= ~(1 << MAX_SIDES_PER_SEGMENT);
				}
			}
		}
	if (level_version == 9) {
		fread(LightColors (), sizeof(CDColor), SegCount () * 6, loadFile); //skip obsolete side colors 
		fread(LightColors (), sizeof(CDColor), SegCount () * 6, loadFile);
		fread(VertexColors (), sizeof(CDColor), VertCount (), loadFile);
		}
	else if (level_version > 9) {
		LoadColors (VertexColors (), VertCount (), 9, 15, loadFile);
		LoadColors (LightColors (), SegCount () * 6, 9, 14, loadFile);
		LoadColors (TexColors (), MAX_D2_TEXTURES, 10, 16, loadFile);
		}
if (GameInfo ().objects.count > MAX_OBJECTS) {
	sprintf(message, "Warning: Max number of objects for this level version exceeded (%ld/%d)", 
			  GameInfo ().objects.count, MAX_OBJECTS2);
	ErrorMsg(message);
	}
return 0;
}

// ------------------------------------------------------------------------
// LoadGameData()
//
// ACTION - Loads the player, object, wall, door, trigger, and
//          materialogrifizationator data from an RDL file.
// ------------------------------------------------------------------------

INT16 CMine::LoadGameData(FILE *loadfile, bool bNewMine) 
{
	INT32 i;
	INT32 start_offset;
	short	nObjsWithTrigger, nObject, nFirstTrigger;
	//  INT32 names.offset;

	start_offset = ftell(loadfile);

	// Set default values
	GameInfo ().objects.count = 0;
	GameInfo ().walls.count = 0;
	GameInfo ().doors.count = 0;
	GameInfo ().triggers.count = 0;
	GameInfo ().control.count = 0;
	GameInfo ().botgen.count = 0;
	GameInfo ().equipgen.count = 0;
	GameInfo ().dl_indices.count = 0;
	GameInfo ().delta_lights.count = 0;

	GameInfo ().objects.offset =-1;
	GameInfo ().walls.offset =-1;
	GameInfo ().doors.offset =-1;
	GameInfo ().triggers.offset =-1;
	GameInfo ().control.offset =-1;
	GameInfo ().botgen.offset =-1;
	GameInfo ().equipgen.offset =-1;
	GameInfo ().dl_indices.offset =-1;
	GameInfo ().delta_lights.offset =-1;

	//==================== = READ FILE INFO========================

	// Read in game_top_fileinfo to get size of saved fileinfo.
	if (fseek(loadfile, start_offset, SEEK_SET)) {
		ErrorMsg("Error seeking in mine.cpp");
		return -1;
	}
	if (fread(&game_top_fileinfo, sizeof(game_top_fileinfo), 1, loadfile) != 1) {
		ErrorMsg("Error reading game info in mine.cpp");
		return -1;
	}
	// Check signature
	if (game_top_fileinfo.fileinfo_signature != 0x6705) {
		ErrorMsg("Game data signature incorrect");
		return -1;
	}
	// Check version number
	//  if (game_top_fileinfo.fileinfo_version < GAME_COMPATIBLE_VERSION)
	//    return -1;

	// Now, Read in the fileinfo
	if (fseek(loadfile, start_offset, SEEK_SET)) {
		ErrorMsg("Error seeking to game info in mine.cpp");
		return -1;
	}
	if (fread(&GameInfo (), (INT16)game_top_fileinfo.fileinfo_size, 1, loadfile)!= 1) {
		ErrorMsg("Error reading game info from mine.cpp");
		return -1;
	}
	if (GameInfo ().fileinfo_version >= 14) {  /*load mine filename */
		char *p;
		for (p = current_level_name; ; p++) {
			*p = fgetc(loadfile);
			if (*p== '\n') *p = 0;
			if (*p== 0) break;
		}
	} else {
		current_level_name [0] = 0;
	}

#if 0
	if (GameInfo ().fileinfo_version >= 19) {  /*load pof names */
		names.offset = ftell(loadfile);
		fread(&n_save_pof_names, 2, 1, loadfile);
		n_total_names = (GameInfo ().player.offset - names.offset)/13;
		if (n_total_names > 100) {
			sprintf(message, "Exceeded maximum number of total pof names (100 max)");
			ErrorMsg(message);
			n_total_names = 100;
			}
		fread(save_pof_names, n_total_names, 13, loadfile);
	}
#endif

	//==================== = READ PLAYER INFO==========================
	//  object_next_signature = 0;
	// 116 bytes of data "GUILE", "BIG_RED", or "RACER_X" + NULL + junk
	// use this area to store version of DMB

	//==================== = READ OBJECT INFO==========================
	// note: same for D1 and D2
	//  gamesave_num_org_robots = 0;
	//  gamesave_num_players = 0;

	if (GameInfo ().objects.offset > -1) {
		if (fseek(loadfile, GameInfo ().objects.offset, SEEK_SET)) {
			ErrorMsg("Error seeking to objects.");
		}
		else if (GameInfo ().objects.count > MAX_OBJECTS2) {
			sprintf(message, "Error: Max number of objects (%ld/%d) exceeded", 
					  GameInfo ().objects.count, MAX_OBJECTS2);
			ErrorMsg(message);
			GameInfo ().objects.count = MAX_OBJECTS;
			}
		else {
			CDObject *obj = Objects ();
			for (i = 0; i < GameInfo ().objects.count; i++, obj++) {
				ReadObject(obj, loadfile, GameInfo ().fileinfo_version);
				//      obj->signature = object_next_signature++;
				//    verify_object(obj);
			}
		}
	}

	//==================== = READ WALL INFO============================
	// note: Wall size will automatically strip last two items
	if (GameInfo ().walls.offset > -1)
		if (!fseek(loadfile, GameInfo ().walls.offset, SEEK_SET))
			if (GameInfo ().walls.count > MAX_WALLS) {
				sprintf(message, "Error: Max number of walls (%d/%d) exceeded",
					GameInfo ().walls.count, MAX_WALLS);
				ErrorMsg(message);
				GameInfo ().walls.count = MAX_WALLS;
				}
			else if (GameInfo ().fileinfo_version < 20)
				ErrorMsg("Wall version < 20, walls not loaded");
			else if (GameInfo ().walls.count &&
						fread(Walls (), (INT16)GameInfo ().walls.size * GameInfo ().walls.count, 1, loadfile)!= 1) {
				ErrorMsg("Error reading walls from mine.cpp");

	//==================== = READ DOOR INFO============================
	// note: not used for D1 or D2 since doors.count is always 0
	if (GameInfo ().doors.offset > -1) {
		if (!fseek(loadfile, GameInfo ().doors.offset, SEEK_SET))  {
			if (GameInfo ().doors.count > MAX_DOORS) {
				sprintf(message, "Error: Max number of doors (%ld/%d) exceeded", GameInfo ().doors.count, MAX_DOORS);
				ErrorMsg(message);
				GameInfo ().doors.count = MAX_DOORS;
			}
			else if (GameInfo ().fileinfo_version < 20)
				ErrorMsg("Door version < 20, doors not loaded");
			else if(sizeof(*ActiveDoors (i)) != GameInfo ().doors.size)
				ErrorMsg("Error: Door size incorrect");
			else if (GameInfo ().doors.count && 
				      fread(ActiveDoors (), (INT16)GameInfo ().doors.size * GameInfo ().doors.count, 1, loadfile)!= 1) {
				ErrorMsg("Error reading doors.");
				}
			}
		}
	}

	//==================== READ TRIGGER INFO==========================
	// note: order different for D2 levels but size is the same
	if (GameInfo ().triggers.offset > -1) {
		if (GameInfo ().triggers.count > MAX_TRIGGERS) {
			sprintf(message, "Error: Max number of triggers (%ld/%d) exceeded",
				GameInfo ().triggers.count, MAX_TRIGGERS);
			ErrorMsg(message);
			GameInfo ().triggers.count = MAX_TRIGGERS;
		}
		if (!fseek(loadfile, GameInfo ().triggers.offset, SEEK_SET)) 
			for (i = 0; i < GameInfo ().triggers.count; i++)
				ReadTrigger (Triggers (i), loadfile, false);
		int bObjTriggersOk = 1;
		if (GameInfo ().fileinfo_version >= 33) {
			int i = ftell (loadfile);
			if (fread (&NumObjTriggers (), sizeof (int), 1, loadfile) != 1) {
				ErrorMsg ("Error reading object triggers from mine.");
				bObjTriggersOk = 0;
				}
			else {
				for (i = 0; i < NumObjTriggers (); i++)
					ReadTrigger (ObjTriggers (i), loadfile, true);
				if (fread (ObjTriggerList (), sizeof (CDObjTriggerList), NumObjTriggers (), loadfile) != NumObjTriggers ()) {
					ErrorMsg ("Error reading object triggers from mine.");
					bObjTriggersOk = 0;
					}
				if (GameInfo ().fileinfo_version < 36) {
					if (fread (ObjTriggerRoot (), sizeof (short), 700, loadfile) != 700) {
						ErrorMsg ("Error reading object triggers from mine.");
						bObjTriggersOk = 0;
						}
					}
				else {
					memset (ObjTriggerRoot (), 0xff, sizeof (short) * MAX_OBJECTS);
					if (fread (&nObjsWithTrigger, sizeof (nObjsWithTrigger), 1, loadfile) != 1) {
						ErrorMsg ("Error reading object triggers from mine.");
						bObjTriggersOk = 0;
						}
					else {
						for (i = 0; i < nObjsWithTrigger; i++) {
							if ((fread (&nObject, sizeof (nObject), 1, loadfile) != 1) ||
								 (fread (&nFirstTrigger, sizeof (nFirstTrigger), 1, loadfile) != 1)) {
								ErrorMsg ("Error reading object triggers from mine.");
								bObjTriggersOk = 0;
								break;
								}
							if ((nObject < 0) || (nObject >= ObjCount ()) || (nFirstTrigger < 0) || (nFirstTrigger >= NumObjTriggers ())) {
								ErrorMsg ("Error reading object triggers from mine.");
								bObjTriggersOk = 0;
								break;
								}
							*ObjTriggerRoot (nObject) = nFirstTrigger;
							}
						}
					}
				}
			}
		if (!(bObjTriggersOk && NumObjTriggers ())) {
			NumObjTriggers () = 0;
			memset (ObjTriggers (), 0, sizeof (CDTrigger) * MAX_OBJ_TRIGGERS);
			memset (ObjTriggerList (), 0xff, sizeof (CDObjTriggerList) * MAX_OBJ_TRIGGERS);
			memset (ObjTriggerRoot (), 0xff, sizeof (short) * MAX_OBJECTS);
			}
	}

	//================ READ CONTROL CENTER TRIGGER INFO============== =
	// note: same for D1 and D2
	if (GameInfo ().control.offset > -1) {
		if (GameInfo ().control.count > MAX_CONTROL_CENTER_TRIGGERS) {
			sprintf(message, "Error: Max number of control center Triggers () (%ld, %d) exceeded",
				GameInfo ().control.count, MAX_CONTROL_CENTER_TRIGGERS);
			ErrorMsg(message);
			GameInfo ().control.count = MAX_CONTROL_CENTER_TRIGGERS;
		}
		if (!fseek(loadfile, GameInfo ().control.offset, SEEK_SET))  {
//			for (i = 0; i < GameInfo ().control.count; i++)
				if (fread(CCTriggers (), TotalSize (GameInfo ().control), 1, loadfile)!= 1) {
					ErrorMsg("Error reading control center triggers from mine.cpp");
//					break;
				}
		}
	}

	//================ READ MATERIALIZATION CENTER INFO============== =
	// note: added robot_flags2 for Descent 2
	if (GameInfo ().botgen.offset > -1) {
		if (GameInfo ().botgen.count > MAX_NUM_MATCENS) {
			sprintf(message, "Error: Max number of robot centers (%ld/%d) exceeded",
				GameInfo ().botgen.count, MAX_NUM_MATCENS);
			ErrorMsg(message);
			GameInfo ().botgen.count = MAX_NUM_MATCENS;
		}
		if (!fseek(loadfile, GameInfo ().botgen.offset, SEEK_SET))  {
			for (i = 0; i < GameInfo ().botgen.count; i++) {
				if (file_type != RDL_FILE) {
					if (fread(BotGens (i), (INT16)GameInfo ().botgen.size, 1, loadfile)!= 1) {
						ErrorMsg("Error reading botgens from mine.cpp");
						break;
					}
				} else {
					BotGens (i)->objFlags[0] = read_INT32(loadfile);
					// skip robot_flags2
					BotGens (i)->hit_points = read_FIX(loadfile);
					BotGens (i)->interval = read_FIX(loadfile);
					BotGens (i)->segnum = read_INT16(loadfile);
					BotGens (i)->fuelcen_num = read_INT16(loadfile);
				}
			}
		}
	}

	//================ READ EQUIPMENT CENTER INFO============== =
	// note: added robot_flags2 for Descent 2
	if (GameInfo ().equipgen.offset > -1) {
		if (GameInfo ().equipgen.count > MAX_NUM_MATCENS) {
			sprintf(message, "Error: Max number of robot centers (%ld/%d) exceeded",
				GameInfo ().equipgen.count, MAX_NUM_MATCENS);
			ErrorMsg(message);
			GameInfo ().equipgen.count = MAX_NUM_MATCENS;
		}
		if (!fseek(loadfile, GameInfo ().equipgen.offset, SEEK_SET))  {
			for (i = 0; i < GameInfo ().equipgen.count; i++) {
				if (file_type != RDL_FILE) {
					if (fread(EquipGens (i), (INT16)GameInfo ().equipgen.size, 1, loadfile)!= 1) {
						ErrorMsg("Error reading equipgens from mine.cpp");
						break;
					}
				} else {
					EquipGens (i)->objFlags[0] = read_INT32(loadfile);
					// skip robot_flags2
					EquipGens (i)->hit_points = read_FIX(loadfile);
					EquipGens (i)->interval = read_FIX(loadfile);
					EquipGens (i)->segnum = read_INT16(loadfile);
					EquipGens (i)->fuelcen_num = read_INT16(loadfile);
				}
			}
		}
	}

	//================ READ DELTA LIGHT INFO============== =
	// note: D2 only
	if (file_type != RDL_FILE) {
		//    sprintf(message, "Number of delta light indices = %ld", GameInfo ().dl_indices.count);
		//    DEBUGMSG(message);
		if (GameInfo ().dl_indices.count > MAX_DL_INDICES) {
			sprintf(message, "Error: Max number of delta light indices (%ld/%d) exceeded",
				GameInfo ().dl_indices.count, MAX_DL_INDICES);
			ErrorMsg(message);
			GameInfo ().dl_indices.count = MAX_DL_INDICES;
		}
		if (GameInfo ().dl_indices.offset > -1 && GameInfo ().dl_indices.count > 0) {
			if (!fseek(loadfile, GameInfo ().dl_indices.offset, SEEK_SET)) {
				if (fread(DLIndex (), TotalSize (GameInfo ().dl_indices), 1, loadfile)!= 1) {
					ErrorMsg("Error reading delta light indices from mine.cpp");
				}
			}
		}
	}

	//==================== READ DELTA LIGHTS==================== =
	// note: D2 only
	if (file_type != RDL_FILE) {
		//    sprintf(message, "Number of delta light values = %ld", GameInfo ().delta_lights.count);
		//    DEBUGMSG(message);
		if (GameInfo ().delta_lights.count > MAX_DELTA_LIGHTS) {
			sprintf(message, "Error: Max number of delta light values (%ld/%d) exceeded",
				GameInfo ().delta_lights.count, MAX_DELTA_LIGHTS);
			ErrorMsg(message);
			GameInfo ().delta_lights.count = MAX_DELTA_LIGHTS;
		}
		if (GameInfo ().delta_lights.offset > -1 && GameInfo ().dl_indices.count > 0) {
			if (!fseek(loadfile, GameInfo ().delta_lights.offset, SEEK_SET)) {
				delta_light *dl, temp_dl;
				dl = DeltaLights ();
				for (i = 0; i < GameInfo ().delta_lights.count; i++) {
					if (fread(&temp_dl, GameInfo ().delta_lights.size, 1, loadfile)!= 1) {
						ErrorMsg("Error reading delta light values from mine.cpp");
						break;
					}
					memcpy(dl, &temp_dl, (INT32)(GameInfo ().delta_lights.size));
					dl++;
				}
			}
		}
	}


	return 0;
}

// ------------------------------------------------------------------------
// ReadObject()
// ------------------------------------------------------------------------

void CMine::ReadObject(CDObject *obj, FILE *f, INT32 version) 
{
	int i;

	obj->type = read_INT8(f);
	obj->id = read_INT8(f);
	obj->control_type = read_INT8(f);
	obj->movement_type = read_INT8(f);
	obj->render_type = read_INT8(f);
	obj->flags = read_INT8(f);
	obj->segnum = read_INT16(f);
	read_vector(&obj->pos, f);
	read_matrix(&obj->orient, f);
	obj->size = read_FIX(f);
	obj->shields = read_FIX(f);
	read_vector(&obj->last_pos, f);
	obj->contains_type = read_INT8(f);
	obj->contains_id = read_INT8(f);
	obj->contains_count = read_INT8(f);

	switch (obj->movement_type) {
    case MT_PHYSICS:
		read_vector(&obj->mtype.phys_info.velocity, f);
		read_vector(&obj->mtype.phys_info.thrust, f);
		obj->mtype.phys_info.mass = read_FIX(f);
		obj->mtype.phys_info.drag = read_FIX(f);
		obj->mtype.phys_info.brakes = read_FIX(f);
		read_vector(&obj->mtype.phys_info.rotvel, f);
		read_vector(&obj->mtype.phys_info.rotthrust, f);
		obj->mtype.phys_info.turnroll = read_FIXANG(f);
		obj->mtype.phys_info.flags = read_INT16(f);
		break;

    case MT_SPINNING:
		read_vector(&obj->mtype.spin_rate, f);
		break;

    case MT_NONE:
		break;

    default:
		break;
	}

	switch (obj->control_type) {
    case CT_AI: {
		INT16 i;
		obj->ctype.ai_info.behavior = read_INT8(f);
		for (i = 0; i < MAX_AI_FLAGS; i++) {
			obj->ctype.ai_info.flags [i] = read_INT8(f);
		}
		obj->ctype.ai_info.hide_segment = read_INT16(f);
		obj->ctype.ai_info.hide_index = read_INT16(f);
		obj->ctype.ai_info.path_length = read_INT16(f);
		obj->ctype.ai_info.cur_path_index = read_INT16(f);
		if (file_type != RL2_FILE) {
			obj->ctype.ai_info.follow_path_start_seg = read_INT16(f);
			obj->ctype.ai_info.follow_path_end_seg = read_INT16(f);
		}
		break;
				}
    case CT_EXPLOSION:
		obj->ctype.expl_info.spawn_time = read_FIX(f);
		obj->ctype.expl_info.delete_time = read_FIX(f);
		obj->ctype.expl_info.delete_objnum = (UINT8)read_INT16(f);
		obj->ctype.expl_info.next_attach = obj->ctype.expl_info.prev_attach = obj->ctype.expl_info.attach_parent =-1;
		break;

    case CT_WEAPON:
		obj->ctype.laser_info.parent_type = read_INT16(f);
		obj->ctype.laser_info.parent_num = read_INT16(f);
		obj->ctype.laser_info.parent_signature = read_INT32(f);
		break;

    case CT_LIGHT:
		obj->ctype.light_info.intensity = read_FIX(f);
		break;

    case CT_POWERUP:
		if (version >= 25) {
			obj->ctype.powerup_info.count = read_INT32(f);
		} else {
			obj->ctype.powerup_info.count = 1;
			//      if (obj->id== POW_VULCAN_WEAPON)
			//          obj->ctype.powerup_info.count = VULCAN_WEAPON_AMMO_AMOUNT;
		}
		break;

    case CT_NONE:
    case CT_FLYING:
    case CT_DEBRIS:
		break;

    case CT_SLEW:    /*the player is generally saved as slew */
		break;

    case CT_CNTRLCEN:
		break;

    case CT_MORPH:
    case CT_FLYTHROUGH:
    case CT_REPAIRCEN:
    default:
		break;
	}

	switch (obj->render_type) {
    case RT_NONE:
		break;

    case RT_MORPH:
    case RT_POLYOBJ: {
		INT16 i;
		INT32 tmo;
		obj->rtype.pobj_info.model_num = read_INT32(f);
		for (i = 0; i < MAX_SUBMODELS; i++) {
			read_angvec(&obj->rtype.pobj_info.anim_angles [i], f);
		}
		obj->rtype.pobj_info.subobj_flags = read_INT32(f);
		tmo = read_INT32(f);
		obj->rtype.pobj_info.tmap_override = tmo;
		obj->rtype.pobj_info.alt_textures = 0;
		break;
					 }

    case RT_WEAPON_VCLIP:
    case RT_HOSTAGE:
    case RT_POWERUP:
    case RT_FIREBALL:
		obj->rtype.vclip_info.vclip_num = read_INT32(f);
		obj->rtype.vclip_info.frametime = read_FIX(f);
		obj->rtype.vclip_info.framenum = read_INT8(f);

		break;

    case RT_LASER:
		break;

	case RT_SMOKE:
		obj->rtype.smokeInfo.nLife = read_INT32 (f);
		obj->rtype.smokeInfo.nSize [0] = read_INT32 (f);
		obj->rtype.smokeInfo.nParts = read_INT32 (f);
		obj->rtype.smokeInfo.nSpeed = read_INT32 (f);
		obj->rtype.smokeInfo.nDrift = read_INT32 (f);
		obj->rtype.smokeInfo.nBrightness = read_INT32 (f);
		for (i = 0; i < 4; i++)
			obj->rtype.smokeInfo.color [i] = read_INT8 (f);
		obj->rtype.smokeInfo.nSide = read_INT8 (f);
		if (level_version < 18)
			obj->rtype.smokeInfo.nType = 0;
		else
			obj->rtype.smokeInfo.nType = read_INT8 (f);
		break;

	case RT_LIGHTNING:
		obj->rtype.lightningInfo.nLife = read_INT32 (f);
		obj->rtype.lightningInfo.nDelay = read_INT32 (f);
		obj->rtype.lightningInfo.nLength = read_INT32 (f);
		obj->rtype.lightningInfo.nAmplitude = read_INT32 (f);
		obj->rtype.lightningInfo.nOffset = read_INT32 (f);
		obj->rtype.lightningInfo.nLightnings = read_INT16 (f);
		obj->rtype.lightningInfo.nId = read_INT16 (f);
		obj->rtype.lightningInfo.nTarget = read_INT16 (f);
		obj->rtype.lightningInfo.nNodes = read_INT16 (f);
		obj->rtype.lightningInfo.nChildren = read_INT16 (f);
		obj->rtype.lightningInfo.nSteps = read_INT16 (f);
		obj->rtype.lightningInfo.nAngle = read_INT8 (f);
		obj->rtype.lightningInfo.nStyle = read_INT8 (f);
		obj->rtype.lightningInfo.nSmoothe = read_INT8 (f);
		obj->rtype.lightningInfo.bClamp = read_INT8 (f);
		obj->rtype.lightningInfo.bPlasma = read_INT8 (f);
		obj->rtype.lightningInfo.bSound = read_INT8 (f);
		obj->rtype.lightningInfo.bRandom = read_INT8 (f);
		obj->rtype.lightningInfo.bInPlane = read_INT8 (f);
		for (i = 0; i < 4; i++)
			obj->rtype.lightningInfo.color [i] = read_INT8 (f);
		break;

	default:
		break;
	}
}

// ------------------------------------------------------------------------
// CMine - save()
//
// ACTION -  saves a level (.RDL) file to disk
// ------------------------------------------------------------------------

INT16 CMine::Save (const char * filename_passed, bool bSaveToHog)
{
#if DEMO== 0
#ifdef _DEBUG
	CMine *mine = theApp.GetMine ();
#endif
	FILE * save_file;
	char filename [128];
	INT32 minedata_offset, gamedata_offset, hostagetext_offset;
	INT32 mine_err, game_err;

	//  if (disable_saves) {
	//    ErrorMsg("Saves disabled, contact Bryan Aamot for your security number.");
	//  }
	strcpy(filename, filename_passed);

	//  if (disable_saves) return 0;
	save_file = fopen(filename, "wb");

	if (!save_file) {
		//    sprintf(message, "Can't open save file < %s>", filename);
		//    show_message(str, RED, 1, 1);
		return(1);
	}

	changes_made = 0;

	// write file signature
	write_INT32('P'*0x1000000L + 'L'*0x10000L + 'V'*0x100 + 'L', save_file); // signature

	// always save as version 7 or greater if its a D2 level
	// otherwise, blinking lights will not work.
	if (level_version < 7 && file_type != RDL_FILE) {
		level_version = 7;
	}
	if ((level_version >= 9) && (level_version < LEVEL_VERSION)) {
		level_version = LEVEL_VERSION;
		//if (level_version < 15)
			ConvertWallNum (MAX_WALLS2 + 1, MAX_WALLS3 + 1);
		}

	// write version
	write_INT32(level_version, save_file);

	write_INT32(0, save_file); // minedata_offset (temporary)
	write_INT32(0, save_file); // gamedata_offset (temporary)


	if (file_type != RDL_FILE) {
		if (level_version >= 8) {
			write_INT16(rand(), save_file);
			write_INT16(rand(), save_file);
			write_INT16(rand(), save_file);
			write_INT8((INT8)rand(), save_file);
		}
	}

	if (file_type== RDL_FILE) {
		write_INT32(0, save_file); // hostagetext_offset (temporary)
	} else {

		// save palette name
		char *name = strrchr(descent2_path, '\\');
		if (!name) {
			name = descent2_path; // point to 1st char if no slash found
		} else {
			name++;               // point to character after slash
		}
		char palette_name [15];
		strncpy(palette_name, name, 12);
		palette_name [13] = NULL;  // null terminate just in case
		// replace extension with *.256
		if (strlen((char *)palette_name) > 4) {
			strcpy(&palette_name [strlen((char *)palette_name) - 4], ".256");
		} else {
			strcpy(palette_name, "GROUPA.256");
		}
		strupr(palette_name);
		strcat(palette_name, "\n"); // add a return to the end
		fwrite(palette_name, strlen((char *)palette_name), 1, save_file);
	}

	// write reactor info
	if (file_type != RDL_FILE) {
		// read descent 2 reactor information
		write_INT32(ReactorTime (), save_file);
		write_INT32(ReactorStrength (), save_file);

		// flickering light new for version 7
		write_INT32(FlickerLightCount (), save_file);
		if (FlickerLightCount () > MAX_FLICKERING_LIGHTS) {
			FlickerLightCount () = MAX_FLICKERING_LIGHTS;
		}
		if (FlickerLightCount () > 0) {
			fwrite(FlickeringLights (), sizeof(FLICKERING_LIGHT), FlickerLightCount (), save_file);
		}

		// write secret cube number
		write_INT32(SecretCubeNum (), save_file);

		// write secret cube orientation?
		write_matrix(&SecretOrient (), save_file);

	}

	// save mine data
	minedata_offset = ftell(save_file);
	mine_err = SaveMineDataCompiled(save_file);

	if (mine_err== -1) {
		fclose(save_file);
		ErrorMsg("Error saving mine data");
		return(2);
	}

	// save game data
	gamedata_offset = ftell(save_file);
	game_err = SaveGameData(save_file);

	if (game_err== -1) {
		fclose(save_file);
		ErrorMsg("Error saving game data");
		return(3);
	}

	// save hostage data
	hostagetext_offset = ftell(save_file);
	// leave hostage text empty

	// now and go back to beginning of file and save offsets
	fseek(save_file, 2*sizeof(INT32), SEEK_SET);
	write_INT32(minedata_offset, save_file);    // gamedata_offset
	write_INT32(gamedata_offset, save_file);    // gamedata_offset
	if (file_type== RDL_FILE) {
		write_INT32(hostagetext_offset, save_file); // hostagetext_offset
	}

	fclose(save_file);
if (HasCustomTextures () && !bSaveToHog) {
	strcpy (strstr (filename, "."), ".pog");
	save_file = fopen(filename, "wb");
	if (save_file) {
		CreatePog (save_file);
		fclose (save_file);
		}
	}
if (HasCustomRobots () && !bSaveToHog) {
	strcpy (strstr (filename, "."), ".hxm");
	save_file = fopen(filename, "wb");
	if (save_file)
		WriteHxmFile (save_file);
	}
#endif //DEMO
	return 0;
}

// ------------------------------------------------------------------------

void CMine::SaveColor (CDColor *pc, FILE *save_file)
{
	int	c;

fwrite (&pc->index, sizeof (pc->index), 1, save_file);
c = (int) (pc->color.r * 0x7fffffff + 0.5);
fwrite (&c, sizeof (c), 1, save_file);
c = (int) (pc->color.g * 0x7fffffff + 0.5);
fwrite (&c, sizeof (c), 1, save_file);
c = (int) (pc->color.b * 0x7fffffff + 0.5);
fwrite (&c, sizeof (c), 1, save_file);
}

// ------------------------------------------------------------------------

void CMine::SortDLIndex (int left, int right)
{
	int	l = left,
			r = right,
			m = (left + right) / 2;
	short	mSeg = DLIndex (m)->d2x.segnum, 
			mSide = DLIndex (m)->d2x.sidenum;
	dl_index	*pl, *pr;

do {
	pl = DLIndex (l);
	while ((pl->d2x.segnum < mSeg) || ((pl->d2x.segnum == mSeg) && (pl->d2x.sidenum < mSide))) {
		pl++;
		l++;
		}
	pr = DLIndex (r);
	while ((pr->d2x.segnum > mSeg) || ((pr->d2x.segnum == mSeg) && (pr->d2x.sidenum > mSide))) {
		pr--;
		r--;
		}
	if (l <= r) {
		if (l < r) {
			dl_index	h = *pl;
			*pl = *pr;
			*pr = h;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (right > l)
   SortDLIndex (l, right);
if (r > left)
   SortDLIndex (left, r);
}

// ------------------------------------------------------------------------

void CMine::SaveColors (CDColor *pc, int nColors, FILE *fp)
{
for (; nColors; nColors--, pc++)
	SaveColor (pc, fp);
}

//--------------------------------------------------------------------------

int CMine::WriteColorMap (FILE *fColorMap)
{
SaveColors (TexColors (), MAX_D2_TEXTURES, fColorMap);
return 0;
}

//--------------------------------------------------------------------------

int CMine::ReadColorMap (FILE *fColorMap)
{
LoadColors (TexColors (), MAX_D2_TEXTURES, 0, 0, fColorMap);
return 0;
}

// ------------------------------------------------------------------------
// SaveMineDataCompiled()
//
// ACTION - Writes a mine data portion of RDL file.
// ------------------------------------------------------------------------
INT16 CMine::SaveMineDataCompiled(FILE *save_file)
{
#if DEMO== 0
	INT16    i, segnum, sidenum; /** was INT32 */
	UINT16   temp_UINT16;
	UINT8 bitmask;

	//============================== = Writing part==============================

	// write version (1 byte)
	write_INT8(COMPILED_MINE_VERSION, save_file);

	// write no. of vertices (2 bytes)
	write_INT16(VertCount (), save_file);

	// write number of Segments () (2 bytes)
	write_INT16(SegCount (), save_file);

	// write all vertices
	fwrite(Vertices (), sizeof (vms_vector), VertCount (), save_file);

	// write segment information
	for (segnum = 0; segnum < SegCount (); segnum++)   {
		INT16   bit;

#if 0
		INT32 off;
		off = ftell(save_file);
		if (segnum < 10) {
			sprintf(message, "offset = %#08lx, segnum = ", off, segnum);
			INFOMSG (message);
		}
#endif


		CDSegment *seg = Segments (segnum);

		if (level_version >= 9) {
			fwrite(&seg->owner, sizeof(UINT8), 1, save_file);
			fwrite(&seg->group, sizeof(INT8), 1, save_file);
			}
		// write child bit mask (1 byte)
		// (first calculate child_bitmask from children)
		bitmask = 0;
		for (sidenum = 0; sidenum < MAX_SIDES_PER_SEGMENT; sidenum++) {
			if(seg->children [sidenum] != -1) {
				bitmask |= (1 << sidenum);
			}
		}
		if (file_type== RDL_FILE) {
			if (seg->special != 0) { // if this is a special cube
				bitmask |= (1 << MAX_SIDES_PER_SEGMENT);
			}
		}

#if 0
		if (bitmask != seg->child_bitmask) {
			sprintf(message, " Saving mine: child_bitmask(%#02x) != calculated one(%#02x)\n"
				"for cube (%d)", seg->child_bitmask, bitmask, segnum);
			DEBUGMSG(message);
		}
#endif
		bitmask = seg->child_bitmask; // override for test purposes

		fwrite(&bitmask, sizeof(UINT8), 1, save_file);

		// write children numbers (0 to 6 bytes)
		for (bit = 0; bit < MAX_SIDES_PER_SEGMENT; bit++) {
			if (bitmask & (1 << bit)) {
				fwrite(&seg->children [bit], sizeof(INT16), 1, save_file);
			}
		}

		// write vertex numbers (16 bytes)
		fwrite(seg->verts, sizeof(INT16), MAX_VERTICES_PER_SEGMENT, save_file);

		// write special info (0 to 4 bytes)
		if (file_type != RL2_FILE) {
			if (bitmask & (1 << MAX_SIDES_PER_SEGMENT)) {
				fwrite(&seg->special, sizeof(UINT8), 1, save_file);
				fwrite(&seg->matcen_num, sizeof(INT8), 1, save_file);
				fwrite(&seg->value, sizeof(INT8), 1, save_file);
				fwrite(&seg->s2_flags, sizeof(UINT8), 1, save_file); // this should be 0
			}

			// write static light (2 bytes)
			temp_UINT16 = (UINT16)((seg->static_light) >> 4);
			fwrite(&temp_UINT16, sizeof(temp_UINT16), 1, save_file);
		}

		// calculate wall bit mask
		bitmask = 0;
		for (sidenum = 0; sidenum < MAX_SIDES_PER_SEGMENT; sidenum++) {
			if(seg->sides [sidenum].nWall < GameInfo ().walls.count) {
				bitmask |= (1 << sidenum);
			}
		}
		fwrite(&bitmask, sizeof(UINT8), 1, save_file);

		// write wall numbers
		for (sidenum = 0; sidenum < MAX_SIDES_PER_SEGMENT; sidenum++) {
			if (bitmask & (1 << sidenum)) {
				if (level_version >= 13)
					fwrite(&seg->sides [sidenum].nWall, sizeof(UINT16), 1, save_file);
				else
					fwrite(&seg->sides [sidenum].nWall, sizeof(UINT8), 1, save_file);
			}
		}

		// write textures and uvls
		for (sidenum = 0; sidenum < MAX_SIDES_PER_SEGMENT; sidenum++)   {
			if ((seg->children [sidenum]==-1) || (bitmask & (1 << sidenum)))   {
				// write texture 1 & 2 number
				temp_UINT16 = seg->sides [sidenum].nBaseTex;
				if (!seg->sides [sidenum].nOvlTex) {
					fwrite(&temp_UINT16, sizeof(UINT16), 1, save_file);
				} else {
					temp_UINT16 |= 0x8000;
					fwrite(&temp_UINT16, sizeof(UINT16), 1, save_file);
					fwrite(&seg->sides [sidenum].nOvlTex, sizeof(INT16), 1, save_file);
				}

				//   write uvls
				for (i = 0; i < 4; i++)   {
					fwrite(&seg->sides [sidenum].uvls [i].u, sizeof(INT16), 1, save_file);
					fwrite(&seg->sides [sidenum].uvls [i].v, sizeof(INT16), 1, save_file);
					fwrite(&seg->sides [sidenum].uvls [i].l, sizeof(INT16), 1, save_file);
				}
			}
		}
  }

  // for Descent 2, save special info here
  if (file_type != RDL_FILE) {
	  CDSegment *seg = Segments ();
	  for (segnum = 0; segnum < SegCount (); segnum++, seg++)   {
		  // write special info (8 bytes)
			if ((seg->special == SEGMENT_IS_ROBOTMAKER) && (seg->matcen_num == -1)) {
				seg->special = SEGMENT_IS_NOTHING;
				seg->value = 0;
				seg->child_bitmask &= ~(1 << MAX_SIDES_PER_SEGMENT);
				}
			fwrite(&seg->special, sizeof(UINT8), 1, save_file);
			fwrite(&seg->matcen_num, sizeof(INT8), 1, save_file);
			fwrite(&seg->value, sizeof(INT8), 1, save_file);
			fwrite(&seg->s2_flags, sizeof(UINT8), 1, save_file);
			fwrite(&seg->static_light, sizeof(FIX), 1, save_file);
	  }
	if (level_version >= 9) {
		SaveColors (VertexColors (), VertCount (), save_file);
		SaveColors (LightColors (), SegCount () * 6, save_file);
		SaveColors (TexColors (), MAX_D2_TEXTURES, save_file);
		}
  }
#endif //DEMO
  return 0;
}

// ------------------------------------------------------------------------

void CMine::WriteTrigger (CDTrigger *t, FILE *fp, bool bObjTrigger)
{
	int	i;
	char	pad = 0;

if (file_type != RDL_FILE) {
	fwrite (&t->type, sizeof(INT8), 1, fp);
	if (bObjTrigger)
		fwrite (&t->flags, sizeof(INT16), 1, fp);
	else
		fwrite (&t->flags, sizeof(INT8), 1, fp);
	fwrite (&t->num_links, sizeof(INT8), 1, fp);
	fwrite (&pad, sizeof(INT8), 1, fp);
	fwrite (&t->value, sizeof(FIX), 1, fp);
	fwrite (&t->time, sizeof(FIX), 1, fp);
	}
else {
	fwrite (&t->type, sizeof(INT8), 1, fp);
	fwrite (&t->flags, sizeof(INT16), 1, fp);
	fwrite (&t->value, sizeof(FIX), 1, fp);
	fwrite (&t->time, sizeof(FIX), 1, fp);
	fwrite (&t->num_links, sizeof(INT8), 1, fp);
	fwrite (&t->num_links, sizeof(INT16), 1, fp);
	}
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	fwrite(t->seg  + i, sizeof(INT16), 1, fp);
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	fwrite(&t->side [i], sizeof(INT16), 1, fp);
}

// ------------------------------------------------------------------------

void CMine::ReadTrigger (CDTrigger *t, FILE *fp, bool bObjTrigger)
{
	int	i;

if (file_type != RDL_FILE) {
	t->type = read_INT8(fp);
	if (bObjTrigger)
		t->flags = read_INT16(fp);
	else
		t->flags = (UINT16) read_INT8(fp);
	t->num_links = read_INT8(fp);
	read_INT8(fp);
	t->value = read_FIX(fp);
	t->time = read_FIX(fp);
	}
else {
	t->type = read_INT8(fp);
	t->flags = read_INT16(fp);
	t->value = read_FIX(fp);
	t->time = read_FIX(fp);
	read_INT8(fp); //skip 8 bit value "link_num"
	t->num_links = (INT8) read_INT16(fp);
	if (t->num_links < 0)
		t->num_links = 0;
	else if (t->num_links > MAX_TRIGGER_TARGETS)
		t->num_links = MAX_TRIGGER_TARGETS;
	}
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	t->seg [i] = read_INT16(fp);
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	t->side [i] = read_INT16(fp);
}

// ------------------------------------------------------------------------
// SaveGameData()
//
//  ACTION - Saves the player, object, wall, door, trigger, and
//           materialogrifizationator data from an RDL file.
// ------------------------------------------------------------------------
INT16 CMine::SaveGameData(FILE *savefile)
{
#if DEMO== 0
	HINSTANCE hInst = AfxGetInstanceHandle();

	INT32 i;
	INT32 start_offset, end_offset;
	short nObject, nObjsWithTrigger, nFirstTrigger;

	start_offset = ftell(savefile);

	//==================== = WRITE FILE INFO========================

	// Do not assume the "sizeof" values are the same as what was read when level was loaded.
	// Also be careful no to use sizeof() because the editor's internal size may not match
	// the size which is used by the game engine.
	GameInfo ().objects.size = 0x108;                         // 248 = sizeof(object)
	GameInfo ().walls.size = 24;                            // 24 = sizeof(wall)
	GameInfo ().doors.size = 16;                            // 16 = sizeof(active_door)
	GameInfo ().triggers.size = (file_type== RDL_FILE) ? 54:52; // 54 = sizeof(trigger)
	GameInfo ().control.size = 42;                            // 42 = sizeof(control_center_trigger)
	GameInfo ().botgen.size = (file_type== RDL_FILE) ? 16:20; // 20 = sizeof(matcen_info)
	GameInfo ().equipgen.size = 20; // 20 = sizeof(matcen_info)
	GameInfo ().dl_indices.size = 6;                             // 6 = sizeof(dl_index)
	GameInfo ().delta_lights.size = 8;                             // 8 = sizeof(delta_light)

	// the offsets will be calculated as we go then rewritten at the end
	//  GameInfo ().doors.offset =-1;
	//  GameInfo ().player.offset =-1;
	//  GameInfo ().objects.offset =-1;
	//  GameInfo ().walls.offset =-1;
	//  GameInfo ().triggers.offset =-1;
	//  GameInfo ().control.offset =-1;
	//  GameInfo ().matcen.offset =-1;
	//  GameInfo ().dl_indices.offset =-1;
	//  GameInfo ().delta_lights.offset =-1;

	// these numbers (.howmany) are updated by the editor
	//  GameInfo ().objects.count = 0;
	//  GameInfo ().walls.count = 0;
	//  GameInfo ().doors.count = 0;
	//  GameInfo ().triggers.count = 0;
	//  GameInfo ().control.count = 0;
	//  GameInfo ().matcen.count = 0;
	//  GameInfo ().dl_indices.count = 0; // D2
	//  GameInfo ().delta_lights.count = 0; // D2

	if (file_type== RDL_FILE) {
		GameInfo ().fileinfo_signature = 0x6705;
		GameInfo ().fileinfo_version = 25;
		GameInfo ().fileinfo_size = 119;
		GameInfo ().level = 0;
		}
	else {
		GameInfo ().fileinfo_signature = 0x6705;
		GameInfo ().fileinfo_version = (level_version < 13) ? 31 : 36;
		GameInfo ().fileinfo_size = (level_version < 13) ? 143 : sizeof (GameInfo ()); // same as sizeof(GameInfo ())
		GameInfo ().level = 0;
	}

	fwrite(&GameInfo (), (INT16)GameInfo ().fileinfo_size, 1, savefile);
	if (GameInfo ().fileinfo_version >= 14) {  /*save mine filename */
		fwrite(current_level_name, sizeof(char), strlen(current_level_name), savefile);
	}
	if (file_type != RDL_FILE) {
		fwrite("\n", 1, 1, savefile); // write an end - of - line
	} else {
		fwrite("", 1, 1, savefile);   // write a null
	}

	// write pof names from resource file
	HRSRC     hRes;
	HGLOBAL   hGlobal;
	UINT8 *save_pof_names;
	INT16 n_save_pof_names, n_pofs;

	if (file_type != RDL_FILE) {
		n_save_pof_names = 166;
		if (!(hRes = FindResource(hInst, MAKEINTRESOURCE(IDR_POF_NAMES2), "RC_DATA")))
			return 1;
		fwrite(&n_save_pof_names, 2, 1, savefile);   // write # of POF names
		}
	else {
		n_save_pof_names = 78;
		if (!(hRes = FindResource(hInst, MAKEINTRESOURCE(IDR_POF_NAMES1), "RC_DATA")))
			return 1;
		n_pofs = 25;   // Don't know exactly what this value is for or why it is 25?
		fwrite(&n_pofs, 2, 1, savefile);
		}
	hGlobal = LoadResource(hInst, hRes);
	ASSERT(hGlobal);

	save_pof_names = (UINT8 *) LockResource(hGlobal);
	ASSERT(save_pof_names);

	fwrite(save_pof_names, n_save_pof_names, 13, savefile); // 13 characters each
	FreeResource(hGlobal);

	//==================== = WRITE PLAYER INFO==========================
	GameInfo ().player.offset = ftell(savefile);
	char* str = "Made with Descent Level Editor XP 32\0\0\0\0\0\0\0";
	fwrite(str, strlen(str) + 1, 1, savefile);

	//==================== = WRITE OBJECT INFO==========================
	// note: same for D1 and D2
	GameInfo ().objects.offset = ftell(savefile);
	for (i = 0; i < GameInfo ().objects.count; i++)
		WriteObject(Objects (i), savefile, GameInfo ().fileinfo_version);

	//==================== = WRITE WALL INFO============================
	// note: Wall size will automatically strip last two items
	//       when saving D1 level
	GameInfo ().walls.offset = ftell(savefile);
	if (GameInfo ().fileinfo_version >= 20) {
//	for (i = 0; i < GameInfo ().walls.count; i++)
			fwrite(Walls (), TotalSize (GameInfo ().walls), 1, savefile);
	}

	//==================== = WRITE DOOR INFO============================
	// note: not used for D1 or D2 since doors.count is always 0
	GameInfo ().doors.offset = ftell(savefile);
	if (GameInfo ().fileinfo_version >= 20)
//	for (i = 0; i < GameInfo ().doors.count; i++)
			fwrite(ActiveDoors (i), TotalSize (GameInfo ().doors), 1, savefile);

	//==================== WRITE TRIGGER INFO==========================
	// note: order different for D2 levels but size is the same
	GameInfo ().triggers.offset = ftell(savefile);
	for (i = 0; i < GameInfo ().triggers.count; i++)
		WriteTrigger (Triggers (i), savefile, false);
	if (level_version >= 12) {
		fwrite (&NumObjTriggers (), sizeof (int), 1, savefile);
		for (i = 0; i < NumObjTriggers (); i++)
			WriteTrigger (ObjTriggers (i), savefile, true);
		fwrite (ObjTriggerList (), sizeof (CDObjTriggerList), NumObjTriggers (), savefile);
		for (nObject = 0, nObjsWithTrigger = 0; nObject < MAX_OBJECTS; nObject++) {
			nFirstTrigger = *ObjTriggerRoot (nObject);
			if ((nFirstTrigger >= 0) && (nFirstTrigger < NumObjTriggers ()))
				nObjsWithTrigger++;
			}
		fwrite (&nObjsWithTrigger, sizeof (short), 1, savefile);
		if (nObjsWithTrigger > 0) {
			for (nObject = 0; nObject < MAX_OBJECTS; nObject++) {
				nFirstTrigger = *ObjTriggerRoot (nObject);
				if ((nFirstTrigger >= 0) && (nFirstTrigger < NumObjTriggers ())) {
					fwrite (&nObject, sizeof (short), 1, savefile);
					fwrite (&nFirstTrigger, sizeof (short), 1, savefile);
					}
				}
			}
		}

	//================ WRITE CONTROL CENTER TRIGGER INFO============== =
	// note: same for D1 and D2
	GameInfo ().control.offset = ftell(savefile);
	//for (i = 0; i < GameInfo ().control.count; i++)
		fwrite(CCTriggers (), TotalSize (GameInfo ().control), 1, savefile);

	//================ WRITE MATERIALIZATION CENTERS INFO============== =
	// note: added robot_flags2 for Descent 2
	GameInfo ().botgen.offset = ftell(savefile);
	if (file_type != RDL_FILE)
		fwrite(BotGens (), TotalSize (GameInfo ().botgen), 1, savefile);
	else {
		for (i = 0; i < GameInfo ().botgen.count; i++) {
			write_INT32(BotGens (i)->objFlags[0], savefile);
			// skip robot_flags2
			write_FIX  (BotGens (i)->hit_points, savefile);
			write_FIX  (BotGens (i)->interval, savefile);
			write_INT16(BotGens (i)->segnum, savefile);
			write_INT16(BotGens (i)->fuelcen_num, savefile);
		}
	}

	//================ WRITE EQUIPMENT CENTERS INFO============== =
	// note: added robot_flags2 for Descent 2
	GameInfo ().equipgen.offset = ftell(savefile);
	if (file_type != RDL_FILE)
		fwrite(EquipGens (), TotalSize (GameInfo ().equipgen), 1, savefile);

	//============== CALCULATE DELTA LIGHT DATA============ =
	if (file_type != RDL_FILE)
		UpdateDeltaLights ();

	//================ WRITE DELTA LIGHT INFO============== =
	// note: D2 only
	GameInfo ().dl_indices.offset = ftell(savefile);
	if ((level_version >= 15) && (GameInfo ().fileinfo_version >= 34))
		SortDLIndex (0, GameInfo ().dl_indices.count - 1);
	if (file_type != RDL_FILE)
		fwrite(DLIndex (), TotalSize (GameInfo ().dl_indices), 1, savefile);

	//================ = WRITE DELTA LIGHTS==================
	// note: D2 only
	GameInfo ().delta_lights.offset = ftell(savefile);
	if (file_type != RDL_FILE) {
		delta_light *dl, temp_dl;
		dl = DeltaLights ();
		for (i = 0; i < GameInfo ().delta_lights.count; i++) {
			memcpy(&temp_dl, dl, (INT16)(GameInfo ().delta_lights.size));
			fwrite(&temp_dl, (INT16)(GameInfo ().delta_lights.size), 1, savefile);
			dl++;
		}
	}

	end_offset = ftell(savefile);

	//==================== = UPDATE FILE INFO OFFSETS====================== =
	fseek(savefile, start_offset, SEEK_SET);
	fwrite(&GameInfo (), (INT16)GameInfo ().fileinfo_size, 1, savefile);

	//============ = LEAVE ROUTINE AT LAST WRITTEN OFFSET================== = */
	fseek(savefile, end_offset, SEEK_SET);
#endif //DEMO
	return(0);
}

// ------------------------------------------------------------------------
// WriteObject()
// ------------------------------------------------------------------------
void CMine::WriteObject(CDObject *obj, FILE *f, INT32 version)
{
#if DEMO== 0
	int i;
	write_INT8(obj->type, f);
	write_INT8(obj->id, f);
	write_INT8(obj->control_type, f);
	write_INT8(obj->movement_type, f);
	write_INT8(obj->render_type, f);
	write_INT8(obj->flags, f);
	write_INT16(obj->segnum, f);
	write_vector(&obj->pos, f);
	write_matrix(&obj->orient, f);
	write_FIX(obj->size, f);
	write_FIX(obj->shields, f);
	write_vector(&obj->last_pos, f);
	write_INT8(obj->contains_type, f);
	write_INT8(obj->contains_id, f);
	write_INT8(obj->contains_count, f);

	switch (obj->movement_type) {
    case MT_PHYSICS:
		write_vector(&obj->mtype.phys_info.velocity, f);
		write_vector(&obj->mtype.phys_info.thrust, f);
		write_FIX(obj->mtype.phys_info.mass, f);
		write_FIX(obj->mtype.phys_info.drag, f);
		write_FIX(obj->mtype.phys_info.brakes, f);
		write_vector(&obj->mtype.phys_info.rotvel, f);
		write_vector(&obj->mtype.phys_info.rotthrust, f);
		write_FIXANG(obj->mtype.phys_info.turnroll, f);
		write_INT16(obj->mtype.phys_info.flags, f);
		break;

    case MT_SPINNING:
		write_vector(&obj->mtype.spin_rate, f);
		break;

    case MT_NONE:
		break;

    default:
		break;
	}

	switch (obj->control_type) {
	case CT_AI: {
		INT16 i;
		write_INT8(obj->ctype.ai_info.behavior, f);
		for (i = 0; i < MAX_AI_FLAGS; i++) {
			write_INT8(obj->ctype.ai_info.flags [i], f);
		}
		write_INT16(obj->ctype.ai_info.hide_segment, f);
		write_INT16(obj->ctype.ai_info.hide_index, f);
		write_INT16(obj->ctype.ai_info.path_length, f);
		write_INT16(obj->ctype.ai_info.cur_path_index, f);
		if (file_type != RL2_FILE) {
			write_INT16(obj->ctype.ai_info.follow_path_start_seg, f);
			write_INT16(obj->ctype.ai_info.follow_path_end_seg, f);
		}
		break;
				}
    case CT_EXPLOSION:
		write_FIX(obj->ctype.expl_info.spawn_time, f);
		write_FIX(obj->ctype.expl_info.delete_time, f);
		write_INT16(obj->ctype.expl_info.delete_objnum, f);
		break;

    case CT_WEAPON:
		write_INT16(obj->ctype.laser_info.parent_type, f);
		write_INT16(obj->ctype.laser_info.parent_num, f);
		write_INT32(obj->ctype.laser_info.parent_signature, f);
		break;

    case CT_LIGHT:
		write_FIX(obj->ctype.light_info.intensity, f);
		break;

    case CT_POWERUP:
		if (version >= 25) {
			write_INT32(obj->ctype.powerup_info.count, f);
		}
		break;


    case CT_NONE:
    case CT_FLYING:
    case CT_DEBRIS:
		break;

    case CT_SLEW:    /*the player is generally saved as slew */
		break;

	case CT_CNTRLCEN:
		break;

    case CT_MORPH:
    case CT_FLYTHROUGH:
    case CT_REPAIRCEN:
    default:
		break;
	}

	switch (obj->render_type) {
    case RT_NONE:
		break;

    case RT_MORPH:
    case RT_POLYOBJ: {
		INT16 i;
		INT32 tmo;

		write_INT32(obj->rtype.pobj_info.model_num, f);
		for (i = 0; i < MAX_SUBMODELS; i++) {
			write_angvec(&obj->rtype.pobj_info.anim_angles [i], f);
		}
		write_INT32(obj->rtype.pobj_info.subobj_flags, f);
		tmo = obj->rtype.pobj_info.tmap_override;
		write_INT32(tmo, f);
		break;
					 }
	case RT_WEAPON_VCLIP:
	case RT_HOSTAGE:
	case RT_POWERUP:
	case RT_FIREBALL:
		write_INT32(obj->rtype.vclip_info.vclip_num, f);
		write_FIX(obj->rtype.vclip_info.frametime, f);
		write_INT8(obj->rtype.vclip_info.framenum, f);
		break;

	case RT_LASER:
		break;

	case RT_SMOKE:
		write_INT32 (obj->rtype.smokeInfo.nLife, f);
		write_INT32 (obj->rtype.smokeInfo.nSize [0], f);
		write_INT32 (obj->rtype.smokeInfo.nParts, f);
		write_INT32 (obj->rtype.smokeInfo.nSpeed, f);
		write_INT32 (obj->rtype.smokeInfo.nDrift, f);
		write_INT32 (obj->rtype.smokeInfo.nBrightness, f);
		for (i = 0; i < 4; i++, f)
			write_INT8 (obj->rtype.smokeInfo.color [i], f);
		write_INT8 (obj->rtype.smokeInfo.nSide, f);
		write_INT8 (obj->rtype.smokeInfo.nType, f);
		break;

	case RT_LIGHTNING:
		write_INT32 (obj->rtype.lightningInfo.nLife, f);
		write_INT32 (obj->rtype.lightningInfo.nDelay, f);
		write_INT32 (obj->rtype.lightningInfo.nLength, f);
		write_INT32 (obj->rtype.lightningInfo.nAmplitude, f);
		write_INT32 (obj->rtype.lightningInfo.nOffset, f);
		write_INT16 (obj->rtype.lightningInfo.nLightnings, f);
		write_INT16 (obj->rtype.lightningInfo.nId, f);
		write_INT16 (obj->rtype.lightningInfo.nTarget, f);
		write_INT16 (obj->rtype.lightningInfo.nNodes, f);
		write_INT16 (obj->rtype.lightningInfo.nChildren, f);
		write_INT16 (obj->rtype.lightningInfo.nSteps, f);
		write_INT8 (obj->rtype.lightningInfo.nAngle, f);
		write_INT8 (obj->rtype.lightningInfo.nStyle, f);
		write_INT8 (obj->rtype.lightningInfo.nSmoothe, f);
		write_INT8 (obj->rtype.lightningInfo.bClamp, f);
		write_INT8 (obj->rtype.lightningInfo.bPlasma, f);
		write_INT8 (obj->rtype.lightningInfo.bSound, f);
		write_INT8 (obj->rtype.lightningInfo.bRandom, f);
		write_INT8 (obj->rtype.lightningInfo.bInPlane, f);
		for (i = 0; i < 4; i++, f)
			write_INT8 (obj->rtype.lightningInfo.color [i], f);
		break;

	default:
		break;

	}
#endif //DEMO
}


// ------------------------------------------------------------------------
// UpdateDeltaLights ()
// ------------------------------------------------------------------------
void CMine::UpdateDeltaLights ()
{
return;
	bool found = FALSE;
	CDSegment *seg = Segments ();
	for (int segnum = 0; segnum < SegCount (); segnum++, seg++) {
		for (int sidenum = 0; sidenum < 6; sidenum++) {
			INT16 tmapnum2 = seg->sides [sidenum].nOvlTex & 0x1fff;
			if (IsLight(tmapnum2) != -1) {
				found = TRUE;
				break;
			}
		}
		if (found) break;
	}
	if (found) {
		if (QueryMsg("Would you like to update the delta light values?\n\n"
			"Note: These values are used for blinking, \n"
			"exploding, and trigger controlled lights.\n")== IDYES) {
			CalcDeltaLightData(1.0, 1);
		}
	}
}


// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
void CMine::CalcOrthoVector(vms_vector &result, INT16 segnum, INT16 sidenum)
{
	struct dvector a, b, c;
	double length;
	INT16 vertnum1, vertnum2;
    // calculate orthogonal vector from lines which intersect point 0
    //
    //       |x  y  z |
    // AxB = |ax ay az|= x(aybz - azby), y(azbx - axbz), z(axby - aybx)
    //       |bx by bz|

    vertnum1 =Segments (segnum)->verts [side_vert [sidenum] [0]];
    vertnum2 =Segments (segnum)->verts [side_vert [sidenum] [1]];
	 vms_vector *v1 = Vertices (vertnum1);
	 vms_vector *v2 = Vertices (vertnum2);
    a.x = (double)(v2->x - v1->x);
    a.y = (double)(v2->y - v1->y);
    a.z = (double)(v2->z - v1->z);
    vertnum1 =Segments (segnum)->verts [side_vert [sidenum] [0]];
    vertnum2 =Segments (segnum)->verts [side_vert [sidenum] [3]];
	 v1 = Vertices (vertnum1);
	 v2 = Vertices (vertnum2);
    b.x = (double)(v2->x - v1->x);
    b.y = (double)(v2->y - v1->y);
    b.z = (double)(v2->z - v1->z);

    c.x = a.y*b.z - a.z*b.y;
    c.y = a.z*b.x - a.x*b.z;
    c.z = a.x*b.y - a.y*b.x;

    // normalize the vector
    length = sqrt(c.x*c.x + c.y*c.y + c.z*c.z);
    if (length>0) {
		c.x /= length;
		c.y /= length;
		c.z /= length;
    }

    result.x = (long) dround_off (-c.x * 0x10000L, 1.0);
    result.y = (long) dround_off (-c.y * 0x10000L, 1.0);
    result.z = (long) dround_off (-c.z * 0x10000L, 1.0);
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
void CMine::CalcCenter(vms_vector &center, INT16 segnum, INT16 sidenum)
{
	int i;

	center.x = center.y = center.z = 0;
	vms_vector *v;
	CDSegment *seg = Segments (segnum);
	for (i = 0; i < 4; i++) {
		v = Vertices (seg->verts [side_vert [sidenum][i]]);
		center.x += (v->x) >> 2;
		center.y += (v->y) >> 2;
		center.z += (v->z) >> 2;
	}
}


//eof mine.cpp