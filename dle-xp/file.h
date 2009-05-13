#ifndef __file_h
#define __file_h

#define MAX_HOGFILES	1000

                         /*--------------------------*/

class CInputDialog : public CDialog {
	public:
		LPSTR		m_pszTitle;
		LPSTR		m_pszPrompt;
		LPSTR		m_pszBuf;
		size_t	m_nBufSize;

		CInputDialog (CWnd *pParentWnd = NULL, LPSTR pszTitle = NULL, LPSTR pszPrompt = NULL, LPSTR pszBuf = NULL, size_t nBufSize = 0);
		virtual BOOL OnInitDialog (void);
		virtual void DoDataExchange (CDataExchange * pDX);
		void OnOK (void);
};

                         /*--------------------------*/

typedef struct tHogFileData {
	long	m_offs;
	long	m_size;
	int	m_fileno;
} tHogFileData;

class CHogManager : public CDialog {
	public:
		bool				m_bInited;
		char				m_name [256];
		LPSTR				m_pszFile;
		LPSTR				m_pszSubFile;
		int				m_type;
		int				*m_pType;
		int				m_bShowAll;
//		tHogFileData	m_fileData;

		CHogManager (CWnd *pParentWnd = NULL, LPSTR pszFile = NULL, LPSTR pszSubFile = NULL);
		virtual BOOL OnInitDialog (void);
		virtual void DoDataExchange (CDataExchange * pDX);
		void EndDialog (int nResult);
		int FindFilename (LPSTR pszName);
		void ClearFileList (void);
		int AddFile (LPSTR pszName, long size, long offset, int fileno);
		int DeleteFile (int index = -1);
		int GetFileData (int index = -1, long *size = NULL, long *offset = NULL);
		int AddFileData (int index, long size, long offset, int fileno);
		bool LoadLevel (LPSTR pszFile = NULL, LPSTR pszSubFile = NULL);
		void OnOK (void);
		void OnCancel (void);
		bool ReadHogData ();
		void Reset (void);
		afx_msg void OnRename ();
		afx_msg void OnDelete ();
		afx_msg void OnImport ();
		afx_msg void OnExport ();
		afx_msg void OnFilter ();
		afx_msg void OnSetFile ();
		inline CListBox *LBFiles ()
			{ return (CListBox *) GetDlgItem (IDC_HOG_FILES); }

	DECLARE_MESSAGE_MAP ()
	};

                         /*--------------------------*/

bool BrowseForFile (BOOL bOpen, LPSTR pszDefExt, LPSTR pszFile, LPSTR pszFilter, DWORD nFlags = 0, CWnd *pParentWnd = NULL);
int SaveToHog (LPSTR szHogFile, LPSTR szSubFile, bool bSaveAs);
bool ReadHogData (LPSTR pszFile, CListBox *plb, bool bAllFiles, bool bOnlyLevels, bool bGetFileData = true);
bool FindFileData (LPSTR pszFile, LPSTR pszSubFile, long *nSize, long *nPos, BOOL bVerbose = TRUE);
bool ExportSubFile (const char *pszSrc, const char *pszDest, long offset, long size);
int ReadMissionFile (char *pszFile);
int WriteMissionFile (char *pszFile, bool bSaveAs = true);
int MakeMissionFile (char *pszFile, char *pszSubFile, int bCustomTextures, int bCustomRobots, bool bSaveAs = true);

                         /*--------------------------*/

#endif //__file_h
