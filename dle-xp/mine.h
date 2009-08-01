#ifndef __mine_h
#define __mine_h

#include "types.h"
#include "segment.h"

#define MAX_LIGHT_DEPTH 6

// external data
extern UINT8 side_vert[6][4];
extern UINT8 opp_side[6];
extern UINT8 opp_side_vert[6][4];
extern UINT8 line_vert[12][2];
extern UINT8 side_line[6][4];
extern UINT8 connect_points[8][3];
extern INT8 point_sides[8][3];
extern INT8 point_corners[8][3];
extern TEXTURE_LIGHT d1_texture_light[NUM_LIGHTS_D1];
extern TEXTURE_LIGHT d2_texture_light[NUM_LIGHTS_D2];
extern uvl default_uvls[4];

// Copyright (C) 1997 Bryan Aamot
//**************************************************************************
// CLASS - Level
//**************************************************************************

typedef struct tMineData {
	game_info					game_fileinfo;
	
	INT32							m_reactor_time;
	INT32							m_reactor_strength;
	INT32							m_secret_cubenum;
	vms_matrix					m_secret_orient;
	
	// robot data
	ROBOT_INFO					Robot_info[MAX_ROBOT_TYPES];
	
	// structure data
	UINT16						numVertices;
	vms_vector					vertices[MAX_VERTICES3];
	
	UINT16						numSegments;
	CDSegment					segments[MAX_SEGMENTS3];
	CDColor 						lightColors [MAX_SEGMENTS3][6];
	CDColor						texColors [MAX_D2_TEXTURES];
	CDColor 						sideColors [MAX_SEGMENTS3][6];
	CDColor						vertexColors [MAX_VERTICES3];
	
	UINT8							vert_status[MAX_VERTICES3];
	
	CDWall						walls[MAX_WALLS3];
	active_door					active_doors[MAX_DOORS];
	CDTrigger					triggers[MAX_TRIGGERS2];
	CDTrigger					objTriggers[MAX_OBJ_TRIGGERS];
	CDObjTriggerList			objTriggerList[MAX_OBJ_TRIGGERS];
	short							objTriggerRoot [MAX_OBJECTS2];
	int							numObjTriggers;
	control_center_trigger	control_center_triggers[MAX_CONTROL_CENTER_TRIGGERS];
	matcen_info					robot_centers[MAX_NUM_MATCENS2];
	matcen_info					equip_centers[MAX_NUM_MATCENS2];
	
	// object data
	CDObject						objects[MAX_OBJECTS2];
	
	// light data
	dl_index						dl_indices[MAX_DL_INDICES_D2X];
	delta_light					delta_lights[MAX_DELTA_LIGHTS_D2X];
	
	// flickering light
	INT16							N_flickering_lights;
	FLICKERING_LIGHT			flickering_lights[MAX_FLICKERING_LIGHTS];

	CDSelection					current1;
	CDSelection					current2;
	CDSelection					*current;

} MINE_DATA;

class CMine {
public:
	
	// level info
//	int							file_type;
//	INT32							level_version;
	char							current_level_name[256];	
	game_top_info				game_top_fileinfo;
#if 1
	MINE_DATA					m_mineData;
#else
	game_info					game_fileinfo;

	INT32							m_reactor_time;
	INT32							m_reactor_strength;
	INT32							m_secret_cubenum;
	vms_matrix					m_secret_orient;
	
	// robot data
	ROBOT_INFO					Robot_info[MAX_ROBOT_TYPES];
	
	// structure data
	UINT16						numVertices;
	vms_vector					vertices[MAX_VERTICES3];
	
	UINT16						numSegments;
	CDSegment					segments[MAX_SEGMENTS3];
	
	UINT8							vert_status[MAX_VERTICES3];
	
	CDWall						walls[MAX_WALLS3];
	active_door					active_doors[MAX_DOORS];
	CDTrigger					triggers[MAX_TRIGGERS];
	CDTrigger					objTriggers[MAX_OBJ_TRIGGERS];
	CDObjTriggerList			objTriggerList[MAX_OBJ_TRIGGERS];
	short							objTriggerRoot [MAX_OBJECTS2];
	int							numObjTriggers;
	control_center_trigger	control_center_triggers[MAX_CONTROL_CENTER_TRIGGERS];
	matcen_info					robot_centers[MAX_NUM_MATCENS];
	
