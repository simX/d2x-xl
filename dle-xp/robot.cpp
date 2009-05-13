// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <io.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "dlc.h"
#include "mine.h"
#include "global.h"
#include "toolview.h"
#include "io.h"
#include "file.h"

//------------------------------------------------------------------------
// ReadHamFile()
//
// Actions
//   1)	Reads all robot data from a HAM file
//   2) Memory will be allocated for polymodel data.  It is the
//	responsibility of the caller to free all non-null polymodel
//	data pointer.
//   3) If polymodel data is allocated aready, it will be freed and
//	reallocated.
//
// Parameters
//	fname - handle of file to read
//      type - type of file (0=DESCENT2.HAM, 1=extended robot HAM file)
//
// Globals used
//
//  N_robot_types
//  Robot_info[MAX_ROBOT_TYPES]
//
//  N_robot_joints
//  Robot_joints[MAX_ROBOT_JOINTS]
//
//  N_polygon_models
//  Polygon_models[MAX_POLYGON_MODELS]
//  Dying_modelnums[N_D2_POLYGON_MODELS]
//  Dead_modelnums[N_D2_POLYGON_MODELS]
//
//  N_object_bitmaps
//  ObjBitmaps[MAX_OBJ_BITMAPS]
//  ObjBitmapPtrs[MAX_OBJ_BITMAPS]
//
//
// Assumptions
//   1) Memory was allocated for globals (except polymodel data)
//------------------------------------------------------------------------

#define MAKESIG(_sig)	(UINT32) *((int *) &(_sig))

