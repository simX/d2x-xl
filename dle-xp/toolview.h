// dlcView.h : interface of the CMineView class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __toolview_h
#define __toolview_h

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "afxcview.h"
#include "Dlc.h"
#include "DlcDoc.h"
#include "Matrix.h"
#include "poly.h"
#include "define.h"
#include "types.h"
#include "global.h"
#include "dlcres.h"
#include "textures.h"
#include "texedit.h"

                        /*--------------------------*/
#if 1
class CExtBitmapButton : public CBitmapButton
{
	public:
		DECLARE_DYNCREATE(CExtBitmapButton)
	public:
		int	m_nId;
		CWnd	*m_pParent;
		UINT	m_nState;
		int	m_nPos;
		//virtual int OnToolHitTest (CPoint point, TOOLINFO* pTI);
		BOOL AutoLoad (UINT nId, CWnd* pParent) {
			m_nId = nId;
			m_pParent = pParent;
			m_nState = WM_LBUTTONUP;
			m_nPos = -1;
			return CBitmapButton::AutoLoad (nId, pParent);
			}
		afx_msg void OnLButtonUp (UINT nFlags, CPoint point);
		afx_msg void OnLButtonDown (UINT nFlags, CPoint point);
		afx_msg void OnMouseMove (UINT nFlags, CPoint point);
		afx_msg void OnSetFocus (CWnd* pOldWnd);
		afx_msg void OnKillFocus (CWnd* pNewWnd);
		void Notify (UINT nMsg);

		DECLARE_MESSAGE_MAP ()
};
#endif
                        /*--------------------------*/

class CConvertDlg : public CDialog
{
	public:
		bool			m_bInited;
		CMine			*m_mine;
		CWnd			m_showD1;
		CWnd			m_showD2;
		HINSTANCE	m_hInst;
		HGLOBAL		m_hTextures;
		INT16			*m_pTextures;

		CConvertDlg (CWnd *pParent = NULL);
      virtual BOOL OnInitDialog ();
		void EndDialog (int nResult);
      virtual void DoDataExchange (CDataExchange *pDX);
		void CreateImgWnd (CWnd *pImgWnd, int nIdC);
		void Reset ();
		void Refresh ();
		CMine *GetMine ();
		afx_msg void OnPaint ();
		afx_msg void OnSetD1 ();
		afx_msg void OnSetD2 ();
		virtual void OnOK (void);
		inline CComboBox *CBD1 ()
			{ return (CComboBox *) GetDlgItem (IDC_CONVERT_D1); }
		inline CComboBox *CBD2 ()
			{ return (CComboBox *) GetDlgItem (IDC_CONVERT_D2); }

		DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CToolDlg : public CPropertyPage
{
	public:
		DECLARE_DYNCREATE(CToolDlg)
   protected:
      CPropertySheet *  m_pParent;
	public:
		CMine *m_mine;
		bool	m_bInited;

		CToolDlg (UINT nIdTemplate = 0, CPropertySheet *pParent = NULL);
		~CToolDlg ()
			{ m_pParent = NULL; m_mine = NULL; m_bInited = false; }
      virtual BOOL OnInitDialog () 
			{ return CPropertyPage::OnInitDialog (); }
      virtual BOOL OnSetActive () {
			if (!CPropertyPage::OnSetActive ())
				return FALSE;
         UpdateData (FALSE);
         return TRUE;
         };
      virtual void DoDataExchange (CDataExchange * pDX) {};
		void Refresh (void)
			{ UpdateData (FALSE); }
		void DDX_Double (CDataExchange * pDX, int nIDC, double& fVal, double min = 1, double max = 0, LPSTR pszFmt = "%1.2f", LPSTR pszErrMsg = NULL);
		void DDX_Slider (CDataExchange * pDX, int nIdC, int& nTic);
		int DDX_Int (CDataExchange * pDX, int nIdC, int i);
		int DDX_Flag (CDataExchange * pDX, int nIdC, int i);
		void InitSlider (int nIdC, int nMin, int nMax);
		int GetCheck (int nIdC);
		void SelectItemData (CComboBox *pcb, int nItemData);
		CMine *GetMine ();
		void EnableControls (int nIdFirst, int nIdLast, BOOL bEnable);
#if 0
		int OnToolHitTest (CPoint point, TOOLINFO* pTI);
#endif
		BOOL OnToolTipNotify (UINT id, NMHDR *pNMHDR, LRESULT *pResult);
		void CreateImgWnd (CWnd * pImgWnd, int nIdC);
		afx_msg void OnSelectPrevTab ();
		afx_msg void OnSelectNextTab ();
		void GetCtrlClientRect (CWnd *pWnd, CRect& rc);
		inline bool Inited ()
			{ return m_bInited; }
		inline CComboBox *CBCtrl (int nId)
			{ return (CComboBox *) GetDlgItem (nId); }
		inline CListBox *LBCtrl (int nId)
			{ return (CListBox *) GetDlgItem (nId); }
		inline CButton *BtnCtrl (int nId)
			{ return (CButton *) GetDlgItem (nId); }
		inline CSliderCtrl *SlCtrl (int nId)
			{ return (CSliderCtrl *) GetDlgItem (nId); }
		inline CScrollBar *SBCtrl (int nId)
			{ return (CScrollBar *) GetDlgItem (nId); }

		DECLARE_MESSAGE_MAP()
};

                        /*--------------------------*/

class CTexToolDlg : public CToolDlg
{
	public:
		DECLARE_DYNCREATE(CTexToolDlg)

	CWnd		m_textureWnd;
	UINT		m_nTimer;
	int		m_nTexWndId;
	int		m_nTimerId;
	COLORREF	m_bkColor;
	int		m_frame [2];
	bool		m_bOtherSeg;

	CTexToolDlg (UINT nIdTemplate = 0, CPropertySheet *pParent = NULL, 
					 int nTexWndId = 0, int nTimerId = -1, COLORREF bkColor = RGB (0,0,0),
					 bool bOtherSeg = false);
	~CTexToolDlg ();
	bool Refresh (INT16 nBaseTex = -1, INT16 nOvlTex = -1, INT16 nVisible = -1);
	void AnimateTexture (void);

   virtual BOOL OnInitDialog ();
	afx_msg void OnPaint ();
	afx_msg void OnTimer (UINT nIdEvent);
	virtual BOOL OnSetActive ();
	virtual BOOL OnKillActive ();
	virtual BOOL TextureIsVisible ();

	DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CAdvObjTool : public CToolDlg
{
	public:
		int		m_mass;
		int		m_drag;
		int		m_brakes;
		int		m_turnRoll;
		int		m_size;
		int		m_flags;
		int		m_shields;
		int		m_vx;
		int		m_vy;
		int		m_vz;
		int		m_tx;
		int		m_ty;
		int		m_tz;
		int		m_rvx;
		int		m_rvy;
		int		m_rvz;
		int		m_rtx;
		int		m_rty;
		int		m_rtz;
		int		m_model;
		int		m_frame;
		int		m_frameNo;
		CAdvObjTool (CPropertySheet *pParent = NULL);
      virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		virtual BOOL OnKillActive ();
		void Reset ();
		void Refresh ();
		afx_msg void OnAccept ();

		DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CDiagTool : public CToolDlg
{
	public:
		int			m_nTrigger;
		int			m_nTargets;
		int			m_iTarget;
		int			m_nCountDown;
		int			m_nSecretReturn;
		char			m_szTarget [40];
		int			m_nObjects [8];
		int			m_nContained [2];
		int			m_nErrors [2];
		int			m_bAutoFixBugs;
		int			m_bShowWarnings;
		control_center_trigger	*m_pTrigger;
		bool			m_bCheckMsgs;

		CDiagTool (CPropertySheet *pParent = NULL);
		~CDiagTool ();
      virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		void Reset ();
		void Refresh ();
		LPSTR ItemText (int nValue, LPSTR pszPrefix = NULL);
		void CountObjects (void);
		int CountTextures (void);
		afx_msg void OnCheckMine ();
		afx_msg void OnShowBug ();
		afx_msg void OnClearList ();
		afx_msg void OnFixBugs ();
		afx_msg void OnShowWarnings ();
		int AddMessage (const char *pszMsg, int nMaxMsgs = 100, bool bCheckMsg = false);
		bool UpdateStats (char *szError, int nErrorLevel, 
							   int segnum = -1, int sidenum = -1, int linenum = -1, int pointnum = -1, 
							   int childnum = -1, int wallnum = -1, int trignum = -1, int objnum = -1);
		double CalcFlatnessRatio (INT16 segnum, INT16 sidenum);
		double CalcDistance (vms_vector *v1,vms_vector *v2,vms_vector *v3);
		double CalcAngle (INT16 vert0,INT16 vert1,INT16 vert2,INT16 vert3);
		void ClearBugList ();
		int CheckId (CDObject *obj);
		bool CheckSegments ();
		bool CheckSegTypes ();
		bool CheckWalls ();
		bool CheckTriggers ();
		bool CheckObjTriggerList (INT16 nTrigger);
		bool CheckObjTriggers ();
		bool CheckObjects ();
		bool CheckVertices ();
		bool CheckBotGens ();
		bool CheckEquipGens ();
		bool MarkSegment (INT16 segnum);
		inline CListView *LVStats ()
			{ return (CListView *) GetDlgItem (IDC_DIAG_STATS); }
		inline CListBox *LBBugs ()
			{ return LBCtrl (IDC_DIAG_BUGLIST); }
		inline CWnd *TargetEdit ()
			{ return GetDlgItem (IDC_REACTOR_TARGET); }
		CDWall *OppWall (UINT16 segnum, UINT16 sidenum);

		DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CReactorTool : public CToolDlg
{
	public:
		int			m_nTrigger;
		int			m_nTargets;
		int			m_iTarget;
		int			m_nCountDown;
		int			m_nSecretReturn;
		char			m_szTarget [40];
		control_center_trigger	*m_pTrigger;

		CReactorTool (CPropertySheet *pParent = NULL);
      virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		void EnableControls (BOOL bEnable);
		void InitLBTargets ();
		void Reset ();
		void Refresh ();
		void AddTarget (INT16 segnum, INT16 sidenum);
		int FindTarget (INT16 segnum, INT16 sidenum);
		afx_msg void OnAddTarget ();
		afx_msg void OnDeleteTarget ();
		afx_msg void OnAddWallTarget ();
		afx_msg void OnDeleteWallTarget ();
		afx_msg void OnCountDown ();
		afx_msg void OnSecretReturn ();
		afx_msg void OnSetTarget ();
		inline CListBox *LBTargets ()
			{ return LBCtrl (IDC_REACTOR_TARGETLIST); }
		inline CWnd *TargetEdit ()
			{ return GetDlgItem (IDC_REACTOR_TARGET); }

		DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CMissionTool : public CToolDlg
{
	public:
		MISSION_DATA	m_missionData;
		char				m_szLevel [26];

		CMissionTool (CPropertySheet *pParent = NULL);
	   virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		virtual void OnOK (void);
		virtual void OnCancel (void);
		afx_msg void OnSetLevelName (void);
		afx_msg void OnAdd (void);
		afx_msg void OnDelete (void);
		afx_msg void OnRename (void);
		afx_msg void OnUp (void);
		afx_msg void OnDown (void);
		afx_msg void OnFromHog (void);
		afx_msg void OnLoadLevel (void);
		void Refresh (void);
		virtual BOOL OnSetActive ();
		void BuildLevelList (void);
		LPSTR CopyLevelName (LPSTR pszDest, LPSTR pszSrc);
		LPSTR FixLevelName (LPSTR pszName);
		inline CListBox *LBLevels ()
			{ return LBCtrl (IDC_MISSION_LEVELLIST); }
		DECLARE_MESSAGE_MAP()
};

                        /*--------------------------*/

class CObjectTool : public CToolDlg
{
	public:
		CWnd		m_showObjWnd;
		CWnd		m_showSpawnWnd;
		CWnd		m_showTextureWnd;
		int		m_nSpawnQty;
		char		m_szInfo [200];

		CObjectTool (CPropertySheet *pParent = NULL);
		~CObjectTool ();
      virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		void Reset ();
		void EnableControls (BOOL bEnable);
		void Refresh ();
		void RefreshRobot ();
		void DrawObjectImages ();
		void DrawObject (CWnd *pWnd, int type, int id);
		void SetTextureOverride ();
		bool SetPlayerId (CDObject *obj, int objType, int *ids, int maxIds, char *pszError);
		void SetObjectId (CComboBox *pcb, INT16 type, INT16 id, INT16 flag = 0);

		void CBInit (CComboBox *pcb, char* pszNames [], UINT8 *pIndex, UINT8 *pItemData, int nMax, int nType = 0, bool bAddNone = false);
		int CBAddString (CComboBox *pcb, char *psz);
		void InitSliders ();
		void UpdateSliders (int i = -1);
		int GetSliderData (CScrollBar *pScrollBar);
		double SliderFactor (int nId);
		void UpdateRobot ();
		int ObjOfAKindCount (int nType = -1, int nId = -1);
		int GetObjectsOfAKind (int nType, CDObject *objList []);
		void SetNewObjId (CDObject *obj, int nType, int nId, int nMaxId);

		afx_msg void OnPaint ();
		afx_msg void OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);

		afx_msg void OnAdd ();
		afx_msg void OnDelete ();
		afx_msg void OnDeleteAll ();
		afx_msg void OnMove ();
		afx_msg void OnReset ();
		afx_msg void OnDefault ();
		afx_msg void OnAdvanced ();
		afx_msg void OnSetObject ();
		afx_msg void OnSetObjType ();
		afx_msg void OnSetObjId ();
		afx_msg void OnSetTexture ();
		afx_msg void OnSetSpawnType ();
		afx_msg void OnSetSpawnId ();
		afx_msg void OnSetSpawnQty ();
		afx_msg void OnSetSoundExplode ();
		afx_msg void OnSetSoundSee ();
		afx_msg void OnSetSoundAttack ();
		afx_msg void OnSetSoundClaw ();
		afx_msg void OnSetSoundDeath ();
		afx_msg void OnSetObjAI ();
		afx_msg void OnSetObjClassAI ();
		afx_msg void OnSetWeapon1 ();
		afx_msg void OnSetWeapon2 ();
		afx_msg void OnSetContType ();
		afx_msg void OnSetContId ();
		afx_msg void OnAIKamikaze ();
		afx_msg void OnAICompanion ();
		afx_msg void OnAIThief ();
		afx_msg void OnAISmartBlobs ();
		afx_msg void OnAIPursue ();
		afx_msg void OnAICharge ();
		afx_msg void OnAIEDrain ();
		afx_msg void OnAIBossType ();
		afx_msg void OnBright ();
		afx_msg void OnCloaked ();
		afx_msg void OnMultiplayer ();
		afx_msg void OnSort ();

		inline CComboBox *CBObjNo ()
			{ return CBCtrl (IDC_OBJ_OBJNO); }
		inline CComboBox *CBObjType ()
			{ return CBCtrl (IDC_OBJ_TYPE); }
		inline CComboBox *CBObjId ()
			{ return CBCtrl (IDC_OBJ_ID); }
		inline CComboBox *CBObjAI ()
			{ return CBCtrl (IDC_OBJ_AI); }
		inline CComboBox *CBObjTexture ()
			{ return CBCtrl (IDC_OBJ_TEXTURE); }
		inline CComboBox *CBSpawnType ()
			{ return CBCtrl (IDC_OBJ_SPAWN_TYPE); }
		inline CComboBox *CBBossType ()
			{ return CBCtrl (IDC_OBJ_AI_BOSSTYPE); }
		inline CComboBox *CBSpawnId ()
			{ return CBCtrl (IDC_OBJ_SPAWN_ID); }
		inline CComboBox *CBObjClassAI ()
			{ return CBCtrl (IDC_OBJ_CLASS_AI); }
		inline CComboBox *CBContType ()
			{ return CBCtrl (IDC_OBJ_CONT_TYPE); }
		inline CComboBox *CBContId ()
			{ return CBCtrl (IDC_OBJ_CONT_ID); }
		inline CComboBox *CBExplType ()
			{ return CBCtrl (IDC_OBJ_EXPLTYPE); }
		inline CComboBox *CBWeapon1 ()
			{ return CBCtrl (IDC_OBJ_WEAPON1); }
		inline CComboBox *CBWeapon2 ()
			{ return CBCtrl (IDC_OBJ_WEAPON2); }
		inline CComboBox *CBSoundExpl ()
			{ return CBCtrl (IDC_OBJ_SOUND_EXPLODE); }
		inline CComboBox *CBSoundSee ()
			{ return CBCtrl (IDC_OBJ_SOUND_SEE); }
		inline CComboBox *CBSoundAttack ()
			{ return CBCtrl (IDC_OBJ_SOUND_ATTACK); }
		inline CComboBox *CBSoundClaw ()
			{ return CBCtrl (IDC_OBJ_SOUND_CLAW); }
		inline CComboBox *CBSoundDeath ()
			{ return CBCtrl (IDC_OBJ_SOUND_DEATH); }

		DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CEffectTool : public CToolDlg
{
	public:
		CDObject				*m_obj;
		tSmokeInfo			m_smoke;
		tLightningInfo		m_lightning;
		int					m_nBufferId;

		CEffectTool (CPropertySheet *pParent = NULL);
		~CEffectTool ();
      virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		virtual BOOL OnKillActive ();
		void Reset ();
		void EnableControls (BOOL bEnable);
		void Refresh ();
		bool AddEffect ();
		void LoadEffectList ();
		void HiliteTarget (void);
		bool FindSlider (CScrollBar *pScrollBar);

		inline CComboBox *CBEffects ()
			{ return CBCtrl(IDC_EFFECT_OBJECTS); }
		inline CComboBox *CBStyle ()
			{ return CBCtrl(IDC_LIGHTNING_STYLE); }
		inline CComboBox *CBType ()
			{ return CBCtrl(IDC_SMOKE_TYPE); }

		afx_msg void OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);
		afx_msg void OnSetObject ();
		afx_msg void OnSetStyle ();
		afx_msg void OnEdit ();
		afx_msg void OnAddSmoke ();
		afx_msg void OnAddLightning ();
		afx_msg void OnDelete ();
		afx_msg void OnCopy ();
		afx_msg void OnPaste ();
		afx_msg void OnPasteAll ();
#if 0
		afx_msg void OnSetSmokeLife ();
		afx_msg void OnSetSmokeSize ();
		afx_msg void OnSetSmokeDensity ();
		afx_msg void OnSetSmokeSpeed ();
		afx_msg void OnSetSmokeDrift ();
		afx_msg void OnSetSmokeBrightness ();

		afx_msg void OnSetLightningSmoothe ();
		afx_msg void OnSetLightningClamp ();
		afx_msg void OnSetLightningPlasma ();
		afx_msg void OnSetLightningSound ();
		afx_msg void OnSetLightningRandom ();
#endif
		DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CCubeTool : public CToolDlg
{
	public:
		int		m_nCube;
		int		m_nSide;
		int		m_nPoint;
		int		m_nVertex;
		int		m_nType;
		int		m_nOwner;
		int		m_nGroup;
		int		m_bEndOfExit;
		double	m_nLight;
		double	m_nCoord [3];
		int		m_nLastCube;
		int		m_nLastSide;
		int		m_bSetDefTexture;

		CCubeTool (CPropertySheet *pParent = NULL);
		virtual BOOL OnInitDialog ();
		virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		void InitCBCubeNo ();
		void Reset ();
		void EnableControls (BOOL bEnable);
		void Refresh ();
		void OnSide (int nSide);
		void OnPoint (int nPoint);
		int FindBot (CListBox *plb, LPSTR pszBot = NULL);
		int FindEquip (CListBox *plb, LPSTR pszBot = NULL);
		bool IsBotMaker (CDSegment *seg);
		bool IsEquipMaker (CDSegment *seg);
		void SetDefTexture (INT16 tmapnum);
	
		afx_msg void OnSetCube ();
		afx_msg void OnSetType ();
		afx_msg void OnSetOwner ();
		afx_msg void OnSetGroup ();
		afx_msg void OnSetCoord ();
		afx_msg void OnResetCoord ();
		afx_msg void OnLight ();
		afx_msg void OnSide1 ();
		afx_msg void OnSide2 ();
		afx_msg void OnSide3 ();
		afx_msg void OnSide4 ();
		afx_msg void OnSide5 ();
		afx_msg void OnSide6 ();
		afx_msg void OnPoint1 ();
		afx_msg void OnPoint2 ();
		afx_msg void OnPoint3 ();
		afx_msg void OnPoint4 ();
		afx_msg void OnAddCube ();
		afx_msg void OnAddBotGen ();
		afx_msg void OnAddFuelCen ();
		afx_msg void OnAddEquipGen ();
		afx_msg void OnAddRepairCen ();
		afx_msg void OnAddControlCen ();
		afx_msg void OnAddWaterCube ();
		afx_msg void OnAddLavaCube ();
		afx_msg void OnSplitCube ();
		afx_msg void OnDeleteCube ();
		afx_msg void OnOtherCube ();
		afx_msg void OnAddObj ();
		afx_msg void OnDeleteObj ();
		afx_msg void OnWallDetails ();
		afx_msg void OnTriggerDetails ();
		afx_msg void OnEndOfExit ();

		void AddBot (void);
		void DeleteBot (void);
		void AddEquip (void);
		void DeleteEquip (void);

		inline CComboBox *CBCubeNo ()
			{ return CBCtrl(IDC_CUBE_CUBENO); }
		inline CComboBox *CBType ()
			{ return CBCtrl(IDC_CUBE_TYPE); }
		inline CListBox *LBTriggers ()
			{ return LBCtrl (IDC_CUBE_TRIGGERS); }
		inline CListBox *LBUsedBots ()
			{ return LBCtrl (IDC_CUBE_USEDBOTS); }
		inline CListBox *LBAvailBots ()
			{ return LBCtrl (IDC_CUBE_AVAILBOTS); }
		inline CButton *EndOfExit ()
			{ return BtnCtrl (IDC_CUBE_ENDOFEXIT); }
		inline CComboBox *CBOwner ()
			{ return CBCtrl(IDC_CUBE_OWNER); }

		DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CWallTool : public CTexToolDlg
{
	public:
		CWnd		m_textureWnd;
		int		m_nCube;
		int		m_nSide;
		int		m_nTrigger;
		int		m_nWall [2];
		int		m_nType;
		int		m_nClip;
		double	m_nStrength;
		double	m_nCloak;
		int		m_bFlyThrough;
		int		m_bKeys [4];
		int		m_bFlags [7];
		char		m_szMsg [256];
		CDWall	*m_pWall [2];
		CDWall	m_defWall;
		CDWall	m_defDoor;
		INT16		m_defTexture;
		INT16		m_defDoorTexture;
		INT16		m_defOvlTexture;
		BOOL		m_bBothSides;
		bool		m_bLock;
		bool		m_bDelayRefresh;

		CWallTool (CPropertySheet *pParent = NULL);
		~CWallTool ();
      virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		virtual BOOL OnKillActive ();
		void InitCBWallNo ();
		void Reset ();
		void EnableControls (BOOL bEnable);
		void Refresh ();
		virtual BOOL TextureIsVisible ();
		bool GetWalls ();
		CDWall *GetOtherWall (void);

		afx_msg void OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);
		afx_msg void OnBothSides ();
		afx_msg void OnAddWall ();
		afx_msg void OnDeleteWall ();
		afx_msg void OnDeleteWallAll ();
		afx_msg void OnLock ();
		afx_msg void OnOtherSide ();
		afx_msg void OnSetWall ();
		afx_msg void OnSetType ();
		afx_msg void OnSetClip ();
		afx_msg void OnNoKey ();
		afx_msg void OnBlueKey ();
		afx_msg void OnGoldKey ();
		afx_msg void OnRedKey ();
		afx_msg void OnBlasted ();
		afx_msg void OnDoorOpen ();
		afx_msg void OnDoorLocked ();
		afx_msg void OnDoorAuto ();
		afx_msg void OnIllusionOff ();
		afx_msg void OnSwitch ();
		afx_msg void OnBuddyProof ();
		afx_msg void OnRenderAdditive ();
		afx_msg void OnIgnoreMarker ();
		afx_msg void OnAddDoorNormal ();
		afx_msg void OnAddDoorExit ();
		afx_msg void OnAddDoorSecretExit ();
		afx_msg void OnAddDoorPrison ();
		afx_msg void OnAddDoorGuideBot ();
		afx_msg void OnAddWallFuelCell ();
		afx_msg void OnAddWallIllusion ();
		afx_msg void OnAddWallForceField ();
		afx_msg void OnAddWallFan ();
		afx_msg void OnAddWallGrate ();
		afx_msg void OnAddWallWaterfall ();
		afx_msg void OnAddWallLavafall ();

		void OnKey (int i);
		void OnFlag (int i); 
		void OnStrength ();
		void OnCloak ();

		inline CComboBox *CBWallNo ()
			{ return CBCtrl(IDC_WALL_WALLNO); }
		inline CComboBox *CBType ()
			{ return CBCtrl(IDC_WALL_TYPE); }
		inline CComboBox *CBClipNo ()
			{ return CBCtrl(IDC_WALL_CLIPNO); }
		inline CButton *KeyBtn (int i)
			{ return BtnCtrl (IDC_WALL_NOKEY + i); }
		inline CButton *FlagBtn (int i)
			{ return BtnCtrl (IDC_WALL_BLASTED + i); }
		inline CScrollBar *TransparencySlider ()
			{ return (CScrollBar *) GetDlgItem (IDC_WALL_TRANSPARENCY); }

		DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CTriggerTool : public CTexToolDlg
{
	public:
		int					m_nClass;
		int					m_nType;
		int					m_nTime;
		int					m_nTargets;
		int					m_iTarget;
		int					m_bD1Flags [MAX_TRIGGER_FLAGS];
		int					m_bD2Flags [6];
		int					m_nSliderValue;
		double				m_nStrength;
		CDTrigger			m_defTrigger;
		CDTrigger			*m_pTrigger;
		int					m_nTrigger;
		CDTrigger			*m_pStdTrigger;
		int					m_nStdTrigger;
		CDObjTriggerList	*m_pObjTrigger;
		int					m_nObjTrigger;
		char					m_szTarget [40];
		int					m_bAutoAddWall;
		CWnd					m_showObjWnd;
		CWnd					m_showTexWnd;
		bool					m_bFindTrigger;

		CTriggerTool (CPropertySheet *pParent = NULL);
		~CTriggerTool ();
		virtual BOOL OnInitDialog ();
		virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		virtual BOOL OnKillActive ();
		void LoadTextureListBoxes ();
		void InitCBTriggerNo ();
		void InitLBTargets ();
		void Reset ();
		void EnableControls (BOOL bEnable);
		void Refresh ();
		void AddTarget (INT16 segnum, INT16 sidenum);
		int FindTarget (INT16 segnum, INT16 sidenum);
		bool OnD1Flag (int i, int j = -1);
		void OnD2Flag (int i, int j = 0);
		void SetTriggerPtr (void);
		void ClearObjWindow (void);
		void DrawObjectImage ();
		virtual BOOL TextureIsVisible ();
		bool FindTrigger (INT16 &trignum);
		bool TriggerHasSlider (void);
		int NumTriggers ();
		void OnSelect1st ();
		void OnSelect2nd ();
		void SelectTexture (int nIdC, bool bFirst);
		afx_msg void OnPaint ();
		afx_msg void OnAddTrigger ();
		afx_msg void OnDeleteTrigger ();
		afx_msg void OnDeleteTriggerAll ();
		afx_msg void OnSetTrigger ();
		afx_msg void OnSetType ();
		afx_msg void OnSetTarget ();
		afx_msg void OnAddTarget ();
		afx_msg void OnDeleteTarget ();
		afx_msg void OnAddWallTarget ();
		afx_msg void OnDeleteWallTarget ();
		afx_msg void OnCopyTrigger ();
		afx_msg void OnPasteTrigger ();
		afx_msg void OnStrength ();
		afx_msg void OnTime ();
		afx_msg void OnStandardTrigger ();
		afx_msg void OnObjectTrigger ();
		afx_msg void OnD2Flag1 ();
		afx_msg void OnD2Flag2 ();
		afx_msg void OnD2Flag3 ();
		afx_msg void OnD2Flag4 ();
		afx_msg void OnD2Flag5 ();
		afx_msg void OnD1Flag1 ();
		afx_msg void OnD1Flag2 ();
		afx_msg void OnD1Flag3 ();
		afx_msg void OnD1Flag4 ();
		afx_msg void OnD1Flag5 ();
		afx_msg void OnD1Flag6 ();
		afx_msg void OnD1Flag7 ();
		afx_msg void OnD1Flag8 ();
		afx_msg void OnD1Flag9 ();
		afx_msg void OnD1Flag10 ();
		afx_msg void OnAddOpenDoor ();
		afx_msg void OnAddRobotMaker ();
		afx_msg void OnAddShieldDrain ();
		afx_msg void OnAddEnergyDrain ();
		afx_msg void OnAddControlPanel ();
		afx_msg void OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);
		inline CComboBox *CBTriggerNo ()
			{ return CBCtrl(IDC_TRIGGER_TRIGGERNO); }
		inline CComboBox *CBType ()
			{ return CBCtrl(IDC_TRIGGER_D2TYPE); }
		inline CListBox *LBTargets ()
			{ return (CListBox *) GetDlgItem (IDC_TRIGGER_TARGETLIST); }
		inline CWnd *TargetEdit ()
			{ return GetDlgItem (IDC_TRIGGER_TARGET); }
		inline CScrollBar *SpeedBoostSlider ()
			{ return (CScrollBar *) GetDlgItem (IDC_TRIGGER_SLIDER); }
		inline CComboBox *CBTexture1 ()
			{ return CBCtrl(IDC_TRIGGER_TEXTURE1); }
		inline CComboBox *CBTexture2 ()
			{ return CBCtrl(IDC_TRIGGER_TEXTURE2); }
		inline INT16 Texture1 (void)
			{ return (INT16) ((m_nTrigger >= 0) && (m_pTrigger && (m_nType == TT_CHANGE_TEXTURE)) ? m_pTrigger->value & 0xffff : 0); }
		inline INT16 Texture2 (void)
			{ return (INT16) ((m_nTrigger >= 0) && (m_pTrigger && (m_nType == TT_CHANGE_TEXTURE)) ? m_pTrigger->value >> 16 : 0); }
		inline void SetTexture (INT16 texture1, INT16 texture2) {
			if ((m_nTrigger >= 0) && m_pTrigger && (m_nType == TT_CHANGE_TEXTURE)) {
				if (texture1 < 0)
					texture1 = Texture1 ();
				if (texture2 < 0)
					texture2 = Texture2 ();
				m_pTrigger->value = (FIX) (texture2 << 16) + (FIX) texture1; 
				}
			}

		DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CTxtFilterTool : public CToolDlg
{
	public:
		CTxtFilterTool (CPropertySheet *pParent = NULL);
		~CTxtFilterTool ();
      virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		//virtual BOOL OnSetActive ();
		virtual BOOL OnKillActive ();
		
		void SetFilter (int i);
		void SetFilterRange (UINT32 nFlags, int nValue);

		afx_msg void OnGrayRock ();
		afx_msg void OnBrownRock ();
		afx_msg void OnRedRock ();
		afx_msg void OnYellowRock ();
		afx_msg void OnGreenRock ();
		afx_msg void OnBlueRock ();
		afx_msg void OnIce ();
		afx_msg void OnStones ();
		afx_msg void OnGrass ();
		afx_msg void OnSand ();
		afx_msg void OnLava ();
		afx_msg void OnWater ();
		afx_msg void OnSteel ();
		afx_msg void OnConcrete ();
		afx_msg void OnBricks ();
		afx_msg void OnTarmac ();
		afx_msg void OnWalls ();
		afx_msg void OnFloors ();
		afx_msg void OnCeilings ();
		afx_msg void OnGrates ();
		afx_msg void OnFans ();
		afx_msg void OnLights ();
		afx_msg void OnEnergy ();
		afx_msg void OnForcefields ();
		afx_msg void OnTech ();
		afx_msg void OnLabels ();
		afx_msg void OnMonitors ();
		afx_msg void OnStripes ();
		afx_msg void OnMovers ();
		afx_msg void OnDoors ();
		afx_msg void OnSwitches ();
		afx_msg void OnRockAll ();
		afx_msg void OnRockNone ();
		afx_msg void OnNatureAll ();
		afx_msg void OnNatureNone ();
		afx_msg void OnBuildingAll ();
		afx_msg void OnBuildingNone ();
		afx_msg void OnTechAll ();
		afx_msg void OnTechNone ();
		afx_msg void OnSignAll ();
		afx_msg void OnSignNone ();
		afx_msg void OnOtherAll ();
		afx_msg void OnOtherNone ();
		afx_msg void OnTxtAll ();
		afx_msg void OnTxtNone ();
		afx_msg void OnTxtInvert ();

		DECLARE_MESSAGE_MAP()
};

                        /*--------------------------*/

class CTextureTool : public CTexToolDlg
{
	public:
		CExtBitmapButton	m_btnZoomIn;
		CExtBitmapButton	m_btnZoomOut;
		CExtBitmapButton	m_btnHShrink;
		CExtBitmapButton	m_btnVShrink;
		CExtBitmapButton	m_btnHALeft;
		CExtBitmapButton	m_btnHARight;
		CExtBitmapButton	m_btnVAUp;
		CExtBitmapButton	m_btnVADown;
		CExtBitmapButton	m_btnRALeft;
		CExtBitmapButton	m_btnRARight;
		CBitmapButton		m_btnStretch2Fit;
		CBitmapButton		m_btnReset;
		CBitmapButton		m_btnResetMarked;
		CBitmapButton		m_btnAlignAll;
		CBitmapButton		m_btnChildAlign;
		CBitmapButton		m_btnAddLight;
		CBitmapButton		m_btnDelLight;
		CBitmapButton		m_btnHFlip;
		CBitmapButton		m_btnVFlip;

		CPaletteWnd			m_paletteWnd;

		char					m_szTextureBuf [100];
		int					last_texture1,
								last_texture2,
								last_mode;
		int					save_texture1,
								save_texture2;
		uvl					save_uvls[4];
//		int					frame [2];
		double				m_lights [4];
//		CWnd					m_textureWnd;
		CWnd					m_alignWnd;
		CWnd					m_lightWnd;
		CWnd					m_colorWnd;
		int					m_bUse1st;
		int					m_bUse2nd;
		int					m_bShowTexture;
		int					m_bShowChildren;
		double				m_alignX;
		double				m_alignY;
		double				m_alignAngle;
		double				m_alignUvPoint [4];
		double				m_zoom;
		int					m_alignRot2nd;
		POINT					m_apts [4];
		POINT					m_minPt,
								m_maxPt,
								m_centerPt;
		UINT					m_nTimerDelay;
		UINT					m_nTimer;
		UINT					m_nEditTimer;
		UINT					m_nLightTimer;
		int					m_nLightDelay;
		double				m_nLightTime;
		int					m_nHighlight;
		char					m_szLight [33];
		int					m_iLight;
		BOOL					m_bLightEnabled;
		BOOL					m_bIgnorePlane;
		int					m_nBrightness;
		int					m_nColorIndex;
		PALETTEENTRY		m_rgbColor;
		COLORREF				m_custColors [16];
		int					m_nEditFunc;

		CTextureTool (CPropertySheet *pParent = NULL);
		~CTextureTool ();
      virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		virtual BOOL OnKillActive ();
		afx_msg void LoadTextureListBoxes ();
		afx_msg void OnPaint ();
		afx_msg void OnTimer (UINT nIdEvent);
		afx_msg void OnSaveTexture ();
		afx_msg void OnEditTexture ();
		afx_msg void OnPasteSide ();
		afx_msg void OnPasteTouching ();
		afx_msg void OnPasteMarked ();
		afx_msg void OnReplace ();
		afx_msg void OnSetLight ();
		afx_msg void OnSelect1st ();
		afx_msg void OnSelect2nd ();
		afx_msg void OnPaste1st ();
		afx_msg void OnPaste2nd ();
		afx_msg void OnAlignX ();
		afx_msg void OnAlignY ();
		afx_msg void OnAlignRot ();
		afx_msg void OnAlignLeft ();
		afx_msg void OnAlignRight ();
		afx_msg void OnAlignUp ();
		afx_msg void OnAlignDown ();
		afx_msg void OnAlignRotLeft ();
		afx_msg void OnAlignRotRight ();
		afx_msg void OnHShrink ();
		afx_msg void OnVShrink ();
		afx_msg void OnAlignReset ();
		afx_msg void OnAlignResetMarked ();
		afx_msg void OnAlignStretch2Fit ();
		afx_msg void OnAlignAll ();
		afx_msg void OnAlignChildren ();
		afx_msg void OnZoomIn ();
		afx_msg void OnZoomOut ();
		afx_msg void OnRot2nd0 ();
		afx_msg void OnRot2nd90 ();
		afx_msg void OnRot2nd180 ();
		afx_msg void OnRot2nd270 ();
		afx_msg void OnHFlip ();
		afx_msg void OnVFlip ();
		afx_msg void OnLight1 ();
		afx_msg void OnLight2 ();
		afx_msg void OnLight3 ();
		afx_msg void OnLight4 ();
		afx_msg void OnLight5 ();
		afx_msg void OnLight6 ();
		afx_msg void OnLight7 ();
		afx_msg void OnLight8 ();
		afx_msg void OnLight9 ();
		afx_msg void OnLight10 ();
		afx_msg void OnLight11 ();
		afx_msg void OnLight12 ();
		afx_msg void OnLight13 ();
		afx_msg void OnLight14 ();
		afx_msg void OnLight15 ();
		afx_msg void OnLight16 ();
		afx_msg void OnLight17 ();
		afx_msg void OnLight18 ();
		afx_msg void OnLight19 ();
		afx_msg void OnLight20 ();
		afx_msg void OnLight21 ();
		afx_msg void OnLight22 ();
		afx_msg void OnLight23 ();
		afx_msg void OnLight24 ();
		afx_msg void OnLight25 ();
		afx_msg void OnLight26 ();
		afx_msg void OnLight27 ();
		afx_msg void OnLight28 ();
		afx_msg void OnLight29 ();
		afx_msg void OnLight30 ();
		afx_msg void OnLight31 ();
		afx_msg void OnLight32 ();
		afx_msg void OnLightEdit ();
		afx_msg void OnLightOff ();
		afx_msg void OnLightOn ();
		afx_msg void OnLightStrobe4 ();
		afx_msg void OnLightStrobe8 ();
		afx_msg void OnLightFlicker ();
		afx_msg void OnLightDefault ();
		afx_msg void OnBrightnessEdit ();
		afx_msg void OnLightTimerEdit ();
		afx_msg void OnAddLight ();
		afx_msg void OnDeleteLight ();
		afx_msg void OnLButtonDown (UINT nFlags, CPoint point);
		afx_msg void ChooseRGBColor (void);

		virtual BOOL OnNotify (WPARAM wParam, LPARAM lParam, LRESULT *pResult);
		void OnEditTimer (void);

		void SetLightString ();
		void SetLightButtons (LPSTR szLight = NULL, int nSpeed = -1);
		bool SetLightDelay (int nSpeed = -1);
		void GetBrightness (int nTexture);
		void SetBrightness (int nBrightness = 0);
		void UpdateLightWnd (void);
		void UpdateLight (void);
		void EnableLightControls (BOOL bEnable);

		void ToggleLight (int i);
		void AnimateTexture (void);
		void AnimateLight (void);
		void OnHScroll(UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);
		void OnVScroll(UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);
		void Rot2nd (int iAngle);
		void HAlign (int dir);
		void VAlign (int dir);
		void RAlign (int dir);
		void HFlip (void);
		void VFlip (void);
		void SelectTexture (int nIdC, bool bFirst);
		void PasteTexture (INT16 segnum, INT16 sidenum, INT16 nDepth);
		bool GetAdjacentSide (INT16 start_segment, INT16 start_side, INT16 linenum,
									 INT16 *neighbor_segnum, INT16 *neighbor_sidenum);
		bool SideHasLight (void);
		void CreateColorCtrl (CWnd *pWnd, int nIdC);
		void UpdateColorCtrl (CWnd *pWnd, COLORREF color);
		void RotateUV (double angle, bool bUpdate = true);
		void Refresh ();
		void RefreshTextureWnd ();
		void RefreshAlignWnd ();
		void UpdateTextureWnd (void);
		void RefreshAlignment ();
		void UpdateAlignWnd (void);
		void UpdatePaletteWnd (void);
		void DrawTexture (INT16 texture1, INT16 texture2, int x0, int y0);
		void DrawAlignment (CDC *pDC);
		int ScrollSpeed (UINT16 texture,int *x,int *y);
		int AlignTextures (INT16 start_segment, INT16 start_side, INT16 only_child);
		void AlignChildTextures (int segnum, int sidenum, int nDepth);
		void AlignChildren (INT16 segnum, INT16 sidenum, bool bStart);
		void SetWallColor (void);

		inline CScrollBar *HScrollAlign ()
			{ return (CScrollBar *) GetDlgItem (IDC_TEXALIGN_HSCROLL); }
		inline CScrollBar *VScrollAlign ()
			{ return (CScrollBar *) GetDlgItem (IDC_TEXALIGN_VSCROLL); }
		inline CComboBox *CBTexture1 ()
			{ return CBCtrl(IDC_TEXTURE_1ST); }
		inline CComboBox *CBTexture2 ()
			{ return CBCtrl(IDC_TEXTURE_2ND); }
		inline CButton *LightButton (int i)
			{ return BtnCtrl (IDC_TEXLIGHT_1 + i); }
		inline CScrollBar *TimerSlider ()
			{ return (CScrollBar *) GetDlgItem (IDC_TEXLIGHT_TIMERSLIDER); }
		inline CScrollBar *BrightnessSlider ()
			{ return (CScrollBar *) GetDlgItem (IDC_TEXTURE_BRIGHTSLIDER); }

		DECLARE_MESSAGE_MAP()
};

                        /*--------------------------*/

class CLightTool : public CToolDlg
{
	public:
		int		m_bIlluminate;
		int		m_bAvgCornerLight;
		int		m_bScaleLight;
		int		m_bCubeLight;
		int		m_bDynCubeLights;
		int		m_bDeltaLight;
		double	m_fBrightness;
		double	m_fLightScale;
		double	m_fCubeLight;
		double	m_fDeltaLight;
		double	m_fVertexLight;
		int		m_nNoLightDeltas;
		int		m_lightRenderDepth;
		int		m_deltaLightRenderDepth;
		int		m_deltaLightFrameRate;
		int		m_bShowLightSource;
		int		m_bCopyTexLights;

		CLightTool (CPropertySheet *pParent = NULL);
      virtual BOOL OnInitDialog ();
		void SetDefaults (void);
		void OnOK (void);
		void OnCancel (void);
      virtual void DoDataExchange (CDataExchange *pDX);
		afx_msg void OnShowDelta ();
		afx_msg void OnShowLightSource ();
		afx_msg void OnSetVertexLight ();
		afx_msg void OnDefaultLightAndColor ();
		afx_msg void OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);

		DECLARE_MESSAGE_MAP()
	};

                        /*--------------------------*/

class CPrefsDlg : public CToolDlg
{
	public:
		CBitmapButton	m_btnBrowseD1PIG;
		CBitmapButton	m_btnBrowseD2PIG;
		CBitmapButton	m_btnBrowseMissions;
		char				m_d1Path [256];
		char				m_d2Path [256];
		char				m_missionsPath [256];
		UINT32			m_mineViewFlags;
		UINT32			m_objViewFlags;
		UINT32			m_texViewFlags;
		int				m_iDepthPerception;
		double			m_depthPerceptions [4];
		int				m_iRotateRate;
		double			m_rotateRates [5];
		double			m_moveRate;
		int				m_bExpertMode;
		int				m_bUseTexColors;
		int				m_nViewDist;
		int				m_nMineCenter;
		int				m_nMaxUndo;
		BOOL				m_bSplashScreen;
		bool				m_bNoRefresh;
		bool				m_bInvalid;

		CPrefsDlg (CPropertySheet *pParent = NULL);
		~CPrefsDlg ();
		bool BrowseFile (LPSTR fileType, LPSTR fileName, LPSTR fileExt, BOOL bOpen);
	   virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		virtual void OnOK (void);
		virtual void OnCancel (void);
		afx_msg void OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);
		afx_msg void OnOpenD1PIG (void);
		afx_msg void OnOpenD2PIG (void);
		afx_msg void OnOpenMissions (void);
		afx_msg void OnViewMineNone (void);
		afx_msg void OnViewMineAll (void);
		afx_msg void OnViewObjectsNone (void);
		afx_msg void OnViewObjectsAll (void);
		afx_msg void OnLayout0 (void);
		afx_msg void OnLayout1 (void);
		afx_msg void OnSetMineCenter (void);
		void SetLayout (int nLayout);
		void FreeTextureHandles (bool bDeleteModified = true);
		void Refresh (void);
		void WritePrivateProfileInt (LPSTR szKey, int nValue);
		void WritePrivateProfileDouble (LPSTR szKey, double nValue);
		void GetAppSettings ();
		void SetAppSettings (bool bInitApp = false);
		void SaveAppSettings (bool bSaveFolders = true);
		void CompletePath (LPSTR pszPath, LPSTR pszFile, LPSTR pszExt);
		inline CScrollBar *ViewDistSlider ()
			{ return (CScrollBar *) GetDlgItem (IDC_PREFS_VIEWDIST); }
		inline CComboBox *CBMineCenter ()
			{ return CBCtrl(IDC_PREFS_MINECENTER); }

		DECLARE_MESSAGE_MAP()
};

                        /*--------------------------*/

class CToolView : public CWnd {
	private:
		int				m_scrollRange [2];
		int				m_scrollPage [2];
		int				m_scrollOffs [2];
		CSize				m_toolSize;
		BOOL				m_bHScroll;
		BOOL				m_bVScroll;
		BOOL				m_bRecalcLayout;

	public:
		DECLARE_DYNCREATE(CToolView)

		CPropertySheet	*m_pTools;
		CCubeTool		*m_cubeTool;
		CWallTool		*m_wallTool;
		CTriggerTool	*m_triggerTool;
		CTextureTool	*m_textureTool;
		CObjectTool		*m_objectTool;
		CEffectTool		*m_effectTool;
		CAdvObjTool		*m_advObjTool;
		CLightTool		*m_lightTool;
		CPrefsDlg		*m_prefsDlg;
		CMissionTool	*m_missionTool;
		CReactorTool	*m_reactorTool;
		CDiagTool		*m_diagTool;
		CTxtFilterTool	*m_txtFilterTool;

		CToolView ();
		~CToolView ();
		void Setup ();
		void Reset () {};
		afx_msg int OnCreate (LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnDestroy (void);
		afx_msg void OnPaint ();
		afx_msg BOOL OnEraseBkgnd (CDC* pDC);
		afx_msg void OnHScroll (UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnVScroll (UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnSelectPrevTab ();
		afx_msg void OnSelectNextTab ();
		void CalcToolSize (void);
		void RecalcLayout (int nToolMode = 0, int nTextureMode = 0);
		void MoveWindow (int x, int y, int nWidth, int nHeight, BOOL bRepaint = TRUE);
		void MoveWindow (LPCRECT lpRect, BOOL bRepaint = TRUE);
		inline CSize& ToolSize ()
			{ return m_toolSize; }
		void SetActive (int nPage);
		inline void EditTexture ()
			{ SetActive (0); }
		inline void EditCube ()
			{ SetActive (1); }
		inline void EditWall ()
			{ SetActive (2); }
		inline void EditTrigger ()
			{ SetActive (3); }
		inline void EditObject ()
			{ SetActive (4); }
		inline void EditEffect ()
			{ SetActive (4); }
		inline void EditAdvObj ()
			{ SetActive (6); }
		inline void EditLight ()
			{ SetActive (7); }
		inline void EditReactor ()
			{ SetActive (8); }
		inline void EditMission ()
			{ SetActive (9); }
		inline void EditDiag ()
			{ SetActive (10); }
		inline void EditTxtFilters ()
			{ SetActive (11); }
		inline void EditPrefs ()
			{ SetActive (12); }
		inline CCubeTool *CubeTool ()
			{ return m_cubeTool; }
		inline CTextureTool *TextureTool ()
			{ return m_textureTool; }
		inline CWallTool *WallTool ()
			{ return m_wallTool; }
		inline CTriggerTool *TriggerTool ()
			{ return m_triggerTool; }
		inline CLightTool *LightTool ()
			{ return m_lightTool; }
		inline CObjectTool *ObjectTool ()
			{ return m_objectTool; }
		inline CMissionTool *MissionTool ()
			{ return m_missionTool; }
		inline CDiagTool *DiagTool ()
			{ return m_diagTool; }
		inline CTxtFilterTool *TxtFilter ()
			{ return m_txtFilterTool; }
		inline CPrefsDlg *PrefsDlg ()
			{ return m_prefsDlg; }
		void Refresh ();
		void CycleTab (int nDir);
		void NextTab ();
		void PrevTab ();
	DECLARE_MESSAGE_MAP()
};
                        
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif //__toolview_h
