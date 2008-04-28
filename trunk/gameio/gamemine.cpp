/* $Id: gamemine.c, v 1.26 2003/10/22 15:00:37 schaffner Exp $ */
/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#define LIGHT_VERSION 4

#ifdef RCS
static char rcsid [] = "$Id: gamemine.c, v 1.26 2003/10/22 15:00:37 schaffner Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "pstypes.h"
#include "mono.h"

#include "inferno.h"
#include "text.h"
#include "segment.h"
#include "textures.h"
#include "wall.h"
#include "object.h"
#include "gamemine.h"
#include "error.h"
#include "gameseg.h"
#include "switch.h"
#include "ogl_defs.h"
#include "oof.h"
#include "lightmap.h"
#include "render.h"
#include "gameseg.h"

#include "game.h"
#include "menu.h"
#include "newmenu.h"

#ifdef EDITOR
#include "editor/editor.h"
#endif

#include "cfile.h"
#include "fuelcen.h"

#include "hash.h"
#include "key.h"
#include "piggy.h"

#include "byteswap.h"
#include "gamesave.h"
#include "u_mem.h"
#include "vecmat.h"
#include "gamepal.h"
#include "paging.h"
#include "maths.h"
#include "network.h"
#include "light.h"
#include "dynlight.h"
#include "renderlib.h"

//------------------------------------------------------------------------------

#define REMOVE_EXT (s)  (* (strchr ((s), '.' ))='\0')

struct mtfi mine_top_fileinfo;    // Should be same as first two fields below...
struct mfi mine_fileinfo;
struct mh mine_header;
struct me mine_editor;

typedef struct v16_segment {
	#ifdef EDITOR
	short   nSegment;             // tSegment number, not sure what it means
	#endif
	tSide    sides [MAX_SIDES_PER_SEGMENT];       // 6 sides
	short   children [MAX_SIDES_PER_SEGMENT];    // indices of 6 children segments, front, left, top, right, bottom, back
	short   verts [MAX_VERTICES_PER_SEGMENT];    // vertex ids of 4 front and 4 back vertices
	#ifdef  EDITOR
	short   group;              // group number to which the tSegment belongs.
	#endif
	short   objects;            // pointer to gameData.objs.objects in this tSegment
	ubyte   special;            // what nType of center this is
	sbyte   nMatCen;         // which center tSegment is associated with.
	short   value;
	fix     xAvgSegLight;       // average static light in tSegment
	#ifndef EDITOR
	short   pad;                // make structure longword aligned
	#endif
} v16_segment;

struct mfi_v19 {
	ushort  fileinfo_signature;
	ushort  fileinfoVersion;
	int     fileinfo_sizeof;
	int     header_offset;      // Stuff common to game & editor
	int     header_size;
	int     editor_offset;      // Editor specific stuff
	int     editor_size;
	int     segment_offset;
	int     segment_howmany;
	int     segment_sizeof;
	int     newseg_verts_offset;
	int     newseg_verts_howmany;
	int     newseg_verts_sizeof;
	int     group_offset;
	int     group_howmany;
	int     group_sizeof;
	int     vertex_offset;
	int     vertex_howmany;
	int     vertex_sizeof;
	int     texture_offset;
	int     texture_howmany;
	int     texture_sizeof;
	tGameItemInfo	walls;
	tGameItemInfo	triggers;
	tGameItemInfo	links;
	tGameItemInfo	tObject;
	int     unused_offset;      // was: doors.offset
	int     unused_howmamy;     // was: doors.count
	int     unused_sizeof;      // was: doors.size
	short   level_shake_frequency;  // Shakes every level_shake_frequency seconds
	short   level_shake_duration;   // for level_shake_duration seconds (on average, random).  In 16ths second.
	int     secret_return_segment;
	vmsMatrix  secret_return_orient;
	tGameItemInfo	lightDeltaIndices;
	tGameItemInfo	lightDeltas;
};

int CreateDefaultNewSegment ();

int bNewFileFormat = 1; // "new file format" is everything newer than d1 shareware

int bD1PigPresent = 0; // can descent.pig from descent 1 be loaded?

/* returns nonzero if nD1Texture references a texture which isn't available in d2. */
int d1_tmap_num_unique (short nD1Texture) 
{
	short t, i;
	static short unique_tmap_nums [] = {
		  0,   2,   4,   5,   6,   7,   9, 
		 10,  11,  12,  17,  18, 
		 20,  21,  25,  28, 
		 38,  39,  41,  44,  49, 
		 50,  55,  57,  88, 
		132, 141, 147, 
		154, 155, 158, 159, 
		160, 161, 167, 168, 169, 
		170, 171, 174, 175, 185, 
		193, 194, 195, 198, 199, 
		200, 202, 210, 211, 
		220, 226, 227, 228, 229, 230, 
		240, 241, 242, 243, 246, 
		250, 251, 252, 253, 257, 258, 259, 
		260, 263, 266, 283, 298, 
		305, 308, 311, 312, 
		315, 317, 319, 320, 321, 
		330, 331, 332, 333, 349, 
		351, 352, 353, 354, 
		355, 357, 358, 359, 
		362, 370, 
		-1};
	  
for (i = 0;; i++) {
	if (0 > (t = unique_tmap_nums [i]))
		break;
	if (t == nD1Texture)
		return 1;
	}
return 0;
}

#define TMAP_NUM_MASK 0x3FFF

#define	VERTVIS(_nSegment, _nVertex) \
	(gameData.segs.bVertVis ? gameData.segs.bVertVis [(_nSegment) * VERTVIS_FLAGS + ((_nVertex) >> 3)] & (1 << ((_nVertex) & 7)) : 0)

//	-----------------------------------------------------------------------------------------------------------
/* Converts descent 1 texture numbers to descent 2 texture numbers.
 * gameData.pig.tex.bmIndex from d1 which are unique to d1 have extra spaces around "return".
 * If we can load the original d1 pig, we make sure this function is bijective.
 * This function was updated using the file config/convtabl.ini from devil 2.2.
 */

typedef struct nD1ToD2Texture {
	short	d1_min, d1_max;
	short	repl [2];
} nD1ToD2Texture;