int CMine::ReadHamFile(char *pszFile, int type) 
{
  FILE *fp;
  INT16 t,t0;
  UINT32 id;
  POLYMODEL pm;
  char szFile [256];
  static char d2HamSig [4] = {'H','A','M','!'};
  static char d2xHamSig [4] = {'M','A','H','X'};

if (!pszFile) {
	if (file_type == RL2_FILE) {
		FSplit (descent2_path, szFile, NULL, NULL);
		strcat (szFile, "descent2.ham");
		}
	else {
		FSplit (descent_path, szFile, NULL, NULL);
		strcat (szFile, "descent.ham");
		}
	pszFile = szFile;
	}

  pm.n_models = 0;
  fp = fopen(pszFile,"rb");
  if (!fp) {
    sprintf(message," Ham manager: Cannot open robot file <%s>.", pszFile);
    DEBUGMSG (message);
    goto abort;
  }

// The extended HAM only contains part of the normal HAM file.
// Therefore, we have to skip some items if we are reading
// a normal HAM cause we are only interested in reading
// the information which is found in the extended ham
// (the robot information)
if (type == NORMAL_HAM)  {
	id = read_INT32(fp); // "HAM!"
	if (id != MAKESIG (d2HamSig)) {//0x214d4148L) {
		sprintf(message,"Not a D2 HAM file (%d)",pszFile);
		ErrorMsg(message);
		goto abort;
		}
	read_INT32(fp); // version (0x00000007)
	t = (INT16) read_INT32(fp);
	fseek(fp,sizeof(BITMAP_INDEX)*t,SEEK_CUR);
	fseek(fp,sizeof(TMAP_INFO)*t,SEEK_CUR);
	t = (INT16) read_INT32(fp);
	fseek(fp,sizeof(UINT8)*t,SEEK_CUR);
	fseek(fp,sizeof(UINT8)*t,SEEK_CUR);
	t = (INT16) read_INT32(fp);
	fseek(fp,sizeof(VCLIP)*t,SEEK_CUR);
	t = (INT16) read_INT32(fp);
	fseek(fp,sizeof(ECLIP)*t,SEEK_CUR);
	t = (INT16) read_INT32(fp);
	fseek(fp,sizeof(WCLIP)*t,SEEK_CUR);
	}
else if (type == EXTENDED_HAM)  {
	id = read_INT32(fp); // "HAM!"
	if (id != MAKESIG (d2xHamSig)) {//0x214d4148L) {
		sprintf(message,"Not a D2X HAM file (%d)",pszFile);
		ErrorMsg(message);
		goto abort;
		}
	read_INT32(fp); //skip version
	t = read_INT32(fp); //skip weapon count
	fseek (fp, t * sizeof (WEAPON_INFO), SEEK_CUR); //skip weapon info
	}

  // read robot information
  //------------------------
  t = (INT16) read_INT32(fp);
  t0 = (type == NORMAL_HAM) ? 0: N_D2_ROBOT_TYPES;
  N_robot_types = t0 + t;
  if (N_robot_types > MAX_ROBOT_TYPES) {
    sprintf(message,"Too many robots (%d) in <%s>.  Max is %d.",t,pszFile,MAX_ROBOT_TYPES-N_D2_ROBOT_TYPES);
    ErrorMsg(message);
	 N_robot_types = MAX_ROBOT_TYPES;
	 t = N_robot_types - t0;
//    goto abort;
  }
  fread(RobotInfo (t0), sizeof(ROBOT_INFO), t, fp );
  memcpy (DefRobotInfo (t0), RobotInfo (t0), sizeof(ROBOT_INFO) * t);

  // skip joints weapons, and powerups
  //----------------------------------
  t = (INT16) read_INT32(fp);
  fseek(fp,sizeof(JOINTPOS)*t,SEEK_CUR);
  if (type == NORMAL_HAM) {
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof(WEAPON_INFO)*t,SEEK_CUR);
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof(POWERUP_TYPE_INFO)*t,SEEK_CUR);
  }

  // read poly model data and write it to a file
  //---------------------------------------------
  t = (INT16) read_INT32(fp);
  if (t > MAX_POLYGON_MODELS) {
    sprintf(message,"Too many polygon models (%d) in <%s>.  Max is %d.",t,pszFile,MAX_POLYGON_MODELS-N_D2_POLYGON_MODELS);
    ErrorMsg(message);
    goto abort;
  }

  INT16 i;
  FILE *file;
  file = fopen("d:\\bc\\dlc2data\\poly.dat","wt");
  if (file) {
    for (i=0; i<t; i++ ) {
      fread(&pm, sizeof(POLYMODEL), 1, fp );
      fprintf(file,"n_models        = %ld\n",pm.n_models);
      fprintf(file,"model_data_size = %ld\n",pm.model_data_size);
      for (int j=0;j<pm.n_models;j++) {
	fprintf(file,"submodel_ptrs[%d]    = %#08lx\n",j,pm.submodel_ptrs[j]);
	fprintf(file,"submodel_offsets[%d] = %#08lx %#08lx %#08lx\n",j,
		pm.submodel_offsets[j].x,pm.submodel_offsets[j].y,pm.submodel_offsets[j].z);
	fprintf(file,"submodel_norms[%d]   = %#08lx %#08lx %#08lx\n",j,
		pm.submodel_norms[j].x,pm.submodel_norms[j].y,pm.submodel_norms[j].z);
	fprintf(file,"submodel_pnts[%d]    = %#08lx %#08lx %#08lx\n",j,
		pm.submodel_pnts[j].x,pm.submodel_pnts[j].y,pm.submodel_pnts[j].z);
	fprintf(file,"submodel_rads[%d]    = %#08lx\n",j,pm.submodel_rads[j]);
	fprintf(file,"submodel_parents[%d] = %d\n",j,pm.submodel_parents[j]);
	fprintf(file,"submodel_mins[%d]    = %#08lx %#08lx %#08lx\n",j,
		pm.submodel_mins[j].x,pm.submodel_mins[j].y,pm.submodel_mins[j].z);
	fprintf(file,"submodel_maxs[%d]    = %#08lx %#08lx %#08lx\n",j,
		pm.submodel_maxs[j].x,pm.submodel_maxs[j].y,pm.submodel_maxs[j].z);
      }
      fprintf(file,"mins            = %#08lx %#08lx %#08lx\n",pm.mins.x,pm.mins.y,pm.mins.z);
      fprintf(file,"maxs            = %#08lx %#08lx %#08lx\n",pm.maxs.x,pm.maxs.y,pm.maxs.z);
      fprintf(file,"rad             = %ld\n",pm.rad);
      fprintf(file,"n_textures      = %d\n",pm.n_textures);
      fprintf(file,"first_texture   = %d\n",pm.first_texture);
      fprintf(file,"simpler_model   = %d\n\n",pm.simpler_model);
    }
    fclose(file);
  }

