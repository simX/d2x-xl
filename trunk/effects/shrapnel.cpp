#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <string.h>	// for memset
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "inferno.h"
#include "error.h"
#include "timer.h"
#include "u_mem.h"
#include "interp.h"
#include "lightning.h"
#include "network.h"
#include "render.h"
#include "objeffects.h"
#include "objrender.h"
#include "objsmoke.h"
#include "automap.h"

CShrapnelManager shrapnelManager;

// -----------------------------------------------------------------------------

#define SHRAPNEL_MAX_PARTS			500
#define SHRAPNEL_PART_LIFE			-1750
#define SHRAPNEL_PART_SPEED		10

static float fShrapnelScale [5] = {0, 5.0f / 3.0f, 2.5f, 10.0f / 3.0f, 5};

void CShrapnel::Create (CObject* objP)
{
	static tRgbaColorf color = {1,1,1,0.5};

m_info.vDir = CFixVector::Random ();
m_info.vPos = objP->info.position.vPos + m_info.vDir * (objP->info.xSize / 4 + rand () % (objP->info.xSize / 2));
m_info.nTurn = 1;
m_info.xSpeed = 3 * (F1_0 / 20 + rand () % (F1_0 / 20)) / 4;
m_info.xLife =
m_info.xTTL = 3 * F1_0 / 2 + rand ();
m_info.tUpdate = gameStates.app.nSDLTicks;
m_info.nSmoke = 
	particleManager.Create (&m_info.vPos, NULL, NULL, objP->info.nSegment, 1, -SHRAPNEL_MAX_PARTS,
								   -PARTICLE_SIZE (1, 4), -1, 1, SHRAPNEL_PART_LIFE, SHRAPNEL_PART_SPEED, SMOKE_PARTICLES, 0x7fffffff, &color, 1, -1);
if (objP->info.xLifeLeft < m_info.xLife)
	objP->info.xLifeLeft = m_info.xLife;
}

// -----------------------------------------------------------------------------

void CShrapnel::Destroy (void)
{
if (m_info.nSmoke >= 0) {
	particleManager.SetLife (m_info.nSmoke, 0);
	m_info.nSmoke = 0;
	}
m_info.xTTL = -1;
}

// -----------------------------------------------------------------------------

void CShrapnel::Move (void)
{
	fix			xSpeed = FixDiv (m_info.xSpeed, 25 * F1_0 / 1000);
	CFixVector	vOffs;
	time_t		nTicks;

if ((nTicks = gameStates.app.nSDLTicks - m_info.tUpdate) < 25)
	return;
xSpeed = (fix) (xSpeed / gameStates.gameplay.slowmo [0].fSpeed);
for (; nTicks >= 25; nTicks -= 25) {
	if (--(m_info.nTurn))
		vOffs = m_info.vOffs;
	else {
		m_info.nTurn = ((m_info.xTTL > F1_0 / 2) ? 2 : 4) + d_rand () % 4;
		vOffs = m_info.vDir;
		vOffs [X] = FixMul (vOffs [X], 2 * d_rand ());
		vOffs [Y] = FixMul (vOffs [Y], 2 * d_rand ());
		vOffs [Z] = FixMul (vOffs [Z], 2 * d_rand ());
		CFixVector::Normalize (vOffs);
		m_info.vOffs = vOffs;
		}
	vOffs *= xSpeed;
	m_info.vPos += vOffs;
	}
particleManager.SetPos (m_info.nSmoke, &m_info.vPos, NULL, -1);
m_info.tUpdate = gameStates.app.nSDLTicks - nTicks;
}

// -----------------------------------------------------------------------------

void CShrapnel::Draw (void)
{
if ((m_info.xTTL > 0) && LoadExplBlast ()) {
	fix	xSize = F1_0 / 2 + d_rand () % (F1_0 / 4);
	G3DrawSprite (m_info.vPos, xSize, xSize, bmpExplBlast, NULL, X2F (m_info.xTTL) / X2F (m_info.xLife) / 2, 0, 0);
	}
}

// -----------------------------------------------------------------------------

