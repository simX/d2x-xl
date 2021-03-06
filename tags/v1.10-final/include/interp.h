/* $Id: interp.h,v 1.6 2003/02/13 22:07:58 btb Exp $ */
/*
 *
 * took out functions declarations from include/3d.h
 * which are implemented in 3d/interp.c
 *
 */

#ifndef _INTERP_H
#define _INTERP_H

#include "fix.h"
//#include "vecmat.h" //the vector/matrix library
#include "gr.h"
#include "cfile.h"
#include "3d.h"

//Object functions:

fix G3PolyModelSize (tPolyModel *pm, int nModel);

//gives the interpreter an array of points to use
void G3SetModelPoints(g3sPoint *pointlist);

//calls the tObject interpreter to render an tObject.  The tObject renderer
//is really a seperate pipeline. returns true if drew
bool G3DrawPolyModel (tObject *objP, void *modelP, grsBitmap **modelBitmaps, vmsAngVec *animAngles, vmsVector *vOffset,
							 fix light, fix *glowValues, tRgbaColorf *obj_colors, tPOFObject *po, int nModel);

int G3DrawPolyModelShadow (tObject *objP, void *modelP, vmsAngVec *pAnimAngles, int nModel);

int G3FreePolyModelItems (tPOFObject *po);

//init code for bitmap models
void G3InitPolyModel(tPolyModel *pm, int nModel);

//un-initialize, i.e., convert color entries back to RGB15
void g3_uninit_polygon_model(void *model_ptr);

//alternate interpreter for morphing tObject
bool G3DrawMorphingModel(void *model_ptr,grsBitmap **model_bitmaps,vmsAngVec *animAngles, vmsVector *vOffset, 
								 fix light, vmsVector *new_points, int nModel);

//this remaps the 15bpp colors for the models into a new palette.  It should
//be called whenever the palette changes
void g3_remap_interp_colors(void);

int G3CheckAndSwap (void *modelP);

void G3FreeAllPolyModelItems (void);

#if defined(WORDS_BIGENDIAN) || defined(__BIG_ENDIAN__)
// routine to convert little to big endian in polygon model data
void swap_polygon_model_data(ubyte *data);
//routines to convert little to big endian in vectors
void VmsVectorSwap(vmsVector *v);
void VmsAngVecSwap(vmsAngVec *v);
#endif

#ifdef WORDS_NEED_ALIGNMENT
/*
 * A chunk struct (as used for alignment) contains all relevant data
 * concerning a piece of data that may need to be aligned.
 * To align it, we need to copy it to an aligned position,
 * and update all pointers  to it.
 * (Those pointers are actually offsets
 * relative to start of modelData) to it.
 */
typedef struct chunk {
	ubyte *old_base; // where the offset sets off from (relative to beginning of modelData)
	ubyte *new_base; // where the base is in the aligned structure
	short offset; // how much to add to base to get the address of the offset
	short correction; // how much the value of the offset must be shifted for alignment
} chunk;
#define MAX_CHUNKS 100 // increase if insufficent
/*
 * finds what chunks the data points to, adds them to the chunk_list, 
 * and returns the length of the current chunk
 */
int get_chunks(ubyte *data, ubyte *new_data, chunk *list, int *no);
#endif //def WORDS_NEED_ALIGNMENT

void G3SwapPolyModelData (ubyte *data);

void G3SetCullAndStencil (int bCullFront, int bZPass);
void G3RenderShadowVolumeFace (tOOF_vector *pv);
void G3RenderFarShadowCapFace (tOOF_vector *pv, int nv);

void ComputeHitbox (int nModel, int iSubObj);
void TransformHitboxes (tObject *objP, vmsVector *vPos, tBox *phb);

extern int hitboxFaceVerts [6][4];
extern vmsVector hitBoxOffsets [8];
extern vmsAngVec avZero;
extern vmsVector vZero;
extern vmsMatrix mIdentity;

#endif //_INTERP_H