#if ALLOCATE_POLYMODELS
  // read joint information
  //-----------------------
  t = (INT16) read_INT32(fp);
  t0 = (type == NORMAL_HAM) ? 0: N_D2_ROBOT_JOINTS;
  N_robot_joints = t0 + t;
  if (N_robot_joints > MAX_ROBOT_JOINTS) {
    sprintf(message,"Too many robot joints (%d) in <%s>.  Max is %d.",t,pszFile,MAX_ROBOT_JOINTS-N_D2_ROBOT_JOINTS);
    ErrorMsg(message);
    goto abort;
  }
  fread( &Robot_joints[t0], sizeof(JOINTPOS), t, fp );

  // skip weapon and powerup data
  //-----------------------------
  if (type == NORMAL_HAM) {
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof(WEAPON_INFO)*t,SEEK_CUR);
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof(POWERUP_TYPE_INFO)*t,SEEK_CUR);
  }

  // read poly model data
  //---------------------
  t = (INT16) read_INT32(fp);
  t0 = (type == NORMAL_HAM) ? 0: N_D2_POLYGON_MODELS;
  N_polygon_models = t0 + t;
  if (N_polygon_models > MAX_POLYGON_MODELS) {
    sprintf(message,"Too many polygon models (%d) in <%s>.  Max is %d.",t,pszFile,MAX_POLYGON_MODELS-N_D2_POLYGON_MODELS);
    ErrorMsg(message);
    goto abort;
  }

  INT16 i;

  for (i=t0; i<t0+t; i++ ) {
    // free poly data memory if already allocated
    if (Polygon_models[i]->model_data != NULL ) {
      free((void *)Polygon_models[i]->model_data);
    }
    fread(Polygon_models[i], sizeof(POLYMODEL), 1, fp );
    fread(&Polygon_model, sizeof(POLYMODEL), 1, fp );
  }
  for (i=t0; i<t0+t; i++ ) {
    Polygon_models[i]->model_data = (UINT8 *) malloc((int)Polygon_models[i]->model_data_size);

    if (Polygon_models[i]->model_data == NULL ) {
      ErrorMsg("Could not allocate memory for polymodel data");
      goto abort;
    }
    fread( Polygon_models[i]->model_data, sizeof(UINT8), (INT16)Polygon_models[i]->model_data_size, fp );
//    g3_init_polygon_model(Polygon_models[i].model_data);
  }

  // extended hog writes over normal hogs dying models instead of adding new ones
  fread( &Dying_modelnums[t0], sizeof(INT32), t, fp );
  fread( &Dead_modelnums[t0], sizeof(INT32), t, fp );

  // skip gague data
  //----------------
  if (type == NORMAL_HAM) {
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof(BITMAP_INDEX)*t,SEEK_CUR); // lores gague
    fseek(fp,sizeof(BITMAP_INDEX)*t,SEEK_CUR); // hires gague
  }

  // read object bitmap data
  //------------------------
  // NOTE: this overwrites D2 object bitmap indices instead of
  // adding more bitmap texture indicies.  I believe that D2
  // writes all 600 indicies even though it doesn't use all
  // of them.
  //----------------------------------------------------------
  t = (INT16) read_INT32(fp);
  t0 = (type == NORMAL_HAM) ? 0: N_D2_OBJBITMAPS;
  if (type == NORMAL_HAM) {
    N_object_bitmaps  = t0 + t;  // only update this if we are reading Descent2.ham file
  }
  if (t+t0 > MAX_OBJ_BITMAPS) {
    sprintf(message,"Too many object bitmaps (%d) in <%s>.  Max is %d.",t,pszFile,MAX_OBJ_BITMAPS-N_D2_OBJBITMAPS);
    ErrorMsg(message);
    goto abort;
  }
  fread( &ObjBitmaps[t0], sizeof(BITMAP_INDEX), t, fp );

  if (type == EXTENDED_HAM) {
    t = (INT16) read_INT32(fp);
    t0 = (type == NORMAL_HAM) ? 0: N_D2_OBJBITMAPPTRS;
    if (t+t0 > MAX_OBJ_BITMAPS) {
      sprintf(message,"Too many object bitmaps pointer (%d) in <%s>.  Max is %d.",t,pszFile,MAX_OBJ_BITMAPS-N_D2_OBJBITMAPPTRS);
      ErrorMsg(message);
      goto abort;
    }
  }
  fread(&ObjBitmapPtrs[t0], sizeof(BITMAP_INDEX), t, fp );
#endif

  fclose(fp);
  return 0;
abort:
  if (fp) fclose(fp);
  return 1;
}


