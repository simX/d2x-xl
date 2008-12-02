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
COPYRIGHT 1993-1998 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#ifndef _PALETTE_H
#define _PALETTE_H

#define DEFAULT_LEVEL_PALETTE "groupa.256" //don't confuse with DEFAULT_PALETTE
#define D1_PALETTE "palette.256"

#include "color.h"
#include "cfile.h"

#define PALETTE_SIZE				256
#define MAX_COMPUTED_COLORS	64
#define MAX_FADE_LEVELS			34
#define FADE_LEVELS				31
#define FADE_RATE					16		//	gots to be a power of 2, else change the code in DiminishPaletteTowardsNormal

typedef struct tBGR {
	ubyte	b,g,r;
} tBGR;

typedef struct tRGB {
	ubyte	r,g,b;
} tRGB;

typedef union tPalette {
	ubyte			raw [PALETTE_SIZE * 3];
	tRgbColorb	rgb [PALETTE_SIZE];
} tPalette;

typedef struct tComputedColor {
	ubyte			nIndex;
	tRgbColorb	color;
} tComputedColor;

class CPalette {
	private:
		tPalette			m_data;
		tComputedColor	m_computedColors [MAX_COMPUTED_COLORS];
		short				m_nComputedColors;

	public:
		CPalette () {};
		~CPalette () {};
		void Init (void);
		void ClearStep ();
		bool Read (CFile& cf);
		bool Write (CFile& cf);
		int ClosestColor (int r, int g, int b);
		void SwapTransparency (void);
		void AddComputedColor (int r, int g, int b, int nIndex);
		void InitComputedColors (void);
		void ToRgbaf (ubyte nColor, tRgbaColorf& color);

		inline tPalette& Data (void) { return m_data; }
		inline tRgbColorb *Color (void) { return m_data.rgb; }
		inline ubyte *Raw (void) { return m_data.raw; }
		inline void Skip (CFile& cf) { cf.Seek (sizeof (m_data), SEEK_CUR); }
		inline size_t Size (void) { return sizeof (m_data); }
		inline void SetBlack (ubyte r, ubyte g, ubyte b) 
			{ m_data.rgb [0].red = r, m_data.rgb [0].green = g, m_data.rgb [0].blue = b; }
		inline void SetTransparency (ubyte r, ubyte g, ubyte b) 
			{ m_data.rgb [PALETTE_SIZE - 1].red = r, m_data.rgb [PALETTE_SIZE - 1].green = g, m_data.rgb [PALETTE_SIZE - 1].blue = b; }
		inline CPalette& operator= (CPalette& source) { 
			memcpy (&Data (), &source.Data (), sizeof (tPalette));
			Init ();
			return *this;
			}
		inline bool operator== (CPalette& source) 
			{ return !memcmp (&Data (), &source.Data (), sizeof (tPalette)); }
	};

//------------------------------------------------------------------------------

typedef struct tPaletteList {
	struct tPaletteList	*next;
	CPalette					palette;
} tPaletteList;

//------------------------------------------------------------------------------

typedef struct tPaletteData {
		CPalette			*default;
		CPalette			*fade;
		CPalette			*game;
		CPalette			*last;
		CPalette			*D1;
		CPalette			*texture;
		CPalette			*current;
		CPalette			*prev;
		tPaletteList	*list;
		char				szLastPaletteLoaded [FILENAME_LEN];
		char				szLastPalettePig [FILENAME_LEN];
		int				nPalettes;
		int				nGamma;
		int				nLastGamma;
		fix				xEffectDuration;
		fix				xLastEffectTime;

		ubyte				fadeTable [PALETTE_SIZE * MAX_FADE_LEVELS];
		tRgbColorb		flash;
		tRgbColorf		fflash;
		tRgbColors		effect;
		tRgbColors		lastEffect;
		bool				bDoEffect;
		bool				bFadedOut;
} tPaletteData;

