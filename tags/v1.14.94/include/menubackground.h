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

#ifndef _MENUBACKGROUND_H
#define _MENUBACKGROUND_H

#include "bitmap.h"
#include "canvas.h"

//------------------------------------------------------------------------------

#define BG_MENU		0
#define BG_STARS		1
#define BG_SCORES		2
#define BG_MAP			3

//------------------------------------------------------------------------------

char* BackgroundName (int nType, int bHires = -1);

//------------------------------------------------------------------------------

class CBackground {
	private:
		CCanvas*	m_canvas [2];		// canvas (screen area) of a menu
		CBitmap*	m_saved [2];		// copy of a screen area covered by a menu
		CBitmap*	m_background;		// complete background
		char*		m_filename;
		bool		m_bIgnoreCanv;
		bool		m_bIgnoreBg;
		bool		m_bTopMenu;
		bool		m_bMenuBox;
		bool		m_bSetup;

	public:
		CBackground () { Init (); }
		~CBackground () { Destroy (); }
		void Init (void);
		void Destroy (void);
		bool Create (char* filename, int x, int y, int width, int height, bool bTop = false);
		void Restore (void);
		void Restore (int dx, int dy, int w, int h, int sx, int sy);
		void Draw (bool bDrawBox, bool bUpdate = false);
		void DrawArea (int left, int top, int right, int bottom);
		void DrawBox (void);

		inline CCanvas* Canvas (uint i = 0) { return m_canvas [i]; }
		inline CBitmap* Background (void) { return m_saved [0]; }
		inline CBitmap* Saved (int i) { return m_saved [i]; }

	private:
		CBitmap* Load (char* filename, int width, int height);
		void Setup (int x, int y, int width, int height);
		void Save (int i, int width, int height);
};

//------------------------------------------------------------------------------

class CBackgroundManager : public CStack<CBackground> {
	private:
		CBackground				m_bg [3];
		CBitmap*					m_background [2];
		char*						m_filename [2];
		int						m_nDepth;
		bool						m_bValid;
		bool						m_bShadow;

	public:
		CBackgroundManager ();
		void Init (void);
		void Destroy (void);
		void Create (void);
		bool Setup (char *filename, int x, int y, int width, int height, bool bTop = false);
		void Load (void);
		inline void Restore (void) { m_bg [m_nDepth].Restore (); }
		inline void Restore (int dx, int dy, int w, int h, int sx, int sy) { m_bg [m_nDepth].Restore (dx, dy, w, h, sy, sy); }
		void Remove (void);
		void Rebuild (void);

		CBitmap* LoadBackground (char* filename);
		bool IsDefault (char* filename);

		inline char* Filename (uint i = 0) { return m_filename [i]; }
		inline CBitmap* Background (uint i = 0) { return m_background [i]; }
		inline int Depth (void) { return m_nDepth; }
		inline bool Shadow (void) { return m_bShadow; }
		inline void SetShadow (bool bShadow) { m_bShadow = bShadow; }
		inline void Draw (bool bUpdate = false) { 
			if (m_nDepth >= 0) 
				m_bg [m_nDepth].Draw (bUpdate); 
			}
		inline void DrawArea (int left, int top, int right, int bottom)
		 { m_bg [m_nDepth].DrawArea (left, top, right, bottom); }
		inline CBitmap* Current (void) { return m_bg [m_nDepth].Background (); }
		inline CCanvas* Canvas (uint i = 0) { return m_bg [m_nDepth].Canvas (i); }

		void Redraw (bool bUpdate = false);
		void DrawBox (int left, int top, int right, int bottom, int nLineWidth, float fAlpha, int bForce);
		inline void DrawBox (int nLineWidth, float fAlpha, int bForce) {
			DrawBox (CCanvas::Current ()->Left (), CCanvas::Current ()->Top (), 
						CCanvas::Current ()->Right (), CCanvas::Current ()->Bottom (), 
						nLineWidth, fAlpha, bForce);
			}

		void LoadStars (bool bTop = false);

	private:
		CBitmap* LoadCustomBackground (void);
	};

//------------------------------------------------------------------------------

extern  CBackgroundManager backgroundManager;

//------------------------------------------------------------------------------

#endif // _MENUBACKGROUND_H 