//------------------------------------------------------------------------
// ReadHxmFile()
//
// Actions
//   1)	Reads all robot data from a HXM file
//   2) Memory will be allocated for polymodel data.  It is the
//	responsibility of the caller to free all non-null polymodel
//	data pointer.
//   3) If polymodel data is allocated aready, it will be freed and
//	reallocated.
//
// Parameters
//	fp - pointer to file (already offset to the correct position)
//
// Globals used
//
//  N_robot_types
//  Robot_info[MAX_ROBOT_TYPES]
//
//  N_robot_joints
//  Robot_joints[MAX_ROBOT_JOINTS]
//
//  N_polygon_models
//  Polygon_models[MAX_POLYGON_MODELS]
//  Dying_modelnums[N_D2_POLYGON_MODELS]
//  Dead_modelnums[N_D2_POLYGON_MODELS]
//
//  N_object_bitmaps
//  ObjBitmaps[MAX_OBJ_BITMAPS]
//  ObjBitmapPtrs[MAX_OBJ_BITMAPS]
//
//
// Assumptions
//   1) Memory was allocated for globals (except polymodel data)
//------------------------------------------------------------------------

int CMine::ReadHxmFile(FILE *fp, long fSize) 
{
	UINT16 t,i,j;
	ROBOT_INFO rInfo;
	long p;

if (!fp) {
	ErrorMsg("Invalid file handle for reading HXM data.");
	goto abort;
	}

p = ftell (fp);
if (fSize < 0)
	fSize = filelength (fileno (fp));
UINT32 id;
id = read_INT32(fp); // "HXM!"
if (id != 0x21584d48L) {
	ErrorMsg("Not a HXM file");
	goto abort;
	}
if (m_pHxmExtraData) {
	free (m_pHxmExtraData);
	m_pHxmExtraData = NULL;
	m_nHxmExtraDataSize = 0;
	}
read_INT32(fp); // version (0x00000001)

// read robot information
//------------------------
t = (UINT16) read_INT32(fp);
for (j=0;j<t;j++) {
	i = (UINT16)read_INT32(fp);
	if (i>=N_robot_types) {
		sprintf(message,"Robots number (%d) out of range.  Range = [0..%d].",i,N_robot_types-1);
		ErrorMsg(message);
		goto abort;
		}
	fread(&rInfo, sizeof(ROBOT_INFO), 1, fp );
	// compare this to existing data
	if (memcmp(&rInfo,RobotInfo (i),sizeof(ROBOT_INFO)) != 0) {
		memcpy(RobotInfo (i),&rInfo,sizeof(ROBOT_INFO));
		RobotInfo (i)->pad[0] = 1; // mark as custom
		}
	}

m_nHxmExtraDataSize = fSize - ftell (fp) + p;
if (m_nHxmExtraDataSize > 0) {
	if (!(m_pHxmExtraData = (char*) malloc (m_nHxmExtraDataSize))) {
		ErrorMsg("Couldn't allocate extra data from hxm file.\nThis data will be lost when saving the level!");
		goto abort;
		}
	if (fread (m_pHxmExtraData, m_nHxmExtraDataSize, 1, fp) != 1) {
		ErrorMsg("Couldn't read extra data from hxm file.\nThis data will be lost when saving the level!");
		goto abort;
		}
	}
return 0;
abort:
//  if (fp) fclose(fp);
return 1;
}

//------------------------------------------------------------------------
// ()
//
// Actions
//   1)	Writes robot info in HXM file format
//
// Parameters
//	fp - pointer to file (already offset to the correct position)
//
// Globals used
//
//  N_robot_types
//  Robot_info[MAX_ROBOT_TYPES]
//------------------------------------------------------------------------

int CMine::WriteHxmFile(FILE *fp) 
{
UINT16 t,i;

for (i = 0,t = 0; i < N_robot_types; i++)
	if (IsCustomRobot (i))
		t++;
if (!(t || m_nHxmExtraDataSize))
	return 0;
if (!fp) {
	ErrorMsg("Invalid file handle for writing HXM data.");
	goto abort;
	}

#if 1
UINT32 id;
id = 0x21584d48L;    // "HXM!"
write_INT32(id,fp);
write_INT32(1,fp);   // version 1
#endif

// write robot information
//------------------------
write_INT32(t,fp); // number of robot info structs stored
for (i=0;i<N_robot_types;i++) {
	if (RobotInfo (i)->pad [0]) {
		write_INT32((UINT32)i,fp);
		fwrite(RobotInfo (i), sizeof(ROBOT_INFO), 1, fp );
		}
	}

#if 1
if (m_nHxmExtraDataSize)
	fwrite (m_pHxmExtraData, m_nHxmExtraDataSize, 1, fp);
else
#endif
// write zeros for the rest of the data
//-------------------------------------
{
write_INT32(0,fp);  //number of joints
write_INT32(0,fp);  //number of polygon models
write_INT32(0,fp);  //number of objbitmaps
write_INT32(0,fp);  //number of objbitmaps
write_INT32(0,fp);  //number of objbitmapptrs
}

if (t) {
	sprintf (message," Hxm manager: Saving %d custom robots",t);
	DEBUGMSG (message);
	}
fclose(fp);
return 0;

abort:

if (fp) 
	fclose(fp);
return 1;
}

