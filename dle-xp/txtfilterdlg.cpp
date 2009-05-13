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
#include "texedit.h"
#include "io.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE [] = __FILE__;
#endif

#define TEXTOOLDLG 1

/////////////////////////////////////////////////////////////////////////////
// CToolView

BEGIN_MESSAGE_MAP(CTxtFilterTool, CToolDlg)
	ON_BN_CLICKED (IDC_TXT_GRAY_ROCK, OnGrayRock)
	ON_BN_CLICKED (IDC_TXT_BROWN_ROCK, OnBrownRock)
	ON_BN_CLICKED (IDC_TXT_RED_ROCK, OnRedRock)
	ON_BN_CLICKED (IDC_TXT_GREEN_ROCK, OnGreenRock)
	ON_BN_CLICKED (IDC_TXT_YELLOW_ROCK, OnYellowRock)
	ON_BN_CLICKED (IDC_TXT_BLUE_ROCK, OnBlueRock)
	ON_BN_CLICKED (IDC_TXT_ICE, OnIce)
	ON_BN_CLICKED (IDC_TXT_STONES, OnStones)
	ON_BN_CLICKED (IDC_TXT_GRASS, OnGrass)
	ON_BN_CLICKED (IDC_TXT_SAND, OnSand)
	ON_BN_CLICKED (IDC_TXT_LAVA, OnLava)
	ON_BN_CLICKED (IDC_TXT_WATER, OnWater)
	ON_BN_CLICKED (IDC_TXT_STEEL, OnSteel)
	ON_BN_CLICKED (IDC_TXT_CONCRETE, OnConcrete)
	ON_BN_CLICKED (IDC_TXT_BRICK, OnBricks)
	ON_BN_CLICKED (IDC_TXT_TARMAC, OnTarmac)
	ON_BN_CLICKED (IDC_TXT_WALL, OnWalls)
	ON_BN_CLICKED (IDC_TXT_FLOOR, OnFloors)
	ON_BN_CLICKED (IDC_TXT_CEILING, OnCeilings)
	ON_BN_CLICKED (IDC_TXT_GRATE, OnGrates)
	ON_BN_CLICKED (IDC_TXT_FAN, OnFans)
	ON_BN_CLICKED (IDC_TXT_LIGHT, OnLights)
	ON_BN_CLICKED (IDC_TXT_ENERGY, OnEnergy)
	ON_BN_CLICKED (IDC_TXT_FORCEFIELD, OnForcefields)
	ON_BN_CLICKED (IDC_TXT_SWITCH, OnSwitches)
	ON_BN_CLICKED (IDC_TXT_TECH, OnTech)
	ON_BN_CLICKED (IDC_TXT_DOOR, OnDoors)
	ON_BN_CLICKED (IDC_TXT_LABEL, OnLabels)
	ON_BN_CLICKED (IDC_TXT_MONITOR, OnMonitors)
	ON_BN_CLICKED (IDC_TXT_STRIPES, OnStripes)
	ON_BN_CLICKED (IDC_TXT_MOVE, OnMovers)

	ON_BN_CLICKED (IDC_TXT_ROCK_ALL, OnRockAll)
	ON_BN_CLICKED (IDC_TXT_ROCK_NONE, OnRockNone)
	ON_BN_CLICKED (IDC_TXT_NATURE_ALL, OnNatureAll)
	ON_BN_CLICKED (IDC_TXT_NATURE_NONE, OnNatureNone)
	ON_BN_CLICKED (IDC_TXT_BUILDING_ALL, OnBuildingAll)
	ON_BN_CLICKED (IDC_TXT_BUILDING_NONE, OnBuildingNone)
	ON_BN_CLICKED (IDC_TXT_OTHER_ALL, OnOtherAll)
	ON_BN_CLICKED (IDC_TXT_OTHER_NONE, OnOtherNone)
	ON_BN_CLICKED (IDC_TXT_TECH_ALL, OnTechAll)
	ON_BN_CLICKED (IDC_TXT_TECH_NONE, OnTechNone)
	ON_BN_CLICKED (IDC_TXT_SIGN_ALL, OnSignAll)
	ON_BN_CLICKED (IDC_TXT_SIGN_NONE, OnSignNone)
	ON_BN_CLICKED (IDC_TXT_ALL, OnTxtAll)
	ON_BN_CLICKED (IDC_TXT_NONE, OnTxtNone)
	ON_BN_CLICKED (IDC_TXT_INVERT, OnTxtInvert)
