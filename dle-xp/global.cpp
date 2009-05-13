//***************************************************************************
//
//  NAME:    GLOBAL.CPP
//  
//	PURPOSE:
//	
//	  BY:      Bryan Aamot
//	  
//		REV:
//		
//***************************************************************************

#include "stdafx.h"

#include "define.h"
#include "types.h"
#include "global.h"
#include "mine.h"

int changes_made = 0;
MISSION_DATA missionData;
//HGLOBAL texture_handle [MAX_D2_TEXTURES]; // MAX_D2_TEXTURES
CPalette *thePalette;
HINSTANCE hInst;
HGLOBAL hdelta_lights;
HGLOBAL hdl_indices;
HGLOBAL ScrnMem;
LPLOGPALETTE MyLogPalette;

char FileName[256]="";
char FilePath[256]="";
char SubFile[20]="";

bool bExpertMode = true;
bool bExtBlkFmt = false;

INT32 preferences = 0;
INT32 level_version=7;

INT16  N_flickering_lights = 0;
//FLICKERING_LIGHT *flickering_lights=0;
LIGHT_TIMER light_timers [MAX_FLICKERING_LIGHTS];
LIGHT_STATUS light_status [MAX_SEGMENTS3][MAX_SIDES_PER_SEGMENT];
UINT32       N_robot_types=0;
//ROBOT_INFO   *Robot_info;     // MAX_ROBOT_TYPES
#if ALLOCATE_POLYOBJECT
UINT32       N_robot_joints=0;
UINT32       N_polygon_models=0;
UINT32       N_object_bitmaps=0;
JOINTPOS     *Robot_joints;   // MAX_ROBOT_JOINTS
POLYMODEL    *Polygon_models[MAX_POLYGON_MODELS];
UINT32       *Dying_modelnums;// N_D2_POLYGON_MODELS
UINT32       *Dead_modelnums; // N_D2_POLYGON_MODELS
BITMAP_INDEX *ObjBitmaps;     // MAX_OBJ_BITMAPS
BITMAP_INDEX *ObjBitmapPtrs;  // MAX_OBJ_BITMAPS
#endif

int skip_about;
int disable_drawing=0;
char skip[16] = "tomÅ";
int left;
int top;
int right;
int bottom;
int x_center;
int y_center;
int x_max;
int y_max;
double aspect_ratio;
int refresh_needed = 0;
int splash = 0;


int level_modified = 0;
//UINT8 texture_modified[1+MAX_D2_TEXTURES/8];
int disable_saves = 1;

UINT8 clip_num[D2_NUM_OF_CLIPS] = {
	0,1,3,4,5,6,7,9,10,11,12,13,14,15,16,17,18,19,20,21,    // added 0 & 7 1/28/97
	22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,
	40,41,42,43,44,45,46,47,48,49,50
};

UINT8 wall_flags[8] = {
	WALL_BLASTED,
	WALL_DOOR_OPENED,
	WALL_DOOR_LOCKED,
	WALL_DOOR_AUTO,
	WALL_ILLUSION_OFF,
	WALL_WALL_SWITCH,
	WALL_BUDDY_PROOF,
	WALL_RENDER_ADDITIVE
};

int texture_resource = D2_TEXTURE_STRING_TABLE;

long ScrnMemMax = 0L;
int ScrnWidth = 0;
int ScrnHeight = 0;

// new data for Descent 2
int file_type = RL2_FILE;
//dl_index     *dl_indices=0;    // MAX_DL_INDICES   x 6 bytes each
//delta_light  *delta_lights=0;  // MAX_DELTA_LIGHTS x 8 bytes each
INT16	     num_static_lights=0;

char string[256];
INT16 spline_segment1,spline_segment2,spline_side1,spline_side2;
char spline_error_message[] = "You must exit spline creation before preforming this function";
vms_vector spline_points[MAX_SPLINES];
vms_vector points[4];
INT16 n_splines=0;
INT16 spline_length1=20,spline_length2=20;
INT16 spline_active=0;

UINT8 object_list[MAX_OBJECT_NUMBER] = {
	OBJ_ROBOT,
	OBJ_HOSTAGE,
	OBJ_PLAYER,
	OBJ_WEAPON,
	OBJ_POWERUP,
	OBJ_CNTRLCEN,
	OBJ_COOP,
	OBJ_CAMBOT,
	OBJ_MONSTERBALL,
	OBJ_SMOKE,
	OBJ_EXPLOSION,
	OBJ_EFFECT
};

UINT8 contains_list[MAX_CONTAINS_NUMBER] = {
	OBJ_ROBOT,
	OBJ_POWERUP
};