int CShrapnel::Update (void)
{
if (m_info.xTTL <= 0)
	return -1;	//dead
Move ();
if (0 < (m_info.xTTL -= (fix) (SECS2X (gameStates.app.tick40fps.nTime) / gameStates.gameplay.slowmo [0].fSpeed)))
	return 1;	//alive
Destroy ();
return 0; //kill
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

uint CShrapnelCloud::Update (void)
{
for (uint i = 0; i < m_tos; ) {
	int j = m_data.buffer [i].Update ();
	if (j == 1)
		i++;
	else if (j == 0)
		Delete (i);
	}
if (m_tos)
	return m_tos;
Destroy ();
return 0;
}

// -----------------------------------------------------------------------------

void CShrapnelCloud::Draw (void)
{
for (uint i = 0; i < m_tos; i++)
	m_data.buffer [i].Draw ();
}

// -----------------------------------------------------------------------------

int CShrapnelCloud::Create (CObject* parentObjP)
{
	short		nObject = CreateFireball (0, parentObjP->info.nSegment, parentObjP->info.position.vPos, 1, RT_SHRAPNELS);

if (0 > nObject)
	return 0;

	int		i, h = (int) (X2F (parentObjP->info.xSize) * fShrapnelScale [gameOpts->render.effects.nShrapnels] + 0.5);
	CObject*	objP = OBJECTS + nObject;

objP->info.xLifeLeft = 0;
objP->cType.explInfo.nSpawnTime = -1;
objP->cType.explInfo.nDeleteObj = -1;
objP->cType.explInfo.nDeleteTime = -1;
h += d_rand () % h;
if (!CStack<CShrapnel>::Create (h))
	return 0;
srand (gameStates.app.nSDLTicks);
for (i = 0; i < h; i++) {
	m_data.buffer [i].Create (parentObjP);
	}
objP->info.xLifeLeft *= 2;
objP->cType.explInfo.nSpawnTime = -1;
objP->cType.explInfo.nDeleteObj = -1;
objP->cType.explInfo.nDeleteTime = -1;
return 1;
}

// -----------------------------------------------------------------------------

void CShrapnelCloud::Destroy (void)
{
for (uint i = 0; i < m_tos; i++)
	m_data.buffer [i].Destroy ();
CStack::Destroy ();
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CShrapnelManager::Init (void)
{
CArray<CShrapnelCloud>::Create (MAX_OBJECTS);
}

// -----------------------------------------------------------------------------

int CShrapnelManager::Create (CObject *objP)
{
if (!SHOW_SMOKE)
	return 0;
if (!gameOpts->render.effects.nShrapnels)
	return 0;
if (objP->info.nFlags & OF_ARMAGEDDON)
	return 0;
if ((objP->info.nType != OBJ_PLAYER) && (objP->info.nType != OBJ_ROBOT))
	return 0;
return Create (objP);
}

// -----------------------------------------------------------------------------

void CShrapnelManager::Draw (CObject *objP)
{
m_data.buffer [OBJ_IDX (objP)].Draw ();
}

// -----------------------------------------------------------------------------

int CShrapnelManager::Update (CObject *objP)
{
if ((objP->LifeLeft () <= 0) || !m_data.buffer [OBJ_IDX (objP)].Update ())
	Destroy (objP);
return 0;
}

// -----------------------------------------------------------------------------

void CShrapnelManager::Destroy (CObject *objP)
{
if ((objP->info.nType != OBJ_FIREBALL) || (objP->info.renderType != RT_SHRAPNELS))
	return;
m_data.buffer [OBJ_IDX (objP)].Destroy ();
objP->SetLifeLeft (-1);
}

//------------------------------------------------------------------------------

void CShrapnelManager::DoFrame (void)
{
	CObject	*objP;
	int		i;

if (!SHOW_SMOKE)
	return;
FORALL_STATIC_OBJS (objP, i)
	if (objP->info.renderType == RT_SHRAPNELS)
		Update (objP);
FORALL_ACTOR_OBJS (objP, i) {
	i = OBJ_IDX (objP);
	if (gameData.objs.bWantEffect [i] & SHRAPNEL_SMOKE) {
		gameData.objs.bWantEffect [i] &= ~SHRAPNEL_SMOKE;
		Create (objP);
		}
	}
}

//------------------------------------------------------------------------------
