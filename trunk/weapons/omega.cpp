#ifdef HAVE_CONFIG_H
#	include <conf.h>
#endif

#include "descent.h"
#include "error.h"
#include "gameseg.h"
#include "network.h"
#include "lightning.h"
#include "omega.h"
#include "slowmotion.h"

//	-------------------------------------------------------------------------------------------------------------------------------
//	***** HEY ARTISTS!!*****
//	Here are the constants you're looking for!--MK

//	Change the following constants to affect the look of the omega cannon.
//	Changing these constants will not affect the damage done.
//	WARNING: If you change DESIRED_OMEGA_DIST and MAX_OMEGA_BLOBS, you don't merely change the look of the cannon,
//	you change its range.  If you decrease DESIRED_OMEGA_DIST, you decrease how far the gun can fire.

#define OMEGA_ENERGY_RATE		(I2X (190) / 17)
//	Note, you don't need to change these constants.  You can control damage and energy consumption by changing the
//	usual bitmaps.tbl parameters.
#define	OMEGA_DAMAGE_SCALE			32				//	Controls how much damage is done.  This gets multiplied by gameData.time.xFrame and then again by the damage specified in bitmaps.tbl in the $WEAPON line.
#define	OMEGA_ENERGY_CONSUMPTION	16				//	Controls how much energy is consumed.  This gets multiplied by gameData.time.xFrame and then again by the energy parameter from bitmaps.tbl.

#define	MIN_OMEGA_CHARGE	 (DEFAULT_MAX_OMEGA_CHARGE/8)
#define	OMEGA_CHARGE_SCALE	4			//	gameData.time.xFrame / OMEGA_CHARGE_SCALE added to gameData.omega.xCharge [IsMultiGame] every frame.

// ---------------------------------------------------------------------------------

fix OmegaEnergy (fix xDeltaCharge)
{
	fix xEnergyUsed;

if (xDeltaCharge < 0)
	return -1;
xEnergyUsed = FixMul (OMEGA_ENERGY_RATE, xDeltaCharge);
if (gameStates.app.nDifficultyLevel < 2)
	xEnergyUsed = FixMul (xEnergyUsed, I2X (gameStates.app.nDifficultyLevel + 2) / 4);
return xEnergyUsed;
}

// ---------------------------------------------------------------------------------
//	*objP is the CObject firing the omega cannon
//	*pos is the location from which the omega bolt starts

int nOmegaDuration [7] = {1, 2, 3, 5, 7, 10, 15};

void SetMaxOmegaCharge (void)
{
gameData.omega.xMaxCharge = DEFAULT_MAX_OMEGA_CHARGE * nOmegaDuration [int (extraGameInfo [0].nOmegaRamp)];
if (gameData.omega.xCharge [IsMultiGame] > gameData.omega.xMaxCharge) {
	LOCALPLAYER.energy += OmegaEnergy (gameData.omega.xCharge [IsMultiGame] - gameData.omega.xMaxCharge);
	gameData.omega.xCharge [IsMultiGame] = gameData.omega.xMaxCharge;
	}
}

//	-------------------------------------------------------------------------------------------------------------------------------

void DeleteOldOmegaBlobs (CObject *parentObjP)
{
	int		nParentObj = parentObjP->cType.laserInfo.parent.nObject;
	CObject	*objP;

FORALL_WEAPON_OBJS (objP, i)
	if ((objP->info.nId == OMEGA_ID) && (objP->cType.laserInfo.parent.nObject == nParentObj))
		ReleaseObject (objP->Index ());
}

// ---------------------------------------------------------------------------------