	// object data
	CDObject						objects[MAX_OBJECTS];
	
	// light data
	dl_index						dl_indices[MAX_DL_INDICES];
	delta_light					delta_lights[MAX_DELTA_LIGHTS];
	
	// flickering light
	INT16							N_flickering_lights;
	FLICKERING_LIGHT			flickering_lights[MAX_FLICKERING_LIGHTS];

	CDSelection		current1;
	CDSelection		current2;
	CDSelection		*current;
#endif	
	ROBOT_INFO					def_robot_info[MAX_ROBOT_TYPES];
	// textures and palettes
//	HGLOBAL						texture_handle[MAX_D2_TEXTURES];
	HPALETTE						ThePalette;
	LPLOGPALETTE				dlcLogPalette;
	
	// strings
	char				message[256];
	char				starting_directory[256];
//	char				descent_path[256];
//	char				descent2_path[256];
//	char				levels_path[256];
	
	// selection
	
	// flags
	INT16				m_selectMode;
	int				disable_drawing;
	int				changes_made;
	
	bool				m_bSplineActive;
	BOOL				m_bSortObjects;
	int				m_nMaxSplines;
	int				m_nNoLightDeltas;
	int				m_lightRenderDepth;
	int				m_deltaLightRenderDepth;

	char				m_szBlockFile [256];
	INT16				m_splineLength1,
						m_splineLength2;
	bool				m_bVertigo;
	char				*m_pHxmExtraData;
	int				m_nHxmExtraDataSize;
	bool				m_bUseTexColors;
// Constructor/Desctuctor
public:
	CMine();
	~CMine();
	void Reset ();
	
public:
	inline MINE_DATA& MineData ()
		{ return m_mineData; }
	inline vms_vector *Vertices (int i = 0)
		{ return MineData ().vertices + i; }
	inline UINT8 *VertStatus (int i = 0)
		{ return MineData ().vert_status + i; }
	inline CDSegment *Segments (int i = 0)
		{ return MineData ().segments + i; }
	inline CDColor *VertexColors (int i = 0)
		{ return &(MineData ().vertexColors [i]); }
	inline CDWall *Walls (int i = 0)
		{ return MineData ().walls + i; }
	inline CDTrigger *Triggers (int i = 0)
		{ return MineData ().triggers + i; }
	inline int &NumTriggers ()
		{ return GameInfo ().triggers.count; }
	inline CDTrigger *ObjTriggers (int i = 0)
		{ return MineData ().objTriggers + i; }
	inline CDObjTriggerList *ObjTriggerList (int i = 0)
		{ return MineData ().objTriggerList + i; }
	inline short *ObjTriggerRoot (int i = 0)
		{ return MineData ().objTriggerRoot + i; }
	inline int& NumObjTriggers ()
		{ return MineData ().numObjTriggers; }
	inline CDObject *Objects (int i = 0)
		{ return MineData ().objects + i; }
	inline matcen_info *BotGens (int i = 0)
		{ return MineData ().robot_centers + i; }
	inline matcen_info *EquipGens (int i = 0)
		{ return MineData ().equip_centers + i; }
	inline control_center_trigger *CCTriggers (int i = 0)
		{ return MineData ().control_center_triggers + i; }
	inline active_door *ActiveDoors (int i = 0)
		{ return MineData ().active_doors + i; }
	inline ROBOT_INFO *RobotInfo (int i = 0)
		{ return MineData ().Robot_info + i; }
	inline ROBOT_INFO *DefRobotInfo (int i = 0)
		{ return def_robot_info + i; }
	inline game_info& GameInfo ()
		{ return MineData ().game_fileinfo; }
	inline UINT16& SegCount ()
		{ return MineData ().numSegments; }
	inline INT32& ObjCount ()
		{ return GameInfo ().objects.count; }
	inline UINT16& VertCount ()
		{ return MineData ().numVertices; }
	inline dl_index *DLIndex (int i = 0)
		{ return MineData ().dl_indices + i; }
	inline delta_light *DeltaLights (int i = 0)
		{ return MineData ().delta_lights + i; }
	inline FLICKERING_LIGHT *FlickeringLights (int i = 0)
		{ return MineData ().flickering_lights + i; }
	inline INT16& FlickerLightCount ()
		{ return MineData ().N_flickering_lights; }
	long TotalSize (game_item_info& gii)
		{ return (long) gii.size * (long) gii.count; }
	inline INT32& ReactorTime ()
		{ return MineData ().m_reactor_time; }
	inline INT32& ReactorStrength ()
		{ return MineData ().m_reactor_strength; }
	inline INT32& SecretCubeNum ()
		{ return MineData ().m_secret_cubenum; }
	inline vms_matrix& SecretOrient ()
		{ return MineData ().m_secret_orient; }
	inline CDSelection* &Current ()
		{ return MineData ().current; }
	inline CDSelection& Current1 ()
		{ return MineData ().current1; }
	inline CDSelection& Current2 ()
		{ return MineData ().current2; }
	inline CDSelection *Other (void)
		{ return (Current () == &Current2 ()) ? &Current1 () : &Current2 (); }
	inline CDColor *TexColors (int i = 0)
		{ return MineData ().texColors + (i & 0x3fff); }
	inline bool& UseTexColors (void)
		{ return m_bUseTexColors; }
	inline void SetTexColor (INT16 nBaseTex, CDColor *pc)	{
		if (UseTexColors () && (IsLight (nBaseTex) != -1))
			*TexColors (nBaseTex) = *pc;
		}
	inline CDColor *GetTexColor (INT16 nBaseTex, bool bIsTranspWall = false)	
		{ return UseTexColors () && (bIsTranspWall || (IsLight (nBaseTex) != -1)) ? TexColors (nBaseTex) : NULL; }
	CDColor *LightColor (int i = 0, int j = 0, bool bUseTexColors = true);
	inline CDColor *LightColors (int i = 0, int j = 0)
		{ return MineData ().lightColors [i] + j; }
	inline CDColor *CurrLightColor ()
		{ return LightColor (Current ()->segment, Current ()->side); }
		