// the following array is used to select a list item by obj->type
INT8 object_selection[MAX_OBJECT_TYPES] = {
	-1,-1,0,1,2,3,-1,4,-1,5,-1,-1,-1,-1,6,-1,7,8,9,10,11
};
INT8 contains_selection[MAX_OBJECT_TYPES] = {
	-1,-1,0,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

UINT8 robot_clip[MAX_ROBOT_IDS_TOTAL] = {
	0x00,0x02,0x03,0x05,0x07,0x09,0x0b,0x0d,
	0x0e,0x10,0x12,0x13,0x14,0x15,0x16,0x18,
	0x19,0x1b,0x1c,0x1e,0x20,0x22,0x24,0x26,
	40,41,43,44,45,46,47,49,
	50,51,52,53,55,56,58,60,	// 50, 52, 53, and 86 were guessed at but seem to work ok
	62,64,65,67,68,69,70,71,
	72,73,74,75,76,77,78,80,
	82,83,85,86,87,88,89,90,
	91,92,
	0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,0xb0,0xb1  // vertigo clip numbers
};

// note1: 0 == not used,
// note2: 100 and 101 are flags but will appear as shields
//        in non multiplayer level
UINT8 powerup_clip[MAX_POWERUP_IDS2] = {
	36, 18, 19, 20, 24, 26, 25,  0,
		0,  0, 34, 35, 51, 37, 38, 39,
		40, 41, 42, 43, 44, 45, 46, 47,
		0, 49,  0,  0, 69, 70, 71, 72,
		77, 73, 74, 75, 76, 83, 78, 89,
		79, 90, 91, 81,102, 82,100,101
};

char *ai_options[MAX_D2_AI_OPTIONS] = {
	"Still", "Normal", "Get behind", "Drop Bombs", "Snipe", "Station", "Follow", "Static", "Smart Bombs"
};

FIX powerup_size[MAX_POWERUP_IDS2] = {
		0x28000L,0x30000L,0x28000L,0x40000L,0x30000L,0x30000L,0x30000L,0x30000L,
		0x30000L,0x30000L,0x28000L,0x30000L,0x30000L,0x40000L,0x40000L,0x40000L,
		0x40000L,0x30000L,0x28000L,0x30000L,0x28000L,0x30000L,0x1ccccL,0x20000L,
		0x30000L,0x29999L,0x30000L,0x30000L,0x40000L,0x40000L,0x40000L,0x40000L,
		0x40000L,0x40000L,0x40000L,0x40000L,0x48000L,0x30000L,0x28000L,0x28000L,
		0x30000L,0x30000L,0x40000L,0x40000L,0x40000L,0x40000L,0x38000L,0x38000L
};


FIX robot_size [MAX_ROBOT_IDS_TOTAL] = {
		399147L,368925L,454202L,316909L,328097L,345407L,399147L,293412L,
		300998L,308541L,246493L,283415L,283415L,227232L,200000L,598958L,
		399147L,1597221L,290318L,345407L,323879L,339488L,294037L,1443273L,
		378417L,408340L,586422L,302295L,524232L,405281L,736493L,892216L,
		400000L,204718L,400000L,400000L,354534L,236192L,373267L,351215L,
		429512L,169251L,310419L,378181L,381597L,1101683L,853047L,423359L,
		402717L,289744L,187426L,361065L,994374L,758384L,429512L,408340L,
		289744L,408340L,289744L,400000L,402717L,169251L,1312272L,169251L,
		905234L,1014749L,
		374114L,318151L,377386L,492146L,257003L,403683L,    // vertigo robots
		342424L,322628L,332831L,1217722L,907806L,378960L    // vertigo robots
};

FIX robot_shield [MAX_ROBOT_IDS_TOTAL] = {
	6553600L,6553600L,6553600L,1638400L,2293760L,6553600L,9830400L,16384000L,
		2293760L,16384000L,2293760L,2293760L,2000000L,9830400L,1310720L,26214400L,
		21299200L,131072000L,6553600L,3276800L,3276800L,4587520L,4587520L,327680000L,
		5570560L,5242880L,9830400L,2949120L,6553600L,6553600L,7864320L,196608000L,
		5000000L,45875200L,5000000L,5000000L,5242880L,786432L,1966080L,4587520L,
		9830400L,1310720L,29491200L,9830400L,11796480L,262144000L,262144000L,13107200L,
		7208960L,655360L,983040L,11141120L,294912000L,32768000L,7864320L,3932160L,
		4587520L,5242880L,4587520L,5000000L,7208960L,1310720L,196608000L,1310720L,
		294912000L,19660800L,
		6553600L,6553600L,6553600L,10485760L,4587520L,16384000L,  // vertigo robots
		6553600L,7864320L,7864320L,180224000L,360448000L,9830400L // vertigo robots
};

TEXTURE_LIGHT d1_texture_light[NUM_LIGHTS_D1] = {
	{250, 0x00b333L}, {251, 0x008000L}, {252, 0x008000L}, {253, 0x008000L},
	{264, 0x01547aL}, {265, 0x014666L}, {268, 0x014666L}, {278, 0x014cccL},
	{279, 0x014cccL}, {280, 0x011999L}, {281, 0x014666L}, {282, 0x011999L},
	{283, 0x0107aeL}, {284, 0x0107aeL}, {285, 0x011999L}, {286, 0x014666L},
	{287, 0x014666L}, {288, 0x014666L}, {289, 0x014666L}, {292, 0x010cccL},
	{293, 0x010000L}, {294, 0x013333L}, {330, 0x010000L}, {333, 0x010000L}, 
	{341, 0x010000L}, {343, 0x010000L}, {345, 0x010000L}, {347, 0x010000L}, 
	{349, 0x010000L}, {351, 0x010000L}, {352, 0x010000L}, {354, 0x010000L}, 
	{355, 0x010000L}, {356, 0x020000L}, {357, 0x020000L}, {358, 0x020000L}, 
	{359, 0x020000L}, {360, 0x020000L}, {361, 0x020000L}, {362, 0x020000L}, 
	{363, 0x020000L}, {364, 0x020000L}, {365, 0x020000L}, {366, 0x020000L}, 
	{367, 0x020000L}, {368, 0x020000L}, {369, 0x020000L}, {370, 0x020000L}
};

TEXTURE_LIGHT d2_texture_light[NUM_LIGHTS_D2] = {
	{235, 0x012666L}, {236, 0x00b5c2L}, {237, 0x00b5c2L}, {243, 0x00b5c2L},
	{244, 0x00b5c2L}, {275, 0x01547aL}, {276, 0x014666L}, {278, 0x014666L},
	{288, 0x014cccL}, {289, 0x014cccL}, {290, 0x011999L}, {291, 0x014666L},
	{293, 0x011999L}, {295, 0x0107aeL}, {296, 0x011999L}, {298, 0x014666L},
	{300, 0x014666L}, {301, 0x014666L}, {302, 0x014666L}, {305, 0x010cccL},
	{306, 0x010000L}, {307, 0x013333L}, {340, 0x00b333L}, {341, 0x00b333L},
	{343, 0x004cccL}, {344, 0x003333L}, {345, 0x00b333L}, {346, 0x004cccL},
	{348, 0x003333L}, {349, 0x003333L}, {353, 0x011333L}, {356, 0x00028fL},
	{357, 0x00028fL}, {358, 0x00028fL}, {359, 0x00028fL}, {364, 0x010000L},
	{366, 0x010000L}, {368, 0x010000L}, {370, 0x010000L}, {372, 0x010000L},
	{374, 0x010000L}, {375, 0x010000L}, {377, 0x010000L}, {378, 0x010000L},
	{380, 0x010000L}, {382, 0x010000L}, {383, 0x020000L}, {384, 0x020000L},
	{385, 0x020000L}, {386, 0x020000L}, {387, 0x020000L}, {388, 0x020000L},
	{389, 0x020000L}, {390, 0x020000L}, {391, 0x020000L}, {392, 0x020000L},
	{393, 0x020000L}, {394, 0x020000L}, {395, 0x020000L}, {396, 0x020000L},
	{397, 0x020000L}, {398, 0x020000L}, {404, 0x010000L}, {405, 0x010000L},
	{406, 0x010000L}, {407, 0x010000L}, {408, 0x010000L}, {409, 0x020000L},
	{410, 0x008000L}, {411, 0x008000L}, {412, 0x008000L}, {419, 0x020000L},
	{420, 0x020000L}, {423, 0x010000L}, {424, 0x010000L}, {425, 0x020000L},
	{426, 0x020000L}, {427, 0x008000L}, {428, 0x008000L}, {429, 0x008000L},
	{430, 0x020000L}, {431, 0x020000L}, {432, 0x00e000L}, {433, 0x020000L},
	{434, 0x020000L}
};

INT16 d2_blastable_lights [] = {
	276, 278, 360, 361, 364, 366, 368,
	370, 372, 374, 375, 377, 380, 382, 
	420, 432,
	431,  -1
	};

INT16 d2_switches [] = {414, 416, 418, -1};

INT16 add_segment_mode = ORTHOGONAL;
char dlc_version[10] = "2.7a";
//char testers_name[20] = "TESTERS_NAME_HERE";
INT16 serial_number   = 27;
char message[256];
//UINT8 test4 = 0xBA;
char starting_directory[256];
//UINT8 test5 = 0xBA;
FIX grid = 0x2000L;   // 0.125
char descent_path[256]="c:\\";
//UINT8 test6 = 0xBA;
char descent2_path[256]="c:\\";
//UINT8 test7 = 0xBA;
char levels_path[256]="c:\\";
//UINT8 test8 = 0xBA;
char player_profile [20] = "";

INT16 trignum = 0;
//INT16 number_of_textures=0;
//INT16  show_lines      = SHOW_LINES_PARTIAL;
#if 0
INT16  show_lines      = SHOW_FILLED_POLYGONS;
INT16  show_walls      = 1;
INT16  show_special    = 1;
INT16  show_lights     = 1;
INT16  show_objects    = (1<<OBJ_ROBOT)  + (1<<OBJ_HOSTAGE) + (1<<OBJ_POWERUP)
+ (1<<OBJ_PLAYER) + (1<<OBJ_COOP) +(1<<OBJ_CNTRLCEN);
INT16  show_powerups   =  POWERUP_POWERUP_MASK + POWERUP_WEAPON_MASK + POWERUP_KEY_MASK;
#endif
//CUBE  current1 = {DEFAULT_SEGMENT,
//DEFAULT_SIDE,
//DEFAULT_LINE,
//DEFAULT_POINT,
//DEFAULT_OBJECT};
//CUBE  current2 = {DEFAULT_SEGMENT,
//DEFAULT_SIDE,
//DEFAULT_LINE,
//DEFAULT_POINT,
//DEFAULT_OBJECT};
//CUBE  *current = &current1;
#if 0
INT16 select_mode     = CUBE_MODE;
INT16 edit_mode       = EDIT_OFF;
#endif
uvl default_uvls[4] = {
	{(UINT16) 0x0000, (UINT16)0x0000, (UINT16)DEFAULT_LIGHTING},
	{(UINT16) 0x0000, (UINT16)0x0800, (UINT16)DEFAULT_LIGHTING},
	{(UINT16)-0x0800, (UINT16)0x0800, (UINT16)DEFAULT_LIGHTING},
	{(UINT16)-0x0800, (UINT16)0x0000, (UINT16)DEFAULT_LIGHTING}
};

char *object_names[MAX_OBJECT_TYPES] = {
	"Wall",
	"Fireball",
	"Robot",
	"Hostage",
	"Player",
	"Mine",
	"Camera",
	"Power Up",
	"Debris",
	"Reactor",
	"Flare",
	"Clutter",
	"Ghost",
	"Light",
	"CoOp",
	"Marker",
	"Camera",
	"Monsterball",
	"Smoke",
	"Explosion",
	"Effect"
};

UINT8 powerup_types[MAX_POWERUP_IDS2] = {
		POWERUP_POWERUP_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_KEY_MASK,
		POWERUP_KEY_MASK,
		POWERUP_KEY_MASK,
		POWERUP_UNKNOWN_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_UNKNOWN_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_UNKNOWN_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_UNKNOWN_MASK,
		POWERUP_UNKNOWN_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_POWERUP_MASK
};

char wall_type_names[6][10] = {
	"Normal",
		"Blastable",
		"Door",
		"Illusion",
		"Open",
		"Closed"
};

char wall_flag_names[8][14] = {
	"Blasted",
		"Door Closed",
		"(not used)",
		"Door Locked",
		"Door Auto",
		"Illusion Off",
		"(not used)",
		"(not used)"
};

char wall_state_names[4][8] = {
	"Closed",
		"Opening",
		"Open",
		"Closing"
};


APOINT *scrn;
INT16 gx0,gy0,gz0;
double spinx,spiny,spinz;
double movex,movey,movez;
INT16 max_x,min_x,max_y,min_y,max_z,min_z;
double sizex,sizey,sizez;
double angle_rate = (double)PI / 16.f;
FIX   move_rate = 0x10000L;
double depth_perception = 1000.0;

/* define points for a given side */
UINT8 side_vert[6][4] = {
	{7,6,2,3},
	{0,4,7,3},
	{0,1,5,4},
	{2,6,5,1},
	{4,5,6,7},
	{3,2,1,0} 
};

/* define oppisite side of a given side */
UINT8 opp_side[6] = {2,3,0,1,5,4};

/* define points for the oppisite side of a given side */
UINT8 opp_side_vert[6][4] = {
	{4,5,1,0},
	{1,5,6,2},
	{3,2,6,7},
	{3,7,4,0},
	{0,1,2,3},
	{7,6,5,4} 
};

/* define 2 points for a given line */
UINT8 line_vert[12][2] = {
	{0,1},
	{1,2},
	{2,3},
	{3,0},
	{0,4},
	{4,5},
	{5,6},
	{6,7},
	{7,4},
	{1,5},
	{2,6},
	{3,7}
};

/* define lines for a given side */
UINT8 side_line[6][4] = {
	{7,10,2,11},//{2,11,7,10},
	{4,8,11,3},//{3,11,8,4},
	{0,9,5,4},//{0,9,5,4},
	{10,6,9,1},//{1,10,6,9},
	{5,6,7,8},//{5,6,7,8},
	{2,1,0,3} //{0,1,2,3}
};

/* define 3 points which connect to a certain point */
UINT8 connect_points[8][3] = {
	{1,3,4},
	{2,0,5},
	{3,1,6},
	{0,2,7},
	{7,5,0},
	{4,6,1},
	{5,7,2},
	{6,4,3}
};

// side numbers for each point (3 sides touch each point)
INT8 point_sides[8][3] = {
    {1,2,5},
    {2,3,5},
    {0,3,5},
    {0,1,5},
    {1,2,4},
    {2,3,4},
    {0,3,4},
    {0,1,4}
};

// uvl corner for point_sides above
INT8 point_corners[8][3] = {
    {0,0,3},
    {1,3,2},
    {2,0,1},
    {3,3,0},
    {1,3,0},
    {2,2,1},
    {1,1,2},
    {0,2,3} 
};

                        /*--------------------------*/

UINT8 clip_door_number [D2_NUM_OF_CLIPS] = {
	1, 1, 4, 5, 10,24, 8,11,13,12,14,17,     // 1/28/97
	18,19,20,21,22,23,25,26,28,29,
	30,31,32,33,34,35,36,37,38,39,
	40,41,42,43,44,45,47,48,49,50,
	51,52,53,54,55,56,57
	};

int renderXOffs = 0;
int renderYOffs = 0;

tTxtFilter txtFiltersD1 [D1_TXT_FILTER_SIZE] = {
	{{0, 5}, TXT_GRAY_ROCK, 0},
	{{6, 6}, TXT_BLUE_ROCK, 0},
	{{7, 7}, TXT_YELLOW_ROCK, 0},
	{{8, 8}, TXT_GRAY_ROCK, 0},
	{{9, 10}, TXT_RED_ROCK, 0},
	{{11, 11}, TXT_BLUE_ROCK, 0},
	{{12, 12}, TXT_GRAY_ROCK, TXT_STONES},
	{{13, 13}, TXT_GRAY_ROCK, 0},
	{{14, 14}, TXT_TARMAC, 0},
	{{15, 15}, TXT_GRAY_ROCK, 0},
	{{16, 16}, TXT_BLUE_ROCK, 0},
	{{17, 17}, TXT_RED_ROCK, 0},
	{{18, 18}, TXT_BLUE_ROCK, TXT_GRAY_ROCK},
	{{19, 19}, TXT_RED_ROCK, 0},
	{{20, 21}, TXT_STEEL, 0},
	{{22, 30}, TXT_RED_ROCK, 0},
	{{31, 31}, TXT_BROWN_ROCK, 0},
	{{32, 32}, TXT_RED_ROCK, TXT_STONES},
	{{33, 38}, TXT_RED_ROCK, 0},
	{{39, 39}, TXT_BRICK, 0},
	{{40, 40}, TXT_GRAY_ROCK, 0},
	{{41, 41}, TXT_FLOOR, 0},
	{{42, 42}, TXT_GRAY_ROCK, 0},
	{{43, 43}, TXT_RED_ROCK, 0},
	{{44, 44}, TXT_BLUE_ROCK, 0},
	{{45, 45}, TXT_RED_ROCK, TXT_LAVA},
	{{46, 47}, TXT_RED_ROCK, 0},
	{{48, 49}, TXT_GRAY_ROCK, 0},
	{{50, 50}, TXT_RED_ROCK, 0},
	{{51, 53}, TXT_BROWN_ROCK, 0},
	{{54, 55}, TXT_RED_ROCK, 0},
	{{56, 56}, TXT_BROWN_ROCK, 0},
	{{57, 57}, TXT_RED_ROCK, 0},
	{{58, 58}, TXT_GRAY_ROCK, TXT_STONES},
	{{59, 59}, TXT_ICE, 0},
	{{60, 60}, TXT_BROWN_ROCK, TXT_STONES},
	{{61, 61}, TXT_GRAY_ROCK, 0},
	{{62, 66}, TXT_BROWN_ROCK, 0},
	{{67, 69}, TXT_GRAY_ROCK, 0},
	{{70, 73}, TXT_GREEN_ROCK, 0},
	{{74, 75}, TXT_RED_ROCK, 0},
	{{76, 77}, TXT_BLUE_ROCK, 0},
	{{78, 78}, TXT_BROWN_ROCK, 0},
	{{79, 79}, TXT_BLUE_ROCK, TXT_ICE},
	{{80, 81}, TXT_BROWN_ROCK, 0},
	{{82, 82}, TXT_GRAY_ROCK, 0},
	{{83, 83}, TXT_BLUE_ROCK, 0},
	{{84, 85}, TXT_GRAY_ROCK, 0},
	{{86, 87}, TXT_BLUE_ROCK, 0},
	{{88, 89}, TXT_BROWN_ROCK, 0},
	{{90, 90}, TXT_BLUE_ROCK, 0},
	{{91, 91}, TXT_BROWN_ROCK, 0},
	{{92, 96}, TXT_RED_ROCK, 0},
	{{97, 97}, TXT_BROWN_ROCK, 0},
	{{98, 98}, TXT_GREEN_ROCK, 0},
	{{99, 99}, TXT_BROWN_ROCK, 0},
	{{100, 100}, TXT_GRAY_ROCK, 0},
	{{101, 105}, TXT_BROWN_ROCK, 0},
	{{106, 109}, TXT_GREEN_ROCK, 0},
	{{110, 110}, TXT_BLUE_ROCK, TXT_ICE},
	{{111, 116}, TXT_GREEN_ROCK, TXT_GRASS},
	{{117, 118}, TXT_BROWN_ROCK, 0},
	{{119, 120}, TXT_CONCRETE, 0},
	{{121, 122}, TXT_BROWN_ROCK, 0},
	{{123, 123}, TXT_GREEN_ROCK, TXT_GRASS},
	{{124, 125}, TXT_BROWN_ROCK, 0},
	{{126, 126}, TXT_BROWN_ROCK, TXT_BLUE_ROCK},
	{{127, 127}, TXT_BLUE_ROCK, 0},
	{{128, 141}, TXT_RED_ROCK, 0},
	{{142, 143}, TXT_RED_ROCK, TXT_LAVA},
	{{144, 144}, TXT_BLUE_ROCK, TXT_ICE},
	{{145, 145}, TXT_GREEN_ROCK, 0},
	{{146, 146}, TXT_BLUE_ROCK, TXT_ICE},
	{{147, 149}, TXT_BROWN_ROCK, 0},
	{{150, 151}, TXT_RED_ROCK, 0},
	{{152, 153}, TXT_GREEN_ROCK, TXT_STONES},
	{{154, 154}, TXT_GRAY_ROCK, TXT_LAVA},
	{{155, 155}, TXT_BROWN_ROCK, TXT_LAVA},
	{{156, 157}, TXT_STEEL, 0},
	{{158, 158}, TXT_CONCRETE, TXT_WALL},
	{{159, 159}, TXT_TECH, TXT_WALL},
	{{160, 160}, TXT_CONCRETE, TXT_WALL},
	{{161, 161}, TXT_TECH, TXT_WALL},
	{{162, 163}, TXT_CONCRETE, TXT_WALL},
	{{164, 164}, TXT_CONCRETE, TXT_SIGN | TXT_WALL},
	{{165, 171}, TXT_CONCRETE, TXT_WALL},
	{{172, 173}, TXT_CONCRETE, TXT_SIGN | TXT_WALL},
	{{174, 185}, TXT_CONCRETE, TXT_WALL},
	{{186, 190}, TXT_STEEL, 0},
	{{191, 196}, TXT_CONCRETE, TXT_WALL},
	{{197, 197}, TXT_STEEL, 0},
	{{198, 199}, TXT_CONCRETE, TXT_WALL},
	{{200, 200}, TXT_CONCRETE, TXT_WALL | TXT_GRATE},
	{{201, 207}, TXT_CONCRETE, TXT_WALL},
	{{208, 208}, TXT_CONCRETE, TXT_WALL | TXT_SIGN},
	{{209, 211}, TXT_CONCRETE, TXT_WALL},
	{{212, 214}, TXT_CONCRETE, TXT_WALL | TXT_LIGHT},
	{{215, 217}, TXT_CONCRETE, TXT_WALL},
	{{218, 218}, TXT_CONCRETE, TXT_WALL | TXT_GRATE},
	{{219, 219}, TXT_CONCRETE, TXT_WALL},
	{{220, 220}, TXT_CONCRETE, TXT_WALL | TXT_GRATE},
	{{221, 222}, TXT_CONCRETE, TXT_WALL | TXT_LIGHT},
	{{223, 230}, TXT_TECH, 0},
	{{231, 234}, TXT_CONCRETE, TXT_WALL},
	{{235, 237}, TXT_CONCRETE, TXT_WALL | TXT_GRATE},
	{{238, 238}, TXT_CONCRETE, TXT_WALL},
	{{239, 239}, TXT_STEEL, TXT_GRATE},
	{{240, 243}, TXT_TECH, 0},
	{{244, 244}, TXT_CONCRETE, TXT_WALL},
	{{245, 245}, TXT_GRATE, TXT_STEEL},
	{{246, 246}, TXT_GRATE, 0},
	{{247, 249}, TXT_GRATE, TXT_STEEL},
	{{250, 253}, TXT_LIGHT, 0},
	{{254, 255}, TXT_GRATE, 0},
	{{256, 260}, TXT_CONCRETE, TXT_WALL},
	{{261, 262}, TXT_CEILING, TXT_STEEL},
	{{263, 263}, TXT_FLOOR, TXT_STEEL},
	{{264, 269}, TXT_LIGHT, 0},
	{{270, 277}, TXT_FLOOR, 0},
	{{278, 289}, TXT_LIGHT, 0},
	{{290, 291}, TXT_FLOOR, 0},
	{{292, 294}, TXT_LIGHT, 0},
	{{295, 296}, TXT_GRATE, TXT_TECH},
	{{297, 297}, TXT_GRATE, 0},
	{{298, 298}, TXT_SIGN, 0},
	{{299, 300}, TXT_TECH, TXT_SIGN},
	{{301, 301}, TXT_ENERGY, 0},
	{{302, 308}, TXT_SIGN, TXT_LABEL},
	{{309, 309}, TXT_GRATE, 0},
	{{310, 312}, TXT_SIGN, TXT_LABEL},
	{{313, 314}, TXT_SIGN, TXT_STRIPES},
	{{315, 315}, TXT_TECH, 0},
	{{316, 316}, TXT_RED_ROCK, TXT_TECH},
	{{317, 317}, TXT_SIGN, TXT_LABEL},
	{{318, 318}, TXT_SIGN, TXT_STRIPES},
	{{319, 321}, TXT_SIGN, TXT_LABEL},
	{{322, 322}, TXT_ENERGY, 0},
	{{323, 323}, TXT_SIGN, TXT_LABEL},
	{{324, 324}, TXT_GRATE, 0},
	{{325, 325}, TXT_FAN, 0},
	{{326, 326}, TXT_TECH, TXT_SIGN},
	{{327, 327}, TXT_SIGN, TXT_LIGHT | TXT_MOVE},
	{{328, 328}, TXT_ENERGY, TXT_LIGHT},
	{{329, 329}, TXT_FAN, 0},
	{{330, 331}, TXT_SIGN, TXT_LIGHT | TXT_MOVE},
	{{332, 332}, TXT_GREEN_ROCK, TXT_TECH},
	{{333, 333}, TXT_RED_ROCK, TXT_TECH},
	{{334, 337}, TXT_CONCRETE, TXT_TECH},
	{{338, 339}, TXT_TECH, 0},
	{{340, 340}, TXT_TARMAC, 0},
	{{341, 354}, TXT_SIGN, TXT_LIGHT | TXT_MONITOR},
	{{355, 356}, TXT_LAVA, TXT_LIGHT},
	{{357, 370}, TXT_SIGN, TXT_LIGHT | TXT_MONITOR},
	{{371, 577}, TXT_DOOR, 0}
};

tTxtFilter txtFiltersD2 [D2_TXT_FILTER_SIZE] = {
	{{0, 0}, TXT_GRAY_ROCK, TXT_CONCRETE},
	{{1, 5}, TXT_GRAY_ROCK, 0},
	{{6, 6}, TXT_BLUE_ROCK, 0},
	{{7, 7}, TXT_RED_ROCK, 0},
	{{8, 14}, TXT_GRAY_ROCK, 0},
	{{15, 15}, TXT_BROWN_ROCK, 0},
	{{16, 16}, TXT_RED_ROCK, TXT_STONES},
	{{17, 21}, TXT_RED_ROCK, 0},
	{{22, 23}, TXT_GRAY_ROCK, 0},
	{{24, 24}, TXT_RED_ROCK, 0},
	{{25, 25}, TXT_RED_ROCK, TXT_LAVA},
	{{26, 27}, TXT_RED_ROCK, 0},
	{{28, 28}, TXT_GRAY_ROCK, 0},
	{{29, 31}, TXT_BROWN_ROCK, 0},
	{{32, 32}, TXT_RED_ROCK, 0},
	{{33, 33}, TXT_BROWN_ROCK, 0},
	{{34, 34}, TXT_GRAY_ROCK, TXT_STONES},
	{{35, 35}, TXT_ICE, 0},
	{{36, 36}, TXT_BROWN_ROCK, TXT_STONES},
	{{37, 37}, TXT_GRAY_ROCK, 0},
	{{38, 42}, TXT_BROWN_ROCK, 0},
	{{43, 43}, TXT_GRAY_ROCK, 0},
	{{44, 44}, TXT_RED_ROCK, 0},
	{{45, 45}, TXT_TARMAC, 0},
	{{46, 49}, TXT_GREEN_ROCK, 0},
	{{50, 51}, TXT_RED_ROCK, 0},
	{{52, 53}, TXT_BLUE_ROCK, 0},
	{{54, 54}, TXT_BROWN_ROCK, 0},
	{{55, 55}, TXT_BLUE_ROCK, TXT_ICE},
	{{56, 58}, TXT_BROWN_ROCK, 0},
	{{59, 59}, TXT_BLUE_ROCK, 0},
	{{60, 61}, TXT_GRAY_ROCK, 0},
	{{62, 63}, TXT_BLUE_ROCK, 0},
	{{64, 64}, TXT_BROWN_ROCK, 0},
	{{65, 65}, TXT_BLUE_ROCK, 0},
	{{66, 66}, TXT_BROWN_ROCK, 0},
	{{67, 71}, TXT_RED_ROCK, 0},
	{{72, 72}, TXT_BROWN_ROCK, 0},
	{{73, 73}, TXT_GREEN_ROCK, 0},
	{{74, 74}, TXT_BROWN_ROCK, 0},
	{{75, 75}, TXT_GRAY_ROCK, 0},
	{{76, 80}, TXT_BROWN_ROCK, 0},
	{{81, 81}, TXT_GREEN_ROCK, TXT_ICE},
	{{82, 82}, TXT_GREEN_ROCK, 0},
	{{83, 83}, TXT_GREEN_ROCK, TXT_ICE},
	{{84, 84}, TXT_GREEN_ROCK, 0},
	{{85, 85}, TXT_GREEN_ROCK, TXT_ICE},
	{{85, 85}, TXT_GREEN_ROCK | TXT_BLUE_ROCK, TXT_ICE},
	{{86, 91}, TXT_GREEN_ROCK, TXT_GRASS},
	{{92, 93}, TXT_BROWN_ROCK, 0},
	{{94, 95}, TXT_CONCRETE, 0},
	{{96, 97}, TXT_BROWN_ROCK, 0},
	{{98, 98}, TXT_GREEN_ROCK, TXT_GRASS},
	{{99, 100}, TXT_BROWN_ROCK, TXT_RED_ROCK},
	{{101, 101}, TXT_BROWN_ROCK | TXT_BLUE_ROCK, TXT_ICE},
	{{102, 102}, TXT_BLUE_ROCK, 0},
	{{103, 103}, TXT_BROWN_ROCK, 0},
	{{104, 105}, TXT_RED_ROCK, TXT_LAVA},
	{{106, 106}, TXT_RED_ROCK, 0},
	{{107, 111}, TXT_BROWN_ROCK, 0},
	{{112, 114}, TXT_RED_ROCK, 0},
	{{115, 116}, TXT_RED_ROCK, TXT_LAVA},
	{{117, 117}, TXT_BLUE_ROCK, TXT_ICE},
	{{118, 118}, TXT_GREEN_ROCK, 0},
	{{119, 119}, TXT_BLUE_ROCK, 0},
	{{120, 121}, TXT_BROWN_ROCK, 0},
	{{122, 123}, TXT_BROWN_ROCK | TXT_RED_ROCK, 0},
	{{124, 125}, TXT_GREEN_ROCK, TXT_GRASS},
	{{126, 127}, TXT_RED_ROCK, 0},
	{{128, 138}, TXT_ICE, 0},
	{{139, 139}, TXT_ICE, 0},
	{{140, 142}, TXT_BROWN_ROCK, 0},
	{{143, 145}, TXT_SAND, 0},
	{{146, 147}, TXT_RED_ROCK, 0},
	{{148, 148}, TXT_BROWN_ROCK, 0},
	{{149, 151}, TXT_BROWN_ROCK, TXT_SAND},
	{{152, 152}, TXT_RED_ROCK, 0},
	{{153, 154}, TXT_BROWN_ROCK, TXT_SAND},
	{{155, 168}, TXT_RED_ROCK, 0},
	{{169, 171}, TXT_GREEN_ROCK, TXT_GRASS},
	{{172, 172}, TXT_BLUE_ROCK, TXT_ICE},
	{{173, 176}, TXT_ICE, 0},
	{{177, 177}, TXT_BROWN_ROCK},
	{{178, 183}, TXT_ICE},
	{{184, 184}, TXT_GREEN_ROCK | TXT_GRASS},
	{{185, 190}, TXT_ICE, 0},
	{{191, 191}, TXT_CONCRETE, 0},
	{{192, 193}, TXT_ICE, 0},
	{{194, 194}, TXT_GREEN_ROCK, TXT_ICE},
	{{195, 195}, TXT_ICE, 0},
	{{196, 196}, TXT_GREEN_ROCK, TXT_ICE},
	{{197, 197}, TXT_BROWN_ROCK, 0},
	{{198, 198}, TXT_GRAY_ROCK, 0},
	{{199, 199}, TXT_BROWN_ROCK, 0},
	{{200, 201}, TXT_STEEL, 0},
	{{202, 217}, TXT_CONCRETE, TXT_WALL},
	{{204, 204}, TXT_CONCRETE, TXT_SIGN | TXT_WALL},
	{{205, 206}, TXT_CONCRETE, TXT_WALL},
	{{207, 208}, TXT_CONCRETE, TXT_SIGN | TXT_WALL},
	{{209, 217}, TXT_CONCRETE, TXT_WALL},
	{{218, 222}, TXT_STEEL, 0},
	{{223, 225}, TXT_CONCRETE, TXT_WALL},
	{{226, 226}, TXT_STEEL, 0},
	{{227, 232}, TXT_CONCRETE, TXT_WALL},
	{{233, 233}, TXT_CONCRETE, TXT_SIGN | TXT_WALL},
	{{234, 234}, TXT_CONCRETE, TXT_WALL},
	{{235, 237}, TXT_CONCRETE, TXT_LIGHT | TXT_WALL},
	{{238, 240}, TXT_CONCRETE, TXT_WALL},
	{{241, 241}, TXT_CONCRETE, TXT_GRATE | TXT_WALL},
	{{242, 242}, TXT_CONCRETE, TXT_WALL},
	{{243, 244}, TXT_CONCRETE, TXT_LIGHT | TXT_WALL},
	{{245, 246}, TXT_TECH, 0},
	{{247, 248}, TXT_GRATE, TXT_STEEL},
	{{249, 252}, TXT_CONCRETE, TXT_WALL},
	{{253, 255}, TXT_CONCRETE, TXT_GRATE | TXT_WALL},
	{{256, 256}, TXT_CONCRETE, TXT_WALL},
	{{257, 257}, TXT_GRATE, TXT_STEEL},
	{{258, 258}, TXT_CONCRETE, TXT_WALL},
	{{259, 262}, TXT_GRATE, TXT_STEEL},
	{{263, 264}, TXT_GRATE, 0},
	{{265, 265}, TXT_CONCRETE, 0},
	{{266, 266}, TXT_FLOOR, 0},
	{{267, 269}, TXT_GRATE, 0},
	{{270, 272}, TXT_TECH, 0},
	{{273, 274}, TXT_STEEL, TXT_CEILING},
	{{275, 279}, TXT_LIGHT, 0},
	{{280, 287}, TXT_FLOOR, 0},
	{{288, 302}, TXT_LIGHT, 0},
	{{303, 304}, TXT_FLOOR, 0},
	{{305, 307}, TXT_LIGHT, 0},
	{{308, 310}, TXT_GRATE, 0},
	{{311, 312}, TXT_SIGN, TXT_TECH | TXT_STRIPES},
	{{313, 313}, TXT_ENERGY, 0},
	{{314, 320}, TXT_SIGN, TXT_LABEL},
	{{321, 321}, TXT_GRATE, 0},
	{{322, 324}, TXT_SIGN, TXT_LABEL},
	{{325, 326}, TXT_SIGN, TXT_STRIPES},
	{{327, 327}, TXT_RED_ROCK, TXT_TECH},
	{{328, 328}, TXT_SIGN, TXT_LABEL},
	{{329, 329}, TXT_SIGN, TXT_STRIPES},
	{{330, 332}, TXT_SIGN, TXT_LABEL},
	{{333, 333}, TXT_ENERGY, 0},
	{{334, 334}, TXT_SIGN, TXT_LABEL},
	{{335, 335}, TXT_GRATE, 0},
	{{336, 336}, TXT_FAN, 0},
	{{337, 337}, TXT_SIGN, TXT_TECH | TXT_STRIPES},
	{{338, 339}, TXT_SIGN, TXT_STRIPES},
	{{340, 347}, TXT_LIGHT, 0},
	{{348, 349}, TXT_LIGHT, TXT_STEEL | TXT_MOVE},
	{{350, 350}, TXT_GRATE, TXT_STEEL | TXT_MOVE},
	{{351, 351}, TXT_SIGN, TXT_LABEL},
	{{352, 352}, TXT_SIGN, TXT_LIGHT | TXT_MOVE},
	{{353, 353}, TXT_ENERGY, TXT_LIGHT},
	{{354, 354}, TXT_FAN, 0},
	{{355, 355}, TXT_GREEN_ROCK, TXT_TECH},
	{{356, 359}, TXT_CONCRETE , TXT_TECH | TXT_WALL | TXT_LIGHT},
	{{360, 361}, TXT_LIGHT, TXT_TECH},
	{{362, 362}, TXT_TARMAC, 0},
	{{363, 377}, TXT_SIGN, TXT_LIGHT | TXT_MONITOR},
	{{378, 378}, TXT_LAVA, 0},
	{{379, 398}, TXT_SIGN, TXT_LIGHT},
	{{399, 403}, TXT_WATER, 0},
	{{404, 409}, TXT_LAVA, 0},
	{{410, 412}, TXT_LIGHT, 0},
	{{413, 419}, TXT_SWITCH, 0},
	{{420, 420}, TXT_FORCEFIELD, TXT_LIGHT},
	{{423, 424}, TXT_SIGN, TXT_LIGHT | TXT_STRIPES},
	{{425, 425}, TXT_SIGN, TXT_LIGHT},
	{{426, 426}, TXT_LIGHT, TXT_TECH},
	{{427, 429}, TXT_LIGHT, 0},
	{{430, 431}, TXT_SIGN, TXT_LIGHT},
	{{432, 432}, TXT_FORCEFIELD, TXT_LIGHT},
	{{433, 434}, TXT_LIGHT, TXT_TECH},
	{{435, 901}, TXT_DOOR, 0}
	};



//UINT16 num_vertices;
//UINT16 num_segments;

//vms_vector *vertices=0;
//segment *Segments ()[MAX_SEGMENTS2];
//object *Objects ()[MAX_OBJECTS];
//UINT8 *vert_status=0; //MAX_VERTICES

//char current_level_name[256];

//wall *Walls (); //MAX_WALLS
//active_door active_doors[MAX_DOORS];
//trigger *Triggers ();// MAX_TRIGGERS;
//control_center_trigger control_center_triggers[MAX_CONTROL_CENTER_TRIGGERS];
//matcen_info robot_centers[MAX_NUM_MATCENS];

//game_top_info game_top_fileinfo;
//game_info GameInfo ();