void CreateOmegaBlobs (short nFiringSeg, CFixVector *vMuzzle, CFixVector *vTargetPos, CObject *parentObjP, CObject *targetObjP)
{
	short			nLastSeg, nLastCreatedObj = -1;
	CFixVector	vGoal;
	fix			xGoalDist;
	int			nOmegaBlobs;
	fix			xOmegaBlobDist;
	CFixVector	vOmegaDelta;
	CFixVector	vBlobPos, vPerturb;
	fix			xPerturbArray [MAX_OMEGA_BLOBS];
	int			i;

if (IsMultiGame)
	DeleteOldOmegaBlobs (parentObjP);
omegaLightnings.Create (vTargetPos, parentObjP, targetObjP);
vGoal = *vTargetPos - *vMuzzle;
xGoalDist = CFixVector::Normalize (vGoal);
if (xGoalDist < MIN_OMEGA_BLOBS * MIN_OMEGA_DIST) {
	xOmegaBlobDist = MIN_OMEGA_DIST;
	nOmegaBlobs = xGoalDist / xOmegaBlobDist;
	if (nOmegaBlobs == 0)
		nOmegaBlobs = 1;
	}
else {
	xOmegaBlobDist = DESIRED_OMEGA_DIST;
	nOmegaBlobs = xGoalDist / xOmegaBlobDist;
	if (nOmegaBlobs > MAX_OMEGA_BLOBS) {
		nOmegaBlobs = MAX_OMEGA_BLOBS;
		xOmegaBlobDist = xGoalDist / nOmegaBlobs;
		}
	else if (nOmegaBlobs < MIN_OMEGA_BLOBS) {
		nOmegaBlobs = MIN_OMEGA_BLOBS;
		xOmegaBlobDist = xGoalDist / nOmegaBlobs;
		}
	}
vOmegaDelta = vGoal;
vOmegaDelta *= xOmegaBlobDist;
//	Now, create all the blobs
vBlobPos = *vMuzzle;
nLastSeg = nFiringSeg;

//	If nearby, don't perturb vector.  If not nearby, start halfway out.
if (xGoalDist < MIN_OMEGA_DIST * 4) {
	for (i = 0; i < nOmegaBlobs; i++)
		xPerturbArray [i] = 0;
	}
else {
	vBlobPos += vOmegaDelta * (I2X (1) / 2);	//	Put first blob half way out.
	for (i = 0; i < nOmegaBlobs / 2; i++) {
		xPerturbArray [i] = I2X (i) + I2X (1) / 4;
		xPerturbArray [nOmegaBlobs - 1 - i] = I2X (i);
		}
	}

//	Create Random perturbation vector, but favor _not_ going up in CPlayerData's reference.
vPerturb = CFixVector::Random ();
vPerturb += parentObjP->info.position.mOrient.UVec () * (-I2X (1) / 2);
for (i = 0; i < nOmegaBlobs; i++) {
	CFixVector	vTempPos;
	short			nBlobObj, nSegment;

	//	This will put the last blob right at the destination CObject, causing damage.
	if (i == nOmegaBlobs - 1)
		vBlobPos += vOmegaDelta * (I2X (15) / 32);	//	Move last blob another (almost) half section
	//	Every so often, re-perturb blobs
	if (i % 4 == 3) {
		CFixVector	vTemp;

		vTemp = CFixVector::Random ();
		vPerturb += vTemp * (I2X (1) / 4);
		}
	vTempPos = vBlobPos + vPerturb * xPerturbArray[i];
	nSegment = FindSegByPos (vTempPos, nLastSeg, 1, 0);
	if (nSegment != -1) {
		CObject		*objP;

		nLastSeg = nSegment;
		nBlobObj = CreateWeapon (OMEGA_ID, -1, nSegment, vTempPos, 0, RT_WEAPON_VCLIP);
		if (nBlobObj == -1)
			break;
		nLastCreatedObj = nBlobObj;
		objP = OBJECTS + nBlobObj;
		objP->info.xLifeLeft = ONE_FRAME_TIME;
		objP->mType.physInfo.velocity = vGoal;
		//	Only make the last one move fast, else multiple blobs might collide with target.
		objP->mType.physInfo.velocity *= (I2X (4));
		objP->info.xSize = gameData.weapons.info [objP->info.nId].blob_size;
		objP->info.xShields = FixMul (OMEGA_DAMAGE_SCALE*gameData.time.xFrame, WI_strength (objP->info.nId,gameStates.app.nDifficultyLevel));
		objP->cType.laserInfo.parent.nType = parentObjP->info.nType;
		objP->cType.laserInfo.parent.nSignature = parentObjP->info.nSignature;
		objP->cType.laserInfo.parent.nObject = OBJ_IDX (parentObjP);
		objP->info.movementType = MT_NONE;	//	Only last one moves, that will get bashed below.
		}
	vBlobPos += vOmegaDelta;
	}

	//	Make last one move faster, but it's already moving at speed = I2X (4).
if (nLastCreatedObj != -1) {
	OBJECTS [nLastCreatedObj].mType.physInfo.velocity *=
		gameData.weapons.info [OMEGA_ID].speed [gameStates.app.nDifficultyLevel]/4;
	OBJECTS [nLastCreatedObj].info.movementType = MT_PHYSICS;
	}
}