	UINT8 *LoadDataResource (LPCTSTR pszRes, HGLOBAL& hGlobal, UINT32& nResSize);
	INT16 LoadDefaultLightAndColor (void);
	BOOL HasCustomLightMap (void);
	BOOL HasCustomLightColors (void);

	INT16 Load(const char *filename = NULL, bool bLoadFromHog = false);
	INT16 Save(const char *filename, bool bSaveToHog = false);
	int WriteColorMap (FILE *fColorMap);
	int ReadColorMap (FILE *fColorMap);
	void  Default();
	inline LPSTR LevelName (void)
		{ return current_level_name; }
	inline int LevelNameSize (void)
		{ return sizeof current_level_name; }
	inline bool	SplineActive (void)
		{ return m_bSplineActive; }
	inline void SetSplineActive (bool bSplineActive)
		{ m_bSplineActive = bSplineActive; }
	void  DeleteSegment(INT16 delete_segnum = -1);
	void  DeleteSegmentWalls (INT16 segnum);
	void	MakeObject (CDObject *obj, INT8 type, INT16 segnum);
	void	SetObjectData (INT8 type);
	bool	CopyObject (UINT8 new_type, INT16 segnum = -1);
	void  DeleteObject(INT16 objectNumber = -1);
	void  DeleteUnusedVertices();
	void  DeleteVertex(INT16 deleted_vertnum);