//------------------------------------------------------------------------
// InitRobotData()
//------------------------------------------------------------------------

void CMine::InitRobotData() 
{
  ReadRobotResource(-1);
}


//------------------------------------------------------------------------
// ReadRobotResource() - reads robot.hxm from resource data into Robot_info[]
//
// if robot_number == -1, then it reads all robots
//------------------------------------------------------------------------
void CMine::ReadRobotResource(int robot_number) 
{
  UINT16 i,j,t;
  UINT8 *ptr;
  HRSRC hFind = FindResource( hInst,"ROBOT_HXM", "RC_DATA");
  HINSTANCE hInst = AfxGetApp ()->m_hInstance;
  HGLOBAL hResource = LoadResource( hInst, hFind);
  if (!hResource) {
    ErrorMsg("Could not find robot resource data");
    return;
  }
  ptr = (UINT8 *)LockResource(hResource);
  if (!ptr) {
    ErrorMsg("Could not lock robot resource data");
    return;
  }
  t = (UINT16)(*((UINT32 *)ptr));
  N_robot_types = min(t,MAX_ROBOT_TYPES);
  ptr += sizeof(UINT32);
  for (j=0;j<t;j++) {
    i = (UINT16)(*((UINT32 *)ptr));
    if (i>MAX_ROBOT_TYPES) break;
    ptr += sizeof(UINT32);
    // copy the robot info for one robot, or all robots
    if (j==robot_number || robot_number == -1) {
      memcpy(RobotInfo (i), ptr, sizeof(ROBOT_INFO));
    }
    ptr += sizeof(ROBOT_INFO);
  }
  FreeResource(hResource);
}

//--------------------------------------------------------------------------
// has_custom_robots()
//--------------------------------------------------------------------------

bool CMine::IsCustomRobot (int i)
{
	bool	bFound = false;
	CDSegment *seg;
	CDObject *obj;
	int j;

if (!RobotInfo (i)->pad [0]) //changed?
	return false;
	// check if actually different from defaults
UINT8 pad = DefRobotInfo (i)->pad [0];
DefRobotInfo (i)->pad [0] = RobotInfo (i)->pad [0]; //make sure it's equal for the comparison
if (memcmp (RobotInfo (i), DefRobotInfo (i), sizeof (ROBOT_INFO))) { //they're different
	// find a robot of that type
	for (j = GameInfo ().objects.count, obj = Objects (); j; j--, obj++)
		if ((obj->type == OBJ_ROBOT) && (obj->id == i))
			break;
	if (j) // found one
		bFound = true;
	else { //no robot of that type present
		// find a matcen producing a robot of that type
		for (j = SegCount (), seg = Segments (); j; j--, seg++)
			if (seg->special == SEGMENT_IS_ROBOTMAKER) {
				int matcen = seg->matcen_num;
				if ((i < 32) ?
					 BotGens (matcen)->objFlags [0] & (1L << i) :
					 BotGens (matcen)->objFlags [1] & (1L << (i-32)))
					break;
				}
		if (j) // found one
			bFound = true;
		else
			RobotInfo (i)->pad [0] = 0; // no matcens or none producing that robot type
		}
	}
else
	RobotInfo (i)->pad [0] = 0; //same as default
DefRobotInfo (i)->pad [0] = pad; //restore
return bFound;
}

//--------------------------------------------------------------------------
// has_custom_robots()
//--------------------------------------------------------------------------

BOOL CMine::HasCustomRobots() 
{
for (int i = 0; i < (int) N_robot_types; i++)
	if (IsCustomRobot (i))
		return TRUE;
return (m_nHxmExtraDataSize > 0);
}

//eof robot.cpp