// ---------------------------------------------------------------------------------
//	Call this every frame to recharge the Omega Cannon.
void OmegaChargeFrame (void)
{
	fix	xOldOmegaCharge;

if (gameData.omega.xCharge [IsMultiGame] == MAX_OMEGA_CHARGE) {
	omegaLightnings.Destroy (LOCALPLAYER.nObject);
	return;
	}
if (!(PlayerHasWeapon (OMEGA_INDEX, 0, -1, 0) & HAS_WEAPON_FLAG)) {
	omegaLightnings.Destroy (LOCALPLAYER.nObject);
	return;
	}
if (gameStates.app.bPlayerIsDead) {
	omegaLightnings.Destroy (LOCALPLAYER.nObject);
	return;
	}
if ((gameData.weapons.nPrimary == OMEGA_INDEX) && !gameData.omega.xCharge [IsMultiGame] && !LOCALPLAYER.energy) {
	omegaLightnings.Destroy (LOCALPLAYER.nObject);
	gameData.weapons.nPrimary--;
	AutoSelectWeapon (0, 1);
	}
//	Don't charge while firing.
if ((gameData.omega.nLastFireFrame == gameData.app.nFrameCount) ||
	 (gameData.omega.nLastFireFrame == gameData.app.nFrameCount - 1))
	return;

omegaLightnings.Destroy (LOCALPLAYER.nObject);
if (LOCALPLAYER.energy) {
	xOldOmegaCharge = gameData.omega.xCharge [IsMultiGame];
	gameData.omega.xCharge [IsMultiGame] += (fix) (gameData.time.xFrame / OMEGA_CHARGE_SCALE / gameStates.gameplay.slowmo [0].fSpeed);
	if (gameData.omega.xCharge [IsMultiGame] > MAX_OMEGA_CHARGE)
		gameData.omega.xCharge [IsMultiGame] = MAX_OMEGA_CHARGE;
	LOCALPLAYER.energy -= OmegaEnergy (gameData.omega.xCharge [IsMultiGame] - xOldOmegaCharge);
	if (LOCALPLAYER.energy < 0)
		LOCALPLAYER.energy = 0;
	}
}

// -- fix	Last_omega_muzzle_flashTime;

// ---------------------------------------------------------------------------------