	void InitSegment (INT16 segNum);
	bool SplitSegment ();
	bool  AddSegment();
	bool  LinkSegments(INT16 segnum1,INT16 sidenum1, INT16 segnum2,INT16 sidenum2, FIX margin);
	void  LinkSides(INT16 segnum1,INT16 sidenum1,INT16 segnum2,INT16 sidenum2, tVertMatch match[4]);
	void	CalcSegCenter(vms_vector &pos,INT16 segnum);
	inline CDSegment *CurrSeg ()
		{ return Segments () + Current ()->segment; }
	inline CDWall *SideWall (int i = 0, int j = 0)
		{ int w = Segments (i)->sides [j].nWall; return (w < 0) ? NULL : Walls (w); }
	inline CDWall *CurrWall ()
		{ int w = CurrSide ()->nWall; return (w < 0) ? NULL : Walls (w); }
	inline CDSide *CurrSide ()
		{ return CurrSeg ()->sides + Current ()->side; }
	inline INT16 CurrVert ()
		{ return CurrSeg ()->verts [side_vert [Current ()->side][Current ()->point]]; }
	inline CDObject *CurrObj ()
		{ return Objects () + Current ()->object; }
	void Mark ();
	void MarkAll ();
	void UnmarkAll ();
	void MarkSegment (INT16 segnum);
	void UpdateMarkedCubes ();
	bool SideIsMarked (INT16 segnum, INT16 sidenum);
	bool SegmentIsMarked (INT16 segnum);

	bool IsPointOfSide (CDSegment *seg, int sidenum, int pointnum);
	bool IsLineOfSide (CDSegment *seg, int sidenum, int linenum);

	void JoinSegments(int automatic = 0);
	void JoinLines();
	void JoinPoints();
	void SplitSegments(int solidify = 0, int sidenum = -1);
	void SplitLines();
	void SplitPoints();

	void CalcOrthoVector (vms_vector &result,INT16 segnum,INT16 sidenum);
	void CalcCenter (vms_vector &center,INT16 segnum,INT16 sidenum);
	double CalcLength (vms_vector *center1, vms_vector *center2);

	int IsLight(int nBaseTex);
	int IsWall (INT16 segnum = -1, INT16 sidenum = -1);
	bool IsLava (int nBaseTex);
	bool IsBlastableLight (int nBaseTex);
	bool IsFlickeringLight (INT16 segnum, INT16 sidenum);
	bool CalcDeltaLights (double fLightScale, int force, int recursion_depth);
	void CalcDeltaLightData (double fLightScale = 1.0, int force = 1);
	int FindDeltaLight (INT16 segnum, INT16 sidenum, INT16 *pi = NULL);
	UINT8 LightWeight(INT16 nBaseTex);
	INT16 GetFlickeringLight(INT16 segnum = -1, INT16 sidenum = -1);
	INT16 AddFlickeringLight( INT16 segnum = -1, INT16 sidenum = -1, UINT32 mask = 0xAAAAAAAA, FIX time = 0x10000 / 4);
	bool DeleteFlickeringLight(INT16 segnum = -1, INT16 sidenum = -1);
	int IsExplodingLight(int nBaseTex);
	bool VisibleWall (UINT16 nWall);
	void SetCubeLight (double fLight, bool bAll = false, bool bDynCubeLights = false);
	void ScaleCornerLight (double fLight, bool bAll = false);
	void CalcAverageCornerLight (bool bAll = false);
	void AutoAdjustLight (double fBrightness, bool bAll = false, bool bCopyTexLights = false);
	void BlendColors (CDColor *psc, CDColor *pdc, double srcBr, double destBr);
	void Illuminate (INT16 source_segnum, INT16 source_sidenum, UINT32 brightness, 
						  double fLightScale, bool bAll = false, bool bCopyTexLights = false);
	bool CalcSideLights (int segnum, int sidenum, vms_vector& source_center, 
								vms_vector *source_corner, vms_vector& A, double *effect,
								double fLightScale, bool bIgnoreAngle);

	void FixChildren();
	void SetLinesToDraw ();

	INT16	MarkedSegmentCount (bool bCheck = false);
	bool	GotMarkedSegments (void)
		{ return MarkedSegmentCount (true) > 0; }
	bool CMine::GotMarkedSides ();

