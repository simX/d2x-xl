// dlcDoc.h : interface of the CDlcDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DLCDOC_H__C46B3F4A_5EFF_11D2_AE2A_00C0F03014A5__INCLUDED_)
#define AFX_DLCDOC_H__C46B3F4A_5EFF_11D2_AE2A_00C0F03014A5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "mine.h"
#include "script.h"

class CDlcDoc : public CDocument
{
protected: // create from serialization only
	CDlcDoc();
	DECLARE_DYNCREATE(CDlcDoc)

// Member variables
public:
// Attributes
	CMine*		m_mine;
	bool			m_bInitDocument;
	char			m_szFile [256];
	char			m_szSubFile [256];
	char			m_szTmpFile [256];
public:
// Operations
	void CreateNewLevel ();
	bool BrowseForFile (LPSTR pszFile, BOOL bOpen);
	BOOL OpenFile (bool bBrowseForFile = true, LPSTR pszFile = NULL, LPSTR pszSubFile = NULL);
	bool SaveFile (bool bSaveAs);
	bool SaveIfModified (void);
	void UpdateCaption ();
	inline char *File ()
		{ return m_szFile; }
	inline char *SubFile ()
		{ return m_szSubFile; }
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlcDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDlcDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CDlcDoc)
	afx_msg void OnOpenFile();
	afx_msg void OnSaveFile();
	afx_msg void OnSaveFileAs();
	afx_msg void OnRunLevel();
	afx_msg void OnInsertCube();
	afx_msg void OnDeleteCube();
	afx_msg void OnInsertCubeReactor ();
	afx_msg void OnInsertCubeRobotMaker ();
	afx_msg void OnInsertCubeFuelCenter ();
	afx_msg void OnInsertCubeRepairCenter ();
	afx_msg void OnInsertCubeWater ();
	afx_msg void OnInsertCubeLava ();
	afx_msg void OnInsertDoorNormal ();
	afx_msg void OnInsertDoorPrison ();
	afx_msg void OnInsertDoorGuideBot ();
	afx_msg void OnInsertDoorExit ();
	afx_msg void OnInsertDoorSecretExit ();
	afx_msg void OnInsertTriggerOpenDoor ();
	afx_msg void OnInsertTriggerRobotMaker ();
	afx_msg void OnInsertTriggerShieldDrain ();
	afx_msg void OnInsertTriggerEnergyDrain ();
	afx_msg void OnInsertTriggerControlPanel ();
	afx_msg void OnInsertWallFuelCells ();
	afx_msg void OnInsertWallIllusion ();
	afx_msg void OnInsertWallForceField ();
	afx_msg void OnInsertWallFan ();
	afx_msg void OnInsertWallGrate ();
	afx_msg void OnInsertWallWaterfall ();
	afx_msg void OnInsertWallLavafall ();
	afx_msg void OnInsertObjectPlayer ();
	afx_msg void OnInsertObjectCoopPlayer ();
	afx_msg void OnInsertObjectPlayerCopy ();
	afx_msg void OnInsertObjectRobot ();
	afx_msg void OnInsertObjectWeapon ();
	afx_msg void OnInsertObjectPowerup ();
	afx_msg void OnInsertObjectReactor ();
	afx_msg void OnInsertObjectGuideBot ();
	afx_msg void OnDeleteObject ();
	afx_msg void OnDeleteWall ();
	afx_msg void OnDeleteTrigger ();
	afx_msg void OnCutBlock ();
	afx_msg void OnCopyBlock ();
	afx_msg void OnQuickCopyBlock ();
	afx_msg void OnPasteBlock ();
	afx_msg void OnQuickPasteBlock ();
	afx_msg void OnDeleteBlock ();
	afx_msg void OnCopyOtherCube ();
	afx_msg void OnFileTest ();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CAutoDoc)
	long Member1;
	afx_msg long Test();
	afx_msg void SetMyText(LPCTSTR string);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()

};

class CDlcDocTemplate : public CSingleDocTemplate {
	public:
		CRuntimeClass	*m_pFrameClass;
		CDlcDocTemplate(
			UINT nIDResource,
			CRuntimeClass* pDocClass,
			CRuntimeClass* pFrameClass,
			CRuntimeClass* pViewClass)
			: CSingleDocTemplate (nIDResource, pDocClass, pFrameClass, pViewClass) 
			{ m_pFrameClass = pFrameClass; }

		CFrameWnd* CreateNewFrame(CDocument* pDoc, CFrameWnd* pOther) {
			return m_pFrameClass ? CDocTemplate::CreateNewFrame(pDoc, pOther) : NULL;
		}
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLCDOC_H__C46B3F4A_5EFF_11D2_AE2A_00C0F03014A5__INCLUDED_)