void DoOmegaStuff (CObject *parentObjP, CFixVector *vMuzzle, CObject *weaponObjP)
{
	short			nTargetObj, nFiringSeg, nParentSeg;
	CFixVector	vTargetPos;
	int			nPlayer = (parentObjP->info.nType == OBJ_PLAYER) ? parentObjP->info.nId : -1;
	int			bSpectate = SPECTATOR (parentObjP);
	static		int nDelay = 0;

#if 1
if (gameStates.gameplay.bMineMineCheat && (gameData.omega.xCharge [IsMultiGame] < MAX_OMEGA_CHARGE))
	gameData.omega.xCharge [IsMultiGame] = MAX_OMEGA_CHARGE - 1;
#endif
if (nPlayer == gameData.multiplayer.nLocalPlayer) {
	//	If charge >= min, or (some charge and zero energy), allow to fire.
	if ((gameData.omega.xCharge [IsMultiGame] < MIN_OMEGA_CHARGE) &&
		 (!gameData.omega.xCharge [IsMultiGame] || gameData.multiplayer.players [nPlayer].energy)) {
		ReleaseObject (OBJ_IDX (weaponObjP));
		omegaLightnings.Destroy (LOCALPLAYER.nObject);
		return;
		}
	gameData.omega.xCharge [IsMultiGame] -= gameData.time.xFrame;
	if (gameData.omega.xCharge [IsMultiGame] < 0)
		gameData.omega.xCharge [IsMultiGame] = 0;
	//	Ensure that the lightning cannon can be fired next frame.
	gameData.laser.xNextFireTime = gameData.time.xGame + 1;
	gameData.omega.nLastFireFrame = gameData.app.nFrameCount;
	}

weaponObjP->cType.laserInfo.parent.nType = parentObjP->info.nType;
weaponObjP->cType.laserInfo.parent.nObject = OBJ_IDX (parentObjP);
weaponObjP->cType.laserInfo.parent.nSignature = parentObjP->info.nSignature;

if (gameStates.limitFPS.bOmega && !gameStates.app.tick40fps.bTick)
#if 1
	return;
if (SlowMotionActive ()) {
	if (nDelay > 0) {
		nDelay -= 2;
		if	(nDelay > 0)
			return;
		}
	nDelay += gameOpts->gameplay.nSlowMotionSpeedup;
	}
#else
	nTargetObj = -1;
else
#endif
	nTargetObj = FindHomingObject (vMuzzle, weaponObjP);
nParentSeg = bSpectate ? gameStates.app.nPlayerSegment : parentObjP->info.nSegment;

if (0 > (nFiringSeg = FindSegByPos (*vMuzzle, nParentSeg, 1, 0))) {
	omegaLightnings.Destroy (OBJ_IDX (parentObjP));
	return;
	}
//	Play sound.
if (parentObjP == gameData.objs.viewerP)
	audio.PlaySound (gameData.weapons.info [weaponObjP->info.nId].flashSound);
else
	audio.CreateSegmentSound (gameData.weapons.info [weaponObjP->info.nId].flashSound,
									  weaponObjP->info.nSegment, 0, weaponObjP->info.position.vPos, 0, I2X (1));
//	Delete the original CObject.  Its only purpose in life was to determine which CObject to home in on.
ReleaseObject (OBJ_IDX (weaponObjP));
if (nTargetObj != -1)
	vTargetPos = OBJECTS [nTargetObj].info.position.vPos;
else {	//	If couldn't lock on anything, fire straight ahead.
	tFVIQuery	fq;
	tFVIData		hit_data;
	int			fate;
	CFixVector	vPerturb, perturbed_fvec;

	vPerturb = CFixVector::Random();
	perturbed_fvec = bSpectate ? gameStates.app.playerPos.mOrient.FVec () : parentObjP->info.position.mOrient.FVec ()
	               + vPerturb * (I2X (1) / 16);
	vTargetPos = *vMuzzle + perturbed_fvec * MAX_OMEGA_DIST;
	fq.startSeg = nFiringSeg;
	fq.p0 = vMuzzle;
	fq.p1	= &vTargetPos;
	fq.radP0 =
	fq.radP1 = 0;
	fq.thisObjNum = OBJ_IDX (parentObjP);
	fq.ignoreObjList = NULL;
	fq.flags = FQ_IGNORE_POWERUPS | FQ_TRANSPOINT | FQ_CHECK_OBJS;		//what about trans walls???
	fq.bCheckVisibility = false;
	fate = FindVectorIntersection (&fq, &hit_data);
	if (fate != HIT_NONE) {
		Assert (hit_data.hit.nSegment != -1);		//	How can this be?  We went from inside the mine to outside without hitting anything?
		vTargetPos = hit_data.hit.vPoint;
		}
	}
//	This is where we create a pile of omega blobs!
CreateOmegaBlobs (nFiringSeg, vMuzzle, &vTargetPos, parentObjP, (nTargetObj < 0) ? NULL : OBJECTS + nTargetObj);
}

// ---------------------------------------------------------------------------------
