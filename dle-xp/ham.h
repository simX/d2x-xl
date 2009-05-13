// Copyright (C) 1997 Bryan Aamot
typedef struct {
  UINT16 index;
} BITMAP_INDEX;

typedef struct {
  UINT8	 flags;		    //values defined above
  UINT8	 pad[3];	    //keep alignment
  FIX	 lighting;	    //how much light this casts
  FIX	 damage;	    //how much damage being against this does (for lava)
  INT16	 eclip_num;	    //the eclip that changes this, or -1
  INT16	 destroyed;	    //bitmap to show when destroyed, or -1
  INT16	 slide_u,slide_v;   //slide rates of texture, stored in 8:8 FIX
} TMAP_INFO;

typedef struct {
  FIX		play_time;  //total time (in seconds) of clip
  INT32		num_frames;
  FIX		frame_time; //time (in seconds) of each frame
  INT32		flags;
  INT16		sound_num;
  BITMAP_INDEX	frames[VCLIP_MAX_FRAMES];
  FIX		light_value;
} VCLIP;

typedef struct {
  VCLIP  vc;			   //imbedded vclip
  FIX	 time_left;		   //for sequencing
  INT32	 frame_count;		   //for sequencing
  INT16	 changing_wall_texture;	   //Which element of Textures array to replace.
  INT16	 changing_object_texture;  //Which element of ObjBitmapPtrs array to replace.
  INT32	 flags;			   //see above
  INT32	 crit_clip;		   //use this clip instead of above one when mine critical
  INT32	 dest_bm_num;		//use this bitmap when monitor destroyed
  INT32	 dest_vclip;		//what vclip to play when exploding
  INT32	 dest_eclip;		//what eclip to play when exploding
  FIX	 dest_size;		//3d size of explosion
  INT32	 sound_num;		//what sound this makes
  INT32	 segnum,sidenum;	//what seg & side, for one-shot clips
} ECLIP;

typedef struct {
  FIX	 play_time;
  INT16	 num_frames;
  INT16	 frames[MAX_CLIP_FRAMES2];
  INT16	 open_sound;
  INT16	 close_sound;
  INT16	 flags;
  char	 filename[13];
  char	 pad;
} WCLIP;

//describes a list of joint positions
typedef struct {
  INT16  n_joints;
  INT16  offset;
} JOINTLIST;

