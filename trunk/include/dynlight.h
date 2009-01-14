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

#ifndef _DYNLIGHT_H
#define _DYNLIGHT_H

//------------------------------------------------------------------------------

#define	MAX_LIGHTS_PER_PIXEL 8
#define USE_OGL_LIGHTS			0
#define MAX_OGL_LIGHTS			(64 * 64) //MUST be a power of 2!
#define MAX_SHADER_LIGHTS		1000

//------------------------------------------------------------------------------

class COglMaterial {
	public:
#if 0 //using global default values instead
		CFloatVector	diffuse;
		CFloatVector	ambient;
#endif
		CFloatVector	specular;
		CFloatVector	emissive;
		ubyte				shininess;
		ubyte				bValid;
		short				nLight;
	public:
		COglMaterial () { Init (); }
		void Init (void);
};

//------------------------------------------------------------------------------

class CActiveDynLight;

class CLightRenderData {
	public:
		CFloatVector		vPosf [2];
		fix					xDistance;
		short					nVerts [4];
		int					nTarget;	//lit segment/face
		int					nFrame;
		ubyte					bShadow;
		ubyte					bLightning;
		ubyte					bExclusive;
		ubyte					bUsed [MAX_THREADS];
		CActiveDynLight*	activeLightsP [MAX_THREADS];

	public:
		CLightRenderData ();
	};

//------------------------------------------------------------------------------

class CDynLightInfo {
	public:
		tFace*			faceP;
		CFixVector		vPos;
		CFloatVector	vDirf;
		tRgbaColorf		color;
		float				fBrightness;
		float				fBoost;
		float				fRange;
		float				fRad;
		float				fSpotAngle;
		float				fSpotExponent;
		short				nSegment;
		short				nSide;
		short				nObject;
		ubyte				nPlayer;
		ubyte				nType;
		ubyte				bState;
		ubyte				bOn;
		ubyte				bSpot;
		ubyte				bVariable;
		ubyte				bPowerup;
	public:
		CDynLightInfo () { Init (); }
		void Init (void) { memset (this, 0, sizeof (*this)); }
};

class CDynLight {
	public:
		CFixVector			vDir;
		CFloatVector		color;
		CFloatVector		fSpecular;
		CFloatVector		fEmissive;
		ubyte					bTransform;
		CDynLightInfo		info;
		CLightRenderData	render;
		CShadowLightData	shadow;

	public:
		CDynLight ();
		void Init (void);
		int Compare (CDynLight& other);
		inline bool operator< (CDynLight& other)
		 { return Compare (other) < 0; }
		inline bool operator> (CDynLight& other)
		 { return Compare (other) > 0; }
		inline bool operator== (CDynLight& other)
		 { return Compare (other) == 0; }
	};

//------------------------------------------------------------------------------

class CActiveDynLight {
	public:
		short			nType;
		CDynLight*	pl;
};

class CDynLightIndex {
	public:
		short	nFirst;
		short	nLast;
		short	nActive;
		short	iVertex;
		short	iStatic;
	};

//------------------------------------------------------------------------------

#define MAX_NEAREST_LIGHTS 32

class CHeadlightData {
	public:
		CDynLight*			pl [MAX_PLAYERS];
		CFloatVector		pos [MAX_PLAYERS];
		CFloatVector3		dir [MAX_PLAYERS];
		float					brightness [MAX_PLAYERS];
		int					nLights;
	public:
		CHeadlightData () { memset (this, 0, sizeof (*this)); }
};

class CDynLightData {
	public:
		CDynLight			lights [2][MAX_OGL_LIGHTS];
		CActiveDynLight	active [MAX_THREADS][MAX_OGL_LIGHTS];
		CDynLightIndex		index [2][MAX_THREADS];
		CArray<short>		nearestSegLights;		//the 8 nearest static lights for every segment
		CArray<short>		nearestVertLights;	//the 8 nearest static lights for every vertex
		CArray<ubyte>		variableVertLights;	//the 8 nearest veriable lights for every vertex
		CArray<short>		owners;
		CHeadlightData		headlights;
		COglMaterial		material;
		CFBO					fbo;
		short					nLights [2];
		short					nVariable;
		short					nDynLights;
		short					nVertLights;
		short					nHeadlights [MAX_PLAYERS];
		short					nSegment;
		GLuint				nTexHandle;

	public:
		CDynLightData ();
		bool Create (void);
		void Init (void);
		void Destroy (void);
};

//------------------------------------------------------------------------------

class CLightManager {
	private:
		CDynLightData	m_data;

	public:
		CLightManager () { Init (); }
		~CLightManager () { Destroy (); }
		void Init (void);
		void Destroy (void);
		bool Create (void);

