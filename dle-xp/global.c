/***************************************************************************

  NAME:    GLOBAL.C

  PURPOSE:

  BY:      Bryan Aamot

  REV:

***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <alloc.h>
#include <dos.h>
#include "define.h"
#include "types.h"

INT32 preferences = 0;
INT32 level_version=7;

INT16  N_flickering_lights = 0;
FLICKERING_LIGHT *flickering_lights=0;
UINT32       N_robot_types=0;
ROBOT_INFO   *Robot_info;     // MAX_ROBOT_TYPES
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

char FileName[256]="";
char FilePath[256]="";
char SubFile[20]="";
int skip_about;
int disable_drawing=0;
INT16 last_segment=0; // for "smart" forward/backward cube selection
int mouse_drag_state;
char skip[16] = "tomÅ";
int left;
int top;
int right;
int bottom;
int x_center;
int y_center;
int x_max;
int y_max;
FLOAT aspect_ratio;
int refresh_needed = 0;
int splash = 0;


int level_modified = 0;
UINT8 texture_modified[1+MAX_D2_TEXTURES/8];
//UINT8 test1 = 0xBA;
int disable_saves = 1;

const UINT8 clip_num[D2_NUM_OF_CLIPS] = {
  0,1,3,4,5,6,7,9,10,11,12,13,14,15,16,17,18,19,20,21,    // added 0 & 7 1/28/97
  22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,
  40,41,42,43,44,45,46,47,48,49,50
};

const UINT8 wall_flags[7] = {
  WALL_BLASTED,
  WALL_DOOR_OPENED,
  WALL_DOOR_LOCKED,
  WALL_DOOR_AUTO,
  WALL_ILLUSION_OFF,
  WALL_WALL_SWITCH,
  WALL_BUDDY_PROOF
};

int texture_resource = D2_TEXTURE_STRING_TABLE;

long ScrnMemMax = 0L;
int ScrnWidth = 0;
int ScrnHeight = 0;

// new data for Descent 2
int file_type = RL2_FILE;
dl_index     *dl_indices=0;    // MAX_DL_INDICES   x 6 bytes each
delta_light HUGE *delta_lights=0;  // MAX_DELTA_LIGHTS x 8 bytes each
INT16	     num_static_lights=0;

//char testtest[12570]; // this is how much space if free in the automatic data segment 7-31-96
char string[256];
//UINT8 test2 = 0xBA;
INT16 spline_segment1,spline_segment2,spline_side1,spline_side2;
char spline_error_message[] = "You must exit spline creation before preforming this function";
vms_vector spline_points[MAX_SPLINES];
//UINT8 test3 = 0xBA;
vms_vector points[4];
INT16 n_splines=0;
INT16 spline_length1=20,spline_length2=20;
INT16 spline_active=0;

const UINT8 object_list[MAX_OBJECT_NUMBER] = {
  OBJ_ROBOT,
  OBJ_HOSTAGE,
  OBJ_PLAYER,
  OBJ_WEAPON,
  OBJ_POWERUP,
  OBJ_CNTRLCEN,
  OBJ_COOP
};

const UINT8 contains_list[MAX_CONTAINS_NUMBER] = {
  OBJ_ROBOT,
  OBJ_POWERUP
};

// the following array is used to select a list item by obj->type
const INT8 object_selection[MAX_OBJECT_TYPES] = {
  -1,-1,0,1,2,3,-1,4,-1,5,-1,-1,-1,-1,6
};
const INT8 contains_selection[MAX_OBJECT_TYPES] = {
  -1,-1,0,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1
};

const UINT8 robot_clip[MAX_ROBOT_IDS_TOTAL] = {
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
const UINT8 powerup_clip[MAX_POWERUP_IDS2] = {
  36, 18, 19, 20, 24, 26, 25,  0,
   0,  0, 34, 35, 51, 37, 38, 39,
  40, 41, 42, 43, 44, 45, 46, 47,
   0, 49,  0,  0, 69, 70, 71, 72,
  77, 73, 74, 75, 76, 83, 78, 89,
  79, 90, 91, 81,102, 82,100,101
};

const char ai_options[MAX_D2_AI_OPTIONS][20] = {
  "Still","Normal","Hides behind","Drop Bombs","Snipe","Station","Follow","Smart Bombs"
};

const FIX powerup_size[MAX_POWERUP_IDS2] = {
  0x28000L,0x30000L,0x28000L,0x40000L,0x30000L,0x30000L,0x30000L,0x30000L,
  0x30000L,0x30000L,0x28000L,0x30000L,0x30000L,0x40000L,0x40000L,0x40000L,
  0x40000L,0x30000L,0x28000L,0x30000L,0x28000L,0x30000L,0x1ccccL,0x20000L,
  0x30000L,0x29999L,0x30000L,0x30000L,0x40000L,0x40000L,0x40000L,0x40000L,
  0x40000L,0x40000L,0x40000L,0x40000L,0x48000L,0x30000L,0x28000L,0x28000L,
  0x30000L,0x30000L,0x40000L,0x40000L,0x40000L,0x40000L,0x38000L,0x38000L
};


const FIX robot_size [MAX_ROBOT_IDS_TOTAL] = {
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

const FIX robot_shield [MAX_ROBOT_IDS_TOTAL] = {
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

const TEXTURE_LIGHT d1_texture_light[NUM_LIGHTS_D1] = {
  {250, 0x00b333L}, {251, 0x008000L}, {252, 0x008000L}, {253, 0x008000L},
  {264, 0x01547aL}, {265, 0x014666L}, {268, 0x014666L}, {278, 0x014cccL},
  {279, 0x014cccL}, {280, 0x011999L}, {281, 0x014666L}, {282, 0x011999L},
  {283, 0x0107aeL}, {284, 0x0107aeL}, {285, 0x011999L}, {286, 0x014666L},
  {287, 0x014666L}, {288, 0x014666L}, {289, 0x014666L}, {292, 0x010cccL},
  {293, 0x010000L}, {294, 0x013333L}, {330, 0x010000L}, {341, 0x010000L},
  {343, 0x010000L}, {345, 0x010000L}, {347, 0x010000L}, {349, 0x010000L},
  {351, 0x010000L}, {352, 0x010000L}, {354, 0x010000L}, {355, 0x010000L},
  {356, 0x020000L}, {357, 0x020000L}, {358, 0x020000L}, {359, 0x020000L},
  {360, 0x020000L}, {361, 0x020000L}, {362, 0x020000L}, {363, 0x020000L},
  {364, 0x020000L}, {365, 0x020000L}, {366, 0x020000L}, {367, 0x020000L},
  {368, 0x020000L}, {369, 0x020000L}, {370, 0x020000L}
};

const TEXTURE_LIGHT d2_texture_light[NUM_LIGHTS_D2] = {
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

INT16 add_segment_mode = MIRROR;
char dlc_version[10] = "2.7a";
//char testers_name[20] = "TESTERS_NAME_HERE";
INT16 serial_number   = 27;
char message[256];
//UINT8 test4 = 0xBA;
char starting_directory[256];
//UINT8 test5 = 0xBA;
FIX grid = 0x2000L;   // 0.125
char descent_path[256]="d:\\games\\descent";
//UINT8 test6 = 0xBA;
char descent2_path[256]="d:\\games\\descent\\descent 2";
//UINT8 test7 = 0xBA;
char levels_path[256]="d:\\\games\\descent\\descent 2\\missions";
//UINT8 test8 = 0xBA;

INT16 trignum = 0;
//INT16 number_of_textures=0;
//INT16  show_lines      = SHOW_LINES_PARTIAL;
INT16  show_lines      = SHOW_FILLED_POLYGONS;
INT16  show_walls      = 1;
INT16  show_special    = 1;
INT16  show_lights     = 1;
INT16  show_objects    = (1<<OBJ_ROBOT)  + (1<<OBJ_HOSTAGE) + (1<<OBJ_POWERUP)
			   + (1<<OBJ_PLAYER) + (1<<OBJ_COOP) +(1<<OBJ_CNTRLCEN);
INT16  show_powerups   =  POWERUP_POWERUP_MASK + POWERUP_WEAPON_MASK + POWERUP_KEY_MASK;
CUBE  current1 = {DEFAULT_SEGMENT,
		  DEFAULT_SIDE,
                  DEFAULT_LINE,
		  DEFAULT_POINT,
                  DEFAULT_OBJECT};
CUBE  current2 = {DEFAULT_SEGMENT,
                  DEFAULT_SIDE,
                  DEFAULT_LINE,
		  DEFAULT_POINT,
                  DEFAULT_OBJECT};
CUBE  *current = &current1;
INT16 select_mode     = CUBE_MODE;
INT16 edit_mode       = EDIT_OFF;
const uvl default_uvls[4] = {
  { 0x0000, 0x0000,DEFAULT_LIGHTING},
  { 0x0000, 0x0800,DEFAULT_LIGHTING},
  {-0x0800, 0x0800,DEFAULT_LIGHTING},
  {-0x0800, 0x0000,DEFAULT_LIGHTING}
};

const char object_names[MAX_OBJECT_TYPES][9] = {
  "Wall",
  "Fireball",
  "Robot",
  "Hostage",
  "Player",
  "Mine",
  "Camrea",
  "Power Up",
  "Debris",
  "Reactor",
  "Flare",
  "Clutter",
  "Ghost",
  "Light",
  "CoOp"
};

const UINT8 powerup_types[MAX_POWERUP_IDS2] = {
  POWERUP_POWERUP_MASK,
  POWERUP_POWERUP_MASK,
  POWERUP_POWERUP_MASK,
  POWERUP_WEAPON_MASK,
  POWERUP_KEY_MASK,
  POWERUP_KEY_MASK,
  POWERUP_KEY_MASK,
  POWERUP_UNKNOWN_MASK,
  POWERUP_UNKNOWN_MASK,
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
  POWERUP_POWERUP_MASK
};

const char wall_type_names[6][10] = {
  "Normal",
  "Blastable",
  "Door",
  "Illusion",
  "Open",
  "Closed"
};

const char wall_flag_names[8][14] = {
  "Blasted",
  "Door Closed",
  "(not used)",
  "Door Locked",
  "Door Auto",
  "Illusion Off",
  "(not used)",
  "(not used)"
};

const char wall_state_names[4][8] = {
  "Closed",
  "Opening",
  "Open",
  "Closing"
};

//APOINT poly_draw[MAX_POLY];

#if 0
const APOINT orgin[MAX_POLY] = {
  {20,  0,  0},
  { 0,  0,  0},
  { 0, 20,  0},
  { 0,  0,  0},
  { 0,  0, 20},
  { 0,  0,  0}
};
#endif

//APOINT view[MAX_POLY];

APOINT *scrn;
INT16 gx0,gy0,gz0;
FLOAT spinx,spiny,spinz;
FLOAT movex,movey,movez;
INT16 max_x,min_x,max_y,min_y,max_z,min_z;
FLOAT sizex,sizey,sizez;
FLOAT angle_rate = PI/16;
FIX   move_rate = 0x10000L;
FLOAT M[4][4];        /* 4x4 matrix used in coordinate transformation */
FLOAT IM[4][4]; /* inverse matrix of M[4][4] */
//INT16 page;
//INT8 old_video_mode;
FLOAT depth_perception = 100.0;