	inline void SetSelectMode (INT16 mode)
		{ m_selectMode = mode; }
	int ScrollSpeed (UINT16 texture,int *x,int *y);
	int AlignTextures (INT16 start_segment, INT16 start_side, INT16 only_child, BOOL bAlign1st, BOOL bAlign2nd, char bAlignedSides = 0);

	void EditGeoFwd();
	void EditGeoBack();
	void EditGeoUp();
	void EditGeoDown(); 
	void EditGeoLeft(); 
	void EditGeoRight(); 
	void EditGeoRotLeft(); 
	void EditGeoRotRight(); 
	void EditGeoGrow(); 
	void EditGeoShrink(); 
	void RotateSelection(double angle, bool perpendicular); 
	void SizeItem(INT32 inc); 
	void MovePoints(int pt0, int pt1); 
	void SizeLine(CDSegment *seg,int point0,int point1,INT32 inc); 
	void MoveOn(char axis,INT32 inc); 
	void SpinSelection(double angle); 
	void RotateVmsVector(vms_vector *vector,double angle,char axis); 
	void RotateVmsMatrix(vms_matrix *matrix,double angle,char axis); 
	void RotateVertex(vms_vector *vertex, vms_vector *orgin, vms_vector *normal, double angle); 
	void SetUV (INT16 segment, INT16 side, INT16 x, INT16 y, double angle);
	void LoadSideTextures (INT16 segNum, INT16 sideNum);

	CDWall *AddWall (INT16 segnum, INT16 sidenum, INT16 type, UINT16 flags, UINT8 keys, INT8 clipnum, INT16 tmapnum);
	CDWall *GetWall (INT16 segnum = -1, INT16 sidenum = -1);
	void DeleteWall (UINT16 wallnum = -1);
	CDWall *FindWall (INT16 segnum = -1, INT16 sidenum = -1);
	void DefineWall (INT16 segnum, INT16 sidenum, UINT16 wallnum,
						  UINT8 type, INT8 clipnum, INT16 tmapnum,
						  bool bRedefine);
	void SetWallTextures (UINT16 wallnum, INT16 tmapnum = 0);
	// trigger stuff
	void InitTrigger (CDTrigger *t, INT16 type, INT16 flags);
	CDTrigger *AddTrigger (UINT16 wallnum, INT16 type, BOOL bAutoAddWall = FALSE);
	void DeleteTrigger (INT16 trignum = -1);
	bool DeleteTriggerTarget (CDTrigger *trigger, INT16 segnum, INT16 sidenum, bool bAutoDeleteTrigger = true);
	void DeleteTriggerTargets (INT16 segnum, INT16 sidenum);
	int DeleteTargetFromTrigger (CDTrigger *trigger, INT16 linknum, bool bAutoDeleteTrigger = true);
	int DeleteTargetFromTrigger (INT16 trignum, INT16 linknum, bool bAutoDeleteTrigger = true);
	INT16 FindTriggerWall (INT16 *trignum, INT16 segnum = -1, INT16 sidenum = -1);
	INT16 FindTriggerWall (INT16 trignum);
	INT16 FindTriggerObject (INT16 *trignum);
	INT16 FindTriggerTarget (INT16 trignum, INT16 segnum, INT16 sidenum);
	CDObjTriggerList *AddObjTrigger (INT16 objnum, INT16 type);
	bool ObjTriggerIsInList (INT16 nTrigger);
	CDObjTriggerList *LinkObjTrigger (INT16 objnum, INT16 trignum);
	CDObjTriggerList *UnlinkObjTrigger (INT16 nTrigger);
	void DeleteObjTrigger (INT16 objnum);
	void DeleteObjTriggers (INT16 objnum);
	INT16 FindObjTriggerTarget (INT16 trignum, INT16 segnum, INT16 sidenum);

	void DrawObject (CWnd *pWnd, int type, int id);
	void ConvertWallNum (UINT16 wNumOld, UINT16 wNumNew);