short ConvertD1Texture (short nD1Texture, int bForce) 
{
	int h, i;

	static nD1ToD2Texture nD1ToD2Texture [] = {
		{0, 0, {43, 137}}, 
		{1, 1, {0, 0}}, 
		{2, 2, {43, 137}}, 
		{3, 3, {1, 1}}, 
		{4, 4, {43, 137}}, 
		{5, 5, {43, 137}}, 
		{6, 7, {-1, 270 - 6}}, 
		{8, 8, {2, 2}}, 
		{9, 9, {62, 138}}, 
		{10, 10, {272}}, 
		{11, 11, {117, 139}}, 
		{12, 12, {12, 140}}, 
		{13, 16, {-1, 3 - 13}}, 
		{17, 17, {52, 141}}, 
		{18, 18, {129, 129}}, 
		{19, 19, {7, 7}}, 
		{20, 20, {22, 142}}, 
		{21, 21, {9, 143}}, 
		{22, 22, {8, 8}}, 
		{23, 23, {9, 9}}, 
		{24, 24, {10, 10}}, 
		{25, 25, {12, 144}}, 
		{26, 27, {-1, 11 - 26}}, 
		{28, 28, {11, 145}}, 
		{29, 37, {-1, -16}}, 
		{38, 38, {163, 163}}, 
		{39, 39, {147, 147}}, 
		{40, 40, {22, 22}}, 
		{41, 41, {266, 266}}, 
		{42, 43, {-1, 23 - 42}}, 
		{44, 44, {136, 136}}, 
		{45, 48, {-1, 25 - 45}}, 
		{49, 49, {43, 146}}, 
		{50, 50, {131, 131}}, 
		{51, 54, {-1, 29 - 51}}, 
		{55, 55, {165, 165}}, 
		{56, 56, {33, 33}}, 
		{57, 57, {132, 132}}, 
		{58, 87, {-1, -24}}, 
		{88, 88, {197, 197}}, 
		{89, 131, {-1, -25}}, 
		{132, 132, {167, 167}}, 
		{133, 140, {-1, -26}}, 
		{141, 141, {110, 148}}, 
		{142, 146, {-1, 115 - 142}}, 
		{147, 147, {93, 149}}, 
		{148, 153, {-1, 120 - 148}}, 
		{154, 154, {27, 150}}, 
		{155, 155, {126, 126}}, 
		{156, 157, {-1, 200 - 156}}, 
		{158, 158, {186, 186}}, 
		{159, 159, {190, 190}}, 
		{160, 160, {206, 151}}, 
		{161, 161, {114, 152}}, 
		{162, 166, {-1, 202 - 162}}, 
		{167, 167, {206, 153}}, 
		{168, 168, {206, 154}}, 
		{169, 169, {206, 155}}, 
		{170, 170, {227, 156}}, 
		{171, 171, {206, 157}}, 
		{172, 173, {-1, 207 - 172}}, 
		{174, 174, {202, 158}}, 
		{175, 175, {206, 159}}, 
		{176, 184, {-1, 33}}, 
		{185, 185, {217, 160}}, 
		{186, 192, {-1, 32}}, 
		{193, 193, {206, 161}}, 
		{194, 194, {203, 162}}, 
		{195, 195, {234, 166}}, 
		{196, 197, {-1, 225 - 196}}, 
		{198, 198, {225, 167}}, 
		{199, 199, {206, 168}}, 
		{200, 200, {206, 169}}, 
		{201, 201, {227, 227}}, 
		{202, 202, {206, 170}}, 
		{203, 209, {-1, 25}}, 
		{210, 210, {234, 171}}, 
		{211, 211, {206, 172}}, 
		{212, 219, {-1, 23}}, 
		{220, 220, {242, 173}}, 
		{221, 222, {-1, 243 - 221}}, 
		{223, 223, {313, 174}}, 
		{224, 225, {-1, 245 - 224}}, 
		{226, 226, {164, 164}}, 
		{227, 227, {179, 179}}, 
		{228, 228, {196, 196}}, 
		{229, 229, {15, 175}}, 
		{230, 230, {15, 176}}, 
		{231, 239, {-1, 18}}, 
		{240, 240, {6, 177}}, 
		{241, 241, {130, 130}}, 
		{242, 242, {78, 178}}, 
		{243, 243, {33, 180}}, 
		{244, 245, {-1, 258 - 244}}, 
//		{246, 246, {321, 181}}, 
		{246, 246, {321, 321}}, 
		{247, 249, {-1, 260 - 247}}, 
		{250, 250, {340, 340}}, 
		{251, 251, {412, 412}}, 
		{252, 253, {-1, 410 - 252}}, 
		{254, 256, {-1, 263 - 254}}, 
		{257, 257, {249, 182}}, 
		{258, 258, {251, 183}}, 
		{259, 259, {252, 184}}, 
		{260, 260, {256, 185}}, 
		{261, 262, {-1, 273 - 261}}, 
		{263, 263, {281, 187}}, 
		{264, 265, {-1, 275 - 264}}, 
		{266, 266, {279, 188}}, 
		{267, 281, {-1, 10}}, 
		{282, 282, {293, 293}}, 
		{283, 283, {295, 189}}, 
		{284, 284, {295, 295}}, 
		{285, 285, {296, 296}}, 
		{286, 286, {298, 298}}, 
		{287, 297, {-1, 13}}, 
		{298, 298, {364, 191}}, 
		{299, 304, {-1, 12}}, 
		{305, 305, {322, 322}}, 
		{306, 307, {-1, 12}}, 
		{308, 308, {324, 324}}, 
		{309, 314, {-1, 12}}, 
		{315, 315, {361, 192}}, 
		{316, 326, {-1, 11}}, 
		{327, 329, {-1, 352 - 327}}, 
		{330, 330, {380, 380}}, 
		{331, 331, {379, 379}}, 
		{332, 332, {350, 350}}, 
		{333, 333, {409, 409}}, 
		{334, 340, {-1, 356 - 334}}, 
//		{341, 341, {364, 364}}, 
		{341, 341, {372, 372}}, 
		{342, 342, {363, 363}}, 
		{343, 343, {366, 366}}, 
		{344, 344, {365, 365}}, 
		{345, 345, {382, 382}}, 
		{346, 346, {376, 376}}, 
		{347, 347, {370, 370}}, 
		{348, 348, {367, 367}}, 
		{349, 349, {331, 331}}, 
		{350, 350, {369, 369}}, 
//		{351, 352, {-1, 374 - 351}}, 
		{351, 351, {394, 394}}, 
		{352, 352, {370, 370}}, 
		{353, 353, {371, 371}}, 
//		{354, 354, {377, 377}}, 
		{354, 354, {394, 394}}, 
		{355, 355, {408, 408}}, 
		{356, 356, {378, 378}}, 
		{357, 361, {-1, 383 - 357}}, 
		{362, 362, {388, 194}}, 
		{363, 363, {388, 388}}, 
		{364, 369, {-1, 388 - 364}}, 
		{370, 370, {392, 195}}, 
		{371, 374, {-1, 64}}
		};

	if (gameStates.app.bHaveD1Data && !bForce)
		return nD1Texture;
	if ((nD1Texture > 370) && (nD1Texture < 584)) {
		if (bNewFileFormat) {
			if (nD1Texture == 550) 
				return 615;
			return nD1Texture + 64;
			}
		// d1 shareware needs special treatment:
		if (nD1Texture < 410) 
			return nD1Texture + 68;
		if (nD1Texture < 417) 
			return nD1Texture + 73;
		if (nD1Texture < 446) 
			return nD1Texture + 91;
		if (nD1Texture < 453) 
			return nD1Texture + 104;
		if (nD1Texture < 462) 
			return nD1Texture + 111;
		if (nD1Texture < 486) 
			return nD1Texture + 117;
		if (nD1Texture < 494) 
			return nD1Texture + 141;
		if (nD1Texture < 584) 
			return nD1Texture + 147;
		}

	for (h = sizeofa (nD1ToD2Texture), i = 0; i < h; i++)
		if ((nD1ToD2Texture [i].d1_min <= nD1Texture) && (nD1ToD2Texture [i].d1_max >= nD1Texture)) {
			if (nD1ToD2Texture [i].repl [0] == -1)	// -> repl [1] contains an offset
				return nD1Texture + nD1ToD2Texture [i].repl [1];
			else
				return nD1ToD2Texture [i].repl [bForce ? 0 : bD1PigPresent];
			}

	{ // handle rare case where orientation != 0
		short nTexture = nD1Texture & TMAP_NUM_MASK;
		short orient = nD1Texture & ~TMAP_NUM_MASK;
	if (orient)
		return orient | ConvertD1Texture (nTexture, bForce);
	//Warning (TXT_D1TEXTURE, nTexture);
	return nD1Texture;
	}
}

#ifdef EDITOR

static char old_tmap_list [MAX_TEXTURES][FILENAME_LEN];
short tmap_xlate_table [MAX_TEXTURES];
static short tmapTimes_used [MAX_TEXTURES];