class CPaletteManager {
	private:
		tPaletteData	m_data;

	public:
		CPaletteManager () { Init (); };
		~CPaletteManager () { Destroy (); }
		void Init (void);
		void Destroy (void); 
		CPalette *Find (CPalette& palette);
		CPalette *Add (CPalette& palette);
		CPalette *Add (ubyte* buffer);
		CPalette *Load (const char *pszPaletteName, const char *pszLevelName, int nUsedForLevel, int bNoScreenChange, int bForce);
		CPalette *Load (const char* filename, const char* levelname);
		CPalette* LoadD1 (void);
		int FindClosestColor15bpp (int rgb);
		void SetGamma (int gamma);
		int GetGamma (void);
		void Flash (void);

		void ClearStep (void);
		void StepUp (int r, int g, int b);
		void StepUp (void);
		void ApplyEffect (void);
		void FadeEffect (void);
		void ResetEffect (void);
		void SaveEffect (void);
		void SaveEffectAndReset (void);
		void LoadEffect (CPalette* palette = NULL);
		void SetEffect (int r, int g, int b);
		void BumpEffect (int dr, int dg, int db);
		inline void SetRedEffect (int color) { m_data.effect.red = color; }
		inline void SetGreenEffect (int color) { m_data.effect.green = color; }
		inline void SetBlueEffect (int color) { m_data.effect.blue = color; }
		void SetEffect (void);
		void ClearEffect (void);
		int ClearEffect (CPalette* palette);
		int FadeIn (void);
		int FadeOut (void);
		inline bool FadedOut (void) { return m_data.bFadedOut; }
		void SetPrev (CPalette *palette) { m_data.prev = palette; }
		inline CPalette* Activate (CPalette* palette) {
			if (palette && (m_data.current != palette)) {
				m_data.current = palette;
				m_data.current->Init ();
				}
			return m_data.current;
			}

		inline sbyte RedEffect (void) { return m_data.effect.red; }
		inline sbyte GreenEffect (void) { return m_data.effect.green; }
		inline sbyte BlueEffect (void) { return m_data.effect.blue; }
		inline void SetRedEffect (sbyte color) {  m_data.effect.red = color; }
		inline void SetGreenEffect (sbyte color) {  m_data.effect.green = color; }
		inline void SetBlueEffect (sbyte color) {  m_data.effect.blue = color; }
		inline void SetEffectDuration (fix xDuration) { m_data.xEffectDuration = xDuration; }
		inline void SetLastEffectTime (fix xTime) { m_data.xLastEffectTime = xTime; }
		inline fix EffectDuration (void) { return m_data.xEffectDuration; }
		inline fix LastEffectTime (void) { return m_data.xLastEffectTime; }

		inline void SetGame (CPalette* palette) { m_data.game = palette; }
		inline void SetFade (CPalette* palette) { m_data.fade = palette; }
		inline void SetTexture (CPalette* palette) { m_data.texture = palette; }
		inline void SetD1 (CPalette* palette) { m_data.D1 = palette; }

		inline CPalette* Default (void) { return m_data.default; }
		inline CPalette* Current (void) { return m_data.current; }
		inline CPalette* Game (void) { return m_data.game; }
		CPalette* Texture (void) { return m_data.texture; };
		inline CPalette* D1 (void) { return m_data.D1; }
		inline void SetDefault (CPalette* defPal) { m_data.default = defPal; }
		inline CPalette* Fade (CPalette* fadePal) { m_data.fade = fadePal; }
		inline CPalette* GetFade (void) { return m_data.fade; }
		inline ubyte* FadeTable (void) { return m_data.fadeTable; }
		inline bool DoEffect (void) { return m_data.bDoEffect; }

		inline int ClosestColor (int r, int g, int b)
			{ return m_data.current ? m_data.current->ClosestColor (r, g, b) : 0; }
	};

extern CPaletteManager paletteManager;

#endif //_PALETTE_H