typedef struct {
  INT32	      	model_num;		  // which polygon model?
  VMS_VECTOR	gun_points[MAX_GUNS];	  // where each gun model is
  UINT8		gun_submodels[MAX_GUNS];  // which submodel is each gun in?

  INT16 	exp1_vclip_num;
  INT16		exp1_sound_num;

  INT16 	exp2_vclip_num;
  INT16		exp2_sound_num;

  INT8		weapon_type;
  INT8		weapon_type2;		  // Secondary weapon number, -1 means none, otherwise gun #0 fires this weapon.
  INT8		n_guns;			  // how many different gun positions
  INT8		contains_id;		  // ID of powerup this robot can contain.

  INT8		contains_count;		  // Max number of things this instance can contain.
  INT8		contains_prob;		  // Probability that this instance will contain something in N/16
  INT8		contains_type;		  // Type of thing contained, robot or powerup, in bitmaps.tbl, !0=robot, 0=powerup
  INT8		kamikaze;		  // !0 means commits suicide when hits you, strength thereof. 0 means no.

  INT16		score_value;		  // Score from this robot.
  INT8		badass;			  // Dies with badass explosion, and strength thereof, 0 means NO.
  INT8		energy_drain;		  // Points of energy drained at each collision.

  FIX		lighting;		  // should this be here or with polygon model?
  FIX		strength;		  // Initial shields of robot

  FIX		mass;			  // how heavy is this thing?
  FIX		drag;			  // how much drag does it have?

  FIX		field_of_view[NDL];	  // compare this value with forward_vector.dot.vector_to_player,
					  // ..if field_of_view <, then robot can see player
  FIX		firing_wait[NDL];	  // time in seconds between shots
  FIX		firing_wait2[NDL];	  // time in seconds between shots
  FIX		turn_time[NDL];		  // time in seconds to rotate 360 degrees in a dimension
// -- unused, mk, 05/25/95	FIX		fire_power[NDL];   // damage done by a hit from this robot
// -- unused, mk, 05/25/95	FIX		shield[NDL];	   // shield strength of this robot
  FIX		max_speed[NDL];		  // maximum speed attainable by this robot
  FIX		circle_distance[NDL];	  // distance at which robot circles player

  INT8		rapidfire_count[NDL];	  // number of shots fired rapidly
  INT8		evade_speed[NDL];	  // rate at which robot can evade shots, 0=none, 4=very fast
  INT8		cloak_type;		  // 0=never, 1=always, 2=except-when-firing
  INT8		attack_type;		  // 0=firing, 1=charge (like green guy)

  UINT8		see_sound;		  // sound robot makes when it first sees the player
  UINT8		attack_sound;		  // sound robot makes when it attacks the player
  UINT8		claw_sound;		  // sound robot makes as it claws you (attack_type should be 1)
  UINT8		taunt_sound;		  // sound robot makes after you die

  INT8		boss_flag;		  // 0 = not boss, 1/2 = D1 boss, 21 + id = D2/Vertigo boss
  INT8		companion;		  // Companion robot, leads you to things.
  INT8		smart_blobs;		  // how many smart blobs are emitted when this guy dies!
  INT8		energy_blobs;		  // how many smart blobs are emitted when this guy gets hit by energy weapon!

  INT8		thief;			  // !0 means this guy can steal when he collides with you!
  INT8		pursuit;		  // !0 means pursues player after he goes around a corner.
					  // ..4 = 4/2 pursue up to 4/2 seconds after becoming invisible if up to 4
					  // ..segments away
  INT8		lightcast;		  // Amount of light cast. 1 is default.  10 is very large.
  INT8		death_roll;		  // 0 = dies without death roll. !0 means does death roll, larger = faster
					  // ..and louder

  //boss_flag, companion, thief, & pursuit probably should also be bits in the flags byte.
  UINT8		flags;			  // misc properties
  UINT8		pad[3];			  // alignment

  UINT8		deathroll_sound;	  // if has deathroll, what sound?
  UINT8		glow;			  // apply this light to robot itself. stored as 4:4 FIXed-point
  UINT8		behavior;		  // Default behavior.
  UINT8		aim;			  // 255 = perfect, less = more likely to miss.  0 != random, would look stupid.
					  // ..0=45 degree spread.  Specify in bitmaps.tbl in range 0.0..1.0

  //animation info
  JOINTLIST anim_states[MAX_GUNS+1][N_ANIM_STATES];

  INT32		always_0xabcd;		  // debugging

  } ROBOT_INFO;

typedef struct {
  INT16 jointnum;
  VMS_ANGVEC angles;
} JOINTPOS;