END_MESSAGE_MAP()

                        /*--------------------------*/

CTxtFilterTool::CTxtFilterTool (CPropertySheet *pParent)
	: CToolDlg (nLayout ? IDD_TXTFILTERS2 : IDD_TXTFILTERS, pParent)
{
}

                        /*--------------------------*/

CTxtFilterTool::~CTxtFilterTool ()
{
}

                        /*--------------------------*/

BOOL CTxtFilterTool::OnInitDialog ()
{
	CRect	rc;

if (!CToolDlg::OnInitDialog ())
   return FALSE;
m_bInited = TRUE;
return TRUE;
}

                        /*--------------------------*/

typedef struct tCtrlMap {
	int	nFlag, nIdC;
} tCtrlMap;

tCtrlMap ctrlMap [] = {
	{TXT_GRAY_ROCK, IDC_TXT_GRAY_ROCK},
	{TXT_BROWN_ROCK, IDC_TXT_BROWN_ROCK},
	{TXT_RED_ROCK, IDC_TXT_RED_ROCK},
	{TXT_GREEN_ROCK, IDC_TXT_GREEN_ROCK},
	{TXT_YELLOW_ROCK, IDC_TXT_YELLOW_ROCK},
	{TXT_BLUE_ROCK, IDC_TXT_BLUE_ROCK},
	{TXT_ICE, IDC_TXT_ICE},
	{TXT_STONES, IDC_TXT_STONES},
	{TXT_GRASS, IDC_TXT_GRASS},
	{TXT_SAND, IDC_TXT_SAND},
	{TXT_LAVA, IDC_TXT_LAVA},
	{TXT_WATER, IDC_TXT_WATER},
	{TXT_STEEL, IDC_TXT_STEEL},
	{TXT_CONCRETE, IDC_TXT_CONCRETE},
	{TXT_BRICK, IDC_TXT_BRICK},
	{TXT_TARMAC, IDC_TXT_TARMAC},
	{TXT_WALL, IDC_TXT_WALL},
	{TXT_FLOOR, IDC_TXT_FLOOR},
	{TXT_CEILING, IDC_TXT_CEILING},
	{TXT_GRATE, IDC_TXT_GRATE},
	{TXT_FAN, IDC_TXT_FAN},
	{TXT_LIGHT, IDC_TXT_LIGHT},
	{TXT_ENERGY, IDC_TXT_ENERGY},
	{TXT_FORCEFIELD, IDC_TXT_FORCEFIELD},
	{TXT_SWITCH, IDC_TXT_SWITCH},
	{TXT_TECH, IDC_TXT_TECH},
	{TXT_DOOR, IDC_TXT_DOOR},
	{TXT_LABEL, IDC_TXT_LABEL},
	{TXT_MONITOR, IDC_TXT_MONITOR},
	{TXT_STRIPES, IDC_TXT_STRIPES}
};

void CTxtFilterTool::DoDataExchange (CDataExchange *pDX)
{
	int		h, i;

for (i = 0; i < sizeof (ctrlMap) / sizeof (tCtrlMap); i++) {
	h = (theApp.TextureView ()->TxtFilter () & ctrlMap [i].nFlag) != 0;
	DDX_Check (pDX, ctrlMap [i].nIdC, h);
	if (h)
		theApp.TextureView ()->TxtFilter () |= ctrlMap [i].nFlag;
	else
		theApp.TextureView ()->TxtFilter () &= ~ctrlMap [i].nFlag;
	}
}

                        /*--------------------------*/

void CTxtFilterTool::SetFilter (int i)
{
if (theApp.TextureView ()->TxtFilter () & ctrlMap [i].nFlag)
	theApp.TextureView ()->TxtFilter () &= ~ctrlMap [i].nFlag;
else
	theApp.TextureView ()->TxtFilter () |= ctrlMap [i].nFlag;
UpdateData (FALSE);
theApp.TextureView ()->Refresh ();
}

                        /*--------------------------*/

void CTxtFilterTool::SetFilterRange (UINT32 nFlags, int nValue)
{
if (nValue == 1)
	theApp.TextureView ()->TxtFilter () |= nFlags;
else if (nValue == 0)
	theApp.TextureView ()->TxtFilter () &= ~nFlags;
else if (nValue == -1)
	theApp.TextureView ()->TxtFilter ()	^= nFlags;
UpdateData (FALSE);
theApp.TextureView ()->Refresh ();
}

                        /*--------------------------*/

