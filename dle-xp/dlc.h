// dlc.h : main header file for the DLC application
//

#ifndef __dlc_h
#define __dlc_h

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "dlcres.h"       // main symbols
#include "dlc_i.h"
#include "mainfrm.h"
#include "mine.h"

extern int nLayout;

/////////////////////////////////////////////////////////////////////////////
// CDlcApp:
// See dlc.cpp for the implementation of this class
//

#define MAX_UNDOS		100

struct tUndoBuffer;

typedef struct tUndoBuffer {
	tUndoBuffer	*prevBuf;
	tUndoBuffer	*nextBuf;
	MINE_DATA	undoBuffer;
} tUndoBuffer;

class CUndoList 
{
	public:
		tUndoBuffer	*m_head;
		tUndoBuffer	*m_tail;
		tUndoBuffer	*m_current;
		int			m_maxSize;
		int			m_size;
		int			m_delay;
		int			m_enabled;

	CUndoList (int maxSize = 100);
	~CUndoList ();
	bool Update (bool bForce = false);
	bool Undo ();
	bool Redo ();
	void Truncate ();
	void Reset ();
	bool Revert ();
	void Delay (bool bDelay);
	int UndoCount ();
	int Enable (int bEnable);
	int SetMaxSize (int maxSize);
	inline int GetMaxSize (void)
		{ return m_maxSize; }
};


class CDlcApp : public CWinApp
{
public:
	CDlcDocTemplate	*m_pDlcDoc;
	char					m_szCaption [128];
	char					m_szExtCaption [256];
	CUndoList			m_undoList;
	int					m_delayUndo;
	int					m_nModified;
	BOOL					m_bSplashScreen;
	bool					m_bMaximized;

	CDlcApp();
	void DelayUndo (bool bDelay);
	bool UpdateUndoBuffer (bool bForce = false);
	bool RevertUndoBuffer ();
	void ResetUndoBuffer ();
	bool Undo ();
	bool Redo ();
	int UndoCount ();
	int EnableUndo (int bEnable);
	inline void LockUndo ()
		{ DelayUndo (true); }
	inline void UnlockUndo ()
		{ DelayUndo (false); }
	inline CMainFrame *MainFrame ()
		{ return (CMainFrame *) m_pMainWnd; }
	inline CMineView *MineView ()
		{ CMainFrame *h; return (h = MainFrame ()) ? h->MineView () : NULL; }
	inline CTextureView *TextureView ()
		{ CMainFrame* h; return (h = MainFrame ()) ? h ->TextureView () : NULL; }
	inline CToolView *ToolView ()
		{ CMainFrame* h; return (h = MainFrame ()) ? MainFrame ()->ToolView () : NULL; }
	inline CDlcDoc *GetDocument ()
		{ CMineView *h; return (h = MineView ()) ? h->GetDocument () : NULL; }
	inline CMine *GetMine ()
		{ CDlcDoc *h; return (h = GetDocument ()) ? h->m_mine : NULL; }
	inline CWnd *TexturePane ()
		{ return MainFrame ()->TexturePane (); }
	inline CWnd *MinePane ()
		{ return MainFrame ()->MinePane (); }
	inline CWnd *ToolPane ()
		{ return MainFrame ()->ToolPane (); }
	inline CSize& ToolSize ()
		{ return ToolView ()->ToolSize (); }
	bool SetModified (BOOL bModified);
	void ResetModified (bool bRevertUndo);
	CDocument* CDlcApp::OpenDocumentFile(LPCTSTR lpszFileName);
	void WritePrivateProfileInt (LPSTR szKey, int nValue);
	void SaveLayout ();
	void LoadLayout ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlcApp)
	public:
	virtual BOOL InitInstance();
		virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	COleTemplateServer m_server;
		// Server object for document creation

	//{{AFX_MSG(CDlcApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL m_bATLInited;
private:
	BOOL InitATL();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

extern CDlcApp	theApp;

#endif //__dlc_h