typedef struct {
  INT8	render_type;		// How to draw 0=laser, 1=blob, 2=object
  INT8	persistent;		// 0 = dies when it hits something, 1 = continues (eg, fusion cannon)
  INT16	model_num;		// Model num if rendertype==2.
  INT16	model_num_inner;	// Model num of inner part if rendertype==2.

  INT8	flash_vclip;		// What vclip to use for muzzle flash
  INT8	robot_hit_vclip;	// What vclip for impact with robot
  INT16	flash_sound;		// What sound to play when fired

  INT8	wall_hit_vclip;		// What vclip for impact with wall
  INT8	fire_count;		// Number of bursts fired from EACH GUN per firing.
				// ..For weapons which fire from both sides, 3*fire_count shots will be fired.
  INT16	robot_hit_sound;	// What sound for impact with robot

  INT8	ammo_usage;		// How many units of ammunition it uses.
  INT8	weapon_vclip;		// Vclip to render for the weapon, itself.
  INT16	wall_hit_sound;		// What sound for impact with wall

  INT8	destroyable;		// If !0, this weapon can be destroyed by another weapon.
  INT8	matter;			// Flag: set if this object is matter (as opposed to energy)
  INT8	bounce;			// 1==always bounces, 2=bounces twice
  INT8	homing_flag;		// Set if this weapon can home in on a target.

  UINT8	speedvar;		// allowed variance in speed below average, /128: 64 = 50% meaning if speed = 100,
				// ..can be 50..100

  UINT8	flags;			// see values above

  INT8	flash;			// Flash effect
  INT8	afterburner_size;	// Size of blobs in F1_0/16 units, specify in bitmaps.tbl as floating point.
				// ..Player afterburner size = 2.5.

  INT8	children;		// ID of weapon to drop if this contains children.  -1 means no children.

  FIX	energy_usage;		// How much fuel is consumed to fire this weapon.
  FIX	fire_wait;		// Time until this weapon can be fired again.

  FIX	multi_damage_scale;	// Scale damage by this amount when applying to player in multiplayer.
				// ..F1_0 means no change.

  BITMAP_INDEX bitmap;		// Pointer to bitmap if rendertype==0 or 1.

  FIX	blob_size;		// Size of blob if blob type
  FIX	flash_size;		// How big to draw the flash
  FIX	impact_size;		// How big of an impact
  FIX	strength[NDL];		// How much damage it can inflict
  FIX	speed[NDL];		// How fast it can move, difficulty level based.
  FIX	mass;			// How much mass it has
  FIX	drag;			// How much drag it has
  FIX	thrust;			// How much thrust it has
  FIX	po_len_to_width_ratio;	// For polyobjects, the ratio of len/width. (10 maybe?)
  FIX	light;			// Amount of light this weapon casts.
  FIX	lifetime;		// Lifetime in seconds of this weapon.
  FIX	damage_radius;		// Radius of damage caused by weapon, used for missiles (not lasers) to apply
				// ..to damage to things it did not hit
//-- unused--	FIX	damage_force;	 // Force of damage caused by weapon, used for missiles (not lasers) to
// ..apply to damage to things it did not hit.
// damage_force was a real mess.  Wasn't Difficulty_level based, and was being applied instead of weapon's
// ..actual strength.  Now use 2*strength instead. --MK, 01/19/95
  BITMAP_INDEX	picture;	// a picture of the weapon for the cockpit
  BITMAP_INDEX	hires_picture;	// a hires picture of the above
} WEAPON_INFO;

typedef struct {
  INT32	vclip_num;
  INT32	hit_sound;
  FIX	size;			// 3d size of longest dimension
  FIX	light;			// amount of light cast by this powerup, set in bitmaps.tbl
} POWERUP_TYPE_INFO;

//used to describe a polygon model
typedef struct {
  INT32		n_models;
  INT32 	model_data_size;
  UINT8 	*model_data;
  INT32 	submodel_ptrs[MAX_SUBMODELS];
  VMS_VECTOR 	submodel_offsets[MAX_SUBMODELS];
  VMS_VECTOR 	submodel_norms[MAX_SUBMODELS];	  // norm for sep plane
  VMS_VECTOR 	submodel_pnts[MAX_SUBMODELS];	  // point on sep plane
  FIX 		submodel_rads[MAX_SUBMODELS];	  // radius for each submodel
  UINT8 	submodel_parents[MAX_SUBMODELS];  // what is parent for each submodel
  VMS_VECTOR 	submodel_mins[MAX_SUBMODELS];
  VMS_VECTOR    submodel_maxs[MAX_SUBMODELS];
  VMS_VECTOR 	mins,maxs;			  // min,max for whole model
  FIX rad;
  UINT8		n_textures;
  UINT16	first_texture;
  UINT8		simpler_model;			  // alternate model with less detail (0 if none, model_num+1 else)
//  VMS_VECTOR min,max;
} POLYMODEL;