	bool GetOppositeSide (INT16& opp_segnum, INT16& opp_sidenum, INT16 segnum = -1, INT16 sidenum = -1);
	bool GetOppositeWall (INT16 &opp_wallnum, INT16 segnum = -1, INT16 sidenum = -1);
	CDSide *OppSide ();
	bool SetTexture (INT16 segnum, INT16 sidenum, INT16 tmapnum, INT16 tmapnum2);
	void CopyOtherCube ();
	bool WallClipFromTexture (INT16 segnum, INT16 sidenum);
	void CheckForDoor (INT16 segnum, INT16 sidenum);
	void RenumberBotGens ();
	void RenumberEquipGens ();

	bool SetDefaultTexture (INT16 tmapnum = -1, INT16 walltype = -1);
	bool DefineSegment (INT16 segnum, UINT8 type, INT16 tmapnum, INT16 walltype = -1);
	void UndefineSegment (INT16 segnum);
	bool GetTriggerResources (UINT16& wallnum);
	bool AutoAddTrigger (INT16 wall_type, UINT16 wall_flags, UINT16 trigger_type);
	bool AddDoorTrigger (INT16 wall_type, UINT16 wall_flags, UINT16 trigger_type);
	bool AddOpenDoorTrigger(); 
	bool AddRobotMakerTrigger (); 
	bool AddShieldTrigger(); 
	bool AddEnergyTrigger(); 
	bool AddUnlockTrigger(); 
	bool AddExit (INT16 type); 
	bool AddNormalExit(); 
	bool AddSecretExit(); 
	bool AddDoor (UINT8 type, UINT8 flags, UINT8 keys, INT8 clipnum, INT16 tmapnum); 
	bool AddAutoDoor (INT8 clipnum = -1, INT16 tmapnum = -1); 
	bool AddPrisonDoor (); 
	bool AddGuideBotDoor(); 
	bool AddFuelCell (); 
	bool AddIllusionaryWall (); 
	bool AddForceField (); 
	bool AddFan ();
	bool AddWaterFall ();
	bool AddLavaFall(); 
	bool AddGrate(); 
	bool AddReactor (INT16 segnum = -1, bool bCreate = true, bool bSetDefTextures = true); 
	bool AddRobotMaker (INT16 segnum = -1, bool bCreate = true, bool bSetDefTextures = true); 
	bool AddEquipMaker (INT16 segnum = -1, bool bCreate = true, bool bSetDefTextures = true); 
	bool AddFuelCenter (INT16 segnum = -1, UINT8 nType = SEGMENT_IS_FUELCEN, bool bCreate = true, bool bSetDefTextures = true); 
	bool AddWaterOrLavaCube (INT16 segnum, bool bCreate, bool bSetDefTextures, UINT8 nType, INT16 nTexture);
	bool AddWaterCube (INT16 segnum = -1, bool bCreate = true, bool bSetDefTextures = true);
	bool AddLavaCube (INT16 segnum = -1, bool bCreate = true, bool bSetDefTextures = true);
	bool AddGoalCube (INT16 segnum, bool bCreate, bool bSetDefTextures, UINT8 nType, INT16 nTexture);
	bool AddTeamCube (INT16 segnum, bool bCreate, bool bSetDefTextures, UINT8 nType, INT16 nTexture);
	bool AddSpeedBoostCube (INT16 segnum, bool bCreate);
	bool AddSkyboxCube (INT16 segnum, bool bCreate);
	bool AddOutdoorCube (INT16 segnum, bool bCreate);
	bool AddBlockedCube (INT16 segnum, bool bCreate);
	bool AddNoDamageCube (INT16 segnum, bool bCreate);
	void AutoLinkExitToReactor ();
	int FuelCenterCount (void);
	inline int& RobotMakerCount (void) 
		{ return GameInfo ().botgen.count; }
	inline int& EquipMakerCount (void) 
		{ return GameInfo ().equipgen.count; }
	inline int& WallCount (void) 
		{ return GameInfo ().walls.count; }
	inline int& TriggerCount (void) 
		{ return GameInfo ().triggers.count; }
	inline int& ObjectCount (void) 
		{ return GameInfo ().objects.count; }