		void SetColor (short nLight, float red, float green, float blue, float fBrightness);
		void SetPos (short nObject);
		short Find (short nSegment, short nSide, short nObject);
		short Update (tRgbaColorf *colorP, float fBrightness, short nSegment, short nSide, short nObject);
		int LastEnabled (void);
		void Swap (CDynLight* pl1, CDynLight* pl2);
		int Toggle (short nSegment, short nSide, short nObject, int bState);
		void Register (tFaceColor *colorP, short nSegment, short nSide);
		int Add (tFace* faceP, tRgbaColorf *colorP, fix xBrightness, short nSegment,
				   short nSide, short nObject, short nTexture, CFixVector *vPos);
		void Delete (short nLight);
		void DeleteLightnings (void);
		int Delete (short nSegment, short nSide, short nObject);
		void Reset (void);
		void SetMaterial (short nSegment, short nSide, short nObject);
		void Lights (void);
		void AddFromGeometry (void);
		void Transform (int bStatic, int bVariable);
		ubyte VariableVertexLights (int nVertex);
		void SetNearestToVertex (int nFace, int nVertex, CFixVector *vNormalP, ubyte nType, int bStatic, int bVariable, int nThread);
		int SetNearestToFace (tFace* faceP, int bTextured);
		short SetNearestToSegment (int nSegment, int nFace, int bVariable, int nType, int nThread);
		void SetNearestStatic (int nSegment, int bStatic, ubyte nType, int nThread);
		short SetNearestToPixel (short nSegment, short nSide, CFixVector *vNormal, CFixVector *vPixelPos, float fLightRad, int nThread);
		void ResetNearestStatic (int nSegment, int nThread);
		void ResetNearestToVertex (int nVertex, int nThread);
		int SetNearestToSgmAvg (short nSegment);
		void ResetSegmentLights (void);
		tFaceColor* AvgSgmColor (int nSegment, CFixVector *vPosP);
		void GatherStaticLights (int nLevel);
		void GatherStaticVertexLights (int nVertex, int nMax, int nThread);

	private:
		static int IsFlickering (short nSegment, short nSide);
		int IsDestructible (short nTexture);
		void DeleteFromList (CDynLight* pl, short nLight);

		int SetActive (CActiveDynLight* activeLightsP, CDynLight* prl, short nType, int nThread);
		CDynLight* GetActive (CActiveDynLight* activeLightsP, int nThread);

		void ResetUsed (CDynLight* prl, int nThread);
		void ResetActive (int nThread, int nActive);
		void ResetAllUsed (int bVariable, int nThread);
		void Sort (void);
};

//------------------------------------------------------------------------------

extern int nMaxNearestLights [21];

extern CLightManager lightManager;

//------------------------------------------------------------------------------

void RegisterLight (tFaceColor *pc, short nSegment, short nSide);
int AddDynLight (tFace *faceP, tRgbaColorf *pc, fix xBrightness, 
					  short nSegment, short nSide, short nOwner, short nTexture, CFixVector *vPos);
int RemoveDynLight (short nSegment, short nSide, short nObject);
void AddDynGeometryLights (void);
void DeleteDynLight (short nLight);
void RemoveDynLights (void);
void SetDynLightPos (short nObject);
void SetDynLightMaterial (short nSegment, short nSide, short nObject);
void MoveDynLight (short nObject);
void TransformDynLights (int bStatic, int bVariable);
short FindDynLight (short nSegment, short nSide, short nObject);
int ToggleDynLight (short nSegment, short nSide, short nObject, int bState);
void SetDynLightMaterial (short nSegment, short nSide, short nObject);
void SetNearestVertexLights (int nFace, int nVertex, CFixVector *vNormalP, ubyte nType, int bStatic, int bVariable, int nThread);
int SetNearestFaceLights (tFace *faceP, int bTextured);
short SetNearestPixelLights (short nSegment, short nSide, CFixVector *vNormal, CFixVector *vPixelPos, float fLightRad, int nThread);
void SetNearestStaticLights (int nSegment, int bStatic, ubyte nType, int nThread);
void ResetNearestStaticLights (int nSegment, int nThread);
void ResetNearestVertexLights (int nVertex, int nThread);
short SetNearestSegmentLights (int nSegment, int nFace, int bVariable, int nType, int nThread);
void ComputeStaticVertexLights (int nVertex, int nMax, int nThread);
void ComputeStaticDynLighting (int nLevel);
CLightRenderData *GetActiveRenderLight (CActiveDynLight *activeLightsP, int nThread);
int CreatePerPixelLightingShader (int nType, int nLights);
void InitPerPixelLightingShaders (void);
void ResetPerPixelLightingShaders (void);
int CreateLightmapShader (int nType);
void InitLightmapShaders (void);
void ResetLightmapShaders (void);
void InitHeadlightShaders (int nLights);
char *BuildLightingShader (const char *pszTemplate, int nLights);
tFaceColor *AvgSgmColor (int nSegment, CFixVector *vPos);
void ResetSegmentLights (void);
int IsLight (int tMapNum);
void ResetUsedLight (CLightRenderData *prl, int nThread);
void ResetUsedLights (int bVariable, int nThread);
void ResetActiveLights (int nThread, int nActive);

#define	SHOW_DYN_LIGHT \
			(!(gameStates.app.bNostalgia || gameStates.render.bBriefing || (gameStates.app.bEndLevelSequence >= EL_OUTSIDE)) && \
			 /*gameStates.render.bHaveDynLights &&*/ \
			 gameOpts->render.nLightingMethod)

#define HAVE_DYN_LIGHT	(gameStates.render.bHaveDynLights && SHOW_DYN_LIGHT)

#define	APPLY_DYN_LIGHT \
			(gameStates.render.bUseDynLight && (gameOpts->ogl.bLightObjects || gameStates.render.nState))

extern tFaceColor tMapColor, lightColor, vertColors [8];

#endif //_DYNLIGHT_H