// -----------------------------------------------------------------------------
//loads from an already-open file
// returns 0=everything ok, 1=old version, -1=error
int load_mine_data (CFILE *loadFile)
{
	int   i, j, oldsizeadjust;
	short tmap_xlate;
	int 	translate;
	char 	*temptr;
	int	mine_start = CFTell (loadFile);
	bD1PigPresent = CFExist (D1_PIGFILE);

	oldsizeadjust= (sizeof (int)*2)+sizeof (vmsMatrix);
	FuelCenReset ();

	for (i=0; i<MAX_TEXTURES; i++ )
		tmapTimes_used [i] = 0;

	#ifdef EDITOR
	// Create a new mine to initialize things.
	//texpage_goto_first ();
	create_new_mine ();
	#endif

	//===================== READ FILE INFO ========================

	// These are the default values... version and fileinfo_sizeof
	// don't have defaults.
	mine_fileinfo.header_offset     =   -1;
	mine_fileinfo.header_size       =   sizeof (mine_header);
	mine_fileinfo.editor_offset     =   -1;
	mine_fileinfo.editor_size       =   sizeof (mine_editor);
	mine_fileinfo.vertex_offset     =   -1;
	mine_fileinfo.vertex_howmany    =   0;
	mine_fileinfo.vertex_sizeof     =   sizeof (vmsVector);
	mine_fileinfo.segment_offset    =   -1;
	mine_fileinfo.segment_howmany   =   0;
	mine_fileinfo.segment_sizeof    =   sizeof (tSegment);
	mine_fileinfo.newseg_verts_offset     =   -1;
	mine_fileinfo.newseg_verts_howmany    =   0;
	mine_fileinfo.newseg_verts_sizeof     =   sizeof (vmsVector);
	mine_fileinfo.group_offset		  =	-1;
	mine_fileinfo.group_howmany	  =	0;
	mine_fileinfo.group_sizeof		  =	sizeof (group);
	mine_fileinfo.texture_offset    =   -1;
	mine_fileinfo.texture_howmany   =   0;
 	mine_fileinfo.texture_sizeof    =   FILENAME_LEN;  // num characters in a name
 	mine_fileinfo.walls.offset		  =	-1;
	mine_fileinfo.walls.count	  =	0;
	mine_fileinfo.walls.size		  =	sizeof (tWall);  
 	mine_fileinfo.triggers.offset	  =	-1;
	mine_fileinfo.triggers.count  =	0;
	mine_fileinfo.triggers.size	  =	sizeof (tTrigger);  
	mine_fileinfo.object.offset		=	-1;
	mine_fileinfo.object.count		=	1;
	mine_fileinfo.object.size		=	sizeof (tObject);  

	mine_fileinfo.level_shake_frequency		=	0;
	mine_fileinfo.level_shake_duration		=	0;

	//	Delta light stuff for blowing out light sources.
//	if (mine_top_fileinfo.fileinfoVersion >= 19) {
		mine_fileinfo.gameData.render.lights.deltaIndices.offset		=	-1;
		mine_fileinfo.gameData.render.lights.deltaIndices.count		=	0;
		mine_fileinfo.gameData.render.lights.deltaIndices.size		=	sizeof (tLightDeltaIndex);  

		mine_fileinfo.deltaLight.offset		=	-1;
		mine_fileinfo.deltaLight.count		=	0;
		mine_fileinfo.deltaLight.size		=	sizeof (tLightDelta);  

//	}

	mine_fileinfo.segment2_offset		= -1;
	mine_fileinfo.segment2_howmany	= 0;
	mine_fileinfo.segment2_sizeof    = sizeof (tSegment2);

	// Read in mine_top_fileinfo to get size of saved fileinfo.

	memset ( &mine_top_fileinfo, 0, sizeof (mine_top_fileinfo) );

	if (CFSeek ( loadFile, mine_start, SEEK_SET ))
		Error ( "Error moving to top of file in gamemine.c" );

	if (CFRead ( &mine_top_fileinfo, sizeof (mine_top_fileinfo), 1, loadFile )!=1)
		Error ( "Error reading mine_top_fileinfo in gamemine.c" );

	if (mine_top_fileinfo.fileinfo_signature != 0x2884)
		return -1;

	// Check version number
	if (mine_top_fileinfo.fileinfoVersion < COMPATIBLE_VERSION )
		return -1;

	// Now, Read in the fileinfo
	if (CFSeek ( loadFile, mine_start, SEEK_SET ))
		Error ( "Error seeking to top of file in gamemine.c" );

	if (CFRead ( &mine_fileinfo, mine_top_fileinfo.fileinfo_sizeof, 1, loadFile )!=1)
		Error ( "Error reading mine_fileinfo in gamemine.c" );

	if (mine_top_fileinfo.fileinfoVersion < 18) {
#if TRACE
		con_printf (1, "Old version, setting shake intensity to 0.\n");
#endif
		gameStates.gameplay.seismic.nShakeFrequency = 0;
		gameStates.gameplay.seismic.nShakeDuration = 0;
		gameData.segs.secret.nReturnSegment = 0;
		gameData.segs.secret.returnOrient = vmdIdentityMatrix;
	} else {
		gameStates.gameplay.seismic.nShakeFrequency = mine_fileinfo.level_shake_frequency << 12;
		gameStates.gameplay.seismic.nShakeDuration = mine_fileinfo.level_shake_duration << 12;
		gameData.segs.secret.nReturnSegment = mine_fileinfo.secret_return_segment;
		gameData.segs.secret.returnOrient = mine_fileinfo.secret_return_orient;
	}

	//===================== READ HEADER INFO ========================

	// Set default values.
	mine_header.num_vertices        =   0;
	mine_header.num_segments        =   0;

	if (mine_fileinfo.header_offset > -1 )
	{
		if (CFSeek ( loadFile, mine_fileinfo.header_offset, SEEK_SET ))
			Error ( "Error seeking to header_offset in gamemine.c" );

		if (CFRead ( &mine_header, mine_fileinfo.header_size, 1, loadFile )!=1)
			Error ( "Error reading mine_header in gamemine.c" );
	}

	//===================== READ EDITOR INFO ==========================

	// Set default values
	mine_editor.current_seg         =   0;
	mine_editor.newsegment_offset   =   -1; // To be written
	mine_editor.newsegment_size     =   sizeof (tSegment);
	mine_editor.Curside             =   0;
	mine_editor.Markedsegp          =   -1;
	mine_editor.Markedside          =   0;

	if (mine_fileinfo.editor_offset > -1 )
	{
		if (CFSeek ( loadFile, mine_fileinfo.editor_offset, SEEK_SET ))
			Error ( "Error seeking to editor_offset in gamemine.c" );

		if (CFRead ( &mine_editor, mine_fileinfo.editor_size, 1, loadFile )!=1)
			Error ( "Error reading mine_editor in gamemine.c" );
	}

	//===================== READ TEXTURE INFO ==========================

	if ((mine_fileinfo.texture_offset > -1) && (mine_fileinfo.texture_howmany > 0))
	{
		if (CFSeek ( loadFile, mine_fileinfo.texture_offset, SEEK_SET ))
			Error ( "Error seeking to texture_offset in gamemine.c" );

		for (i=0; i< mine_fileinfo.texture_howmany; i++ )
		{
			if (CFRead ( &old_tmap_list [i], mine_fileinfo.texture_sizeof, 1, loadFile )!=1)
				Error ( "Error reading old_tmap_list [i] in gamemine.c" );
		}
	}

	//=============== GENERATE TEXTURE TRANSLATION TABLE ===============

	translate = 0;

	Assert (gameData.pig.tex.nTextures < MAX_TEXTURES);

	{
		hashtable ht;

		hashtable_init ( &ht, gameData.pig.tex.nTextures );

		// Remove all the file extensions in the textures list

		for (i=0;i<gameData.pig.tex.nTextures;i++)	{
			temptr = strchr (gameData.pig.tex.pTMapInfo [i].filename, '.');
			if (temptr) *temptr = '\0';
			hashtable_insert ( &ht, gameData.pig.tex.pTMapInfo [i].filename, i );
		}

		// For every texture, search through the texture list
		// to find a matching name.
		for (j=0;j<mine_fileinfo.texture_howmany;j++) 	{
			// Remove this texture name's extension
			temptr = strchr (old_tmap_list [j], '.');
			if (temptr) *temptr = '\0';

			tmap_xlate_table [j] = hashtable_search ( &ht, old_tmap_list [j]);
			if (tmap_xlate_table [j]	< 0 )	{
				//tmap_xlate_table [j] = 0;
				;
			}
			if (tmap_xlate_table [j] != j ) translate = 1;
			if (tmap_xlate_table [j] >= 0)
				tmapTimes_used [tmap_xlate_table [j]]++;
		}

		{
			int count = 0;
			for (i=0; i<MAX_TEXTURES; i++ )
				if (tmapTimes_used [i])
					count++;
#if TRACE
			con_printf (CONDBG, "This mine has %d unique textures in it (~%d KB)\n", count, (count*4096) /1024 );
#endif
		}

		hashtable_free ( &ht );
	}

	//====================== READ VERTEX INFO ==========================

	// New check added to make sure we don't read in too many vertices.
	if ( mine_fileinfo.vertex_howmany > MAX_VERTICES )
		{
#if TRACE
		con_printf (CONDBG, "Num vertices exceeds maximum.  Loading MAX %d vertices\n", MAX_VERTICES);
#endif
		mine_fileinfo.vertex_howmany = MAX_VERTICES;
		}

	if ((mine_fileinfo.vertex_offset > -1) && (mine_fileinfo.vertex_howmany > 0))
	{
		if (CFSeek ( loadFile, mine_fileinfo.vertex_offset, SEEK_SET ))
			Error ( "Error seeking to vertex_offset in gamemine.c" );

		for (i=0; i< mine_fileinfo.vertex_howmany; i++ )
		{
			// Set the default values for this vertex
			gameData.segs.vertices [i].x = 1;
			gameData.segs.vertices [i].y = 1;
			gameData.segs.vertices [i].z = 1;

			if (CFRead ( &gameData.segs.vertices [i], mine_fileinfo.vertex_sizeof, 1, loadFile )!=1)
				Error ( "Error reading gameData.segs.vertices [i] in gamemine.c" );
		}
	}

	//==================== READ SEGMENT INFO ===========================

	// New check added to make sure we don't read in too many segments.
	if ( mine_fileinfo.segment_howmany > MAX_SEGMENTS ) {
#if TRACE
		con_printf (CONDBG, "Num segments exceeds maximum.  Loading MAX %d segments\n", MAX_SEGMENTS);
#endif
		mine_fileinfo.segment_howmany = MAX_SEGMENTS;
		mine_fileinfo.segment2_howmany = MAX_SEGMENTS;
	}

	// [commented out by mk on 11/20/94 (weren't we supposed to hit final in October?) because it looks redundant.  I think I'll test it now...]  FuelCenReset ();

	if ((mine_fileinfo.segment_offset > -1) && (mine_fileinfo.segment_howmany > 0))	{

		if (CFSeek ( loadFile, mine_fileinfo.segment_offset, SEEK_SET ))

			Error ( "Error seeking to segment_offset in gamemine.c" );

		gameData.segs.nLastSegment = mine_fileinfo.segment_howmany-1;

		for (i=0; i< mine_fileinfo.segment_howmany; i++ ) {

			// Set the default values for this tSegment (clear to zero )
			//memset ( &gameData.segs.segments [i], 0, sizeof (tSegment) );

			if (mine_top_fileinfo.fileinfoVersion < 20) {
				v16_segment v16_seg;

				Assert (mine_fileinfo.segment_sizeof == sizeof (v16_seg);

				if (CFRead ( &v16_seg, mine_fileinfo.segment_sizeof, 1, loadFile )!=1)
					Error ( "Error reading segments in gamemine.c" );

				#ifdef EDITOR
				gameData.segs.segments [i].nSegment = v16_seg.nSegment;
				// -- gameData.segs.segments [i].pad = v16_seg.pad;
				#endif

				for (j=0; j<MAX_SIDES_PER_SEGMENT; j++)
					gameData.segs.segments [i].sides [j] = v16_seg.sides [j];

				for (j=0; j<MAX_SIDES_PER_SEGMENT; j++)
					gameData.segs.segments [i].children [j] = v16_seg.children [j];

				for (j=0; j<MAX_VERTICES_PER_SEGMENT; j++)
					gameData.segs.segments [i].verts [j] = v16_seg.verts [j];

				gameData.segs.segment2s [i].special = v16_seg.special;
				gameData.segs.segment2s [i].value = v16_seg.value;
				gameData.segs.segment2s [i].s2Flags = 0;
				gameData.segs.segment2s [i].nMatCen = v16_seg.nMatCen;
				gameData.segs.segment2s [i].xAvgSegLight = v16_seg.xAvgSegLight;
				FuelCenActivate ( &gameData.segs.segments [i], gameData.segs.segment2s [i].special );

			} else  {
				if (CFRead (gameData.segs.segments + i, mine_fileinfo.segment_sizeof, 1, loadFile )!=1)
					Error ("Unable to read tSegment %i\n", i);
			}

			gameData.segs.segments [i].objects = -1;
			#ifdef EDITOR
			gameData.segs.segments [i].group = -1;
			#endif

			if (mine_top_fileinfo.fileinfoVersion < 15) {	//used old tUVL ranges
				int sn, uvln;

				for (sn=0;sn<MAX_SIDES_PER_SEGMENT;sn++)
					for (uvln=0;uvln<4;uvln++) {
						gameData.segs.segments [i].sides [sn].uvls [uvln].u /= 64;
						gameData.segs.segments [i].sides [sn].uvls [uvln].v /= 64;
						gameData.segs.segments [i].sides [sn].uvls [uvln].l /= 32;
					}
			}

			if (translate == 1)
				for (j=0;j<MAX_SIDES_PER_SEGMENT;j++) {
					unsigned short orient;
					tmap_xlate = gameData.segs.segments [i].sides [j].nBaseTex;
					gameData.segs.segments [i].sides [j].nBaseTex = tmap_xlate_table [tmap_xlate];
					if ((WALL_IS_DOORWAY (gameData.segs.segments + i, j, NULL) & WID_RENDER_FLAG))
						if (gameData.segs.segments [i].sides [j].nBaseTex < 0)	{
#if TRACE
							con_printf (CONDBG, "Couldn't find texture '%s' for Segment %d, tSide %d\n", old_tmap_list [tmap_xlate], i, j);
#endif
							Int3 ();
							gameData.segs.segments [i].sides [j].nBaseTex = gameData.pig.tex.nTextures-1;
						}
					tmap_xlate = gameData.segs.segments [i].sides [j].nOvlTex;
					if (tmap_xlate != 0) {
						int xlated_tmap = tmap_xlate_table [tmap_xlate];

						if ((WALL_IS_DOORWAY (gameData.segs.segments + i, j, NULL) & WID_RENDER_FLAG))
							if (xlated_tmap <= 0)	{
#if TRACE
								con_printf (CONDBG, "Couldn't find texture '%s' for Segment %d, tSide %d\n", old_tmap_list [tmap_xlate], i, j);
#endif
								Int3 ();
								gameData.segs.segments [i].sides [j].nOvlTex = gameData.pig.tex.nTextures - 1;
							}
						gameData.segs.segments [i].sides [j].nOvlTex = xlated_tmap;
					}
				}
		}


		if (mine_top_fileinfo.fileinfoVersion >= 20)
			for (i = 0; i<=gameData.segs.nLastSegment; i++) {
				CFRead (gameData.segs.segment2s + i, sizeof (tSegment2), 1, loadFile);
				FuelCenActivate (gameData.segs.segments + i, gameData.segs.segment2s [i].special );
			}
	}

	//===================== READ NEWSEGMENT INFO =====================

	#ifdef EDITOR

	{		// Default tSegment created.
		vmsVector	sizevec;
		med_create_new_segment (VmVecMake (&sizevec, DEFAULT_X_SIZE, DEFAULT_Y_SIZE, DEFAULT_Z_SIZE);		// New_segment = gameData.segs.segments [0];
		//memset ( &New_segment, 0, sizeof (tSegment) );
	}

	if (mine_editor.newsegment_offset > -1)
	{
		if (CFSeek ( loadFile, mine_editor.newsegment_offset, SEEK_SET ))
			Error ( "Error seeking to newsegment_offset in gamemine.c" );
		if (CFRead ( &New_segment, mine_editor.newsegment_size, 1, loadFile )!=1)
			Error ( "Error reading new_segment in gamemine.c" );
	}

	if ((mine_fileinfo.newseg_verts_offset > -1) && (mine_fileinfo.newseg_verts_howmany > 0))
	{
		if (CFSeek ( loadFile, mine_fileinfo.newseg_verts_offset, SEEK_SET ))
			Error ( "Error seeking to newseg_verts_offset in gamemine.c" );
		for (i=0; i< mine_fileinfo.newseg_verts_howmany; i++ )
		{
			// Set the default values for this vertex
			gameData.segs.vertices [NEW_SEGMENT_VERTICES+i].x = 1;
			gameData.segs.vertices [NEW_SEGMENT_VERTICES+i].y = 1;
			gameData.segs.vertices [NEW_SEGMENT_VERTICES+i].z = 1;
		
			if (CFRead ( &gameData.segs.vertices [NEW_SEGMENT_VERTICES+i], mine_fileinfo.newseg_verts_sizeof, 1, loadFile )!=1)
				Error ( "Error reading gameData.segs.vertices [NEW_SEGMENT_VERTICES+i] in gamemine.c" );

			New_segment.verts [i] = NEW_SEGMENT_VERTICES+i;
		}
	}

	#endif
														
	//========================= UPDATE VARIABLES ======================

	#ifdef EDITOR

	// Setting to Markedsegp to NULL ignores Curside and Markedside, which
	// we want to do when reading in an old file.

 	Markedside = mine_editor.Markedside;
	Curside = mine_editor.Curside;
	for (i=0;i<10;i++)
		Groupside [i] = mine_editor.Groupside [i];

	if ( mine_editor.current_seg != -1 )
		Cursegp = mine_editor.current_seg + gameData.segs.segments;
	else
 		Cursegp = NULL;

	if (mine_editor.Markedsegp != -1 ) 
		Markedsegp = mine_editor.Markedsegp + gameData.segs.segments;
	else
		Markedsegp = NULL;

	num_groups = 0;
	current_group = -1;

	#endif

	gameData.segs.nVertices = mine_fileinfo.vertex_howmany;
	gameData.segs.nSegments = mine_fileinfo.segment_howmany;
	gameData.segs.nLastVertex = gameData.segs.nVertices-1;
	gameData.segs.nLastSegment = gameData.segs.nSegments-1;

	ResetObjects (1);		//one tObject, the tPlayer

	#ifdef EDITOR
	gameData.segs.nLastVertex = MAX_SEGMENT_VERTICES-1;
	gameData.segs.nLastSegment = MAX_SEGMENTS-1;
	set_vertexCounts ();
	gameData.segs.nLastVertex = gameData.segs.nVertices-1;
	gameData.segs.nLastSegment = gameData.segs.nSegments-1;

	warn_if_concave_segments ();
	#endif

	#ifdef EDITOR
		ValidateSegmentAll ();
	#endif

	//create_local_segment_data ();

	//gamemine_find_textures ();

	if (mine_top_fileinfo.fileinfoVersion < MINE_VERSION )
		return 1;		//old version
	else
		return 0;

}
#endif

//------------------------------------------------------------------------------

#define COMPILED_MINE_VERSION 0

void read_children (int nSegment, ubyte bit_mask, CFILE *loadFile)
{
	int bit;

	for (bit=0; bit<MAX_SIDES_PER_SEGMENT; bit++) {
		if (bit_mask & (1 << bit)) {
			gameData.segs.segments [nSegment].children [bit] = CFReadShort (loadFile);
		} else
			gameData.segs.segments [nSegment].children [bit] = -1;
	}
}

//------------------------------------------------------------------------------

void ReadColor (tFaceColor *pc, CFILE *loadFile, int bFloatData, int bRegisterColor)
{
pc->index = CFReadByte (loadFile);
if (bFloatData) {
	tRgbColord	c;
	CFRead (&c, sizeof (c), 1, loadFile);
	pc->color.red = (float) c.red;
	pc->color.green = (float) c.green;
	pc->color.blue = (float) c.blue;
	}
else {
	int c = CFReadInt (loadFile);
	pc->color.red = (float) c / (float) 0x7fffffff;
	c = CFReadInt (loadFile);
	pc->color.green = (float) c / (float) 0x7fffffff;
	c = CFReadInt (loadFile);
	pc->color.blue = (float) c / (float) 0x7fffffff;
	}
if (bRegisterColor &&
	 (((pc->color.red > 0) && (pc->color.red < 1)) ||
	 ((pc->color.green > 0) && (pc->color.green < 1)) ||
	 ((pc->color.blue > 0) && (pc->color.blue < 1))))
	gameStates.render.bColored = 1;
pc->color.alpha = 1;
}

//------------------------------------------------------------------------------

void ReadSegVerts (int nSegment, CFILE *loadFile)
{
for (int i = 0; i < MAX_VERTICES_PER_SEGMENT; i++)
	gameData.segs.segments [nSegment].verts [i] = CFReadShort (loadFile);
}

//------------------------------------------------------------------------------

void ReadSegSpecialType (int nSegment, ubyte bit_mask, CFILE *loadFile)
{
if (bit_mask & (1 << MAX_SIDES_PER_SEGMENT)) {
	gameData.segs.segment2s [nSegment].special = CFReadByte (loadFile);
	gameData.segs.segment2s [nSegment].nMatCen = CFReadByte (loadFile);
	gameData.segs.segment2s [nSegment].value = (char) CFReadShort (loadFile);
	} 
else {
	gameData.segs.segment2s [nSegment].special = 0;
	gameData.segs.segment2s [nSegment].nMatCen = -1;
	gameData.segs.segment2s [nSegment].value = 0;
	}
}

//------------------------------------------------------------------------------

typedef struct tLightDist {
	int		nIndex;
	int		nDist;
} tLightDist;

void QSortLightDist (tLightDist *pDist, int left, int right)
{
	int			l = left, 
					r = right, 
					m = pDist [(l + r) / 2].nDist;
	tLightDist	h;

do {
	while (pDist [l].nDist < m)
		l++;
	while (pDist [r].nDist > m)
		r--;
	if (l <= r) {
		if (l < r) {
			h = pDist [l];
			pDist [l] = pDist [r];
			pDist [r] = h;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (l < right)
	QSortLightDist (pDist, l, right);
if (left < r)
	QSortLightDist (pDist, left, r);
}

//------------------------------------------------------------------------------

static inline int IsLightVert (int nVertex, grsFace *faceP)
{
ushort *pv = (gameStates.render.bTriangleMesh ? faceP->triIndex : faceP->index);
for (int i = faceP->nVerts; i; i--, pv++)
	if (*pv == (ushort) nVertex)
		return 1;
return 0;
}

//------------------------------------------------------------------------------

int nMaxNearestLights [21] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,20,24,28,32};

int ComputeNearestSegmentLights (int i)
{
	tSegment				*segP;
	tDynLight			*pl;
	int					h, j, k, l, m, n, nMaxLights;
	vmsVector			center;
	struct tLightDist	*pDists;

PrintLog ("computing nearest segment lights (%d)\n", i);
if (!gameData.render.lights.dynamic.nLights)
	return 0;
if (!(pDists = (tLightDist *) D2_ALLOC (gameData.render.lights.dynamic.nLights * sizeof (tLightDist)))) {
	gameOpts->render.bDynLighting = 0;
	gameData.render.shadows.nLights = 0;
	return 0;
	}
nMaxLights = MAX_NEAREST_LIGHTS;
if (gameStates.app.bMultiThreaded)
	j = i ? gameData.segs.nSegments : gameData.segs.nSegments / 2;
else
	INIT_PROGRESS_LOOP (i, j, gameData.segs.nSegments);
for (segP = gameData.segs.segments + i; i < j; i++, segP++) {
	COMPUTE_SEGMENT_CENTER (&center, segP);
	pl = gameData.render.lights.dynamic.lights;
	for (l = n = 0; l < gameData.render.lights.dynamic.nLights; l++, pl++) {
		m = (pl->nSegment < 0) ? gameData.objs.objects [pl->nObject].nSegment : pl->nSegment;
		if (!SEGVIS (m, i))
			continue;
		h = (int) (VmVecDist (&center, &pl->vPos) - fl2f (pl->rad) / 10.0f);
		if (h > MAX_LIGHT_RANGE * pl->range)
			continue;
		pDists [n].nDist = h;
		pDists [n++].nIndex = l;
		}
	if (n)
		QSortLightDist (pDists, 0, n - 1);
	h = (nMaxLights < n) ? nMaxLights : n;
	k = i * MAX_NEAREST_LIGHTS;
	for (l = 0; l < h; l++)
		gameData.render.lights.dynamic.nNearestSegLights [k + l] = pDists [l].nIndex;
	for (; l < MAX_NEAREST_LIGHTS; l++)
		gameData.render.lights.dynamic.nNearestSegLights [k + l] = -1;
	}
D2_FREE (pDists);
return 1;
}

//------------------------------------------------------------------------------

#ifdef _DEBUG
extern int nDbgVertex;
#endif

int ComputeNearestVertexLights (int nVertex)
{
	vmsVector			*vertP;
	tDynLight			*pl;
	tSide					*sideP;
	int					h, j, k, l, n, nMaxLights;
	vmsVector			vLightToVert;
	struct tLightDist	*pDists;

PrintLog ("computing nearest vertex lights (%d)\n", nVertex);
if (!gameData.render.lights.dynamic.nLights)
	return 0;
if (!(pDists = (tLightDist *) D2_ALLOC (gameData.render.lights.dynamic.nLights * sizeof (tLightDist)))) {
	gameOpts->render.bDynLighting = 0;
	gameData.render.shadows.nLights = 0;
	return 0;
	}
#ifdef _DEBUG
if (nVertex == nDbgVertex)
	nDbgVertex = nDbgVertex;
#endif
nMaxLights = MAX_NEAREST_LIGHTS;
if (gameStates.app.bMultiThreaded) 
	j = nVertex ? gameData.segs.nVertices : gameData.segs.nVertices / 2;
else
	INIT_PROGRESS_LOOP (nVertex, j, gameData.segs.nVertices);
for (vertP = gameData.segs.vertices + nVertex; nVertex < j; nVertex++, vertP++) {
#ifdef _DEBUG
	if (nVertex == nDbgVertex)
		nVertex = nVertex;
#endif
	pl = gameData.render.lights.dynamic.lights;
	for (l = n = 0; l < gameData.render.lights.dynamic.nLights; l++, pl++) {
#ifdef _DEBUG
		if (pl->nSegment == nDbgSeg)
			nDbgSeg = nDbgSeg;
#endif
		if (IsLightVert (nVertex, pl->faceP)) 
			h = 0;
		else {
			h = (pl->nSegment < 0) ? gameData.objs.objects [pl->nObject].nSegment : pl->nSegment;
			if (!VERTVIS (h, nVertex))
				continue;
			VmVecSub (&vLightToVert, vertP, &pl->vPos);
			h = VmVecNormalize (&vLightToVert) - (int) (pl->rad * 6553.6f);
			if (h > MAX_LIGHT_RANGE * pl->range)
				continue;
			if ((pl->nSegment >= 0) && (pl->nSide >= 0)) {
				sideP = SEGMENTS [pl->nSegment].sides + pl->nSide;
				if ((VmVecDot (sideP->normals, &vLightToVert) < -F1_0 / 6) && 
					 ((sideP->nType == SIDE_IS_QUAD) || (VmVecDot (sideP->normals + 1, &vLightToVert) < -F1_0 / 6)))
					continue;
				}	
			}
		pDists [n].nDist = h;
		pDists [n].nIndex = l;
		n++;
		}
	if (n)
		QSortLightDist (pDists, 0, n - 1);
	h = (nMaxLights < n) ? nMaxLights : n;
	k = nVertex * MAX_NEAREST_LIGHTS;
	for (l = 0; l < h; l++)
		gameData.render.lights.dynamic.nNearestVertLights [k + l] = pDists [l].nIndex;
	for (; l < MAX_NEAREST_LIGHTS; l++)
		gameData.render.lights.dynamic.nNearestVertLights [k + l] = -1;
	}
D2_FREE (pDists);
return 1;
}

//------------------------------------------------------------------------------

#ifdef _DEBUG
grsFace *FindDupFace (short nSegment, short nSide)
{
	tSegFaces	*segFaceP = SEGFACES + nSegment;
	grsFace		*faceP0, *faceP1;
	int			i, j;

for (i = segFaceP->nFaces, faceP0 = segFaceP->pFaces; i; faceP0++, i--)
	if (faceP0->nSide == nSide)
		break;
for (i = 0, segFaceP = SEGFACES; i < gameData.segs.nSegments; i++, segFaceP++) {
	for (j = segFaceP->nFaces, faceP1 = segFaceP->pFaces; j; faceP1++, j--) {
		if (faceP1 == faceP0)
			continue;
		if ((faceP1->nIndex == faceP0->nIndex) || !memcmp (faceP1->index, faceP0->index, sizeof (faceP0->index)))
			return faceP1;
		}
	}
return 0;
}

#endif

//------------------------------------------------------------------------------

void LoadSegmentsCompiled (short nSegment, CFILE *loadFile)
{
	short			lastSeg, nSide, i;
	tSegment		*segP;
	tSegFaces	*segFaceP;
	tSide			*sideP;
	short			temp_short;
	ushort		nWall, temp_ushort = 0;
	short			sideVerts [4];
	ubyte			bit_mask;

INIT_PROGRESS_LOOP (nSegment, lastSeg, gameData.segs.nSegments);
for (segP = SEGMENTS + nSegment, segFaceP = SEGFACES + nSegment; nSegment < lastSeg; nSegment++, segP++, segFaceP++) {

#ifdef EDITOR
	segP->nSegment = nSegment;
	segP->group = 0;
#endif

#ifdef _DEBUG
	if (nSegment == nDbgSeg)
		nSegment = nSegment;
#endif
	segFaceP->nFaces = 0;
	if (gameStates.app.bD2XLevel) { 
		gameData.segs.xSegments [nSegment].owner = CFReadByte (loadFile);
		gameData.segs.xSegments [nSegment].group = CFReadByte (loadFile);
		}
	else {
		gameData.segs.xSegments [nSegment].owner = -1;
		gameData.segs.xSegments [nSegment].group = -1;
		}
	if (bNewFileFormat)
		bit_mask = CFReadByte (loadFile);
	else
		bit_mask = 0x7f; // read all six children and special stuff...

	if (gameData.segs.nLevelVersion == 5) { // d2 SHAREWARE level
		ReadSegSpecialType (nSegment, bit_mask, loadFile);
		ReadSegVerts (nSegment, loadFile);
		read_children (nSegment, bit_mask, loadFile);
		}
	else {
		read_children (nSegment, bit_mask, loadFile);
		ReadSegVerts (nSegment, loadFile);
		if (gameData.segs.nLevelVersion <= 1) { // descent 1 level
			ReadSegSpecialType (nSegment, bit_mask, loadFile);
			}
		}
	segP->objects = -1;

	if (gameData.segs.nLevelVersion <= 5) { // descent 1 thru d2 SHAREWARE level
		// Read fix	segP->xAvgSegLight (shift down 5 bits, write as short)
		temp_ushort = CFReadShort (loadFile);
		gameData.segs.segment2s [nSegment].xAvgSegLight	= ((fix)temp_ushort) << 4;
		//CFRead ( &segP->xAvgSegLight, sizeof (fix), 1, loadFile );
		}

	// Read the walls as a 6 byte array
	for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++ )	{
		segP->sides [nSide].nFrame = 0;
		}

	if (bNewFileFormat)
		bit_mask = CFReadByte (loadFile);
	else
		bit_mask = 0x3f; // read all six sides
	for (nSide = 0, sideP = segP->sides; nSide < MAX_SIDES_PER_SEGMENT; nSide++, sideP++) {
		sideP->nWall = (ushort) -1;
		if (bit_mask & (1 << nSide)) {
			if (gameData.segs.nLevelVersion >= 13)
				nWall = (ushort) CFReadShort (loadFile);
			else
				nWall = (ushort) ((ubyte) CFReadByte (loadFile));
			if (IS_WALL (nWall))
				sideP->nWall = nWall;
			}
		}

#ifdef _DEBUG
	if (nSegment == nDbgSeg)
		nSegment = nSegment;
#endif
	for (nSide = 0, sideP = segP->sides; nSide < MAX_SIDES_PER_SEGMENT; nSide++, sideP++ )	{
#ifdef _DEBUG
		int bReadSideData;
		nWall = WallNumI (nSegment, nSide);
		if (segP->children [nSide] == -1)
			bReadSideData = 1;
		else if (IS_WALL (nWall))
			bReadSideData = 2;
		else
			bReadSideData = 0;
		if (bReadSideData) {
#else
		nWall = WallNumI (nSegment, nSide);
		if ((segP->children [nSide] == -1) || IS_WALL (nWall)) {
#endif
			// Read short sideP->nBaseTex;
			if (bNewFileFormat) {
				temp_ushort = CFReadShort (loadFile);
				sideP->nBaseTex = temp_ushort & 0x7fff;
				} 
			else
				sideP->nBaseTex = CFReadShort (loadFile);
			if (gameData.segs.nLevelVersion <= 1)
				sideP->nBaseTex = ConvertD1Texture (sideP->nBaseTex, 0);
			if (bNewFileFormat && !(temp_ushort & 0x8000))
				sideP->nOvlTex = 0;
			else {
				// Read short sideP->nOvlTex;
				short h = CFReadShort (loadFile);
				sideP->nOvlTex = h & 0x3fff;
				sideP->nOvlOrient = (h >> 14) & 3;
				if ((gameData.segs.nLevelVersion <= 1) && sideP->nOvlTex)
					sideP->nOvlTex = ConvertD1Texture (sideP->nOvlTex, 0);
				}

			// Read tUVL sideP->uvls [4] (u, v>>5, write as short, l>>1 write as short)
			GetSideVertIndex (sideVerts, nSegment, nSide);
			for (i = 0; i < 4; i++) {
				temp_short = CFReadShort (loadFile);
				sideP->uvls [i].u = ((fix)temp_short) << 5;
				temp_short = CFReadShort (loadFile);
				sideP->uvls [i].v = ((fix)temp_short) << 5;
				temp_ushort = CFReadShort (loadFile);
#if 0 //LIGHTMAPS
				if (USE_LIGHTMAPS)
					sideP->uvls [i].l = F1_0 / 2;
				else
#endif
				sideP->uvls [i].l = ((fix)temp_ushort) << 1;
				gameData.render.color.vertBright [sideVerts [i]] = f2fl (sideP->uvls [i].l);
				}
			} 
		else {
			sideP->nBaseTex =
			sideP->nOvlTex = 0;
			}
		}
	}
}

//------------------------------------------------------------------------------

void LoadSegment2sCompiled (CFILE *loadFile)
{
	int	i;

gameData.matCens.nRepairCenters = 0;
for (i = 0; i < gameData.segs.nSegments; i++) {
	if (gameData.segs.nLevelVersion > 5)
		ReadSegment2 (gameData.segs.segment2s + i, loadFile);
	FuelCenActivate (gameData.segs.segments + i, gameData.segs.segment2s [i].special);
	}
}

//------------------------------------------------------------------------------

void LoadVertLightsCompiled (int i, CFILE *loadFile)
{
	int	j;

gameData.render.shadows.nLights = 0;
if (gameStates.app.bD2XLevel) {
	INIT_PROGRESS_LOOP (i, j, gameData.segs.nVertices);
	for (; i < j; i++) {
		ReadColor (gameData.render.color.ambient + i, loadFile, gameData.segs.nLevelVersion <= 14, 1);
		}
	}
}

//------------------------------------------------------------------------------

void InitTexColors (void)
{
	int			i;
	GLfloat		color [3];
	tFaceColor	*pf = gameData.render.color.textures;

// get the default colors
memset (gameData.render.color.textures, 0, sizeof (gameData.render.color.textures));
for (i = 0; i < MAX_WALL_TEXTURES; i++, pf++) {
	if (GetLightColor (i, color) > 0) {
		pf->index = 1;
		pf->color.red = color [0];
		pf->color.green = color [1];
		pf->color.blue = color [2];
		}
	}
}

//------------------------------------------------------------------------------

int HasColoredLight (void)
{
	int			i, bColored = 0;
	tFaceColor	*pvc = gameData.render.color.ambient;

if (!gameStates.app.bD2XLevel)
	return 0;
// get the default colors
for (i = 0; i < gameData.segs.nVertices; i++, pvc++) {
#if 0
	if (pvc->index <= 0)
		continue;
#endif
	if ((pvc->color.red == 0) && (pvc->color.green == 0) && (pvc->color.blue == 0)) {
		pvc->index = 0;
		continue;
		}
	if ((pvc->color.red < 1) || (pvc->color.green < 1) || (pvc->color.blue < 1))
		bColored = 1;
	}
return bColored;
}

//------------------------------------------------------------------------------

void LoadTexColorsCompiled (int i, CFILE *loadFile)
{
	int			j;

// get the default colors
if (gameStates.app.bD2XLevel) {
	INIT_PROGRESS_LOOP (i, j, MAX_WALL_TEXTURES);
	for (; i < j; i++)
		ReadColor (gameData.render.color.textures + i, loadFile, gameData.segs.nLevelVersion <= 15, gameOpts->render.bDynLighting);
	}
}

//------------------------------------------------------------------------------

void LoadSideLightsCompiled (int i, CFILE *loadFile)
{
	tFaceColor	*pc;
	int			j;

gameData.render.shadows.nLights = 0;
#if LIGHTMAPS
if (gameStates.app.bD2XLevel) { 
	INIT_PROGRESS_LOOP (i, j, gameData.segs.nSegments * 6);
	pc = gameData.render.color.lights + i;
	for (; i < j; i++, pc++) {
		ReadColor (pc, loadFile, gameData.segs.nLevelVersion <= 13, 1);
#if 0//SHADOWS
		RegisterLight (pc, (short) (i / 6), (short) (i % 6));
#endif
		}
	}
else {
#else
	{
#endif
#if 0//SHADOWS
	tSegment	*segP;
	tSide		*sideP;
	int		h;

	INIT_PROGRESS_LOOP (i, j, gameData.segs.nSegments);
	segP = gameData.segs.segments + i;
	for (i = 0; i < j; i++, segP++)
		for (h = 0, sideP = segP->sides; h < 6; h++, sideP++)
			if (IsLight (sideP->nBaseTex) || IsLight (sideP->nOvlTexf))
				RegisterLight (NULL, (short) i, (short) h);
#endif
	}
}

//------------------------------------------------------------------------------

void ComputeSegSideCenters (int nSegment)
{
	int			i, j, nSide;
	tSegment		*segP;
	tSide			*sideP;
#if CALC_SEGRADS
	fix			xSideDists [6], xMinDist, xMaxDist, xDist;
	short			k;
	vmsVector	v, vMin, vMax;
#endif

INIT_PROGRESS_LOOP (nSegment, j, gameData.segs.nSegments);

for (i = nSegment * 6, segP = gameData.segs.segments + nSegment; nSegment < j; nSegment++, segP++) {
	ComputeSegmentCenter (gameData.segs.segCenters [0] + nSegment, segP);
#if CALC_SEGRADS
	GetSideDists (gameData.segs.segCenters [0] + nSegment, nSegment, xSideDists, 0);
	xMinDist = 0x7fffffff;
#endif
	for (nSide = 0, sideP = segP->sides; nSide < 6; nSide++, i++) {
		ComputeSideCenter (gameData.segs.sideCenters + i, segP, nSide);
#if CALC_SEGRADS
		if (xMinDist > xSideDists [nSide])
			xMinDist = xSideDists [nSide];
#endif
		}
#if CALC_SEGRADS
	gameData.segs.segRads [0][nSegment] = xMinDist;
	vMin.p.x = vMin.p.y = vMin.p.z = 0x7FFFFFFF;
	vMax.p.x = vMax.p.y = vMax.p.z = -0x7FFFFFFF;
	for (k = 0, xMaxDist = 0; k < 8; k++) {
		VmVecSub (&v, gameData.segs.segCenters [0] + nSegment, gameData.segs.vertices + segP->verts [k]);
		if (vMin.p.x > v.p.x)
			vMin.p.x = v.p.x;
		if (vMin.p.y > v.p.y)
			vMin.p.y = v.p.y;
		if (vMin.p.z > v.p.z)
			vMin.p.z = v.p.z;
		if (vMax.p.x < v.p.x)
			vMax.p.x = v.p.x;
		if (vMax.p.y < v.p.y)
			vMax.p.y = v.p.y;
		if (vMax.p.z < v.p.z)
			vMax.p.z = v.p.z;
		xDist = VmVecMag (&v);
		if (xMaxDist < xDist)
			xMaxDist = xDist;
		}
	gameData.segs.segRads [1][nSegment] = xMaxDist;
	gameData.segs.extent [nSegment].vMin = vMin;
	gameData.segs.extent [nSegment].vMax = vMax;
#endif
	}
}

//------------------------------------------------------------------------------

inline int SetVertVis (short nSegment, short nVertex, ubyte b)
{
	static int bSemaphore = 0;

if (bSemaphore)
	return 0;
bSemaphore = 1;
if (gameData.segs.bVertVis)
	gameData.segs.bVertVis [nSegment * VERTVIS_FLAGS + (nVertex >> 3)] |= (1 << (nVertex & 7));
bSemaphore = 0;
return 1;
}

//------------------------------------------------------------------------------

inline int SetSegVis (short nSrcSeg, short nDestSeg)
{
	static int bSemaphore = 0;

if (bSemaphore)
	return 0;
bSemaphore = 1;
gameData.segs.bSegVis [nSrcSeg * SEGVIS_FLAGS + (nDestSeg >> 3)] |= (1 << (nDestSeg & 7));
bSemaphore = 0;
return 1;
}

//------------------------------------------------------------------------------

inline int IsSegVert (short nSegment, int nVertex)
{
	int	i;
	short	*psv;

if (nSegment < 0)
	return 0;
for (i = 8, psv = gameData.segs.segments [nSegment].verts; i; i--, psv++)
	if (nVertex == *psv)
		return 1;
return 0;
}

//------------------------------------------------------------------------------
// Check segment to segment visibility by calling the renderer's visibility culling routine
// Do this for each side of the current segment, using the side normal(s) as forward vector
// of the viewer

void ComputeSingleSegmentVisibility (short nStartSeg)
{
	tSide			*sideP;
	short			nSegment, nSide, i, j;
	vmsVector	vNormal;
	vmsAngVec	vAngles;
	tObject		viewer;

//PrintLog ("computing visibility of segment %d\n", nStartSeg);
gameStates.ogl.bUseTransform = 1;
#ifdef _DEBUG
if (nStartSeg == nDbgSeg)
	nDbgSeg = nDbgSeg;
#endif
gameData.objs.viewer = &viewer;
viewer.nSegment = nStartSeg;
COMPUTE_SEGMENT_CENTER_I (&viewer.position.vPos, nStartSeg);
for (sideP = SEGMENTS [nStartSeg].sides, nSide = 6; nSide; nSide--, sideP++) {
	VmVecAdd (&vNormal, sideP->normals, sideP->normals + 1);
	VmVecScale (&vNormal, -F1_0 / 2);
	VmExtractAnglesVector (&vAngles, &vNormal);
	VmAngles2Matrix (&viewer.position.mOrient, &vAngles);
	G3StartFrame (0, 0);
	RenderStartFrame ();
	G3SetViewMatrix (&viewer.position.vPos, &viewer.position.mOrient, gameStates.render.xZoom, 1);
	BuildRenderSegList (nStartSeg, 0);	
	G3EndFrame ();
	//PrintLog ("   flagging visible segments\n");
	for (i = 0; i < gameData.render.mine.nRenderSegs; i++) {
		if (0 > (nSegment = gameData.render.mine.nSegRenderList [i]))
			continue;
#ifdef _DEBUG
		if (nSegment >= gameData.segs.nSegments)
			continue;
#endif
		if (SEGVIS (nStartSeg, nSegment))
			continue;
		while (!SetSegVis (nStartSeg, nSegment))
			;
#if 1
		tSegFaces	*segFaceP = SEGFACES + nSegment;
		grsFace		*faceP;
		grsTriangle	*triP;
		int			nFaces, nTris;

		for (nFaces = segFaceP->nFaces, faceP = segFaceP->pFaces; nFaces; nFaces--, faceP++) {
#ifdef _DEBUG
		if ((nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide == nDbgSide)))
			nSegment = nSegment;
#endif
			if (gameStates.render.bTriangleMesh) {
				for (nTris = faceP->nTris, triP = gameData.segs.faces.tris + faceP->nTriIndex; nTris; nTris--, triP++)
					for (j = 0; j < 3; j++)
						while (!SetVertVis (nStartSeg, triP->index [j], 1))
							;
				}
			else {
				for (j = 0; j < 4; j++)
					while (!SetVertVis (nStartSeg, faceP->index [j], 1))
						;
				}	
			}
#else
		for (j = 8, vertP = SEGMENTS [nSegment].verts; j; j--, vertP++) {
#ifdef _DEBUG
			if ((*vertP < 0) || (*vertP >= gameData.segs.nVertices))
				continue;
#endif
			while (!SetVertVis (nStartSeg, *vertP, 1))
				;
			}
#endif
		}
	}
gameStates.ogl.bUseTransform = 0;
}

//------------------------------------------------------------------------------

void ComputeSegmentVisibility (int startI)
{
	int			i, endI;

PrintLog ("computing segment visibility (%d)\n", startI);
if (startI <= 0) {
	i = sizeof (*gameData.segs.bVertVis) * gameData.segs.nVertices * VERTVIS_FLAGS;
	if (!(gameData.segs.bVertVis = (ubyte *) D2_ALLOC (i)))
		return;
	memset (gameData.segs.bVertVis, 0, i);
	memset (gameData.segs.bSegVis, 0, sizeof (*gameData.segs.bSegVis) * gameData.segs.nSegments * SEGVIS_FLAGS);
	}
else if (!gameData.segs.bVertVis)
	return;
if (gameStates.app.bMultiThreaded) {
	endI = startI ? gameData.segs.nSegments : gameData.segs.nSegments / 2;
	}
else
	INIT_PROGRESS_LOOP (startI, endI, gameData.segs.nSegments);
if (startI < 0)
	startI = 0;
// every segment can see itself and its neighbours
for (i = startI; i < endI; i++)
	ComputeSingleSegmentVisibility (i);
}


//------------------------------------------------------------------------------

static int loadIdx = 0;
static int loadOp = 0;
static CFILE *mineDataFile;

static void LoadSegmentsPoll (int nItems, tMenuItem *m, int *key, int cItem)
{
	int	bLightmaps = 0, bShadows = 0;

#if LIGHTMAPS
if (gameStates.app.bD2XLevel && gameStates.render.color.bLightMapsOk)
	bLightmaps = 1;
#endif
#if SHADOWS
bShadows = 1;
#endif

GrPaletteStepLoad (NULL);
if (loadOp == 0) {
	LoadSegmentsCompiled (loadIdx, mineDataFile);
	loadIdx += PROGRESS_INCR;
	if (loadIdx >= gameData.segs.nSegments) {
		loadIdx = 0;
		loadOp = 1;
		}
	}
else if (loadOp == 1) {
	ValidateSegmentAll ();			// Fill in tSide nType and normals.
	loadOp = 2;
	}
else if (loadOp == 2) {
	LoadSegment2sCompiled (mineDataFile);
	loadOp = 3;
	}
else if (loadOp == 3) {
	LoadVertLightsCompiled (loadIdx, mineDataFile);
	loadIdx += PROGRESS_INCR;
	if (!gameStates.app.bD2XLevel || (loadIdx >= gameData.segs.nVertices)) {
		loadIdx = 0;
		loadOp = 4;
		}
	}
else if (loadOp == 4) {
	LoadSideLightsCompiled (loadIdx, mineDataFile);
	loadIdx += PROGRESS_INCR;
	if (loadIdx >= (gameStates.app.bD2XLevel ? 
						 gameData.segs.nSegments * 6 : bShadows ? gameData.segs.nSegments : 1)) {
		loadIdx = 0;
		loadOp = 5;
		}
	}
else if (loadOp == 5) {
	LoadTexColorsCompiled (loadIdx, mineDataFile);
	loadIdx += PROGRESS_INCR;
	if (!gameStates.app.bD2XLevel || (loadIdx >= MAX_WALL_TEXTURES)) {
		loadIdx = 0;
		loadOp = 6;
		}
	}
else if (loadOp == 6) {
	ComputeSegSideCenters (loadIdx);
	loadIdx += PROGRESS_INCR;
	if (loadIdx >= gameData.segs.nSegments) {
		loadIdx = 0;
		loadOp = 7;
		}
	}
else {
	*key = -2;
	GrPaletteStepLoad (NULL);
	return;
	}
m [0].value++;
m [0].rebuild = 1;
*key = 0;
GrPaletteStepLoad (NULL);
}

//------------------------------------------------------------------------------

static void SortLightsPoll (int nItems, tMenuItem *m, int *key, int cItem)
{
GrPaletteStepLoad (NULL);
if (loadOp == 0) {
	ComputeSegmentVisibility (loadIdx);
	loadIdx += PROGRESS_INCR;
	if (loadIdx >= gameData.segs.nSegments) {
		loadIdx = 0;
		loadOp = 1;
		}
	}
if (loadOp == 1) {
	ComputeNearestSegmentLights (loadIdx);
	loadIdx += PROGRESS_INCR;
	if (loadIdx >= gameData.segs.nSegments) {
		loadIdx = 0;
		loadOp = 2;
		}
	}
else if (loadOp == 2) {
	ComputeNearestVertexLights (loadIdx);
	loadIdx += PROGRESS_INCR;
	if (loadIdx >= gameData.segs.nVertices) {
		loadIdx = 0;
		loadOp = 3;
		}
	}
if (loadOp == 3) {
	*key = -2;
	GrPaletteStepLoad (NULL);
	return;
	}
m [0].value++;
m [0].rebuild = 1;
*key = 0;
GrPaletteStepLoad (NULL);
}

//------------------------------------------------------------------------------

int SortLightsGaugeSize (void)
{
if (gameStates.app.bNostalgia)
	return 0;
if (gameStates.app.bMultiThreaded ||
	 !(gameOpts->render.bDynLighting || 
	  (gameOpts->render.color.bAmbientLight && !gameStates.render.bColored) ||
	   gameStates.app.bEnableShadows))
	return 0;
return
#if !SHADOWS
	(!SHOW_DYN_LIGHT && gameStates.app.bD2XLevel) ? 0 :
#endif
	PROGRESS_STEPS (gameData.segs.nSegments) * 2 +
	PROGRESS_STEPS (gameData.segs.nVertices)
	;
}

//------------------------------------------------------------------------------

int LoadMineGaugeSize (void)
{
	int	i = 2 * PROGRESS_STEPS (gameData.segs.nSegments) + 2;
	int	bLightmaps = 0, bShadows = 0;

#if LIGHTMAPS
	if (gameStates.render.color.bLightMapsOk)
		bLightmaps = 1;
#endif
#if SHADOWS
	bShadows = 1;
#endif
if (gameStates.app.bD2XLevel) {
	i += PROGRESS_STEPS (gameData.segs.nVertices) + PROGRESS_STEPS (MAX_WALL_TEXTURES);
	i += PROGRESS_STEPS (gameData.segs.nSegments * 6);
	}
else {
	i++;
	if (bShadows)
		i += PROGRESS_STEPS (gameData.segs.nSegments);
	else
		i++;
	}
return i;
}

//------------------------------------------------------------------------------

void LoadSegmentsGauge (CFILE *loadFile)
{
loadOp = 0;
loadIdx = 0;
mineDataFile = loadFile;
NMProgressBar (TXT_PREP_DESCENT, 0, LoadMineGaugeSize () + PagingGaugeSize () + SortLightsGaugeSize (), LoadSegmentsPoll); 
}

//------------------------------------------------------------------------------

typedef struct tPreFileHeader {
	int	nVersion;
	int	nSegments;
	int	nVertices;
	int	nLights;
	int	nMaxLightRange;
	int	nMethod;
	} tPreFileHeader;

int LoadPrecompiledLights (int nLevel)
{
	CFILE				cf;
	tPreFileHeader	pfh;
	int				bOk;
	char				szFilename [FILENAME_LEN], szFullname [FILENAME_LEN];

if (!gameStates.app.bCacheLights)
	return 0;
CFSplitPath (*gameHogFiles.AltHogFiles.szName ? gameHogFiles.AltHogFiles.szName : 
				 gameStates.app.bD1Mission ? gameHogFiles.D1HogFiles.szName : gameHogFiles.D2HogFiles.szName, 
				 NULL, szFilename, NULL);
if (nLevel < 0)
	sprintf (szFullname, "%s-s%d.pre", szFilename, -nLevel);
else
	sprintf (szFullname, "%s-%d.pre", szFilename, nLevel);
if (!CFOpen (&cf, szFullname, gameFolders.szTempDir, "rb", 0))
	return 0;
bOk = (CFRead (&pfh, sizeof (pfh), 1, &cf) == 1);
if (bOk)
	bOk = (pfh.nVersion == LIGHT_VERSION) && 
			(pfh.nSegments == gameData.segs.nSegments) && 
			(pfh.nVertices == gameData.segs.nVertices) && 
			(pfh.nLights == gameData.render.lights.dynamic.nLights) && 
			(pfh.nMaxLightRange == MAX_LIGHT_RANGE) &&
			(pfh.nMethod = LightingMethod ());
if (bOk)
	bOk = 
			(CFRead (gameData.segs.bSegVis, sizeof (ubyte) * pfh.nSegments * SEGVIS_FLAGS, 1, &cf) == 1) &&
#if 0
			(CFRead (gameData.segs.bVertVis, sizeof (ubyte) * pfh.nVertices * VERTVIS_FLAGS, 1, &cf) == 1) &&
#endif
			(CFRead (gameData.render.lights.dynamic.nNearestSegLights, sizeof (short) * pfh.nSegments * MAX_NEAREST_LIGHTS, 1, &cf) == 1) &&
			(CFRead (gameData.render.lights.dynamic.nNearestVertLights, sizeof (short) * pfh.nVertices * MAX_NEAREST_LIGHTS, 1, &cf) == 1);
CFClose (&cf);
return bOk;
}

//------------------------------------------------------------------------------

int SavePrecompiledLights (int nLevel)
{
	CFILE				cf;
	tPreFileHeader pfh = {LIGHT_VERSION, 
								 gameData.segs.nSegments, 
								 gameData.segs.nVertices, 
								 gameData.render.lights.dynamic.nLights, 
								 MAX_LIGHT_RANGE, 
								 LightingMethod ()};
	int				bOk;
	char				szFilename [FILENAME_LEN], szFullname [FILENAME_LEN];

if (!gameStates.app.bCacheLights)
	return 0;
CFSplitPath (*gameHogFiles.AltHogFiles.szName ? gameHogFiles.AltHogFiles.szName : 
				 gameStates.app.bD1Mission ? gameHogFiles.D1HogFiles.szName : gameHogFiles.D2HogFiles.szName, 
				 NULL, szFilename, NULL);
if (nLevel < 0)
	sprintf (szFullname, "%s-s%d.pre", szFilename, -nLevel);
else
	sprintf (szFullname, "%s-%d.pre", szFilename, nLevel);
if (!CFOpen (&cf, szFullname, gameFolders.szTempDir, "wb", 0))
	return 0;
bOk = (CFWrite (&pfh, sizeof (pfh), 1, &cf) == 1) &&
		(CFWrite (gameData.segs.bSegVis, sizeof (ubyte) * pfh.nSegments * SEGVIS_FLAGS, 1, &cf) == 1) &&
#if 0
		(CFWrite (gameData.segs.bVertVis, sizeof (ubyte) * pfh.nVertices * VERTVIS_FLAGS, 1, &cf) == 1) &&
#endif
		(CFWrite (gameData.render.lights.dynamic.nNearestSegLights, sizeof (short) * pfh.nSegments * MAX_NEAREST_LIGHTS, 1, &cf) == 1) &&
		(CFWrite (gameData.render.lights.dynamic.nNearestVertLights, sizeof (short) * pfh.nVertices * MAX_NEAREST_LIGHTS, 1, &cf) == 1);
CFClose (&cf);
return bOk;
}

//------------------------------------------------------------------------------

#if MULTI_THREADED_PRECALC

static tThreadInfo	ti [2];

//------------------------------------------------------------------------------

int _CDECL_ SegLightsThread (void *pThreadId)
{
	int		nId = *((int *) pThreadId);

ComputeNearestSegmentLights (nId ? gameData.segs.nSegments / 2 : 0);
SDL_SemPost (ti [nId].done);
ti [nId].bDone = 1;
return 0;
}

//------------------------------------------------------------------------------

int _CDECL_ VertLightsThread (void *pThreadId)
{
	int		nId = *((int *) pThreadId);

ComputeNearestVertexLights (nId ? gameData.segs.nVertices / 2 : 0);
SDL_SemPost (ti [nId].done);
ti [nId].bDone = 1;
return 0;
}

//------------------------------------------------------------------------------

static void StartOglLightThreads (pThreadFunc pFunc)
{
	int	i;

for (i = 0; i < 2; i++) {
	ti [i].bDone = 0;
	ti [i].done = SDL_CreateSemaphore (0);
	ti [i].nId = i;
	ti [i].pThread = SDL_CreateThread (pFunc, &ti [i].nId);
	}
#if 1
SDL_SemWait (ti [0].done);
SDL_SemWait (ti [1].done);
#else
while (!(ti [0].bDone && ti [1].bDone))
	G3_SLEEP (0);
#endif
for (i = 0; i < 2; i++) {
	SDL_WaitThread (ti [i].pThread, NULL);
	SDL_DestroySemaphore (ti [i].done);
	}
}

#endif //MULTI_THREADED_PRECALC

//------------------------------------------------------------------------------

void ComputeNearestLights (int nLevel)
{
if (gameStates.app.bNostalgia)
	return;
if (!(SHOW_DYN_LIGHT || 
	  (gameOpts->render.color.bAmbientLight && !gameStates.render.bColored) ||
	   (gameStates.app.bEnableShadows && !COMPETITION)))
	return;
loadOp = 0;
loadIdx = 0;
PrintLog ("Looking for precompiled light data\n");
if (LoadPrecompiledLights (nLevel))
	return;
else 
#if MULTI_THREADED_PRECALC
if (gameStates.app.bMultiThreaded && (gameData.segs.nSegments > 15)) {
	gameData.physics.side.bCache = 0;
	PrintLog ("Computing segment visibility\n");
	ComputeSegmentVisibility (-1);
	PrintLog ("Starting segment light calculation threads\n");
	StartOglLightThreads (SegLightsThread);
	PrintLog ("Starting vertex light calculation threads\n");
	StartOglLightThreads (VertLightsThread);
	gameData.physics.side.bCache = 1;
	}
else {
	int bMultiThreaded = gameStates.app.bMultiThreaded;
	gameStates.app.bMultiThreaded = 0;
#endif
	if (gameStates.app.bProgressBars && gameOpts->menus.nStyle)
		NMProgressBar (TXT_PREP_DESCENT, 
							LoadMineGaugeSize () + PagingGaugeSize (), 
							LoadMineGaugeSize () + PagingGaugeSize () + SortLightsGaugeSize (), SortLightsPoll); 
	else {
		PrintLog ("Computing segment visibility\n");
		ComputeSegmentVisibility (-1);
		PrintLog ("Computing segment lights\n");
		ComputeNearestSegmentLights (-1);
		PrintLog ("Computing vertex lights\n");
		ComputeNearestVertexLights (-1);
		}
	gameStates.app.bMultiThreaded = bMultiThreaded;
	}
D2_FREE (gameData.segs.bVertVis);
PrintLog ("Saving precompiled light data\n");
SavePrecompiledLights (nLevel);
}

//------------------------------------------------------------------------------

int LoadMineSegmentsCompiled (CFILE *loadFile)
{
	int			i;
	ubyte			nCompiledVersion;
	char			*psz;

gameStates.render.bColored = 0;
bD1PigPresent = CFExist (D1_PIGFILE, gameFolders.szDataDir, 0);
psz = strchr (gameData.segs.szLevelFilename, '.');
bNewFileFormat = !psz || strcmp (psz, ".sdl");
//	For compiled levels, textures map to themselves, prevent nTexOverride always being gray, 
//	bug which Matt and John refused to acknowledge, so here is Mike, fixing it.
#ifdef EDITOR
for (i = 0; i < MAX_TEXTURES; i++)
	tmap_xlate_table [i] = i;
#endif

//	memset ( gameData.segs.segments, 0, sizeof (tSegment)*MAX_SEGMENTS );
FuelCenReset ();
InitTexColors ();
//=============================== Reading part ==============================
nCompiledVersion = CFReadByte (loadFile);
//Assert ( nCompiledVersion==COMPILED_MINE_VERSION );
#if TRACE
if (nCompiledVersion != COMPILED_MINE_VERSION)
	con_printf (CONDBG, "compiled mine version=%i\n", nCompiledVersion); //many levels have "wrong" versions.  Theres no point in aborting because of it, I think.
con_printf (CONDBG, "   compiled mine version = %d\n", nCompiledVersion);
#endif
gameData.segs.nVertices = bNewFileFormat ? CFReadShort (loadFile) : CFReadInt (loadFile);
Assert (gameData.segs.nVertices <= MAX_VERTICES);
#if TRACE
con_printf (CONDBG, "   %d vertices\n", gameData.segs.nVertices);
#endif
gameData.segs.nSegments = bNewFileFormat ? CFReadShort (loadFile) : CFReadInt (loadFile);
if (gameData.segs.nSegments >= MAX_SEGMENTS) {
	Warning (TXT_LEVEL_TOO_LARGE);
	return -1;
	}
#if TRACE
con_printf (CONDBG, "   %d segments\n", gameData.segs.nSegments);
#endif
for (i = 0; i < gameData.segs.nVertices; i++) {
	CFReadVector (gameData.segs.vertices + i, loadFile);
#if !FLOAT_COORD
	gameData.segs.fVertices [i].p.x = f2fl (gameData.segs.vertices [i].p.x);
	gameData.segs.fVertices [i].p.y = f2fl (gameData.segs.vertices [i].p.y);
	gameData.segs.fVertices [i].p.z = f2fl (gameData.segs.vertices [i].p.z);
#endif
	}
memset (gameData.segs.segments, 0, MAX_SEGMENTS * sizeof (tSegment));
#if TRACE
con_printf (CONDBG, "   loading segments ...\n");
#endif
gameData.segs.nLastVertex = gameData.segs.nVertices - 1;
gameData.segs.nLastSegment = gameData.segs.nSegments - 1;
if (gameStates.app.bProgressBars && gameOpts->menus.nStyle)
	LoadSegmentsGauge (loadFile);
else {
	LoadSegmentsCompiled (-1, loadFile);
	ValidateSegmentAll ();			// Fill in side type and normals.
	LoadSegment2sCompiled (loadFile);
	LoadVertLightsCompiled (-1, loadFile);
	LoadSideLightsCompiled (-1, loadFile);
	LoadTexColorsCompiled (-1, loadFile);
	ComputeSegSideCenters (-1);
	}
if (!(gameStates.app.bNostalgia || gameStates.render.bColored || (gameStates.render.bColored = HasColoredLight ())))
	InitTexColors ();
ResetObjects (1);		//one tObject, the player
return 0;
}

//------------------------------------------------------------------------------
//eof