	inline CDSegment *OtherSeg (void)
		{ return Segments () + Other ()->segment; }
	inline CDSide *OtherSide (void)
		{ return OtherSeg ()->sides + Other ()->side; }
	inline void SetCurrent (INT16 nSegment = -1, INT16 nSide = -1, INT16 nLine = -1, INT16 nPoint = -1) {
		if (nSegment >= 0) Current ()->segment = nSegment;
		if (nSide >= 0) Current ()->side = nSide;
		if (nLine >= 0) Current ()->line = nLine;
		if (nPoint >= 0) Current ()->point = nPoint;
		}
	inline void GetCurrent (INT16 &nSegment, INT16& nSide) {
		if (nSegment < 0) nSegment = Current ()->segment;
		if (nSide < 0) nSide = Current ()->side;
		}

	void InitRobotData();
	int WriteHxmFile (FILE *fp);
	int ReadHxmFile (FILE *fp, long fSize);

	INT16 ReadSegmentInfo (FILE *file);
	void WriteSegmentInfo (FILE *file, INT16 /*segnum*/);
	void CutBlock ();
	void CopyBlock (char *pszBlkFile = NULL);
	void PasteBlock (); 
	int ReadBlock (char *name,int option); 
	void QuickPasteBlock  ();
	void DeleteBlock ();

	inline void wrap (INT16 *x, INT16 delta,INT16 min,INT16 max) {
		*x += delta;
		if (*x > max)
			*x = min;
		else if (*x < min)
			*x = max;
		}

	void TunnelGenerator ();
	void IncreaseSpline ();
	void DecreaseSpline ();
	void CalcSpline ();
	void UntwistSegment (INT16 segnum,INT16 sidenum);
	int MatchingSide (int j);
	INT16 LoadPalette (void);

	void SortObjects ();
	void RenumberObjTriggers (short i, short objnum);
	void QSortObjects (INT16 left, INT16 right);
	int QCmpObjects (CDObject *pi, CDObject *pm);
	bool IsCustomRobot (int i);
	BOOL HasCustomRobots();
	INT16 LoadMineSigAndType (FILE* fp);

private:
	int FindClip (CDWall *wall, INT16 tmapnum);
	INT16 CreateNewLevel ();
	void DefineVertices(INT16 new_verts[4]);
	void UnlinkChild(INT16 parent_segnum,INT16 sidenum);
	INT16 FixIndexValues();
	void ResetSide (INT16 segnum,INT16 sidenum);

	int ReadHamFile(char *fname = NULL, int type = NORMAL_HAM);
	void ReadPigTextureTable();
	void ReadRobotResource(int robot_number);
	void ReadColor (CDColor *pc, FILE *load_file);
	void SaveColor (CDColor *pc, FILE *save_file);
	void ReadTrigger (CDTrigger *t, FILE *fp, bool bObjTrigger);
	void WriteTrigger (CDTrigger *t, FILE *fp, bool bObjTrigger);
	void LoadColors (CDColor *pc, int nColors, int nFirstVersion, int nNewVersion, FILE *fp);
	void SaveColors (CDColor *pc, int nColors, FILE *fp);
	INT16 LoadMineDataCompiled (FILE *load_file, bool bNewMine);
	INT16 LoadMine (char *filename, bool bLoadFromHog, bool bNewMine);
	INT16 LoadGameData(FILE *loadfile, bool bNewMine);
	INT16 SaveMineDataCompiled(FILE *save_file);
	INT16 SaveGameData(FILE *savefile);
	void ReadObject(CDObject *obj,FILE *f,INT32 version);
	void WriteObject(CDObject *obj,FILE *f,INT32 version);
	int ReadWall (CDWall* wallP, FILE* fp, INT32 version);
	void ClearMineData();
	void UpdateDeltaLights ();
	double dround_off(double value, double round);
	void SetSegmentChildNum(CDSegment *pParent, INT16 segnum,INT16 recursion_level);
	void UnlinkSeg (CDSegment *pSegment, CDSegment *pRoot);
	void LinkSeg (CDSegment *pSegment, CDSegment *pRoot);
	void SortDLIndex (int left, int right);
	};

#endif //__mine_h