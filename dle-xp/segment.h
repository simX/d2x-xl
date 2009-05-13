// Segment.h

#ifndef __segment_h
#define __segment_h

typedef struct uvl {
	INT16 u,v,l; /* was FIX */
} uvl;

class CDSide {
public:
	UINT16 nWall;  /* (was INT16) Index into Walls array, which wall (probably door) is on this side */
	INT16  nBaseTex;  /* Index into array of textures specified in bitmaps.bin */
	INT16  nOvlTex; /* Index, as above, texture which gets overlaid on nBaseTex */
	uvl    uvls[4];   /* uvl coordinates at each point */
};

typedef struct rgbColor {
	double	r, g, b;
} rgbColor;

class CDColor {
	public:
		UINT8		index;
		rgbColor	color;
};

class CDSegment {
public:

	CDSide sides [MAX_SIDES_PER_SEGMENT];    /* 6 sides */
	INT16  children [MAX_SIDES_PER_SEGMENT];  /* indices of 6 children segments, front, left, top, right, bottom, back */
	INT16  verts [MAX_VERTICES_PER_SEGMENT];  /* vertex ids of 4 front and 4 back vertices */
	UINT8  special;        /* special property of a segment (such as damaging, trigger, etc.) */
	INT8   matcen_num;     /* which center segment is associated with, high bit set */
	INT8   value;          /* matcens: bitmask of producable robots, fuelcenters: energy given? --MK, 3/15/95 */
	UINT8  s2_flags;	 // New for Descent 2
	FIX    static_light;   /* average static light in segment */
	UINT8  child_bitmask;  /* bit0 to 5: children, bit6: unused, bit7: special */
	UINT8  wall_bitmask;   /* bit0 to 5: door/walls, bit6: deleted, bit7: marked segment */
	INT16  seg_number;     /* used for cut & paste to help link children */
	INT16  map_bitmask;    /* which lines are drawn when displaying wireframe */
	INT8   owner;
	INT8   group;
//	INT16	 prevSeg;
//	INT16	 nextSeg;
//	INT16	 rootSeg;
	};

#endif //__segment_h