/* define points for a given side */
const UINT8 side_vert[6][4] = {
  {7,6,2,3},//{2,3,7,6},
  {0,4,7,3},//{0,3,7,4},
  {0,1,5,4},//{0,1,5,4},
  {2,6,5,1},//{1,2,6,5},
  {4,5,6,7},//{4,5,6,7},
  {3,2,1,0} //{0,1,2,3}
};

/* define oppisite side of a given side */
const UINT8 opp_side[6] = {2,3,0,1,5,4};

/* define points for the oppisite side of a given side */
const UINT8 opp_side_vert[6][4] = {
  {4,5,1,0},//{1,0,4,5},
  {1,5,6,2},//{1,2,6,5},
  {3,2,6,7},//{3,2,6,7},
  {3,7,4,0},//{0,3,7,4},
  {0,1,2,3},//{0,1,2,3},
  {7,6,5,4} //{4,5,6,7}
};

/* define 2 points for a given line */
const UINT8 line_vert[12][2] = {
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
const UINT8 side_line[6][4] = {
  {7,10,2,11},//{2,11,7,10},
  {4,8,11,3},//{3,11,8,4},
  {0,9,5,4},//{0,9,5,4},
  {10,6,9,1},//{1,10,6,9},
  {5,6,7,8},//{5,6,7,8},
  {2,1,0,3} //{0,1,2,3}
};

/* define 3 points which connect to a certain point */
const UINT8 connect_points[8][3] = {
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
const INT8 point_sides[8][3] = {
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
const INT8 point_corners[8][3] = {
    {0,0,3},
    {1,3,2},
    {2,0,1},
    {3,3,0},
    {1,3,0}, // was 2 3 0
    {2,2,1},
    {1,1,2},
    {0,2,3}  // was 0 0 3
};

UINT16 num_vertices;
UINT16 num_segments;

vms_vector *vertices=0;
segment *segments[MAX_SEGMENTS2];
//UINT8 test9 = 0xBA;
object *objects[MAX_OBJECTS];
//UINT8 test10 = 0xBA;
UINT8 *vert_status=0; //MAX_VERTICES

char current_level_name[256];
//UINT8 test11 = 0xBA;

wall *walls; //MAX_WALLS
active_door active_doors[MAX_DOORS];
//UINT8 test12 = 0xBA;
trigger *triggers;// MAX_TRIGGERS;
control_center_trigger control_center_triggers[MAX_CONTROL_CENTER_TRIGGERS];
//UINT8 test13 = 0xBA;
matcen_info robot_centers[MAX_ROBOT_CENTERS];
//UINT8 test14 = 0xBA;

game_top_info game_top_fileinfo;
game_info game_fileinfo;