void CTxtFilterTool::OnGrayRock () { SetFilter (0); }
void CTxtFilterTool::OnBrownRock () { SetFilter (1); }
void CTxtFilterTool::OnRedRock () { SetFilter (2); }
void CTxtFilterTool::OnGreenRock () { SetFilter (3); }
void CTxtFilterTool::OnYellowRock () { SetFilter (4); }
void CTxtFilterTool::OnBlueRock () { SetFilter (5); }
void CTxtFilterTool::OnIce () { SetFilter (6); }
void CTxtFilterTool::OnStones () { SetFilter (7); }
void CTxtFilterTool::OnGrass () { SetFilter (8); }
void CTxtFilterTool::OnSand () { SetFilter (9); }
void CTxtFilterTool::OnLava () { SetFilter (10); }
void CTxtFilterTool::OnWater () { SetFilter (11); }
void CTxtFilterTool::OnSteel () { SetFilter (12); }
void CTxtFilterTool::OnConcrete () { SetFilter (13); }
void CTxtFilterTool::OnBricks () { SetFilter (14); }
void CTxtFilterTool::OnTarmac () { SetFilter (15); }
void CTxtFilterTool::OnWalls () { SetFilter (16); }
void CTxtFilterTool::OnFloors () { SetFilter (17); }
void CTxtFilterTool::OnCeilings () { SetFilter (18); }
void CTxtFilterTool::OnGrates () { SetFilter (19); }
void CTxtFilterTool::OnFans () { SetFilter (20); }
void CTxtFilterTool::OnLights () { SetFilter (21); }
void CTxtFilterTool::OnEnergy () { SetFilter (22); }
void CTxtFilterTool::OnForcefields () { SetFilter (23); }
void CTxtFilterTool::OnSwitches () { SetFilter (24); }
void CTxtFilterTool::OnTech () { SetFilter (25); }
void CTxtFilterTool::OnDoors () { SetFilter (26); }
void CTxtFilterTool::OnLabels () { SetFilter (27); }
void CTxtFilterTool::OnMonitors () { SetFilter (28); }
void CTxtFilterTool::OnStripes () { SetFilter (29); }
void CTxtFilterTool::OnMovers () { SetFilterRange (TXT_LIGHT | TXT_STRIPES, -1); }
void CTxtFilterTool::OnRockAll () { SetFilterRange (TXT_ROCK, 1); }
void CTxtFilterTool::OnRockNone () { SetFilterRange (TXT_ROCK, 0); }
void CTxtFilterTool::OnNatureAll () { SetFilterRange (TXT_NATURE, 1); }
void CTxtFilterTool::OnNatureNone () { SetFilterRange (TXT_NATURE, 0); }
void CTxtFilterTool::OnBuildingAll () { SetFilterRange (TXT_BUILDING, 1); }
void CTxtFilterTool::OnBuildingNone () { SetFilterRange (TXT_BUILDING, 0); }
void CTxtFilterTool::OnTechAll () { SetFilterRange (TXT_TECHMAT, 1); }
void CTxtFilterTool::OnTechNone () { SetFilterRange (TXT_TECHMAT, 0); }
void CTxtFilterTool::OnOtherAll () { SetFilterRange (TXT_OTHER, 1); }
void CTxtFilterTool::OnOtherNone () { SetFilterRange (TXT_OTHER, 0); }
void CTxtFilterTool::OnSignAll () { SetFilterRange (TXT_SIGNS, 1); }
void CTxtFilterTool::OnSignNone () { SetFilterRange (TXT_SIGNS, 0); }
void CTxtFilterTool::OnTxtAll () { SetFilterRange (0xFFFFFFFF, 1); }
void CTxtFilterTool::OnTxtNone () { SetFilterRange (0xFFFFFFFF, 0); }
void CTxtFilterTool::OnTxtInvert () { SetFilterRange (0xFFFFFFFF, -1); }

                        /*--------------------------*/

BOOL CTxtFilterTool::OnKillActive ()
{
UpdateData (TRUE);
theApp.TextureView ()->Refresh ();
return CToolDlg::OnKillActive ();
}


                        /*--------------------------*/
		
		//eof texturedlg.cpp