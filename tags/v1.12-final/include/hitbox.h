/* $Id: interp.h,v 1.6 2003/02/13 22:07:58 btb Exp $ */
/*
 *
 * took out functions declarations from include/3d.h
 * which are implemented in 3d/interp.c
 *
 */

#ifndef _HITBOX_H
#define _HITBOX_H

#include "fix.h"
//#include "vecmat.h" //the vector/matrix library
#include "gr.h"
#include "cfile.h"
#include "3d.h"

void ComputeHitbox (int nModel, int iSubObj);
void TransformHitboxes (tObject *objP, vmsVector *vPos, tBox *phb);

#endif //_HITBOX_H

