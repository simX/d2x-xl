// Copyright (C) 1997 Bryan Aamot
#ifndef __texedit_h
#define __texedit_h

                        /*--------------------------*/

class CPaletteWnd : public CWnd
{
	public:
		int				m_nWidth;
		int				m_nHeight;
		CWnd				*m_pParentWnd;
		CDC				*m_pDC;
		CPalette			*m_pOldPal;
		UINT16			m_nColor;
		UINT8				m_nSortedPalIdx [256];
		PALETTEENTRY	m_palColors [256];

		CPaletteWnd ();
		~CPaletteWnd ();
		int Create (CWnd *pParentWnd = NULL, int nWidth = 32, int nHeight = 8);
		bool SelectColor (CPoint& point, int& color, PALETTEENTRY *pRGB = NULL);
		void DrawPalette (void);
		void CPaletteWnd::SetPalettePixel (int x, int y);
		bool BeginPaint ();
		void EndPaint ();
		void Update ();
		void CreatePalette ();
		void SortPalette (int l, int r);
		int CmpColors (PALETTEENTRY *c, PALETTEENTRY *m);
#if 0		
		afx_msg void OnLButtonUp (UINT nFlags, CPoint point);
		afx_msg void OnRButtonUp (UINT nFlags, CPoint point);
		afx_msg void OnLButtonDown (UINT nFlags, CPoint point);
		afx_msg void OnRButtonDown (UINT nFlags, CPoint point);
#endif		
	DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CTextureEdit : public CDialog 
{
	public:
		CWnd			m_textureWnd;
		CPaletteWnd	m_paletteWnd;
		CWnd			m_layerWnd;
		CDTexture	m_texture;
		UINT8			*m_bitmap,
						*m_backupBM;
		tRGBA			*m_tga,
						*m_backupTGA;
		int			m_fgColor,
						m_bgColor;
		bool			m_lBtnDown,
						m_rBtnDown;
		bool			m_bModified;
		int			m_iTexture;
		char			m_szColors [80];
		CDC			*m_pDC;
		CWnd			*m_pPaintWnd;
		CPalette		*m_pOldPal;
		CDTexture	*m_pTx;
		UINT32		m_nWidth,
						m_nHeight,
						m_nSize,
						m_nFormat,
						m_nOldWidth,
						m_nOldHeight,
						m_nOldSize,
						m_nOldFormat;
		char			m_szDefExt [5];

		CTextureEdit (CWnd * pParent = NULL);
		~CTextureEdit ();
      virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		void OnOK (void);
		void Backup (void);
		void ColorPoint (UINT nFlags, CPoint &point, int& color);
		void OnButtonDown (UINT nFlags, CPoint point, int& color);
		void Refresh ();
		bool BeginPaint (CWnd *pWnd);
		void EndPaint ();
		void DrawTexture (void);
		void DrawPalette (void);
		void DrawLayers (void);
		void SetTexturePixel (int x, int y);
		void SetPalettePixel (int x, int y);
		void Update (CWnd *pWnd);
		void GetClientRect (CWnd *pWnd, CRect& rc);
		bool PtInRect (CRect& rc, CPoint& pt);
		bool LoadBitmap (FILE *file);
		bool LoadTGA (FILE *file);
		void SaveBitmap (FILE *file);
		void SaveTGA (FILE *file);

		afx_msg void OnMouseMove (UINT nFlags, CPoint point);
		afx_msg void OnLButtonDown (UINT nFlags, CPoint point);
		afx_msg void OnRButtonDown (UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp (UINT nFlags, CPoint point);
		afx_msg void OnRButtonUp (UINT nFlags, CPoint point);
		afx_msg void OnPaint (void);
		afx_msg void OnSave (void);
		afx_msg void OnLoad (void);
		afx_msg void OnDefault (void);
		afx_msg void OnUndo (void);
	DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

#endif __texedit_h
